#include "general.h"
#include "algo.h"
#include "block_allocator.h"
#include "buddy.h"
#include "io.h"
#include "memory.h"

/* we want to optimise our algorithm a little by adding custom allocation for
   relatively small segments */
#define MAX_ORDER 6
/* if you want to set MAX_ORDER to be more than 11 please note that PAGE_SIZE
   is of order 12; */
block_allocator *caches[MAX_ORDER + 1];

define_splay_tree(ptr)
splay_node(ptr) *used_addresses = NULL;

void setup_malloc() {
    for (uint64_t order = 0, size = 1; order <= MAX_ORDER; ++order, size <<= 1) {
        caches[order] = create_block_allocator(size);
    }
    splay_node_allocator(ptr) = create_block_allocator(sizeof(splay_node(ptr)));
}

ptr encode(ptr addr, uint16_t payload) {
    return ((((uint64_t) payload) << 48) & VIRTUAL_BASE) ^ addr;
}

ptr decode_addr(ptr encoded) {
    return VIRTUAL_BASE | encoded;
}

uint16_t decode_payload(ptr encoded) {
    return (decode_addr(encoded) ^ encoded) >> 48;
}

ptr malloc(uint64_t size) {
    ptr res = 0;
    uint16_t allocator_used = 0;
    for (uint64_t order = 0; order <= MAX_ORDER; ++order) {
        if (size <= (1ULL << order)) {
            allocator_used = order;
            res = allocate_block(caches[order]);
            break;
        }
    }
    if (!res) {
        allocator_used = MAX_ORDER + 1;
        res = allocate_buddy(align_up(size) / PAGE_SIZE);
    }
    if (res) {
        /* here we want to add some information to the pointer to know
           with which allocator it was allocated --- as bits 48-64 are omitted
           --- we will use them */
        splay_insert(ptr)(&used_addresses, encode(res, allocator_used));
    }
    return res;
}

void free(ptr addr) {
    if ((addr & VIRTUAL_BASE) != VIRTUAL_BASE) {
        printf("Invalid address!\n");
        return;
    }
    for (uint64_t order = 0; order <= MAX_ORDER; ++order) {
        uint64_t encoded = encode(addr, order);
        if (splay_erase(ptr)(&used_addresses, encoded)) {
            free_block(caches[order], addr);
            return;
        }
    }
    uint64_t encoded = encode(addr, MAX_ORDER + 1);
    if (splay_erase(ptr)(&used_addresses, encoded)) {
        free_buddy(addr);
        return;
    }
    printf("Specified address is not allocated!\n");
}