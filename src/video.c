#include "general.h"
#include "io.h"
#include "video.h"

static int xpos = 0;
static int ypos = 0;
static volatile unsigned char *video = (unsigned char *) VIDEO;

void set_pointer(int x, int y) {
    xpos = x;
    ypos = y;
}

int video_get_x() {
    return xpos;
}

int video_get_y() {
    return ypos;
}

void cls() {
    video = (unsigned char *) VIDEO;
    for (int i = 0; i < COLUMNS * LINES * 2; ++i) {
        video[i] = 0;
    }
    set_pointer(0, 0);
}

static inline void newline() {
    xpos = 0;
    if (++ypos >= LINES) {
        ypos = 0;
    }
}

static inline void write_video_char_at(char c, int x, int y) {
    video[(x + y * COLUMNS) * 2] = c & 0xFF;
    video[(x + y * COLUMNS) * 2 + 1] = ATTRIBUTE;
}

int write_video_char(struct additional_info *info, char c) {
    ++info; // actually we don't need this information
    write_video_char_at(c, xpos, ypos);
    xpos++;
    if (xpos >= COLUMNS) {
        newline();
    }
    return 1;
}