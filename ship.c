#include "ship.h"

#include "vec2.h"
#include "particles.h"
#include "space.h"

#include "GLES/gl.h"
#include <stdlib.h>

// Offsets and lengths in the ship mesh
#define NUM_BODY   4
#define START_BODY 0
#define NUM_THRUST 3
#define THRUST_START  4

// Parameters for emitting RCS particles
#define SPREAD 5
#define NUM_PARTICLES 16
#define DISK_RADIUS 0.005

static float ship_mesh[NUM_BODY*2 + NUM_THRUST*2] = {
    // Body
    0,  0,
    1, -1,
    0,  3,
   -1, -1,
   // Main thrust
   -.5, -.5,
   0, -2,
   .5, -.5,
};

void input_physics(struct rocket *s) {
    // Enable stabilization, fire opposite rotation
    // (0.5 because half as powerful as normal thrusters)
    if (s->damping) s->input.x += s->angle_vel < 0 ? .5 : -.5;

    // Maneuvering thrusters
    if (s->rcs_fuel > 0 && s->input.x != 0) {
        s->angle_vel += s->input.x * s->angle_force;
        // Consume fuel proportional to the force
        s->rcs_fuel -= s->rcs_fuel_rate * abs(s->input.x);
    } else if (s->rcs_fuel < 0) s->rcs_fuel = 0;

    // Linear thruster (can't fire backwards)
    if (s->main_fuel > 0 && s->input.y > 0) {
        s->main_fuel -= s->main_fuel_rate * s->input.y;
        vec2 direction = v2angle(s->angle);
        v2muli(&direction, s->input.y * s->thrust);
        s->accel = direction;
    } else if (s->main_fuel < 0) s->main_fuel = 0;
}

void draw_rocket(const struct rocket *s) {
    // Use the rocket model
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, ship_mesh);

    // Perform rocket transforms
    glPushMatrix();
    glTranslatef(s->pos.x, s->pos.y, 0);
    glScalef(0.05, 0.05, 0.05);
    glRotatef(-s->angle, 0, 0, 1);

    // Draw the main rocket
    glDrawArrays(GL_LINE_LOOP, 0, NUM_BODY);

    // Maneuvering thrusters
    // Draw the RCS jets if the rotation controls are enabled
    if (s->rcs_fuel > 0 && s->input.x != 0) {
        vec2 forward = v2angle(s->angle);
        v2muli(&forward, 0.1);
        // The center point to emit from
        // TODO: Emit from either side of this
        vec2 point = v2add(&s->pos, &forward);
        float angle_offset = 0;
        if (s->input.x > 0) angle_offset = -90;
        else angle_offset = 90;

        // The central angle to emit at
        float angle = s->angle + angle_offset;

        for (int i = 0; i < NUM_PARTICLES; i++) {
            // Randomly vary the direction a little bit
            vec2 dir = v2angle(angle + randf() * SPREAD);
            v2muli(&dir, 0.01);
            v2inc(&dir, &s->vel);
            // Emit on a random disk to prevent stripe artifacts
            vec2 modifier = v2angle(randf()*180);
            v2muli(&modifier, DISK_RADIUS);
            vec2 point2 = v2add(&point, &modifier);

            // Emit each particle with the parameters
            emit(s->rcs_particles, &point2, &dir);
        }
    }

    // Linear thruster
    if (s->main_fuel > 0 && s->input.y > 0) {
        glDrawArrays(GL_LINE_STRIP, THRUST_START, NUM_THRUST);
    }

    // Back to global space for rendering
    glPopMatrix();
}

void update_rocket(struct rocket *s) {
    v2inc(&s->vel, &s->accel);
    v2inc(&s->pos, &s->vel);
    s->angle += s->angle_vel;
    // Acceleration needs to be provided by controls
    // don't let it accumulate
    s->accel = v2zero;
}
