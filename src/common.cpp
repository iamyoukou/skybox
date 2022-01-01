#include "common.h"

// ================================================
// Build shaders
// Parameters:
//   1. vsDir: vertex shader file
//   2. fsDir: fragment shader file
// Return: shader executable
// ================================================
GLuint buildShader(string vsDir, string fsDir)
{
    GLuint vs, fs;
    GLint linkOk;
    GLuint exeShader;

    // Compile
    vs = compileShader(vsDir, GL_VERTEX_SHADER);
    fs = compileShader(fsDir, GL_FRAGMENT_SHADER);

    // Link
    exeShader = linkShader(vs, fs);

    return exeShader;
}

// ================================================
// Compile shader file
// Parameters:
//   1. fileName: shader file
//   2. type: shader type
// Return: shader object
// ================================================
GLuint compileShader(string fileName, GLenum type)
{
    // Read shader file
    string sTemp = readFile(fileName);
    const GLchar *source = sTemp.c_str();

    // Set shader type
    string info;
    switch (type)
    {
        case GL_VERTEX_SHADER:
        {
            info = "Vertex";
            break;
        }
        case GL_FRAGMENT_SHADER:
        {
            info = "Fragment";
            break;
        }
    }

    // If reading shader file fails
    if (source == NULL)
    {
        std::cout << info << " Shader : Can't read shader source file." << std::endl;
        return 0;
    }

    // Compile shader file
    const GLchar *sources[] = {source};
    GLuint objShader = glCreateShader(type);
    glShaderSource(objShader, 1, sources, NULL);
    glCompileShader(objShader);

    // If compiling shader file fails
    GLint compile_ok;
    glGetShaderiv(objShader, GL_COMPILE_STATUS, &compile_ok);
    if (compile_ok == GL_FALSE)
    {
        std::cout << info << " Shader : Fail to compile." << std::endl;
        printLog(objShader);
        glDeleteShader(objShader);
        return 0;
    }

    return objShader;
}

// ================================================
// Link shaders
// Parameters:
//   1. vsObj: vertex shader object
//   2. fsObj: fragment shader object
// Return: shader program object
// ================================================
GLuint linkShader(GLuint vsObj, GLuint fsObj)
{
    // Attach shader objects to create an executable
    // Then link the executable to rendering pipeline
    GLuint exe = glCreateProgram();
    glAttachShader(exe, vsObj);
    glAttachShader(exe, fsObj);
    glLinkProgram(exe);

    // Check linking result
    GLint linkOk;
    glGetProgramiv(exe, GL_LINK_STATUS, &linkOk);
    if (linkOk == GL_FALSE)
    {
        std::cout << "Failed to link shader program." << std::endl;
        printLog(exe);
        glDeleteProgram(exe);

        return 0;
    }

    return exe;
}

// ================================================
// Print error log
// Parameters:
//   object: shader object
// ================================================
void printLog(GLuint &object)
{
    // Get compile/build/link log length
    GLint logLength = 0;
    if (glIsShader(object))
    {
        glGetShaderiv(object, GL_INFO_LOG_LENGTH, &logLength);
    }
    else if (glIsProgram(object))
    {
        glGetProgramiv(object, GL_INFO_LOG_LENGTH, &logLength);
    }
    else
    {
        cerr << "printlog: Not a shader or a program" << endl;
        return;
    }

    // Get compile/build/link log
    // Then output log
    char *log = (char *)malloc(logLength);
    if (glIsShader(object))
        glGetShaderInfoLog(object, logLength, NULL, log);
    else if (glIsProgram(object))
        glGetProgramInfoLog(object, logLength, NULL, log);
    cerr << log << endl;

    // Release resource
    free(log);
}

// ================================================
// Get uniform variable in shaders
// Parameters:
//   1. prog: shader program object
//   2. name: variable name (string)
// Return: uniform variable (handler)
// ================================================
GLint myGetUniformLocation(GLuint &prog, string name)
{
    // Get uniform variable
    GLint location = glGetUniformLocation(prog, name.c_str());

    // If failed, glGetUniformLocation returns -1
    if (location == -1)
    {
        cerr << "Could not bind uniform : " << name << ". "
             << "Did you set the right name? "
             << "Or is " << name << " not used?" << endl;
    }

    return location;
}

// ================================================
// Read file into a string
// Parameters:
//   fileName: file to read
// Return: string
// ================================================
std::string readFile(const std::string fileName)
{
    std::ifstream in;
    in.open(fileName.c_str());
    std::stringstream ss;
    ss << in.rdbuf();
    std::string sOut = ss.str();
    in.close();

    return sOut;
}

