.text

.code64
.global handler_labels
.global interrupt_handler0
.global interrupt_handler1

.macro before_handler
    pushq %rax
    pushq %rbx
    pushq %rcx
    pushq %rdx
    pushq %rbp
    pushq %rdi
    pushq %rsi
    pushq %r9
    pushq %r10
    pushq %r11
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15
    cld
.endm

.macro after_handler
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %r11
    popq %r10
    popq %r9
    popq %rsi
    popq %rdi
    popq %rbp
    popq %rdx
    popq %rcx
    popq %rbx
    popq %rax
.endm

wrapper0:
    before_handler
    call interrupt_handler0
    after_handler
    iretq

wrapper1:
    before_handler
    call interrupt_handler1
    after_handler
    iretq

handler_labels:
    .quad wrapper0
    .quad wrapper1
