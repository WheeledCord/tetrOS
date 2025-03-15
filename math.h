#ifndef MATH_H
#define MATH_H

int round(float n) {
    return (int)(n * 10 + 0.5) / 10.;
}
int floor(float n) {
    return (int) n;
}
int power_of(int n, int x) {
    int out = 1;
    for (unsigned int i = 0; i < x; i++) {
        out *= n;
    }
    return out;
}

#endif