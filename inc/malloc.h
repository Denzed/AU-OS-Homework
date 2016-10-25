#ifndef __MALLOC_H__
#define __MALLOC_H__

void setup_malloc();
ptr malloc(uint64_t);
void free(ptr);

#endif /*__MALLOC_H__*/
