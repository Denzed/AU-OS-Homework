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
            write_serial_string("Reserved interrupt ");
        } else {
            write_serial_string("Software interrupt ");
        }
        write_serial_number(n, 10);
        write_serial_string(" happened with errcode ");
        write_serial_number(errcode, 10);
        write_serial_string("\n");
    } else {
        if (irq == 0) {
            static int steps = 0;
            if (!steps--) {
                steps += PIT_interrupt_div;
                write_serial_string("Tick\n");
            }
        } else {
            write_serial_string("Hardware interrupt ");
            write_serial_number(n, 10);
            write_serial_string(" happened on irq ");
            write_serial_number(irq, 10);
            write_serial_string(" with errcode ");
            write_serial_number(errcode, 10);
            write_serial_string("\n");
        }
        send_EOI_PIC(irq);
    }
}