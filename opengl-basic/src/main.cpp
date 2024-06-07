#include <glad/glad.h>
#include <glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#define ASSERT(x) if (!(x)) __debugbreak();
#define GLCall(x) glClearError();\
    x;\
    ASSERT(glLogCall(#x, __FILE__, __LINE__))

using namespace std;
using namespace glm;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
unsigned int drawCube();
unsigned int drawTriangle();
unsigned int drawSquare();

static void glClearError()
{
    while (glGetError() != GL_NO_ERROR);
}

static bool glLogCall(const char* function, const char* file, int line)
{
    while (GLenum error = glGetError())
    {
        cout << "[OpenGL ERROR] (" << error << "): " << function <<
           " " << file << ": " << line << endl;
        return false;
    }
    return true;
}

struct ShaderProgramSource
{
    string VertexSource;
    string FragmentSource;
};

static ShaderProgramSource parseShader(const string filePath) 
{
    ifstream stream(filePath);
    enum class ShaderType
    {
        NONE = -1, VERTEX = 0, FRAGMENT = 1
    };
    string line;
    stringstream ss[2];
    ShaderType type = ShaderType::NONE;

    while (getline(stream, line)) 
    {
        if (line.find("#shader") != string::npos)
        {
            if (line.find("vertex") != string::npos)
            {
                // set the mode to vertex
                type = ShaderType::VERTEX;

            }
            else if (line.find("fragment") != string::npos)
            {
                // set the mode to fragment
                type = ShaderType::FRAGMENT;
            }
        }
        else
        {
            // append line to string stream
            ss[(int)type] << line << '\n';
        }
    }
    return {ss[0].str(), ss[1].str()};
}

static unsigned int compileShader(unsigned int type, string& source)
{
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    // Error handling
    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE)
    {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*)alloca(sizeof(char) * length);
        glGetShaderInfoLog(id, length, &length, message);
        cout << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader" << endl;
        cout << message << endl;
        glDeleteShader(id);
        return 0;
    }
    return id;
}

static unsigned int createShader(string& vertexShader, string& fragmentShader)
{
    unsigned int program = glCreateProgram();
    unsigned int vs = compileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fragmentShader);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

int main() {

    const unsigned SCREEN_WIDTH = 800, SCREEN_HEIGHT = 600;

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // creating OpenGL window
    GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Test", NULL, NULL);
    if (window == NULL)
    {
        cout << "Failed to create GLFW window" << endl;
        glfwTerminate();
        return -1;
    }

    // setting the context of the current window
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // initializing GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        cout << "Failed to initialize GLAD" << endl;
        return -1;
    }

    unsigned int vertexCount = drawSquare();

    ShaderProgramSource source = parseShader("opengl-basic/res/shaders/basic.shader");
    unsigned int shader = createShader(source.VertexSource, source.FragmentSource);
    glUseProgram(shader);

    // render loop
    while (!glfwWindowShouldClose(window))
    {
        // handling events
        processInput(window);

        // rendering 
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        GLCall(glDrawElements(GL_TRIANGLES, vertexCount, GL_INT, nullptr));

        glfwSwapBuffers(window);    // swap back and front buffer
        glfwPollEvents();   // poll for IO events
    }

    glDeleteProgram(shader);
    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
}

unsigned int drawTriangle() 
{
    float vertices[] = {
        -0.5f, -0.5f, // 0
         0.5f, -0.5f, // 1
         0.5f, 0.5f   // 2
    };

 
    unsigned int buffer, vertexArr;
    
    glGenBuffers(1, &buffer);
    glGenVertexArrays(1, &vertexArr);
    
    glBindVertexArray(vertexArr);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0);
    glEnableVertexAttribArray(0);

    return 3;
}

unsigned int drawSquare()
{
    float vertices[] = {
        -0.5f, -0.5f, //0
         0.5f, -0.5f, //1
         0.5f, 0.5f,  //2
        -0.5f, 0.5f   //3
    };

    // using index buffer
    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0
    };

    unsigned int buffer, vertexArr;
        
    glGenBuffers(1, &buffer);
    glGenVertexArrays(1, &vertexArr);

    glBindVertexArray(vertexArr);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0);
    glEnableVertexAttribArray(0);

    unsigned int ibo;

    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    return 6;
}

// we're not using index buffers here
unsigned int drawCube()
{
    float vertices[] = {
        -0.5f, -0.5f, -0.5f,  
         0.5f, -0.5f, -0.5f,  
         0.5f,  0.5f, -0.5f,  
         0.5f,  0.5f, -0.5f,  
        -0.5f,  0.5f, -0.5f,  
        -0.5f, -0.5f, -0.5f,  

        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,

        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        
         0.5f,  0.5f,  0.5f, 
         0.5f,  0.5f, -0.5f, 
         0.5f, -0.5f, -0.5f, 
         0.5f, -0.5f, -0.5f, 
         0.5f, -0.5f,  0.5f, 
         0.5f,  0.5f,  0.5f, 

        -0.5f, -0.5f, -0.5f, 
         0.5f, -0.5f, -0.5f, 
         0.5f, -0.5f,  0.5f, 
         0.5f, -0.5f,  0.5f, 
        -0.5f, -0.5f,  0.5f, 
        -0.5f, -0.5f, -0.5f, 

        -0.5f,  0.5f, -0.5f, 
         0.5f,  0.5f, -0.5f, 
         0.5f,  0.5f,  0.5f, 
         0.5f,  0.5f,  0.5f, 
        -0.5f,  0.5f,  0.5f, 
        -0.5f,  0.5f, -0.5f,
    };

    unsigned int buffer, vertexArr;

    glGenBuffers(1, &buffer);
    glGenVertexArrays(1, &vertexArr);

    glBindVertexArray(vertexArr);

    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(0);

    return 36;
}