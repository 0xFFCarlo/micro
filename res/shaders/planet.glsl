#version 330 core

//Stars effect
#ifdef GL_ES
precision mediump float;
#endif

in vec4 o_color;
in vec2 o_texcoord;
out vec4 fragColor;

uniform sampler2D u_texture;

in vec4 gl_FragCoord;
uniform vec2 resolution;

// Simple 2D random function
float random(vec2 st) {
  return fract(sin(dot(st.xy, vec2(12.9898, 78.233))) * 43758.5453123);
}

void main() {
  vec2 uv = 2. * gl_FragCoord.xy / resolution.xy - 1.;
  vec2 uvs = uv * resolution.xy / max(resolution.x, resolution.y);
  vec2 center = vec2(0.0, 0.0);

  float dist = distance(uvs, center);
  
  // Planet texture
  float areaType = dist < 1.0 ? 1.0 : 0.0;
  float r = random(uvs);
  vec4 planetColor = vec4(0.25 + r * 0.25, 0.25 + r * 0.25, 0.25 + r * 0.25, 1.0);
  
  //Final color 
  vec4 backgroundColor = vec4(0.0, 0.0, 0.0, 0.0);
  fragColor = planetColor * areaType + backgroundColor * (1.0 - areaType);

  // Quantize frag color
  fragColor.x = floor(fragColor.x * 8.0) / 8.0;
  fragColor.y = floor(fragColor.y * 8.0) / 8.0;
  fragColor.z = floor(fragColor.z * 8.0) / 8.0;
}
