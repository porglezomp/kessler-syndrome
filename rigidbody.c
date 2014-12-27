#include "rigidbody.h"

#include "vec2.h"

#include <stdlib.h>
#include <math.h>

vec2 rb_point_velocity(const struct rigidbody *rb, const vec2 *relative_pos) {
    vec2 vel = rb->vel;
    float vel_rads = rb->angle_vel*M_PI/180;
    float rads = rb->angle*M_PI/180;
    float r = v2len(relative_pos);
    float fac = r*vel_rads;
    vec2 rotational = (vec2) {fac*cos(rads), -fac*sin(rads)};
    v2inc(&vel, &rotational);
    return vel;
}

void update_rigidbody(struct rigidbody *rbody) {
    // We don't mind multiplying force in-place because we're
    // going to set it to zero in a moment.
    // Divide by the mass to get acceleration
    v2muli(&rbody->force, 1/rbody->mass);

    // Perform euler integration
    v2inc(&rbody->vel, &rbody->force);
    v2inc(&rbody->pos, &rbody->vel);
    rbody->angle_vel += rbody->torque/rbody->moment_inertia;
    rbody->angle += rbody->angle_vel;

    // Acceleration needs to be provided per timestep, don't let it accumulate
    rbody->force = v2zero;
    rbody->torque = 0;
}

void rb_apply_force(struct rigidbody *rb, const vec2 *point, const vec2 *force) {
    // TODO: Apply force based on the point
    v2inc(&rb->force, force);
}
