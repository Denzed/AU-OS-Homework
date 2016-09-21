#include <ioport.h>
#include <pic.h>
#include <video.h>
#include <stdint.h>

#ifndef __HANDLERS_H__
#define __HANDLERS_H__

#define enable_interrupts() __asm__ volatile ("sti")
#define gen_interrupt(arg) __asm__ volatile ("int %0\n" : : "N"((arg)))
#define disable_interrupts() __asm__ volatile ("cli")

#define PIT_interrupt_div 100

void interrupt_handler(uint64_t n) {
    int16_t irq, isr;
    for (isr = pic_get_isr(), irq = (isr ? 0 : -1); isr && !(isr & 1); isr >>= 1, ++irq);
    if (irq == -1) {
        write_serial_string("Interrupt ");
        write_serial_number(n, 10);
        write_serial_string(" happened\n");
    } else {
        if (irq == 0) {
            static volatile int steps = 0;
            if (!steps--) {
                steps += PIT_interrupt_div;
                write_serial_string("Tick\n");
            }
        } else {
            write_serial_string("Interrupt ");
            write_serial_number(n, 10);
            write_serial_string(" happened on irq ");
            write_serial_number(irq, 10);
            write_serial_string("\n");
        }
        send_EOI_PIC(irq);
    }
}

#endif /* __HANDLERS_H__ */