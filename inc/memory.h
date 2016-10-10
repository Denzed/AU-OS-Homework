#ifndef __MEMORY_H__
#define __MEMORY_H__

#define VIRTUAL_BASE    0xffffffff80000000
#define PAGE_SIZE       0x1000
#define KERNEL_CS       0x08
#define KERNEL_DS       0x10

#ifndef __ASM_FILE__

typedef uint64_t ptr;

// multiboot info header
extern uint32_t multiboot_info_h[];

struct multiboot_info_header {
    uint32_t magic;
    uint32_t flags;
    uint32_t checksum;
    uint32_t header_addr;
    uint32_t load_addr;
    uint32_t load_end_addr;
    uint32_t bss_end_addr;
    uint32_t entry_addr;
} __attribute__((packed));

typedef struct multiboot_info_header multiboot_info_header_t;
multiboot_info_header_t *get_multiboot_info_header(void);
void output_multiboot_info_header(multiboot_info_header_t *);

// multiboot info structure
extern uint32_t multiboot_info_s;

struct multiboot_info_structure {
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_addr;
    uint32_t syms[4];
    uint32_t mmap_length;
    uint32_t mmap_addr;
    uint32_t drives_length;
    uint32_t drives_addr;
    uint32_t config_table;
    uint32_t boot_loader_name;
    uint32_t apm_table;
    uint32_t vbe_control_info;
    uint32_t vbe_mode_info;
    uint16_t vbe_mode;
    uint16_t vbe_interface_seg;
    uint16_t vbe_interface_off;
    uint16_t vbe_interface_len;
} __attribute__((packed));

typedef struct multiboot_info_structure multiboot_info_structure_t;

multiboot_info_structure_t *get_multiboot_info_structure(void);
void output_multiboot_info_structure(multiboot_info_structure_t *);

// memory map
#define MEMORY_MAP_MAXIMUM_SIZE 32

struct mmap_entry {
    uint32_t size;
    uint64_t base_addr;
    uint64_t length;
    uint32_t type;
} __attribute__((packed));

typedef struct mmap_entry mmap_entry_t;

extern mmap_entry_t mmap[MEMORY_MAP_MAXIMUM_SIZE];
extern uint32_t mmap_actual_size;

void setup_memory_map(bool);
void output_memory_map(mmap_entry_t[], uint32_t);

#endif /*__ASM_FILE__*/

#endif /*__MEMORY_H__*/
