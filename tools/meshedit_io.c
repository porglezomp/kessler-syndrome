#include "meshedit.h"

#include <stdio.h>
#include <stdlib.h>

#include "stretchy_buffer.h"

static void write_single_mesh(FILE *fd, ksl_vert *verts, ksl_line *lines, ksl_vert o) {
  // Write the mesh header
  fputs("mesh\n", fd);
  // 1024 units per meter
  fputs("meter 1024\n", fd);
  
  // Write the vertex header with a vertex count for easier loading
  int vertcount = sb_count(verts);
  // Include the vertex format (currently always xy, could be xyrgb, rxgyb, etc)
  fprintf(fd, "verts %d xy\n", vertcount);
  // Write out all the verts
  for (int i = 0; i < vertcount; i++) {
    fprintf(fd, "% 07d % 07d\n", verts[i].x - o.x, verts[i].y - o.y);
  }

  // Write the lines header with a line count for easer loading
  int linecount = sb_count(lines);
  fprintf(fd, "lines %i\n", linecount);
  // Write out all the lines
  for (int i = 0; i < linecount; i++) {
    fprintf(fd, " %06u  %06u\n", lines[i].start, lines[i].end);
  }
}

// Writes the mesh out to a file
void save(state *st, const char *fname) {
  FILE *fd = fopen(fname, "w");

  // If the last item is NULL (an empty mesh), then ignore it in the count
  int real_count = sb_count(st->line_ll);
  if (sb_last(st->line_ll) == NULL) real_count--;

  // Write the header
  fputs("ksl01\n", fd);
  // Version for upgrade tool purposes
  fputs("version 0.1\n", fd);
  fprintf(fd, "meshes %d\n", real_count);

  for (int i = 0; i < real_count; i++) {
    write_single_mesh(fd, st->vert_ll[i], st->line_ll[i], st->origin);
  }
  
  fclose(fd);
}

int load(state *st, const char *fname) {
  ksl_mesh_list *mesh_list = ksl_load_meshes(fname, NULL);
  if (mesh_list == NULL || mesh_list->count == 0) {
    logf("%s\n", ksl_get_error());
    log("Failed to load mesh");
    return 1;
  }
  // Stop adding a line if one is being added
  st->state = NEUTRAL;

  // Free all of the old mesh components
  int mesh_count_old = sb_count(st->vert_ll);
  logf("old: %d\n", mesh_count_old);
  for (int i = 0; i < mesh_count_old; i++) {
    sb_free(st->vert_ll[i]);
    sb_free(st->line_ll[i]);
  }
  
  // Copy the mesh components into the mesh edit buffer
  int mesh_count = mesh_list->count;
  logf("new: %d\n", mesh_count);
  sb_free(st->vert_ll);
  sb_free(st->line_ll);
  (void) sb_add(st->vert_ll, mesh_count);
  (void) sb_add(st->line_ll, mesh_count);
  for (int i = 0; i < mesh_count; i++) {
    ksl_mesh *mesh = mesh_list->meshes[i];
    (void) sb_add(st->vert_ll[i], mesh->vert_count);    
    (void) sb_add(st->line_ll[i], mesh->line_count);
    memcpy(st->vert_ll[i], mesh->verts, sizeof(ksl_vert)*mesh->vert_count);
    memcpy(st->line_ll[i], mesh->lines, sizeof(ksl_line)*mesh->line_count);
  }

  // Release the original meshes and mesh list
  ksl_free_mesh_list(mesh_list);
  
  return 0;
}
