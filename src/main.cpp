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
GLint uniform_diffuseColor, uniform_ambientColor, uniform_specularColor,
    uniform_tex;
mat4 ori_model_skybox, model_skybox, view_skybox, projection_skybox;
mat4 model_model, view_model, projection_model;
GLuint vsSkybox, fsSkybox, vsModel, fsModel;
GLuint programSkybox, programModel;

void computeMatricesFromInputs();
void keyCallback(GLFWwindow *, int, int, int, int);

void initGL();
void initOther();
void initShader();
void initMatrix();
void initLight();
void initSkybox();

int main(int argc, char **argv) {
  initGL();
  initOther();
  initShader();
  initMatrix();
  initLight();

  // skybox
  initSkybox();

  // for 3d model
  Mesh mesh = loadObj("./model/rock_cube.obj");
  initMesh(mesh);

  // a rough way to solve cursor position initialization problem
  // must call glfwPollEvents once to activate glfwSetCursorPos
  // this is a glfw mechanism problem
  glfwPollEvents();
  glfwSetCursorPos(window, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);

  /* Loop until the user closes the window */
  while (!glfwWindowShouldClose(window)) {
    /* Render here */
    glClearColor(97 / 256.f, 175 / 256.f, 239 / 256.f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // view control
    computeMatricesFromInputs();

    // draw skybox
    glUseProgram(programSkybox);
    glBindVertexArray(vao_skybox);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // draw 3d models
    glUseProgram(programModel);
    glBindVertexArray(mesh.vao);
    glDrawArrays(GL_TRIANGLES, 0, mesh.faces.size() * 3);

    // update frame
    glfwSwapBuffers(window);

    /* Poll for and process events */
    glfwPollEvents();
  }

  glfwTerminate();

  // FreeImage library
  FreeImage_DeInitialise();

  return EXIT_SUCCESS;
}

void computeMatricesFromInputs() {
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
  // The cursor is set to the center of the screen last frame,
  // so (currentCursorPos - center) is the offset of this frame
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
  mat4 newProject = perspective(initialFoV, 1.f * WINDOW_WIDTH / WINDOW_HEIGHT,
                                0.1f, farPlane);
  // Camera matrix
  mat4 newView = lookAt(eyePoint, eyePoint + direction, newUp);

  // update for skybox
  glUseProgram(programSkybox);
  view_skybox = newView;
  projection_skybox = newProject;
  glUniformMatrix4fv(uniform_view_skybox, 1, GL_FALSE, value_ptr(view_skybox));
  glUniformMatrix4fv(uniform_projection_skybox, 1, GL_FALSE,
                     value_ptr(projection_skybox));

  // make sure that the center of skybox is always at eyePoint
  // the GLM matrix is column major
  model_skybox[3][0] = ori_model_skybox[0][3] + eyePoint.x;
  model_skybox[3][1] = ori_model_skybox[1][3] + eyePoint.y;
  model_skybox[3][2] = ori_model_skybox[2][3] + eyePoint.z;
  glUniformMatrix4fv(uniform_model_skybox, 1, GL_FALSE,
                     value_ptr(model_skybox));

  // update for mesh
  glUseProgram(programModel);
  view_model = newView;
  projection_model = newProject;
  glUniformMatrix4fv(uniform_view_model, 1, GL_FALSE, value_ptr(view_model));
  glUniformMatrix4fv(uniform_projection_model, 1, GL_FALSE,
                     value_ptr(projection_model));

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
      std::cout << "eyePoint: " << to_string(eyePoint) << '\n';
      std::cout << "verticleAngle: " << fmod(verticalAngle, 6.28f) << ", "
                << "horizontalAngle: " << fmod(horizontalAngle, 6.28f) << endl;
      break;
    }
    default:
      break;
    } // end switch
  }   // end if
}

void initGL() {
  // Initialise GLFW
  if (!glfwInit()) {
    fprintf(stderr, "Failed to initialize GLFW\n");
    getchar();
    exit(EXIT_FAILURE);
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
    exit(EXIT_FAILURE);
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
    exit(EXIT_FAILURE);
  }

  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
}

void initOther() {
  // FreeImage library
  FreeImage_Initialise(true);
}

void initShader() {
  string vsDir, fsDir;

  // skybox
  vsDir = shaderDir + "vsSkybox.glsl";
  fsDir = shaderDir + "fsSkybox.glsl";
  programSkybox = buildShader(vsDir, fsDir);

  // mesh
  vsDir = shaderDir + "vsModel.glsl";
  fsDir = shaderDir + "fsModel.glsl";
  programModel = buildShader(vsDir, fsDir);
}

void initMatrix() {
  glUseProgram(programModel);
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
}

void initLight() {
  glUseProgram(programModel);

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
}

void initSkybox() {
  /* vao_skybox */
  glGenVertexArrays(1, &vao_skybox);
  glBindVertexArray(vao_skybox);

  // texture
  vector<string> texture_images;
  texture_images.push_back("./res/right.png");
  texture_images.push_back("./res/left.png");
  texture_images.push_back("./res/bottom.png");
  texture_images.push_back("./res/top.png");
  texture_images.push_back("./res/back.png");
  texture_images.push_back("./res/front.png");

  glActiveTexture(GL_TEXTURE0);
  glGenTextures(1, &obj_skybox_tex);
  glBindTexture(GL_TEXTURE_CUBE_MAP, obj_skybox_tex);

  for (GLuint i = 0; i < texture_images.size(); i++) {
    int width, height;
    FIBITMAP *image;

    image = FreeImage_ConvertTo24Bits(
        FreeImage_Load(FIF_PNG, texture_images[i].c_str()));
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

  glUseProgram(programSkybox);
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

  // for skybox
  glGenBuffers(1, &vbo_skybox);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_skybox);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 6 * 3, skyboxVertices,
               GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(0);
}
