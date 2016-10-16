#ifndef __BUDDY_H__
#define __BUDDY_H__

void setup_buddy_allocators();
uint64_t allocate_buddy(uint64_t);
uint64_t free_buddy(uint64_t);

#endif /*__BUDDY_H__*/
