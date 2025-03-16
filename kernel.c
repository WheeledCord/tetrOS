#include "game_settings.h"
#include "kernel.h"

// Ticks
    unsigned int tick = 0;
    unsigned int lc_tick = 0;
    unsigned int lc_start_tick = 0;
    unsigned int anti_gravity_tick = 0;
    unsigned int anti_gravity_slide_tick = 0;
    unsigned int gravity_start_tick = 0;

// Times
    Time session_start;
    Time round_start;

// Game Variables
    int current_shape_x,current_shape_y,ghost_shape_y,current_shape_rot;
    unsigned int bag[total_shapes];
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
    bool show_ghost = default_show_ghost;
    bool do_wall_kicks = default_do_wall_kicks;
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

// Drawing to Screen
    void draw_boxes_and_grid() {
        // Get stats labels width
        int stats_label_width = sizeof(lvl_label)-1;

        // Get strings for grid and box characters, using ascii set if in ascii mode
        char chars[11][2];
        for (unsigned int i = 0; i < 11; i++) {
            if (i < 2) {
                if (ascii_mode) {
                    chars[i][0] = grid_chars_ascii[i];
                } else {
                    chars[i][0] = grid_chars[i];
                }
            } else {
                if (ascii_mode) {
                    chars[i][0] = box_chars_ascii[i-2];
                } else {
                    chars[i][0] = box_chars[i-2];
                }
            }
            chars[i][1] = 0x00;
        }
        char shape_ui_filler[3] = "  ";
        if (show_shape_ui_grids) {
            shape_ui_filler[0] = chars[0][0];
            shape_ui_filler[1] = chars[1][0];
        }
        // Draw the play area
            // Draw the top edge
            unsigned int x = 0;
            print(chars[2],box_colour,add_v2(play_area_top_left,v2(x++,0))); // top left corner
            for (unsigned int i = 0; i < grid_width*2; i++) {
                print(chars[3],box_colour,add_v2(play_area_top_left,v2(x++,0))); // top edge
            }
            print(chars[4],box_colour,add_v2(play_area_top_left,v2(x++,0))); // top right corner
            // Draw the left and right edges, and the grid
            for (unsigned int y = 1; y <= grid_height; y++) {
                x = 0;
                print(chars[5],box_colour,add_v2(play_area_top_left,v2(x++,y))); // left edge
                for (unsigned int i = 0; i < grid_width; i++) {
                    print(chars[0],grid_colour,add_v2(play_area_top_left,v2(x++,y))); // left half of tile on grid
                    print(chars[1],grid_colour,add_v2(play_area_top_left,v2(x++,y))); // right half of tile on grid
                }
                print(chars[5],box_colour,add_v2(play_area_top_left,v2(x++,y))); // right edge
            }
            // Draw the bottom edge
            x = 0;
            print(chars[6],box_colour,add_v2(play_area_top_left,v2(x++,grid_height+1))); // bottom left corner
            for (unsigned int i = 0; i < grid_width*2; i++) {
                print(chars[7],box_colour,add_v2(play_area_top_left,v2(x++,grid_height+1))); // bottom edge
            }
            print(chars[8],box_colour,add_v2(play_area_top_left,v2(x++,grid_height+1))); // bottom right corner

        // Draw the next shape box
            // Draw the top edge
            x = 0;
            print(chars[2],box_colour,add_v2(next_shape_top_left,v2(x++,0))); // top left corner
            for (unsigned int i = 0; i < 8; i++) {
                print(chars[3],box_colour,add_v2(next_shape_top_left,v2(x++,0))); // top edge
            }
            print(chars[4],box_colour,add_v2(next_shape_top_left,v2(x++,0))); // top right corner
            // Draw the label + left and right edges
            print(chars[5],box_colour,add_v2(next_shape_top_left,v2(0,1))); // left edge
            print(next_label,box_colour,add_v2(next_shape_top_left,v2(1,1))); // label
            print(chars[5],box_colour,add_v2(next_shape_top_left,v2(1+8,1))); // right edge
            // Draw the left and right joints, and the seperating line
            x = 0;
            print(chars[9],box_colour,add_v2(next_shape_top_left,v2(x++,2))); // left joint
            for (unsigned int i = 0; i < 8; i++) {
                print(chars[7],box_colour,add_v2(next_shape_top_left,v2(x++,2))); // seperating line
            }
            print(chars[10],box_colour,add_v2(next_shape_top_left,v2(x++,2))); // right joint
            // Draw the left and right edges, and the space
            for (unsigned int y = 1; y <= 4; y++) {
                x = 0;
                print(chars[5],box_colour,add_v2(next_shape_top_left,v2(x++,2+y))); // left edge
                for (unsigned int i = 0; i < 4; i++) {
                    print(shape_ui_filler,grid_colour,add_v2(next_shape_top_left,v2(x++,2+y))); // space
                    x++;
                }
                print(chars[5],box_colour,add_v2(next_shape_top_left,v2(x++,2+y))); // right edge
            }
            // Draw the bottom edge
            x = 0;
            print(chars[6],box_colour,add_v2(next_shape_top_left,v2(x++,2+5))); // bottom left corner
            for (unsigned int i = 0; i < 8; i++) {
                print(chars[7],box_colour,add_v2(next_shape_top_left,v2(x++,2+5))); // bottom edge
            }
            print(chars[8],box_colour,add_v2(next_shape_top_left,v2(x++,2+5))); // bottom right corner

        // Draw the held shape box
            // Draw the top edge
            x = 0;
            print(chars[2],box_colour,add_v2(held_shape_top_left,v2(x++,0))); // top left corner
            for (unsigned int i = 0; i < 8; i++) {
                print(chars[3],box_colour,add_v2(held_shape_top_left,v2(x++,0))); // top edge
            }
            print(chars[4],box_colour,add_v2(held_shape_top_left,v2(x++,0))); // top right corner
            // Draw the label + left and right edges
            print(chars[5],box_colour,add_v2(held_shape_top_left,v2(0,1))); // left edge
            print(held_label,box_colour,add_v2(held_shape_top_left,v2(1,1))); // label
            print(chars[5],box_colour,add_v2(held_shape_top_left,v2(1+8,1))); // right edge
            // Draw the left and right joints, and the seperating line
            x = 0;
            print(chars[9],box_colour,add_v2(held_shape_top_left,v2(x++,2))); // left joint
            for (unsigned int i = 0; i < 8; i++) {
                print(chars[7],box_colour,add_v2(held_shape_top_left,v2(x++,2))); // seperating line
            }
            print(chars[10],box_colour,add_v2(held_shape_top_left,v2(x++,2))); // right joint
            // Draw the left and right edges, and the space
            for (unsigned int y = 1; y <= 4; y++) {
                x = 0;
                print(chars[5],box_colour,add_v2(held_shape_top_left,v2(x++,2+y))); // left edge
                for (unsigned int i = 0; i < 4; i++) {
                    print(shape_ui_filler,grid_colour,add_v2(held_shape_top_left,v2(x++,2+y))); // space
                    x++;
                }
                print(chars[5],box_colour,add_v2(held_shape_top_left,v2(x++,2+y))); // right edge
            }
            // Draw the bottom edge
            x = 0;
            print(chars[6],box_colour,add_v2(held_shape_top_left,v2(x++,2+5))); // bottom left corner
            for (unsigned int i = 0; i < 8; i++) {
                print(chars[7],box_colour,add_v2(held_shape_top_left,v2(x++,2+5))); // bottom edge
            }
            print(chars[8],box_colour,add_v2(held_shape_top_left,v2(x++,2+5))); // bottom right corner
        
        // Draw the lvl stats box
            // Draw the top edge
            x = 0;
            print(chars[2],box_colour,add_v2(stat_lvl_top_left,v2(x++,0))); // top left corner
            for (unsigned int i = 0; i < stats_label_width; i++) {
                print(chars[3],box_colour,add_v2(stat_lvl_top_left,v2(x++,0))); // top edge
            }
            print(chars[4],box_colour,add_v2(stat_lvl_top_left,v2(x++,0))); // top right corner
            // Draw the label + left and right edges
            print(chars[5],box_colour,add_v2(stat_lvl_top_left,v2(0,1))); // left edge
            print(lvl_label,box_colour,add_v2(stat_lvl_top_left,v2(1,1))); // label
            print(chars[5],box_colour,add_v2(stat_lvl_top_left,v2(1+stats_label_width,1))); // right edge
            // Draw the left and right joints, and the seperating line
            x = 0;
            print(chars[9],box_colour,add_v2(stat_lvl_top_left,v2(x++,2))); // left joint
            for (unsigned int i = 0; i < stats_label_width; i++) {
                print(chars[7],box_colour,add_v2(stat_lvl_top_left,v2(x++,2))); // seperating line
            }
            print(chars[10],box_colour,add_v2(stat_lvl_top_left,v2(x++,2))); // right joint
            // Draw the left and right edges, and the space
            x = 0;
            print(chars[5],box_colour,add_v2(stat_lvl_top_left,v2(x++,2+1))); // left edge
            for (unsigned int i = 0; i < stats_label_width; i++) {
                print(" ",grid_colour,add_v2(stat_lvl_top_left,v2(x++,2+1))); // space
            }
            print(chars[5],box_colour,add_v2(stat_lvl_top_left,v2(x++,2+1))); // right edge
            // Draw the bottom edge
            x = 0;
            print(chars[6],box_colour,add_v2(stat_lvl_top_left,v2(x++,2+2))); // bottom left corner
            for (unsigned int i = 0; i < stats_label_width; i++) {
                print(chars[7],box_colour,add_v2(stat_lvl_top_left,v2(x++,2+2))); // bottom edge
            }
            print(chars[8],box_colour,add_v2(stat_lvl_top_left,v2(x++,2+2))); // bottom right corner

        // Draw the lines stats box
            // Draw the top edge
            x = 0;
            print(chars[2],box_colour,add_v2(stat_lines_top_left,v2(x++,0))); // top left corner
            for (unsigned int i = 0; i < stats_label_width; i++) {
                print(chars[3],box_colour,add_v2(stat_lines_top_left,v2(x++,0))); // top edge
            }
            print(chars[4],box_colour,add_v2(stat_lines_top_left,v2(x++,0))); // top right corner
            // Draw the label + left and right edges
            print(chars[5],box_colour,add_v2(stat_lines_top_left,v2(0,1))); // left edge
            print(lines_label,box_colour,add_v2(stat_lines_top_left,v2(1,1))); // label
            print(chars[5],box_colour,add_v2(stat_lines_top_left,v2(1+stats_label_width,1))); // right edge
            // Draw the left and right joints, and the seperating line
            x = 0;
            print(chars[9],box_colour,add_v2(stat_lines_top_left,v2(x++,2))); // left joint
            for (unsigned int i = 0; i < stats_label_width; i++) {
                print(chars[7],box_colour,add_v2(stat_lines_top_left,v2(x++,2))); // seperating line
            }
            print(chars[10],box_colour,add_v2(stat_lines_top_left,v2(x++,2))); // right joint
            // Draw the left and right edges, and the space
            x = 0;
            print(chars[5],box_colour,add_v2(stat_lines_top_left,v2(x++,2+1))); // left edge
            for (unsigned int i = 0; i < stats_label_width; i++) {
                print(" ",grid_colour,add_v2(stat_lines_top_left,v2(x++,2+1))); // space
            }
            print(chars[5],box_colour,add_v2(stat_lines_top_left,v2(x++,2+1))); // right edge
            // Draw the bottom edge
            x = 0;
            print(chars[6],box_colour,add_v2(stat_lines_top_left,v2(x++,2+2))); // bottom left corner
            for (unsigned int i = 0; i < stats_label_width; i++) {
                print(chars[7],box_colour,add_v2(stat_lines_top_left,v2(x++,2+2))); // bottom edge
            }
            print(chars[8],box_colour,add_v2(stat_lines_top_left,v2(x++,2+2))); // bottom right corner

        // Draw the score stats box
            // Draw the top edge
            x = 0;
            print(chars[2],box_colour,add_v2(stat_score_top_left,v2(x++,0))); // top left corner
            for (unsigned int i = 0; i < stats_label_width; i++) {
                print(chars[3],box_colour,add_v2(stat_score_top_left,v2(x++,0))); // top edge
            }
            print(chars[4],box_colour,add_v2(stat_score_top_left,v2(x++,0))); // top right corner
            // Draw the label + left and right edges
            print(chars[5],box_colour,add_v2(stat_score_top_left,v2(0,1))); // left edge
            print(score_label,box_colour,add_v2(stat_score_top_left,v2(1,1))); // label
            print(chars[5],box_colour,add_v2(stat_score_top_left,v2(1+stats_label_width,1))); // right edge
            // Draw the left and right joints, and the seperating line
            x = 0;
            print(chars[9],box_colour,add_v2(stat_score_top_left,v2(x++,2))); // left joint
            for (unsigned int i = 0; i < stats_label_width; i++) {
                print(chars[7],box_colour,add_v2(stat_score_top_left,v2(x++,2))); // seperating line
            }
            print(chars[10],box_colour,add_v2(stat_score_top_left,v2(x++,2))); // right joint
            // Draw the left and right edges, and the space
            x = 0;
            print(chars[5],box_colour,add_v2(stat_score_top_left,v2(x++,2+1))); // left edge
            for (unsigned int i = 0; i < stats_label_width; i++) {
                print(" ",grid_colour,add_v2(stat_score_top_left,v2(x++,2+1))); // space
            }
            print(chars[5],box_colour,add_v2(stat_score_top_left,v2(x++,2+1))); // right edge
            // Draw the bottom edge
            x = 0;
            print(chars[6],box_colour,add_v2(stat_score_top_left,v2(x++,2+2))); // bottom left corner
            for (unsigned int i = 0; i < stats_label_width; i++) {
                print(chars[7],box_colour,add_v2(stat_score_top_left,v2(x++,2+2))); // bottom edge
            }
            print(chars[8],box_colour,add_v2(stat_score_top_left,v2(x++,2+2))); // bottom right corner
    }

    void draw_current_shape() {
        // Don't draw anything if clearing line
        if (lc_tick == 0) {
            // Get the shape data
            Shape shape = shapes[current_shape];
            // Get the relative points
            vec2 *shape_points = shape.rotations[current_shape_rot];
            // Get shape colour. If in ascii mode, only colour foreground, else colour both background and foreground
            char colour;
            if (ascii_mode) {
                colour = shape.bg;
            } else {
                colour = colour_combo(shape.bg,shape.fg);
            }
            // Plot each point onto the grid
            for (unsigned int p = 0; p < 4; p++) {
                print("[]",colour,add_v2(play_area_top_left,v2(1+2*(current_shape_x+shape_points[p].x),1+current_shape_y+shape_points[p].y)));
            }
        }
    }

    void draw_ghost_shape() {
        // Don't draw anything if clearing line
        if (lc_tick == 0) {
            // Get the relative points
            vec2 *shape_points = shapes[current_shape].rotations[current_shape_rot];
            // Get shape colour. If in ascii mode, only colour foreground, else colour both background and foreground
            char colour;
            if (ascii_mode) {
                colour = shapes[unset].bg;
            } else {
                colour = colour_combo(shapes[unset].bg,shapes[unset].fg);
            }
            // Plot the points onto the grid
            for (unsigned int p = 0; p < 4; p++) {
                print("[]",colour,add_v2(play_area_top_left,v2(1+2*(current_shape_x+shape_points[p].x),1+ghost_shape_y+shape_points[p].y)));
            }
        }
    }

    void draw_next_shape() {
        // Get the shape data
        Shape shape = shapes[next_shape];
        // Get the relative points for the default rotation
        vec2 *shape_points = shape.rotations[0];
        // Get shape colour. If in ascii mode, only colour foreground, else colour both background and foreground
        char colour;
        if (ascii_mode) {
            colour = shape.bg;
        } else {
            colour = colour_combo(shape.bg,shape.fg);
        }
        // Plot the points onto the indicator space grid
        for (unsigned int p = 0; p < 4; p++) {
            print("[]",colour,add_v2(next_shape_top_left,v2(1+2*shape_points[p].x+shape.ui_offset.x,3+shape_points[p].y+shape.ui_offset.y)));
        }
    }

    void draw_held_shape() {
        // Don't draw anything if no shape is held
        if (held_shape != unset) {
            // Get the shape data
            Shape shape = shapes[held_shape];
            // Get the relative points for the default rotation
            vec2 *shape_points = shape.rotations[0];
            // Get shape colour. If in ascii mode, only colour foreground, else colour both background and foreground
            char colour;
            if (ascii_mode) {
                colour = shape.bg;
            } else {
                colour = colour_combo(shape.bg,shape.fg);
            }
            // Plot the points onto the indicator space grid
            for (unsigned int p = 0; p < 4; p++) {
                print("[]",colour,add_v2(held_shape_top_left,v2(1+2*shape_points[p].x+shape.ui_offset.x,3+shape_points[p].y+shape.ui_offset.y)));
            }
        }
    }

    void draw_stamped_shapes() {
        for (unsigned int y = 0; y < grid_height; y++) {
            for (int x = 0; x < grid_width; x++) {
                int tile = tilemap[y][x];
                // Select shape colour
                char bg,fg;
                if (tile == -1) { // line flashing
                    bg = flash_colour;
                    fg = flash_colour;
                } else if (tile != 0) { // non empty tiles
                    bg = shapes[tile-1].bg;
                    fg = shapes[tile-1].fg;
                }
                // If in ascii mode, only colour foreground, else colour both background and foreground
                char colour;
                if (ascii_mode) {
                    colour = bg;
                } else {
                    colour = colour_combo(bg,fg);
                }
                // Plot the point onto the indicator space grid
                if (tile != 0) {
                    print("[]",colour,add_v2(play_area_top_left,v2(1+2*x,1+y)));
                }
            }
        }
    }

    void draw_indicators() {
        if (paused) {
            printc(paused_indicator,paused_indicator_colour,paused_indicator_pos);
        }
        if (collision_bug_occured) {
            printc(collision_bug_indicator,collision_bug_indicator_colour,collision_bug_indicator_pos);
        }
    }

    void draw_stats() {
        // Center text to box - Level
        char lvl_str[4];
        itoa(lvl_str,lvl,10);
        unsigned int len = get_str_length(lvl_str);
        unsigned int offset = 0;
        switch (len) {
            case 1: offset = 1; break;
            case 2: offset = 0; break;
            case 3: offset = 0; break;
        }
        print(lvl_str,stat_colour,add_v2(stat_lvl_top_left,v2(4+offset,3)));
        // Center text to box - Lines
        char lines_str[4];
        itoa(lines_str,total_lines_cleared,10);
        len = get_str_length(lines_str);
        offset = 0;
        switch (len) {
            case 1: offset = 1; break;
            case 2: offset = 0; break;
            case 3: offset = 0; break;
            case 4: offset = 0; break;
        }
        print(lines_str,stat_colour,add_v2(stat_lines_top_left,v2(4+offset,3)));
        // Center text to box - Score
        char score_str[4];
        itoa(score_str,score,10);
        len = get_str_length(score_str);
        offset = 0;
        switch (len) {
            case 1: offset = 2; break;
            case 2: offset = 1; break;
            case 3: offset = 1; break;
            case 4: offset = 0; break;
            case 5: offset = 0; break;
            case 6: offset = 0; break;
        }
        print(score_str,stat_colour,add_v2(stat_score_top_left,v2(3+offset,3)));
    }

    void draw_time() {
        Time time = read_time();
        char time_str[get_str_length(time_message)+1];
        format_time(time_str,time_message,time);
        print(time_str,time_colour,time_pos);

        char session_time_str[get_str_length(session_time_message)+1];
        Time session_time = time_from_seconds(time.unix_time-session_start.unix_time);
        format_time(session_time_str,session_time_message,session_time);
        print(session_time_str,time_colour,session_time_pos);

        char round_time_str[get_str_length(round_time_message)+1];
        Time round_time = time_from_seconds(time.unix_time-round_start.unix_time);
        format_time(round_time_str,round_time_message,round_time);
        print(round_time_str,time_colour,round_time_pos);
    }

