#include "desc.h"

void read_idtr(struct desc_table_ptr *ptr) {
    __asm__ ("sidt %0" : "=m"(*ptr));
}

void write_idtr(const struct desc_table_ptr *ptr) {
    __asm__ ("lidt %0" : : "m"(*ptr));
}

void read_gdtr(struct desc_table_ptr *ptr) {
    __asm__ ("sgdt %0" : "=m"(*ptr));
}

void write_gdtr(const struct desc_table_ptr *ptr) {
    __asm__ ("lgdt %0" : : "m"(*ptr));
}

void make_entry(uint64_t offset, 
                uint16_t segment, 
                uint8_t P, 
                uint8_t type, 
                struct IDT_entry* entry_address) {
    entry_address->offset15_0 = offset & 0xffff;
    entry_address->offset31_16 = (offset >> 16) & 0xffff;
    entry_address->offset63_32 = offset >> 32;
    entry_address->segment_selector = segment;
    entry_address->free_space0 = 
        entry_address->free_space1 = 0;
    entry_address->P_space_type = ((P & 0x1) << 7) | (type & 0xf);
}