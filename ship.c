#include "game_input.h"
#include "vec2.h"
#include "ship.h"

#include "PiGL.h"
#include "GLES/gl.h"
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <linux/input.h>
#include <math.h>

#define NUM_BODY   4
#define START_BODY 0
#define NUM_THRUST 3
#define MAIN_THRUST_START  4
#define LEFT_THRUST_START  7
#define RIGHT_THRUST_START 10

static float ship_mesh[NUM_BODY*2 + NUM_THRUST*2*3] = {
    // Body
    0,  0,
    1, -1,
    0,  3,
   -1, -1,
   // Main thrust
   -.5, -.5,
   0, -2,
   .5, -.5,
   // Right thrust
   -.33, 2,
   -1, 2.33,
   -.2, 2.5,
   // Left thrust
   .33, 2,
   1, 2.33,
   .2, 2.5,
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
        s->accel.x = sin(s->angle*M_PI/180) * s->input.y * s->thrust;
        s->accel.y = cos(s->angle*M_PI/180) * s->input.y * s->thrust;
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
    if (s->rcs_fuel > 0 && s->input.x != 0) {
        // Draw the RCS thrusters if the rotation controls are enabled
        if (s->input.x > 0) glDrawArrays(GL_LINE_STRIP, LEFT_THRUST_START, NUM_THRUST);
        else glDrawArrays(GL_LINE_STRIP, RIGHT_THRUST_START, NUM_THRUST);
    }

    // Linear thruster
    if (s->main_fuel > 0 && s->input.y > 0) {
        glDrawArrays(GL_LINE_STRIP, MAIN_THRUST_START, NUM_THRUST);
    }
    glPopMatrix();
}

void physics(struct rocket *s) {
    v2inc(&s->vel, &s->accel);
    v2inc(&s->pos, &s->vel);
    s->angle += s->angle_vel;
    s->accel = v2zero;
}