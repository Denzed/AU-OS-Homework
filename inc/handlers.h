#ifndef __HANDLERS_H__
#define __HANDLERS_H__

#define PIT_interrupt_div 100

struct stack_frame {
    uint64_t rax, rbx, rcx, rdx, rdi, rsi, rbp;
    uint64_t r8, r9, r10, r11, r12, r13, r14, r15;
    uint64_t n, errcode;
} __attribute__((packed));

void enable_interrupts();
void disable_interrupts();
#define gen_interrupt(arg) __asm__ volatile ("int %0\n" : : "N"((arg)))
void interrupt_handler(struct stack_frame*);

#endif /* __HANDLERS_H__ */