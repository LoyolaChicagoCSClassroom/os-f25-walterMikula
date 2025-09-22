#include <stdint.h> include "rprintf.h"

extern void putc(int ch);
extern void clear_screen(void);
extern int  get_cpl(void);

#define MULTIBOOT2_HEADER_MAGIC 0xe85250d6

const unsigned int multiboot_header[]
__attribute__((section(".multiboot"))) =
{
    MULTIBOOT2_HEADER_MAGIC, 0, 16, -(16+MULTIBOOT2_HEADER_MAGIC), 0, 12
};

void main(void) {
    clear_screen();
    esp_printf(putc, "Hello from my kernel!\r\n");
    esp_printf(putc, "CPL = %d\r\n", get_cpl());
    for (int i=0; i < 110; i++) esp_printf(putc,"Line Number: %d\r\n", i);
    // Stop here so text stays visible
    for (;;)
        __asm__ __volatile__("hlt");
}
