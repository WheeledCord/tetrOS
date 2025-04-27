#include "console.h"
#include "string.h"
#include "gdt.h"
#include "idt.h"
#include "keyboard.h"
#include "vga.h"
#include "8259_pic.h"
#include "io_ports.h"
#include "isr.h"
#include <stdint.h>

#define PIT_FREQUENCY      1193182
#define TIMER_FREQUENCY    100
#define IRQ_TIMER_VECTOR   (IRQ_BASE + IRQ0_TIMER)
#define GRID_COLS          10
#define GRID_ROWS           20
#define CELL_WIDTH         2
#define GRID_X_OFFSET      10
#define GRID_Y_OFFSET      5
#define EMPTY_CELL         255
#define GRID_BUFFER_COLS   (GRID_COLS * CELL_WIDTH)
#define NUM_TETROMINOS     7

volatile uint32 timer_ticks = 0;
volatile uint32 seconds_elapsed = 0;
uint8 grid[GRID_ROWS][GRID_COLS];

typedef struct {
    int shape;
    int rotation;
    int x;
    int y;
    int tspin;
} Tetromino;

Tetromino fallingPiece;
uint32 lastFallTick = 0;

uint32 score = 0;
uint32 level = 1;
uint32 totalLinesCleared = 0;
uint32 combo = 0;
int lastClearWasDifficult = 0;

static const uint8 tetrominoes[NUM_TETROMINOS][4][4][4] = {
    { { {0,0,0,0}, {0,1,1,0}, {0,1,1,0}, {0,0,0,0} },
      { {0,0,0,0}, {0,1,1,0}, {0,1,1,0}, {0,0,0,0} },
      { {0,0,0,0}, {0,1,1,0}, {0,1,1,0}, {0,0,0,0} },
      { {0,0,0,0}, {0,1,1,0}, {0,1,1,0}, {0,0,0,0} } },
    { { {0,0,0,0}, {2,2,2,2}, {0,0,0,0}, {0,0,0,0} },
      { {0,0,2,0}, {0,0,2,0}, {0,0,2,0}, {0,0,2,0} },
      { {0,0,0,0}, {2,2,2,2}, {0,0,0,0}, {0,0,0,0} },
      { {0,2,0,0}, {0,2,0,0}, {0,2,0,0}, {0,2,0,0} } },
    { { {0,0,0,0}, {3,3,3,0}, {0,3,0,0}, {0,0,0,0} },
      { {0,3,0,0}, {3,3,0,0}, {0,3,0,0}, {0,0,0,0} },
      { {0,3,0,0}, {3,3,3,0}, {0,0,0,0}, {0,0,0,0} },
      { {0,3,0,0}, {0,3,3,0}, {0,3,0,0}, {0,0,0,0} } },
    { { {0,0,0,0}, {0,4,4,0}, {4,4,0,0}, {0,0,0,0} },
      { {4,0,0,0}, {4,4,0,0}, {0,4,0,0}, {0,0,0,0} },
      { {0,0,0,0}, {0,4,4,0}, {4,4,0,0}, {0,0,0,0} },
      { {4,0,0,0}, {4,4,0,0}, {0,4,0,0}, {0,0,0,0} } },
    { { {0,0,0,0}, {5,5,0,0}, {0,5,5,0}, {0,0,0,0} },
      { {0,5,0,0}, {5,5,0,0}, {5,0,0,0}, {0,0,0,0} },
      { {0,0,0,0}, {5,5,0,0}, {0,5,5,0}, {0,0,0,0} },
      { {0,5,0,0}, {5,5,0,0}, {5,0,0,0}, {0,0,0,0} } },
    { { {0,0,0,0}, {6,6,6,0}, {0,0,6,0}, {0,0,0,0} },
      { {0,6,0,0}, {0,6,0,0}, {6,6,0,0}, {0,0,0,0} },
      { {6,0,0,0}, {6,6,6,0}, {0,0,0,0}, {0,0,0,0} },
      { {0,6,6,0}, {0,6,0,0}, {0,6,0,0}, {0,0,0,0} } },
    { { {0,0,0,0}, {7,7,7,0}, {7,0,0,0}, {0,0,0,0} },
      { {7,7,0,0}, {0,7,0,0}, {0,7,0,0}, {0,0,0,0} },
      { {0,0,7,0}, {7,7,7,0}, {0,0,0,0}, {0,0,0,0} },
      { {0,7,0,0}, {0,7,0,0}, {0,7,7,0}, {0,0,0,0} } }
};

