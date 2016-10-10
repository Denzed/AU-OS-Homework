#include "general.h"
#include "io.h"
#include "memory.h"

multiboot_info_header_t *get_multiboot_info_header(void) {
    return (multiboot_info_header_t *) multiboot_info_h;
}

void output_multiboot_info_header(multiboot_info_header_t *addr) {
    printf("Multiboot info header at %#x\n:", addr);
    printf("\tmagic: %#x\n", addr->magic);
    printf("\tflags: %#x\n", addr->flags);
    printf("\tchecksum: %#x\n", addr->checksum);
    printf("\theader_addr: %#x\n", addr->header_addr);
    printf("\tload_addr: %#x\n", addr->load_addr);
    printf("\tload_end_addr: %#x\n", addr->load_end_addr);
    printf("\tbss_end_addr: %#x\n", addr->bss_end_addr);
    printf("\tentry_addr: %#x\n", addr->entry_addr);
}


multiboot_info_structure_t *get_multiboot_info_structure(void) {
    return (multiboot_info_structure_t *) (uint64_t) multiboot_info_s;
}

void output_multiboot_info_structure(multiboot_info_structure_t *addr) {
    printf("Multiboot info structure at %#x\n:", addr);
    printf("\tflags: %#x\n", addr->flags);
    printf("\tmem_lower: %#x\n", addr->mem_lower);
    printf("\tmem_upper: %#x\n", addr->mem_upper);
    printf("\tboot_device: %#x\n", addr->boot_device);
    printf("\tcmdline: %#x\n", addr->cmdline);
    printf("\tmods_count: %#x\n", addr->mods_count);
    printf("\tmods_addr: %#x\n", addr->mods_addr);
    printf("\tsyms[4]: %#x\n", addr->syms);
    printf("\tmmap_length: %#x\n", addr->mmap_length);
    printf("\tmmap_addr: %#x\n", addr->mmap_addr);
    printf("\tdrives_length: %#x\n", addr->drives_length);
    printf("\tdrives_addr: %#x\n", addr->drives_addr);
    printf("\tconfig_table: %#x\n", addr->config_table);
    printf("\tboot_loader_name: %#x\n", addr->boot_loader_name);
    printf("\tapm_table: %#x\n", addr->apm_table);
    printf("\tvbe_control_info: %#x\n", addr->vbe_control_info);
    printf("\tvbe_mode_info: %#x\n", addr->vbe_mode_info);
    printf("\tvbe_mode: %#x\n", addr->vbe_mode);
    printf("\tvbe_interface_seg: %#x\n", addr->vbe_interface_seg);
    printf("\tvbe_interface_off: %#x\n", addr->vbe_interface_off);
    printf("\tvbe_interface_len: %#x\n", addr->vbe_interface_len);
}


mmap_entry_t mmap[MEMORY_MAP_MAXIMUM_SIZE];
uint32_t mmap_actual_size = 0;

void setup_memory_map(bool debug) {
    multiboot_info_header_t *mboot_info_h = get_multiboot_info_header();
    multiboot_info_structure_t *mboot_info_s = get_multiboot_info_structure();
    if (debug) {
        if (!mboot_info_h) {
            printf("Multiboot info header not found!\n");
        }
        output_multiboot_info_header(mboot_info_h);
        if (!((mboot_info_h->flags >> 1) & 1)) {
            printf("Multiboot info header flag 1 is not set -- kernel memory "
                "sector information is not provided!");
        }
        if (!mboot_info_s) {
            printf("Multiboot info structure not found!\n");
        }
        output_multiboot_info_structure(mboot_info_s);
        if (!((mboot_info_s->flags >> 6) & 1)) {
            printf("Multiboot info structure flag 6 is not set -- memory map is"
                " not provided!");
        }
    }
    // setup kernel entry
    mmap[0].base_addr = mboot_info_h->load_addr;
    mmap[0].length = mboot_info_h->bss_end_addr - 
                     mboot_info_h->load_addr;
    mmap[0].type = 0;
    ++mmap_actual_size;
    uint32_t kernel_begin = mboot_info_h->load_addr;
    uint32_t kernel_end = mboot_info_h->bss_end_addr;
    // copy the memory map from multiboot info structure
    uint32_t mmap_length = mboot_info_s->mmap_length;
    ptr mmap_addr = (ptr) mboot_info_s->mmap_addr;
    for (ptr mmap_ptr = mmap_addr; mmap_ptr < mmap_addr + mmap_length; ) {
        mmap_entry_t entry = *(mmap_entry_t *) mmap_ptr;
        // handle overlaps with kernel sector
        uint32_t entry_end = entry.base_addr + entry.length;
        // left end
        if (kernel_begin < entry.base_addr && entry.base_addr < kernel_end) {
            entry.base_addr = kernel_end;
        }
        // right end
        if (kernel_begin < entry_end && entry_end < kernel_end) {
            entry_end = kernel_begin;
        }
        // if everything is okay -- add to mmap
        if (entry_end > entry.base_addr) {
            entry.length = entry_end - entry.base_addr;
            mmap[mmap_actual_size++] = entry;
        }
        mmap_ptr += sizeof(entry.size) + entry.size;
        if (mmap_actual_size > MEMORY_MAP_MAXIMUM_SIZE) {
            printf("Memory map overflow! Current MEMORY_MAP_MAXIMUM_SIZE "
                "constant is not enough!\n");
        }
    }
}

void output_memory_map(mmap_entry_t mmap[], uint32_t mmap_size) {
    for (uint32_t cur = 0; cur < mmap_size; ++cur) {
        // printf("memory range: %#x-%#x, type %lld\n", 
        printf("memory range: %#x-%#x, type %lld\n", 
               mmap[cur].base_addr, 
               mmap[cur].base_addr + mmap[cur].length - 1,
               mmap[cur].type);
    }
}