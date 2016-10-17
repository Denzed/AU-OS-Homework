#include "general.h"
#include "buddy.h"
#include "io.h"
#include "memory.h"

// buddy allocator here will be implemented as a binary tree
struct buddy_node;
typedef struct buddy_node buddy_node_t;
typedef buddy_node_t * buddy_node_p;

struct buddy_node {
    uint64_t order;
    bool used;
    buddy_node_p prev, next;
} __attribute__((packed));

static inline void make_node(buddy_node_p p,
                             uint64_t order,
                             bool used,
                             buddy_node_p prev, 
                             buddy_node_p next) {
    p->used = used;
    p->order = order;
    p->prev = prev;
    p->next = next;
}

static inline void insert_node(buddy_node_p node, 
                               buddy_node_p head) {
    if (head) {
        node->prev = head->prev;
        head->prev = node;
        node->next = head;
    }
}

static inline void erase_node(buddy_node_p node) {
    if (node->prev) {
        node->prev->next = node->next;
    }
    if (node->next) {
        node->next->prev = node->prev;
    }
}

static inline void set_state(buddy_node_p p, bool used) {
    p->used = used;
}

/* as memory map can have several free segments we have to be able to create 
   more than one allocator */
struct buddy_allocator {
    uint64_t begin, page_count, maximum_order;
    buddy_node_p *lists, storage;
} __attribute__((packed));

typedef struct buddy_allocator buddy_allocator_t;
typedef buddy_allocator_t * buddy_allocator_p;

// same as before -- a reasonable constant
#define BUDDY_MAXIMUM_ALLOCATORS 7
static uint64_t buddy_allocator_count = 0;
static buddy_allocator_t buddy_allocators[BUDDY_MAXIMUM_ALLOCATORS];

static inline uint64_t get_page_from_base(buddy_allocator_p p,
                                          uint64_t base) {
    return (base - p->begin) / PAGE_SIZE + 1;
}

static inline uint64_t get_base_from_page(buddy_allocator_p p,
                                          uint64_t page) {
    return p->begin + PAGE_SIZE * (page - 1);
}

static inline bool is_enough(buddy_allocator_p p) {
    uint64_t current_end = p->maximum_order * sizeof(buddy_node_p) + 
        (1 << p->maximum_order) * (sizeof(buddy_node_t) + PAGE_SIZE);
    return current_end + (current_end % PAGE_SIZE ? 
                          PAGE_SIZE - current_end % PAGE_SIZE:
                          0) <= p->page_count * PAGE_SIZE;
}

