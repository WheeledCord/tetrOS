#ifndef CLASSES_H
#define CLASSES_H

// Booleans
#define bool _Bool
#define true 1
#define false 0

// Null
#define null 0

// Shapes
enum ShapeID {
    o,
    i,
    l,
    j,
    s,
    z,
    t,
    unset
};

struct Shape {
    char color_bg;
    char color_fg;
    int data[4][4][4];
};


#endif