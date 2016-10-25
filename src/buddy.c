#include "general.h"
#include "algo.h"
#include "buddy.h"
#include "io.h"
#include "memory.h"

typedef struct buddy_node buddy_node;

struct buddy_node {
    uint8_t order:7, 
            used:1;
    buddy_node *prev, *next;
};

static void make_node(buddy_node *p,
                      uint8_t order,
                      bool used,
                      buddy_node *prev, 
                      buddy_node *next) {
    p->used = used;
    p->order = order;
    p->prev = prev;
    p->next = next;
}

define_list_operations(buddy_node)

/* as memory map can have several free segments we have to be able to create 
   more than one allocator */
typedef struct buddy_allocator {
    ptr begin, page_count;
    #define MAXIMUM_ORDER 63
    buddy_node *lists[MAXIMUM_ORDER], *storage;
} buddy_allocator;

// same as before -- a reasonable constant
#define BUDDY_MAXIMUM_ALLOCATORS 7
static uint64_t buddy_allocator_count = 0;
static buddy_allocator buddy_allocators[BUDDY_MAXIMUM_ALLOCATORS];

static inline void build_buddy(buddy_allocator *p) {
    p->page_count = p->page_count * PAGE_SIZE / (sizeof(buddy_node) + PAGE_SIZE);
    p->storage = (buddy_node *) p->begin;
    for (int64_t order = MAXIMUM_ORDER, offset = 0; order > -1; --order) {
        if (p->page_count & (1ULL << order)) {
            make_node(&p->storage[offset], order, 0, NULL, NULL);
            p->lists[order] = &p->storage[offset];
            offset += 1ULL << order;
        } else {
            p->lists[order] = NULL;
        }
    }
    p->begin = align_up(p->begin + p->page_count * sizeof(buddy_node));
}

/* iterate the memory map and build a buddy allocator on every free segment
   (those with type = 1); segments are alinged to PAGE_SIZE multiples */
void setup_buddy_allocators() {
    uint64_t total_pages = 0;
    for (uint64_t i = 0; i < mmap_actual_size; ++i) {
        mmap_entry_t *entry = &mmap[i];
        if (entry->type != 1) {
            continue;
        }
        buddy_allocator *new_buddy = &buddy_allocators[buddy_allocator_count++];
        new_buddy->begin = virt_addr(align_up(entry->base_addr + !entry->base_addr));
        ptr buddy_end = virt_addr(align_down(entry->base_addr + entry->length));
        if (new_buddy->begin >= buddy_end) {
            --buddy_allocator_count;
            continue;
        }
        new_buddy->page_count = (buddy_end - new_buddy->begin) / PAGE_SIZE;
        build_buddy(new_buddy);
        printf("%lld ", new_buddy->page_count);
        total_pages += new_buddy->page_count;
    }
    printf("sums up to %lld pages available for use\n", total_pages);
}

static ptr _allocate_buddy(buddy_allocator *p, uint64_t order, uint64_t len) {
    if (order > MAXIMUM_ORDER) {
        return 0;
    }
    uint64_t cur_len = (1ULL << order);
    if (cur_len < len || p->lists[order] == NULL) {
        return _allocate_buddy(p, order + 1, len);
    }
    uint64_t cur = p->lists[order] - p->storage;
    buddy_node_erase_head(&p->lists[order]);
    if (cur_len >= 2 * len) {
        uint64_t cur_buddy = cur ^ (1 << (order - 1));
        make_node(&p->storage[cur_buddy], order - 1, 0, NULL, NULL);
        buddy_node_insert_head(&p->storage[cur_buddy], &p->lists[order - 1]);
        make_node(&p->storage[cur], order - 1, 0, NULL, NULL);
        buddy_node_insert_head(&p->storage[cur], &p->lists[order - 1]);
        return _allocate_buddy(p, order - 1, len);
    }
    make_node(&p->storage[cur], order, true, NULL, NULL);
    return p->begin + PAGE_SIZE * cur;
}

/* go through all allocators and try to allocate needed amount of pages;
   returns the address of the beginning of the page segment,
   0 if fails */
ptr allocate_buddy(uint64_t len) {
    for (uint64_t i = 0; i < buddy_allocator_count; ++i) {
        buddy_allocator *current_allocator = &buddy_allocators[i];
        ptr res = _allocate_buddy(current_allocator, 0, len);
        if (res) {
            return res;
        }
    }
    return 0;
}

static void _free_buddy(buddy_allocator *p, uint64_t cur) {
    if (!p->storage[cur].used) {
        printf("Address is already free!\n");
        return;
    }
    uint64_t order = p->storage[cur].order;
    uint64_t cur_buddy = cur ^ (1 << order);
    if (!p->storage[cur_buddy].used && p->storage[cur_buddy].order == order &&
            cur_buddy < p->page_count) {
        if (&p->storage[cur_buddy] == p->lists[order]) {
            buddy_node_erase_head(&p->lists[order]);
        } else {
            buddy_node_erase_node(&p->storage[cur_buddy]);
        }
        ++p->storage[cur].order;
        if (cur_buddy < cur) {
            p->storage[cur].used = false;
            p->storage[cur_buddy].used = true;
            cur = cur_buddy;
        }
        _free_buddy(p, cur);
        return;
    }
    p->storage[cur].used = false;
    buddy_node_insert_head(&p->storage[cur], &p->lists[order]);
    p->lists[order] = &p->storage[cur];
}

/* go through all allocators and try to free the previously allocated memory
   starting at given address */
void free_buddy(ptr addr) {
    if (addr % PAGE_SIZE) {
        printf("Address is not aligned!\n");
        return;
    }
    for (uint64_t i = 0; i < buddy_allocator_count; ++i) {
        buddy_allocator *cur = &buddy_allocators[i];
        if (cur->begin <= addr &&
            addr < cur->begin + 
                   cur->page_count * PAGE_SIZE) {
            _free_buddy(cur, (addr - cur->begin) / PAGE_SIZE);
            return;
        }
    }
    printf("Specified address is not allocated!\n");
}