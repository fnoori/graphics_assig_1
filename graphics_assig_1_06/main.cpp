// ==========================================================================
// Barebones OpenGL Core Profile Boilerplate
//    using the GLFW windowing system (http://www.glfw.org)
//
// Loosely based on
//  - Chris Wellons' example (https://github.com/skeeto/opengl-demo) and
//  - Camilla Berglund's example (http://www.glfw.org/docs/latest/quick.html)
//
// Author:  Farzam Noori, based off of the boilerplate code provided by: Sonny Chan, University of Calgary
// Date:    Jan 26, 2017
// ==========================================================================

#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <vector>
#include <iterator>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Specify that we want the OpenGL core profile before including GLFW headers
#ifndef LAB_LINUX
#include <glad/glad.h>
#else
#define GLFW_INCLUDE_GLCOREARB
#define GL_GLEXT_PROTOTYPES
#endif
#include <GLFW/glfw3.h>

using namespace std;
using namespace glm;
// --------------------------------------------------------------------------
// OpenGL utility and support function prototypes

void QueryGLVersion();
bool CheckGLErrors();

GLenum DRAW_ELEMENT_TYPE = GL_LINE_STRIP;

vector<float> vertices;
vector<float> colours;

float spiralSize = 0.01;
int spiralLimit = 90;

int squareIteration = 1;

string LoadSource(const string &filename);
GLuint CompileShader(GLenum shaderType, const string &source);
GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader);
void drawTriangle();


// --------------------------------------------------------------------------
// Functions to set up OpenGL shader programs for rendering

struct MyShader
{
    // OpenGL names for vertex and fragment shaders, shader program
    GLuint  vertex;
    GLuint  fragment;
    GLuint  program;
    
    // initialize shader and program names to zero (OpenGL reserved value)
    MyShader() : vertex(0), fragment(0), program(0)
    {}
};

// load, compile, and link shaders, returning true if successful
bool InitializeShaders(MyShader *shader)
{
    // load shader source from files
    string vertexSource = LoadSource("vertex.glsl");
    string fragmentSource = LoadSource("fragment.glsl");
    if (vertexSource.empty() || fragmentSource.empty()) return false;
    
    // compile shader source into shader objects
    shader->vertex = CompileShader(GL_VERTEX_SHADER, vertexSource);
    shader->fragment = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);
    
    // link shader program
    shader->program = LinkProgram(shader->vertex, shader->fragment);
    
    // check for OpenGL errors and return false if error occurred
    return !CheckGLErrors();
}

// deallocate shader-related objects
void DestroyShaders(MyShader *shader)
{
    // unbind any shader programs and destroy shader objects
    glUseProgram(0);
    glDeleteProgram(shader->program);
    glDeleteShader(shader->vertex);
    glDeleteShader(shader->fragment);
}

// --------------------------------------------------------------------------
// Functions to set up OpenGL buffers for storing textures


// --------------------------------------------------------------------------
// Functions to set up OpenGL buffers for storing geometry data

struct MyGeometry
{
    // OpenGL names for array buffer objects, vertex array object
    GLuint  vertexBuffer;
    GLuint  colourBuffer;
    //GLuint  indiceBuffer;
    GLuint  vertexArray;
    GLsizei elementCount;
    
    // initialize object names to zero (OpenGL reserved value)
    MyGeometry() : vertexBuffer(0), colourBuffer(0), /*indiceBuffer(0),*/ vertexArray(0), elementCount(0)
    {}
};

