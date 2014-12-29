#include "rocket.h"

#include "vec2.h"
#include "particles.h"
#include "space.h"

#include "GLES/gl.h"
#include <stdlib.h>
#include <math.h>

// Offsets and lengths in the ship mesh
//#define START_BODY 0
//#define NUM_THRUST 3
//#define THRUST_START  4

// Parameters for emitting RCS particles
#define SPREAD 10 // degrees
#define NUM_PARTICLES 16
#define DISK_RADIUS 0.01
#define PARTICLE_VEL 0.02

struct rocket new_rocket() {
    struct rigidbody rbody = {
        .mass=100000,
        .moment_inertia=5
    };
    struct rocket r = {
        .scale=.03,
        .angle_force=1,
        .thrust=30,
        .max_rcs_fuel=100,
        .rcs_fuel=100,
        .rcs_fuel_rate=0.02,
        .max_main_fuel=3000,
        .main_fuel=3000,
        .main_fuel_rate=0.3,
        .rbody=rbody
    };
    return r;
}

#define NUM_VERTS 30
static float ship_mesh[NUM_VERTS*2] = {
    // Main border (0)
    1, 4,
    2.5, 3,
    3.5, -2,
    2.5, -3,
    1.5, -3.2,
    -1.5, -3.2,
    -2.5, -3,
    -3.5, -2,
    -2.5, 3,
    -1, 4,
    // Back container (10)
    -1, -1,
    -1.5, -3,
    -1.5, -3.5,
    -1.2, -4,
    1.2, -4,
    1.5, -3.5,
    1.5, -3,
    1, -1,
    // Cockpit (18)
    -0.6, 2,
    0.6, 2,
    // Bumpers (20)
    3, 2,
    3.5, 0,
    -3, 2,
    -3.5, 0,
    // Engines (24)
    -3.25, -2.2,
    -3.33, -3.5,
    -1.5, -3.7,
    1.5, -3.7,
    3.33, -3.5,
    3.25, -2.2
};

#define NUM_BODY (32*2)
static unsigned short ship_indices[NUM_BODY] = {
    0, 1,
    1, 2,
    2, 3,
    3, 4,
    5, 6,
    6, 7,
    7, 8,
    8, 9,
    9, 0,
    10, 11,
    11, 12,
    12, 13,
    13, 14,
    14, 15,
    15, 16,
    16, 17,
    17, 10,
    10, 18,
    18, 19,
    19, 17,
    1, 20,
    20, 21,
    21, 2,
    8, 22,
    22, 23,
    23, 7,
    24, 25,
    25, 26,
    27, 28,
    28, 29
};

static vec2 rcs_points[4] = {
    {0.05, 0.1},
    {-0.05, 0.1},
    {0.07, -0.1},
    {-0.07, -0.1}
};

static float rcs_angles[4] = { 90, -90, 90, -90 };

void input_physics(struct rocket *s) {
    // Enable stabilization, fire opposite rotation
    // (0.5 because half as powerful as normal thrusters)
    if (s->damping) s->input.x += s->rbody.angle_vel < 0 ? .5 : -.5;

    // Maneuvering thrusters
    if (s->rcs_fuel > 0 && s->input.x != 0) {
        int index;
        if (s->input.x > 0) index = 1;
        else index = 0;
        // The center point to emit from
        vec2 point = rcs_points[index];
        v2roti(&point, s->rbody.angle);
        v2inc(&point, &s->rbody.pos);

        double mag_x = fabs(s->input.x);
        vec2 thrust = v2angle(s->rbody.angle + rcs_angles[index]);
        v2muli(&thrust, s->angle_force * -mag_x);
        rb_apply_force(&s->rbody, &point, &thrust);
        // Consume fuel proportional to the force
        s->rcs_fuel -= s->rcs_fuel_rate * mag_x;
    } else if (s->rcs_fuel < 0) s->rcs_fuel = 0;

    // Linear thruster (can't fire backwards)
    if (s->main_fuel > 0 && s->input.y > 0) {
        s->main_fuel -= s->main_fuel_rate * s->input.y;
        vec2 direction = v2angle(s->rbody.angle);
        v2muli(&direction, s->input.y * s->thrust);
        rb_apply_force(&s->rbody, &s->rbody.pos, &direction);
    } else if (s->main_fuel < 0) s->main_fuel = 0;
}

void draw_rocket(const struct rocket *s) {
    // Perform rocket transforms
    glPushMatrix();
    glTranslatef(s->rbody.pos.x, s->rbody.pos.y, 0);
    glScalef(s->scale, s->scale, s->scale);
    glRotatef(-s->rbody.angle, 0, 0, 1);

    // Draw the main rocket
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, ship_mesh);
    glDrawElements(GL_LINES, NUM_BODY, GL_UNSIGNED_SHORT, ship_indices);

    // Maneuvering thrusters
    // Draw the RCS jets if the rotation controls are enabled
    // Particle emission
    if (s->rcs_fuel > 0 && s->input.x != 0) {

        int index;
        // Fire differently for left and right input
        if (s->input.x > 0) index = 1;
        else index = 0;
        vec2 point = rcs_points[index];

        // Move the origin of the thruster from local coordinates into
        // world coordinates
        v2roti(&point, s->rbody.angle);
        vec2 relative_vel = rb_point_velocity(&s->rbody, &point);
        v2inc(&point, &s->rbody.pos);

        // The central angle to emit at
        float angle = s->rbody.angle + rcs_angles[index];

        for (int i = 0; i < NUM_PARTICLES; i++) {
            // Randomly vary the direction a little bit
            // in order to produce a cone of particles
            vec2 dir = v2angle(angle + randf() * SPREAD);
            v2muli(&dir, PARTICLE_VEL);
            v2inc(&dir, &relative_vel);
            // Emit on a random disk to prevent stripe artifacts
            vec2 modifier = v2angle(randf()*180);
            v2muli(&modifier, DISK_RADIUS*randf());
            vec2 point2 = v2add(&point, &modifier);

            // Emit each particle with the parameters
            emit(s->rcs_particles, &point2, &dir);
        }
    }

    // Linear thruster
    if (s->main_fuel > 0 && s->input.y > 0) {
        int i;
        vec2 origin = s->rbody.pos;
        vec2 backwards = v2angle(s->rbody.angle+180);
        v2muli(&backwards, 0.1);
        v2inc(&origin, &backwards);
//        vec2 time_offset = rb_point_velocity(&s->rbody, &origin);

        for (i = 0; i < 16; i++) {
//            double time = 64.0/i;

            vec2 vector = v2angle(s->rbody.angle+180+randf()*SPREAD);
            v2muli(&vector, 0.02);
            v2inc(&vector, &s->rbody.vel);

            vec2 disk = v2angle(randf()*180);
            v2muli(&disk, 0.01);
            vec2 origin2 = origin;
  //          vec2 delta = v2mul(&time_offset, time);
//            v2inc(&origin2, &delta);
            v2inc(&origin2, &disk);
            emit(s->main_particles, &origin2, &vector);
        }
    }

    // Back to global space for rendering
    glPopMatrix();
}
