#include "general.h"
#include "buddy.h"
#include "io.h"
#include "memory.h"

// buddy allocator here will be implemented as a binary tree
struct buddy_node;
typedef struct buddy_node buddy_node_t;
typedef buddy_node_t * buddy_node_p;

struct buddy_node {
    uint8_t state; /* 0 -- does not exist, 
                      1 -- free,
                      2 -- split into buddies,
                      3 -- the node itself is used */
    buddy_node_p left, right;
} __attribute__((packed));

// just some reasonable constant
#define BUDDY_MAXIMUM_NODES (1 << 20)
static uint64_t buddy_node_count = 0;
static buddy_node_t buddy_nodes[BUDDY_MAXIMUM_NODES];

static inline buddy_node_p make_buddy_node(uint8_t _state, 
                                      buddy_node_p _left, 
                                      buddy_node_p _right) {
    buddy_nodes[buddy_node_count].state = _state;
    buddy_nodes[buddy_node_count].left = _left;
    buddy_nodes[buddy_node_count].right = _right;
    return &buddy_nodes[buddy_node_count++];
}

static inline uint8_t get_state(buddy_node_p p) {
    return (p ? p->state : 0);
}

static inline void change_state(buddy_node_p p, uint8_t state) {
    if (p) {
        p->state = state;
    }
}

// build a full buddy allocator tree 
static buddy_node_p _build_buddy(uint64_t l, uint64_t r) {
    // printf("%lld %lld\n", l, r);
    if (l >= r) {
        return NULL;
    }
    buddy_node_p left = NULL, right = NULL;
    if (l + 1 < r) {
        left = _build_buddy(l, (l + r) >> 1);
        right = _build_buddy((l + r) >> 1, r);
    }
    return make_buddy_node(0, left, right);
}

static buddy_node_p build_buddy(uint64_t l, uint64_t r) {
    buddy_node_p res = _build_buddy(l, r);
    change_state(res, 1);
    return res;    
}

// returns is 0 if we don't succeed, page number otherwise (starting from 1)
static uint64_t _allocate_buddy(buddy_node_p p, 
                                uint64_t l, 
                                uint64_t r, 
                                uint64_t len) {
    /*printf("%lld, %lld: %lld, %lld, %lld\n", l, r, get_state(p), 
                                       get_state(p->left), 
                                       get_state(p->right));*/
    if (l + len > r || get_state(p) == 0 || get_state(p) == 3) {
        return 0;
    }
    if (get_state(p) == 1) {
        if (p->right) {
            change_state(p, 2);
            change_state(p->left, 1);
            change_state(p->right, 1);
            uint64_t res = _allocate_buddy(p, l, r, len);
            if (res) {
                return res;
            }
        }
        change_state(p, 3);
        change_state(p->left, 0);
        change_state(p->right, 0);
        return l;
    } else if (get_state(p) == 2) {
        uint64_t res = 0;
        if ((res = _allocate_buddy(p->left, l, (l + r) >> 1, len)) ||
            (res = _allocate_buddy(p->right, (l + r) >> 1, r, len))) {
            return res;
        }
    }
    return 0;
}

// returns 0 on success, 1 otherwise
static uint64_t _free_buddy(buddy_node_p p, 
                                uint64_t l, 
                                uint64_t r, 
                                uint64_t i) {
    if (l > i || !p) {
        return 1;
    }
    if (l == i && get_state(p) == 3) {
        change_state(p, 1);
        return 0;
    }
    if (get_state(p) == 2 &&
            (!_free_buddy(p->left, l, (l + r) >> 1, i) ||
             !_free_buddy(p->right, (l + r) >> 1, r, i))) {
        if (get_state(p->left) == 1 && get_state(p->right) == 1) {
            change_state(p, 1);
            change_state(p->left, 0);
            change_state(p->right, 0);
        }
        return 0;
    }
    return 1;
}

/* as memory map can have several free segments we have to be able to create 
   more than one allocator */
struct buddy_allocator {
    uint64_t begin, page_count;
    buddy_node_p root;
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
        if (new_buddy->begin >= entry->base_addr + entry->length) {
            /*printf("Sas: %lld > %lld\n", new_buddy->begin, 
                                         entry->base_addr + entry->length);*/
            --buddy_allocator_count;
            continue;
        }
        new_buddy->page_count = (entry->base_addr + 
                                entry->length - 
                                new_buddy->begin) / PAGE_SIZE;
        total_pages += new_buddy->page_count;
        new_buddy->root = build_buddy(1, new_buddy->page_count + 1);
        // change_state(new_buddy->root, 1);
    }
    printf("Total pages available for use: %lld\n", total_pages);
}

/* go through all allocators and try to allocate needed amount of pages;
   returns the address of the beginning of the page segment,
   0 if fails */
uint64_t allocate_buddy(uint64_t len) {
    for (uint64_t i = 0; i < buddy_allocator_count; ++i) {
        buddy_allocator_t *current_allocator = &buddy_allocators[i];
        // printf("Trying %d\n", i);
        uint64_t res = _allocate_buddy(current_allocator->root, 
                                       1, 
                                       current_allocator->page_count + 1, 
                                       len);
        if (res) {
            return get_base_from_page(current_allocator, res);
        }
    }
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
            uint64_t res = _free_buddy(current_allocator->root, 
                                       1, 
                                       current_allocator->page_count + 1, 
                                       page);
            return res;
        }
    }
    return 1;
}