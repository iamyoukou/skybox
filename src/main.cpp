// skybox

#include "common.h"

GLFWwindow *window;

float verticalAngle = -1.775f;
float horizontalAngle = 0.935f;
float initialFoV = 45.0f;
float speed = 5.0f;
float mouseSpeed = 0.005f;
float farPlane = 2000.f;

vec3 eyePoint = vec3(2.f, 1.2f, -0.8f);
vec3 eyeDirection =
    vec3(sin(verticalAngle) * cos(horizontalAngle), cos(verticalAngle),
         sin(verticalAngle) * sin(horizontalAngle));
vec3 up = vec3(0.f, 1.f, 0.f);

vec3 lightPosition = vec3(3.f, 3.f, 3.f);
vec3 lightColor = vec3(1.f, 1.f, 1.f);
float lightPower = 12.f;

vec3 materialDiffuseColor = vec3(0.f, 1.f, 0.f);
vec3 materialAmbientColor = vec3(0.f, 0.05f, 0.f);
vec3 materialSpecularColor = vec3(1.f, 1.f, 1.f);

const string shaderDir("/Users/YJ-work/cpp/myGL_glfw/skybox/shader/");

const float SIZE = 500.f;
GLfloat skyboxVertices[] = {
    // Positions
    -SIZE, SIZE,  -SIZE, -SIZE, -SIZE, -SIZE, SIZE,  -SIZE, -SIZE,
    SIZE,  -SIZE, -SIZE, SIZE,  SIZE,  -SIZE, -SIZE, SIZE,  -SIZE,

    -SIZE, -SIZE, SIZE,  -SIZE, -SIZE, -SIZE, -SIZE, SIZE,  -SIZE,
    -SIZE, SIZE,  -SIZE, -SIZE, SIZE,  SIZE,  -SIZE, -SIZE, SIZE,

    SIZE,  -SIZE, -SIZE, SIZE,  -SIZE, SIZE,  SIZE,  SIZE,  SIZE,
    SIZE,  SIZE,  SIZE,  SIZE,  SIZE,  -SIZE, SIZE,  -SIZE, -SIZE,

    -SIZE, -SIZE, SIZE,  -SIZE, SIZE,  SIZE,  SIZE,  SIZE,  SIZE,
    SIZE,  SIZE,  SIZE,  SIZE,  -SIZE, SIZE,  -SIZE, -SIZE, SIZE,

    -SIZE, SIZE,  -SIZE, SIZE,  SIZE,  -SIZE, SIZE,  SIZE,  SIZE,
    SIZE,  SIZE,  SIZE,  -SIZE, SIZE,  SIZE,  -SIZE, SIZE,  -SIZE,

    -SIZE, -SIZE, -SIZE, -SIZE, -SIZE, SIZE,  SIZE,  -SIZE, -SIZE,
    SIZE,  -SIZE, -SIZE, -SIZE, -SIZE, SIZE,  SIZE,  -SIZE, SIZE};

GLuint vbo_skybox, obj_skybox_tex;
GLuint vbo_model, vbo_model_normal;
GLuint vao_skybox, vao_model;
GLuint ibo_model;
GLint uniform_model_skybox, uniform_view_skybox, uniform_projection_skybox;
GLint uniform_model_model, uniform_view_model, uniform_projection_model;
GLint uniform_lightColor, uniform_lightPosition, uniform_lightPower;
GLint uniform_diffuseColor, uniform_ambientColor, uniform_specularColor;
mat4 ori_model_skybox, model_skybox, view_skybox, projection_skybox;
mat4 model_model, view_model, projection_model;
GLuint vsSkybox, fsSkybox, vsModel, fsModel;
GLuint programSkybox, programModel;

void computeMatricesFromInputs(mat4 &, mat4 &);
void keyCallback(GLFWwindow *, int, int, int, int);
GLuint loadCubemap(vector<string> &);

