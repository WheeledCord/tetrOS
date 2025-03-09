#include "kernel.h"

const int grid_width = 10;
const int grid_height = 20;
const unsigned int slide_time = 24;

int grid_sel_x,grid_sel_y,current_shape,next_shape,held_shape,score,lines_cleared,lvl,current_rot;
float fall_speed;
bool should_slide = false;
bool held_this_turn;
bool should_stamp = false;
bool highlight_bg = true;
int grid_mode = 0;
bool show_timer = true;
bool show_keycodes = false;
bool konami = false;
int konami_inputs[11][2] = {
    {72,200}, // up
    {72,200}, // up
    {80,208}, // down
    {80,208}, // down
    {75,203}, // left
    {77,205}, // right
    {75,203}, // left
    {77,205}, // right
    {48,176}, // B
    {30,158}, // A
    {28,156} // enter
};
int konami_progress = 0;

char grid_modes[][2][2] = {
    {{0xC4,0xC5},{GRAY,GRAY}}, //  ─┼
    {" .",{GRAY,GRAY}},
    {{' ',0xDC},{BLACK,GRAY}}
};

volatile unsigned int tick_count = 0;
volatile unsigned int fall_tick_count = 0;
volatile unsigned int pause_tick_count = 0;
volatile unsigned int konami_ticks = 0;

void outb(unsigned short port, unsigned char data) {
    asm volatile ("outb %0, %1" : : "a"(data), "Nd"(port));
}
unsigned char inb(unsigned short port) {
    unsigned char ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}
void pic_remap() { // remap pics to avoid irq conflicts
    unsigned char a1 = inb(0x21);
    unsigned char a2 = inb(0xA1);
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, a1);
    outb(0xA1, a2);
}
struct idt_entry {
    unsigned short base_low;
    unsigned short sel;
    unsigned char always0;
    unsigned char flags;
    unsigned short base_high;
} __attribute__((packed));
struct idt_ptr {
    unsigned short limit;
    unsigned int base;
} __attribute__((packed));
struct idt_entry idt[256];
struct idt_ptr idtp;
void idt_set_gate(unsigned char num, unsigned int base, unsigned short sel, unsigned char flags) {
    idt[num].base_low  = base & 0xFFFF;
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].sel       = sel;
    idt[num].always0   = 0;
    idt[num].flags     = flags;
}
extern void idt_load(unsigned int);
void k_install_idt() {
    idtp.limit = sizeof(struct idt_entry) * 256 - 1;
    idtp.base  = (unsigned int)&idt;
    extern void timer_handler_stub();
    extern void keyboard_handler_stub();
    idt_set_gate(32, (unsigned int)timer_handler_stub, 0x08, 0x8E);
    idt_set_gate(33, (unsigned int)keyboard_handler_stub, 0x08, 0x8E);
    idt_load((unsigned int)&idtp);
}
void itoa(int value, char *str, int base) {
    char *rc = str, *ptr = str, *low;
    if (base < 2 || base > 36) {
        *str = '\0';
        return;
    }
    if (value < 0 && base == 10) {
        *ptr++ = '-';
        value = -value;
    }
    int num = value;
    do {
        *ptr++ = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[num % base];
        num /= base;
    } while (num);
    *ptr = '\0';
    low = rc;
    if (*rc == '-') {
        low++;
    }
    char *high = ptr - 1;
    while (low < high) {
        char temp = *low;
        *low++ = *high;
        *high-- = temp;
    }
}

void kernel_loop() {
    while (1) {
        asm volatile ("hlt");
    }
}

void set_vga_palette(int color_index, int r, int g, int b) {
    outb(0x3C8, color_index); // Select the color index to modify
    outb(0x3C9, r & 0x3F);    // Set red component (0-63)
    outb(0x3C9, g & 0x3F);    // Set green component (0-63)
    outb(0x3C9, b & 0x3F);    // Set blue component (0-63)
}
void set_custom_palette() {
    set_vga_palette(0,  0,  0,  0);   // BLACK
    set_vga_palette(1,  63, 63, 63);  // WHITE
    set_vga_palette(2,  26, 26, 26);  // GRAY
    set_vga_palette(3,  0,  55, 55);  // CYAN
    set_vga_palette(4,  0,  0,  63);  // BLUE
    set_vga_palette(5,  63, 32, 0);   // ORANGE
    set_vga_palette(7,  0,  50, 0);   // GREEN
    set_vga_palette(20, 50, 50, 0);   // YELLOW (using index 20)
    set_vga_palette(56, 63, 0,  63);  // PURPLE (using index 56)
    set_vga_palette(57, 63, 0,  0);   // RED (using index 57)
    set_vga_palette(58, 50, 63, 63);  // LIGHT_CYAN (using index 58)
    set_vga_palette(59, 63, 48, 32);  // LIGHT_ORANGE (using index 59)
    set_vga_palette(60, 63, 63, 50);  // LIGHT_YELLOW (using index 60)
    set_vga_palette(61, 50, 63, 50);  // LIGHT_GREEN (using index 61)
    set_vga_palette(62, 63, 48, 63);  // LIGHT_PURPLE (using index 62)
    set_vga_palette(63, 63, 32, 32);  // LIGHT_RED (using index 63)
}