// ================================================
// Read mesh data from file (Wavefront .obj)
// Parameters:
//   fileName: mesh data file
// Return: Mesh instance
// ================================================
Mesh loadMeshModel(std::string fileName)
{
    Mesh mesh;

    // Open mesh data file
    std::ifstream fin;
    fin.open(fileName.c_str());
    if (!(fin.good()))
    {
        std::cout << "failed to open file : " << fileName << std::endl;
    }

    // Read mesh data
    while (fin.peek() != EOF)
    {
        std::string s;
        fin >> s;

        // Vertex coordinate
        if ("v" == s)
        {
            float x, y, z;
            fin >> x;
            fin >> y;
            fin >> z;
            mesh.vertices.push_back(glm::vec3(x, y, z));
        }
        // Texture coordinate
        else if ("vt" == s)
        {
            float u, v;
            fin >> u;
            fin >> v;
            mesh.uvs.push_back(glm::vec2(u, v));
        }
        // Face normal (recorded as vn in .obj file)
        else if ("vn" == s)
        {
            float x, y, z;
            fin >> x;
            fin >> y;
            fin >> z;
            mesh.normals.push_back(glm::vec3(x, y, z));
        }
        // Vertices contained in face, and face normal
        else if ("f" == s)
        {
            Face f;

            // v1/vt1/vn1
            fin >> f.v1;
            fin.ignore(1);
            fin >> f.vt1;
            fin.ignore(1);
            fin >> f.vn1;

            // v2/vt2/vn2
            fin >> f.v2;
            fin.ignore(1);
            fin >> f.vt2;
            fin.ignore(1);
            fin >> f.vn2;

            // v3/vt3/vn3
            fin >> f.v3;
            fin.ignore(1);
            fin >> f.vt3;
            fin.ignore(1);
            fin >> f.vn3;

            // Note:
            //  v, vt, vn in "v/vt/vn" start from 1,
            //  but indices of std::vector start from 0,
            //  so we need minus 1 for all elements
            f.v1 -= 1;
            f.vt1 -= 1;
            f.vn1 -= 1;

            f.v2 -= 1;
            f.vt2 -= 1;
            f.vn2 -= 1;

            f.v3 -= 1;
            f.vt3 -= 1;
            f.vn3 -= 1;

            mesh.faces.push_back(f);
        }
        else
        {
            continue;
        }
    }

    fin.close();

    return mesh;
}

// ================================================
// Initialize OpenGL contents for a mesh
// Parameters:
//   mesh: Mesh instance
// ================================================
void initMeshObject(Mesh &mesh)
{
    // Write vertex coordinate to array
    // 3 vertices per face, 3 float per vertex coord, 2 float per tex coord
    int nOfFaces = mesh.faces.size();
    GLfloat *aVtxCoords = new GLfloat[nOfFaces * 3 * 3];
    GLfloat *aUvs = new GLfloat[nOfFaces * 3 * 2];
    GLfloat *aNormals = new GLfloat[nOfFaces * 3 * 3];

    for (size_t i = 0; i < nOfFaces; i++)
    {
        // Vertex 1
        int vtxIdx = mesh.faces[i].v1;
        aVtxCoords[i * 9 + 0] = mesh.vertices[vtxIdx].x;
        aVtxCoords[i * 9 + 1] = mesh.vertices[vtxIdx].y;
        aVtxCoords[i * 9 + 2] = mesh.vertices[vtxIdx].z;

        // Normal for vertex 1
        int nmlIdx = mesh.faces[i].vn1;
        aNormals[i * 9 + 0] = mesh.normals[nmlIdx].x;
        aNormals[i * 9 + 1] = mesh.normals[nmlIdx].y;
        aNormals[i * 9 + 2] = mesh.normals[nmlIdx].z;

        // UV for vertex 1
        int uvIdx = mesh.faces[i].vt1;
        aUvs[i * 6 + 0] = mesh.uvs[uvIdx].x;
        aUvs[i * 6 + 1] = mesh.uvs[uvIdx].y;

        // Vertex 2
        vtxIdx = mesh.faces[i].v2;
        aVtxCoords[i * 9 + 3] = mesh.vertices[vtxIdx].x;
        aVtxCoords[i * 9 + 4] = mesh.vertices[vtxIdx].y;
        aVtxCoords[i * 9 + 5] = mesh.vertices[vtxIdx].z;

        // Normal for vertex 2
        nmlIdx = mesh.faces[i].vn2;
        aNormals[i * 9 + 3] = mesh.normals[nmlIdx].x;
        aNormals[i * 9 + 4] = mesh.normals[nmlIdx].y;
        aNormals[i * 9 + 5] = mesh.normals[nmlIdx].z;

        // UV for vertex 2
        uvIdx = mesh.faces[i].vt2;
        aUvs[i * 6 + 2] = mesh.uvs[uvIdx].x;
        aUvs[i * 6 + 3] = mesh.uvs[uvIdx].y;

        // Vertex 3
        vtxIdx = mesh.faces[i].v3;
        aVtxCoords[i * 9 + 6] = mesh.vertices[vtxIdx].x;
        aVtxCoords[i * 9 + 7] = mesh.vertices[vtxIdx].y;
        aVtxCoords[i * 9 + 8] = mesh.vertices[vtxIdx].z;

        // Normal for vertex 3
        nmlIdx = mesh.faces[i].vn3;
        aNormals[i * 9 + 6] = mesh.normals[nmlIdx].x;
        aNormals[i * 9 + 7] = mesh.normals[nmlIdx].y;
        aNormals[i * 9 + 8] = mesh.normals[nmlIdx].z;

        // UV for vertex 3
        uvIdx = mesh.faces[i].vt3;
        aUvs[i * 6 + 4] = mesh.uvs[uvIdx].x;
        aUvs[i * 6 + 5] = mesh.uvs[uvIdx].y;
    }

    // Bind vao
    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);

    // Set vbo for vertex
    glGenBuffers(1, &mesh.vboVtx);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vboVtx);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * nOfFaces * 3 * 3, aVtxCoords, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    // Set vbo for texture
    glGenBuffers(1, &mesh.vboUv);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vboUv);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * nOfFaces * 3 * 2, aUvs, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    // Set vbo for normal
    glGenBuffers(1, &mesh.vboNormal);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vboNormal);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * nOfFaces * 3 * 3, aNormals, GL_STATIC_DRAW);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2);

    // Release resource
    delete[] aVtxCoords;
    delete[] aUvs;
    delete[] aNormals;
}
