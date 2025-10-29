#include <stdint.h>
#include <stddef.h>
#include "mmu.h"


#define PAGE_SIZE        4096

#define PAGE_DIR_ENTRIES 1024

#define PAGE_TBL_ENTRIES 1024

struct page_directory_entry page_directory[PAGE_DIR_ENTRIES]

    __attribute__((aligned(PAGE_SIZE)));



struct page first_page_table[PAGE_TBL_ENTRIES]

    __attribute__((aligned(PAGE_SIZE)));



static inline uint32_t pd_index(uint32_t vaddr) {

    return (vaddr >> 22) & 0x3FF;

}



static inline uint32_t pt_index(uint32_t vaddr) {

    return (vaddr >> 12) & 0x3FF;

}



static inline uint32_t align_down_4k(uint32_t addr) {

    return addr & 0xFFFFF000;

}

void *map_pages(void *vaddr,

                struct ppage *pglist,

                struct page_directory_entry *pd)

{

    uint32_t curr_vaddr = (uint32_t)vaddr;

    struct ppage *curr_page = pglist;



    while (curr_page != NULL) {



        uint32_t pdi = pd_index(curr_vaddr);

        uint32_t pti = pt_index(curr_vaddr);



        if (!pd[pdi].present) {

            pd[pdi].present       = 1;

            pd[pdi].rw            = 1;

            pd[pdi].user          = 0;

            pd[pdi].writethru     = 0;

            pd[pdi].cachedisabled = 0;

            pd[pdi].accessed      = 0;

            pd[pdi].pagesize      = 0; // 0 => points to 4KB page table

            pd[pdi].ignored       = 0;

            pd[pdi].os_specific   = 0;

            pd[pdi].frame         =

                ((uint32_t)first_page_table) >> 12;

        }
	first_page_table[pti].present  = 1;

        first_page_table[pti].rw       = 1;

        first_page_table[pti].user     = 0;

        first_page_table[pti].accessed = 0;

        first_page_table[pti].dirty    = 0;

        first_page_table[pti].unused   = 0;

        first_page_table[pti].frame    =

                (curr_page->physical_addr) >> 12;



        curr_vaddr += PAGE_SIZE;

        curr_page  = curr_page->next;

    }



    return vaddr;

}
static inline void loadPageDirectory(struct page_directory_entry *pd) {

    asm volatile(

        "mov %0, %%cr3"

        :

        : "r"(pd)

        : "memory"

    );

}



static inline void enablePaging(void) {

    asm volatile(

        "mov %%cr0, %%eax\n"

        "or  $0x80000001, %%eax\n"

        "mov %%eax, %%cr0\n"

        :

        :

        : "eax", "memory"

    );

}


extern uint32_t _end_kernel;



void kernel_main(void) {

    // clear tables first

    for (int i = 0; i < PAGE_DIR_ENTRIES; i++) {

        ((uint32_t*)&page_directory[i])[0] = 0;

    }

    for (int i = 0; i < PAGE_TBL_ENTRIES; i++) {

        ((uint32_t*)&first_page_table[i])[0] = 0;

    }



    // identity-map kernel region [0x00100000, &_end_kernel)

    {

        uint32_t start = 0x00100000;

        uint32_t end   = (uint32_t)&_end_kernel;

        if (end & 0xFFF) {

            end = (end & 0xFFFFF000) + PAGE_SIZE;

        }
	for (uint32_t addr = start; addr < end; addr += PAGE_SIZE) {

            struct ppage tmp;

            tmp.next = NULL;

            tmp.physical_addr = addr;

            map_pages((void*)addr, &tmp, page_directory);

        }

    }



    // identity-map stack page(s)

    {

        uint32_t esp;

        asm("mov %%esp,%0" : "=r"(esp));

        uint32_t stack_page_base = align_down_4k(esp);



        for (uint32_t addr = stack_page_base - PAGE_SIZE;

             addr <= stack_page_base;

             addr += PAGE_SIZE)

        {

            struct ppage tmp;

            tmp.next = NULL;

            tmp.physical_addr = addr;

            map_pages((void*)addr, &tmp, page_directory);

        }

    }
    // identity-map VGA text buffer 0xB8000

    {

        struct ppage tmp;

        tmp.next = NULL;

        tmp.physical_addr = 0x000B8000;

        map_pages((void*)0x000B8000, &tmp, page_directory);

    }



    // load CR3 and enable paging

    loadPageDirectory(page_directory);

    enablePaging();
}
