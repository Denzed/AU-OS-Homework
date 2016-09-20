#include <ioport.h>
#include <pic.h>
#include <video.h>

#ifndef __HANDLERS_H__
#define __HANDLERS_H__

#define enable_interrupts() __asm__ volatile ("sti")
#define gen_interrupt(arg) __asm__ volatile ("int %0\n" : : "N"((arg)))
#define disable_interrupts() __asm__ volatile ("cli")

#define ENTRIES 2
#define PIT_interrupt_div 100

void interrupt_handler0(void) {
    static volatile int steps = 0;
    if (!steps--) {
        steps += PIT_interrupt_div;
        // printword("00 happened\n");
        write_serial_string("00 happened\n");
    }
    send_EOI_PIC(0);
}

void interrupt_handler1(void) {
    printword("01 happened\n");
    send_EOI_PIC(1);
}

#endif /* __HANDLERS_H__ */