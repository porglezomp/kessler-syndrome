#ifndef _SHIP_H
#define _SHIP_H

#include "rigidbody.h"
#include "vec2.h"
#include "kslmesh.h"

#define NUM_THRUSTERS 8
#define THRUSTER(n) (1<<(n-1))

extern vec2 rcs_points[NUM_THRUSTERS];

extern float rcs_angles[NUM_THRUSTERS];

struct rocket {
    struct rigidbody rbody;
    vec2 input;
    float angle_force, thrust, max_rcs_fuel,
          max_main_fuel, rcs_fuel, main_fuel,
          rcs_fuel_rate, main_fuel_rate, scale;
    ksl_mesh_handle mesh;
    int damping, active_thrusters, firing_thrusters;
    struct particle_system *rcs_particles, *main_particles;
};

void all_rcs(struct rocket *r);
void toggle_thruster(struct rocket *r, int index);
void fire_rcs(struct rocket *r, int index, float force);
void input_physics(struct rocket *s);
void draw_rocket(const struct rocket *s);
struct rocket new_rocket();

#endif
