#include "general.h"
#include "buddy.h"
#include "block_allocator.h"
#include "io.h"
#include "memory.h"

/* we want to optimise our algorithm a little by adding custom allocation for
   relatively small segments */
#define MAX_ORDER 6
/* if you want to set MAX_ORDER to be more than 11 please note that PAGE_SIZE
   is of order 12 */
block_allocator *caches[MAX_ORDER + 1];

void setup_malloc() {
    for (uint64_t order = 0, size = 1; order <= MAX_ORDER; ++order, size <<= 1) {
        caches[order] = create_block_allocator(size);
    }
}

ptr malloc(uint64_t size) {
    for (uint64_t order = 0; order <= MAX_ORDER; ++order) {
        if (size <= (1ULL << order)) {
            return allocate_block(caches[order]);
        }
    }
    return allocate_buddy(align_up(size) / PAGE_SIZE);
}

void free(ptr addr) {
    for (uint64_t order = 0; order <= MAX_ORDER; ++order) {
        if (is_owned(caches[order], addr)) {
            free_block(caches[order], addr);
            return;
        }
    }
    if (is_buddy_allocated(addr)) {
        free_buddy(addr);
        return;
    }
    printf("Specified address is not allocated!\n");
}