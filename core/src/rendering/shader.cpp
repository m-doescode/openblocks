#include <fstream>
#include <GL/glew.h>
#include <GL/gl.h>
#include <glm/gtc/type_ptr.hpp>

#include "logger.h"
#include "panic.h"
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
        Logger::fatalErrorf("Fragment shader %s failed to compile: [%d]: %s", path.c_str(), success, infoLog);
        panic();
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
        Logger::fatalErrorf("Shader program failed to link: [%d]: %s", success, infoLog);
        panic();
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

void Shader::set(std::string key, int value) {
    glUniform1i(glGetUniformLocation(id, key.c_str()), value);
}

void Shader::set(std::string key, float value) {
    glUniform1f(glGetUniformLocation(id, key.c_str()), value);
}

void Shader::set(std::string key, Material value) {
    set((key + ".diffuse").c_str(), value.diffuse);
    set((key + ".specular").c_str(), value.specular);
    set((key + ".shininess").c_str(), value.shininess);
}

void Shader::set(std::string key, DirLight value) {
    set((key + ".direction").c_str(), value.direction);
    set((key + ".ambient").c_str(), value.ambient);
    set((key + ".diffuse").c_str(), value.diffuse);
    set((key + ".specular").c_str(), value.specular);
}

void Shader::set(std::string key, PointLight value) {
    set((key + ".position").c_str(), value.position);
    set((key + ".ambient").c_str(), value.ambient);
    set((key + ".diffuse").c_str(), value.diffuse);
    set((key + ".specular").c_str(), value.specular);

    set((key + ".constant").c_str(), value.constant);
    set((key + ".linear").c_str(), value.linear);
    set((key + ".quadratic").c_str(), value.quadratic);
}

void Shader::set(std::string key, glm::vec3 value) {
    glUniform3f(glGetUniformLocation(id, key.c_str()), value.x, value.y, value.z);
}

void Shader::set(std::string key, glm::mat3 value) {
    glUniformMatrix3fv(glGetUniformLocation(id, key.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::set(std::string key, glm::mat4 value) {
    glUniformMatrix4fv(glGetUniformLocation(id, key.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}