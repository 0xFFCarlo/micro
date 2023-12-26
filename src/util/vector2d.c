#include "vector2d.h"
#include <math.h>

// Function to compute the dot product of two 2D vectors
double v2d_dot(Vector2D a, Vector2D b)
{
  return a.x * b.x + a.y * b.y;
}

// Function to compute the magnitude of a 2D vector
double v2d_len(Vector2D v)
{
  return sqrt(v.x * v.x + v.y * v.y);
}

// Function to normalize a 2D vector
Vector2D v2d_normalize(Vector2D v)
{
  double mag = v2d_len(v);
  Vector2D result = {v.x / mag, v.y / mag};
  return result;
}

// Function to reflect velocity vector v using normal vector n
Vector2D v2d_reflect(Vector2D v, Vector2D n)
{
  Vector2D n_normalized = v2d_normalize(n);

  double dot = v2d_dot(v, n_normalized);

  Vector2D reflection;
  reflection.x = v.x - 2 * dot * n_normalized.x;
  reflection.y = v.y - 2 * dot * n_normalized.y;

  return reflection;
}
