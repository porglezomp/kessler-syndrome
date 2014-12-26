#ifndef _SHIP_H
#define _SHIP_H

#include "vec2.h"

struct rocket {
    vec2 accel, vel, pos, input;
    float angle, angle_vel,
          angle_force, thrust, max_rcs_fuel,
          max_main_fuel, rcs_fuel, main_fuel,
          rcs_fuel_rate, main_fuel_rate;
    int damping;
    struct particle_system *rcs_particles;
};

void input_physics(struct rocket *s);
void physics(struct rocket *s);
void draw_rocket(const struct rocket *s);

#endif
