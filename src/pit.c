#include <stdint.h>
#include "io.h"
#include "pit.h"

void init_PIT() {
    uint8_t BCD = 0 & 0x1;
    uint8_t mode = 2 & 0x7;   // interrupt on zero and restart
    uint16_t div_we_want = CALC_DIV(100); // ~0.01s
    // uint16_t div_we_want = -1; // ~0.05s
    uint8_t div_least = div_we_want & 0xff;
    uint8_t div_high = div_we_want >> 8;
    uint8_t div = 3 & 0x3;
    uint8_t ch = 0 & 0x3;   // choose out: 
                            // we want one connected to Master PIC's 0 input
    uint16_t command = ((((((ch << 2) | div) << 3) | mode) << 1) | BCD);
    out8(PIT_control_port, command);
    out8(PIT_data_port, div_least);
    out8(PIT_data_port, div_high);
}