void initGL() {
  // Initialise GLFW
  if (!glfwInit()) {
    fprintf(stderr, "Failed to initialize GLFW\n");
    getchar();
  }

  // without setting GLFW_CONTEXT_VERSION_MAJOR and _MINOR，
  // OpenGL 1.x will be used
  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

  // must be used if OpenGL version >= 3.0
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // Open a window and create its OpenGL context
  window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT,
                            "GLFW window with AntTweakBar", NULL, NULL);

  if (window == NULL) {
    std::cout << "Failed to open GLFW window." << std::endl;
    glfwTerminate();
  }

  glfwMakeContextCurrent(window);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetKeyCallback(window, keyCallback);

  /* Initialize GLEW */
  // without this, glGenVertexArrays will report ERROR!
  glewExperimental = GL_TRUE;

  if (glewInit() != GLEW_OK) {
    fprintf(stderr, "Failed to initialize GLEW\n");
    getchar();
    glfwTerminate();
  }

  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  // glCullFace(GL_FRONT_AND_BACK);

  // FreeImage library
  FreeImage_Initialise(true);
}

void initShader() {
  string vsDir = shaderDir + "vsSkybox.glsl";
  string fsDir = shaderDir + "fsSkybox.glsl";
  programSkybox = buildShader(vsDir, fsDir);

  vsDir = shaderDir + "vsModel.glsl";
  fsDir = shaderDir + "fsModel.glsl";
  programModel = buildShader(vsDir, fsDir);

  glUseProgram(programSkybox);
  glUseProgram(programModel);
}

