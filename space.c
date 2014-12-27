#include "vec2.h"
#include <stdlib.h>

double randf() {
    return (rand() - RAND_MAX/2) / (double) (RAND_MAX/2.0);
}

// Make the aspect ratio available to everyone
float asp;
void loop(vec2*);

// You disappear past the far border and appear at
// the near boarder
#define BORDER 0.05
#define FAR_BORDER (BORDER + 0.01)
void loop(vec2 *pos) {
    if (pos->x > asp + FAR_BORDER)  pos->x = -asp - BORDER;
    if (pos->x < -asp - FAR_BORDER) pos->x = asp + BORDER;
    if (pos->y > 1 + FAR_BORDER)    pos->y = -1 - BORDER;
    if (pos->y < -1 - FAR_BORDER)   pos->y = 1 + BORDER;
}
