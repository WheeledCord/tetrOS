#ifndef CLASSES_H
#define CLASSES_H

// Booleans
#define bool _Bool
#define true 1
#define false 0

// Null
#define null 0

// Vector2
#pragma region 
    struct vec2 { int x,y; }; typedef struct vec2 vec2;
    vec2 v2(int x, int y) { vec2 out = {x,y}; return out; }
    vec2 add_v2(vec2 a, vec2 b) { vec2 out = a; out.x += b.x; out.y += b.y; return out; }
    vec2 sub_v2(vec2 a, vec2 b) { vec2 out = a; out.x -= b.x; out.y -= b.y; return out; }
    vec2 mult_v2(vec2 a, vec2 b) { vec2 out = a; out.x *= b.x; out.y *= b.y; return out; }
    vec2 div_v2(vec2 a, vec2 b) { vec2 out = a; out.x /= b.x; out.y /= b.y; return out; }
#pragma endregion 

// Shape
struct Shape { char bg,fg; vec2 ui_offset; vec2 rotations[4][4]; }; typedef struct Shape Shape;

// GravityProgressionLevel
struct GravityProgressionLevel { float amount; unsigned int levelsSize; unsigned int levels[8]; }; typedef struct GravityProgressionLevel GravityProgressionLevel;

// Time
struct Time { int sec,min,hour,day,month,year; }; typedef struct Time Time;

#endif