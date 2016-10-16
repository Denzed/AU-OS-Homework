#include "general.h"
#include "buddy.h"
#include "debug.h"
#include "desc.h"
#include "handlers.h"
#include "io.h"
#include "memory.h"
#include "pic.h"
#include "pit.h"

static void qemu_gdb_hang(void) {
#ifdef DEBUG
    static volatile int wait = 1;

    while (wait);
    wait = 1;
#endif
}

void main(void) {
// initial hang
    qemu_gdb_hang();
// setup memory mapping
    setup_memory_map(false);
    output_memory_map(mmap, mmap_actual_size);
// setup page allocation
    setup_buddy_allocators();
/*  
// page allocator test
    uint64_t tmp[2];
    printf("%#x\n", tmp[0] = allocate_buddy(2));
    printf("%#x\n", tmp[1] = allocate_buddy(1));
    free_buddy(tmp[0]);
    printf("%#x\n", tmp[0] = allocate_buddy(1));
    free_buddy(tmp[1]);
    printf("%#x\n", tmp[1] = allocate_buddy(1));
*/
// backtracing test
/*
    printf("Stack begins at %x\n", GLOBAL_STACK_BOTTOM);
    printf("Main:\n");
    backtrace();
// disable interrupts so we can safely setup them
    disable_interrupts();
// setup IDT
    struct IDT_entry table[ENTRIES]; // 256 maximum
                                     // first 32 are reserved for exceptions

    for (int i = 0; i < ENTRIES; ++i) {
        make_entry(handler_labels[i],
                   KERNEL_CS,
                   1,
                   0xe,
                   table + i);
    }

    struct desc_table_ptr ptr = {sizeof(table) - 1, (uint64_t) table};
    write_idtr(&ptr);

    printf("Finished IDT!\n");

// setup serial port
    init_serial();
// setup PICs -- interrupt controllers
    init_master_PIC();
    mask_master_PIC(0xff);
    init_slave_PIC();
    mask_slave_PIC(0xff);
// setup PIT -- timer
    init_PIT();

    printf("Finished init!\n");

// unmask some interrupts on PIC
    mask_master_PIC(0xff ^ 0x1); // allow irq 0
    mask_slave_PIC(0xff);
// allow interrupts
    enable_interrupts();
// interrupts test
    gen_interrupt(29);
*/

    while (1);
}
