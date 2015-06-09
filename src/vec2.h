#ifndef _VEC2_H
#define _VEC2_H

typedef struct {
    double x, y;
} vec2;
typedef struct {
    float x, y;
} vec2f;

extern vec2 v2zero;  // [ 0  0]
extern vec2 v2one;   // [ 1  1]
extern vec2 v2right; // [ 1  0]
extern vec2 v2left;  // [-1  0]
extern vec2 v2up;    // [ 0  1]
extern vec2 v2down;  // [ 0 -1]

extern vec2f v2fzero;  // [ 0  0]
extern vec2f v2fone;   // [ 1  1]
extern vec2f v2fright; // [ 1  0]
extern vec2f v2fleft;  // [-1  0]
extern vec2f v2fup;    // [ 0  1]
extern vec2f v2fdown;  // [ 0 -1]

// 0 degrees is vertical
// Angles should increase clockwise
vec2 v2angle(double degrees);
// Convert a vec2f to a vec2
vec2 v2ftov2(const vec2f *v2);
// Convert a vec2 to a vec2f
vec2f v2tov2f(const vec2 *v);

// Arithmetic for vec2 (double)
vec2 v2add(const vec2 *lhs, const vec2 *rhs); // +
void v2inc(vec2 *lhs, const vec2 *rhs);       // +=
vec2 v2sub(const vec2 *lhs, const vec2 *rhs); // -
void v2dec(vec2 *lhs, const vec2 *rhs);       // -=
vec2 v2mul(const vec2 *vec, double s);        // *
void v2muli(vec2 *vec, double s);             // *=

// Arithmetic for vec2f (float)
void v2finc(vec2f*, const vec2f*);
void v2fmuli(vec2f*, float);

// Vector math for vec2 (double)
double v2dot(const vec2 *lhs, const vec2 *rhs);
vec2 v2proj(const vec2 *lhs, const vec2 *rhs); // project lhs onto rhs
double v2len(const vec2 *vec);
vec2 v2normed(const vec2 *vec);
void v2norm(vec2 *vec);
double v2cross(const vec2 *lhs, const vec2 *rhs);

// Transformations
vec2 v2rot(const vec2 *vec, double theta);
void v2roti(vec2 *vec, double theta);

// Vector math for vec2f (float)
float v2fdot(const vec2f*, const vec2f*);
float v2flen(const vec2f*);
void v2fnorm(vec2f*);

#endif
