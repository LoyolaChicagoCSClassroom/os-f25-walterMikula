#ifndef PAGE_H
#define PAGE_H

#include <stddef.h>

struct ppage {
    struct ppage *next;
    struct ppage *prev;
    void *physical_addr;   // base of 2 MiB frame
};

extern struct ppage *free_physical_pages_head;

void init_pfa_list(void);
struct ppage *allocate_physical_pages(unsigned int npages);
void free_physical_pages(struct ppage *ppage_list);

#endif
