#ifndef __VIDEO_H__
#define __VIDEO_H__

#define COLUMNS         80
#define LINES           24
#define ATTRIBUTE       7
#define VIDEO           0xb8000

void cls();
void set_pointer(int, int);
int write_video_char(struct additional_info *, char);

#endif /* __VIDEO_H__ */