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
    vec2 rotate_v2(vec2 v, vec2 c, int angle) {
        int x = v.x - c.x;
        int y = v.y - c.y;
    
        switch ((angle % 4 + 4) % 4) { // Ensures -1 → 3, -2 → 2, -3 → 1
            case 1:  return v2(-y + c.x, x + c.y);
            case 2:  return v2(-x + c.x, -y + c.y);
            case 3:  return v2(y + c.x, -x + c.y);
            default: return v;
        }
    }
#pragma endregion 

// Shape
struct Shape { char bg,fg; vec2 ui_offset; vec2 rotations[4][4]; }; typedef struct Shape Shape;

// GravityProgressionLevel
struct GravityProgressionLevel { float amount; unsigned int levelsSize; unsigned int levels[8]; }; typedef struct GravityProgressionLevel GravityProgressionLevel;

#endif