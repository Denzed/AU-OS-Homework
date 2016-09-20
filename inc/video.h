#include <stdint.h>

#ifndef __VIDEO_H__
#define __VIDEO_H__


#define COLS 80
#define ROWS 25
#define ADDR 0xb8000
#define ATTR 7

#define TAB_WIDTH 4
#define swap(a, b) (a) ^= (b); (b) ^= (a); (a) ^= (b);

typedef unsigned int            uint;
typedef unsigned long long      ull;

volatile int xpos = 0;
volatile int ypos = 0;
volatile unsigned char *cur_addr = (unsigned char *) ADDR;

void cls(void) {
    cur_addr = (unsigned char *) ADDR;
    for (int i = 0; i < COLS * ROWS * 2; i++) {
        *(cur_addr + i) = 0;
    }
    xpos = 0;
    ypos = 0;
}

void newline() {
    xpos = 0;
    ypos++;
    if (ypos >= ROWS) {
        ypos = 0;
    }
}

void putchar(int c) {
    if (c == '\n') {
        newline();
        return;
    }
    if (c == '\t') {
        for (int to_add = TAB_WIDTH - xpos % TAB_WIDTH; to_add--; putchar(' '));
        return;
    }
    if (c == '\r') {
        xpos = 0;
        return;
    }

    *(cur_addr + (xpos + ypos * COLS) * 2) = c & 0xFF;
    *(cur_addr + (xpos + ypos * COLS) * 2 + 1) = ATTR;

    xpos++;
    if (xpos >= COLS) {
        newline();
    }
}

void putchar_at(int c, int x, int y) {
    x %= COLS;
    y %= ROWS;
    swap(x, xpos);
    swap(y, ypos);
    putchar(c);
    swap(x, xpos);
    swap(y, ypos);
}

void printword(char *word) {
    for (; *word; ) {
        putchar(*(word++));
    }
}

void printnum(uint x, uint base) {
    char buf[100];
    uint len = 0;
    for (; x; ) {
        buf[len] = (x % base > 9 ? 'a' + x % base - 10 : '0' + x % base);
        ++len;
        x /= base;
    }
    if (len == 0) {
        buf[0] = '0';
        len = 1;
    }
    for (int i = len - 1; i > -1; --i) {
        putchar(buf[i]);
    }
}

void printlnum(ull x) {
    if (x >> 32) {
        printnum(x >> 32, 16);
    }
    printnum(x & 0xffffffff, 16);
}

#endif /*__VIDEO_H__*/