#include "vec2.h"
#include "ship.h"
#include "particles.h"
#include "gui.h"
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

static volatile int running = 1;
void handle_interrupt(int dummy) {
    running = 0;
}

void cleanup() {
    printf("\033[H\033[2J");
}

int main() {
    // Initialize the OpenGL context and framebuffer
    OGL_Init();
    atexit(OGL_Quit);

    // Set up the view
    asp = (float) raspiGL_screen_width / raspiGL_screen_height;
    glMatrixMode(GL_PROJECTION_MATRIX);
    glOrthof(-asp, asp, -1, 1, -1, 1);
    glMatrixMode(GL_MODELVIEW_MATRIX);
    glEnable(GL_POINT_SMOOTH);
    glClearColor(0, 0, 0, 1);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    // Load our handler for ^C, should only quit
    signal(SIGINT, handle_interrupt);
    atexit(cleanup);

    // Setup the input handler using the default input device
    // (default is "/dev/input/event1")
    ei_setup(NULL);

    // Initialize the particle system for the rocket
    // and the rocket itself
    struct particle_system *ps = new_particle_system(2048);
    if (ps == NULL) return EXIT_FAILURE;
    struct rocket ship = {
        .angle_force = 0.1,
        .thrust = 0.0001,
        .max_rcs_fuel = 100,
        .rcs_fuel = 100,
        .rcs_fuel_rate = 0.02,
        .max_main_fuel = 3000,
        .main_fuel = 3000,
        .main_fuel_rate = 0.3,
        .rcs_particles = ps
    };

    // Mainloop
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
        update_rigidbody(&ship.rbody);
        update_particles(ps);

        // Loop the ship around the screen!
        loop(&ship.rbody.pos);

        draw_rocket(&ship);
        draw_rocket_gui(&ship);
        draw_particles(ps);

        OGL_SwapBuffers();
        // Quit on escape key
        if (ei_key_down(KEY_ESC)) running = 0;
    }

    free_particle_system(ps);
    return EXIT_SUCCESS;
}