static const VGA_COLOR_TYPE tetrominoColors[NUM_TETROMINOS] = {
    COLOR_YELLOW,
    COLOR_CYAN,
    COLOR_MAGENTA,
    COLOR_GREEN,
    COLOR_RED,
    COLOR_BLUE,
    COLOR_BROWN
};

static uint8_t cmos_read(uint8_t reg) {
    outportb(0x70, (reg & 0x7F));
    return inportb(0x71);
}

static void cmos_wait_update(void) {
    while (cmos_read(0x0A) & 0x80) { }
}

static uint32_t get_random_entropy(void) {
    uint32_t entropy = 0;

    cmos_wait_update();
    uint8_t sec  = cmos_read(0x00);
    uint8_t min  = cmos_read(0x02);
    uint8_t hour = cmos_read(0x04);
    uint8_t day  = cmos_read(0x07);
    uint8_t mon  = cmos_read(0x08);
    uint8_t year = cmos_read(0x09);

    entropy ^= ((uint32_t)sec << 0)  | ((uint32_t)min << 8)
             | ((uint32_t)hour << 16) | ((uint32_t)day << 24);
    entropy ^= ((uint32_t)mon << 0)  | ((uint32_t)year << 8);

    entropy ^= timer_ticks;

    volatile uint8_t* vga = (volatile uint8_t*)0xB8000;
    for (int i = 0; i < 16; i++) {
        entropy ^= vga[i] << (i % 8);
    }

    uint8_t* ram = (uint8_t*)0x0000;
    for (int i = 0; i < 32; i++) {
        entropy ^= ram[i] << (i % 8);
    }

    uint32_t tsc;
    asm volatile ("rdtsc" : "=A" (tsc));
    entropy ^= tsc;

    entropy ^= inportb(0x60);
    entropy ^= inportb(0x64);
    entropy ^= inportb(0x80);

    entropy ^= (entropy << 21);
    entropy ^= (entropy >> 35);
    entropy ^= (entropy << 4);

    return entropy;
}

int ram_rand(void) {
    static uint32_t state = 0;
    static int init = 0;

    if (!init) {
        state = get_random_entropy();
        if (state == 0) state = 0xDEADBEEF;
        init = 1;
    }

    state ^= (state << 13);
    state ^= (state >> 17);
    state ^= (state << 5);

    return (int)(state % NUM_TETROMINOS);
}

