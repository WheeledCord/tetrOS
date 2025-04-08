#include "console.h"
#include "string.h"
#include "gdt.h"
#include "idt.h"
#include "keyboard.h"
#include "vga.h"
#include "8259_pic.h"
#include "io_ports.h"
#include "isr.h"

#define PIT_FREQUENCY    1193182
#define TIMER_FREQUENCY  100
#define IRQ_TIMER_VECTOR (IRQ_BASE + IRQ0_TIMER)

volatile uint32 timer_ticks = 0;
volatile uint32 seconds_elapsed = 0;

void update_timer_display(const char* sec_str) {
    char timerStr[32];
    strcpy(timerStr, "Time: ");
    strcat(timerStr, sec_str);
    int len = strlen(timerStr);
    int x = VGA_WIDTH - len;
    int y = 0;
    uint16 *vga = (uint16*)VGA_ADDRESS;
    int i = 0;
    while (timerStr[i]) {
        int pos = y * VGA_WIDTH + (x + i);
        vga[pos] = vga_item_entry(timerStr[i], COLOR_YELLOW, COLOR_BLACK);
        i++;
    }
    while (i < 16) {
        int pos = y * VGA_WIDTH + (x + i);
        vga[pos] = vga_item_entry(' ', COLOR_YELLOW, COLOR_BLACK);
        i++;
    }
}

void timer_handler(REGISTERS *regs) {
    (void)regs;
    timer_ticks++;
    if (timer_ticks % TIMER_FREQUENCY == 0) {
        seconds_elapsed++;
        char buf[16];
        itoa(buf, 10, seconds_elapsed);
        update_timer_display(buf);
    }
    pic8259_eoi(IRQ0_TIMER);
}

void timer_init(void) {
    uint16 divisor = PIT_FREQUENCY / TIMER_FREQUENCY;
    outportb(0x43, 0x36);
    outportb(0x40, (uint8)(divisor & 0xFF));
    outportb(0x40, (uint8)((divisor >> 8) & 0xFF));
}

void kmain(void) {
    gdt_init();
    idt_init();
    console_init(COLOR_WHITE, COLOR_BLACK);
    keyboard_init();
    pic8259_init();
    timer_init();
    isr_register_interrupt_handler(IRQ_TIMER_VECTOR, timer_handler);
    console_clear(COLOR_WHITE, COLOR_BLACK);
    console_gotoxy(0, 2);
    console_putstr("Simple Text Editor\n");
    console_putstr("Type below; use Backspace to delete.\n");
    while (1) {
        char c = kb_getchar();
        if (c == '\b') {
            console_ungetchar();
        } else {
            console_putchar(c);
        }
    }
}
