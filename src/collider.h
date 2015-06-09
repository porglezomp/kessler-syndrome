#ifndef _COLLIDER_H
#define _COLLIDER_H

struct rigidbody;

struct aabb {
  double left, right, top, bottom;
};

struct collider {
  struct aabb bounding_box;
  int vertex_count;
  struct vec2 *hull;
  struct rigidbody *owner;
};

struct aabb make_aabb(double x0, double y0, double x1, double y1);
int aabb_overlap(const struct aabb *a, const struct aabb *b);

#endif
