#ifndef COLOUR_LABELS_H
#define COLOUR_LABELS_H

unsigned int pallete[16][3] = {
    {0,  0,  0}, // Black
    {26, 26, 26}, // Gray
    {63, 63, 63}, // White
    {63, 0,  0}, // Red
    {63, 32, 0}, // Orange
    {50, 50, 0}, // Yellow
    {0,  50, 0}, // Green
    {0,  55, 55}, // Cyan
    {0,  0,  63}, // Blue
    {63, 0,  63}, // Purple
    {63, 32, 32},  // Light Red
    {63, 48, 32}, // Light Orange
    {63, 63, 50}, // Light Yellow
    {50, 63, 63}, // Light Cyan
    {50, 63, 50}, // Light Green
    {63, 48, 63} // Light Purple
};

#define BLACK 0x00
#define GRAY 0x01
#define WHITE 0x02
#define RED 0x03
#define ORANGE 0x04
#define YELLOW 0x05
#define GREEN 0x06
#define CYAN 0x07
#define BLUE 0x08
#define PURPLE 0x09
#define LIGHT_RED 0x0A
#define LIGHT_ORANGE 0x0B
#define LIGHT_YELLOW 0x0C
#define LIGHT_GREEN 0x0D
#define LIGHT_CYAN 0x0E
#define LIGHT_BLUE CYAN
#define LIGHT_PURPLE 0x0F

char colour_combo(char bg, char fg) {
    return (char)((int)bg*16+(int)fg);
}

#endif