void build_buddy(buddy_allocator_p p) {
    for (p->maximum_order = 0; is_enough(p); ++p->maximum_order);
    p->maximum_order -= (p->maximum_order > 0);
    p->page_count = 1 << p->maximum_order;
    p->storage = (buddy_node_p) p->begin + p->maximum_order + 1;
    make_node(p->storage, p->maximum_order, 0, NULL, NULL);
    p->lists = (buddy_node_p *) p->begin;
    for (uint64_t order = 0; order < p->maximum_order; ++order) {
        p->lists[order] = NULL;
    }
    p->lists[p->maximum_order] = p->storage;
    p->begin += p->maximum_order * sizeof(buddy_node_p) + 
          (1 << p->maximum_order) * sizeof(buddy_node_t);
    // align
    if (p->begin % PAGE_SIZE) {
        p->begin += PAGE_SIZE - p->begin % PAGE_SIZE;
    }
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
        buddy_allocator_t *new_buddy = &buddy_allocators[buddy_allocator_count++];
        new_buddy->begin = entry->base_addr + (entry->base_addr == 0);
        if (new_buddy->begin % PAGE_SIZE) {
            new_buddy->begin += PAGE_SIZE - new_buddy->begin % PAGE_SIZE;
        }
        uint64_t buddy_end = entry->base_addr + entry->length;
        // aligned
        buddy_end -= buddy_end % PAGE_SIZE;
        if (new_buddy->begin >= buddy_end) {
            // printf("Sas: %lld > %lld\n", new_buddy->begin, 
            //                              entry->base_addr + entry->length);
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

static inline void print_list(buddy_node_p cur) {
    printf("%#x%s", cur, (cur == NULL ? "\n" : " -> "));
    if (cur) {
        print_list(cur->next);
    }
}

static inline uint64_t _allocate_buddy(buddy_allocator_p p, uint64_t order, uint64_t len) {
    if (order > p->maximum_order) {
        return 0;
    }
    uint64_t cur_len = (1ULL << order);
    if (cur_len < len || p->lists[order] == NULL) {
        /*if (cur_len < len) {
            printf("%lld is not enough, trying order %lld\n", cur_len, order + 1);
        } else {
            printf("List is empty at order %lld, trying order %lld\n", order, order + 1);
        }*/
        return _allocate_buddy(p, order + 1, len);
    }
    uint64_t cur = p->lists[order] - p->storage;
    // printf("%lld\n", (p->lists[order] - p->storage));
    // print_list(p->lists[order]);
    p->lists[order] = p->storage[cur].next;
    // print_list(p->lists[order]);
    if (cur_len >= 2 * len) {
        uint64_t cur_buddy = cur ^ (1 << (order - 1));
        // printf("Splitting %lld into %lld, %lld at order %lld\n", cur, cur, cur_buddy, order);
        make_node(&p->storage[cur], order - 1, 0, NULL, &p->storage[cur_buddy]);
        make_node(&p->storage[cur_buddy], order - 1, 0, &p->storage[cur], p->lists[order - 1]);
        p->lists[order - 1] = &p->storage[cur];

        return _allocate_buddy(p, order - 1, len);
    }
    // printf("%lld at order %lld is okay\n", cur, order);
    make_node(&p->storage[cur], order, true, NULL, NULL);
    return cur + 1;
}

/* go through all allocators and try to allocate needed amount of pages;
   returns the address of the beginning of the page segment,
   0 if fails */
uint64_t allocate_buddy(uint64_t len) {
    for (uint64_t i = 0; i < buddy_allocator_count; ++i) {
        buddy_allocator_t *current_allocator = &buddy_allocators[i];
        // printf("Trying %d\n", i);
        uint64_t res = _allocate_buddy(current_allocator, 0, len);
        if (res) {
            return get_base_from_page(current_allocator, res);
        }
    }
    return 0;
}

uint64_t _free_buddy(buddy_allocator_p p, uint64_t cur) {
    if (!p->storage[--cur].used) {
        return 1;
    }
    uint64_t order = p->storage[cur].order;
    // printf("Freeing %lld of order %lld\n", cur, order);
    uint64_t cur_buddy = cur ^ (1 << order);
    // printf("Its buddy %lld has state %lld and order %lld\n", cur_buddy, p->storage[cur_buddy].used, p->storage[cur_buddy].order);
    if (!p->storage[cur_buddy].used && p->storage[cur_buddy].order == order) {
        // print_list(p->lists[order]);
        // printf("Merging with a buddy\nNow ");
        erase_node(&p->storage[cur_buddy]);
        if (&p->storage[cur_buddy] == p->lists[order]) {
            p->lists[order] = p->lists[order]->next;
        }
        ++p->storage[cur].order;
        if (cur_buddy < cur) {
            set_state(&p->storage[cur], false);
            set_state(&p->storage[cur_buddy], true);
            cur = cur_buddy;
        }
        // print_list(p->lists[order]);
        return _free_buddy(p, cur + 1);
    }
    // print_list(p->lists[order]);
    set_state(&p->storage[cur], false);
    insert_node(&p->storage[cur], p->lists[order]);
    p->lists[order] = &p->storage[cur];
    // print_list(p->lists[order]);
    return 0;
}

/* go through all allocators and try to free the previously allocated memory
   starting at given address;
   returns 0 if succeeds, 1 if fails */
uint64_t free_buddy(uint64_t base) {
    if (base % PAGE_SIZE) {
        return 1;
    }
    for (uint64_t i = 0; i < buddy_allocator_count; ++i) {
        buddy_allocator_p current_allocator = &buddy_allocators[i];
        if (current_allocator->begin <= base &&
            base < current_allocator->begin + 
                   current_allocator->page_count * PAGE_SIZE) {
            uint64_t page = get_page_from_base(current_allocator, base);
            uint64_t res = _free_buddy(current_allocator, page);
            return res;
        }
    }
    return 1;
}