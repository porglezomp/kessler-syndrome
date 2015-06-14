#include "rocket.h"

#include "vec2.h"
#include "particles.h"
#include "space.h"

#include "GLES/gl.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

// Parameters for emitting RCS particles
#define SPREAD 10 // degrees
#define NUM_PARTICLES 12
#define DISK_RADIUS 0.01
#define PARTICLE_VEL 0.02
#define log(x) {FILE *fd = fopen("game.log", "a"); fputs(x, fd); fclose(fd);}
#define logf(...) {FILE *fd = fopen("game.log", "a"); fprintf(fd, __VA_ARGS__); fclose(fd);}

struct rocket new_rocket() {
    struct rigidbody rbody = {
        .mass=100000,
        .moment_inertia=25
    };
    ksl_mesh_list *mesh_list = ksl_load_meshes("mesh/rocket.ksl", NULL);
    if (mesh_list == NULL) {
      log(ksl_get_error());
      log("\n");
      exit(1);
    }
    ksl_mesh_handle mesh = ksl_make_handle(mesh_list->meshes[0]);
    ksl_free_mesh_list(mesh_list);
    struct rocket r = {
        .scale=.03,
        .angle_force=2.5,
        .thrust=30,
        .max_rcs_fuel=100,
        .rcs_fuel=100,
        .rcs_fuel_rate=0.01,
        .max_main_fuel=3000,
        .main_fuel=3000,
        .main_fuel_rate=0.3,
        .rbody=rbody,
	.mesh=mesh
    };
    return r;
}

void toggle_thruster(struct rocket *r, int index) {
    r->active_thrusters ^= 1<<index;
}

void all_rcs(struct rocket *r) {
    int i;
    for (i = 0; i < NUM_THRUSTERS; i++) {
        if ((1<<i) & r->active_thrusters) {
            fire_rcs(r, i, 1);
        }
    }
}

void fire_rcs(struct rocket *r, int index, float force) {
    vec2 point = rcs_points[index];
    v2roti(&point, r->rbody.angle);
    v2inc(&point, &r->rbody.pos);

    vec2 thrust = v2angle(r->rbody.angle + rcs_angles[index]);
    v2muli(&thrust, r->angle_force * -force);
    rb_apply_force(&r->rbody, &point, &thrust);

    // Consume fuel proportional to the force
    r->rcs_fuel -= r->rcs_fuel_rate * force;
    // Set the bitflag for which thruster is firing
    r->firing_thrusters |= 1<<index;
}

void input_physics(struct rocket *s) {
    // Enable stabilization, fire opposite rotation
    // (0.5 because half as powerful as normal thrusters)
    if (s->damping) s->input.x += s->rbody.angle_vel < 0 ? .5 : -.5;

    // Maneuvering thrusters
    if (s->rcs_fuel > 0 && s->input.x != 0) {
        int index, index_map;
        if (s->input.x > 0) index_map = THRUSTER(2)|THRUSTER(3);
        else index_map = THRUSTER(1)|THRUSTER(4);
        // The center point to emit from
        for (index = 0; index < NUM_THRUSTERS; index++) {
            // Skip items not in the set of indices
            if (!((1<<index) & index_map)) continue;
            double mag_x = fabs(s->input.x);
            fire_rcs(s, index, mag_x);
        }
    } else if (s->rcs_fuel < 0) s->rcs_fuel = 0;

    // Linear thruster (can't fire backwards)
    if (s->main_fuel > 0 && s->input.y > 0) {
        s->main_fuel -= s->main_fuel_rate * s->input.y;
        vec2 direction = v2angle(s->rbody.angle);
        v2muli(&direction, s->input.y * s->thrust);
        rb_apply_force(&s->rbody, &s->rbody.pos, &direction);
    } else if (s->main_fuel < 0) s->main_fuel = 0;
}

