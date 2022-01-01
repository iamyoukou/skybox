// =======================================
// Headers: order matters
// =======================================
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

// =======================================
// Triangle face structure
// =======================================
typedef struct
{
    // Vertex coordinate indices
    GLuint v1, v2, v3;

    // Vertex texture coordinate indices
    GLuint vt1, vt2, vt3;

    // Vertex normal coordinate indices
    GLuint vn1, vn2, vn3;
} Face;

// =======================================
// Mesh class definition
// =======================================
class Mesh
{
  public:
    // =======================================
    // Vertex Attributes
    // =======================================
    // Position list
    std::vector<glm::vec3> vertices;

    // UV coordinate list
    std::vector<glm::vec2> uvs;

    // Face normal list
    std::vector<glm::vec3> normals;

    // Face list
    std::vector<Face> faces;

    // =======================================
    // OpenGL objects
    // =======================================
    GLuint vboVtx, vboUv, vboNormal, vao;

    // =======================================
    // Constructor and destructor
    // =======================================
    Mesh(){};
    ~Mesh()
    {
        glDeleteBuffers(1, &vboVtx);
        glDeleteBuffers(1, &vboUv);
        glDeleteBuffers(1, &vboNormal);
        glDeleteVertexArrays(1, &vao);
    };
};

// =======================================
// OpenGL utilities
// =======================================
void printLog(GLuint &);
GLint myGetUniformLocation(GLuint &, string);
GLuint buildShader(string, string);
GLuint compileShader(string, GLenum);
GLuint linkShader(GLuint, GLuint);
std::string readFile(const std::string);
void initMeshObject(Mesh &);
Mesh loadMeshModel(std::string);
