#include "vmath.h"
#include <math.h>
#include <string.h>

void vec3_set(Vec3 v, float x, float y, float z)
{
  v[0] = x;
  v[1] = y;
  v[2] = z;
}

void vec3_add(Vec3 o, const Vec3 a, const Vec3 b)
{
  o[0] = a[0] + b[0];
  o[1] = a[1] + b[1];
  o[2] = a[2] + b[2];
}

void vec3_sub(Vec3 o, const Vec3 a, const Vec3 b)
{
  o[0] = a[0] - b[0];
  o[1] = a[1] - b[1];
  o[2] = a[2] - b[2];
}

void vec3_scale(Vec3 o, const Vec3 a, float s)
{
  o[0] = a[0] * s;
  o[1] = a[1] * s;
  o[2] = a[2] * s;
}

float vec3_dot(const Vec3 a, const Vec3 b)
{
  return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

void vec3_cross(Vec3 o, const Vec3 a, const Vec3 b)
{
  o[0] = a[1] * b[2] - a[2] * b[1];
  o[1] = a[2] * b[0] - a[0] * b[2];
  o[2] = a[0] * b[1] - a[1] * b[0];
}

void vec3_norm(Vec3 o, const Vec3 a)
{
  float l = sqrtf(vec3_dot(a, a));
  if (l > 0)
  {
    float s = 1.0f / l;
    vec3_scale(o, a, s);
  }
  else
  {
    o[0] = o[1] = 0;
    o[2] = 1;
  }
}

void mat4_identity(Mat4 m)
{
  memset(m, 0, 16 * sizeof(float));
  m[MIDX(0, 0)] = m[MIDX(1, 1)] = m[MIDX(2, 2)] = m[MIDX(3, 3)] = 1.0f;
}

void mat4_mul(Mat4 out, const Mat4 a, const Mat4 b)
{
  float r[16];
  for (int c = 0; c < 4; ++c)
  {
    for (int r_i = 0; r_i < 4; ++r_i)
    {
      r[MIDX(r_i, c)] = a[MIDX(r_i, 0)] * b[MIDX(0, c)] +
                        a[MIDX(r_i, 1)] * b[MIDX(1, c)] +
                        a[MIDX(r_i, 2)] * b[MIDX(2, c)] +
                        a[MIDX(r_i, 3)] * b[MIDX(3, c)];
    }
  }
  memcpy(out, r, sizeof(r));
}

void quat_normalize(Quat q)
{
  float l = sqrtf(q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3]);
  if (l > 0)
  {
    float s = 1.0f / l;
    q[0] *= s;
    q[1] *= s;
    q[2] *= s;
    q[3] *= s;
  }
  else
  {
    q[0] = q[1] = q[2] = 0;
    q[3] = 1;
  }
}

void quat_from_axis_angle(Quat q, const Vec3 axis, float angle)
{
  float s = sinf(0.5f * angle);
  q[0] = axis[0] * s;
  q[1] = axis[1] * s;
  q[2] = axis[2] * s;
  q[3] = cosf(0.5f * angle);
  quat_normalize(q);
}

void quat_mul(Quat out, const Quat a, const Quat b)
{ // out=a*b
  out[0] = a[3] * b[0] + a[0] * b[3] + a[1] * b[2] - a[2] * b[1];
  out[1] = a[3] * b[1] - a[0] * b[2] + a[1] * b[3] + a[2] * b[0];
  out[2] = a[3] * b[2] + a[0] * b[1] - a[1] * b[0] + a[2] * b[3];
  out[3] = a[3] * b[3] - a[0] * b[0] - a[1] * b[1] - a[2] * b[2];
}

void quat_from_euler(Quat q, float yaw, float pitch, float roll)
{
  // yaw about +Y (world), pitch about +X (local), roll about +Z (local) —
  // applied Z * X * Y
  float cy = cosf(yaw * 0.5f), sy = sinf(yaw * 0.5f);
  float cx = cosf(pitch * 0.5f), sx = sinf(pitch * 0.5f);
  float cz = cosf(roll * 0.5f), sz = sinf(roll * 0.5f);
  // Yaw * Pitch * Roll
  float qy[4] = {0.0f * sy, 1.0f * sy, 0.0f * sy, cy};
  float qx[4] = {1.0f * sx, 0.0f * sx, 0.0f * sx, cx};
  float qz[4] = {0.0f * sz, 0.0f * sz, 1.0f * sz, cz};
  float t[4];
  quat_mul(t, qy, qx);
  quat_mul(q, t, qz);
  quat_normalize(q);
}

// rotate vector by quaternion (local->world)
void quat_rotate_vec3(Vec3 out, const Quat q, const Vec3 v)
{
  // Using: out = v + 2*cross(q.xyz, cross(q.xyz, v)+q.w*v)
  float u[3] = {q[0], q[1], q[2]};
  float uv[3], uuv[3];
  vec3_cross(uv, u, v);
  vec3_cross(uuv, u, uv);
  float s = 2.0f * q[3];
  float t[3] = {uv[0] * s + v[0], uv[1] * s + v[1], uv[2] * s + v[2]};
  out[0] = t[0] + 2.0f * uuv[0];
  out[1] = t[1] + 2.0f * uuv[1];
  out[2] = t[2] + 2.0f * uuv[2];
}

void quat_to_mat3_cols(const Quat q, Vec3 xcol, Vec3 ycol, Vec3 zcol)
{
  // Columns (basis vectors) from quaternion
  float x[3] = {1, 0, 0}, y[3] = {0, 1, 0}, z[3] = {0, 0, 1};
  quat_rotate_vec3(xcol, q, x);
  quat_rotate_vec3(ycol, q, y);
  quat_rotate_vec3(zcol, q, z);
}

void quat_from_rotation_matrix_cols(Quat q, const Vec3 xcol, const Vec3 ycol,
                                    const Vec3 zcol)
{
  // Build 3x3 row-major entries from columns to get quaternion
  float m00 = xcol[0], m01 = ycol[0], m02 = zcol[0];
  float m10 = xcol[1], m11 = ycol[1], m12 = zcol[1];
  float m20 = xcol[2], m21 = ycol[2], m22 = zcol[2];
  float t = m00 + m11 + m22;
  if (t > 0.0f)
  {
    float s = sqrtf(t + 1.0f) * 2.0f;
    q[3] = 0.25f * s;
    q[0] = (m21 - m12) / s;
    q[1] = (m02 - m20) / s;
    q[2] = (m10 - m01) / s;
  }
  else if ((m00 > m11) && (m00 > m22))
  {
    float s = sqrtf(1.0f + m00 - m11 - m22) * 2.0f;
    q[3] = (m21 - m12) / s;
    q[0] = 0.25f * s;
    q[1] = (m01 + m10) / s;
    q[2] = (m02 + m20) / s;
  }
  else if (m11 > m22)
  {
    float s = sqrtf(1.0f + m11 - m00 - m22) * 2.0f;
    q[3] = (m02 - m20) / s;
    q[0] = (m01 + m10) / s;
    q[1] = 0.25f * s;
    q[2] = (m12 + m21) / s;
  }
  else
  {
    float s = sqrtf(1.0f + m22 - m00 - m11) * 2.0f;
    q[3] = (m10 - m01) / s;
    q[0] = (m02 + m20) / s;
    q[1] = (m12 + m21) / s;
    q[2] = 0.25f * s;
  }
  quat_normalize(q);
}
