#ifndef __PIT_H__
#define __PIT_H__

#define PIT_control_port 0x43
#define PIT_data_port    0x40  // for the control port above

#define FREQ 1193180    // PIT frequency in Hz
#define CALC_DIV(x) ((FREQ+(x)/2)/(x))

void init_PIT();

#endif /* __PIT_H__ */