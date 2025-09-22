#include <stdint.h>
#include <stddef.h>

#define VGA_BASE   ((volatile uint16_t*)0xB8000)
#define WIDTH      80
#define HEIGHT     25

static size_t row = 0, col = 0;
static uint8_t attr = (0 << 4) | 7;  // black background, light grey text
static inline uint16_t vga_entry(char ch, uint8_t color) {
    return (uint16_t)ch | ((uint16_t)color << 8);
}

static void scroll(void) {
    for (size_t r = 1; r < HEIGHT; ++r) {
        for (size_t c = 0; c < WIDTH; ++c) {
            VGA_BASE[(r-1)*WIDTH + c] = VGA_BASE[r*WIDTH + c];
        }
    }
    for (size_t c = 0; c < WIDTH; ++c) {
        VGA_BASE[(HEIGHT-1)*WIDTH + c] = vga_entry(' ', attr);
    }
    row = HEIGHT - 1;
    col = 0;
}

void putc(int ch) {
    unsigned char c = (unsigned char)ch;
    if (c == '\n') {
        col = 0;
        if (++row >= HEIGHT) scroll();
        return;
    }
    if (c == '\r') {
        col = 0;
        return;
    }
    VGA_BASE[row * WIDTH + col] = vga_entry((char)c, attr);
    if (++col >= WIDTH) {
        col = 0;
        if (++row >= HEIGHT) scroll();
    }
}

void clear_screen(void) {
    for (size_t i = 0; i < WIDTH * HEIGHT; i++) {
        VGA_BASE[i] = vga_entry(' ', attr);
    }
    row = col = 0;
}

static inline uint16_t read_cs(void) {
    uint16_t cs;
    __asm__ volatile ("mov %%cs, %0" : "=r"(cs));
    return cs;
}

int get_cpl(void) {
    return (int)(read_cs() & 0x3);
}
