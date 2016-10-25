#include "general.h"
#include "io.h"
#include "memory.h"

#define TABLE_ENTRIES   512
#define PTE_PRESENT     (1 << 0)
#define PTE_WRITE       (1 << 1)
#define PTE_LARGE       (1 << 7)

extern ptr bootstrap_pml4;
uint64_t *pml4_entries[TABLE_ENTRIES] = {0};

void setup_paging() {
    uint64_t GB = 1ULL << 30;
    // #define GB 0x40000000
    uint64_t have_gigabytes = memory_upper_bound / GB + 
                             (memory_upper_bound % GB != 0);
    uint64_t need_entries   = have_gigabytes / TABLE_ENTRIES + 
                             (have_gigabytes % TABLE_ENTRIES != 0);
    /* time to search for needed amount of memory across the first 4GB
       for which we have a mapping */
    for (uint64_t i = 0; i < mmap_actual_size && need_entries; ++i) {
        mmap_entry_t *entry = &mmap[i];
        if (entry->type != 1) {
            continue;
        }
        // 512 * 8 = 64KB = PAGE_SIZE --- what a coincidence!
        ptr entry_begin = align_up(entry->base_addr);
        ptr entry_end = entry->base_addr + entry->length;
        if (entry_end > 4 * GB) {
            entry_end = 4 * GB;
        }
        uint64_t have_space_for = (entry_end - entry_begin) / PAGE_SIZE;
        if (entry_end <= entry_begin || !have_space_for) {
            continue;
        }
        if (have_space_for > need_entries) {
            have_space_for = need_entries;
        }
        // add system mmap entry 
        if (mmap_actual_size >= MEMORY_MAP_MAXIMUM_SIZE) {
            printf("Memory map overflow! Current MEMORY_MAP_MAXIMUM_SIZE "
                "constant is not enough!\n");
        }
        mmap[mmap_actual_size].base_addr = entry_begin;
        uint64_t length = PAGE_SIZE * have_space_for;
        mmap[mmap_actual_size].length = length;
        mmap[mmap_actual_size].type = TYPE_KERNEL;
        ++mmap_actual_size;
        // "allocate" found memory
        for (uint64_t i = need_entries - have_space_for; i < need_entries; ++i) {
            pml4_entries[i] = (uint64_t *) entry_begin;
            entry_begin += PAGE_SIZE;
        }
        need_entries -= have_space_for;
        // update the original entry 
        entry->length -= entry_begin - entry->base_addr;
        entry->base_addr = entry_begin;
    }
    // now we fill our table
    for (uint64_t i = 0; i < have_gigabytes; ++i) {
        uint64_t upper = TABLE_ENTRIES / 2;
        pml4_entries[i / TABLE_ENTRIES + upper][i % TABLE_ENTRIES] = 
            (i * GB) | PTE_WRITE | PTE_PRESENT | PTE_LARGE;
    }
    for (uint64_t i = 0; i < have_gigabytes; i += TABLE_ENTRIES) {
        ((uint64_t *) bootstrap_pml4)[i] = 
            *pml4_entries[i] | PTE_PRESENT | PTE_WRITE;
    }
}