void clear_screen() {
    char *vidmem = (char *)0xb8000;
    unsigned int i = 0;
    while (i < (80 * 25 * 2)) {
        vidmem[i++] = ' ';
        vidmem[i++] = WHITE;
    }
}
void disable_cursor() {
    outb(0x3D4, 0x0A);
    outb(0x3D5, 0x20);
}
unsigned int print(char *message, char color, unsigned int line) {
    char *vidmem = (char *)0xb8000;
    unsigned int i = line * 80 * 2;
    while (*message) {
        if (*message == '\n') {
            line++;
            i = line * 80 * 2;
            message++;
        } else {
            vidmem[i++] = *message++;
            vidmem[i++] = color;
        }
    }
    return 1;
}
unsigned int print_at(char *message, char color, unsigned int line, unsigned int pos) {
    char *vidmem = (char *)0xb8000;
    unsigned int i = line * 80 * 2 + (2*pos);
    while (*message) {
        if (*message == '\n') {
            line++;
            i = line * 80 * 2;
            message++;
        } else {
            vidmem[i++] = *message++;
            vidmem[i++] = color;
        }
    }
    return 1;
}
unsigned int print_cols(char *message, char *color, unsigned int line) {
    char *vidmem = (char *)0xb8000;
    unsigned int i = line * 80 * 2;
    while (*message || *color) {
        if (*message == '\n') {
            line++;
            i = line * 80 * 2;
            message++;
            color++;
        } else {
            vidmem[i++] = *message++;
            vidmem[i++] = *color++;
        }
    }
    return 1;
}
unsigned int print_cols_at(char *message, char *color, unsigned int line, unsigned int pos) {
    char *vidmem = (char *)0xb8000;
    unsigned int i = line * 80 * 2 + (2*pos);
    while (*message || *color) {
        if (*message == '\n') {
            line++;
            i = line * 80 * 2;
            message++;
            color++;
        } else {
            vidmem[i++] = *message++;
            vidmem[i++] = *color++;
        }
    }
    return 1;
}

int round(float n) {
    return (int)(n * 10 + 0.5) / 10.;
}
int floor(float n) {
    return (int) n;
}

#define RAND_MAX = 2147483647
static unsigned long rand_state = 6783489;
void srand(unsigned long seed) {
    rand_state = seed;
}
long rand() {
    rand_state = (rand_state * 1103515245 + 12345) % 2147483648;
    return rand_state;
}
int randInt(int max) {
    return rand() % max;
}

// Shapes as defined previously
int shape_o[4][4][4] = {
    { {1,1,0,0}, {2,1,0,0}, {0,0,0,0}, {0,0,0,0} },
    { {1,1,0,0}, {2,1,0,0}, {0,0,0,0}, {0,0,0,0} },
    { {1,1,0,0}, {2,1,0,0}, {0,0,0,0}, {0,0,0,0} },
    { {1,1,0,0}, {2,1,0,0}, {0,0,0,0}, {0,0,0,0} }
};
int shape_i[4][4][4] = {
    { {1,2,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0} },
    { {1,0,0,0}, {2,0,0,0}, {1,0,0,0}, {1,0,0,0} },
    { {1,1,2,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0} },
    { {1,0,0,0}, {1,0,0,0}, {2,0,0,0}, {1,0,0,0} }
};
int shape_s[4][4][4] = {
    { {0,1,1,0}, {1,2,0,0}, {0,0,0,0}, {0,0,0,0} },
    { {1,0,0,0}, {2,1,0,0}, {0,1,0,0}, {0,0,0,0} },
    { {0,2,1,0}, {1,1,0,0}, {0,0,0,0}, {0,0,0,0} },
    { {1,0,0,0}, {1,2,0,0}, {0,1,0,0}, {0,0,0,0} }
};
int shape_z[4][4][4] = {
    { {1,1,0,0}, {0,2,1,0}, {0,0,0,0}, {0,0,0,0} },
    { {0,1,0,0}, {2,1,0,0}, {1,0,0,0}, {0,0,0,0} },
    { {1,2,0,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0} },
    { {0,1,0,0}, {1,2,0,0}, {1,0,0,0}, {0,0,0,0} }
};
int shape_l[4][4][4] = {
    { {0,0,1,0}, {1,2,1,0}, {0,0,0,0}, {0,0,0,0} },
    { {1,0,0,0}, {2,0,0,0}, {1,1,0,0}, {0,0,0,0} },
    { {1,2,1,0}, {1,0,0,0}, {0,0,0,0}, {0,0,0,0} },
    { {1,1,0,0}, {0,2,0,0}, {0,1,0,0}, {0,0,0,0} }
};
int shape_j[4][4][4] = {
    { {1,0,0,0}, {1,2,1,0}, {0,0,0,0}, {0,0,0,0} },
    { {1,1,0,0}, {2,0,0,0}, {1,0,0,0}, {0,0,0,0} },
    { {1,2,1,0}, {0,0,1,0}, {0,0,0,0}, {0,0,0,0} },
    { {0,1,0,0}, {0,2,0,0}, {1,1,0,0}, {0,0,0,0} }
};
int shape_t[4][4][4] = {
    { {0,1,0,0}, {1,2,1,0}, {0,0,0,0}, {0,0,0,0} },
    { {1,0,0,0}, {2,1,0,0}, {1,0,0,0}, {0,0,0,0} },
    { {1,2,1,0}, {0,1,0,0}, {0,0,0,0}, {0,0,0,0} },
    { {0,1,0,0}, {1,2,0,0}, {0,1,0,0}, {0,0,0,0} }
};

int bag[7] = {-1,-1,-1,-1,-1,-1,-1};
void shuffle_bag() {   
    srand(rand_state/(tick_count+56123)); 
    if (7 > 1) {
        for (int i = 7 - 1; i > 0; i--) {
            unsigned int j = randInt(7);
            unsigned int t = bag[j];
            bag[j] = bag[i];
            bag[i] = t;
        }
    }
}
unsigned int get_from_bag() {
    bool bag_empty = true;
    for (int i = 0; i < 7; i++) {
        if (bag[i] != -1) {
            bag_empty = false;
            break;
        }
    }
    if (bag_empty) {
        for (int i = 0; i < 7; i++) {
            bag[i] = i;
        }
        shuffle_bag();
    }
    for (int i = 0; i < 7; i++) {
        if (bag[i] != -1) {
            unsigned int item = bag[i];
            bag[i] = -1;
            return item;
        }
    }
    return 0;
}

int tilemap[20][10];
bool full_lines[20];
bool full_konami_lines[10];

void reset() {
    score = 0;
    lines_cleared = 0;
    lvl = 0;
    fall_speed = 14.4;

    current_shape = get_from_bag();
    next_shape = get_from_bag();
    held_shape = -1;
    held_this_turn = false;
    grid_sel_x = 4;
    grid_sel_y = 1;
    current_rot = 0;

    tick_count = 0;
    fall_tick_count = 0;
    should_slide = false;

    konami = false;
    konami_progress = 0;

    for (int y = 0; y < grid_height; y++) {
        for (int x = 0; x < grid_width; x++) {
            tilemap[y][x] = 0;
        }
    }
    clear_screen();
    print("Welcome to TetrOS!", WHITE, 0);
    init_timer();
    draw_grid();
}

