#version 330

layout(location = 0) in vec3 position;
out vec3 uv;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main() {
  // Projected vertex position
  gl_Position = projection * view * model * vec4(position, 1.0);

  // Both x and y of vertex position are [-1, 1],
  // so they can directly become uv-coordinate
  uv = -position;
}
