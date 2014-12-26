#ifndef _GAME_INPUT_H
#define _GAME_INPUT_H

#include <linux/input.h>

void setup_input();
void map_input(float *x, float *y);

int get_key_event(struct input_event *ev);

int key_down(int direction);
void handle_input();

#endif
