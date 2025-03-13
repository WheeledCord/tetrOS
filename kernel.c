#include "kernel.h"
#include "game_settings.h"
#include "classes.h"
#include "math.h"

// Ticks
    unsigned int tick = 0;
    unsigned int lcp_tick = 0;

// Game Variables
    unsigned int current_shape_x,current_shape_y,current_shape_rot;
    unsigned int bag[7];
    enum Shape next_shape;
    enum Shape held_shape;
    enum Shape current_shape;

    float gravity;

    bool collision_bug_occured = false;

// Game Settings
    bool ascii_mode = default_ascii_mode; 
    bool paused = false; 

// Tilemap
    int tilemap[grid_height][grid_width];
    void clear_tilemap() {
    for (unsigned int y = 0; y < grid_height; y++) {
        for (unsigned int x = 0; x < grid_width; x++) {
            tilemap[y][x] = 0;
        }
    }
}

// Points
    int shape_points[4][2];
    void calc_shape_points(enum Shape shape, unsigned int rot) {
        // Select which rotation to take points from
        int (*r)[4];
        switch(shape) {
            case o: r = shape_o[rot]; break;
            case i: r = shape_i[rot]; break;
            case l: r = shape_l[rot]; break;
            case j: r = shape_j[rot]; break;
            case s: r = shape_s[rot]; break;
            case z: r = shape_z[rot]; break;
            case t: r = shape_t[rot]; break;
        }
        // Plot points to new list, and determine center positions
        int cx = 0, cy = 0;
        int points[4][2] = {{0,0},{0,0},{0,0},{0,0}};
        unsigned int i = 0;
        for (int y = 0; y < 4; y++) {
            int *row = r[y];
            for (int x = 0; x < 4; x++) {
                switch(row[x]) {
                    case 1: points[i][0] = x; points[i][1] = y; i++; break; // normal point
                    case 2: points[i][0] = x; points[i][1] = y; cx = x; cy = y; i++; break; // center point
                    default: break;
                }
            }
        }
        // Copy points over to output array
        for (i = 0; i < 4; i++) {
            shape_points[i][0] = points[i][0] - cx;
            shape_points[i][1] = points[i][1] - cy;
        }
    }

    bool illegalities[5];
    bool any_illegality = false;
    unsigned int calc_illegality(enum Shape shape, unsigned int rot, unsigned int cx, unsigned int cy) {
        // Calculate the relative points
        calc_shape_points(shape,rot);
        for (unsigned int i = 0; i < 4; i++) {
            // Get tile x and y
            int x = shape_points[i][0]+cx;
            int y = shape_points[i][1]+cy;
            illegalities[0] = x<0; // piece is out of bound (left)
            illegalities[1] = x>=grid_width; // piece is out of bound (right)
            illegalities[2] = y<0; // piece is out of bound (up)
            illegalities[3] = y>=grid_height; // piece is out of bound (down)
            illegalities[4] = tilemap[y][x] != 0; // piece is overlapping a stamped piece
            any_illegality = false;
            if (x<0 || x>=grid_width || y<0 || y>=grid_height || tilemap[y][x] != 0) {
                any_illegality = true;
            }
        }
    }