int main(int argc, char **argv) {
  initGL();
  initShader();

  /* vao_skybox */
  glGenVertexArrays(1, &vao_skybox);
  glBindVertexArray(vao_skybox);

  // texture
  vector<string> texture_images;

  // texture_images.push_back("right.png");
  // texture_images.push_back("left.png");
  // texture_images.push_back("bottom.png");
  // texture_images.push_back("top.png");
  // texture_images.push_back("back.png");
  // texture_images.push_back("front.png");

  texture_images.push_back("./res/sahara_rt.tga");
  texture_images.push_back("./res/sahara_lf.tga");
  texture_images.push_back("./res/sahara_dn.tga");
  texture_images.push_back("./res/sahara_up.tga");
  texture_images.push_back("./res/sahara_bk.tga");
  texture_images.push_back("./res/sahara_ft.tga");

  glGenTextures(1, &obj_skybox_tex);
  glActiveTexture(0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, obj_skybox_tex);

  for (GLuint i = 0; i < texture_images.size(); i++) {
    int width, height;
    FIBITMAP *image;

    // image = FreeImage_ConvertTo24Bits( FreeImage_Load(FIF_PNG,
    // texture_images[i].c_str()) );
    image = FreeImage_ConvertTo24Bits(
        FreeImage_Load(FIF_TARGA, texture_images[i].c_str()));
    width = FreeImage_GetWidth(image);
    height = FreeImage_GetHeight(image);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height,
                 0, GL_BGR, GL_UNSIGNED_BYTE, (void *)FreeImage_GetBits(image));
    FreeImage_Unload(image);
  }

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  uniform_model_skybox = myGetUniformLocation(programSkybox, "model");
  uniform_view_skybox = myGetUniformLocation(programSkybox, "view");
  uniform_projection_skybox = myGetUniformLocation(programSkybox, "projection");

  model_skybox = translate(mat4(1.f), vec3(0.f, 0.f, -4.f));
  ori_model_skybox = model_skybox;
  view_skybox = lookAt(eyePoint, eyePoint + eyeDirection, up);
  projection_skybox = perspective(
      initialFoV, 1.f * WINDOW_WIDTH / WINDOW_HEIGHT, 0.01f, farPlane);
  glUniformMatrix4fv(uniform_model_skybox, 1, GL_FALSE,
                     value_ptr(model_skybox));
  glUniformMatrix4fv(uniform_view_skybox, 1, GL_FALSE, value_ptr(view_skybox));
  glUniformMatrix4fv(uniform_projection_skybox, 1, GL_FALSE,
                     value_ptr(projection_skybox));

  /* 创建 vbo */
  // for skybox

  glGenBuffers(1, &vbo_skybox);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_skybox);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 6 * 3, skyboxVertices,
               GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(0);

  // for 3d model
  // (next) change mesh_info_t to the rewritten one
  mesh_info_t meshData = load_obj("./model/torus.obj");
  int vertexNumber = meshData.vertexTable.size();

  // write vertex coordinate to array
  GLfloat *meshCoordinate = new GLfloat[vertexNumber * 3];
  for (size_t i = 0; i < vertexNumber; i++) {
    vec3 &vtxCoord = meshData.vertexTable[i].vertexCoordinate;
    meshCoordinate[i * 3] = vtxCoord.x;
    meshCoordinate[i * 3 + 1] = vtxCoord.y;
    meshCoordinate[i * 3 + 2] = vtxCoord.z;
  }

  // write vertex index to array
  int indexNumber = meshData.triangleIndex.size();
  GLushort *meshIndex = new GLushort[indexNumber * 3];
  for (size_t i = 0; i < indexNumber; i++) {
    ivec3 &idx = meshData.triangleIndex[i];
    meshIndex[i * 3] = idx[0];
    meshIndex[i * 3 + 1] = idx[1];
    meshIndex[i * 3 + 2] = idx[2];
  }

  // write vertex normal to array
  GLfloat *meshNormal = new GLfloat[vertexNumber * 3];
  for (size_t i = 0; i < vertexNumber; i++) {
    vec3 &vtxNormal = meshData.vertexTable[i].vertexNormal;
    meshNormal[i * 3] = vtxNormal.x;
    meshNormal[i * 3 + 1] = vtxNormal.y;
    meshNormal[i * 3 + 2] = vtxNormal.z;
  }

  // buffer objects
  glGenBuffers(1, &vbo_model);
  glGenBuffers(1, &vbo_model_normal);
  glGenBuffers(1, &ibo_model);
  glGenVertexArrays(1, &vao_model);

  glBindVertexArray(vao_model);

  // vbo
  glBindBuffer(GL_ARRAY_BUFFER, vbo_model);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertexNumber * 3,
               meshCoordinate, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(0);

  // ibo_model(index buffer object)
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_model);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * indexNumber * 3,
               meshIndex, GL_STATIC_DRAW);

  // vertex normal
  glBindBuffer(GL_ARRAY_BUFFER, vbo_model_normal);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertexNumber * 3, meshNormal,
               GL_STATIC_DRAW);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(1);

  uniform_model_model = myGetUniformLocation(programModel, "model");
  uniform_view_model = myGetUniformLocation(programModel, "view");
  uniform_projection_model = myGetUniformLocation(programModel, "projection");

  model_model = translate(mat4(1.f), vec3(0.f, 0.f, -4.f));
  view_model = lookAt(eyePoint, eyePoint + eyeDirection, up);
  projection_model = perspective(initialFoV, 1.f * WINDOW_WIDTH / WINDOW_HEIGHT,
                                 0.01f, farPlane);

  glUniformMatrix4fv(uniform_model_model, 1, GL_FALSE, value_ptr(model_model));
  glUniformMatrix4fv(uniform_view_model, 1, GL_FALSE, value_ptr(view_model));
  glUniformMatrix4fv(uniform_projection_model, 1, GL_FALSE,
                     value_ptr(projection_model));

  // light
  uniform_lightColor = myGetUniformLocation(programModel, "lightColor");
  glUniform3fv(uniform_lightColor, 1, value_ptr(lightColor));

  uniform_lightPosition = myGetUniformLocation(programModel, "lightPosition");
  glUniform3fv(uniform_lightPosition, 1, value_ptr(lightPosition));

  uniform_lightPower = myGetUniformLocation(programModel, "lightPower");
  glUniform1f(uniform_lightPower, lightPower);

  uniform_diffuseColor = myGetUniformLocation(programModel, "diffuseColor");
  glUniform3fv(uniform_diffuseColor, 1, value_ptr(materialDiffuseColor));

  uniform_ambientColor = myGetUniformLocation(programModel, "ambientColor");
  glUniform3fv(uniform_ambientColor, 1, value_ptr(materialAmbientColor));

  uniform_specularColor = myGetUniformLocation(programModel, "specularColor");
  glUniform3fv(uniform_specularColor, 1, value_ptr(materialSpecularColor));

  /* Loop until the user closes the window */
  while (!glfwWindowShouldClose(window)) {
    /* Render here */
    glClearColor(97 / 256.f, 175 / 256.f, 239 / 256.f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // draw skybox
    glUseProgram(programSkybox);
    glBindVertexArray(vao_skybox);
    computeMatricesFromInputs(projection_skybox, view_skybox);
    glUniformMatrix4fv(uniform_view_skybox, 1, GL_FALSE,
                       value_ptr(view_skybox));
    glUniformMatrix4fv(uniform_projection_skybox, 1, GL_FALSE,
                       value_ptr(projection_skybox));
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // draw 3d models
    glUseProgram(programModel);
    glBindVertexArray(vao_model);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_model);
    computeMatricesFromInputs(projection_model, view_model);
    glUniformMatrix4fv(uniform_view_model, 1, GL_FALSE, value_ptr(view_model));
    glUniformMatrix4fv(uniform_projection_model, 1, GL_FALSE,
                       value_ptr(projection_model));

    int size;
    glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
    glDrawElements(GL_TRIANGLES, size / sizeof(GLushort), GL_UNSIGNED_SHORT, 0);

    glfwSwapBuffers(window);

    /* Poll for and process events */
    glfwPollEvents();
  }

  glfwTerminate();

  // FreeImage library
  FreeImage_DeInitialise();

  return EXIT_SUCCESS;
}

