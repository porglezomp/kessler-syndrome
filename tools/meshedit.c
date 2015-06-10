#include <unistd.h>
#include <linux/input.h>
#include <stdio.h>

#include "PiGL.h"
#include "GLES/gl.h"
#include "easyinput.h"

#include "../src/vec2.h"
#include "../stretchy_buffer.h"

#define DEBUG

typedef struct {
  GLushort start, end;
} line;

vec2f *verts = NULL;
line *lines = NULL;

void save(void);
void draw(int);
void place_line(void);
int search_vec2(vec2f*, vec2f);
int move_selection(int);
int delete_line(int);
vec2f start = {0, 0}, pos = {0, 0};
int adding_line = 0;

int main(void) {
  OGL_Init();
  atexit(OGL_Quit);
  ei_init(NULL);

  glClearColor(0, 0, 0, 1);
  glPointSize(8);
  float asp = (float) raspiGL_screen_width / raspiGL_screen_height;
  glMatrixMode(GL_PROJECTION_MATRIX);
  glOrthof(-asp, asp, -1, 1, -1, 1);
  glMatrixMode(GL_MODELVIEW_MATRIX);
  float step = 0.1;

#ifdef DEBUG
  fclose(fopen("test.log", "w"));
#endif
  
  int selected = -1;

  while (1) {
    // Start the frame
    glClear(GL_COLOR_BUFFER_BIT);
    ei_frame_start();
    ei_poll_all();

    // Core logic
    if (ei_frame_keypress(KEY_UP))    pos.y += step;
    if (ei_frame_keypress(KEY_DOWN))  pos.y -= step;
    if (ei_frame_keypress(KEY_LEFT))  pos.x -= step;
    if (ei_frame_keypress(KEY_RIGHT)) pos.x += step;

    if (ei_frame_keypress(KEY_EQUAL)) step *= 2.0;
    if (ei_frame_keypress(KEY_MINUS)) step *= 0.5;

    if (ei_frame_keypress(KEY_RIGHTCTRL)) place_line();
    if (ei_frame_keypress(KEY_TAB)) selected = move_selection(selected);
    if (ei_frame_keypress(KEY_X) && selected >= 0) selected = delete_line(selected);

    if (ei_frame_keypress(KEY_S)) save();
    if (ei_frame_keypress(KEY_P)) {
      sb_free(lines);
      sb_free(verts);
    }

    draw(selected);
    
    // End the frame
    OGL_SwapBuffers();
    if (ei_frame_keypress(KEY_ESC)) break;
    usleep(10000);
  }

  ei_quit();
}

void draw(int selected) {
  glColor4f(1, 1, 1, 1);
  glVertexPointer(2, GL_FLOAT, 0, verts);
  glDrawElements(GL_LINES, sb_count(lines)*2, GL_UNSIGNED_SHORT, lines);

  if (selected >= 0) {
    glColor4f(1, 0.3, 0.2, 1);
    glDrawElements(GL_LINES, 2, GL_UNSIGNED_SHORT, &lines[selected]);
  }

  glColor4f(1, 1, 1, 1);
  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(2, GL_FLOAT, 0, &pos);
  glDrawArrays(GL_POINTS, 0, 1);

  if (adding_line) {
    vec2f pts[] = {start, pos};
    glVertexPointer(2, GL_FLOAT, 0, &pts);
    glDrawArrays(GL_LINES, 0, 2);
  }
}

int search_vec2(vec2f *list, vec2f item) {
  int len = sb_count(list);
  for (int i = 0; i < len; i++) {
    vec2f it = list[i];
    if (it.x == item.x && it.y == item.y) return i;
  }
  return -1;
}

int move_selection(int s) {
  // Negative values indicate no selection, but that selection should
  // resume at the bitwise negation of the number. -1 -> 0, -2 -> 1, etc.
  // We return the resumed value if it's in a paused state
  if (s < 0) return ~s;
  
  // Rotate backwards while shift is held
  s += (ei_key_down(KEY_LEFTSHIFT)) ? -1 : 1;
  
  // Selection should loop around
  // % is remainder, not mod, so wrapping on the bottom will get stuck -
  int count = sb_count(lines);
  if (s < 0) return count - (-s);
  s %= count;
  return s;
}

// Writes the mesh out to a file
void save(void) {
  FILE *fd = fopen("save.ksl", "w");

  fputs("kessler\n", fd);
  int vertcount = sb_count(verts);
  fprintf(fd, "verts %i\n", vertcount);
  for (int i = 0; i < vertcount; i++) {
    fprintf(fd, "% f % f\n", verts[i].x, verts[i].y);
  }

  int linecount = sb_count(lines);
  fprintf(fd, "lines %i\n", linecount);
  for (int i = 0; i < linecount; i++) {
    fprintf(fd, "%04i %04i\n", lines[i].start, lines[i].end);
  }
  
  fclose(fd);
}

void print_debug(const char *word) {
#ifdef DEBUG
  FILE *fd = fopen("test.log", "a");
  fprintf(fd, "%s\n", word);
  int count = sb_count(lines);
  for (int i = 0; i < count; i++) {
    fprintf(fd, " %i--%i ", lines[i].start, lines[i].end);
  }
  fprintf(fd, "\n");
  count = sb_count(verts);
  for (int i = 0; i < count; i++) {
    fprintf(fd, " [%02f, %02f] ", verts[i].x, verts[i].y);
  }
  fprintf(fd, "\n\n");
  fclose(fd);
#endif
}

// Deletes the line indicated by index and returns the new selection position
int delete_line(int index) {
  print_debug("before");

  line l = lines[index];
  int startv = l.start, endv = l.end;
  int keepstart = 0, keepend = 0;
  sb_remove(lines, index);

  print_debug("step1");
    
  // Check if anything still references either vertex
  int count = sb_count(lines);
  for (int i = 0; i < count; i++) {
    l = lines[i];
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
    sb_remove(verts, startv);
    // If the end vertex has a higher index, reduce the target index so that we
    // can correctly adjust those higher than the end vertex in the next pass
    if (endv > startv) endv--;
    // Shift the indices of all higher vertices down by one
    count = sb_count(lines);
    for (int i = 0; i < count; i++) {
      if (lines[i].start >= startv) lines[i].start--;
      if (lines[i].end >= startv) lines[i].end--;
    }
  }

  // If there's no remaining reference to the end vertex
  if (!keepend) {
    // Remove it from the vertex list
    sb_remove(verts, endv);
    // And shift the indices of all higher vertices down by one
    count = sb_count(lines);
    for (int i = 0; i < count; i++) {
      if (lines[i].start >= endv) lines[i].start--;
      if (lines[i].end >= endv) lines[i].end--;
    }
  }

  print_debug("after");
  return -1;
}

void place_line(void) {
  if (! adding_line) {
    adding_line = 1;
    start = pos;
  } else {
    adding_line = 0;    
    line l;
    
    // Don't add degenerate lines
    if (start.x == pos.x && start.y == pos.y) return;

    // Search through the vertices, if one matches then reuse it
    int start_index = search_vec2(verts, start);
    if (start_index == -1) {
      // If none match, create a new one
      sb_push(verts, start);
      l.start = sb_count(verts)-1;
    } else {
      l.start = start_index;
    }
	
    int end_index = search_vec2(verts, pos);
    if (end_index == -1) {
      sb_push(verts, pos);
      l.end = sb_count(verts)-1;
    } else {
      l.end = end_index;
    }

    sb_push(lines, l);
  }
}