// Drawing to Screen
    #define top_margin 3
    #define left_margin (40-grid_width-1)
    #define shapes_middle_margin ((top_margin+grid_height)-8+1)
    #define shapes_left_margin (80-left_margin/2-5)

    void draw_grid() {
        // Get strings for grid characters, from either grid_chars_ascii if in ascii mode, or grid_chars if not.
        char chars[11][2];
        for (unsigned int i = 0; i < 11; i++) {
            if (ascii_mode) {
                chars[i][0] = grid_chars_ascii[i];
            } else {
                chars[i][0] = grid_chars[i];
            }
            chars[i][1] = 0x00;
        }
        // Draw the play area
            // Draw the top edge
            unsigned int x = 0;
            print(chars[2],WHITE,top_margin,left_margin+x++); // top left corner
            for (unsigned int i = 0; i < grid_width*2; i++) {
                print(chars[3],WHITE,top_margin,left_margin+x++); // top edge
            }
            print(chars[4],WHITE,top_margin,left_margin+x++); // top right corner
            // Draw the left and right edges, and the grid
            for(unsigned int y = 1; y <= grid_height; y++) {
                x = 0;
                print(chars[5],WHITE,top_margin+y,left_margin+x++); // left edge
                for (unsigned int i = 0; i < grid_width; i++) {
                    print(chars[0],GRAY,top_margin+y,left_margin+x++); // left half of tile on grid
                    print(chars[1],GRAY,top_margin+y,left_margin+x++); // right half of tile on grid
                }
                print(chars[5],WHITE,top_margin+y,left_margin+x++); // right edge
            }
            // Draw the bottom edge
            x = 0;
            print(chars[6],WHITE,top_margin+grid_height,left_margin+x++); // bottom left corner
            for (unsigned int i = 0; i < grid_width*2; i++) {
                print(chars[7],WHITE,top_margin+grid_height,left_margin+x++); // bottom edge
            }
            print(chars[8],WHITE,top_margin+grid_height,left_margin+x++); // bottom right corner

        // Draw the next shape box
            // Draw the top edge
            x = 0;
            print(chars[2],WHITE,top_margin,shapes_left_margin+x++); // top left corner
            for (unsigned int i = 0; i < 8; i++) {
                print(chars[3],WHITE,top_margin,shapes_left_margin+x++); // top edge
            }
            print(chars[4],WHITE,top_margin,shapes_left_margin+x++); // top right corner
            // Draw the left and right edges, and the "NEXT" Label
            print(chars[5],WHITE,top_margin+1,shapes_left_margin); // left edge
            print("  NEXT: ",WHITE,top_margin+1,shapes_left_margin+1); // "NEXT" label
            print(chars[5],WHITE,top_margin+1,shapes_left_margin+1+8); // right edge
            // Draw the left and right joints, and the seperating line
            x = 0;
            print(chars[9],WHITE,top_margin+2,shapes_left_margin+x++); // left joint
            for (unsigned int i = 0; i < 8; i++) {
                print(chars[7],WHITE,top_margin+2,shapes_left_margin+x++); // seperating line
            }
            print(chars[10],WHITE,top_margin+2,shapes_left_margin+x++); // right joint
            // Draw the left and right edges, and the space
            for(unsigned int y = 1; y <= 4; y++) {
                x = 0;
                print(chars[5],WHITE,top_margin+2+y,shapes_left_margin+x++); // left edge
                for (unsigned int i = 0; i < 4; i++) {
                    print("  ",GRAY,top_margin+2+y,shapes_left_margin+x++); // space
                    x++;
                }
                print(chars[5],WHITE,top_margin+2+y,shapes_left_margin+x++); // right edge
            }
            // Draw the bottom edge
            x = 0;
            print(chars[6],WHITE,top_margin+2+5,shapes_left_margin+x++); // bottom left corner
            for (unsigned int i = 0; i < 8; i++) {
                print(chars[7],WHITE,top_margin+2+5,shapes_left_margin+x++); // bottom edge
            }
            print(chars[8],WHITE,top_margin+2+5,shapes_left_margin+x++); // bottom right corner

        // Draw the held shape box
            // Draw the top edge
            x = 0;
            print(chars[2],WHITE,shapes_middle_margin,shapes_left_margin+x++); // top left corner
            for (unsigned int i = 0; i < 8; i++) {
                print(chars[3],WHITE,shapes_middle_margin,shapes_left_margin+x++); // top edge
            }
            print(chars[4],WHITE,shapes_middle_margin,shapes_left_margin+x++); // top right corner
            // Draw the left and right edges, and the "HELD" Label
            print(chars[5],WHITE,shapes_middle_margin+1,shapes_left_margin); // left edge
            print("  HELD: ",WHITE,shapes_middle_margin+1,shapes_left_margin+1); // "HELD" label
            print(chars[5],WHITE,shapes_middle_margin+1,shapes_left_margin+1+8); // right edge
            // Draw the left and right joints, and the seperating line
            x = 0;
            print(chars[9],WHITE,shapes_middle_margin+2,shapes_left_margin+x++); // left joint
            for (unsigned int i = 0; i < 8; i++) {
                print(chars[7],WHITE,shapes_middle_margin+2,shapes_left_margin+x++); // seperating line
            }
            print(chars[10],WHITE,shapes_middle_margin+2,shapes_left_margin+x++); // right joint
            // Draw the left and right edges, and the space
            for(unsigned int y = 1; y <= 4; y++) {
                x = 0;
                print(chars[5],WHITE,shapes_middle_margin+2+y,shapes_left_margin+x++); // left edge
                for (unsigned int i = 0; i < 4; i++) {
                    print("  ",GRAY,shapes_middle_margin+2+y,shapes_left_margin+x++); // space
                    x++;
                }
                print(chars[5],WHITE,shapes_middle_margin+2+y,shapes_left_margin+x++); // right edge
            }
            // Draw the bottom edge
            x = 0;
            print(chars[6],WHITE,shapes_middle_margin+2+5,shapes_left_margin+x++); // bottom left corner
            for (unsigned int i = 0; i < 8; i++) {
                print(chars[7],WHITE,shapes_middle_margin+2+5,shapes_left_margin+x++); // bottom edge
            }
            print(chars[8],WHITE,shapes_middle_margin+2+5,shapes_left_margin+x++); // bottom right corner
    }

    void draw_current_shape() {
        // Calculate the relative points
        calc_shape_points(current_shape,current_shape_rot);
        for(unsigned int p = 0; p < 4; p++) {
            // Select shape colour
            char bg,fg;
            switch (current_shape) {
                case o: bg = YELLOW; fg = LIGHT_YELLOW; break;
                case i: bg = CYAN; fg = LIGHT_CYAN; break;
                case l: bg = GREEN; fg = LIGHT_GREEN; break;
                case j: bg = RED; fg = LIGHT_RED; break;
                case s: bg = ORANGE; fg = LIGHT_ORANGE; break;
                case z: bg = BLUE; fg = LIGHT_BLUE; break;
                case t: bg = PURPLE; fg = LIGHT_PURPLE; break;
                default: bg = GRAY; fg = WHITE; break;
            }
            // If in ascii mode, only colour foreground, else colour both background and foreground
            char colour;
            if (ascii_mode) {
                colour = bg;
            } else {
                colour = colour_combo(bg,fg);
            }
            // Plot the point onto the grid
            print("[]",colour,top_margin+1+current_shape_y+shape_points[p][1],left_margin+1+2*(current_shape_x+shape_points[p][0]));
        }
    }

    void draw_next_shape() {
        // Calculate the relative points for the default rotation
        calc_shape_points(next_shape,0);
        for(unsigned int p = 0; p < 4; p++) {
            // Select shape colour and offset
            char bg,fg;
            int x_offset = 0, y_offset = 0;
            switch (next_shape) {
                case o: bg = YELLOW; fg = LIGHT_YELLOW; x_offset = 2; y_offset = 2; break;
                case i: bg = CYAN; fg = LIGHT_CYAN; x_offset = 2; y_offset = 1; break;
                case l: bg = GREEN; fg = LIGHT_GREEN; x_offset = 3; y_offset = 2; break;
                case j: bg = RED; fg = LIGHT_RED; x_offset = 3; y_offset = 2; break;
                case s: bg = ORANGE; fg = LIGHT_ORANGE; x_offset = 3; y_offset = 2; break;
                case z: bg = BLUE; fg = LIGHT_BLUE; x_offset = 3; y_offset = 2; break;
                case t: bg = PURPLE; fg = LIGHT_PURPLE; x_offset = 3; y_offset = 2; break;
                default: bg = GRAY; fg = WHITE; x_offset = 3; y_offset = 2; break;
            }
            // If in ascii mode, only colour foreground, else colour both background and foreground
            char colour;
            if (ascii_mode) {
                colour = bg;
            } else {
                colour = colour_combo(bg,fg);
            }
            // Plot the point onto the indicator space grid
            print("[]",colour,top_margin+3+shape_points[p][1]+y_offset,shapes_left_margin+1+2*(shape_points[p][0])+x_offset);
        }
    }

    void draw_held_shape() {
        // Don't draw anything if no shape is held
        if (held_shape != unset) {
            // Calculate the relative points for the default rotation
            calc_shape_points(held_shape,0);
            for(unsigned int p = 0; p < 4; p++) {
                // Select shape colour and offset
                char bg,fg;
                int x_offset = 0, y_offset = 0;
                switch (held_shape) {
                    case o: bg = YELLOW; fg = LIGHT_YELLOW; x_offset = 2; y_offset = 2; break;
                    case i: bg = CYAN; fg = LIGHT_CYAN; x_offset = 2; y_offset = 1; break;
                    case l: bg = GREEN; fg = LIGHT_GREEN; x_offset = 3; y_offset = 2; break;
                    case j: bg = RED; fg = LIGHT_RED; x_offset = 3; y_offset = 2; break;
                    case s: bg = ORANGE; fg = LIGHT_ORANGE; x_offset = 3; y_offset = 2; break;
                    case z: bg = BLUE; fg = LIGHT_BLUE; x_offset = 3; y_offset = 2; break;
                    case t: bg = PURPLE; fg = LIGHT_PURPLE; x_offset = 3; y_offset = 2; break;
                    default: bg = GRAY; fg = WHITE; x_offset = 3; y_offset = 2; break;
                }
                // If in ascii mode, only colour foreground, else colour both background and foreground
                char colour;
                if (ascii_mode) {
                    colour = bg;
                } else {
                    colour = colour_combo(bg,fg);
                }
                // Plot the point onto the indicator space grid
                print("[]",colour,shapes_middle_margin+3+shape_points[p][1]+y_offset,shapes_left_margin+1+2*(shape_points[p][0])+x_offset);
            }
        }
    }

    void draw_stamped_shapes() {
        for (unsigned int y = 0; y < grid_height; y++) {
            for (int x = 0; x < grid_width; x++) {
                // Select shape colour
                char bg,fg;
                switch (tilemap[y][x]) {
                    case -1: bg = WHITE; fg = WHITE; break;
                    case 1: bg = YELLOW; fg = LIGHT_YELLOW; break;
                    case 2: bg = CYAN; fg = LIGHT_CYAN; break;
                    case 3: bg = GREEN; fg = LIGHT_GREEN; break;
                    case 4: bg = RED; fg = LIGHT_RED; break;
                    case 5: bg = ORANGE; fg = LIGHT_ORANGE; break;
                    case 6: bg = BLUE; fg = LIGHT_BLUE; break;
                    case 7: bg = PURPLE; fg = LIGHT_PURPLE; break;
                    default: bg = BLACK; fg = GRAY; break;
                }
                // If in ascii mode, only colour foreground, else colour both background and foreground
                char colour;
                if (ascii_mode) {
                    colour = bg;
                } else {
                    colour = colour_combo(bg,fg);
                }
                // Plot the point onto the indicator space grid
                if (tilemap[y][x] != 0) {
                    print("[]",colour,top_margin+1+y,left_margin+1+2*x);
                }
            }
        }
    }

    void draw_indicators() {
        if (paused) {
            if (ascii_mode) {
                printc(paused_indicator_ascii,paused_indicator_ascii_colour,paused_indicator_ascii_pos[1],paused_indicator_ascii_pos[0]);
            } else {
                printc(paused_indicator,paused_indicator_colour,paused_indicator_pos[1],paused_indicator_pos[0]);
            }
        }
        if (collision_bug_occured) {
            if (ascii_mode) {
                print(collision_bug_indicator_ascii,collision_bug_indicator_ascii_colour,collision_bug_indicator_ascii_pos[1],collision_bug_indicator_ascii_pos[0]);
            } else {
                print(collision_bug_indicator,collision_bug_indicator_colour,collision_bug_indicator_pos[1],collision_bug_indicator_pos[0]);
            }
        }
    }

