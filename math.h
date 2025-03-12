#ifndef MATH_H
#define MATH_H

int round(float n) {
    return (int)(n * 10 + 0.5) / 10.;
}
int floor(float n) {
    return (int) n;
}

#endif