// int ram_rand(void){
//     volatile uint8* vga=(volatile uint8*)0xB8000;int entropy=0;int t=timer_ticks;
//     for(int i=0;i<160;i+=7){uint8 v=vga[i];uint8 m=((v^i)+(t>>(i%5)))*(i+3);vga[i]=v^m;entropy^=((m<<(i%3))|(m>>(8-(i%3))));}
//     for(int i=0;i<10;i++){int pos=(entropy>>(i*2))%(80*25*2);vga[pos]^=(entropy>>(i%8));entropy+=((vga[pos]*i)^(pos+t));}
//     for(int i=0;i<20;i++){int pos=(entropy>>(i%7))%(80*25*2);vga[pos]=(vga[pos]+(entropy>>((i+3)%8)))^i;entropy^=((vga[pos]<<(i%5))|(vga[pos]>>(8-(i%5))));}
//     int* ram=(int*)0x0000;for(int i=0;i<0x8000;i+=4){ram[i/4]^=(entropy^t^i);ram[i/4]+=entropy*((i%13)+1);}
//     uint8* bios=(uint8*)0xF0000;for(int i=0;i<128;i++){bios[i]^=((entropy>>(i%8))+(i*17));bios[i]+=entropy^(i*3);}
//     for(int i=0;i<64;i++){uint8* idt=(uint8*)(0x00000000+(i*8));*idt^=(entropy^(t>>i));*(idt+4)^=(entropy>>((i%5)+2));}
//     for(int i=0;i<128;i++){int* ptr=(int*)((entropy+i*23)%0x30000);*ptr^=(entropy^(t>>i));*ptr+=(i*entropy)^(~t);}
//     for(int i=0;i<16;i++){uint8* magic=(uint8*)(0xC0000+(i*16));*magic^=((entropy>>(i*3))+(t*i));*magic+=((~entropy)^(i*13));}
//     for(int i=0;i<16;i++){unsigned short port=(unsigned short)(0x1F0+(i*3));unsigned char data=(unsigned char)((entropy>>(i%5))&0xFF);outportb(port,data);}
//     for(int i=0;i<8;i++){unsigned short port=(unsigned short)(0x3C0+(i*7));unsigned char data=(unsigned char)((entropy>>(i%3))&0xFF);outportb(port,data);}
//     for(int i=0;i<32;i++){unsigned short port=(unsigned short)(0x170+(i*2));unsigned char data=(unsigned char)((entropy^(t>>i))&0xFF);outportb(port,data);}
//     if((entropy%17)==0){int m=((entropy%7)-3);if(!m)m=1;switch(entropy%3){case 0:level*=m;break;case 1:score*=m;break;default:timer_ticks*=m;}}
//     entropy^=(t*2654435761);entropy=((entropy>>13)^(entropy<<7))^0xA5A5A5A5;entropy=((entropy*7)+(entropy>>3))^(entropy<<5);
//     entropy+=(entropy^0xDEADBEEF)-((entropy>>2)+(entropy<<3));if(entropy<0)entropy=-entropy;return entropy%NUM_TETROMINOS;
//     }

int can_place(Tetromino *piece, int newX, int newY, int newRot) {
    int r, c;
    for (r = 0; r < 4; r++) {
        for (c = 0; c < 4; c++) {
            uint8 cell = tetrominoes[piece->shape][newRot][r][c];
            if (cell) {
                int gridX = newX + c;
                int gridY = newY + r;
                if (gridX < 0 || gridX >= GRID_COLS || gridY < 0 || gridY >= GRID_ROWS)
                    return 0;
                if (grid[gridY][gridX] != EMPTY_CELL)
                    return 0;
            }
        }
    }
    return 1;
}

void merge_piece(Tetromino *piece) {
    int r, c;
    for (r = 0; r < 4; r++) {
        for (c = 0; c < 4; c++) {
            if (tetrominoes[piece->shape][piece->rotation][r][c]) {
                int gridX = piece->x + c;
                int gridY = piece->y + r;
                if (gridX >= 0 && gridX < GRID_COLS && gridY >= 0 && gridY < GRID_ROWS)
                    grid[gridY][gridX] = piece->shape;
            }
        }
    }
}

int clear_lines(void) {
    int row, col, dst, cleared = 0;
    for (row = 0; row < GRID_ROWS; row++) {
        int full = 1;
        for (col = 0; col < GRID_COLS; col++) {
            if (grid[row][col] == EMPTY_CELL) { full = 0; break; }
        }
        if (full) {
            cleared++;
            for (dst = row; dst > 0; dst--) {
                for (col = 0; col < GRID_COLS; col++)
                    grid[dst][col] = grid[dst-1][col];
            }
            for (col = 0; col < GRID_COLS; col++)
                grid[0][col] = EMPTY_CELL;
        }
    }
    return cleared;
}

void spawn_new_piece(void) {
    fallingPiece.shape = ram_rand();
    fallingPiece.rotation = 0;
    fallingPiece.x = (GRID_COLS / 2) - 2;
    fallingPiece.y = 0;
    fallingPiece.tspin = 0;
    if (!can_place(&fallingPiece, fallingPiece.x, fallingPiece.y, fallingPiece.rotation)) {
        int r, c;
        for (r = 0; r < GRID_ROWS; r++)
            for (c = 0; c < GRID_COLS; c++)
                grid[r][c] = EMPTY_CELL;
        score = 0;
        level = 1;
        totalLinesCleared = 0;
        combo = 0;
        lastClearWasDifficult = 0;
    }
}

