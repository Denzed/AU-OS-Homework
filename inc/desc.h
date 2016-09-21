#ifndef __DESC_H__
#define __DESC_H__

struct IDT_entry {                  // Interrupt Descriptor Table entry
    uint16_t offset15_0;            
    uint16_t segment_selector; 
    uint8_t  free_space1;
    uint8_t  P_space_type;          // Two magic fields + space between them
                                    // [1 bit of P | 
                                    //  3 bits of free space | 
                                    //  4 bits of type]
    uint16_t offset31_16;
    uint32_t offset63_32;           // Handler 64-bit address 
                                    // is split into three parts
    uint32_t free_space0;
} __attribute__((packed));


void make_entry(uint64_t, uint16_t, uint8_t, uint8_t, struct IDT_entry*);

struct desc_table_ptr {
    uint16_t size;
    uint64_t addr;
} __attribute__((packed));

static inline void read_idtr(struct desc_table_ptr *ptr) {
    __asm__ ("sidt %0" : "=m"(*ptr));
}

static inline void write_idtr(const struct desc_table_ptr *ptr) {
	__asm__ ("lidt %0" : : "m"(*ptr));
}

static inline void read_gdtr(struct desc_table_ptr *ptr) {
	__asm__ ("sgdt %0" : "=m"(*ptr));
}

static inline void write_gdtr(const struct desc_table_ptr *ptr) {
	__asm__ ("lgdt %0" : : "m"(*ptr));
}

#endif /*__DESC_H__*/