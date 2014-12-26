#include "PiGL.h"
#include "game_input.h"
#include "GLES/gl.h"
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <linux/input.h>
#include <math.h>

void panic() {
    fflush(stdout);
    fprintf(stderr, "%s.\n", strerror(errno));
    exit(EXIT_FAILURE);
}

static volatile int running = 1;
void handle_interrupt(int dummy) {
    running = 0;
}

void cleanup() {
    printf("\033[H\033[2J");
}

static float pos_x = 0;
static float pos_y = 0;
static float force = 0.0001;

#define NUM_BODY   4
#define START_BODY 0
#define NUM_THRUST 3
#define MAIN_THRUST_START  4
#define LEFT_THRUST_START  7
#define RIGHT_THRUST_START 10

static float data[NUM_BODY*2 + NUM_THRUST*2*3] = {
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
   .2, 2.5
};

static float accel_x = 0;
static float accel_y = 0;
static float vel_x = 0;
static float vel_y = 0;
static float angle = 0;
static float angle_vel = 0;
static float input_x = 0;
static float input_y = 0;
const static float angle_force = 0.1;
const static float bounce = 0.5;
static int damping = 0;

void input();

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

        input_x = 0, input_y = 0, damping = 0;
        if (key_down(KEY_UP))    input_y += 1;
        if (key_down(KEY_DOWN))  input_y -= 1;
        if (key_down(KEY_LEFT))  input_x -= 1;
        if (key_down(KEY_RIGHT)) input_x += 1;
        if (key_down(KEY_S))     damping = 1;

        // Collide with the edges of the screen
        if (pos_x > asp)  pos_x = asp;
        if (pos_x < -asp) pos_x = -asp;
        if (pos_y > 1)    pos_y = 1;
        if (pos_y < -1)   pos_y = -1;

        glTranslatef(pos_x, pos_y, 0);
        glScalef(0.05, 0.05, 0.05);
        glRotatef(-angle, 0, 0, 1);

        glDrawArrays(GL_LINE_LOOP, 0, NUM_BODY);
        input();

        OGL_SwapBuffers();
        glPopMatrix();
    }

    printf("%d %d\n", raspiGL_screen_width, raspiGL_screen_height);
    return EXIT_SUCCESS;
}

void input() {
    if (damping) input_x += angle_vel < 0 ? .5 : -.5;

    angle_vel += input_x * angle_force;
    angle += angle_vel;
    if (input_x > 0) {
        glDrawArrays(GL_LINE_STRIP, LEFT_THRUST_START, NUM_THRUST);
    } else if (input_x < 0) {
        glDrawArrays(GL_LINE_STRIP, RIGHT_THRUST_START, NUM_THRUST);
    }
    if (input_y < 0) input_y = 0;
    if (input_y > 0) {
        glDrawArrays(GL_LINE_STRIP, MAIN_THRUST_START, NUM_THRUST);
    }
    accel_x = sin(angle*M_PI/180) * input_y * force;
    accel_y = cos(angle*M_PI/180) * input_y * force;
    vel_x += accel_x;
    vel_y += accel_y;
    pos_x += vel_x;
    pos_y += vel_y;
}
