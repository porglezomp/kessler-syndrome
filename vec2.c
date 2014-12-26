#include "vec2.h"

vec2 v2zero = {0, 0};
vec2 v2one = {1, 1};
vec2 v2right = {1, 0};
vec2 v2left = {-1, 0};
vec2 v2up = {0, 1};
vec2 v2down = {0, -1};

vec2 v2add(const vec2 *lhs, const vec2 *rhs) {
    vec2 res = {lhs->x + rhs->x, lhs->y + rhs->y};
    return res;
}

void v2inc(vec2 *lhs, const vec2 *rhs) {
    lhs->x += rhs->x;
    lhs->y += rhs->y;
}

vec2 v2sub(const vec2 *lhs, const vec2 *rhs) {
    vec2 res = {lhs->x - rhs->x, lhs->y - rhs->y};
    return res;
}

void v2dec(vec2 *lhs, const vec2 *rhs) {
    lhs->x -= rhs->x;
    lhs->y -= rhs->y;
}

float v2dot(const vec2 *lhs, const vec2 *rhs) {
    return (lhs->x * rhs->x) + (lhs->y * rhs->y);
}
