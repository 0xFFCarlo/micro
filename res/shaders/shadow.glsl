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
uniform float radius;
uniform float lightDepth;
uniform vec2 planet_center;
uniform vec2 view_center;
uniform float view_angle;

// Simple 2D random function
float random(vec2 st)
{
  return fract(sin(dot(st.xy, vec2(12.9898, 78.233))) * 43758.5453123);
}

vec2 rotateUV(vec2 uv, float rotation, vec2 mid)
{
  return vec2(cos(rotation) * (uv.x - mid.x) + sin(rotation) * (uv.y - mid.y) +
                mid.x,
              cos(rotation) * (uv.y - mid.y) - sin(rotation) * (uv.x - mid.x) +
                mid.y);
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

void main()
{
  fragColor = vec4(0.0, 0.0, 0.0, 1.0);

  vec2 uv = 2. * gl_FragCoord.xy / resolution.xy - 1.;
  vec2 uvs = uv * resolution.xy / resolution.y;
  // float radius_n = radius / max(resolution.x, resolution.y);
  float radius_n = radius;
  uvs = rotateUV(uvs, -view_angle, vec2(0.0, 0.0));

  // vec2 lightDir = vec2(1.0, 0.0); //normalize(vec2(sin(time * 1.0), cos(time
  // * 1.0)));
  vec4 ambient = vec4(0.0, 0.0, 0.0, 1.0);
  float dist = distance(uvs, planet_center);

  // Shadow effect
  float areaType = dist < radius_n ? 1.0 : 0.0;
  vec4 shadowColor = vec4(0.0, 0.0, 0.0, 0.0);
  vec2 dir = normalize(planet_center - uvs);
  float shadowIntensity = ((dist - (radius_n - lightDepth)) / (lightDepth));
  // intensity *= max(0.5, dot(lightDir, dir)); //directional light
  // intensity = floor(intensity * 8.0) / 8.0;
  // vec3 r = nrand3(uv * 0.1);
  // vec4 planetColor = vec4(0.25 + r.x * 0.25, 0.25 + r.x * 0.25, 0.25 + r.x *
  // 0.25, 1.0); shadowColor = mix(ambient, planetColor, shadowIntensity);
  shadowIntensity = floor(shadowIntensity * 10.0) / 10.0;
  shadowColor = mix(ambient, vec4(0.0, 0.0, 0.0, 0.0), shadowIntensity);

  // Atmosphere
  // float atmosphereIntensity = min(1.0, max(0.0, sqrt(dist - radius_n) * 1.3 +
  // 0.2)); vec4 atmosphereColor = vec4(0.6, 0.6, 1.0, 1.0 -
  // atmosphereIntensity);

  // Final color is either shadow or atmosphere (avoid if statement)
  // fragColor = shadowColor * areaType + atmosphereColor * (1.0 - areaType);
  fragColor = shadowColor;
}