// Bag
    void refil_bag() {
        // Placeholder code, replace once randomness is sorted.
        unsigned int new_bag[7] = {1,2,3,4,5,6,7};
        for(unsigned int i = 0; i < 7; i++) {
            bag[i] = new_bag[i];
        }
    }
    enum Shape take_from_bag() {
        // Test if bag is empty
        unsigned int i;
        bool bag_empty = true;
        for(i = 0; i < 7; i++) {
            if (bag[i] != 0) {
                bag_empty = false;
                break;
            }
        }
        // If bag is empty, refil it
        if (bag_empty) {
            refil_bag();
        }
        // Find the first shape left in the bag, remove it from the bag, and return it
        for(i = 0; i < 7; i++) {
            if (bag[i] != 0) {
                enum Shape selected;
                switch(bag[i]) {
                    case 1: selected = o; break;
                    case 2: selected = i; break;
                    case 3: selected = l; break;
                    case 4: selected = j; break;
                    case 5: selected = s; break;
                    case 6: selected = z; break;
                    case 7: selected = t; break;
                }
                bag[i] = 0;
                return selected;
            }
        }
    }

    // Game logic functions
void stamp() {
    // Calculate the relative points
    calc_shape_points(current_shape,current_shape_rot);
    for(unsigned int p = 0; p < 4; p++) {
        // Plot the point onto the tilemap
        tilemap[current_shape_y+shape_points[p][1]][current_shape_x+shape_points[p][0]] = ((int)current_shape)+1;
    }
    // Get new shape
    current_shape_x = spawn_x;
    current_shape_y = spawn_y;
    current_shape_rot = spawn_rot;
    current_shape = next_shape;
    next_shape = take_from_bag();
}