// create buffers and fill with geometry data, returning true if successful
bool InitializeGeometry(MyGeometry *geometry, vector<GLfloat> vertices, vector<GLfloat> colours)
{
    geometry->elementCount = vertices.size()/2;
    
    //std::cout << "vert size: " << vertices.size() << std::endl;
    
    // these vertex attribute indices correspond to those specified for the
    // input variables in the vertex shader
    const GLuint VERTEX_INDEX = 0;
    const GLuint COLOUR_INDEX = 1;
    
    // create an array buffer object for storing our vertices
    glGenBuffers(1, &geometry->vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, geometry->vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_DYNAMIC_DRAW);
    
    // create another one for storing our colours
    glGenBuffers(1, &geometry->colourBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, geometry->colourBuffer);
    glBufferData(GL_ARRAY_BUFFER, colours.size() * sizeof(float), &colours[0], GL_DYNAMIC_DRAW);
     
    // create a vertex array object encapsulating all our vertex attributes
    glGenVertexArrays(1, &geometry->vertexArray);
    glBindVertexArray(geometry->vertexArray);
    
    // associate the position array with the vertex array object
    glBindBuffer(GL_ARRAY_BUFFER, geometry->vertexBuffer);
    glVertexAttribPointer(VERTEX_INDEX, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(VERTEX_INDEX);
    
    // assocaite the colour array with the vertex array object
    glBindBuffer(GL_ARRAY_BUFFER, geometry->colourBuffer);
    glVertexAttribPointer(COLOUR_INDEX, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(COLOUR_INDEX);
    
    // unbind our buffers, resetting to default state
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    // check for OpenGL errors and return false if error occurred
    return !CheckGLErrors();
}

// deallocate geometry-related objects
void DestroyGeometry(MyGeometry *geometry)
{
    // unbind and destroy our vertex array object and associated buffers
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &geometry->vertexArray);
    glDeleteBuffers(1, &geometry->vertexBuffer);
    glDeleteBuffers(1, &geometry->colourBuffer);
}

// --------------------------------------------------------------------------
// Rendering function that draws our scene to the frame buffer

void RenderScene(MyGeometry *geometry, MyShader *shader)
{
    // clear screen to a dark grey colour
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    // bind our shader program and the vertex array object containing our
    // scene geometry, then tell OpenGL to draw our geometry
    glUseProgram(shader->program);
    glBindVertexArray(geometry->vertexArray);
    glDrawArrays(DRAW_ELEMENT_TYPE, 0, geometry->elementCount);
    
    
    // reset state to default (no shader or geometry bound)
    glBindVertexArray(0);
    glUseProgram(0);
    
    // check for an report any OpenGL errors
    CheckGLErrors();
}

// --------------------------------------------------------------------------
// GLFW callback functions

// reports GLFW errors
void ErrorCallback(int error, const char* description)
{
    cout << "GLFW ERROR " << error << ":" << endl;
    cout << description << endl;
}

// handles keyboard input events
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
    else if (key == GLFW_KEY_A && action == GLFW_PRESS)
    {
        vertices.clear();
        colours.clear();
        
        DRAW_ELEMENT_TYPE = GL_LINE_STRIP;
        
        vertices.push_back(-0.5f); vertices.push_back(0.5f);
        vertices.push_back(-0.5f); vertices.push_back(-0.5f);
        vertices.push_back(0.5f); vertices.push_back(-0.5f);
        vertices.push_back(0.5f); vertices.push_back(0.5f);
        vertices.push_back(-0.5f); vertices.push_back(0.5f);
        
        colours.push_back(1.0f); colours.push_back(1.0f); colours.push_back(1.0f);
        colours.push_back(1.0f); colours.push_back(1.0f); colours.push_back(1.0f);
        colours.push_back(1.0f); colours.push_back(1.0f); colours.push_back(1.0f);
        colours.push_back(1.0f); colours.push_back(1.0f); colours.push_back(1.0f);
        colours.push_back(1.0f); colours.push_back(1.0f); colours.push_back(1.0f);
        colours.push_back(1.0f); colours.push_back(1.0f); colours.push_back(1.0f);
    }
    else if (key == GLFW_KEY_Q && action == GLFW_PRESS)
    {
        squareIteration++;
        
        vector<float> firstThree;
        for (int i = ((vertices.size()-2)-8); i < vertices.size()-2; i+=2)
        {
            float newX = (vertices[i] + vertices[i+2])/2;
            float newY = (vertices[i+1] + vertices[i+3])/2;
            
            firstThree.push_back(newX);
            firstThree.push_back(newY);
        }
        
        float lastNewX = firstThree[firstThree.size() - firstThree.size()];
        float lastNewY = firstThree[firstThree.size() - (firstThree.size())+1];
        
        firstThree.push_back(lastNewX);
        firstThree.push_back(lastNewY);
        
        for (int i = 0; i < 15; i++)
        {
            if (squareIteration % 2 == 0)
            {
                colours.push_back(1.0f);
                colours.push_back(0.0f);
                colours.push_back(0.0f);
            }
            else
            {
                colours.push_back(1.0f);
                colours.push_back(1.0f);
                colours.push_back(1.0f);
            }
        }
        
        vertices.insert(vertices.end(), firstThree.begin(), firstThree.end());
    }
    else if (key == GLFW_KEY_S && action == GLFW_PRESS)
    {
        vertices.clear();
        colours.clear();
        
        DRAW_ELEMENT_TYPE = GL_LINE_STRIP;
        
        float x = 0;
        float y = 0;
        float angle = 0.0f;
        
        for (int i = 0; i < spiralLimit; i++)
        {
            angle = 0.1 * i;
            x = (spiralSize * angle) * cos(angle);
            y = (spiralSize * angle) * sin(angle);
            
            vertices.push_back(x);
            vertices.push_back(y);
            
            for (int i = 0; i < 3; i++)
            {
                colours.push_back(1.0f);
            }
        }
        
    }
    else if (key == GLFW_KEY_W && action == GLFW_PRESS)
    {
        float x = 0;
        float y = 0;
        float angle = 0.0f;
        
        int beginIter = spiralLimit;
        spiralLimit = beginIter+10;
        
        for (int i = beginIter; i < spiralLimit; i++)
        {
            angle = 0.1 * i;
            x = (spiralSize * angle) * cos(angle);
            y = (spiralSize * angle) * sin(angle);
            
            vertices.push_back(x);
            vertices.push_back(y);
            
            for (int i = 0; i < 3; i++)
            {
                colours.push_back(1.0f);
                colours.push_back(0.0f);
                colours.push_back(0.0f);
            }
        }
    }
    else if (key == GLFW_KEY_D && action == GLFW_PRESS)
    {
        vertices.clear();
        colours.clear();
        
        DRAW_ELEMENT_TYPE = GL_TRIANGLES;
        
        vertices.push_back(-0.6f); vertices.push_back(-0.4f);
        vertices.push_back(0.0f); vertices.push_back(0.6f);
        vertices.push_back(0.6f); vertices.push_back(-0.4f);
        
        colours.push_back(1.0f);
        colours.push_back(0.0f);
        colours.push_back(0.0f);
        
        colours.push_back(1.0f);
        colours.push_back(0.0f);
        colours.push_back(0.0f);
        
        colours.push_back(1.0f);
        colours.push_back(0.0f);
        colours.push_back(0.0f);
    }
    else if (key == GLFW_KEY_E && action == GLFW_PRESS)
    {
        /*
         This was supposed to draw the Sierpinski Triangle.
         However, I could not figure out how finish it.
        */
        
        vector<float> newTriangle;
        for (int i = 0; i < vertices.size()-3; i+=2)
        {
            float x = (vertices[i] + vertices[i+2])/2;
            float y = (vertices[i+1] + vertices[i+3])/2;
            
            newTriangle.push_back(x);
            newTriangle.push_back(y);
        }
        
        float newTriangleLastX = newTriangle[newTriangle.size() - newTriangle.size()+4];
        float newTriangleLastY = newTriangle[newTriangle.size() - newTriangle.size()+5];

        vertices.insert(vertices.end(), newTriangle.begin(), newTriangle.end());
        
        for (int i = 0; i < 15; i++)
        {
            colours.push_back(0.0f);
            colours.push_back(0.0f);
            colours.push_back(0.0f);
        }
    }
}

