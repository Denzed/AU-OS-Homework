#ifndef __BUDDY_H__
#define __BUDDY_H__

void setup_buddy_allocators();
ptr allocate_buddy(uint64_t);
void free_buddy(ptr);

#endif /*__BUDDY_H__*/
