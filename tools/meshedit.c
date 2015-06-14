#include "meshedit.h"

#include <assert.h>
#include <unistd.h>
#include <linux/input.h>
#include <stdio.h>
#include <string.h>

#include "PiGL.h"
#include "GLES/gl.h"
#include "easyinput.h"

#include "stretchy_buffer.h"

int main(void) {
  OGL_Init();
  atexit(OGL_Quit);
  ei_init(NULL);

  // Set up OpenGL viewport
  glClearColor(0, 0, 0, 1);
  glPointSize(8);
  float asp = (float) raspiGL_screen_width / raspiGL_screen_height;
  glMatrixMode(GL_PROJECTION_MATRIX);
  glOrthof(-asp, asp, -1, 1, -1, 1);
  glMatrixMode(GL_MODELVIEW_MATRIX);

  log_init("test.log");

  state st = {
    .line_start = {0, 0}, .line_end = {0, 0},
    .origin = {0, 0}, .pos = {0, 0},
    .scale = 1.0, .view_x = 0.0, .view_y = 0.0,
    .units_per_meter = 1024,
    .quit_timer = 0,
    .selected = -1, .item = 0,
    .step = 256,
    .vert_ll = NULL, .line_ll = NULL,
    .state = NEUTRAL
  };
  st.item = edit_item(&st, 0);
  
  while (1) {
    // Start the frame
    glClear(GL_COLOR_BUFFER_BIT);
    ei_frame_start();
    ei_poll_all();

    // Core logic
    if (ei_key_down(KEY_LEFTSHIFT)) {
      if (ei_frame_keypress(KEY_UP))    st.view_y += st.step;
      if (ei_frame_keypress(KEY_DOWN))  st.view_y -= st.step;
      if (ei_frame_keypress(KEY_LEFT))  st.view_x -= st.step;
      if (ei_frame_keypress(KEY_RIGHT)) st.view_x += st.step;      
    } else {
      if (ei_frame_keypress(KEY_UP))    st.pos.y += st.step;
      if (ei_frame_keypress(KEY_DOWN))  st.pos.y -= st.step;
      if (ei_frame_keypress(KEY_LEFT))  st.pos.x -= st.step;
      if (ei_frame_keypress(KEY_RIGHT)) st.pos.x += st.step;
    }

    if (ei_frame_keypress(KEY_N)) {
      st.item = edit_item(&st, st.item - 1);
      // Selections can't persist between meshes because they might not have
      // the same number of lines
      st.selected = -1;
    }
    if (ei_frame_keypress(KEY_M)) {
      st.item = edit_item(&st, st.item + 1);
      st.selected = -1;
    }

    // Navigation
    if (ei_frame_keypress(KEY_EQUAL))      st.step  *= 2;
    if (ei_frame_keypress(KEY_MINUS))      st.step  /= 2;
    if (ei_frame_keypress(KEY_LEFTBRACE))  st.scale /= 2;
    if (ei_frame_keypress(KEY_RIGHTBRACE)) st.scale *= 2;

    if (ei_frame_keypress(KEY_RIGHTCTRL) &&
	(st.state == NEUTRAL || st.state == PLACE_LINE)) {
      place_line(&st);
    }
    if (ei_frame_keypress(KEY_O) &&
	(st.state == NEUTRAL || st.state == PLACE_ORIGIN)) {
      place_origin(&st);
    }
    if (ei_frame_keypress(KEY_TAB)) st.selected = move_selection(&st, st.selected);
    if (ei_frame_keypress(KEY_X) && st.selected >= 0) st.selected = delete_line(&st, st.selected);
    

    // Deselect
    if (ei_frame_keypress(KEY_D) && st.selected >= 0) st.selected = ~st.selected;
    
    if (ei_frame_keypress(KEY_S)) save(&st, "save.ksl");
    if (ei_frame_keypress(KEY_L)) {
      load(&st, "save.ksl");
      st.selected = -1;
    }
    /* Questionable behavior, causes Segfaults
    if (ei_frame_keypress(KEY_P)) {
      sb_free(st.line_ll[st.item]);
      sb_remove(st.line_ll, st.item);
      sb_free(st.vert_ll[st.item]);
      sb_remove(st.vert_ll, st.item);
      st.item = edit_item(&st, st.item - 1);
      }*/

    draw(&st);
    
    // End the frame
    OGL_SwapBuffers();

    if (ei_frame_keypress(KEY_ESC)) {
      if (st.quit_timer <= 0) {
	st.quit_timer = 60;
      } else {
	break;
      }
    }
    usleep(10000);
  }

  ei_quit();
}

