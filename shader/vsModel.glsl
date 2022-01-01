#version 330
layout(location = 0) in vec3 vtxCoord;
layout(location = 1) in vec2 vtxUv;
layout(location = 2) in vec3 vtxNormal;

out vec3 finalVertexColor;

uniform mat4 model, view, projection;
uniform vec3 diffuseColor, ambientColor, specularColor;
uniform vec3 lightColor, lightPosition;
uniform float lightPower;

void main() {
  // Projected vertex position
  gl_Position = projection * view * model * vec4(vtxCoord, 1.0);

  // Compute vertex position in view space
  vec3 vtxCoordView = (view * model * vec4(vtxCoord, 1.0)).xyz;

  // Transform world-space normal into view-space normal
  vec3 vtxNormalView =
      normalize((transpose(inverse(view)) * model * vec4(vtxNormal, 0.0)).xyz);

  // Compute point light direction in view space
  vec3 lightPosView = (view * model * vec4(lightPosition, 1.0)).xyz;
  vec3 lightDirView = normalize(lightPosView - vtxCoordView);

  // Compute eye direction in view space
  // - Eye position in view space is (0 0 0)
  vec3 eyeDirView = normalize(vec3(0.0) - vtxCoordView);

  // Compute view-space reflect vector
  vec3 reflectView = reflect(-lightDirView, vtxNormalView);

  // For diffuseColor coefficient
  float cosTheta = clamp(dot(lightDirView, vtxNormalView), 0.0, 1.0);

  // For specularColor coefficient
  float cosAlpha = clamp(dot(eyeDirView, reflectView), 0.0, 1.0);

  // For distance damping
  float distanceView = length(lightPosView - vtxCoordView);

  // DiffuseColor
  finalVertexColor = diffuseColor * lightColor * lightPower * cosTheta /
                     (distanceView * distanceView);

  // AmbientColor
  finalVertexColor += ambientColor;

  // SpecularColor
  finalVertexColor +=
      specularColor * lightColor * lightPower * pow(cosAlpha, 5);
}