int shape_points[4][2];
void get_shape_points(int shape, bool rotate) {
    int (*rot)[4];
    if (rotate) {
        switch(shape) {
            case 0: rot = shape_o[current_rot]; break;
            case 1: rot = shape_i[current_rot]; break;
            case 2: rot = shape_s[current_rot]; break;
            case 3: rot = shape_z[current_rot]; break;
            case 4: rot = shape_l[current_rot]; break;
            case 5: rot = shape_j[current_rot]; break;
            case 6: rot = shape_t[current_rot]; break;
            default: rot = shape_o[current_rot]; break;
        }
    } else {
        switch(shape) {
            case 0: rot = shape_o[0]; break;
            case 1: rot = shape_i[0]; break;
            case 2: rot = shape_s[0]; break;
            case 3: rot = shape_z[0]; break;
            case 4: rot = shape_l[0]; break;
            case 5: rot = shape_j[0]; break;
            case 6: rot = shape_t[0]; break;
            default: rot = shape_o[0]; break;
        }
    }
    int center_x = 0, center_y = 0;
    int points[4][2] = { {0,0}, {0,0}, {0,0}, {0,0} };
    unsigned int i = 0;
    for (int y = 0; y < 4; y++) {
        int *row = rot[y];
        for (int x = 0; x < 4; x++) {
            int ch = row[x];
            if (ch == 1 || ch == 2) {
                points[i][0] = x;
                points[i][1] = y;
                i++;
            }
            if (ch == 2) {
                center_x = x;
                center_y = y;
            }
        }
    }
    for (i = 0; i < 4; i++) {
        shape_points[i][0] = points[i][0] - center_x;
        shape_points[i][1] = points[i][1] - center_y;
    }
}
bool illegality(unsigned int rule) {
    get_shape_points(current_shape,true);
    bool legality[5];
    for (int i = 0; i < 5; i++) {
        legality[i] = true;
    }
    for (int i = 0; i < 4; i++) {
        int x = shape_points[i][0]+grid_sel_x;
        int y = shape_points[i][1]+grid_sel_y;
        if (tilemap[y][x] != 0) {
            legality[0] = false;
        }
        if (x < 0) {
            legality[1] = false;
        }
        if (x >= grid_width) {
            legality[2] = false;
        }
        if (y < 0) {
            legality[3] = false;
        }
        if (y >= grid_height) {
            legality[4] = false;
        }
    }
    for (int i = 0; i < 5; i++) {
        legality[i] = !legality[i];
    }
    return legality[rule];
}
bool any_illegality() {
    for (int i = 0; i < 5; i++) {
        if (illegality(i)) {
            return true;
        }
    }
    return false;
}

