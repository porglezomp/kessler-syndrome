#ifndef _PARTICLES_H
#define _PARTICLES_H

#include "vec2.h"

struct particle {
    vec2 pos;
    vec2 vel;
    // Need to include g, b, a for coloring the particle
    // life is treated as the red channel, g, b, and a are
    // set to be the same as life in the update loop
    float life, g, b, a, radius;
};

struct particle_system {
    struct particle *particles;
    int offset, count, nparticles;
};

struct particle_system *new_particle_system(int nparticles);

struct particle *nth_particle(struct particle_system *ps, int n);
void emit(struct particle_system *ps, const vec2 *pos, const vec2 *vel);
void update_particles(struct particle_system *ps);
void draw_particles(struct particle_system *ps);
void kill_particle(struct particle_system *ps);

void free_particle_system(struct particle_system *ps);

#endif
