#include "vec2.h"
#include "ship.h"
#include "particles.h"
#include "space.h"

#include "easyinput.h"
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

void draw_rocket_gui(const struct rocket*);
static volatile int running = 1;
void handle_interrupt(int dummy) {
    running = 0;
}

void cleanup() {
    printf("\033[H\033[2J");
}

static float line_mesh[4] = {
    0, 0,
    1, 0
};

int main() {
    OGL_Init();
    signal(SIGINT, handle_interrupt);
    atexit(cleanup);
    atexit(OGL_Quit);
    // Setup the input handler using the default input device
    // ("/dev/input/event1")
    ei_setup(NULL);

    struct rocket ship = {
        .angle_force = 0.1,
        .thrust = 0.0001,
        .max_rcs_fuel = 100,
        .rcs_fuel = 100,
        .rcs_fuel_rate = 0.02,
        .max_main_fuel = 3000,
        .main_fuel = 3000,
        .main_fuel_rate = 0.3,
    };

    asp = (float) raspiGL_screen_width / raspiGL_screen_height;
    printf("%f\n", asp);
    glMatrixMode(GL_PROJECTION_MATRIX);
    glOrthof(-asp, asp, -1, 1, -1, 1);
    glMatrixMode(GL_MODELVIEW_MATRIX);
    glPointSize(2.2);
    glEnable(GL_POINT_SMOOTH);
    glClearColor(0, 0, 0, 1);

    struct particle_system *ps = new_particle_system(2048);
    if (ps == NULL) {
        return EXIT_FAILURE;
    }
    ship.rcs_particles = ps;

    while (running) {
        glClear(GL_COLOR_BUFFER_BIT);

        ship.input = v2zero, ship.damping = 0;
        ei_poll_all();
        if (ei_key_down(KEY_UP))    ship.input.y += 1;
        if (ei_key_down(KEY_DOWN))  ship.input.y -= 1;
        if (ei_key_down(KEY_LEFT))  ship.input.x -= 1;
        if (ei_key_down(KEY_RIGHT)) ship.input.x += 1;
        if (ei_key_down(KEY_S))     ship.damping = 1;
        input_physics(&ship);
        physics(&ship);
        update_particles(ps);

        // Loop around!
        loop(&ship.pos);

        draw_rocket(&ship);
        draw_rocket_gui(&ship);
        glColor4f(.3, .3, .3, 1);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        draw_particles(ps);
        glColor4f(1, 1, 1, 1);

        OGL_SwapBuffers();
        if (ei_key_down(KEY_ESC)) running = 0;
    }

    free_particle_system(ps);
    return EXIT_SUCCESS;
}

void draw_rocket_gui(const struct rocket *ship) {
    // Draw our straight line
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, line_mesh);

    glPushMatrix();
    glTranslatef(-asp + 0.1, 0.9, 0);

    // Draw the main fuel indicator
    glPushMatrix();
    glScalef(ship->main_fuel/ship->max_main_fuel, 1, 1);
    glDrawArrays(GL_LINES, 0, 2);
    glPopMatrix();

    // Draw the rcs fuel indicator
    glPushMatrix();
    glScalef(ship->rcs_fuel/ship->max_rcs_fuel, 1, 1);
    glTranslatef(0, -0.05, 0);
    glDrawArrays(GL_LINES, 0, 2);
    glPopMatrix();

    glPopMatrix();
}
