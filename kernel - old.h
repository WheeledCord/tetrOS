#ifndef KERNEL_H
#define KERNEL_H

//vga text mode color attribute
#define BLACK 0x00
#define WHITE 0x01
#define GRAY 0x02
#define CYAN 0x03
#define BLUE 0x04
#define ORANGE 0x05
#define YELLOW 0x06
#define GREEN 0x07
#define PURPLE 0x08
#define RED 0x09
#define LIGHT_CYAN 0x0A
#define LIGHT_BLUE 0x03  // light blue = cyan
#define LIGHT_ORANGE 0x0B
#define LIGHT_YELLOW 0x0C
#define LIGHT_GREEN 0x0D
#define LIGHT_PURPLE 0x0E
#define LIGHT_RED 0x0F

// booleans
#define bool _Bool
#define true 1
#define false 0

//vga functions
void k_clear_screen();
unsigned int k_printf(char *message, unsigned int line);
void disable_cursor();

//utility functions
void itoa(int value, char *str, int base);

//io port functions
void outb(unsigned short port, unsigned char data);
unsigned char inb(unsigned short port);

//pic and interrupt setup
void pic_remap();
void k_install_idt();
void idt_load(unsigned int);

//device driver initialization
void init_timer();

//kernel main function
void k_main();

//kernel draw grid func
void draw_grid();

#endif
