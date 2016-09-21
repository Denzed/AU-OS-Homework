#ifndef __HANDLERS_H__
#define __HANDLERS_H__

#define PIT_interrupt_div 100

void enable_interrupts();
void disable_interrupts();
#define gen_interrupt(arg) __asm__ volatile ("int %0\n" : : "N"((arg)))
void interrupt_handler(uint64_t, uint64_t);

#endif /* __HANDLERS_H__ */