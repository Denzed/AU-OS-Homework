#ifndef __HANDLERS_H__
#define __HANDLERS_H__

#define enable_interrupts() __asm__ volatile ("sti")
#define gen_interrupt(arg) __asm__ volatile ("int %0\n" : : "N"((arg)))
#define disable_interrupts() __asm__ volatile ("cli")

#define PIT_interrupt_div 100

void interrupt_handler(uint64_t);

#endif /* __HANDLERS_H__ */