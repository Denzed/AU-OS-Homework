#include "general.h"
#include "debug.h"
#include "io.h"

static inline uint64_t *get_prev(uint64_t *addr) {
    return (uint64_t *) *addr;
}

static inline void _backtrace(uint64_t *rbp) {
    if (rbp) {
        uint64_t *prev_rbp = get_prev(rbp);
        _backtrace(prev_rbp);
        printf((prev_rbp ? " -> %x" : "%x"), rbp[1]); 
        // [1] is because return address is stored in 0x8(rbp)
    }
}

void backtrace() {
    register uint64_t *rbp __asm__ ("rbp");
    printf("Call stack: ");
    _backtrace(rbp);
    printf("\n");
}
