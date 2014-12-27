#include "gui.h"

#include "ship.h"
#include "space.h"
#include "GLES/gl.h"

static float line_mesh[4] = {
    0, 0,
    1, 0
};

void draw_rocket_gui(const struct rocket *ship) {
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