void draw(state *st) {
  glPushMatrix();
  float s = st->scale/st->units_per_meter;
  glScalef(s, s, s);
  glTranslatef(st->view_x, st->view_y, 0);

  ksl_vert *verts = st->vert_ll[st->item];
  ksl_line *lines = st->line_ll[st->item];
  // If the lines and verts are initialized
  if (verts && lines) {
    glColor4f(1, 1, 1, 1);
    glVertexPointer(2, GL_SHORT, 0, verts);
    glDrawElements(GL_LINES, sb_count(lines)*2, GL_UNSIGNED_SHORT, lines);

    if (st->selected >= 0) {
      glColor4f(1, 0.3, 0.2, 1);
      glDrawElements(GL_LINES, 2, GL_UNSIGNED_SHORT, &lines[st->selected]);
    }
  }

  // Draw the origin
  glColor4f(1, 0, 0, 0.3);
  glVertexPointer(2, GL_SHORT, 0, &st->origin);
  glDrawArrays(GL_POINTS, 0, 1);

  if (st->state == PLACE_ORIGIN) glColor4f(1, 0, 0, 1);
  else glColor4f(1, 1, 1, 1);
  
  // Draw the cursor
  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(2, GL_SHORT, 0, &st->pos);
  glDrawArrays(GL_POINTS, 0, 1);

  if (st->state == PLACE_LINE) {
    ksl_vert pts[] = {st->line_start, st->pos};
    glVertexPointer(2, GL_SHORT, 0, pts);
    glDrawArrays(GL_LINES, 0, 2);
  }
  
  glPopMatrix();
}

void append_new_mesh(state *st) {
  // Add new empty meshes
  sb_push(st->line_ll, NULL);
  sb_push(st->vert_ll, NULL);
}

int edit_item(state *st, int to_edit) {
  if (st->line_ll == NULL) {
    (void) sb_add(st->line_ll, 1);
    (void) sb_add(st->vert_ll, 1);
  }
  int count = sb_count(st->vert_ll);
  assert(count == sb_count(st->line_ll));

  // Loop below
  if (to_edit < 0) {
    if (sb_last(st->line_ll) == NULL) {
      // If the current top element is an empty mesh, loop to it
      to_edit = count - 1;
    } else {
      // Otherwise, create a new empty mesh and loop to it
      append_new_mesh(st);
      to_edit = count;
    }
  } else if (to_edit >= count) {
    // Handle looping above
    if (sb_last(st->line_ll) == NULL) {
      // If we're above an empty mesh, loop
      to_edit = 0;
    } else {
      // If we're not above an empty mesh, create one and go to it
      append_new_mesh(st);
      to_edit = count;
    }
  }

  return to_edit;
}

int search_vert(ksl_vert *list, ksl_vert item) {
  int len = sb_count(list);
  for (int i = 0; i < len; i++) {
    ksl_vert it = list[i];
    if (it.x == item.x && it.y == item.y) return i;
  }
  return -1;
}

int move_selection(state *st, int s) {
  // Rotate backwards while shift is held
  s += (ei_key_down(KEY_LEFTSHIFT)) ? -1 : 1;

  // Negative values indicate no selection, but that selection should
  // resume at the bitwise negation of the number. -1 -> 0, -2 -> 1, etc.
  // We return the resumed value if it's in a paused state
  if (s < 0) {
    // Only immediately return if we're moving forwards, we want to immediately
    // backtrack by one if we're going backwards
    if (!ei_key_down(KEY_LEFTSHIFT)) return ~s;
    s = ~s;
  }
   
  // Selection should loop around
  // % is remainder, not mod, so wrapping on the bottom will get stuck -
  int count = sb_count(st->line_ll[st->item]);
  if (s < 0) return count - (-s);
  s %= count;
  return s;
}

