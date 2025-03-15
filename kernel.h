#include "os_utils.h"
#include "string_utils.h"
#include "colours.h"
#include "classes.h"
#ifndef KERNEL_H
#define KERNEL_H


// Screen utilities
void disable_cursor() {
    outb(0x3D4, 0x0A);
    outb(0x3D5, 0x20);
}
void __set_palette_colour(int color_index, int rgb[3]) {
    outb(0x3C8, color_index); // Select the color index to modify
    outb(0x3C9, rgb[0] & 0x3F);    // Set red component (0-63)
    outb(0x3C9, rgb[1] & 0x3F);    // Set green component (0-63)
    outb(0x3C9, rgb[2] & 0x3F);    // Set blue component (0-63)
}
void set_palette() {
    __set_palette_colour(0, pallete[0]);
    __set_palette_colour(1, pallete[1]);
    __set_palette_colour(2, pallete[2]);
    __set_palette_colour(3, pallete[3]);
    __set_palette_colour(4, pallete[4]);
    __set_palette_colour(5, pallete[5]);
    __set_palette_colour(7, pallete[7]);
    __set_palette_colour(20, pallete[6]);
    __set_palette_colour(56, pallete[8]);
    __set_palette_colour(57, pallete[9]);
    __set_palette_colour(58, pallete[10]);
    __set_palette_colour(59, pallete[11]);
    __set_palette_colour(60, pallete[12]);
    __set_palette_colour(61, pallete[13]);
    __set_palette_colour(62, pallete[14]);
    __set_palette_colour(63, pallete[15]);
}

char screen_lines[2][25][80];
void clear_screen() {
    for(unsigned int y = 0; y < 25; y++) {
        for(unsigned int x = 0; x < 80; x++) {
            screen_lines[0][y][x] = ' ';
            screen_lines[1][y][x] = WHITE;
        }
    }
}
void screen_update() {
    char *vidmem = (char *)0xB8000;
    unsigned int i = 0;
    for(unsigned int y = 0; y < 25; y++) {
        for(unsigned int x = 0; x < 80; x++) {
            vidmem[i++] = screen_lines[0][y][x];
            vidmem[i++] = screen_lines[1][y][x];
        }
    }
    clear_screen();
}
void print(char string[], char colour, vec2 pos) {
    int x = pos.x;
    if (x < 0) { x = 0; }
    int y = pos.y;
    if (y < 0) { y = 0; }
    while (*string) {
        if (*string == '\n') {
            y++;
            x = 0;
            string++;
        } else {
            screen_lines[0][y][x] = *string++;
            screen_lines[1][y][x++] = colour;
        }
    }
}
void printi(int i, char colour, vec2 pos) {
    int x = pos.x;
    if (x < 0) { x = 0; }
    int y = pos.y;
    if (y < 0) { y = 0; }
    char *string = "";
    itoa(i,string,10);
    while (*string) {
        if (*string == '\n') {
            y++;
            x = 0;
            string++;
        } else {
            screen_lines[0][y][x] = *string++;
            screen_lines[1][y][x++] = colour;
        }
    }
}

void printb(bool b, char colour, vec2 pos) {
    int x = pos.x;
    if (x < 0) { x = 0; }
    int y = pos.y;
    if (y < 0) { y = 0; }
    char *string = "aaaaa";
    if (b) {
        set_str(string,"true");
    } else {
        set_str(string,"false");
    }
    while (*string) {
        if (*string == '\n') {
            y++;
            x = 0;
            string++;
        } else {
            screen_lines[0][y][x] = *string++;
            screen_lines[1][y][x++] = colour;
        }
    }
}

char __printc_switch(char c) {
    switch(c) {
        case '1': return GRAY; break;
        case '2': return WHITE; break;
        case '3': return RED; break;
        case '4': return ORANGE; break;
        case '5': return YELLOW; break;
        case '6': return GREEN; break;
        case '7': return CYAN; break;
        case '8': return BLUE; break;
        case '9': return PURPLE; break;
        case 'A': return LIGHT_RED; break;
        case 'B': return LIGHT_YELLOW; break;
        case 'C': return LIGHT_GREEN; break;
        case 'D': return LIGHT_CYAN; break;
        case 'E': return LIGHT_BLUE; break;
        case 'F': return LIGHT_PURPLE; break;
        case '-': return ' '; break;
        default: return BLACK; break;
    }
}

void printc(char *string, char *colours, vec2 pos) {
    int x = pos.x;
    if (x < 0) { x = 0; }
    int y = pos.y;
    if (y < 0) { y = 0; }
    unsigned int string_length = get_str_length(string);
    while (*string) {
        if (*string == '\n') {
            y++;
            x = 0;
            string++;
        } else {
            screen_lines[0][y][x++] = *string++;
        }
    }
    x = x-string_length;
    while (*colours) {
        char colour = __printc_switch(*colours++);
        if (colour == ' ') {
            colour = (__printc_switch(*colours++)*16)+__printc_switch(*colours++);
        }
        if (colour == BLACK) {
            colour = WHITE;
        }
        screen_lines[1][y][x++] = colour;
    }
}

// Time
int time_offset = 46800;
int unix_time;
Time time;
void unix_to_rtc() {
    int days = unix_time / 86400;
    int rem = unix_time % 86400;
    time.hour = rem / 3600;
    rem %= 3600;
    time.min = rem / 60;
    time.sec = rem % 60;
    time.year = 1970;
    while (days >= 365) {
        int leap = (time.year % 4 == 0) && (time.year % 100 != 0 || time.year % 400 == 0);
        if (days < 365 + leap) break;
        days -= 365 + leap;
        (time.year)++;
    }
    int month_days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if ((time.year % 4 == 0) && (time.year % 100 != 0 || time.year % 400 == 0)) {
        month_days[1] = 29;
    }
    time.month = 0;
    while (days >= month_days[time.month]) {
        days -= month_days[time.month];
        (time.month)++;
    }
    time.day = days + 1;
    time.month += 1;
}
int read_time() {
    unix_time = rtc_to_unix(bcd_to_bin(read_cmos(0x00)),bcd_to_bin(read_cmos(0x02)),bcd_to_bin(read_cmos(0x04)),bcd_to_bin(read_cmos(0x07)),bcd_to_bin(read_cmos(0x08)),bcd_to_bin(read_cmos(0x09))+2000);
    unix_time += time_offset;
    unix_to_rtc();
}

// Randomness
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

#endif