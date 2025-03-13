#include "classes.h"
#ifndef GAME_SETTINGS_H
#define GAME_SETTINGS_H

unsigned int pallete[16][3] = {
    {0,  0,  0}, // Black
    {63, 63, 63}, // White
    {26, 26, 26}, // Gray
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

char grid_chars[11] = {0xc4,0xc5,0xc9,0xcd,0xbb,0xba,0xc8,0xcd,0xbc,0xcc,0xb9}; // ─┼╔═╗║╚═╝╠╣
char grid_chars_ascii[11] = " .+-+|+-+++";

char paused_indicator[7] = "PAUSED";
char paused_indicator_colour[7] = "222222";
unsigned int paused_indicator_pos[2] = {1,1};

char paused_indicator_ascii[7] = "PAUSED";
char paused_indicator_ascii_colour[7] = "222222";
unsigned int paused_indicator_ascii_pos[2] = {1,1};

char collision_bug_indicator[32] = "There has been a collision bug!";
char collision_bug_indicator_colour = 0x03;
unsigned int collision_bug_indicator_pos[2] = {1,2};

char collision_bug_indicator_ascii[32] = "There has been a collision bug!";
char collision_bug_indicator_ascii_colour = 0x03;
unsigned int collision_bug_indicator_ascii_pos[2] = {1,2};

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

#define grid_width 10
#define grid_height 20

#define default_ascii_mode false;

unsigned int spawn_x = ((int)((grid_width-1)/2));
unsigned int spawn_y = 1;
unsigned int spawn_rot = 0;
float start_gravity = 14.4;
float lcp_duration = 1;

#endif