#include <stdint.h>
#include "desc.h"

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