void print_debug(state *st, const char *word) {
#ifdef DEBUG
  FILE *fd = fopen("test.log", "a");
  fprintf(fd, "%s\n", word);
  ksl_line *lines = st->line_ll[st->item];
  int count = sb_count(lines);
  for (int i = 0; i < count; i++) {
    ksl_line line = lines[i];
    fprintf(fd, " %u--%u ", line.start, line.end);
  }
  fprintf(fd, "\n");
  
  ksl_vert *verts = st->vert_ll[st->item];
  count = sb_count(verts);
  for (int i = 0; i < count; i++) {
    ksl_vert vert = verts[i];
    fprintf(fd, " [%d, %d] ", vert.x, vert.y);
  }
  fprintf(fd, "\n\n");
  fclose(fd);
#endif
}

// Deletes the line indicated by index and returns the new selection position
int delete_line(state *st, int index) {
  print_debug(st, "before");

  ksl_line l = st->line_ll[st->item][index];
  int startv = l.start, endv = l.end;
  int keepstart = 0, keepend = 0;
  sb_remove(st->line_ll[st->item], index);
  
  print_debug(st, "step1");
    
  // Check if anything still references either vertex
  int count = sb_count(st->line_ll[st->item]);
  for (int i = 0; i < count; i++) {
    l = st->line_ll[st->item][i];
    if (startv == l.start || startv == l.end) {
      keepstart = 1; //if (keepend) break;
    }
    if (endv == l.start || endv == l.end) {
      keepend = 1; //if (keepstart) break;
    }
  }
  // If the two vertices are somehow the same, only allow it to be deleted once
  if (startv == endv) keepend = 1;

  // If there's no remaining reference to the start vertex
  if (!keepstart) {
    // Remove it from the vertex list
    sb_remove(st->vert_ll[st->item], startv);
    // If the end vertex has a higher index, reduce the target index so that we
    // can correctly adjust those higher than the end vertex in the next pass
    if (endv > startv) endv--;
    // Shift the indices of all higher vertices down by one
    ksl_line *lines = st->line_ll[st->item];
    count = sb_count(lines);
    for (int i = 0; i < count; i++) {
      if (lines[i].start >= startv) lines[i].start--;
      if (lines[i].end >= startv) lines[i].end--;
    }
  }

  // If there's no remaining reference to the end vertex
  if (!keepend) {
    // Remove it from the vertex list
    sb_remove(st->vert_ll[st->item], endv);
    // And shift the indices of all higher vertices down by one
    ksl_line *lines = st->line_ll[st->item];
    count = sb_count(lines);
    for (int i = 0; i < count; i++) {
      if (lines[i].start >= endv) lines[i].start--;
      if (lines[i].end >= endv) lines[i].end--;
    }
  }

  print_debug(st, "after");
  return ~index;
}

void place_line(state *st) {
  if (st->state != PLACE_LINE) {
    st->state = PLACE_LINE;
    st->line_start = st->pos;
  } else {
    st->state = NEUTRAL;
    ksl_line l;
    
    // Don't add degenerate lines
    if (st->line_start.x == st->pos.x &&
	st->line_start.y == st->pos.y) return;

    // Search through the vertices, if one matches then reuse it
    int start_index = search_vert(st->vert_ll[st->item], st->line_start);
    if (start_index == -1) {
      // If none match, create a new one
      sb_push(st->vert_ll[st->item], st->line_start);
      l.start = sb_count(st->vert_ll[st->item])-1;
    } else {
      l.start = start_index;
    }
	
    int end_index = search_vert(st->vert_ll[st->item], st->pos);
    if (end_index == -1) {
      sb_push(st->vert_ll[st->item], st->pos);
      l.end = sb_count(st->vert_ll[st->item])-1;
    } else {
      l.end = end_index;
    }

    sb_push(st->line_ll[st->item], l);
  }
}

void place_origin(state *st) {
  if (st->state == NEUTRAL) st->state = PLACE_ORIGIN;
  else if (st->state == PLACE_ORIGIN) {
    st->state = NEUTRAL;
    st->origin = st->pos;
  }
}
