#ifndef VECTOR2D_H
#define VECTOR2D_H

typedef struct Vector2D
{
  double x;
  double y;
} Vector2D;

// Function to compute the dot product of two 2D vectors
double v2d_dot(Vector2D a, Vector2D b);

// Function to compute the magnitude of a 2D vector
double v2d_len(Vector2D v);

// Function to normalize a 2D vector
Vector2D v2d_normalize(Vector2D v);

// Function to reflect velocity vector v using normal vector n
Vector2D v2d_reflect(Vector2D v, Vector2D n);

#endif // VECTOR2D_H
