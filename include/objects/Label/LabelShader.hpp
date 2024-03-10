#ifndef GRAPHICS_LABELSHADER_HPP
#define GRAPHICS_LABELSHADER_HPP

#include <glad/gl.h>
//

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

//
#include "Label/helpers.hpp"
#include "Shader.hpp"
#include "math/Color.hpp"
#include "math/Matrix3.hpp"
#include "math/Matrix4.hpp"
#include "math/Vector2.hpp"
#include "objects/Object3D.hpp"

namespace graphics {

struct LabelShader : public Object3D {
    float opacity = 1.0;
    int textLineSpacing = 15;
    Font font = { 0 };
    const char* labelText = "Signed Distance Fields";
    Vector2 textSize = { 0.0f, 0.0f };
    float fontSize = 2.0f;
    float spacing = 0.0f;
    Color tint = { 0.0f, 1.0f, 1.0f };

    LabelShader(const char* labelName, const std::string& vertexShaderPath, const std::string& fragmentShaderPath, const std::string& fontPath);
    std::string ReadShaderFile(const std::string& filePath) const;
    void createProgram(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
    // Convert image data to OpenGL texture (returns OpenGL valid Id)
    unsigned int loadTexture(const void* data, int width, int height, int format, int mipmapCount);
    Texture LoadTextureFromImage(Image image);
    void init_font(const std::string& fontPath);
    Vector2 MeasureTextEx(const char* text, float fontSize, float spacing);

    std::unordered_map<std::string, int> getAttributes();
    std::unordered_map<std::string, UniformInfo> getUniforms();

    // set uniform functions
    void set_glUniform1f(const std::string& uniformName, const float& newValue);
    void set_glUniform3f(const std::string& uniformName, const Color& newValue);
    void set_glUniformMatrix3fv(const std::string& uniformName, const Matrix3& newValue);
    void set_glUniformMatrix4fv(const std::string& uniformName, const Matrix4 newValue);

    void set_raylib_projection();

    // vertex building functions
    void DrawTexturePro(Rectangle source, Rectangle dest, Vector2 origin, float rotation);
    void DrawTextCodepoint(int codepoint, Vector2 position);
    void DrawTextCodepoint3D(int codepoint, Vector3 position, bool backface);
    void DrawText3D(Vector3 position, bool backface);
    // Draw text using Font
    // NOTE: chars spacing is NOT proportional to fontSize
    void DrawTextEx(Vector3 position);
    void DrawTexture(int posX, int posY, float rotation, float scale);
    void buildIndexBufferData();
    void buildVertices(Vector3 fontPosition);
    // buffer functions
    void createTextBuffer(GLenum usage);

    // render functions
    void Use() const;
    void render();
    void renderIndexAttribute(int start, int count);
    void destroy();

   private:
    std::unordered_map<std::string, UniformInfo> uniformMap;
    std::unordered_map<std::string, int> cachedAttributes;
    std::unordered_map<std::string, Buffer> buffers_;

    // buffer variables
    unsigned int program = -1;
    unsigned int textureId = 0;
    struct Texture texture = { 0 };
    GLuint VAO;
    GLuint pointsVBO;
    GLuint textCoordsVBO;
    GLuint indexVBO;
    std::vector<float> vertexData;  // both vertex position and texCoords points should be stored here in order to set attrib and update buffers
    std::vector<float> textCoordsData;
    std::vector<unsigned int> indexData;  // both vertex position and texCoords points should be stored here in order to set attrib and update buffers

    LabelShader() = default;
};

}  // namespace graphics

#endif
