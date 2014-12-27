#ifndef _SPACE_H
#define _SPACE_H

// Make the aspect ratio available everywhere
extern float asp;

// Random float [-1, 1]
double randf();
// Loop a position around from one side of the screen to the other
void loop(vec2*);

#endif
