#ifndef __IOPORT_H__
#define __IOPORT_H__

// direct interaction with desired port
void out8(unsigned short, uint8_t);
uint8_t in8(unsigned short);
void out16(unsigned short, uint16_t);
uint16_t in16(unsigned short);
void out32(unsigned short, uint32_t);
uint32_t in32(unsigned short);

// serial port
#define BDA 0x0400                    // BIOS Data Area
#define COM1 *((uint16_t*) (BDA + 0)) // Get address of COM1 serial port

// Initialise COM1 serial port
void init_serial();

// Receiving data
char read_serial();

// Sending data
int vprintf(const char[], va_list);
int printf(const char[], ...);
int vsnprintf(char *, size_t, const char[], va_list);
int snprintf(char *, size_t, const char[], ...);

#endif /* __IOPORT_H__ */