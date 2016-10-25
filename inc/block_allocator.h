#ifndef __BLOCK_H__
#define __BLOCK_H__

typedef struct _free_block _free_block;
// for the sake of simplicity we won't implement allocated page freeing
typedef struct block_allocator {
    ptr unmapped_begin, unmapped_end;
    uint16_t block_size;
    _free_block *free_begin;
} block_allocator;

block_allocator *create_block_allocator(uint16_t);
ptr allocate_block(block_allocator *);
void free_block(block_allocator *, ptr);

#endif /* __BLOCK_H__ */