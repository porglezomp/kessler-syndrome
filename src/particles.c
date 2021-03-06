#include "particles.h"

#include "vec2.h"

#include "GLES/gl.h"

#include <stdlib.h>
#include <math.h>

struct particle_system *new_particle_system(int nparticles) {
    struct particle_system *ps = malloc(sizeof(struct particle_system));
    if (ps == NULL) return NULL;
    struct particle *particles = calloc(nparticles, sizeof(struct particle));
    if (particles == NULL) {
        free(ps);
        return NULL;
    }
    // Default particle animation properties
    ps->nparticles = nparticles;
    ps->particles = particles;
    ps->offset = 0;
    ps->count = 0;
    ps->life = 0.5;
    ps->growth = 1;
    ps->life_dec = 0.05;
    ps->brightness = 8;
    return ps;
}

struct particle *nth_particle(struct particle_system *ps, int n) {
  // return the nth particle in the ring buffer, based on the offset
  int offset = (ps->offset + n) % ps->nparticles;
  return &ps->particles[offset];
}

void emit(struct particle_system *ps, const vec2 *pos, const vec2 *vel) {
    int n = ps->count;
    if (ps->count < ps->nparticles) {
        ps->count += 1;
    } else {
        // If the buffer is saturated, move the offset over one
        // (Removes the first and adds a new last)
        ps->offset = (ps->offset + 1) % ps->nparticles;
    }
    struct particle p = {.pos=v2tov2f(pos), .vel=v2tov2f(vel), .life=ps->life,
                         .r=0, .g=0, .b=0, .a=1, .radius=5};
    *nth_particle(ps, n) = p;
}

void update_particles(struct particle_system *ps) {
    int to_remove = 0;
    for (int i = 0; i < ps->count; i++) {
        struct particle *nth = nth_particle(ps, i);
        v2finc(&nth->pos, &nth->vel);
        nth->life -= ps->life_dec;
        nth->radius += ps->growth;

        float area = nth->radius*nth->radius;
        float color = ps->brightness*nth->life/area;
        nth->r = nth->g = nth->b = color;

        // The particles all have the same lifetime, so the oldest ones are always
        // at the front of the buffer. We can simply remove the n from the front of
        // the buffer if n particles have died.
        if (nth->life <= 0) to_remove += 1;
    }
    // Remove all of the dead particles
    kill_particles(ps, to_remove);
}

void draw_particles(struct particle_system *ps) {
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, sizeof(struct particle), &(ps->particles[0].pos));
    glEnableClientState(GL_POINT_SIZE_ARRAY_OES);
    glPointSizePointerOES(GL_FLOAT, sizeof(struct particle), &(ps->particles[0].radius));
    glEnableClientState(GL_COLOR_ARRAY);
    glColorPointer(4, GL_FLOAT, sizeof(struct particle), &(ps->particles[0].r));

    int drawcount = ps->count;
    if (ps->offset + ps->count >= ps->nparticles) {
        // Particles: [* * * * * . . . . . . . + + + + + + + +]
        // This part draws the *s
        drawcount = ps->nparticles - ps->offset;
        glDrawArrays(GL_POINTS, 0, ps->count - drawcount);
    }
    
    glDrawArrays(GL_POINTS, ps->offset, drawcount);

    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_POINT_SIZE_ARRAY_OES);
}

void kill_particle(struct particle_system *ps) {
    // Remove from the front of the buffer, since that's the oldest particle
    if (ps->count > 0) {
        ps->count -= 1;
        ps->offset = (ps->offset + 1) % ps->nparticles;
    }
}

void kill_particles(struct particle_system *ps, int n) {
  // Remove n particles from the front of the buffer
  // If there are less than n particles in the buffer, remove them all
  if (ps->count >= n) {
    ps->count -= n;
    ps->offset = (ps->offset + n) % ps->nparticles;
  } else {
    ps->count = ps->offset = 0;
  }
}

void free_particle_system(struct particle_system *ps) {
    free(ps->particles);
    free(ps);
}

