#ifndef _VEC2_H
#define _VEC2_H

typedef struct {
    float x, y;
} vec2;

extern vec2 v2zero;
extern vec2 v2one;
extern vec2 v2right;
extern vec2 v2left;
extern vec2 v2up;
extern vec2 v2down;

vec2 v2add(const vec2 *lhs, const vec2 *rhs);
void v2inc(vec2 *lhs, const vec2 *rhs);
vec2 v2sub(const vec2 *lhs, const vec2 *rhs);
void v2dec(vec2 *lhs, const vec2 *rhs);
float v2dot(const vec2 *lhs, const vec2 *rhs);

#endif