// ==========================================================================
// PROGRAM ENTRY POINT

int main(int argc, char *argv[])
{
    // initialize the GLFW windowing system
    if (!glfwInit()) {
        cout << "ERROR: GLFW failed to initialize, TERMINATING" << endl;
        return -1;
    }
    glfwSetErrorCallback(ErrorCallback);
    
    // attempt to create a window with an OpenGL 4.1 core profile context
    GLFWwindow *window = 0;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    int width = 800, height = 800;
    window = glfwCreateWindow(width, height, "CPSC 453 OpenGL Boilerplate", 0, 0);
    if (!window) {
        cout << "Program failed to create GLFW window, TERMINATING" << endl;
        glfwTerminate();
        return -1;
    }
    
    // set keyboard callback function and make our context current (active)
    glfwSetKeyCallback(window, KeyCallback);
    glfwMakeContextCurrent(window);
    
    //Intialize GLAD
#ifndef LAB_LINUX
    if (!gladLoadGL())
    {
        cout << "GLAD init failed" << endl;
        return -1;
    }
#endif
    
    // query and print out information about our OpenGL environment
    QueryGLVersion();
    
    // call function to load and compile shader programs
    MyShader shader;
    if (!InitializeShaders(&shader)) {
        cout << "Program could not initialize shaders, TERMINATING" << endl;
        return -1;
    }
    MyGeometry geometry;

    
    // run an event-triggered main loop
    while (!glfwWindowShouldClose(window))
    {
        if (!InitializeGeometry(&geometry, vertices, colours))
            cout << "Program failed to intialize geometry!" << endl;
        
        // call function to draw our scene
        RenderScene(&geometry, &shader);
        glfwSetKeyCallback(window, KeyCallback);
        
        glfwSwapBuffers(window);
        
        glfwPollEvents();
    }
    
    // clean up allocated resources before exit
    DestroyGeometry(&geometry);
    DestroyShaders(&shader);
    glfwDestroyWindow(window);
    glfwTerminate();
    
    cout << "Goodbye!" << endl;
    return 0;
}