void update_score_display(void) {
    char buf[32], scoreStr[16], levelStr[16];
    itoa(scoreStr, 10, score);
    itoa(levelStr, 10, level);
    strcpy(buf, "Score: ");
    strcat(buf, scoreStr);
    strcat(buf, " Lvl: ");
    strcat(buf, levelStr);
    int len = strlen(buf);
    int x = VGA_WIDTH - len;
    int y = 0;
    uint16 *vga = (uint16*)VGA_ADDRESS;
    int i = 0;
    while (buf[i]) {
        int pos = y * VGA_WIDTH + (x + i);
        vga[pos] = vga_item_entry(buf[i], COLOR_CYAN, COLOR_BLACK);
        i++;
    }
    while (i < 20) {
        int pos = y * VGA_WIDTH + (x + i);
        vga[pos] = vga_item_entry(' ', COLOR_CYAN, COLOR_BLACK);
        i++;
    }
}

uint32 get_fall_delay(void) {
    // Using a classic NES Tetris style delay:
    // Level 0: 48 frames, then 43, 38, 33, 28, 23, 18, 13, 8, 6 and levels 10+ use 6.
    if (level < 10)
        return 48 - level * 5;
    else
        return 6;
}

void draw_grid_buffer(void) {
    int r, c;
    uint16 buffer[GRID_ROWS][GRID_BUFFER_COLS];
    for (r = 0; r < GRID_ROWS; r++) {
        for (c = 0; c < GRID_COLS; c++) {
            int cellIndex = c * CELL_WIDTH;
            if (grid[r][c] != EMPTY_CELL) {
                VGA_COLOR_TYPE col = tetrominoColors[ grid[r][c] ];
                buffer[r][cellIndex]   = vga_item_entry('[', col, COLOR_BLACK);
                buffer[r][cellIndex+1] = vga_item_entry(']', col, COLOR_BLACK);
            } else {
                buffer[r][cellIndex]   = vga_item_entry(' ', COLOR_WHITE, COLOR_BLACK);
                buffer[r][cellIndex+1] = vga_item_entry('.', COLOR_WHITE, COLOR_BLACK);
            }
        }
    }
    int tr, tc;
    for (tr = 0; tr < 4; tr++) {
        for (tc = 0; tc < 4; tc++) {
            uint8 cell = tetrominoes[fallingPiece.shape][fallingPiece.rotation][tr][tc];
            if (cell) {
                int gx = fallingPiece.x + tc;
                int gy = fallingPiece.y + tr;
                if (gx >= 0 && gx < GRID_COLS && gy >= 0 && gy < GRID_ROWS) {
                    int cellIndex = gx * CELL_WIDTH;
                    VGA_COLOR_TYPE col = tetrominoColors[fallingPiece.shape];
                    buffer[gy][cellIndex]   = vga_item_entry('[', col, COLOR_BLACK);
                    buffer[gy][cellIndex+1] = vga_item_entry(']', col, COLOR_BLACK);
                }
            }
        }
    }
    uint16 *vga = (uint16*)VGA_ADDRESS;
    for (r = 0; r < GRID_ROWS; r++) {
        int dest_row = GRID_Y_OFFSET + r;
        int dest_index = dest_row * VGA_WIDTH + GRID_X_OFFSET;
        memcpy(&vga[dest_index], buffer[r], GRID_BUFFER_COLS * sizeof(uint16));
    }
}

