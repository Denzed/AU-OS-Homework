#include "general.h"
#include "memory.h"
#include "debug.h"
#include "io.h"

static inline uint64_t *get_prev(uint64_t *addr) {
    return (uint64_t *) *addr;
}

static inline void _backtrace(uint64_t *rbp) {
    if (GLOBAL_STACK_BOTTOM <= rbp) {
        uint64_t *prev_rbp = get_prev(rbp);
        _backtrace(prev_rbp);
        printf("%s%x", 
               (GLOBAL_STACK_BOTTOM <= prev_rbp ? " -> " : ""),
               rbp[1]);
        // [1] is because return address is stored in 0x8(rbp)
    }
}

void backtrace() {
    register uint64_t *rbp __asm__ ("rbp");
    printf("Call stack: ");
    _backtrace(rbp);
    printf("\n");
}
