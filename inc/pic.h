#include <desc.h>
#include <stdint.h>
#include <ioport.h>

#ifndef __PIC_H__
#define __PIC_H__


#define master_PIC_command_register 0x20
#define master_PIC_data_register 0x21
#define slave_PIC_command_register 0xA0
#define slave_PIC_data_register 0xA1

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

void init_master_PIC(void) {
    // init command
    out8(master_PIC_command_register, PIC_init);
    // map first irq to the specified IDT table position
    out8(master_PIC_data_register, 0x20);
    // bit mask of slave connections
    out8(master_PIC_data_register, 1 << slave_PIC_irq);
    // other parameters
    out8(master_PIC_data_register, 0x1);
}

void mask_master_PIC(uint8_t new_mask) {
    out8(master_PIC_data_register, new_mask);
}

void init_slave_PIC(void) {
    // init command
    out8(slave_PIC_command_register, PIC_init);
    // map first irq to the specified IDT table position
    out8(slave_PIC_data_register, 32 + 8 - 1);
    // irq of master PIC to which current slave is connected
    out8(slave_PIC_data_register, 0x2);         
    // other parameters
    out8(slave_PIC_data_register, 0x1);
}

void mask_slave_PIC(uint8_t new_mask) {
    out8(slave_PIC_data_register, new_mask);
}

void send_EOI_PIC(unsigned char irq) {
    if (irq > 8) {
        out8(slave_PIC_command_register, PIC_EOI + irq - 8);
        out8(master_PIC_command_register, PIC_EOI + slave_PIC_irq);
    } else {
        out8(master_PIC_command_register, PIC_EOI + irq);
    }
}

static uint16_t __pic_get_irq_reg(int ocw3)
{
    out8(master_PIC_command_register, ocw3);
    out8(slave_PIC_command_register, ocw3);
    return (in8(slave_PIC_command_register) << 8) | 
            in8(master_PIC_command_register);
}

// get irq request register 
uint16_t pic_get_irr(void) {
    return __pic_get_irq_reg(PIC_read_IRR);
}

// get in-service register 
uint16_t pic_get_isr(void) {
    return __pic_get_irq_reg(PIC_read_ISR);
}

#endif /* __PIC_H__ */