void update_timer_display(const char* sec_str) {
    char timerStr[32];
    strcpy(timerStr, "Time: ");
    strcat(timerStr, sec_str);
    int len = strlen(timerStr);
    int x = VGA_WIDTH - len;
    int y = 1;
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
    spawn_new_piece();
    lastFallTick = timer_ticks;
    update_score_display();
    while (1) {
        sc = kb_poll_scancode();
        if (sc != 0) {
            if (sc == 0x4B) {
                if (can_place(&fallingPiece, fallingPiece.x - 1, fallingPiece.y, fallingPiece.rotation))
                    fallingPiece.x--;
            } else if (sc == 0x4D) {
                if (can_place(&fallingPiece, fallingPiece.x + 1, fallingPiece.y, fallingPiece.rotation))
                    fallingPiece.x++;
            } else if (sc == 0x48) {
                int newRot = (fallingPiece.rotation + 1) % 4;
                if (fallingPiece.shape == 2) {
                    if (can_place(&fallingPiece, fallingPiece.x, fallingPiece.y, newRot)) {
                        fallingPiece.rotation = newRot;
                        fallingPiece.tspin = (fallingPiece.x < 1 || fallingPiece.x > GRID_COLS-3) ? 1 : 2;
                    }
                } else {
                    if (can_place(&fallingPiece, fallingPiece.x, fallingPiece.y, newRot))
                        fallingPiece.rotation = newRot;
                }
            } else if (sc == 0x50) {
                if (can_place(&fallingPiece, fallingPiece.x, fallingPiece.y + 1, fallingPiece.rotation)) {
                    fallingPiece.y++;
                    score += 1;
                    update_score_display();
                }
            } else if (sc == 0x39) {
                int dropDistance = 0;
                while (can_place(&fallingPiece, fallingPiece.x, fallingPiece.y + 1, fallingPiece.rotation)) {
                    fallingPiece.y++;
                    dropDistance++;
                }
                score += dropDistance * 2;
                update_score_display();
                merge_piece(&fallingPiece);
                int lines = clear_lines();
                if (lines > 0) {
                    totalLinesCleared += lines;
                    int baseScore = 0;
                    int difficult = 0;
                    if (fallingPiece.shape == 2 && fallingPiece.tspin) {
                        if (lines == 1) { baseScore = (fallingPiece.tspin == 1) ? 200 : 800; difficult = 1; }
                        else if (lines == 2) { baseScore = (fallingPiece.tspin == 1) ? 400 : 1200; difficult = 1; }
                        else if (lines == 3) { baseScore = 1600; difficult = 1; }
                    } else {
                        if (lines == 1) { baseScore = 100; }
                        else if (lines == 2) { baseScore = 300; }
                        else if (lines == 3) { baseScore = 500; }
                        else if (lines == 4) { baseScore = 800; difficult = 1; }
                    }
                    if (lastClearWasDifficult && difficult)
                        baseScore = (baseScore * 3) / 2;
                    lastClearWasDifficult = difficult;
                    if (lines > 0)
                        combo++;
                    else
                        combo = 0;
                    baseScore += 50 * combo;
                    score += baseScore * level;
                    if (totalLinesCleared / 10 > level - 1)
                        level = (totalLinesCleared / 10) + 1;
                    update_score_display();
                } else {
                    combo = 0;
                }
                spawn_new_piece();
                fallingPiece.tspin = 0;
                lastFallTick = timer_ticks;
                continue;
            }
        }
        if (timer_ticks - lastFallTick >= get_fall_delay()) {
            lastFallTick = timer_ticks;
            if (can_place(&fallingPiece, fallingPiece.x, fallingPiece.y + 1, fallingPiece.rotation))
                fallingPiece.y++;
            else {
                merge_piece(&fallingPiece);
                int lines = clear_lines();
                if (lines > 0) {
                    totalLinesCleared += lines;
                    int baseScore = 0;
                    int difficult = 0;
                    if (fallingPiece.shape == 2 && fallingPiece.tspin) {
                        if (lines == 1) { baseScore = (fallingPiece.tspin == 1) ? 200 : 800; difficult = 1; }
                        else if (lines == 2) { baseScore = (fallingPiece.tspin == 1) ? 400 : 1200; difficult = 1; }
                        else if (lines == 3) { baseScore = 1600; difficult = 1; }
                    } else {
                        if (lines == 1) { baseScore = 100; }
                        else if (lines == 2) { baseScore = 300; }
                        else if (lines == 3) { baseScore = 500; }
                        else if (lines == 4) { baseScore = 800; difficult = 1; }
                    }
                    if (lastClearWasDifficult && difficult)
                        baseScore = (baseScore * 3) / 2;
                    lastClearWasDifficult = difficult;
                    if (lines > 0)
                        combo++;
                    else
                        combo = 0;
                    baseScore += 50 * combo;
                    score += baseScore * level;
                    if (totalLinesCleared / 10 > level - 1)
                        level = (totalLinesCleared / 10) + 1;
                    update_score_display();
                } else {
                    combo = 0;
                }
                spawn_new_piece();
                fallingPiece.tspin = 0;
            }
        }
        draw_grid_buffer();
    }
}
