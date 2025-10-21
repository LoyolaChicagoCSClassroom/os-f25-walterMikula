#include <stdint.h>
#include "rprintf.h"

extern void putc(int ch);

// esp_printf expects int (*)(int); bridge your void putc(int)
static int putc_adapter(int ch) { putc(ch); return ch; }

/* ---- x86 I/O port access ---- */
static inline uint8_t inb(uint16_t port) {
    uint8_t val;
    __asm__ volatile ("inb %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

/* ---- PS/2 controller ports / bits ---- */
#define KBD_DATA 0x60
#define KBD_STAT 0x64
#define KBD_OBF  0x01   // output buffer full (data available)

/* ---- simple US keyboard scancode set 1 map ---- */
static const unsigned char keyboard_map[128] = {
    0, 27,'1','2','3','4','5','6','7','8',
   '9','0','-','=', '\b',
   '\t',
   'q','w','e','r',
   't','y','u','i','o','p','[',']','\n',
    0,
   'a','s','d','f','g','h','j','k','l',';','\'','`', 0,
   '\\','z','x','c','v','b','n','m',',','.','/', 0,
   '*', 0, ' ', 0,
   0,0,0,0,0,0,0,0, 0,
   0, 0, 0, 0, 0, '-', 0, 0, 0, '+',
   0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0,
};

static inline char printable(unsigned char ch) {
    return (ch >= 32 && ch < 127) ? (char)ch : '.';
}

/* Call this from kernel_main when you want to poll the keyboard */
void ps2_poll_loop(void) {
    esp_printf(putc_adapter, "Polling PS/2 controller\r\n");

    for (;;) {
        uint8_t status = inb(KBD_STAT);
        if (status & KBD_OBF) {
            uint8_t sc = inb(KBD_DATA);

            // Ignore key releases (high bit set)
            if (sc & 0x80)
                continue;

            char ch = (sc < 128) ? (char)keyboard_map[sc] : 0;
            if (ch) {
                esp_printf(putc_adapter, "press: 0x%02x  '%c'\r\n", sc, printable((unsigned char)ch));
            } else {
                esp_printf(putc_adapter, "press: 0x%02x  (unmapped)\r\n", sc);
            }
        }

        // tiny spin delay to avoid hammering the bus too hard
        for (volatile int i = 0; i < 20000; i++) __asm__ volatile ("pause");
    }
}
