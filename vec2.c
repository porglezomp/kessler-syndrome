#include "vec2.h"
#include "math.h"

// Provide some useful constants
vec2 v2zero = {0, 0};
vec2 v2one = {1, 1};
vec2 v2right = {1, 0};
vec2 v2left = {-1, 0};
vec2 v2up = {0, 1};
vec2 v2down = {0, -1};

vec2 v2angle(float angle) {
    float rads = angle/180*M_PI;
    // Swap sin and cos so that 0 degrees points up
    // I think this makes angles increase clockwise?
    vec2 res = {sin(rads), cos(rads)};
    return res;
}

// lhs + rhs
vec2 v2add(const vec2 *lhs, const vec2 *rhs) {
    vec2 res = {lhs->x + rhs->x, lhs->y + rhs->y};
    return res;
}

// lhs += rhs
void v2inc(vec2 *lhs, const vec2 *rhs) {
    lhs->x += rhs->x;
    lhs->y += rhs->y;
}

// lhs - rhs
vec2 v2sub(const vec2 *lhs, const vec2 *rhs) {
    vec2 res = {lhs->x - rhs->x, lhs->y - rhs->y};
    return res;
}

// lhs -= rhs
void v2dec(vec2 *lhs, const vec2 *rhs) {
    lhs->x -= rhs->x;
    lhs->y -= rhs->y;
}

// dot product
float v2dot(const vec2 *lhs, const vec2 *rhs) {
    return (lhs->x * rhs->x) + (lhs->y * rhs->y);
}

// vec * scalar
vec2 v2mul(const vec2 *vec, float s) {
    vec2 res = {vec->x * s, vec->y * s};
    return res;
}

// vec *= scalar
void v2muli(vec2 *vec, float s) {
    vec->x *= s;
    vec->y *= s;
}


