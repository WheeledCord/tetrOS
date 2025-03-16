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

// Randomness
#define RAND_MAX = 2147483647
static unsigned long rand_state = 6783489;
void srand(unsigned long seed) {
    rand_state = seed;
}
long rand() {
    rand_state = (rand_state * 1103515245 + 12345) % 2147483648;
    return rand_state;
}
int randInt(int max) {
    return rand() % max;
}

#endif