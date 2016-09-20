#include <stdint.h>
#include <video.h>

#ifndef __IOPORT_H__
#define __IOPORT_H__


static inline void out8(unsigned short port, uint8_t data)
{ __asm__ volatile("outb %0, %1" : : "a"(data), "d"(port)); }

static inline uint8_t in8(unsigned short port)
{
	uint8_t value;

	__asm__ volatile("inb %1, %0" : "=a"(value) : "d"(port));
	return value;
}

static inline void out16(unsigned short port, uint16_t data)
{ __asm__ volatile("outw %0, %1" : : "a"(data), "d"(port)); }

static inline uint16_t in16(unsigned short port)
{
	uint16_t value;

	__asm__ volatile("inw %1, %0" : "=a"(value) : "d"(port));
	return value;
}

static inline void out32(unsigned short port, uint32_t data)
{ __asm__ volatile("outl %0, %1" : : "a"(data), "d"(port)); }

static inline uint32_t in32(unsigned short port)
{
	uint32_t value;

	__asm__ volatile("inl %1, %0" : "=a"(value) : "d"(port));
	return value;
}

#define BDA 0x0400                    // BIOS Data Area
#define COM1 *((uint16_t*) (BDA + 0)) // Get address of COM1 serial port

// Initialise COM1 serial port
void init_serial() {
   out8(COM1 + 1, 0x00);    // Disable all interrupts
   out8(COM1 + 3, 0x80);    // Enable DLAB (set baud rate divisor)
   out8(COM1 + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
   out8(COM1 + 1, 0x00);    //                  (hi byte)
   out8(COM1 + 3, 0x03);    // 8 bits, no parity, one stop bit
   out8(COM1 + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
   out8(COM1 + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

// Receiving data
int serial_received() {
   return in8(COM1 + 5) & 1;
}
 
char read_serial() {
   while (serial_received() == 0);
   return in8(COM1);
}

// Sending data
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

void write_serial_number(uint x, uint base) {
    char buf[100];
    uint len = 0;
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

#endif /* __IOPORT_H__ */

