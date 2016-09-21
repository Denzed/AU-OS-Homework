#include <stdint.h>
#include "io.h"

void init_serial() {
    out8(COM1 + 1, 0x00);    // Disable all interrupts
    out8(COM1 + 3, 0x80);    // Enable DLAB (set baud rate divisor)
    out8(COM1 + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
    out8(COM1 + 1, 0x00);    //                  (hi byte)
    out8(COM1 + 3, 0x03);    // 8 bits, no parity, one stop bit
    out8(COM1 + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
    out8(COM1 + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

int serial_received() {
    return in8(COM1 + 5) & 1;
}
 
char read_serial() {
    while (serial_received() == 0);
    return in8(COM1);
}

int is_transmit_empty() {
    return in8(COM1 + 5) & 0x20;
}
 
void write_serial_char(char a) {
    while (is_transmit_empty() == 0); 
    out8(COM1, a);
}

void write_serial_string(char s[]) {
    for (int i = 0; s[i]; ++i) {
        write_serial_char(s[i]);
    }
}

void write_serial_number(uint32_t x, uint32_t base) {
    char buf[100];
    uint32_t len = 0;
    for (; x; ) {
        buf[len] = (x % base > 9 ? 'a' + x % base - 10 : '0' + x % base);
        ++len;
        x /= base;
    }
    if (len == 0) {
        buf[0] = '0';
        len = 1;
    }
    for (int i = len - 1; i > -1; --i) {
        write_serial_char(buf[i]);
    }
}

void printf(char format[], ...) {
    write_serial_string(format); // dummy
}