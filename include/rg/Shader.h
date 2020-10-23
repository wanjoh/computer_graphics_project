//
// Created by spaske on 22.10.20..
//

#ifndef PROJECT_BASE_SHADER_H
#define PROJECT_BASE_SHADER_H

#include <fstream> // Because we need compile-time breaks :)
#include <rg/Error.h>
#include <sstream>

struct ShaderFailedToCompileError : public std::exception {
    std::string message;
    explicit ShaderFailedToCompileError(std::string msg)
    : message(msg) {}
};

struct ProgramFailedToLinkError : public std::exception {
    std::string message;
    explicit ProgramFailedToLinkError(std::string msg)
    : message(msg){}
};

class Shader {
    unsigned int ShaderProgramId{0};

public:
    using ShaderId = int;
    static std::string loadShaderSource(const char* shaderPath);
    static bool shaderCompiledSuccessfully(int shader);
    static ShaderId compileShader(const char* shaderSource, GLenum shaderType) {
        int shader = glCreateShader(shaderType);
        glShaderSource(shader, 1, &shaderSource, nullptr);
        glCompileShader(shader);
        return shader;
    }
    Shader(const char* vertexShaderPath, const char *fragmentShaderPath);
    void useProgram();
    void setInt(const std::string& name, int value);
    void setBool(const std::string& name, bool value);
    void setFloat(const std::string& name, float value);

    void deleteShaderProgram();
    ~Shader();
};

Shader::Shader(const char *vertexShaderPath, const char *fragmentShaderPath) {
    std::string vertexShaderSource(Shader::loadShaderSource(vertexShaderPath));
    std::string fragmentShaderSource(Shader::loadShaderSource(fragmentShaderPath));

    ShaderId vertexShaderId = compileShader(vertexShaderSource.c_str(), GL_VERTEX_SHADER);
    if (!shaderCompiledSuccessfully(vertexShaderId)) {
        throw ShaderFailedToCompileError{"Vertex shader failed to compile!"};
    }

    ShaderId fragmentShaderId = compileShader(fragmentShaderSource.c_str(), GL_FRAGMENT_SHADER);
    if (!shaderCompiledSuccessfully(fragmentShaderId)) {
        throw ShaderFailedToCompileError{"Fragment shader failed to compile!"};
    }

    ShaderProgramId = glCreateProgram();
    glAttachShader(ShaderProgramId, vertexShaderId);
    glAttachShader(ShaderProgramId, fragmentShaderId);
    glLinkProgram(ShaderProgramId);

    glDeleteShader(vertexShaderId);
    glDeleteShader(fragmentShaderId);
    int success;
    char infoLog[4096];
    glGetProgramiv(ShaderProgramId, GL_LINK_STATUS, &success);
    if (!success) {

    }


}

Shader::~Shader() {

}

void Shader::useProgram() {

}

void Shader::setInt(const std::string &name, int value) {

}

void Shader::setBool(const std::string &name, bool value) {

}

void Shader::setFloat(const std::string &name, float value) {

}

void Shader::deleteShaderProgram() {

}

std::string Shader::loadShaderSource(const char *shaderPath) {
    std::ifstream fileStream;
    fileStream.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fileStream.open(shaderPath);
    std::ostringstream shaderSourceStream;
    shaderSourceStream << fileStream.rdbuf();
    return shaderSourceStream.str();
}

bool Shader::shaderCompiledSuccessfully(int shader) {
    char infoLog[4096];
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, sizeof(infoLog), nullptr, infoLog);
        std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n"
        << "ShaderId: " << shader
        << "\nMessage: " << infoLog << "\n";
    }
    return success;
}

#endif //PROJECT_BASE_SHADER_H
