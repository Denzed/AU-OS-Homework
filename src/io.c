#include "general.h"
#include "io.h"

void out8(unsigned short port, uint8_t data) {
    __asm__ volatile ("outb %0, %1" : : "a"(data), "d"(port));
}

uint8_t in8(unsigned short port) {
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "d"(port));
    return value;
}

void out16(unsigned short port, uint16_t data) {
    __asm__ volatile ("outw %0, %1" : : "a"(data), "d"(port));
}

uint16_t in16(unsigned short port) {
    uint16_t value;
    __asm__ volatile ("inw %1, %0" : "=a"(value) : "d"(port));
    return value;
}

void out32(unsigned short port, uint32_t data) {
    __asm__ volatile ("outl %0, %1" : : "a"(data), "d"(port));
}

uint32_t in32(unsigned short port) {
    uint32_t value;
    __asm__ volatile ("inl %1, %0" : "=a"(value) : "d"(port));
    return value;
}

void init_serial() {
    out8(COM1 + 1, 0x00);    // Disable all interrupts
    out8(COM1 + 3, 0x80);    // Enable DLAB (set baud rate divisor)
    out8(COM1 + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
    out8(COM1 + 1, 0x00);    //                  (hi byte)
    out8(COM1 + 3, 0x03);    // 8 bits, no parity, one stop bit
    out8(COM1 + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
    out8(COM1 + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

static inline int serial_received() {
    return in8(COM1 + 5) & 1;
}
 
char read_serial() {
    while (serial_received() == 0);
    return in8(COM1);
}

static inline int is_transmit_empty() {
    return in8(COM1 + 5) & 0x20;
}
 
static inline void write_serial_char(char c) {
    while (is_transmit_empty() == 0); 
    out8(COM1, c);
}

static char *s;
static int pos, limit;

static inline void write_sn_char(char c) {
    if (pos < limit) {
        s[pos++] = c;
    }
}

static inline void write_string(void (*write_char)(char), char s[]) {
    for (int i = 0; s[i]; ++i) {
        write_char(s[i]);
    }
}

static inline void write_number(void (*write_char)(char), uint64_t x, uint64_t base) {
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
        write_char(buf[i]);
    }
}

static inline void write_signed_number(void (*write_char)(char), int64_t x, uint64_t base) {
    if (x < 0) {
        write_char('-');
        x *= -1;
    }
    write_number(write_char, (uint64_t) x, base);
}

// state decipher:
// -1 -- error
// 0 -- initial
// 1 -- started scanning specifier
// 2 -- read "l"
// 3 -- read "ll"
// 4 -- read "h"
// 5 -- read "hh"
static inline int _vprintf(void (*write_char)(char), const char format[], va_list args) {
    int state = 0;
    int64_t n;
    uint64_t un;
    for (int i = 0; format[i] && state != -1; ++i) {
        if (state == 0) {
            if (format[i] == '%') {
                state = 1;
            } else {
                write_char(format[i]);
            }
        } else if (format[i] == 'l') {
            if (state == 1) {
                state = 2;
            } else if (state == 2) {
                state = 3;
            } else {
                state = -1;
            }
        } else if (format[i] == 'h') {
            if (state == 1) {
                state = 4;
            } else if (state == 4) {
                state = 5;
            } else {
                state = -1;
            }
        } else if (format[i] == 'd' || format[i] == 'i') {
            if (state == 1) {
                n = va_arg(args, int);
            } else if (state == 2) {
                n = va_arg(args, long int);
            } else if (state == 3) {
                n = va_arg(args, long long int);
            } else if (state == 4 || state == 5) {
                n = va_arg(args, int);
            } else {
                state = -1;
                continue;
            }
            write_signed_number(write_char, n, 10);
            state = 0;
        } else if (format[i] == 'u') {
            if (state == 1 || state == 4 || state == 5) {
                un = va_arg(args, unsigned int);
            } else if (state == 2) {
                un = va_arg(args, unsigned long int);
            } else if (state == 3) {
                un = va_arg(args, unsigned long long int);
            } else {
                state = -1;
                continue;
            }
            write_number(write_char, un, 10);
            state = 0;
        } else if (format[i] == 'o' || format[i] == 'x') {
            write_number(write_char, va_arg(args, uint64_t), 
                         (format[i] == 'x' ? 16 : 8));
            state = 0;
        } else if (format[i] == 'c') {
            write_char(va_arg(args, unsigned int));
            state = 0;
        } else if (format[i] == 's') {
            write_string(write_char, va_arg(args, char*));
            state = 0;
        } else {
            state = -1;
            continue;
        }
    }
    return (state ? -1 : 0);
}

int vprintf(const char format[], va_list args) {
    return _vprintf(&write_serial_char, format, args);
}

int printf(const char format[], ...) {
    va_list args;
    va_start(args, format);
    int res = vprintf(format, args);
    va_end(args);
    return res;
}

int vsnprintf(char *str, size_t n, const char format[], va_list args) {
    s = str;
    limit = n;
    pos = 0;
    return _vprintf(&write_sn_char, format, args);
}

int snprintf(char *str, size_t n, const char format[], ...) {
    va_list args;
    va_start(args, format);
    int res = vsnprintf(str, n, format, args);
    va_end(args);
    return res;
}