//
// draw_grid centers the board and draws an outline using double-line box drawing characters:
// Top border: ╔ (0xC9), ═ (0xCD), ╗ (0xBB)
// Side borders: ║ (0xBA)
// Bottom border: ╚ (0xC8), ═ (0xCD), ╝ (0xBC)
//
void draw_grid() {
    unsigned int left_margin = (80 - (grid_width * 2 + 2)) / 2;
    unsigned int top_margin = 1+(27 - (grid_height + 2)) / 2;
    
    char line[256];
    char col_line[256];
    char buffer[16];
    unsigned int pos, i;
    
    pos = 0;
    if (highlight_bg) {
        line[pos] = 0xC9;  // ╔
    } else {
        line[pos] = '+';
    }
    col_line[pos] = WHITE;
    pos++;
    for (i = 0; i < grid_width * 2; i++) {
        if (highlight_bg) {
            line[pos] = 0xCD;  // ═
        } else {
            line[pos] = '-';
        }
        col_line[pos] = WHITE;
        pos++;
    }
    if (highlight_bg) {
        line[pos] = 0xBB;  // ╗
    } else {
        line[pos] = '+';
    }
    col_line[pos] = WHITE;
    pos++;
    line[pos] = '\0';
    col_line[pos] = '\0';
    print_cols_at(line, col_line, top_margin, left_margin);
    
    // Draw Level
    itoa(lvl, buffer, 10);
    print_at("LEVEL:", WHITE, 0, left_margin);
    print_at(buffer, WHITE, 0, left_margin+7);
    
    // Draw Lines cleared
    itoa(lines_cleared, buffer, 10);
    print_at("LINES:", WHITE, 1, left_margin);
    print_at(buffer, WHITE, 1, left_margin+7);

    // Draw Score
    itoa(score, buffer, 10);
    print_at("SCORE:", WHITE, 2, left_margin);
    print_at(buffer, WHITE, 2, left_margin+7);

    // Draw Next Shape Box
    pos = 0;
    if (highlight_bg) {
        line[pos++] = 0xC9;  // ╔
    } else {
        line[pos++] = '+';
    }
    for (i = 0; i < 8; i++) {
        if (highlight_bg) {
            line[pos++] = 0xCD;  // ═
        } else {
            line[pos++] = '-';
        }
    }
    if (highlight_bg) {
        line[pos++] = 0xBB;  // ╗
    } else {
        line[pos++] = '+';
    }
    line[pos] = '\0';
    print_at(line, WHITE, top_margin, left_margin+grid_width*2+2+6);
    pos = 0;
    if (highlight_bg) {
        line[pos++] = 0xBA;  // ║
    } else {
        line[pos++] = '|';
    }
    for (i = 0; i < 8; i++) {
        line[pos++] = "  NEXT: "[i];
    }
    if (highlight_bg) {
        line[pos++] = 0xBA;  // ║
    } else {
        line[pos++] = '|';
    }
    line[pos] = '\0';
    print_at(line, WHITE, top_margin+1, left_margin+grid_width*2+2+6);
    pos = 0;
    if (highlight_bg) {
        line[pos++] = 0xCC;  // ╠ 
    } else {
        line[pos++] = '+';
    }
    for (i = 0; i < 8; i++) {
        if (highlight_bg) {
            line[pos++] = 0xCD;  // ═
        } else {
            line[pos++] = '-';
        }
    }
    if (highlight_bg) {
        line[pos++] = 0xB9;  // ╣
    } else {
        line[pos++] = '+';
    }
    line[pos] = '\0';
    print_at(line, WHITE, top_margin+2, left_margin+grid_width*2+2+6);
    pos = 0;
    if (highlight_bg) {
        line[pos] = 0xBA;  // ║
    } else {
        line[pos] = '|';
    }
    col_line[pos++] = WHITE;
    for (i = 0; i < 4; i++) {
        if (highlight_bg) {
            line[pos] = grid_modes[grid_mode][0][0];
            col_line[pos++] = grid_modes[grid_mode][1][0];
            line[pos] = grid_modes[grid_mode][0][1];
            col_line[pos++] = grid_modes[grid_mode][1][1];
        } else {
            line[pos] = grid_modes[1][0][0];
            col_line[pos++] = grid_modes[1][1][0];
            line[pos] = grid_modes[1][0][1];
            col_line[pos++] = grid_modes[1][1][1];
        }
    }
    if (highlight_bg) {
        line[pos] = 0xBA;  // ║
    } else {
        line[pos] = '|';
    }
    col_line[pos++] = WHITE;
    col_line[pos] = '\0';
    print_cols_at(line, col_line, top_margin+3, left_margin+grid_width*2+2+6);
    print_cols_at(line, col_line, top_margin+4, left_margin+grid_width*2+2+6);
    print_cols_at(line, col_line, top_margin+5, left_margin+grid_width*2+2+6);
    print_cols_at(line, col_line, top_margin+6, left_margin+grid_width*2+2+6);
    line[pos] = '\0';
    pos = 0;
    if (highlight_bg) {
        line[pos++] = 0xC8;  // ╚
    } else {
        line[pos++] = '+';
    }
    for (i = 0; i < 8; i++) {
        if (highlight_bg) {
            line[pos++] = 0xCD;  // ═
        } else {
            line[pos++] = '-';
        }
    }
    if (highlight_bg) {
        line[pos++] = 0xBC;  // ╝
    } else {
        line[pos++] = '+';
    }
    line[pos] = '\0';
    print_at(line, WHITE, top_margin+7, left_margin+grid_width*2+2+6);

    // Draw Held Shape Box
    pos = 0;
    if (highlight_bg) {
        line[pos++] = 0xC9;  // ╔
    } else {
        line[pos++] = '+';
    }
    for (i = 0; i < 8; i++) {
        if (highlight_bg) {
            line[pos++] = 0xCD;  // ═
        } else {
            line[pos++] = '-';
        }
    }
    if (highlight_bg) {
        line[pos++] = 0xBB;  // ╗
    } else {
        line[pos++] = '+';
    }
    line[pos] = '\0';
    print_at(line, WHITE, top_margin+10, left_margin+grid_width*2+2+6);
    pos = 0;
    if (highlight_bg) {
        line[pos++] = 0xBA;  // ║
    } else {
        line[pos++] = '|';
    }
    for (i = 0; i < 8; i++) {
        line[pos++] = "  HELD: "[i];
    }
    if (highlight_bg) {
        line[pos++] = 0xBA;  // ║
    } else {
        line[pos++] = '|';
    }
    line[pos] = '\0';
    print_at(line, WHITE, top_margin+11, left_margin+grid_width*2+2+6);
    pos = 0;
    if (highlight_bg) {
        line[pos++] = 0xCC;  // ╠ 
    } else {
        line[pos++] = '+';
    }
    for (i = 0; i < 8; i++) {
        if (highlight_bg) {
            line[pos++] = 0xCD;  // ═
        } else {
            line[pos++] = '-';
        }
    }
    if (highlight_bg) {
        line[pos++] = 0xB9;  // ╣
    } else {
        line[pos++] = '+';
    }
    line[pos] = '\0';
    print_at(line, WHITE, top_margin+12, left_margin+grid_width*2+2+6);
    pos = 0;
    if (highlight_bg) {
        line[pos] = 0xBA;  // ║
    } else {
        line[pos] = '|';
    }
    col_line[pos++] = WHITE;
    for (i = 0; i < 4; i++) {
        if (highlight_bg) {
            line[pos] = grid_modes[grid_mode][0][0];
            col_line[pos++] = grid_modes[grid_mode][1][0];
            line[pos] = grid_modes[grid_mode][0][1];
            col_line[pos++] = grid_modes[grid_mode][1][1];
        } else {
            line[pos] = grid_modes[1][0][0];
            col_line[pos++] = grid_modes[1][1][0];
            line[pos] = grid_modes[1][0][1];
            col_line[pos++] = grid_modes[1][1][1];
        }
    }
    if (highlight_bg) {
        line[pos] = 0xBA;  // ║
    } else {
        line[pos] = '|';
    }
    col_line[pos++] = WHITE;
    col_line[pos] = '\0';
    print_cols_at(line, col_line, top_margin+13, left_margin+grid_width*2+2+6);
    print_cols_at(line, col_line, top_margin+14, left_margin+grid_width*2+2+6);
    print_cols_at(line, col_line, top_margin+15, left_margin+grid_width*2+2+6);
    print_cols_at(line, col_line, top_margin+16, left_margin+grid_width*2+2+6);
    line[pos] = '\0';
    pos = 0;
    if (highlight_bg) {
        line[pos++] = 0xC8;  // ╚
    } else {
        line[pos++] = '+';
    }
    for (i = 0; i < 8; i++) {
        if (highlight_bg) {
            line[pos++] = 0xCD;  // ═
        } else {
            line[pos++] = '-';
        }
    }
    if (highlight_bg) {
        line[pos++] = 0xBC;  // ╝
    } else {
        line[pos++] = '+';
    }
    line[pos] = '\0';
    print_at(line, WHITE, top_margin+17, left_margin+grid_width*2+2+6);

    // Draw the next shape
    get_shape_points(next_shape,false);
    unsigned int top_offset = 0;
    unsigned int left_offset = 0;
    char cell_color = WHITE;
    char cell_inner = WHITE;
    switch(next_shape) {
        case 0: cell_color = YELLOW; cell_inner = LIGHT_YELLOW; top_offset = 1; left_offset = 2; break;
        case 1: cell_color = CYAN; cell_inner = LIGHT_CYAN; top_offset = 2; left_offset = 0; break;
        case 2: cell_color = GREEN; cell_inner = LIGHT_GREEN; top_offset = 1; left_offset = 1; break;
        case 3: cell_color = RED; cell_inner = LIGHT_RED; top_offset = 1; left_offset = 1; break;
        case 4: cell_color = ORANGE; cell_inner = LIGHT_ORANGE; top_offset = 1; left_offset = 1; break;
        case 5: cell_color = BLUE; cell_inner = LIGHT_BLUE; top_offset = 1; left_offset = 1; break;
        case 6: cell_color = PURPLE; cell_inner = LIGHT_PURPLE; top_offset = 1; left_offset = 1; break;
    }
    unsigned char attr;
    if (highlight_bg) {
        attr = (cell_color << 4) | cell_inner;
    } else {
        attr = cell_color;
    }
    int min_x = 10;
    int min_y = 10;
    for(i = 0; i < 4; i++) {
        int x = shape_points[i][0];
        int y = shape_points[i][1];
        if (x < min_x) min_x = x;
        if (y < min_y) min_y = y;
    }
    for(i = 0; i < 4; i++) {
        int x = shape_points[i][0]-min_x;
        int y = shape_points[i][1]-min_y;
        print_at("[]", attr, top_margin+3+top_offset+y, left_margin+grid_width*2+2+6+1+left_offset+2*x);
    }

    // Draw the held shape
    if (held_shape != -1) {
        get_shape_points(held_shape,false);
        unsigned int top_offset = 0;
        unsigned int left_offset = 0;
        char cell_color = WHITE;
        char cell_inner = WHITE;
        switch(held_shape) {
            case 0: cell_color = YELLOW; cell_inner = LIGHT_YELLOW; top_offset = 1; left_offset = 2; break;
            case 1: cell_color = CYAN; cell_inner = LIGHT_CYAN; top_offset = 0; left_offset = 2; break;
            case 2: cell_color = GREEN; cell_inner = LIGHT_GREEN; top_offset = 1; left_offset = 1; break;
            case 3: cell_color = RED; cell_inner = LIGHT_RED; top_offset = 1; left_offset = 1; break;
            case 4: cell_color = ORANGE; cell_inner = LIGHT_ORANGE; top_offset = 1; left_offset = 2; break;
            case 5: cell_color = BLUE; cell_inner = LIGHT_BLUE; top_offset = 1; left_offset = 2; break;
            case 6: cell_color = PURPLE; cell_inner = LIGHT_PURPLE; top_offset = 1; left_offset = 1; break;
        }
        unsigned char attr;
        if (highlight_bg) {
            attr = (cell_color << 4) | cell_inner;
        } else {
            attr = cell_color;
        }
        int min_x = 10;
        int min_y = 10;
        for(i = 0; i < 4; i++) {
            int x = shape_points[i][0];
            int y = shape_points[i][1];
            if (x < min_x) min_x = x;
            if (y < min_y) min_y = y;
        }
        for(i = 0; i < 4; i++) {
            int x = shape_points[i][0]-min_x;
            int y = shape_points[i][1]-min_y;
            print_at("[]", attr, top_margin+13+top_offset+y, left_margin+grid_width*2+2+6+1+left_offset+2*x);
        }
    }
    
    get_shape_points(current_shape,true);

    // Ghost stuff
    unsigned int temp;
    unsigned int ghost_y;
    if (konami) {
        temp = grid_sel_x;
        while(!(illegality(1) || illegality(0))) {
            grid_sel_x--;
        }
        grid_sel_x++;
        ghost_y = grid_sel_x;
        grid_sel_x = temp;
    } else {
        temp = grid_sel_y;
        while(!(illegality(4) || illegality(0))) {
            grid_sel_y++;
        }
        grid_sel_y--;
        ghost_y = grid_sel_y;
        grid_sel_y = temp;
    }

    // Draw grid rows with vertical borders
    for (int r = 0; r < grid_height; r++) {
        pos = 0;
        if (highlight_bg) {
            line[pos] = 0xBA;  // ║
        } else {
            line[pos] = '|';
        }
        col_line[pos] = WHITE;
        pos++;
        for (int c = 0; c < grid_width; c++) {
            bool in_points = false;
            for (i = 0; i < 4; i++) {
                if (c == shape_points[i][0]+grid_sel_x && r == shape_points[i][1]+grid_sel_y) {
                    in_points = true;
                    break;
                }
            }
            bool in_ghost_points = false;
            for (i = 0; i < 4; i++) {
                if (konami) {
                    if ((konami && c == shape_points[i][0]+ghost_y && r == shape_points[i][1]+grid_sel_y)) {
                        in_ghost_points = true;
                        break;
                    }
                } else {
                    if (c == shape_points[i][0]+grid_sel_x && r == shape_points[i][1]+ghost_y) {
                        in_ghost_points = true;
                        break;
                    }
                }
            }
            if (pause_tick_count > 0) {
                in_ghost_points = false;
            }
            if (in_ghost_points || in_points || tilemap[r][c] != 0) {
                char cell_color = BLACK;
                if (!highlight_bg) {
                    cell_color = GRAY;
                }
                char cell_inner = WHITE;
                if (in_points) {
                    switch(current_shape + 1) {
                        case 1: cell_color = YELLOW; cell_inner = LIGHT_YELLOW; break;
                        case 2: cell_color = CYAN; cell_inner = LIGHT_CYAN; break;
                        case 3: cell_color = GREEN; cell_inner = LIGHT_GREEN; break;
                        case 4: cell_color = RED; cell_inner = LIGHT_RED; break;
                        case 5: cell_color = ORANGE; cell_inner = LIGHT_ORANGE; break;
                        case 6: cell_color = BLUE; cell_inner = LIGHT_BLUE; break;
                        case 7: cell_color = PURPLE; cell_inner = LIGHT_PURPLE; break;
                    }
                } else if (tilemap[r][c] != 0) {
                    switch(tilemap[r][c]) {
                        case -1: cell_color = WHITE; cell_inner = WHITE; break;
                        case 1: cell_color = YELLOW; cell_inner = LIGHT_YELLOW; break;
                        case 2: cell_color = CYAN; cell_inner = LIGHT_CYAN; break;
                        case 3: cell_color = GREEN; cell_inner = LIGHT_GREEN; break;
                        case 4: cell_color = RED; cell_inner = LIGHT_RED; break;
                        case 5: cell_color = ORANGE; cell_inner = LIGHT_ORANGE; break;
                        case 6: cell_color = BLUE; cell_inner = LIGHT_BLUE; break;
                        case 7: cell_color = PURPLE; cell_inner = LIGHT_PURPLE; break;
                    }
                }
                unsigned char attr;
                if (highlight_bg && (tilemap[r][c] != 0 || in_points || in_ghost_points)) {
                    attr = (cell_color << 4) | cell_inner;
                } else {
                    attr = cell_color;
                }
                // Draw the cell as "[ ]" with the proper attribute
                if (in_ghost_points) {
                    // line[pos] = 0xB0;
                    line[pos] = '[';
                } else {
                    line[pos] = '[';
                }
                col_line[pos] = attr;
                pos++;
                if (in_ghost_points) {
                    // line[pos] = 0xB0;
                    line[pos] = ']';
                } else {
                    line[pos] = ']';
                }
                col_line[pos] = attr;
                pos++;
            } else {
                // Draw an empty cell
                if (highlight_bg) {
                    line[pos] = grid_modes[grid_mode][0][0];
                    col_line[pos++] = grid_modes[grid_mode][1][0];
                    line[pos] = grid_modes[grid_mode][0][1];
                    col_line[pos++] = grid_modes[grid_mode][1][1];
                } else {
                    line[pos] = grid_modes[1][0][0];
                    col_line[pos++] = grid_modes[1][1][0];
                    line[pos] = grid_modes[1][0][1];
                    col_line[pos++] = grid_modes[1][1][1];
                }
            }
        }
        if (highlight_bg) {
            line[pos] = 0xBA;  // ║
        } else {
            line[pos] = '|';
        }
        col_line[pos] = WHITE;
        pos++;
        line[pos] = '\0';
        col_line[pos] = '\0';
        print_cols_at(line, col_line, top_margin + 1 + r, left_margin);
    }
    
    // Draw bottom border using ╚, ═, ╝
    pos = 0;
    if (highlight_bg) {
        line[pos] = 0xC8;  // ╚
    } else {
        line[pos] = '+';
    }
    col_line[pos] = WHITE;
    pos++;
    for (i = 0; i < grid_width * 2; i++) {
        if (highlight_bg) {
            line[pos] = 0xCD;  // ═
        } else {
            line[pos] = '-';
        }
        col_line[pos] = WHITE;
        pos++;
    }
    if (highlight_bg) {
        line[pos] = 0xBC;  // ╝
    } else {
        line[pos] = '+';
    }
    col_line[pos] = WHITE;
    pos++;
    line[pos] = '\0';
    col_line[pos] = '\0';
    print_cols_at(line, col_line, top_margin + grid_height + 1, left_margin);
    
    // Stamp the piece if should_stamp is true (update tilemap)
    if (should_stamp) {
        should_stamp = false;
        held_this_turn = false;
        bool did_reset = false;
        for (int index = 0; index < 4; index++) {
            int x = shape_points[index][0]+grid_sel_x;
            int y = shape_points[index][1]+grid_sel_y;
            if (x >= 0 && y >= 0 && x < grid_width && y < grid_height) {
                if (tilemap[y][x] != 0) {
                    reset();
                    did_reset = true;
                    break;
                } else {
                    tilemap[y][x] = 1 + current_shape;
                }
            }
        }
        if(!did_reset) {
            unsigned int cleared_count = 0;
            if (konami) {
                for (int x = 0; x < grid_width; x++) {
                    bool line_full = true;
                    for (int y = 0; y < grid_height; y++) {
                        if (tilemap[y][x] == 0) {
                            line_full = false;
                            break;
                        }
                    };
                    if(line_full) {
                        pause_tick_count = 6;
                        cleared_count++;
                    }
                    full_konami_lines[x] = line_full;
                }
            } else {
                for (int y = 0; y < grid_height; y++) {
                    bool line_full = true;
                    for (int x = 0; x < grid_width; x++) {
                        if (tilemap[y][x] == 0) {
                            line_full = false;
                            break;
                        }
                    };
                    if(line_full) {
                        pause_tick_count = 6;
                        cleared_count++;
                    }
                    full_lines[y] = line_full;
                }
            }
            lines_cleared += cleared_count;
            unsigned int temp = lvl;
            lvl = round(lines_cleared / 10);
            if (lvl != temp) {
                if (lvl < 9) {
                    fall_speed -= 1.5;
                } else if (lvl == 9) {
                    fall_speed -= 0.6;
                } else if (lvl == 10 || lvl == 13 || lvl == 16 || lvl == 19 || lvl == 29) {
                    fall_speed -= 0.3;
                }
            }
            score += round(cleared_count / 4)*(1200*(lvl+1)); // tetrises
            score += round((cleared_count % 4) / 3)*(300*(lvl+1)); // triples
            score += round(((cleared_count % 4) % 3) / 2)*(100*(lvl+1)); // doubles
            score += (((cleared_count % 4) % 3) % 2)*(40*(lvl+1)); // singles
        } else {
            print("COLLISON BUG DETECTED!", WHITE, 23);
            print("Retrace your steps to figure it out.", WHITE, 24);
            pause_tick_count = 18*10;
        }
    }
}