void computeMatricesFromInputs(mat4 &newProject, mat4 &newView) {
  // glfwGetTime is called only once, the first time this function is called
  static float lastTime = glfwGetTime();

  // Compute time difference between current and last frame
  float currentTime = glfwGetTime();
  float deltaTime = float(currentTime - lastTime);

  // Get mouse position
  double xpos, ypos;
  glfwGetCursorPos(window, &xpos, &ypos);

  // Reset mouse position for next frame
  glfwSetCursorPos(window, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);

  // Compute new orientation
  //因为事先一步固定光标在屏幕中心
  //所以 WINDOW_WIDTH/2.f - xpos 和 WINDOW_HEIGHT/2.f - ypos 成了移动量
  horizontalAngle += mouseSpeed * float(xpos - WINDOW_WIDTH / 2.f);
  verticalAngle += mouseSpeed * float(-ypos + WINDOW_HEIGHT / 2.f);

  // Direction : Spherical coordinates to Cartesian coordinates conversion
  vec3 direction =
      vec3(sin(verticalAngle) * cos(horizontalAngle), cos(verticalAngle),
           sin(verticalAngle) * sin(horizontalAngle));

  // Right vector
  vec3 right = vec3(cos(horizontalAngle - 3.14 / 2.f), 0.f,
                    sin(horizontalAngle - 3.14 / 2.f));

  // new up vector
  vec3 newUp = cross(right, direction);

  // Move forward
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    eyePoint += direction * deltaTime * speed;
  }
  // Move backward
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    eyePoint -= direction * deltaTime * speed;
  }
  // Strafe right
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
    eyePoint += right * deltaTime * speed;
  }
  // Strafe left
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    eyePoint -= right * deltaTime * speed;
  }

  // float FoV = initialFoV;
  newProject = perspective(initialFoV, 1.f * WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f,
                           farPlane);
  // Camera matrix
  newView = lookAt(eyePoint, eyePoint + direction, newUp);

  //使 skybox 的中心永远位于 eyePoint
  //注意：GLM 的矩阵是 column major
  model_skybox[3][0] = ori_model_skybox[0][3] + eyePoint.x;
  model_skybox[3][1] = ori_model_skybox[1][3] + eyePoint.y;
  model_skybox[3][2] = ori_model_skybox[2][3] + eyePoint.z;
  glUniformMatrix4fv(uniform_model_skybox, 1, GL_FALSE,
                     value_ptr(model_skybox));

  // For the next frame, the "last time" will be "now"
  lastTime = currentTime;
}

void keyCallback(GLFWwindow *keyWnd, int key, int scancode, int action,
                 int mods) {
  if (action == GLFW_PRESS) {
    switch (key) {
    case GLFW_KEY_ESCAPE: {
      glfwSetWindowShouldClose(keyWnd, GLFW_TRUE);
      break;
    }
    case GLFW_KEY_F: {
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      break;
    }
    case GLFW_KEY_L: {
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      break;
    }
    case GLFW_KEY_I: {
      // std::cout << "eyePoint: " << to_string( eyePoint ) << '\n';
      // std::cout << "verticleAngle: " << fmod(verticalAngle, 6.28f) << ", "
      //    << "horizontalAngle: " << fmod(horizontalAngle, 6.28f) << endl;
      break;
    }
    default:
      break;
    }
  }
}

GLuint loadCubemap(vector<string> &faces) {
  GLuint textureID;
  glGenTextures(1, &textureID);
  glActiveTexture(GL_TEXTURE0);

  int width, height;
  FIBITMAP *image;

  glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
  for (GLuint i = 0; i < faces.size(); i++) {
    image = FreeImage_Load(FIF_PNG, faces[i].c_str());
    FreeImage_ConvertTo24Bits(image);
    width = FreeImage_GetWidth(image);
    height = FreeImage_GetHeight(image);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
  }

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

  return textureID;
}
