#include <fstream>
#include <GL/glew.h>
#include <GL/gl.h>

#include "shader.h"

std::string getContents(std::string filePath) {
    std::ifstream ifs(filePath);
    std::string content( (std::istreambuf_iterator<char>(ifs) ),
                       (std::istreambuf_iterator<char>()    ) );

    return content;
}

unsigned int compileShader(std::string path, GLenum type) {
    int success;
    unsigned int shader = glCreateShader(type);

    std::string source = getContents(path);
    const char* source2 = source.c_str();
    glShaderSource(shader, 1, &source2, NULL);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if(success != 1) {
        char infoLog[256];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        printf("Fragment shader %s failed to compile: [%d]: %s\n", path.c_str(), success, infoLog);
        abort();
    }

    return shader;
}

Shader::Shader(std::string vertexShaderPath, std::string fragmentShaderPath) {
    unsigned int vertexShader = compileShader(vertexShaderPath, GL_VERTEX_SHADER);
    unsigned int fragmentShader = compileShader(fragmentShaderPath, GL_FRAGMENT_SHADER);

    int success;

    id = glCreateProgram();
    glAttachShader(id, vertexShader);
    glAttachShader(id, fragmentShader);
    glLinkProgram(id);
    glGetProgramiv(id, GL_LINK_STATUS, &success);
    if(success != 1) {
        char infoLog[256];
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        printf("Shader program failed to link: [%d]: %s\n", success, infoLog);
        abort();
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

Shader::~Shader() {
    glDeleteProgram(id);
}

void Shader::use() {
    glUseProgram(id);
}