// Bag
    void refil_bag() {
        // Empty the bag
        for (unsigned int i = 0; i < total_shapes; i++) {
            bag[i] = 0;
        }
        // Keep generating shapes untill the bag is full
        unsigned int i = 0;
        while (i < total_shapes) {
            // Generate random shape
            int item = 1+randInt(total_shapes);
            // Check if shape is already in the bag, if so, don't add it
            bool item_in_bag = false;
            for (unsigned int ii = 0; ii < total_shapes; ii++) {
                if (bag[ii] == item) {
                    item_in_bag = true;
                    break;
                }
            }
            if (!item_in_bag) {
                bag[i++] = item;
            }
        }
    }
    enum ShapeID take_from_bag() {
        // Test if bag is empty
        unsigned int i;
        bool bag_empty = true;
        for (i = 0; i < total_shapes; i++) {
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
        for (i = 0; i < total_shapes; i++) {
            if (bag[i] != 0) {
                enum ShapeID selected = bag[i]-1;
                bag[i] = 0;
                return selected;
            }
        }
    }

// Game logic functions
bool shape_illegalities[5];
bool shape_illegal = false;
void calc_shape_illegality(enum ShapeID shapeId, int rot, int cx, int cy) {
    // Get the relative points
    vec2 *shape_points = shapes[shapeId].rotations[rot];
    for (unsigned int i = 0; i < 5; i++) {
        shape_illegalities[i] = false;
    }
    for (unsigned int p = 0; p < 4; p++) {
        // Get tile x and y
        int x = cx+shape_points[p].x;
        int y = cy+shape_points[p].y;
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
    // Keep moving down untill landed
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
        lc_start_tick = tick;
        // Update total lines cleared counter
        total_lines_cleared += lines_cleared_amount;
        unsigned int old_level = lvl;
        // Update level
        lvl = floor(total_lines_cleared / lines_per_level);
        if (lvl != old_level) {
            // Progress gravity
            for (unsigned int i = 0; i < total_gravity_progression_levels; i++) {
                GravityProgressionLevel p = gravity_progression[i];
                bool progressed = false;
                for (unsigned int ii = 0; ii < p.levelsSize; ii++) {
                    if (lvl == p.levels[ii]) {
                        progressed = true;
                        gravity -= p.amount;
                        break;
                    }
                }
                if (progressed) {
                    break;
                }
            }
        }
        // Update score
        unsigned int handled_lines = 0;
        while (handled_lines < lines_cleared_amount) {
            if (lines_cleared_amount-handled_lines >= 4) {
                score += score_add_tetris*(lvl+1);
                handled_lines += 4;
            } else if (lines_cleared_amount-handled_lines >= 3) {
                score += score_add_triple*(lvl+1);
                handled_lines += 3;
            } else if (lines_cleared_amount-handled_lines >= 2) {
                score += score_add_double*(lvl+1);
                handled_lines += 2;
            } else if (lines_cleared_amount-handled_lines >= 1) {
                score += score_add_single*(lvl+1);
                handled_lines++;
            }
        }
    }
    return lines_cleared_amount;
}

void stamp() {
    // Get the relative points
    vec2 *shape_points = shapes[current_shape].rotations[current_shape_rot];
    for (unsigned int p = 0; p < 4; p++) {
        // Plot the point onto the tilemap
        tilemap[current_shape_y+shape_points[p].y][current_shape_x+shape_points[p].x] = ((int)current_shape)+1;
    }
    // Test for line clears
    unsigned int lines_cleared_amount = check_line_clears();
    // Get new shape if no lines cleared, if there is, the new shape will be assigned once the lines stop flashing
    if (lines_cleared_amount == 0) {
        current_shape_x = spawn_pos.x;
        current_shape_y = spawn_pos.y;
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
    current_shape_x = spawn_pos.x;
    current_shape_y = spawn_pos.y;
    current_shape_rot = spawn_rot;
    gravity = gravity_progression[0].amount;
    refil_bag();
    current_shape = take_from_bag();
    next_shape = take_from_bag();
    held_shape = unset;
    held_this_turn = false;
    game_over = false;
    round_start = read_time();
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

    // Max stats
    if (lvl > 999) {
        lvl = 999;
    }
    if (total_lines_cleared > 9999) {
        total_lines_cleared = 9999;
    }
    if (score > 999999) {
        score = 999999;
    }

    // Display
    printc(title_message,title_message_colour,title_message_pos);
    draw_boxes_and_grid();
    draw_stamped_shapes();
    calc_ghost_y();
    if (show_ghost) {
        draw_ghost_shape();
    }
    draw_current_shape();
    draw_next_shape();
    draw_held_shape();
    draw_stats();
    draw_indicators();
    draw_time();
}

void tick_handler() {
    tick++;
    // Handle ticks (e.g. gravity or line clear flashing) if not paused
    if (!paused) {
        // Handle line clear flashing
        // The second condition of this if statement should be replaced, as currently it waits for the next tick that matches, creating inconsistency in when the flashes start
        if (lc_tick > 0 && (tick-lc_start_tick) % round(18*lc_flash_duration) == 0) {
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
                current_shape_x = spawn_pos.x;
                current_shape_y = spawn_pos.y;
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
            // Allow for sliding ONLY WHEN the shape is on solid ground
            if (anti_gravity_slide_tick > 0) {
                // Check if on solid ground
                current_shape_y++;
                calc_shape_illegality(current_shape,current_shape_rot,current_shape_x,current_shape_y);
                current_shape_y--;
                if (!shape_illegal) {
                    // Stop slidng
                    anti_gravity_slide_tick = 0;
                } else {
                    anti_gravity_slide_tick--;
                }
            }
            // The second condition of this if statement should be replaced, as currently it waits for the next tick that matches, creating inconsistency in anti-gravity length
            if (anti_gravity_tick == 0 && anti_gravity_slide_tick == 0 && (tick-gravity_start_tick) % round(gravity) == 0) {
                // Lower the shape
                current_shape_y++;
                // If the shape goes out of bounds, move it back in
                calc_shape_illegality(current_shape,current_shape_rot,current_shape_x,current_shape_y);
                if (shape_illegal) {
                    current_shape_y--;
                    stamp();
                } else {
                    // Check if should slide
                    current_shape_y++;
                    calc_shape_illegality(current_shape,current_shape_rot,current_shape_x,current_shape_y);
                    current_shape_y--;
                    if (shape_illegal) {
                        anti_gravity_slide_tick = round(18*slide_time);
                    }
                }
                gravity_start_tick = tick;
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
            case K_left:
                // Move shape left
                current_shape_x--;
                // If the shape goes out of bounds, move it back in
                calc_shape_illegality(current_shape,current_shape_rot,current_shape_x,current_shape_y);
                if (shape_illegal) {
                    current_shape_x++;
                }
                break;
            case K_right:
                // Move shape right
                current_shape_x++;
                // If the shape goes out of bounds, move it back in
                calc_shape_illegality(current_shape,current_shape_rot,current_shape_x,current_shape_y);
                if (shape_illegal) {
                    current_shape_x--;
                }
                break;
            case K_soft_drop:
                // Move shape down
                current_shape_y++;
                // If the shape goes out of bounds, move it back in
                calc_shape_illegality(current_shape,current_shape_rot,current_shape_x,current_shape_y);
                if (shape_illegal) {
                    current_shape_y--;
                } else {
                    // If touching floor, allow slide time, else use anti gravity
                    current_shape_y++;
                    calc_shape_illegality(current_shape,current_shape_rot,current_shape_x,current_shape_y);
                    current_shape_y--;
                    if (shape_illegal) {
                        anti_gravity_slide_tick = round(18*slide_time);
                    } else {
                        anti_gravity_tick = round(gravity);
                    }
                    score += score_add_soft_drop;
                }
                break;
            case K_hard_drop:
                calc_ghost_y();
                score += score_add_hard_drop*(ghost_shape_y-current_shape_y);
                current_shape_y = ghost_shape_y;
                stamp();
                break;
            case K_rotate_right1:
            case K_rotate_right2:
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
            case K_rotate_left:
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
            case K_hold:
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
                    current_shape_x = spawn_pos.x;
                    current_shape_y = spawn_pos.y;
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
            case K_reset:
                reset();
                break;
        }
    }
    if (keycode == K_toggle_pause) {
        paused = !paused;
    }
    if (keycode == K_toggle_ascii) {
        ascii_mode = !ascii_mode;
    }
    if (keycode == K_toggle_ghost) {
        show_ghost = !show_ghost;
    }
    if (keycode == K_toggle_wall_kicks) {
        do_wall_kicks = !do_wall_kicks;
    }
    if (keycode == K_reset_settings) {
        ascii_mode = default_ascii_mode;
        show_ghost = default_show_ghost;
        do_wall_kicks = default_do_wall_kicks;
    }

    outb(0x20, 0x20);
}

void kernel_init() {
    // OS Stuff
    outb(0x3C6, 0xFF);

    // Seed the random generator
    session_start = read_time();
    round_start = read_time();
    srand(session_start.unix_time);

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