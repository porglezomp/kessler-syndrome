#include "vec2.h"
#include <stdlib.h>

#define RAND_HALF (RAND_MAX / 2)
double randf() {
    return (rand() - RAND_HALF) / (double) RAND_HALF;
}

float asp;
void loop(vec2*);

#define BORDER 0.05
#define LARGE_BORDER (BORDER + 0.01)
void loop(vec2 *pos) {
    if (pos->x > asp + LARGE_BORDER)  pos->x = -asp - BORDER;
    if (pos->x < -asp - LARGE_BORDER) pos->x = asp + BORDER;
    if (pos->y > 1 + LARGE_BORDER)    pos->y = -1 - BORDER;
    if (pos->y < -1 - LARGE_BORDER)   pos->y = 1 + BORDER;
}
