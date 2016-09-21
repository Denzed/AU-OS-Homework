#ifndef __PIC_H__
#define __PIC_H__

#define master_PIC_command_register 0x20
#define master_PIC_data_register 0x21
#define slave_PIC_command_register 0xA0
#define slave_PIC_data_register 0xA1
#define master_PIC_first_input_IDT_position 0x20
// the last one is chosen in a way that other interrupts will have 
// smaller number
// IMPORTANT: must be divisible by 8 as PIC ignores 3 least bits

// irq stands for Interrupt ReQuest

#define slave_PIC_irq 0x2
#define PIC_init      0x11
#define PIC_EOI       0x60
#define PIC_read_IRR  0x0a
#define PIC_read_ISR  0x0b

// Current configuration is:
// Master PIC 
// + 
// Slave one connected to its second input

void init_master_PIC(void);
void mask_master_PIC(uint8_t);
void init_slave_PIC(void);
void mask_slave_PIC(uint8_t);
void send_EOI_PIC(unsigned char);

uint16_t pic_get_irr(void); 
uint16_t pic_get_isr(void);

#endif /* __PIC_H__ */