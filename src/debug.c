#include "general.h"
#include "debug.h"
#include "io.h"

static inline void _backtrace(uint64_t *rbp) {
    if (rbp != 0) {
        printf("%x -> ", *rbp);
    }
    _backtrace((uint64_t*) *rbp);
}

void backtrace() {
    register uint64_t *rbp __asm__ ("rbp");
    _backtrace(rbp);
    printf("\n");
}
