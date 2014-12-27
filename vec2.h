#ifndef _VEC2_H
#define _VEC2_H

typedef struct {
    float x, y;
} vec2;

extern vec2 v2zero;  // [ 0  0]
extern vec2 v2one;   // [ 1  1]
extern vec2 v2right; // [ 1  0]
extern vec2 v2left;  // [-1  0]
extern vec2 v2up;    // [ 0  1]
extern vec2 v2down;  // [ 0 -1]

// 0 degrees is vertical
// Angles should increase clockwise
vec2 v2angle(float degrees);

vec2 v2add(const vec2 *lhs, const vec2 *rhs); // +
void v2inc(vec2 *lhs, const vec2 *rhs);       // +=
vec2 v2sub(const vec2 *lhs, const vec2 *rhs); // -
void v2dec(vec2 *lhs, const vec2 *rhs);       // -=
float v2dot(const vec2 *lhs, const vec2 *rhs);
vec2 v2mul(const vec2 *vec, float s);         // *
void v2muli(vec2 *vec, float s);              // *=

#endif
