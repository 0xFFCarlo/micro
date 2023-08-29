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

uniform float time;
uniform vec2 resolution;

uniform float planet_radius = 1.0;
uniform vec2 planet_center = vec2(0.0, 0.2 + 1.0);

uniform vec2 view_center = vec2(0.0, 0.0);
uniform float view_angle = 0.0;

uniform vec3 nebulaColor = vec3(0.57, 0.27, 0.41);
uniform float starfieldThreshold1 = 40.0;
uniform float starfieldThreshold2 = 40.0;
uniform float atmosphereMaxIntensity = 0.5;
uniform float atmosphereDecay = 0.9;
uniform vec4 atmosphereColor = vec4(0.6, 0.6, 1.0, 1.0);

float field(in vec3 p, float s)
{
  float strength = 7. + .03 * log(1.e-6 + fract(sin(time) * 4373.11));
  float accum = s / 4.;
  float prev = 0.;
  float tw = 0.;
  for (int i = 0; i < 20; ++i)
  {
    float mag = dot(p, p);
    float w = exp(-float(i) / 7.);
    p = abs(p) / mag + vec3(-.5, -.4, -1.5);
    accum += w * exp(-strength * pow(abs(mag - prev), 2.2));
    tw += w;
    prev = mag;
  }
  return max(0., 5. * accum / tw - .7) + 0.2;
}

float field2(in vec3 p, float s)
{
  float strength = 7. + .03 * log(1.e-6 + fract(sin(time) * 4373.11));
  float accum = s / 4.;
  float prev = 0.;
  float tw = 0.;
  for (int i = 0; i < 13; ++i)
  {
    float mag = dot(p, p);
    p += abs(p) / mag + vec3(-.5, -.4, -1.5);
    float w = exp(-float(i) / 7.);
    accum += w * exp(-strength * pow(abs(mag - prev), 2.2));
    tw += w;
    prev = mag;
  }
  return max(0., 5. * accum / tw - .7);
}

vec3 nrand3(vec2 co)
{
  vec3 a = fract(cos(co.x * 0.3e-3 + co.y * vec3(12., 87., 1.0)) *
                 vec3(1.3e5, 4.7e5, 2.9e5) *
                 fract(sin(dot(co, vec2(12.8697, 776.1243)))));
  vec3 b = fract(sin(co.x * 8.3e-3 + co.y * vec3(12., 87., 1.0)) *
                 vec3(8.1e5, 1.0e5, 0.1e5) *
                 floor(sin(dot(co, vec2(26.7416, 17.8943)))));
  vec3 c = mix(a, b, 0.2);
  return c;
}

vec2 rotateUV(vec2 uv, float rotation, vec2 mid)
{
  return vec2(cos(rotation) * (uv.x - mid.x) + sin(rotation) * (uv.y - mid.y) +
                mid.x,
              cos(rotation) * (uv.y - mid.y) - sin(rotation) * (uv.x - mid.x) +
                mid.y);
}

void main()
{
  vec2 uv = 2. * gl_FragCoord.xy / resolution.xy - 1.;
  vec2 uvs = uv * resolution.xy / resolution.y;
  vec2 uvs_atm = rotateUV(uvs, -view_angle, vec2(0.0, 0.0));
  uvs = rotateUV(uvs, -view_angle, view_center);
  vec3 p = vec3(uvs / 6., 0) + vec3(1., -1.3, 0.);
  p += .2 * vec3(sin(time / 16.), sin(time / 12.), sin(time / 128.));
  // p.z += 0.2;

  float freqs[4];
  freqs[0] = 0.02;
  freqs[1] = nebulaColor.y; // GREEN
  freqs[2] = nebulaColor.x; // RED
  freqs[3] = nebulaColor.z; // BLUE

  float t = field(p, freqs[2]);
  float v = (1. - exp((abs(uv.x) - 1.) * 6.)) *
            (1. - exp((abs(uv.y) - 1.) * 6.));

  float zoom_pos = time;
  vec3 p2 = vec3(uvs / (6. + sin(zoom_pos * 0.11) * 0.2 + 0.2 +
                        sin(zoom_pos * 0.15) * 0.3 + 0.4),
                 1.5) +
            vec3(2., -1.3, -1.);
  p2 += 0.25 * vec3(sin(time / 16.), sin(time / 12.), sin(time / 128.));
  // p2.z += 0.2;
  // float t2 = field2(p2,freqs[3]);
  // vec4 c2 = mix(.4, 0.5, v) * vec4(0.8 * t2 * t2 * t2 , 1.5 * t2 * t2 , 1.5 *
  // t2, t2);
  vec4 c2 = vec4(0.0, 0.0, 0.0, 1.0);

  vec2 seed = p.xy * 2.0;
  seed = floor(seed * resolution.x);
  vec3 rnd = nrand3(seed);
  vec4 starcolor = vec4(pow(rnd.y, starfieldThreshold1));

  vec2 seed2 = p2.xy * 2.0;
  seed2 = floor(seed2 * resolution.x);
  vec3 rnd2 = nrand3(seed2);
  starcolor += vec4(pow(rnd2.y, starfieldThreshold2));

  fragColor = mix(freqs[3] - .3, 1., v) * vec4(1.5 * freqs[2] * t * t * t,
                                               1.2 * freqs[1] * t * t,
                                               freqs[3] * t, 1.0) +
              c2 + starcolor;

  // Atmosphere
  float dist = distance(uvs_atm, planet_center);
  float radius_n = planet_radius - 8.0 / resolution.y;
  float areaType = dist > radius_n ? 1.0 : 0.0;
  float atmosphereIntensity = min(1.0,
                                  max(0.0,
                                      sqrt(dist - radius_n) * atmosphereDecay +
                                        (1.0 - atmosphereMaxIntensity)));
  fragColor = mix(atmosphereColor, fragColor, atmosphereIntensity);
}
