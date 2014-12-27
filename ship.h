#ifndef _SHIP_H
#define _SHIP_H

#include "rigidbody.h"
#include "vec2.h"

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
void draw_rocket(const struct rocket *s);
struct rocket new_rocket();

#endif