void reset() {
    collision_bug_occured = false;
    clear_tilemap();
    current_shape_x = spawn_x;
    current_shape_y = spawn_y;
    current_shape_rot = spawn_rot;
    gravity = start_gravity;
    refil_bag();
    current_shape = take_from_bag();
    next_shape = take_from_bag();
    held_shape = unset;
}


void main_loop() {
    // Game logic
    calc_illegality(current_shape,current_shape_rot,current_shape_x,current_shape_y);
    if (any_illegality) {
        collision_bug_occured = true;
    }

    // Display
    print("Welcome to TetrOS!",WHITE,1,40-9);
    draw_grid();
    draw_stamped_shapes();
    draw_current_shape();
    draw_next_shape();
    draw_held_shape();
    draw_indicators();
}

void tick_handler() {
    tick++;
    if (!paused) {
        if (lcp_tick > 0 && tick % round(18/lcp_duration) == 0) {
            lcp_tick--;
        }
    }

    main_loop();
    screen_update();

    outb(0x20, 0x20);
}
void key_handler() {
    // Get keycode
    unsigned char keycode = inb(0x60);
    // Don't allow input if paused, or lines are clearing, and ignore the "key up" keycode.
    if (!paused && keycode != 224) {
        switch(keycode) {
            case 0x4B: // left arrow (move left)
                if (current_shape_x > 0) {
                    current_shape_x --;
                    calc_illegality(current_shape,current_shape_rot,current_shape_x,current_shape_y);
                    if (any_illegality) {
                        current_shape_x ++;
                    }
                }
                break;
            case 0x4D: // right arrow (move right)
                if (current_shape_x < grid_width-1) {
                    current_shape_x ++;
                    calc_illegality(current_shape,current_shape_rot,current_shape_x,current_shape_y);
                    if (any_illegality) {
                        current_shape_x --;
                    }
                }
                break;
            case 0x50: // down arrow (soft drop)
                if (current_shape_y < grid_height-1) {
                    current_shape_y ++;
                    calc_illegality(current_shape,current_shape_rot,current_shape_x,current_shape_y);
                    if (any_illegality) {
                        current_shape_y --;
                    }
                }
                break;
            case 0x39: // space (hard drop)
                break;
            case 0x48: // up arrow (rotate right)
                break;
            case 0x2D: // x (rotate right)
                break;
            case 0x2C: // z (rotate left)
                break;
            case 0x2E: // c (hold)
                if (held_shape == unset) {
                    held_shape = current_shape;
                    current_shape = next_shape;
                    next_shape = take_from_bag();
                } else {
                    enum Shape temp_shape = current_shape;
                    current_shape = held_shape;
                    held_shape = temp_shape;
                }
                break;
            case 0x13: // r (reset)
                reset();
                break;
            case 0x3C: // f2 (debug switch shape)
                current_shape = next_shape;
                next_shape = take_from_bag();
                break;
            case 0x3D: // f3 (debug stamp)
                stamp();
                break;
            default:
                break;
        }
    }
    if (keycode == 0x01) { // esc (toggle pause)
        paused = !paused;
    }
    if (keycode == 0x3B) { // f1 (toggle ascii)
        ascii_mode = !ascii_mode;
    }

    outb(0x20, 0x20);
}

void kernel_init() {
    outb(0x3C6, 0xFF);

    // Set up display
    clear_screen();
    disable_cursor();
    set_palette();
    screen_update();

    // Start the game
    reset();

    // OS Stuff
    pic_remap();
    k_install_idt();
    asm volatile ("sti");
    while (true) { asm volatile ("hlt"); };
}