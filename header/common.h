#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/ext.hpp>

#include <GLFW/glfw3.h>
#include <FreeImage.h>

using namespace std;
using namespace glm;

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

typedef struct {
  GLuint v1, v2, v3;
  GLuint vt1, vt2, vt3;
  GLuint vn1, vn2, vn3;
} Face;

class Mesh {
public:
  std::vector<glm::vec3> vertices;
  std::vector<glm::vec2> uvs;
  std::vector<glm::vec3> faceNormals;
  std::vector<Face> faces;

  /* Constructors */
  Mesh(){};
  ~Mesh(){};

  /* Member functions */
  void translate(glm::vec3);
  void scale(glm::vec3);
  void rotate(glm::vec3);
};

std::string readFile(const std::string);
Mesh loadObj(std::string);
void printLog(GLuint &);
void keyCallback(GLFWwindow *, int, int, int, int);
GLint myGetUniformLocation(GLuint &, string);
GLuint buildShader(string, string);
GLuint compileShader(string, GLenum);
GLuint linkShader(GLuint, GLuint);
