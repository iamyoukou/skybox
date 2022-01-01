#version 330
in vec3 finalVertexColor;
out vec4 fragmentColor;

void main() { fragmentColor = vec4(finalVertexColor, 1.0); }
