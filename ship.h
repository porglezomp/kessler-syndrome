#ifndef _SHIP_H
#define _SHIP_H

#include "vec2.h"

struct rigidbody {
    vec2 pos, vel, accel;
    double angle, angle_vel, angle_accel;
};

struct rocket {
    struct rigidbody rbody;
    vec2 input;
    float angle_force, thrust, max_rcs_fuel,
          max_main_fuel, rcs_fuel, main_fuel,
          rcs_fuel_rate, main_fuel_rate;
    int damping;
    struct particle_system *rcs_particles;
};

void input_physics(struct rocket *s);
void update_rocket(struct rocket *s);
void draw_rocket(const struct rocket *s);

#endif
