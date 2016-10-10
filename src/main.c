#include "general.h"
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
    qemu_gdb_hang();

    setup_memory_map(false);
    output_memory_map(mmap, mmap_actual_size);

/*
    printf("Stack begins at %x\n", GLOBAL_STACK_BOTTOM);
    printf("Main:\n");
    backtrace();
    disable_interrupts(); // just in case :)

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

    init_serial();
    init_master_PIC();
    mask_master_PIC(0xff);
    init_slave_PIC();
    mask_slave_PIC(0xff);
    init_PIT();

    printf("Finished init!\n");

    mask_master_PIC(0xff ^ 0x1); // allow irq 0
    mask_slave_PIC(0xff);
    enable_interrupts();

    gen_interrupt(29);
*/

    while (1);
}
