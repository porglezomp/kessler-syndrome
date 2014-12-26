#include "vec2.h"
#include "ship.h"

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

    float asp = (float) raspiGL_screen_width / raspiGL_screen_height;
    printf("%f\n", asp);
    glMatrixMode(GL_PROJECTION_MATRIX);
    glOrthof(-asp, asp, -1, 1, -1, 1);
    glMatrixMode(GL_MODELVIEW_MATRIX);
    glPointSize(4);
    glClearColor(0, 0, 0, 1);
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

#define BORDER 0.05
#define BORDER_FUDGE 0.01
        // Loop around!
        if (ship.pos.x > asp + BORDER + BORDER_FUDGE)  ship.pos.x = -asp - BORDER;
        if (ship.pos.x < -asp - BORDER - BORDER_FUDGE) ship.pos.x = asp + BORDER;
        if (ship.pos.y > 1 + BORDER + BORDER_FUDGE)    ship.pos.y = -1 - BORDER;
        if (ship.pos.y < -1 - BORDER - BORDER_FUDGE)   ship.pos.y = 1 + BORDER;

        draw_rocket(&ship);

        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(2, GL_FLOAT, 0, line_mesh);
        glPushMatrix();
        glTranslatef(-asp + 0.1, 0.9, 0);
        glPushMatrix();
        glScalef(ship.main_fuel/ship.max_main_fuel, 1, 1);
        glDrawArrays(GL_LINES, 0, 2);
        glPopMatrix();
        glPushMatrix();
        glScalef(ship.rcs_fuel/ship.max_rcs_fuel, 1, 1);
        glTranslatef(0, -0.05, 0);
        glDrawArrays(GL_LINES, 0, 2);
        glPopMatrix();
        glPopMatrix();

        OGL_SwapBuffers();
        if (ei_key_down(KEY_ESC)) running = 0;
    }

    return EXIT_SUCCESS;
}

