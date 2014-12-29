#include "camera.h"
#include "vec2.h"
#include "rocket.h"
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

void draw_bg();

#define GRID_SIZE 200
#define GRID_SCALE 0.5
vec2f *bg;

int main() {
    // Initialize the OpenGL context and framebuffer
    OGL_Init();
    atexit(OGL_Quit);

    bg = calloc(GRID_SIZE*GRID_SIZE, sizeof(vec2f));
    int x, y;
    for (y = 0; y < GRID_SIZE; y++) {
        for (x = 0; x < GRID_SIZE; x++) {
            bg[y*GRID_SIZE+x] = (vec2f) {(x-GRID_SIZE/2)*GRID_SCALE, (y-GRID_SIZE/2)*GRID_SCALE};
        }
    }

    // Set up the view
    asp = (float) raspiGL_screen_width / raspiGL_screen_height;
    glMatrixMode(GL_PROJECTION_MATRIX);
    glOrthof(-asp, asp, -1, 1, -1, 1);
    glMatrixMode(GL_MODELVIEW_MATRIX);
    glEnable(GL_POINT_SMOOTH);
    glClearColor(0, 0, 0, 1);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    struct camera cam = {
        .pos=v2zero,
        .scale=1
    };

    // Load our handler for ^C, should only quit
    signal(SIGINT, handle_interrupt);
    atexit(cleanup);

    // Setup the input handler using the default input device
    // (default is "/dev/input/event1")
    ei_setup(NULL);

    // Initialize the particle system for the rocket
    // and the rocket itself
    struct particle_system *rcs_ps = new_particle_system(512);
    if (rcs_ps == NULL) return EXIT_FAILURE;
    struct particle_system *main_ps = new_particle_system(2048);
    if (main_ps == NULL) return EXIT_FAILURE;
    main_ps->life = 5;
    main_ps->growth = 1;

    struct rocket ship = new_rocket();
    ship.rcs_particles = rcs_ps;
    ship.main_particles = main_ps;

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
        update_particles(rcs_ps);
        update_particles(main_ps);
        update_camera(&cam, &ship);

        camera_start(&cam);
        draw_bg();
        draw_rocket(&ship);
        draw_particles(rcs_ps);
        draw_particles(main_ps);
        camera_end();

        draw_rocket_gui(&ship);

        OGL_SwapBuffers();
        // Quit on escape key
        if (ei_key_down(KEY_ESC)) running = 0;
    }

    free_particle_system(rcs_ps);
    free_particle_system(main_ps);
    free(bg);
    return EXIT_SUCCESS;
}

void draw_bg() {
    glPointSize(4);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, bg);

    glDrawArrays(GL_POINTS, 0, GRID_SIZE*GRID_SIZE);
}
