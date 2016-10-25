#include "general.h"
#include "algo.h"
#include "block_allocator.h"
#include "buddy.h"
#include "io.h"
#include "memory.h"

struct _free_block {
    block_allocator *parent;
    _free_block *prev, *next;
};

void make_block(_free_block *block, 
                block_allocator *alloc, 
                _free_block *prev, 
                _free_block *next) {
    block->parent = alloc;
    block->prev = prev;
    block->next = next;
}

define_list_operations(_free_block)

block_allocator *create_block_allocator(uint16_t size) {
    if (size >= PAGE_SIZE || !size) {
        printf("Wrong size of block requested: %d!\n", size);
    }
    uint64_t pages = (8 * size < PAGE_SIZE ? 1 : 10);
    ptr addr = allocate_buddy(pages);
    block_allocator *alloc = (block_allocator *) addr;
    alloc->unmapped_begin = addr + sizeof(block_allocator);
    alloc->unmapped_end = addr + PAGE_SIZE * pages;
    alloc->block_size = size;
    alloc->free_begin = NULL;
    return alloc;
}

ptr allocate_block(block_allocator *alloc) {
    if (alloc->free_begin == NULL) {
        if (alloc->unmapped_begin + sizeof(_free_block) + alloc->block_size >
                alloc->unmapped_end) {
            uint64_t pages = (8 * alloc->block_size < PAGE_SIZE ? 1 : 10);
            ptr addr = allocate_buddy(pages);
            alloc->unmapped_begin = addr;
            alloc->unmapped_end = addr + PAGE_SIZE * pages;
        }
        _free_block *new_block = (_free_block *) alloc->unmapped_begin;
        make_block(new_block, alloc, NULL, NULL);
        _free_block_insert_head(new_block, &alloc->free_begin);
        alloc->unmapped_begin += sizeof(_free_block) + alloc->block_size;
    }
    ptr res = ((ptr) alloc->free_begin) + sizeof(_free_block);
    _free_block_erase_head(&alloc->free_begin);
    return res;
}

bool is_owned(block_allocator *alloc, ptr addr) {
    return ((_free_block *) (addr - sizeof(_free_block)))->parent == alloc;
}

void free_block(block_allocator *alloc, ptr addr) {
    if (!(alloc && addr)) {
        printf("Incorrect address or allocator pointer!\n");
        return;
    } else if (!is_owned(alloc, addr)) {
        printf("The address is not owned by the specified allocator!\n");
        return;
    }
    _free_block *new_block = (_free_block *) (addr - sizeof(_free_block));
    make_block(new_block, alloc, NULL, NULL);
    _free_block_insert_head(new_block, &alloc->free_begin);
}