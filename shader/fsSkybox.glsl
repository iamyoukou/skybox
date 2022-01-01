#version 330

in vec3 uv;
out vec4 fragColor;

// Skybox texture (cubemap)
uniform samplerCube skyboxTexture;

void main() {
  // Get color from texture
  fragColor = texture(skyboxTexture, uv);
}
