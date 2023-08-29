#version 330 core

// Stars effect
#ifdef GL_ES
precision mediump float;
#endif

// in vec4 o_color;
// in vec2 o_texcoord;
out vec4 fragColor;

uniform sampler2D u_texture;

in vec4 gl_FragCoord;
uniform vec2 resolution;

#define M_PI 3.14159265358979323846

float rand(vec2 co)
{
  return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}
float rand(vec2 co, float l)
{
  return rand(vec2(rand(co), l));
}
float rand(vec2 co, float l, float t)
{
  return rand(vec2(rand(co, l), t));
}

float perlin(vec2 p, float dim, float time)
{
  vec2 pos = floor(p * dim);
  vec2 posx = pos + vec2(1.0, 0.0);
  vec2 posy = pos + vec2(0.0, 1.0);
  vec2 posxy = pos + vec2(1.0);

  float c = rand(pos, dim, time);
  float cx = rand(posx, dim, time);
  float cy = rand(posy, dim, time);
  float cxy = rand(posxy, dim, time);

  vec2 d = fract(p * dim);
  d = -0.5 * cos(d * M_PI) + 0.5;

  float ccx = mix(c, cx, d.x);
  float cycxy = mix(cy, cxy, d.x);
  float center = mix(ccx, cycxy, d.y);

  return center * 2.0 - 1.0;
}

// p must be normalized!
float perlin(vec2 p, float dim)
{

  /*vec2 pos = floor(p * dim);
  vec2 posx = pos + vec2(1.0, 0.0);
  vec2 posy = pos + vec2(0.0, 1.0);
  vec2 posxy = pos + vec2(1.0);

  // For exclusively black/white noise
  /*float c = step(rand(pos, dim), 0.5);
  float cx = step(rand(posx, dim), 0.5);
  float cy = step(rand(posy, dim), 0.5);
  float cxy = step(rand(posxy, dim), 0.5);*/

  /*float c = rand(pos, dim);
  float cx = rand(posx, dim);
  float cy = rand(posy, dim);
  float cxy = rand(posxy, dim);

  vec2 d = fract(p * dim);
  d = -0.5 * cos(d * M_PI) + 0.5;

  float ccx = mix(c, cx, d.x);
  float cycxy = mix(cy, cxy, d.x);
  float center = mix(ccx, cycxy, d.y);

  return center * 2.0 - 1.0;*/
  return perlin(p, dim, 0.0);
}

// Simple 2D random function
float random(vec2 st)
{
  return fract(sin(dot(st.xy, vec2(12.9898, 78.233))) * 43758.5453123);
}

void main()
{
  vec2 uv = 2. * gl_FragCoord.xy / resolution.xy - 1.;
  vec2 uvs = uv * resolution.xy / max(resolution.x, resolution.y);
  vec2 center = vec2(0.0, 0.0);

  float dist = distance(uvs, center);

  // Planet texture
  float areaType = dist < 1.0 ? 1.0 : 0.0;
  float r = random(uvs);
  // float r = (perlin(uvs, 100.0, 0.0) + 1.0) * 0.5;
  vec4 planetColor = vec4(0.25 + r * 0.25, 0.25 + r * 0.25, 0.25 + r * 0.25,
                          1.0);

  // Final color
  vec4 backgroundColor = vec4(0.0, 0.0, 0.0, 0.0);
  fragColor = planetColor * areaType + backgroundColor * (1.0 - areaType);

  // Quantize frag color
  fragColor.x = floor(fragColor.x * 8.0) / 8.0;
  fragColor.y = floor(fragColor.y * 8.0) / 8.0;
  fragColor.z = floor(fragColor.z * 8.0) / 8.0;
}