void init_timer() {
    print("0s", WHITE, 1);
}
void timer_handler() {
    tick_count++;
    fall_tick_count++;
    if (tick_count % 18 == 0 && show_timer) {
        unsigned int seconds = round(tick_count / 18);
        char buffer[16];
        itoa(seconds, buffer, 10);
        unsigned int i = 0;
        while (buffer[i]) i++;
        buffer[i] = 's';
        buffer[i+1] = '\0';
        print(buffer, WHITE, 1);
    }
    if(konami_ticks > 0) {
        konami_ticks--;
    } else {
        konami_progress = 0;
    }
    if (pause_tick_count == 0) {
        // turn off should_slide if innapropriate
        if (should_slide) {
            if (konami) {
                grid_sel_x--;
            } else {
                grid_sel_y++;
            }
            if(!any_illegality()) {
                should_slide = false;
            }
            if (konami) {
                grid_sel_x++;
            } else {
                grid_sel_y--;
            }
        }

        unsigned int fs = round(fall_speed);
        if (should_slide) {
            fs = slide_time;
        }
        if (fs == 0) {
            fs = 1;
        }
        if (fall_tick_count % fs == 0) {
            if (konami) {
                grid_sel_x--;
            } else {
                grid_sel_y++;
            }
            if(any_illegality()) {
                if (konami) {
                    grid_sel_x++;
                } else {
                    grid_sel_y--;
                }
                fall_tick_count = 0;
                if (should_slide) {
                    should_slide = false;
                    should_stamp = true;
                    draw_grid();
                    current_shape = next_shape;
                    next_shape = get_from_bag();
                    if (konami) {
                        grid_sel_x = 8;
                        grid_sel_y = 9;
                        current_rot = 1;
                    } else {
                        grid_sel_x = 4;
                        grid_sel_y = 1;
                        current_rot = 0;
                    }
                    if(any_illegality()) {
                        reset();
                    }
                } else {
                    should_slide = true;
                }
            }
        }
    } else if (pause_tick_count > 0) {
        if (tick_count % 3 == 0) {
            pause_tick_count --;
        }
        if (pause_tick_count < 0) {
            pause_tick_count = 0;
        }
        if (konami) {
            for (int x = 0; x < grid_width; x++) {
                bool line_full = full_konami_lines[x];
                if(line_full) {
                    if(pause_tick_count == 0) {
                        for (int x2 = x; x2 < (grid_width-1); x2++) {
                            for (int y = 0; y < grid_height; y++) {
                                tilemap[y][x2] = tilemap[y][x2+1];
                            }
                        }
                        full_konami_lines[x] = false;
                    } else {
                        for (int y = 0; y < grid_height; y++) {
                            if (pause_tick_count % 2 == 0) {
                                tilemap[y][x] = 0;
                            } else {
                                tilemap[y][x] = -1;
                            }
                        }
                    }
                }
            }
        } else {
            for (int y = 0; y < grid_height; y++) {
                bool line_full = full_lines[y];
                if(line_full) {
                    if(pause_tick_count == 0) {
                        for (int y2 = y; y2 > 0; y2--) {
                            for (int x = 0; x < grid_width; x++) {
                                tilemap[y2][x] = tilemap[y2-1][x];
                            }
                        }
                        full_lines[y] = false;
                    } else {
                        for (int x = 0; x < grid_width; x++) {
                            if (pause_tick_count % 2 == 0) {
                                tilemap[y][x] = 0;
                            } else {
                                tilemap[y][x] = -1;
                            }
                        }
                    }
                }
            }
        }
    }
    draw_grid();
    outb(0x20, 0x20);
}

