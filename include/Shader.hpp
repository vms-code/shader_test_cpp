#ifndef GRAPHICS_PROGRAM_HPP
#define GRAPHICS_PROGRAM_HPP

#include <glad/gl.h>
//

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

//
#include "math/Color.hpp"
#include "math/Matrix3.hpp"
#include "math/Matrix4.hpp"

using namespace graphics;

struct Buffer {
    unsigned int buffer{};
    int type{};
    int bytesPerElement{};
    unsigned int version{};
};

struct UniformInfo {
    GLenum type;
    std::string name;
    GLint size;
    GLint location;
};

struct Shader {
    Shader(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
    std::string ReadShaderFile(const std::string& filePath) const;
    void createProgram(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);

    // buffer functions
    void createBuffer(std::string attributeName, std::vector<float> vertexData, GLenum usage,
                      GLint vertexSize, GLsizei stride, size_t offset);

    std::unordered_map<std::string, int> getAttributes();
    std::unordered_map<std::string, UniformInfo> getUniforms();

    // set uniform functions
    void set_glUniform1f(const std::string& uniformName, const float& newValue);
    void set_glUniform3f(const std::string& uniformName, const Color& newValue);
    void set_glUniformMatrix3fv(const std::string& uniformName, const Matrix3& newValue);
    void set_glUniformMatrix4fv(const std::string& uniformName, const Matrix4 newValue);

    // render functions
    void Use() const;
    void render(unsigned int mode_, int start, int count);
    void renderIndexAttribute(int start, int count);
    void destroy();

   private:
    std::unordered_map<std::string, UniformInfo> uniformMap;
    std::unordered_map<std::string, int> cachedAttributes;
    std::unordered_map<std::string, Buffer> buffers_;

    // buffer variables
    unsigned int program = -1;
    GLuint VAO;
    GLuint VBO;

    Shader() = default;
};

#endif
