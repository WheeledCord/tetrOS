#include "kernel.h"
#include "game_settings.h"

void main_loop() {
    print("Welcome to TetrOS!",WHITE,0,0);
}

unsigned int tick = 0;
void tick_handler() {
    tick++;
    main_loop();
    screen_update();

    outb(0x20, 0x20);
}
void key_handler() {
    unsigned char scancode = inb(0x60);



    outb(0x20, 0x20);
}

void kernel_init() {
    outb(0x3C6, 0xFF);

    // Set up display
    clear_screen();
    disable_cursor();
    set_palette();
    screen_update();

    pic_remap();
    k_install_idt();
    asm volatile ("sti");
    while (true) { asm volatile ("hlt"); };
}
