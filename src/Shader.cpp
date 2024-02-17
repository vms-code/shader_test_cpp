
#include "Shader.hpp"

#include <cmath>
#include <fstream>
#include <iostream>
#include <list>
#include <sstream>
#include <string>
#include <vector>
// #include "renderer/gl/UniformUtils.hpp"

void CheckCompilationErrors(GLuint shaderId, GLenum shaderType) {
    GLint success;
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);
    GLchar infoLog[512];
    glGetShaderInfoLog(shaderId, sizeof(infoLog), nullptr, infoLog);
    if (!success) {
        std::cerr << "Error compiling " << (shaderType == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader:"
                  << infoLog << std::endl;
        throw std::runtime_error("Shader compilation failed");
    }
}

inline unsigned int createShader(int shaderType, const char* sourceCode) {
    if (glCreateShader == nullptr) {
        std::cout << "glCreateShader is a nullptr: " << glCreateShader
                  << std::endl;
    }

    GLuint shaderId = glCreateShader(shaderType);
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "Program.cpp OpenGL error: " << error << std::endl;
        // Handle the error as needed
    }

    glShaderSource(shaderId, 1, &sourceCode, nullptr);
    glCompileShader(shaderId);

    // Check for compilation errors
    CheckCompilationErrors(shaderId, shaderType);

    return shaderId;
}

std::unordered_map<std::string, GLint> fetchAttributeLocations(GLuint program) {
    std::unordered_map<std::string, GLint> attributes;

    GLint n;
    glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &n);

    GLsizei length;
    GLint size;
    GLenum type;
    GLchar nameBuffer[256];
    for (int i = 0; i < n; ++i) {
        glGetActiveAttrib(program, i, 256, &length, &size, &type, nameBuffer);

        std::string name(nameBuffer, length);
        attributes[name] = glGetAttribLocation(program, name.c_str());
    }

    return attributes;
}

Shader::Shader(const std::string& vertexShaderPath, const std::string& fragmentShaderPath) {
    createProgram(vertexShaderPath, fragmentShaderPath);
    // cache uniforms
    getUniforms();
    getAttributes();
}

void Shader::createProgram(const std::string& vertexShaderPath, const std::string& fragmentShaderPath) {
    // Read shader source code from files
    std::string vertexGlsl = ReadShaderFile(vertexShaderPath);
    std::string fragmentGlsl = ReadShaderFile(fragmentShaderPath);

    // Compile shaders
    const auto glVertexShader = createShader(GL_VERTEX_SHADER, vertexGlsl.c_str());
    const auto glFragmentShader = createShader(GL_FRAGMENT_SHADER, fragmentGlsl.c_str());

    this->program = glCreateProgram();
    glAttachShader(program, glVertexShader);
    glAttachShader(program, glFragmentShader);

    glLinkProgram(program);

    glDeleteShader(glVertexShader);
    glDeleteShader(glFragmentShader);
}

std::string Shader::ReadShaderFile(const std::string& filePath) const {
    std::ifstream shaderFile(filePath);
    if (!shaderFile.is_open()) {
        std::cerr << "Error reading shader file: " << filePath << std::endl;
        throw std::runtime_error("Failed to read shader file");
    }

    std::stringstream shaderStream;
    shaderStream << shaderFile.rdbuf();
    shaderFile.close();

    return shaderStream.str();
}

// vertexSize is size of point(Vector2, Vector3, etc...) = 2, 3, etc...
void Shader::createBuffer(std::string attributeName, std::vector<float> vertexData, GLenum usage, GLint vertexSize, GLsizei stride, size_t offset) {
    int attributeLocation = cachedAttributes[attributeName];

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    // Upload vertex data
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(GL_FLOAT), vertexData.data(), usage);
    glEnableVertexAttribArray(attributeLocation);
    glVertexAttribPointer(attributeLocation, vertexSize, GL_FLOAT, GL_FALSE, stride, (GLvoid*)offset);
    glBindVertexArray(0);
}

