#include "../src/vec2.h"

#include <stdio.h>

void printvec(const vec2 v) {
    printf("(%.02f %.02f)\n", v.x, v.y);
}

int main() {
    vec2 v = {0, 1};
    printvec(v);
    v2roti(&v, 90);
    printvec(v);
    v2roti(&v, 45);
    printvec(v);
    printvec(v2rot(&v, 45+180));
    printf("%i %i\n", (1<<0), (1<<1));
    return 0;
}
