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
#define GRID_COLS        10
#define GRID_ROWS         20
#define CELL_WIDTH         2
#define GRID_X_OFFSET     10
#define GRID_Y_OFFSET      5
#define EMPTY_CELL       255

volatile uint32 timer_ticks = 0;
volatile uint32 seconds_elapsed = 0;
uint8 grid[GRID_ROWS][GRID_COLS];
typedef struct {
    int x;
    int y;
} Block;
Block fallingBlock;
uint32 lastFallTick = 0;

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

void spawn_new_block(void) {
    fallingBlock.x = GRID_COLS / 2;
    fallingBlock.y = 0;
}

void draw_cell(int row, int col, int filled) {
    int screen_x = GRID_X_OFFSET + col * CELL_WIDTH;
    int screen_y = GRID_Y_OFFSET + row;
    uint16 *vga = (uint16*)VGA_ADDRESS;
    int index = screen_y * VGA_WIDTH + screen_x;
    if (filled) {
        vga[index] = vga_item_entry('[', COLOR_WHITE, COLOR_BLACK);
        vga[index+1] = vga_item_entry(']', COLOR_WHITE, COLOR_BLACK);
    } else {
        vga[index] = vga_item_entry(' ', COLOR_WHITE, COLOR_BLACK);
        vga[index+1] = vga_item_entry('.', COLOR_WHITE, COLOR_BLACK);
    }
}

void draw_grid(void) {
    int r, c;
    for (r = 0; r < GRID_ROWS; r++) {
        for (c = 0; c < GRID_COLS; c++) {
            int cellFilled = (grid[r][c] != EMPTY_CELL);
            if (r == fallingBlock.y && c == fallingBlock.x)
                cellFilled = 1;
            draw_cell(r, c, cellFilled);
        }
    }
}

void kmain(void) {
    int sc;
    gdt_init();
    idt_init();
    console_init(COLOR_WHITE, COLOR_BLACK);
    keyboard_init();
    pic8259_init();
    timer_init();
    isr_register_interrupt_handler(IRQ_TIMER_VECTOR, timer_handler);
    vga_disable_cursor();
    console_clear(COLOR_WHITE, COLOR_BLACK);
    int r, c;
    for (r = 0; r < GRID_ROWS; r++)
        for (c = 0; c < GRID_COLS; c++)
            grid[r][c] = EMPTY_CELL;
    spawn_new_block();
    lastFallTick = timer_ticks;
    while (1) {
        sc = kb_poll_scancode();
        if (sc != 0) {
            if (sc == 0x4B) {
                if (fallingBlock.x > 0 && grid[fallingBlock.y][fallingBlock.x - 1] == EMPTY_CELL)
                    fallingBlock.x--;
            } else if (sc == 0x4D) {
                if (fallingBlock.x < GRID_COLS - 1 && grid[fallingBlock.y][fallingBlock.x + 1] == EMPTY_CELL)
                    fallingBlock.x++;
            }
        }
        if (timer_ticks - lastFallTick >= 30) {
            lastFallTick = timer_ticks;
            int newY = fallingBlock.y + 1;
            if (newY >= GRID_ROWS || grid[newY][fallingBlock.x] != EMPTY_CELL) {
                grid[fallingBlock.y][fallingBlock.x] = 1;
                spawn_new_block();
            } else {
                fallingBlock.y = newY;
            }
        }
        draw_grid();
    }
}
