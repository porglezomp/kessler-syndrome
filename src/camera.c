#include "camera.h"

#include "vec2.h"
#include "rocket.h"

#include "GLES/gl.h"

void update_camera(struct camera *cam, const struct rocket *r) {
    vec2 offset = r->rbody.vel;
    v2muli(&offset, 10);
    double max_size = 1/cam->scale;
    if (v2len(&offset) > max_size) {
        v2norm(&offset);
        v2muli(&offset, max_size);
    }
    cam->pos = v2add(&r->rbody.pos, &offset);
}

void camera_start(const struct camera *cam) {
    glPushMatrix();
    glScalef(cam->scale, cam->scale, cam->scale);
    glTranslatef(-cam->pos.x, -cam->pos.y, 0);
}

void camera_end() {
    glPopMatrix();
}
