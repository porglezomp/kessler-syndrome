#ifndef _CAMERA_H
#define _CAMERA_H

#include "vec2.h"
#include "rocket.h"

struct camera {
    vec2 pos;
    double scale;
};

void camera_start(const struct camera *);
void camera_end();
void update_camera(struct camera*, const struct rocket*);

#endif