// ==========================================================================
// SUPPORT FUNCTION DEFINITIONS

// --------------------------------------------------------------------------
// OpenGL utility functions

void QueryGLVersion()
{
    // query opengl version and renderer information
    string version = reinterpret_cast<const char *>(glGetString(GL_VERSION));
    string glslver = reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION));
    string renderer = reinterpret_cast<const char *>(glGetString(GL_RENDERER));
    
    cout << "OpenGL [ " << version << " ] "
    << "with GLSL [ " << glslver << " ] "
    << "on renderer [ " << renderer << " ]" << endl;
}

bool CheckGLErrors()
{
    bool error = false;
    for (GLenum flag = glGetError(); flag != GL_NO_ERROR; flag = glGetError())
    {
        cout << "OpenGL ERROR:  ";
        switch (flag) {
            case GL_INVALID_ENUM:
                cout << "GL_INVALID_ENUM" << endl; break;
            case GL_INVALID_VALUE:
                cout << "GL_INVALID_VALUE" << endl; break;
            case GL_INVALID_OPERATION:
                cout << "GL_INVALID_OPERATION" << endl; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                cout << "GL_INVALID_FRAMEBUFFER_OPERATION" << endl; break;
            case GL_OUT_OF_MEMORY:
                cout << "GL_OUT_OF_MEMORY" << endl; break;
            default:
                cout << "[unknown error code]" << endl;
        }
        error = true;
    }
    return error;
}

// --------------------------------------------------------------------------
// OpenGL shader support functions

// reads a text file with the given name into a string
string LoadSource(const string &filename)
{
    string source;
    
    ifstream input(filename.c_str());
    if (input) {
        copy(istreambuf_iterator<char>(input),
             istreambuf_iterator<char>(),
             back_inserter(source));
        input.close();
    }
    else {
        cout << "ERROR: Could not load shader source from file "
        << filename << endl;
    }
    
    return source;
}

// creates and returns a shader object compiled from the given source
GLuint CompileShader(GLenum shaderType, const string &source)
{
    // allocate shader object name
    GLuint shaderObject = glCreateShader(shaderType);
    
    // try compiling the source as a shader of the given type
    const GLchar *source_ptr = source.c_str();
    glShaderSource(shaderObject, 1, &source_ptr, 0);
    glCompileShader(shaderObject);
    
    // retrieve compile status
    GLint status;
    glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint length;
        glGetShaderiv(shaderObject, GL_INFO_LOG_LENGTH, &length);
        string info(length, ' ');
        glGetShaderInfoLog(shaderObject, info.length(), &length, &info[0]);
        cout << "ERROR compiling shader:" << endl << endl;
        cout << source << endl;
        cout << info << endl;
    }
    
    return shaderObject;
}

// creates and returns a program object linked from vertex and fragment shaders
GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader)
{
    // allocate program object name
    GLuint programObject = glCreateProgram();
    
    // attach provided shader objects to this program
    if (vertexShader)   glAttachShader(programObject, vertexShader);
    if (fragmentShader) glAttachShader(programObject, fragmentShader);
    
    // try linking the program with given attachments
    glLinkProgram(programObject);
    
    // retrieve link status
    GLint status;
    glGetProgramiv(programObject, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint length;
        glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &length);
        string info(length, ' ');
        glGetProgramInfoLog(programObject, info.length(), &length, &info[0]);
        cout << "ERROR linking shader program:" << endl;
        cout << info << endl;
    }
    
    return programObject;
}
