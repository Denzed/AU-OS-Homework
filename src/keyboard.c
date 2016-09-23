#include "general.h"
#include "io.h"
#include "keyboard.h"


// PS/2 keyboard code.
// Dependencies:
// inb function and scancode table.
char get_scancode() {
    char c = 0;
    do {
        if (in8(KEYBOARD_OUT) != c) {
            c = in8(0x60);
            if (c > 0) {
                return c;
            }
        }
    } while (1);
}

// char getchar() {
//     return scancode[getScancode()+1];
// }