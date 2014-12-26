#include "game_input.h"
#include "vec2.h"

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

void physics_input();

static volatile int running = 1;
void handle_interrupt(int dummy) {
    running = 0;
}

void cleanup() {
    printf("\033[H\033[2J");
}

#define NUM_BODY   4
#define START_BODY 0
#define NUM_THRUST 3
#define MAIN_THRUST_START  4
#define LEFT_THRUST_START  7
#define RIGHT_THRUST_START 10
#define LINE_START 13

static float data[NUM_BODY*2 + NUM_THRUST*2*3 + 4] = {
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
   // Straight Line
   0, 0,
   1, 0
};

struct rocket {
    vec2 accel, vel, pos;
    float angle, angle_vel,
          angle_force, thrust, max_rcs_fuel,
          max_main_fuel, rcs_fuel, main_fuel,
          rcs_fuel_rate, main_fuel_rate;
    int damping;
};

static float input_x = 0;
static float input_y = 0;

static struct rocket ship = {
    .angle_force = 0.1,
    .thrust = 0.0001,
    .max_rcs_fuel = 100,
    .rcs_fuel = 100,
    .rcs_fuel_rate = 0.02,
    .max_main_fuel = 3000,
    .main_fuel = 3000,
    .main_fuel_rate = 0.3,
};

int main() {
    OGL_Init();
    signal(SIGINT, handle_interrupt);
    atexit(cleanup);
    atexit(OGL_Quit);
    setup_input();

    float asp = (float) raspiGL_screen_width / raspiGL_screen_height;
    printf("%f\n", asp);
    glMatrixMode(GL_PROJECTION_MATRIX);
    glOrthof(-asp, asp, -1, 1, -1, 1);
    glMatrixMode(GL_MODELVIEW_MATRIX);
    glPointSize(4);
    glClearColor(0, 0, 0, 1);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, data);
    while (running) {
        glPushMatrix();
        glClear(GL_COLOR_BUFFER_BIT);

        handle_input();
        input_x = 0, input_y = 0, ship.damping = 0;
        if (key_down(KEY_UP))    input_y += 1;
        if (key_down(KEY_DOWN))  input_y -= 1;
        if (key_down(KEY_LEFT))  input_x -= 1;
        if (key_down(KEY_RIGHT)) input_x += 1;
        if (key_down(KEY_S))     ship.damping = 1;

        // Collide with the edges of the screen
        if (ship.pos.x > asp)  ship.pos.x = asp;
        if (ship.pos.x < -asp) ship.pos.x = -asp;
        if (ship.pos.y > 1)    ship.pos.y = 1;
        if (ship.pos.y < -1)   ship.pos.y = -1;

        glTranslatef(ship.pos.x, ship.pos.y, 0);
        glScalef(0.05, 0.05, 0.05);
        glRotatef(-ship.angle, 0, 0, 1);

        glDrawArrays(GL_LINE_LOOP, 0, NUM_BODY);
        // Input also draws the thrust effects
        physics_input();
        glPopMatrix();

        glPushMatrix();
        glTranslatef(-asp + 0.1, 0.9, 0);
        glPushMatrix();
        glScalef(ship.main_fuel/ship.max_main_fuel, 1, 1);
        glDrawArrays(GL_LINES, LINE_START, 2);
        glPopMatrix();
        glPushMatrix();
        glScalef(ship.rcs_fuel/ship.max_rcs_fuel, 1, 1);
        glTranslatef(0, -0.05, 0);
        glDrawArrays(GL_LINES, LINE_START, 2);
        glPopMatrix();
        glPopMatrix();

        OGL_SwapBuffers();
        if (key_down(KEY_ESC)) running = 0;
    }

    return EXIT_SUCCESS;
}

void physics_input() {
    // Enable stabilization, fire opposite rotation
    // (0.5 because half as powerful as normal thrusters)
    if (ship.damping) input_x += ship.angle_vel < 0 ? .5 : -.5;

    // Maneuvering thrusters
    if (ship.rcs_fuel > 0) {
        ship.angle_vel += input_x * ship.angle_force;
        // Draw the RCS thrusters if the rotation controls are enabled
        if (input_x > 0) {
            glDrawArrays(GL_LINE_STRIP, LEFT_THRUST_START, NUM_THRUST);
            ship.rcs_fuel -= ship.rcs_fuel_rate * input_x;
        } else if (input_x < 0) {
            glDrawArrays(GL_LINE_STRIP, RIGHT_THRUST_START, NUM_THRUST);
            ship.rcs_fuel -= ship.rcs_fuel_rate * -input_x;
        }
    } else if (ship.rcs_fuel < 0) ship.rcs_fuel = 0;


    // Linear thruster
    if (ship.main_fuel > 0) {
        // Don't let the thruster be fired backwards
        if (input_y < 0) input_y = 0;
        if (input_y > 0) {
            glDrawArrays(GL_LINE_STRIP, MAIN_THRUST_START, NUM_THRUST);
            ship.main_fuel -= ship.main_fuel_rate * input_y;
        }
        ship.accel.x = sin(ship.angle*M_PI/180) * input_y * ship.thrust;
        ship.accel.y = cos(ship.angle*M_PI/180) * input_y * ship.thrust;
    } else if (ship.main_fuel < 0) ship.main_fuel = 0;

    // Perform the physics
    v2inc(&ship.vel, &ship.accel);
    v2inc(&ship.pos, &ship.vel);
    ship.angle += ship.angle_vel;
}
