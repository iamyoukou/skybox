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
// void keyCallback(GLFWwindow *, int, int, int, int);
GLuint loadCubemap(vector<string> &);
void drawMesh(Mesh &);

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

void initMatrices() {
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

int main(int argc, char **argv) {
  initGL();
  initShader();
  initMatrices();
  initLight();

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
  Mesh meshData = loadObj("./model/torus.obj");

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
    drawMesh(meshData);

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

void drawMesh(Mesh &tMesh) {
  // # of faces
  const int nFaces = tMesh.faces.size();

  // # of vertices
  const int nVertices = nFaces * 3;

  // # of normals
  const int nNormals = nVertices;

  // prepare opengl objects
  GLuint vao, vboVtx, vboClr, vboNml;

  // std::cout << "# of vertices: " << nVertices << '\n';
  // std::cout << "# of faces: " << nFaces << '\n';

  // prepare arrays for vertex attributes
  // vertex positions
  // 3 vertices per face
  // and 3 coordinates per vertex
  GLfloat *vtxArray = new GLfloat[nVertices * 3];

  // face normals
  GLfloat *nmlArray = new GLfloat[nVertices * 3];

  // colors
  GLfloat *clrArray = new GLfloat[nVertices * 3];

  // prepare data
  for (int i = 0; i < nFaces; i++) {
    // vertex 1
    int vtxIdx = tMesh.faces[i][0];
    vtxArray[i * 9 + 0] = tMesh.vertices[vtxIdx].x;
    vtxArray[i * 9 + 1] = tMesh.vertices[vtxIdx].y;
    vtxArray[i * 9 + 2] = tMesh.vertices[vtxIdx].z;

    // color for vertex 1
    clrArray[i * 9 + 0] = 0.f;
    clrArray[i * 9 + 1] = 1.f;
    clrArray[i * 9 + 2] = 0.f;

    // normal for vertex 1
    int nmlIdx = tMesh.faces[i][3];
    nmlArray[i * 9 + 0] = tMesh.faceNormals[nmlIdx].x;
    nmlArray[i * 9 + 1] = tMesh.faceNormals[nmlIdx].y;
    nmlArray[i * 9 + 2] = tMesh.faceNormals[nmlIdx].z;

    // vertex 2
    vtxIdx = tMesh.faces[i][1];
    vtxArray[i * 9 + 3] = tMesh.vertices[vtxIdx].x;
    vtxArray[i * 9 + 4] = tMesh.vertices[vtxIdx].y;
    vtxArray[i * 9 + 5] = tMesh.vertices[vtxIdx].z;

    // color for vertex 2
    clrArray[i * 9 + 3] = 0.f;
    clrArray[i * 9 + 4] = 1.f;
    clrArray[i * 9 + 5] = 0.f;

    // normal for vertex 2
    nmlArray[i * 9 + 3] = tMesh.faceNormals[nmlIdx].x;
    nmlArray[i * 9 + 4] = tMesh.faceNormals[nmlIdx].y;
    nmlArray[i * 9 + 5] = tMesh.faceNormals[nmlIdx].z;

    // vertex 3
    vtxIdx = tMesh.faces[i][2];
    vtxArray[i * 9 + 6] = tMesh.vertices[vtxIdx].x;
    vtxArray[i * 9 + 7] = tMesh.vertices[vtxIdx].y;
    vtxArray[i * 9 + 8] = tMesh.vertices[vtxIdx].z;

    // color for vertex 3
    clrArray[i * 9 + 6] = 0.f;
    clrArray[i * 9 + 7] = 1.f;
    clrArray[i * 9 + 8] = 0.f;

    // normal for vertex 3
    nmlArray[i * 9 + 6] = tMesh.faceNormals[nmlIdx].x;
    nmlArray[i * 9 + 7] = tMesh.faceNormals[nmlIdx].y;
    nmlArray[i * 9 + 8] = tMesh.faceNormals[nmlIdx].z;
  }

  // prepare vao
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  // prepare vbo
  // vertex vbo
  glGenBuffers(1, &vboVtx);
  glBindBuffer(GL_ARRAY_BUFFER, vboVtx);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * nVertices * 3, vtxArray,
               GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(0);

  // normal vbo
  glGenBuffers(1, &vboNml);
  glBindBuffer(GL_ARRAY_BUFFER, vboNml);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * nNormals * 3, nmlArray,
               GL_STATIC_DRAW);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(1);

  // color vbo
  glGenBuffers(1, &vboClr);
  glBindBuffer(GL_ARRAY_BUFFER, vboClr);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * nVertices * 3, clrArray,
               GL_STATIC_DRAW);
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(2);

  // draw mesh
  glDrawArrays(GL_TRIANGLES, 0, nVertices);

  // delete opengl objects
  glDeleteBuffers(1, &vboVtx);
  glDeleteBuffers(1, &vboClr);
  glDeleteBuffers(1, &vboNml);
  glDeleteVertexArrays(1, &vao);

  // delete arrays
  delete[] vtxArray;
  delete[] clrArray;
  delete[] nmlArray;
}