std::unordered_map<std::string, int> Shader::getAttributes() {
    if (cachedAttributes.empty()) {
        cachedAttributes = fetchAttributeLocations(program);
    }

    return cachedAttributes;
}

std::unordered_map<std::string, UniformInfo> Shader::getUniforms() {
    if (uniformMap.empty()) {
        int numUniforms;
        glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &numUniforms);

        for (int i = 0; i < numUniforms; ++i) {
            const GLsizei bufSize = 256;
            GLsizei length;
            GLint size;
            GLenum type;
            GLchar nameBuffer[bufSize];

            glGetActiveUniform(program, i, bufSize, &length, &size, &type, nameBuffer);

            // Construct a std::string from the buffer with the correct length
            std::string name(nameBuffer, length);

            UniformInfo info;
            info.type = type;
            info.name = name;
            info.size = size;
            info.location = glGetUniformLocation(program, name.c_str());

            uniformMap[name] = info;
        }
    }

    return uniformMap;
}

void Shader::set_glUniform1f(const std::string& uniformName, const float& newValue) {
    auto it = uniformMap.find(uniformName);

    if (it != uniformMap.end()) {
        // Uniform found in the map
        UniformInfo& info = it->second;
        glUniform1f(info.location, newValue);
        // Check for OpenGL errors
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            // Handle error (print error message, log, etc.)
            std::cerr << "OpenGL error after setting glUniform1f variable: " << error << std::endl;
        }
    } else {
        // Uniform not found in the map
        std::cout << "Uniform '" << uniformName << "' not found in the map." << std::endl;
    }
}

void Shader::set_glUniform3f(const std::string& uniformName, const Color& newValue) {
    auto it = uniformMap.find(uniformName);

    if (it != uniformMap.end()) {
        // Uniform found in the map
        UniformInfo& info = it->second;
        glUniform3f(info.location, newValue.r, newValue.g, newValue.b);
    } else {
        // Uniform not found in the map
        std::cout << "Uniform '" << uniformName << "' not found in the map." << std::endl;
    }
}

void Shader::set_glUniformMatrix3fv(const std::string& uniformName, const Matrix3& newValue) {
    auto it = uniformMap.find(uniformName);

    if (it != uniformMap.end()) {
        // Uniform found in the map
        UniformInfo& info = it->second;
        glUniformMatrix3fv(info.location, 1, false, newValue.elements.data());
    } else {
        // Uniform not found in the map
        std::cout << "Uniform '" << uniformName << "' not found in the map." << std::endl;
    }
}

void Shader::set_glUniformMatrix4fv(const std::string& uniformName, const Matrix4 newValue) {
    glUseProgram(program);
    auto it = uniformMap.find(uniformName);

    if (it != uniformMap.end()) {
        // Uniform found in the map
        UniformInfo& info = it->second;
        glUniformMatrix4fv(info.location, 1, false, newValue.elements.data());
        // Check for OpenGL errors
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            // Handle error (print error message, log, etc.)
            std::cerr << "OpenGL error after setting UniformMatrix4fv variable: " << error << std::endl;
        }
    } else {
        // Uniform not found in the map
        std::cout << "Uniform '" << uniformName << "' not found in the map." << std::endl;
    }
}

// Use the shader program
void Shader::Use() const {
    glUseProgram(program);
}

void Shader::render(unsigned int mode_, int start, int count) {
    glUseProgram(program);
    glBindVertexArray(VAO);
    glDrawArrays(mode_, start, count);
}

void Shader::renderIndexAttribute(int start, int count) {
    // auto buffer = buffers_.at(indexAttribute);
    // glDrawElements(mode_, count, buffer.type, (GLvoid*)(start * buffer.bytesPerElement));
}

void Shader::destroy() {
    glDeleteProgram(program);
    this->program = -1;
}
