#ifndef MESHEDIT_H
#define MESHEDIT_H

#define DEBUG

#include "kslmesh.h"

#ifdef DEBUG
char *logfile;
#define log_init(x) (logfile = x, fclose(fopen(logfile, "w")))
#define log(x) {			\
    FILE *fd = fopen(logfile, "a");	\
    fputs(x, fd);			\
    fclose(fd);				\
  }
#define logf(...) {			\
    FILE *fd = fopen(logfile, "a");	\
    fprintf(fd, __VA_ARGS__);		\
    fclose(fd);				\
  }
#else
#define log_init(x)
#define log(x)
#define logf(...)
#endif

enum STATES {
  NEUTRAL,
  PLACE_LINE,
  PLACE_ORIGIN
};

typedef struct {
  ksl_vert line_start, line_end, origin, pos;
  float scale, view_x, view_y;
  int units_per_meter, quit_timer, selected, item;
  short step;
  ksl_vert **vert_ll;
  ksl_line **line_ll;
  int state;
} state;

void save(state*, const char*);
int load(state*, const char*);
int edit_item(state*, int);
void draw(state*);
void place_line(state*);
int search_vert(ksl_vert*, ksl_vert);
int move_selection(state*, int);
int delete_line(state*, int);
void place_origin(state*);

#endif
