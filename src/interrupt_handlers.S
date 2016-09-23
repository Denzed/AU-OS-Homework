.text
.code64
.altmacro

.macro AKA_PUSHA
    pushq %r15
    pushq %r14
    pushq %r13
    pushq %r12
    pushq %r11
    pushq %r10
    pushq %r9
    pushq %r8
    pushq %rbp
    pushq %rsi
    pushq %rdi
    pushq %rdx
    pushq %rcx
    pushq %rbx
    pushq %rax
.endm

.macro AKA_POPA
    popq %rax
    popq %rbx
    popq %rcx
    popq %rdx
    popq %rdi
    popq %rsi
    popq %rbp
    popq %r8
    popq %r9
    popq %r10
    popq %r11
    popq %r12
    popq %r13
    popq %r14
    popq %r15
.endm

.macro FOR l r func
    .if \r == (\l+1)
        func \l
    .else
        FOR l, %((\l+\r)/2), func
        FOR %((\l+\r)/2), r, func
    .endif
.endm

.macro DEF_WRAPPER n
    wrapper\n:
        pushq $0
        pushq $\n
        jmp inner_wrapper
.endm

.macro DEF_WRAPPER_ERROR n
    wrapper\n:
        # add $0x8, %rsp      // remove the errcode
        pushq $\n
        jmp inner_wrapper
.endm

.macro LIST_WRAPPER n
    .quad wrapper\n
.endm

.global handler_labels
.extern interrupt_handler

inner_wrapper:
    AKA_PUSHA
    movq %rsp, %rdi             // stack frame pointer to first argument
    # movq 0x78(%rsp), %rdi    // load n to first argument
    # movq 0x7c(%rsp), %rsi    // load errcode to second argument
    cld
    call interrupt_handler
    AKA_POPA                  // restore state
    addq $0x10, %rsp          // "pop" n and errcode
    iretq

#define ENTRIES 2

FOR 0, 8, DEF_WRAPPER
FOR 8, 9, DEF_WRAPPER_ERROR
FOR 9, 10, DEF_WRAPPER
FOR 10, 15, DEF_WRAPPER_ERROR
FOR 15, 17, DEF_WRAPPER
FOR 17, 18, DEF_WRAPPER_ERROR
FOR 18, 30, DEF_WRAPPER
FOR 30, 31, DEF_WRAPPER_ERROR
FOR 31, 32+ENTRIES, DEF_WRAPPER

handler_labels:
    FOR 0, 32+ENTRIES, LIST_WRAPPER
