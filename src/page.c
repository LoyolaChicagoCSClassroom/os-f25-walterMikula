#include "page.h"
#include <stdint.h>

#define PAGE_SIZE_2MB (2u * 1024u * 1024u)

struct ppage physical_page_array[128];
struct ppage *free_physical_pages_head = NULL;

void init_pfa_list(void) {
    const int N = (int)(sizeof(physical_page_array)/sizeof(physical_page_array[0]));
    for (int i = 0; i < N; i++) {
        struct ppage *p = &physical_page_array[i];
        p->physical_addr = (void*)((uintptr_t)i * (uintptr_t)PAGE_SIZE_2MB);
        p->prev = (i > 0) ? &physical_page_array[i - 1] : NULL;
        p->next = (i + 1 < N) ? &physical_page_array[i + 1] : NULL;
    }
    free_physical_pages_head = &physical_page_array[0];
}

struct ppage *allocate_physical_pages(unsigned int npages) {
    if (npages == 0 || free_physical_pages_head == NULL) return NULL;

    // ensure enough pages exist
    struct ppage *scan = free_physical_pages_head;
    for (unsigned int i = 1; i < npages && scan; i++) scan = scan->next;
    if (scan == NULL) return NULL; // not enough pages

    // detach first npages
    struct ppage *alloc_head = free_physical_pages_head;
    struct ppage *alloc_tail = alloc_head;
    for (unsigned int i = 1; i < npages; i++) alloc_tail = alloc_tail->next;

    free_physical_pages_head = alloc_tail->next;
    if (free_physical_pages_head) free_physical_pages_head->prev = NULL;

    alloc_tail->next = NULL;
    alloc_head->prev = NULL;

    return alloc_head;
}

void free_physical_pages(struct ppage *ppage_list) {
    if (!ppage_list) return;

    // find tail
    struct ppage *tail = ppage_list;
    while (tail->next) tail = tail->next;

    tail->next = free_physical_pages_head;
    if (free_physical_pages_head) free_physical_pages_head->prev = tail;

    ppage_list->prev = NULL;
    free_physical_pages_head = ppage_list;
}
