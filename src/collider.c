#include "collider.h"
#include "vec2.h"

struct aabb make_aabb(double x0, double y0, double x1, double y1) {
  struct aabb box;
  // Ensure that the left and bottom are smaller than the right and top
  if (x0 < x1) box.left = x0, box.right = x1;
  else box.left = x1, box.right = x0;

  if (y0 < y1) box.bottom = y0, box.top = y1;
  else box.bottom = y1, box.top = y0;

  return box;
}

int aabb_overlap(const struct aabb *a, const struct aabb *b) {
  // If any one of these are true, there is a seperating axis
  return !(a->bottom > b->top ||
	   a->top < b->bottom ||
	   a->left > b->right ||
	   a->right < b->left);
}