void draw_rcs(const struct rocket *s, int index) {
   vec2 point = rcs_points[index];

   // Move the origin of the thruster from local coordinates into
   // world coordinates
   v2roti(&point, s->rbody.angle);
   vec2 relative_vel = rb_point_velocity(&s->rbody, &point);
   v2inc(&point, &s->rbody.pos);

   // The central angle to emit at
   float angle = s->rbody.angle + rcs_angles[index];

   for (int i = 0; i < NUM_PARTICLES; i++) {
       // Randomly vary the direction a little bit
       // in order to produce a cone of particles
       vec2 dir = v2angle(angle + randf() * SPREAD);
       v2muli(&dir, PARTICLE_VEL);
       v2inc(&dir, &relative_vel);
       // Emit on a random disk to prevent stripe artifacts
       vec2 modifier = v2angle(randf()*180);
       v2muli(&modifier, DISK_RADIUS*randf());
       vec2 point2 = v2add(&point, &modifier);

       // Emit each particle with the parameters
       emit(s->rcs_particles, &point2, &dir);
   }
}

vec2 main_thruster_points[2];
void draw_rocket(const struct rocket *s) {
    int i;
    ksl_mesh *mesh = s->mesh.shared_mesh;
    
    // Perform rocket transforms
    glPushMatrix();
    glTranslatef(s->rbody.pos.x, s->rbody.pos.y, 0);
    double scale = s->scale;
    glScalef(scale, scale, scale);
    glRotatef(-s->rbody.angle, 0, 0, 1);

    // Move to integer units using mesh scale
    glPushMatrix();
    scale = 1.0 / mesh->meter_size;
    glScalef(scale, scale, scale);

    // Draw the main rocket
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_SHORT, 0, mesh->verts);
    glDrawElements(GL_LINES, mesh->line_count*2, GL_UNSIGNED_SHORT, mesh->lines);

    glPopMatrix();

    // Maneuvering thrusters
    // Draw the RCS jets if the rotation controls are enabled
    // Particle emission
    if (s->rcs_fuel > 0 && s->firing_thrusters != 0) {
        for (i = 0; i < NUM_THRUSTERS; i++) {
            // If the index is in the set of firing thrusters, draw it
            if ((1<<i) & s->firing_thrusters) {
                draw_rcs(s, i);
            }
        }
    }

    glPushMatrix();

    glColor4f(1, 1, 1, 1);
    scale = 1/s->scale;
    glScalef(scale, scale, scale);
    vec2f temp_points[NUM_THRUSTERS];
    for (i = 0; i < NUM_THRUSTERS; i++) {
        temp_points[i] = v2tov2f(&rcs_points[i]);
    }
    glVertexPointer(2, GL_FLOAT, 0, temp_points);
    glPointSize(8);

    for (i = 0; i < NUM_THRUSTERS; i++) {
        if ((1<<i) & s->active_thrusters) {
            glDrawArrays(GL_POINTS, i, 1);
        }
    }
    glPopMatrix();

    // Linear thruster
    if (s->main_fuel > 0 && s->input.y > 0) {
        // Transform the origins of the thrusters into global coords
        vec2 thruster1 = main_thruster_points[0];
        vec2 thruster2 = main_thruster_points[1];
        v2roti(&thruster1, s->rbody.angle);
        v2roti(&thruster2, s->rbody.angle);
        v2inc(&thruster1, &s->rbody.pos);
        v2inc(&thruster2, &s->rbody.pos);

        // We emit multiple particles each frame that the thruster is firing
        for (i = 0; i < 16; i++) {
            // Generate the direction to fire the particle
            vec2 vector = v2angle(s->rbody.angle+180+randf()*SPREAD);
            v2muli(&vector, 0.03);
            v2inc(&vector, &s->rbody.vel);

            // Distribute the particles over a disk to avoid artifacts
            vec2 disk = v2angle(randf()*180);
            v2muli(&disk, 0.01);
            // Alternate emitting particles on the left and right
            if (i%2 == 0) v2inc(&disk, &thruster1);
            else v2inc(&disk, &thruster2);
            // Emit the particle using the generated vectors
            emit(s->main_particles, &disk, &vector);
        }
    }

    // Back to global space for rendering
    glPopMatrix();
}

vec2 main_thruster_points[2] = {
    {0.07, -0.1},
    {-0.07, -0.1}
};

vec2 rcs_points[NUM_THRUSTERS] = {
    {0.05, 0.08},
    {-0.05, 0.08},
    {0.05, -0.08},
    {-0.05, -0.08},
    {0.04, 0.09},
    {-0.04, 0.09},
    {0.05, -0.09},
    {-0.05, -0.09},
};

float rcs_angles[NUM_THRUSTERS] = { 90, -90, 90, -90, 0, 0, 180, 180 };