void keyboard_handler() {
    unsigned char scancode = inb(0x60);
    srand(rand_state+((int) (scancode-'0')));
    if (pause_tick_count == 0 && scancode != 224) {
        switch(scancode) {
            case 0x4B: // left arrow (move left)
                if (konami) {
                    grid_sel_x--;
                    if (illegality(1) || illegality(0)) {
                        grid_sel_x++;
                    } else {
                        fall_tick_count = 0;
                        score++;
                    }
                } else {
                    grid_sel_x--;
                    if (illegality(1) || illegality(0)) {
                        grid_sel_x++;
                    }
                }
                break;
            case 0x4D: // right arrow (move right)
                if (konami) {
                    current_rot++;
                    if (current_rot >= 4) {
                        current_rot = 0;
                    }
                    if (any_illegality()) {
                        bool should_reset_state = false;
                        unsigned int old_rot = current_rot-1;
                        if (old_rot < 0) {
                            old_rot = 3;
                        }
                        unsigned int reset_state[3] = {grid_sel_x,grid_sel_y,old_rot};
                        if(illegality(1)) {
                            unsigned int moved = 0;
                            while (illegality(1) && moved < 2) {
                                grid_sel_x++;
                            }
                        } else if (illegality(2)) {
                            unsigned int moved = 0;
                            while (illegality(2) && moved < 2) {
                                grid_sel_x--;
                            }
                        }
                        if(any_illegality()) {
                            should_reset_state = true;
                        }
                        if (should_reset_state) {
                            grid_sel_x = reset_state[0];
                            grid_sel_y = reset_state[1];
                            current_rot = reset_state[2];
                        }
                    }
                } else {
                    grid_sel_x++;
                    if (illegality(2) || illegality(0)) {
                        grid_sel_x--;
                    }
                }
                break;
            case 0x50: // down arrow (soft drop)
                if (konami) {
                    grid_sel_y++;
                    if (illegality(4) || illegality(0)) {
                        grid_sel_y--;
                    }
                } else {
                    grid_sel_y++;
                    if (illegality(4) || illegality(0)) {
                        grid_sel_y--;
                    } else {
                        fall_tick_count = 0;
                        score++;
                    }
                }
                break;
            case 0x39: // space (hard drop)
                unsigned int temp;
                if (konami) {
                    temp = grid_sel_x;
                    while(!(illegality(1) || illegality(0))) {
                        grid_sel_x--;
                    }
                    grid_sel_x++;
                    score += 2*(temp-grid_sel_x);
                } else {
                    temp = grid_sel_y;
                    while(!(illegality(4) || illegality(0))) {
                        grid_sel_y++;
                    }
                    grid_sel_y--;
                    score += 2*(grid_sel_y - temp);
                }
                fall_tick_count = 0;
                should_stamp = true;
                draw_grid();
                current_shape = next_shape;
                next_shape = get_from_bag();
                if (konami) {
                    grid_sel_x = 8;
                    grid_sel_y = 9;
                    current_rot = 1;
                } else {
                    grid_sel_x = 4;
                    grid_sel_y = 1;
                    current_rot = 0;
                }
                if(any_illegality()) {
                    reset();
                }
                break;
            case 0x48: // up arrow (rotate right)
                if (konami) {
                    grid_sel_y--;
                    if (illegality(3) || illegality(0)) {
                        grid_sel_y++;
                    }
                } else {
                    current_rot++;
                    if (current_rot >= 4) {
                        current_rot = 0;
                    }
                    if (any_illegality()) {
                        bool should_reset_state = false;
                        unsigned int old_rot = current_rot-1;
                        if (old_rot < 0) {
                            old_rot = 3;
                        }
                        unsigned int reset_state[3] = {grid_sel_x,grid_sel_y,old_rot};
                        if(illegality(1)) {
                            unsigned int moved = 0;
                            while (illegality(1) && moved < 2) {
                                grid_sel_x++;
                            }
                        } else if (illegality(2)) {
                            unsigned int moved = 0;
                            while (illegality(2) && moved < 2) {
                                grid_sel_x--;
                            }
                        }
                        if(any_illegality()) {
                            should_reset_state = true;
                        }
                        if (should_reset_state) {
                            grid_sel_x = reset_state[0];
                            grid_sel_y = reset_state[1];
                            current_rot = reset_state[2];
                        }
                    }
                }
                break;
            case 0x2C: // z (rotate left)
                current_rot--;
                if (current_rot < 0) {
                    current_rot = 3;
                }
                if (any_illegality()) {
                    bool should_reset_state = false;
                    unsigned int old_rot = current_rot+1;
                    if (old_rot >= 4) {
                        old_rot = 0;
                    }
                    unsigned int reset_state[3] = {grid_sel_x,grid_sel_y,old_rot};
                    if(illegality(1)) {
                        unsigned int moved = 0;
                        while (illegality(1) && moved < 2) {
                            grid_sel_x++;
                        }
                    } else if (illegality(2)) {
                        unsigned int moved = 0;
                        while (illegality(2) && moved < 2) {
                            grid_sel_x--;
                        }
                    }
                    if(any_illegality()) {
                        should_reset_state = true;
                    }
                    if (should_reset_state) {
                        grid_sel_x = reset_state[0];
                        grid_sel_y = reset_state[1];
                        current_rot = reset_state[2];
                    }
                }
                break;
            case 0x2D: // x (rotate right)
                current_rot++;
                if (current_rot >= 4) {
                    current_rot = 0;
                }
                if (any_illegality()) {
                    bool should_reset_state = false;
                    unsigned int old_rot = current_rot-1;
                    if (old_rot < 0) {
                        old_rot = 3;
                    }
                    unsigned int reset_state[3] = {grid_sel_x,grid_sel_y,old_rot};
                    if(illegality(1)) {
                        unsigned int moved = 0;
                        while (illegality(1) && moved < 2) {
                            grid_sel_x++;
                        }
                    } else if (illegality(2)) {
                        unsigned int moved = 0;
                        while (illegality(2) && moved < 2) {
                            grid_sel_x--;
                        }
                    }
                    if(any_illegality()) {
                        should_reset_state = true;
                    }
                    if (should_reset_state) {
                        grid_sel_x = reset_state[0];
                        grid_sel_y = reset_state[1];
                        current_rot = reset_state[2];
                    }
                }
                break;
            case 0x2E: // c (hold)
                if (!held_this_turn) {
                    held_this_turn = true;
                    if (held_shape == -1) {
                        held_shape = current_shape;
                        current_shape = next_shape;
                        next_shape = get_from_bag();
                        fall_tick_count = 0;
                    } else {
                        unsigned int temp = current_shape;
                        current_shape = held_shape;
                        held_shape = temp;
                        fall_tick_count = 0;
                    }
                    if (konami) {
                        grid_sel_x = 8;
                        grid_sel_y = 10;
                        current_rot = 1;
                    } else {
                        grid_sel_x = 4;
                        grid_sel_y = 1;
                        current_rot = 0;
                    }
                    if(any_illegality()) {
                        reset();
                    }
                }
                break;
            case 0x13: // r (reset)
                reset();
                break;
            case 0x3B: // f1 (toggle highlights)
                highlight_bg = !highlight_bg;
                break;
            case 0x3C: // f2 (toggle grid mode)
                if (highlight_bg) {
                    grid_mode++;
                    if (grid_mode >= *(&grid_modes + 1) - grid_modes) {
                        grid_mode = 0;
                    }
                }
                break;
            case 0x3D: // f3 (toggle timer)
                show_timer = !show_timer;
                if (show_timer) {
                    unsigned int seconds = round(tick_count / 18);
                    char buffer[16];
                    itoa(seconds, buffer, 10);
                    unsigned int i = 0;
                    while (buffer[i]) i++;
                    buffer[i] = 's';
                    buffer[i+1] = '\0';
                    print(buffer, WHITE, 1);
                } else {
                    print("        ", WHITE, 1);
                }
                break;
            case 0x3E: // f4 (toggle keycode viewer)
                show_keycodes = !show_keycodes;
                if (!show_keycodes){
                    print("                 ", WHITE, 2);
                }
                break;
            default:
                break;
        }
        // Konami handling
        if ((int) scancode == konami_inputs[konami_progress][1]) {
            konami_progress++;
            konami_ticks = 18;
            if (konami_progress == 11) {
                konami_progress = 0;
                konami = !konami;
            }
        } else if ((int) scancode != konami_inputs[konami_progress][0]) {
            konami_progress = 0;
        }

        if (show_keycodes) {
            print("KEYCODE:         ", WHITE, 2);
            char buffer[16];
            itoa(scancode, buffer, 10);
            print_at(buffer, WHITE, 2, 10);
        }
    }
    draw_grid();
    outb(0x20, 0x20);
}
void k_main() {
    outb(0x3C6, 0xFF);    
    set_custom_palette();   
    disable_cursor();   
    reset();    
    draw_grid();   
    pic_remap();  
    k_install_idt(); 
    init_timer();
    //asm volatile ("sti"); 
    //kernel_loop();
}
