#include "kernel.h"
#include "game_settings.h"
#include "classes.h"
#include "math.h"

// Ticks
    unsigned int tick = 0;
    unsigned int lc_tick = 0;
    unsigned int anti_gravity_tick = 0;

// Game Variables
    int current_shape_x,current_shape_y,ghost_shape_y,current_shape_rot;
    unsigned int bag[7];
    enum ShapeID next_shape;
    enum ShapeID held_shape;
    enum ShapeID current_shape;

    unsigned int score = 0, lvl = 0, total_lines_cleared = 0;
    float gravity;

    bool collision_bug_occured = false;
    bool game_over = false;
    bool held_this_turn = false;

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
    void calc_shape_points(enum ShapeID shape, int rot) {
        // Get points data
        int r[4][4] = shapes[shape].data[rot];
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

// Drawing to Screen
    #define top_margin 3
    #define left_margin (40-grid_width-1)
    #define shapes_middle_margin ((top_margin+grid_height)-8)
    #define shapes_left_margin (80-left_margin/2-5)
    // #define stats_middle_margin ((top_margin+grid_height)-8)
    #define stats_left_margin (left_margin/2-5)
    #define stats_width 7
    #define stats_gap 4

    void draw_boxes_and_grid() {
        // Get strings for grid characters, from either grid_chars_ascii if in ascii mode, or grid_chars if not
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
            for (unsigned int y = 1; y <= grid_height; y++) {
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
            print(chars[6],WHITE,top_margin+grid_height+1,left_margin+x++); // bottom left corner
            for (unsigned int i = 0; i < grid_width*2; i++) {
                print(chars[7],WHITE,top_margin+grid_height+1,left_margin+x++); // bottom edge
            }
            print(chars[8],WHITE,top_margin+grid_height+1,left_margin+x++); // bottom right corner

        // Draw the next shape box
            // Draw the top edge
            x = 0;
            print(chars[2],WHITE,top_margin+2,shapes_left_margin+x++); // top left corner
            for (unsigned int i = 0; i < 8; i++) {
                print(chars[3],WHITE,top_margin+2,shapes_left_margin+x++); // top edge
            }
            print(chars[4],WHITE,top_margin+2,shapes_left_margin+x++); // top right corner
            // Draw the left and right edges, and the "NEXT" Label
            print(chars[5],WHITE,top_margin+2+1,shapes_left_margin); // left edge
            print("  NEXT: ",WHITE,top_margin+2+1,shapes_left_margin+1); // "NEXT" label
            print(chars[5],WHITE,top_margin+2+1,shapes_left_margin+1+8); // right edge
            // Draw the left and right joints, and the seperating line
            x = 0;
            print(chars[9],WHITE,top_margin+2+2,shapes_left_margin+x++); // left joint
            for (unsigned int i = 0; i < 8; i++) {
                print(chars[7],WHITE,top_margin+2+2,shapes_left_margin+x++); // seperating line
            }
            print(chars[10],WHITE,top_margin+2+2,shapes_left_margin+x++); // right joint
            // Draw the left and right edges, and the space
            for (unsigned int y = 1; y <= 4; y++) {
                x = 0;
                print(chars[5],WHITE,top_margin+2+2+y,shapes_left_margin+x++); // left edge
                for (unsigned int i = 0; i < 4; i++) {
                    print("  ",GRAY,top_margin+2+2+y,shapes_left_margin+x++); // space
                    x++;
                }
                print(chars[5],WHITE,top_margin+2+2+y,shapes_left_margin+x++); // right edge
            }
            // Draw the bottom edge
            x = 0;
            print(chars[6],WHITE,top_margin+2+2+5,shapes_left_margin+x++); // bottom left corner
            for (unsigned int i = 0; i < 8; i++) {
                print(chars[7],WHITE,top_margin+2+2+5,shapes_left_margin+x++); // bottom edge
            }
            print(chars[8],WHITE,top_margin+2+2+5,shapes_left_margin+x++); // bottom right corner

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
            for (unsigned int y = 1; y <= 4; y++) {
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
        
        // Draw the lvl stats box
            // Draw the top edge
            x = 0;
            print(chars[2],WHITE,top_margin+2,stats_left_margin+x++); // top left corner
            for (unsigned int i = 0; i < stats_width; i++) {
                print(chars[3],WHITE,top_margin+2,stats_left_margin+x++); // top edge
            }
            print(chars[4],WHITE,top_margin+2,stats_left_margin+x++); // top right corner
            // Draw the left and right edges, and the "LVL" Label
            print(chars[5],WHITE,top_margin+2+1,stats_left_margin); // left edge
            print("  LVL: ",WHITE,top_margin+2+1,stats_left_margin+1); // "LVL" label
            print(chars[5],WHITE,top_margin+2+1,stats_left_margin+1+stats_width); // right edge
            // Draw the left and right joints, and the seperating line
            x = 0;
            print(chars[9],WHITE,top_margin+2+2,stats_left_margin+x++); // left joint
            for (unsigned int i = 0; i < stats_width; i++) {
                print(chars[7],WHITE,top_margin+2+2,stats_left_margin+x++); // seperating line
            }
            print(chars[10],WHITE,top_margin+2+2,stats_left_margin+x++); // right joint
            // Draw the left and right edges, and the space
            x = 0;
            print(chars[5],WHITE,top_margin+2+2+1,stats_left_margin+x++); // left edge
            for (unsigned int i = 0; i < stats_width; i++) {
                print(" ",GRAY,top_margin+2+2+1,stats_left_margin+x++); // space
            }
            print(chars[5],WHITE,top_margin+2+2+1,stats_left_margin+x++); // right edge
            // Draw the bottom edge
            x = 0;
            print(chars[6],WHITE,top_margin+2+2+2,stats_left_margin+x++); // bottom left corner
            for (unsigned int i = 0; i < stats_width; i++) {
                print(chars[7],WHITE,top_margin+2+2+2,stats_left_margin+x++); // bottom edge
            }
            print(chars[8],WHITE,top_margin+2+2+2,stats_left_margin+x++); // bottom right corner

        // Draw the lines cleared stats box
            // Draw the top edge
            x = 0;
            print(chars[2],WHITE,top_margin+2+stats_gap+2,stats_left_margin+x++); // top left corner
            for (unsigned int i = 0; i < stats_width; i++) {
                print(chars[3],WHITE,top_margin+2+stats_gap+2,stats_left_margin+x++); // top edge
            }
            print(chars[4],WHITE,top_margin+2+stats_gap+2,stats_left_margin+x++); // top right corner
            // Draw the left and right edges, and the "LVL" Label
            print(chars[5],WHITE,top_margin+2+stats_gap+2+1,stats_left_margin); // left edge
            print(" LINES:",WHITE,top_margin+2+stats_gap+2+1,stats_left_margin+1); // "LVL" label
            print(chars[5],WHITE,top_margin+2+stats_gap+2+1,stats_left_margin+1+stats_width); // right edge
            // Draw the left and right joints, and the seperating line
            x = 0;
            print(chars[9],WHITE,top_margin+2+stats_gap+2+2,stats_left_margin+x++); // left joint
            for (unsigned int i = 0; i < stats_width; i++) {
                print(chars[7],WHITE,top_margin+2+stats_gap+2+2,stats_left_margin+x++); // seperating line
            }
            print(chars[10],WHITE,top_margin+2+stats_gap+2+2,stats_left_margin+x++); // right joint
            // Draw the left and right edges, and the space
            x = 0;
            print(chars[5],WHITE,top_margin+2+stats_gap+2+2+1,stats_left_margin+x++); // left edge
            for (unsigned int i = 0; i < stats_width; i++) {
                print(" ",GRAY,top_margin+2+stats_gap+2+2+1,stats_left_margin+x++); // space
            }
            print(chars[5],WHITE,top_margin+2+stats_gap+2+2+1,stats_left_margin+x++); // right edge
            // Draw the bottom edge
            x = 0;
            print(chars[6],WHITE,top_margin+2+stats_gap+2+2+2,stats_left_margin+x++); // bottom left corner
            for (unsigned int i = 0; i < stats_width; i++) {
                print(chars[7],WHITE,top_margin+2+stats_gap+2+2+2,stats_left_margin+x++); // bottom edge
            }
            print(chars[8],WHITE,top_margin+2+stats_gap+2+2+2,stats_left_margin+x++); // bottom right corner
    }

    void draw_current_shape() {
        // Don't draw anything if clearing line
        if (lc_tick == 0) {
            // Calculate the relative points
            calc_shape_points(current_shape,current_shape_rot);
            for (unsigned int p = 0; p < 4; p++) {
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
    }

    void draw_ghost_shape() {
        // Don't draw anything if clearing line
        if (lc_tick == 0) {
            // Calculate the relative points
            calc_shape_points(current_shape,current_shape_rot);
            for (unsigned int p = 0; p < 4; p++) {
                // If in ascii mode, only colour foreground, else colour both background and foreground
                char colour;
                if (ascii_mode) {
                    colour = GRAY;
                } else {
                    colour = colour_combo(GRAY,WHITE);
                }
                // Plot the point onto the grid
                print("[]",colour,top_margin+1+ghost_shape_y+shape_points[p][1],left_margin+1+2*(current_shape_x+shape_points[p][0]));
            }
        }
    }

    void draw_next_shape() {
        // Calculate the relative points for the default rotation
        calc_shape_points(next_shape,0);
        for (unsigned int p = 0; p < 4; p++) {
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
            print("[]",colour,top_margin+2+3+shape_points[p][1]+y_offset,shapes_left_margin+1+2*(shape_points[p][0])+x_offset);
        }
    }

    void draw_held_shape() {
        // Don't draw anything if no shape is held
        if (held_shape != unset) {
            // Calculate the relative points for the default rotation
            calc_shape_points(held_shape,0);
            for (unsigned int p = 0; p < 4; p++) {
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
        // Placeholder code, replace once randomness is sorted
        unsigned int new_bag[7] = {1,2,3,4,5,6,7};
        for (unsigned int i = 0; i < 7; i++) {
            bag[i] = new_bag[i];
        }
    }
    enum ShapeID take_from_bag() {
        // Test if bag is empty
        unsigned int i;
        bool bag_empty = true;
        for (i = 0; i < 7; i++) {
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
        for (i = 0; i < 7; i++) {
            if (bag[i] != 0) {
                enum ShapeID selected;
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
bool shape_illegalities[5];
bool shape_illegal = false;
void calc_shape_illegality(enum ShapeID shape, int rot, int cx, int cy) {
    // Calculate the relative points
    calc_shape_points(shape,rot);
    for (unsigned int i = 0; i < 5; i++) {
        shape_illegalities[i] = false;
    }
    for (unsigned int p = 0; p < 4; p++) {
        // Get tile x and y
        int x = cx+shape_points[p][0];
        int y = cy+shape_points[p][1];
        if (!shape_illegalities[0]) {
            shape_illegalities[0] = x < 0; // point out of bound (left)
        }
        if (!shape_illegalities[1]) {
            shape_illegalities[1] = x >= grid_width; // point out of bound (right)
        }
        if (!shape_illegalities[2]) {
            shape_illegalities[2] = y < 0; // point out of bound (up)
        }
        if (!shape_illegalities[3]) {
            shape_illegalities[3] = y >= grid_height; // point out of bound (down)
        }
        if (!shape_illegalities[4]) {
            shape_illegalities[4] = tilemap[y][x] != 0; // point overlapping stamped piece
        }
    }
    shape_illegal = false;
    for (unsigned int i = 0; i < 5; i++) {
        if (shape_illegalities[i]) {
            shape_illegal = true;
            break;
        }
    }
}

int calc_ghost_y() {
    ghost_shape_y = current_shape_y;
    shape_illegal = false;
    while (!shape_illegal) {
        ghost_shape_y++;
        calc_shape_illegality(current_shape,current_shape_rot,current_shape_x,ghost_shape_y);
    }
    ghost_shape_y--;
}

bool lines_cleared[grid_height];
unsigned int check_line_clears() {
    unsigned int lines_cleared_amount = 0;
    for (unsigned int y = 0; y < grid_height; y++) {
        // If this line has any gaps, it isn't full
        bool line_full = true;
        for (unsigned int x = 0; x < grid_width; x++) {
            if (tilemap[y][x] == 0) {
                line_full = false;
                break;
            }
        }
        // Add result to the output array and increase cleared lines counter
        lines_cleared[y] = line_full;
        if (line_full) {
            lines_cleared_amount++;
        }
    }
    if (lines_cleared_amount > 0) {
        // Start flashing lines
        lc_tick = 2*lc_flashes;
        // Update total lines cleared counter
        total_lines_cleared += lines_cleared_amount;
        unsigned int old_level = lvl;
        // Update level
        lvl = round(total_lines_cleared / 10);
        if (lvl != old_level) {
            // Update gravity
            if (lvl < 9) {
                gravity -= 1.5;
            } else if (lvl == 9) {
                gravity -= 0.6;
            } else if (lvl == 10 || lvl == 13 || lvl == 16 || lvl == 19 || lvl == 29) {
                gravity -= 0.3;
            }
        }
        // Update score
        score += round(lines_cleared_amount / 4)*(1200*(lvl+1)); // tetrises
        score += round((lines_cleared_amount % 4) / 3)*(300*(lvl+1)); // triples
        score += round(((lines_cleared_amount % 4) % 3) / 2)*(100*(lvl+1)); // doubles
        score += (((lines_cleared_amount % 4) % 3) % 2)*(40*(lvl+1)); // singles
    }
    return lines_cleared_amount;
}

void stamp() {
    // Calculate the relative points
    calc_shape_points(current_shape,current_shape_rot);
    for (unsigned int p = 0; p < 4; p++) {
        // Plot the point onto the tilemap
        tilemap[current_shape_y+shape_points[p][1]][current_shape_x+shape_points[p][0]] = ((int)current_shape)+1;
    }
    // Test for line clears
    unsigned int lines_cleared_amount = check_line_clears();
    // Get new shape if no lines cleared, if there is, the new shape will be assigned once the lines stop flashing
    if (lines_cleared_amount == 0) {
        current_shape_x = spawn_x;
        current_shape_y = spawn_y;
        current_shape_rot = spawn_rot;
        current_shape = next_shape;
        next_shape = take_from_bag();
        calc_shape_illegality(current_shape,current_shape_rot,current_shape_x,current_shape_y);
        if (shape_illegal) {
            game_over = true;
        }
    }
    // Allow holding on new turn
    held_this_turn = false;
}

void reset() {
    total_lines_cleared = 0;
    score = 0;
    lvl = 0;
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
    held_this_turn = false;
    game_over = false;
}

void main_loop() {
    // Game logic
    if (game_over) {
        reset();
    }
    if (lc_tick == 0) {
        calc_shape_illegality(current_shape,current_shape_rot,current_shape_x,current_shape_y);
        if (shape_illegal) {
            collision_bug_occured = true;
        }
    }

    // Display
    print("Welcome to TetrOS!",WHITE,1,40-9);
    draw_boxes_and_grid();
    draw_stamped_shapes();
    calc_ghost_y();
    draw_ghost_shape();
    draw_current_shape();
    draw_next_shape();
    draw_held_shape();
    draw_indicators();
}

void tick_handler() {
    tick++;
    // Handle ticks (e.g. gravity or line clear flashing) if not paused
    if (!paused) {
        // Handle line clear flashing
        // The second condition of this if statement should be replaced, as currently it waits for the next tick that matches, creating inconsistency in when the flashes start
        if (lc_tick > 0 && tick % round(18*lc_flash_duration) == 0) {
            for (unsigned int y = 0; y < grid_height; y++) {
                // Only adjust if the line was cleared
                if (lines_cleared[y]) {
                    for (unsigned int x = 0; x < grid_width; x++) {
                        if (lc_tick % 2 == 0) {
                            // flash white
                            tilemap[y][x] = -1;
                        } else {
                            // flash black
                            tilemap[y][x] = 0;
                        }
                    }
                }
            }
            // Go to next state (flash white / black / finished)
            lc_tick--;
            // Prepare to continue gameplay
            if (lc_tick == 0) {
                // Make lines with no support fall
                for (unsigned int y = 0; y < grid_height; y++) {
                    if (lines_cleared[y]) {
                        for (unsigned int y2 = y; y2 > 0; y2--) {
                            for (unsigned int x = 0; x < grid_width; x++) {
                                tilemap[y2][x] = tilemap[y2-1][x];
                            }
                        }
                        // Un-mark line as cleared
                        lines_cleared[y] = false;
                    }
                }
                // Pause gravity of current shape
                anti_gravity_tick = round(gravity);
                // Spawn the new shape
                current_shape_x = spawn_x;
                current_shape_y = spawn_y;
                current_shape_rot = spawn_rot;
                current_shape = next_shape;
                next_shape = take_from_bag();
                // If the shape spawns illegaly, end the game
                calc_shape_illegality(current_shape,current_shape_rot,current_shape_x,current_shape_y);
                if (shape_illegal) {
                    game_over = true;
                }
            }
        }
        // Handle gravity if not flashing line clears
        if (lc_tick == 0) {
            // Don't fall if just soft dropped
            if (anti_gravity_tick > 0) {
                anti_gravity_tick--;
            }
            // The second condition of this if statement should be replaced, as currently it waits for the next tick that matches, creating inconsistency in anti-gravity length
            if (anti_gravity_tick == 0 && tick % round(gravity) == 0) {
                // Lower the shape
                current_shape_y++;
                // If the shape goes out of bounds, move it back in
                calc_shape_illegality(current_shape,current_shape_rot,current_shape_x,current_shape_y);
                if (shape_illegal) {
                    current_shape_y--;
                    stamp();
                }
            }
        }
    }

    // Run the main loop, and draw the screen
    main_loop();
    screen_update();

    outb(0x20, 0x20);
}
void key_handler() {
    // Get keycode
    unsigned char keycode = inb(0x60);
    // Don't allow input if paused, or lines are clearing, and ignore the "key up" keycode
    if (!paused && lc_tick == 0 && keycode != 224) {
        switch(keycode) {
            case 0x4B: // left arrow (move left)
                // Move shape left
                current_shape_x--;
                // If the shape goes out of bounds, move it back in
                calc_shape_illegality(current_shape,current_shape_rot,current_shape_x,current_shape_y);
                if (shape_illegal) {
                    current_shape_x++;
                }
                break;
            case 0x4D: // right arrow (move right)
                // Move shape right
                current_shape_x++;
                // If the shape goes out of bounds, move it back in
                calc_shape_illegality(current_shape,current_shape_rot,current_shape_x,current_shape_y);
                if (shape_illegal) {
                    current_shape_x--;
                }
                break;
            case 0x50: // down arrow (soft drop)
                // Move shape down
                current_shape_y++;
                // If the shape goes out of bounds, move it back in
                calc_shape_illegality(current_shape,current_shape_rot,current_shape_x,current_shape_y);
                if (shape_illegal) {
                    current_shape_y--;
                } else {
                    anti_gravity_tick = round(gravity);
                }
                break;
            case 0x39: // space (hard drop)
                calc_ghost_y();
                current_shape_y = ghost_shape_y;
                stamp();
                break;
            case 0x48: // up arrow (rotate right)
            case 0x2D: // x (rotate right)
                // Rotate shape right
                current_shape_rot++;
                // Clip rotation to range 1-4
                if (current_shape_rot >= 4) {
                    current_shape_rot = 0;
                }
                // If the shape goes out of bounds, move it back in
                calc_shape_illegality(current_shape,current_shape_rot,current_shape_x,current_shape_y);
                if (shape_illegal) {
                    current_shape_rot--;
                    if (current_shape_rot < 0) {
                        current_shape_rot = 3;
                    }
                }
                break;
            case 0x2C: // z (rotate left)
                // Rotate shape left
                current_shape_rot--;
                // Clip rotation to range 1-4
                if (current_shape_rot < 0) {
                    current_shape_rot = 3;
                }
                // If the shape goes out of bounds, move it back in
                calc_shape_illegality(current_shape,current_shape_rot,current_shape_x,current_shape_y);
                if (shape_illegal) {
                    current_shape_rot++;
                    if (current_shape_rot >= 4) {
                        current_shape_rot = 0;
                    }
                }
                break;
            case 0x2E: // c (hold)
                // Don't let the player hold more than once per turn
                if (!held_this_turn) {
                    // If no shape is currently held, hold the current shape and generate a new next shape, else swap the held shape and current shape
                    if (held_shape == unset) {
                        held_shape = current_shape;
                        current_shape = next_shape;
                        next_shape = take_from_bag();
                    } else {
                        enum ShapeID temp_shape = current_shape;
                        current_shape = held_shape;
                        held_shape = temp_shape;
                    }
                    // Set the spawn position and rotation of the new current shape
                    current_shape_x = spawn_x;
                    current_shape_y = spawn_y;
                    current_shape_rot = 0;
                    // If the shape spawns illegaly, end the game
                    calc_shape_illegality(current_shape,current_shape_rot,current_shape_x,current_shape_y);
                    if (shape_illegal) {
                        game_over = true;
                    }
                    // Don't let the player hold more than once per turn
                    held_this_turn = true;
                }
                break;
            case 0x13: // r (reset)
                reset();
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
    // OS Stuff
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