#ifndef _RIGIDBODY_H
#define _RIGIDBODY_H

#include "vec2.h"

struct rigidbody {
    vec2 pos, vel, force;
    double angle, angle_vel, torque;
    double mass, moment_inertia;
};

void update_rigidbody(struct rigidbody*);
vec2 rb_point_velocity(const struct rigidbody*, const vec2*);
void rb_apply_force(struct rigidbody*, const vec2*, const vec2*);

#endif
