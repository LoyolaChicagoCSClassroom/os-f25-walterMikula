
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <sys/io.h>

static unsigned char keyboard_map[128] = {
    0, 27,'1','2','3','4','5','6','7','8',      /* 9 */
   '9','0','-','=', '\b',                        /* backspace */
   '\t',                                         /* tab */
   'q','w','e','r',                              /* 19 */
   't','y','u','i','o','p','[',']','\n',         /* enter */
    0,                                           /* ctrl */
   'a','s','d','f','g','h','j','k','l',';','\'','`', 0, /* lshift */
   '\\','z','x','c','v','b','n','m',',','.','/', 0,     /* rshift */
   '*', 0, ' ', 0,                               /* alt, space, caps */
   0,0,0,0,0,0,0,0, 0,                           /* F1..F10 (unused) */
   0, 0, 0, 0, 0, '-', 0, 0, 0, '+',             /* arrows, keypad + -  */
   0, 0, 0, 0, 0, 0, 0,                          /* rest unused */
   0, 0, 0, 0, 0, 0, 0,                          /* F11, F12, ... */
};

static char printable(unsigned char ch) {
    return (ch >= 32 && ch < 127) ? (char)ch : '.';
}

int main(void) {
    if (ioperm(0x60, 1, 1) != 0 || ioperm(0x64, 1, 1) != 0) {
        perror("ioperm (need to run as root)");
        return 1;
    }

    puts("Polling PS/2 controller");
    fflush(stdout);

    // Poll forever
    for (;;) {
        // Read status register at 0x64
        unsigned char status = inb(0x64);

        // 1 means data available
        if (status & 0x01) {
            // Read scancode from data port 0x60
            unsigned char sc = inb(0x60);

            if (sc & 0x80) {
                continue;
            }

            char ch = (sc < 128) ? (char)keyboard_map[sc] : 0;

            if (ch) {
                printf("press: 0x%02x  '%c'\n", sc, printable((unsigned char)ch));
            } else {
                printf("press: 0x%02x  (unmapped)\n", sc);
            }
            fflush(stdout);
        }

        usleep(2000);
    }

    return 0;
}

