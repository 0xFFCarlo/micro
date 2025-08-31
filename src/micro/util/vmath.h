#ifndef VMATH_H
#define VMATH_H

// column-major 4x4 index
#define MIDX(r, c) ((c)*4 + (r))

typedef float Vec2[2];
typedef float Vec3[3];
typedef float Vec4[4];
typedef float Mat4[16];
typedef float Quat[4];

void vec3_set(Vec3 v, float x, float y, float z);
void vec3_add(Vec3 o, const Vec3 a, const Vec3 b);
void vec3_sub(Vec3 o, const Vec3 a, const Vec3 b);
void vec3_scale(Vec3 o, const Vec3 a, float s);
float vec3_dot(const Vec3 a, const Vec3 b);
void vec3_cross(Vec3 o, const Vec3 a, const Vec3 b);
void vec3_norm(Vec3 o, const Vec3 a);

void mat4_identity(Mat4 m);
void mat4_mul(Mat4 out, const Mat4 a, const Mat4 b);
void mat4_print(const Mat4 m);

// quaternion (x,y,z,w), rotates local->world
void quat_normalize(Quat q);
void quat_from_axis_angle(Quat q, const Vec3 axis, float angle);
void quat_mul(Quat out, const Quat a, const Quat b);
void quat_from_euler(Quat q, float yaw, float pitch, float roll);
void quat_rotate_vec3(Vec3 out, const Quat q, const Vec3 v);
void quat_to_mat3_cols(const Quat q, Vec3 xcol, Vec3 ycol, Vec3 zcol);
void quat_from_rotation_matrix_cols(Quat q, const Vec3 xcol, const Vec3 ycol,
                                    const Vec3 zcol);

#endif // VMATH_H
