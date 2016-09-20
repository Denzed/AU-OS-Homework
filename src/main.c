#include <stdbool.h>
#include <stdint.h>
#include <memory.h>
#include <desc.h>
#include <ioport.h>
#include <handlers.h>
#include <pic.h>
#include <pit.h>
#include <video.h>

extern uint64_t handler_labels[];

static void qemu_gdb_hang(void)
{
#ifdef DEBUG
    static volatile int wait = 1;

    while (wait);
    wait = 1;
#endif
}

struct small_IDT {
    struct IDT_entry table[256]; // 256 maximum
                                // first 32 are reserved for exceptions
} __attribute__((packed));

struct small_IDT IDT;

void main(void) {
    // qemu_gdb_hang();

    disable_interrupts(); // just in case :)
    cls();
    printword("handler addresses:\n");
    for (int i = 0; i < ENTRIES; ++i) {
        printnum(i, 10);
        printword(": ");
        printlnum((ull) handler_labels[i]);
        newline();

        make_entry(handler_labels[i],
                   KERNEL_CS,
                   1,
                   0xe,
                   IDT.table + 32 + i);
    }

    struct desc_table_ptr ptr = {sizeof(IDT) - 1, (uint64_t) &IDT};

    write_idtr(&ptr);

    init_serial();
    init_master_PIC();
    mask_master_PIC(0xff);
    init_slave_PIC();
    mask_slave_PIC(0xff);
    init_PIT();

    qemu_gdb_hang();

    printword("finished init!\n");

    mask_master_PIC(0xff ^ 0x1);
    mask_slave_PIC(0xff);
    enable_interrupts();
    gen_interrupt(33);

	while (1) {
        // uint8_t cur_clock = in8(PIT_data_port);
        // write_serial_number(cur_clock, 10);
        // write_serial_string("\r");
    }
}
