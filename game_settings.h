#include "classes.h"
#include "colours.h"
#ifndef GAME_SETTINGS_H
#define GAME_SETTINGS_H

// Grid Size
    #define grid_width 10
    #define grid_height 20

// Drawing
    // Boxes
        // Outlines
        char box_chars[] = {0xc9,0xcd,0xbb,0xba,0xc8,0xcd,0xbc,0xcc,0xb9}; // ╔═╗║╚═╝╠╣
        char box_chars_ascii[] = "+-+|+-+++";
        char box_colour = WHITE;

        // Grids
        char grid_chars[] = {0xc4,0xc5}; // ─┼
        char grid_chars_ascii[] = " .";
        char grid_colour = GRAY; 
        bool show_shape_ui_grids = false;

        // Labels
        char next_label[] = "  NEXT: ";
        char held_label[] = "  HELD: ";
        char lvl_label[] = "   LVL:  ";
        char lines_label[] = "  LINES: "; 
        char score_label[] = "  SCORE: ";
        char stat_colour = WHITE;

        // Positions
        vec2 play_area_top_left = {29,3};
        vec2 next_shape_top_left = {61,5};
        vec2 held_shape_top_left = {61,15};
        vec2 stat_lvl_top_left = {8,4};
        vec2 stat_lines_top_left = {8,11};
        vec2 stat_score_top_left = {8,18};

    // Paused Indicator
    char paused_indicator[] = "PAUSED";
    char paused_indicator_colour[] = "222222";
    vec2 paused_indicator_pos = {1,1};

    // Collision Bug Indicator
    char collision_bug_indicator[] = "There has been a collision bug!";
    char collision_bug_indicator_colour[] = "3333333333333333333333333333333";
    vec2 collision_bug_indicator_pos = {1,2};

    // Title
    char title_message[] = "Welcome to TetrOS!";
    char title_message_colour[] = "222222222222222FF2";
    vec2 title_message_pos = {31,1};

    // Time
    char time_message[] = "%2% :%1%  (%0% ) - %3% /%4% /%5% ";
    char time_colour = WHITE;
    vec2 time_pos = {0,0};

    // Flashing
    unsigned int lc_flashes = 3;
    float lc_flash_duration = 0.25;
    char flash_colour = WHITE;

// Shapes
    #define total_shapes 7
    enum ShapeID {
        o,i,l,j,s,z,t,
        unset
    };
    Shape shapes[] = {
        {YELLOW,LIGHT_YELLOW,{2,2},{ // o piece
            {
                {0,-1},{1,-1},
                {0,0},{1,0},
            },
            {
                {0,-1},{1,-1},
                {0,0},{1,0},
            },
            {
                {0,-1},{1,-1},
                {0,0},{1,0},
            },
            {
                {0,-1},{1,-1},
                {0,0},{1,0},
            }
        }},
        {CYAN,LIGHT_CYAN,{2,1},{ // i piece
            {
                {-1,0},{0,0},{1,0},{2,0}
            },
            {
                {0,-1},
                {0,0},
                {0,1},
                {0,2},
            },
            {
                {-2,0},{-1,0},{0,0},{1,0}
            },
            {
                {0,-2},
                {0,-1},
                {0,0},
                {0,1},
            }
        }},
        {GREEN,LIGHT_GREEN,{3,2},{ // l piece
            {
                            {1,-1},
                {-1,0},{0,0},{1,0}
            },
            {
                {0,-1},
                {0,0},
                {0,1},{1,1}
            },
            {
                {-1,0},{0,0},{1,0},
                {-1,1}
            },
            {
                {-1,-1},{0,-1},
                        {0,0},
                        {0,1}
            }
        }},
        {RED,LIGHT_RED,{3,2},{ // j piece
            {
                {-1,-1},
                {-1,0},{0,0},{1,0}
            },
            {
                {0,-1},{1,-1},
                {0,0},
                {0,1}
            },
            {
                {-1,0},{0,0},{1,0},
                            {1,1}
            },
            {
                    {0,-1},
                    {0,0},
                {-1,1},{0,1}
            }
        }},
        {ORANGE,LIGHT_ORANGE,{3,2},{ // s piece
            {
                    {0,-1},{1,-1},
                {-1,0},{0,0}
            },
            {
                {0,-1},
                {0,0},{1,0},
                    {1,1}
            },
            {
                    {0,0},{1,0},
                {-1,1},{0,1}
            },
            {
                {-1,-1},
                {-1,0},{0,0},
                    {0,1}
            }
        }},
        {BLUE,LIGHT_BLUE,{3,2},{ // z piece
            {
                {-1,-1},{0,-1},
                        {0,0},{1,0}
            },
            {
                    {1,-1},
                {0,0},{1,0},
                {0,1}
            },
            {
                {-1,0},{0,0},
                    {0,1},{1,1}
            },
            {
                    {0,-1},
                {-1,0},{0,0},
                {-1,1}
            }
        }},
        {PURPLE,LIGHT_PURPLE,{3,2},{ // t piece
            {
                    {0,-1},
                {-1,0},{0,0},{1,0}
            },
            {
                {0,-1},
                {0,0},{1,0},
                {0,1}
            },
            {
                {-1,0},{0,0},{1,0},
                    {0,1}
            },
            {
                    {0,-1},
                {-1,0},{0,0},
                    {0,1}
            }
        }},
        {WHITE,GRAY,{0,0},{ // unset
            {{0,0},{0,0},{0,0},{0,0}},
            {{0,0},{0,0},{0,0},{0,0}},
            {{0,0},{0,0},{0,0},{0,0}},
            {{0,0},{0,0},{0,0},{0,0}}
        }}
    };

    // Spawn conditions
    vec2 spawn_pos = {(int)((grid_width-1)/2),1}; // Not recomended to touch; currently set up to spawn in middle of grid, prefering left for even grid widths, and keeping immediate rotation possible.
    unsigned int spawn_rot = 0;

// Progression
    // Gravity Progression
    #define total_gravity_progression_levels 4
    GravityProgressionLevel gravity_progression[] = {
        {14.4,1,{0}}, // start_gravity
        {1.5,8,{1,2,3,4,5,6,7,8}}, // < 9
        {0.6,1,{9}}, // 9
        {0.3,5,{10,13,16,19,29}} // 10,13,16,19,29
    };

    // Level progresstion
    #define lines_per_level 10

    // Score progression
    #define score_add_tetris 1200
    #define score_add_triple 300
    #define score_add_double 100
    #define score_add_single 40
    #define score_add_hard_drop 2
    #define score_add_soft_drop 1

// Default settings
    #define default_ascii_mode false;
    #define default_show_ghost true;
    #define default_do_wall_kicks true;

// Controls
    #define K_left 0x4B // left arrow
    #define K_right 0x4D // right arrow
    #define K_soft_drop 0x50 // down arrow
    #define K_hard_drop 0x39 // space
    #define K_rotate_right1 0x48 // up arrow
    #define K_rotate_right2 0x2D // z
    #define K_rotate_left 0x2C // x
    #define K_hold 0x2E // c
    #define K_reset 0x13 // r
    #define K_toggle_pause 0x01 // esc
    #define K_toggle_ascii 0x3B // f1
    #define K_toggle_ghost 0x3C // f2
    #define K_toggle_wall_kicks 0x3D // f3
    #define K_reset_settings 0x3E // f4

// Misc
    #define slide_time 1.0

#endif