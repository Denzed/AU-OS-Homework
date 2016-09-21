#include <stdarg.h>
#include <stdint.h>
#include "io.h"
#include "handlers.h"
#include "pic.h"

void enable_interrupts() {
    __asm__ volatile ("sti");
}

void disable_interrupts() {
    __asm__ volatile ("cli");
}

void interrupt_handler(uint64_t n, uint64_t errcode) {
    int16_t irq = n - master_PIC_first_input_IDT_position;
    if (irq < 0) {
        if (n < 32) { // is reserved
            printf("Reserved interrupt ");
        } else {
            printf("Software interrupt ");
        }
        printf("%d happened with errcode %d\n", n, errcode);
    } else {
        if (irq == 0) {
            static int steps = 0;
            if (!steps--) {
                steps += PIT_interrupt_div;
                printf("Tick\n");
            }
        } else {
            printf("Hardware interrupt %d happened on irq %d with errcode %d\n",
                   n, irq, errcode);
        }
        send_EOI_PIC(irq);
    }
}