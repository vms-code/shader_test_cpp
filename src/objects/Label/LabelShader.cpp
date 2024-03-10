
#include "Label/LabelShader.hpp"

#include <cmath>
#include <fstream>
#include <iostream>
#include <list>
#include <sstream>
#include <string>
#include <vector>

#include "Label/helpers.hpp"
#include "math/MathUtils.hpp"
#include "math/Vector2.hpp"
#include "math/Vector3.hpp"

void CheckCompilationErrors(GLuint shaderId, GLenum shaderType);
inline unsigned int createShader(int shaderType, const char* sourceCode);
std::unordered_map<std::string, GLint> fetchAttributeLocations(GLuint program);

void print_opengl_error() {
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL ERROR: " << error << std::endl;
    }
    if (error == GL_INVALID_ENUM) {
        std::cerr << "GL_INVALID_ENUM: " << error << std::endl;
    }
    if (error == GL_INVALID_VALUE) {
        std::cerr << "GL_INVALID_VALUE: " << error << std::endl;
    }
    if (error == GL_INVALID_OPERATION) {
        std::cerr << "GL_INVALID_OPERATION: " << error << std::endl;
    }
    if (error == GL_INVALID_FRAMEBUFFER_OPERATION) {
        std::cerr << "GL_INVALID_FRAMEBUFFER_OPERATION: " << error << std::endl;
    }
    if (error == GL_OUT_OF_MEMORY) {
        std::cerr << "GL_OUT_OF_MEMORY: " << error << std::endl;
    }
    if (error == GL_STACK_UNDERFLOW) {
        std::cerr << "GL_STACK_UNDERFLOW: " << error << std::endl;
    }
    if (error == GL_STACK_OVERFLOW) {
        std::cerr << "GL_STACK_OVERFLOW: " << error << std::endl;
    }
}

graphics::Vector2 graphics::LabelShader::MeasureTextEx(const char* text, float fontSize, float spacing) {
    graphics::Vector2 textSize = { 0, 0 };

    if ((font.texture.id == 0) || (text == NULL)) return textSize;

    int size = TextLength(text);  // Get size in bytes of text
    int tempByteCounter = 0;      // Used to count longer text line num chars
    int byteCounter = 0;

    float textWidth = 0.0f;
    float tempTextWidth = 0.0f;  // Used to count longer text line width

    float textHeight = fontSize;
    float scaleFactor = fontSize / (float)font.baseSize;

    int letter = 0;  // Current character
    int index = 0;   // Index position in sprite font

    for (int i = 0; i < size;) {
        byteCounter++;

        int next = 0;
        letter = GetCodepointNext(&text[i], &next);
        index = GetGlyphIndex(font, letter);

        i += next;

        if (letter != '\n') {
            if (font.glyphs[index].advanceX != 0)
                textWidth += font.glyphs[index].advanceX;
            else
                textWidth += (font.recs[index].width + font.glyphs[index].offsetX);
        } else {
            if (tempTextWidth < textWidth) tempTextWidth = textWidth;
            byteCounter = 0;
            textWidth = 0;

            // NOTE: Line spacing is a global variable, use SetTextLineSpacing() to setup
            textHeight += (float)textLineSpacing;
        }

        if (tempByteCounter < byteCounter) tempByteCounter = byteCounter;
    }

    if (tempTextWidth < textWidth) tempTextWidth = textWidth;

    textSize.x = tempTextWidth * scaleFactor + (float)((tempByteCounter - 1) * spacing);
    textSize.y = textHeight;

    return textSize;
}

graphics::LabelShader::LabelShader(const char* labelName, const std::string& vertexShaderPath, const std::string& fragmentShaderPath, const std::string& fontPath) {
    // labelText = labelName;
    createProgram(vertexShaderPath, fragmentShaderPath);
    init_font(fontPath);
    // cache uniforms
    getUniforms();
    getAttributes();
    // build vertices before creating buffers (first argument is text position)
    // buildVertices({ this->position.x, this->position.y });  // create vertexData points and texture data
    textSize = MeasureTextEx(labelName, fontSize, 0);

    // this->position.z = -1;
    //  float x = GetScreenWidth() / 2 - textSize.x / 2;
    //  float y = GetScreenHeight() / 2 - textSize.y / 2 + 80;
    //  buildVertices({ 312.0f, 297.0f });  // create vertexData points and texture data
    buildVertices({ 0.0f, 0.0f, 0.0f });  // create vertexData points and texture data
    // std::cout << "position x: " << x << std::endl;
    // std::cout << "position y: " << y << std::endl;
    std::cout << "position z: " << this->position.z << std::endl;
    // build buffers using vertexData
    createTextBuffer(GL_DYNAMIC_DRAW);  // previous: GL_STREAM_DRAW, can also be: GL_STATIC_DRAW
}

void graphics::LabelShader::createProgram(const std::string& vertexShaderPath, const std::string& fragmentShaderPath) {
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

std::string LabelShader::ReadShaderFile(const std::string& filePath) const {
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

// Textures data management
//-----------------------------------------------------------------------------------------
// Convert image data to OpenGL texture (returns OpenGL valid Id)
unsigned int graphics::LabelShader::loadTexture(const void* data, int width, int height, int format, int mipmapCount) {
    glBindTexture(GL_TEXTURE_2D, 0);  // Free any old binding
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glGenTextures(1, &texture.id);  // Generate texture id

    glBindTexture(GL_TEXTURE_2D, texture.id);

    int mipWidth = width;
    int mipHeight = height;
    int mipOffset = 0;  // Mipmap data offset, only used for tracelog

    // NOTE: Added pointer math separately from function to avoid UBSAN complaining
    unsigned char* dataPtr = NULL;
    if (data != NULL) dataPtr = (unsigned char*)data;

    // Load the different mipmap levels
    for (int i = 0; i < mipmapCount; i++) {
        unsigned int mipSize = rlGetPixelDataSize(mipWidth, mipHeight, format);

        unsigned int glInternalFormat, glFormat, glType;
        rlGetGlTextureFormats(format, &glInternalFormat, &glFormat, &glType);

        // TRACELOG("TEXTURE: Load mipmap level %i (%i x %i), size: %i, offset: %i", i, mipWidth, mipHeight, mipSize, mipOffset);

        if (glInternalFormat != 0) {
            if (format < RL_PIXELFORMAT_COMPRESSED_DXT1_RGB) glTexImage2D(GL_TEXTURE_2D, i, glInternalFormat, mipWidth, mipHeight, 0, glFormat, glType, dataPtr);

            if (format == RL_PIXELFORMAT_UNCOMPRESSED_GRAYSCALE) {
                GLint swizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_ONE };
                glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
            } else if (format == RL_PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA) {
                GLint swizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_GREEN };
                glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
            }
        }

        mipWidth /= 2;
        mipHeight /= 2;
        mipOffset += mipSize;                  // Increment offset position to next mipmap
        if (data != NULL) dataPtr += mipSize;  // Increment data pointer to next mipmap

        // Security check for NPOT textures
        if (mipWidth < 1) mipWidth = 1;
        if (mipHeight < 1) mipHeight = 1;
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);  // Set texture to repeat on x-axis
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);  // Set texture to repeat on y-axis
    // Magnification and minification filters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);  // Alternative: GL_LINEAR
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);  // Alternative: GL_LINEAR

    if (mipmapCount > 1) {
        // Activate Trilinear filtering if mipmaps are available
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    }

    // At this point we have the texture loaded in GPU and texture parameters configured

    // NOTE: If mipmaps were not in data, they are not generated automatically

    // Unbind current texture
    glBindTexture(GL_TEXTURE_2D, 0);

    if (id > 0)
        TRACELOG(RL_LOG_INFO, "TEXTURE: [ID %i] Texture loaded successfully (%ix%i | %s | %i mipmaps)", id, width, height, rlGetPixelFormatName(format), mipmapCount);
    else
        TRACELOG(RL_LOG_WARNING, "TEXTURE: Failed to load texture");

    return id;
}

// Load a texture from image data
// NOTE: image is not unloaded, it must be done manually
Texture graphics::LabelShader::LoadTextureFromImage(Image image) {
    if ((image.width != 0) && (image.height != 0)) {
        loadTexture(image.data, image.width, image.height, image.format, image.mipmaps);
    } else
        TRACELOG(LOG_WARNING, "IMAGE: Data is not valid to load texture");

    texture.width = image.width;
    texture.height = image.height;
    texture.mipmaps = image.mipmaps;
    texture.format = image.format;

    return texture;
}

// Unload image from CPU memory (RAM)
void UnloadImage(Image image) {
    free(image.data);
}

void graphics::LabelShader::init_font(const std::string& fontPath) {
    // Loading file to memory
    int fileSize = 0;

    // unsigned char* fileData = LoadFileData(fontPath.c_str(), &fileSize);
    unsigned char* fileData = LoadFileData("C:\\Users\\vitor\\Documents\\CODE\\C++\\ShaderTraining\\simple_test_project\\build\\assets\\fonts\\anonymous_pro_bold.ttf", &fileSize);

    // Default font generation from TTF font
    font.baseSize = 16;
    font.glyphCount = 95;
    // Parameters > font size: 16, no glyphs array provided (0), glyphs count: 0 (defaults to 95)
    font.glyphs = LoadFontData(fileData, fileSize, 16, 0, 0, FONT_SDF);
    // Parameters > glyphs count: 95, font size: 16, glyphs padding in image: 0 px, pack method: 1 (Skyline algorythm)
    Image atlas = GenImageFontAtlas(font.glyphs, &font.recs, 95, 16, 0, 1);
    font.texture = LoadTextureFromImage(atlas);
    UnloadImage(atlas);

    free(fileData);  // Free memory from loaded file

    SetTextureFilter(texture, TEXTURE_FILTER_BILINEAR);  // Required for SDF font
}

std::unordered_map<std::string, int> graphics::LabelShader::getAttributes() {
    if (cachedAttributes.empty()) {
        cachedAttributes = fetchAttributeLocations(program);
    }

    return cachedAttributes;
}

std::unordered_map<std::string, UniformInfo> graphics::LabelShader::getUniforms() {
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

void graphics::LabelShader::set_glUniform1f(const std::string& uniformName, const float& newValue) {
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

void graphics::LabelShader::set_glUniform3f(const std::string& uniformName, const Color& newValue) {
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

void graphics::LabelShader::set_glUniformMatrix3fv(const std::string& uniformName, const Matrix3& newValue) {
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

void graphics::LabelShader::set_glUniformMatrix4fv(const std::string& uniformName, const Matrix4 newValue) {
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

void graphics::LabelShader::set_raylib_projection() {
    glUseProgram(program);
    auto it = uniformMap.find("projection");
    float matMVPfloat[16] = {
        0.00249999994, 0.00000000, 0.00000000, 0.00000000,
        0.00000000, -0.00444444455, 0.00000000, 0.00000000,
        0.00000000, 0.00000000, -2.00000000, 0.00000000,
        -1.00000000, 1.00000000, -1.00000000, 1.00000000
    };

    if (it != uniformMap.end()) {
        // Uniform found in the map
        UniformInfo& info = it->second;
        glUniformMatrix4fv(info.location, 1, false, matMVPfloat);
        // Check for OpenGL errors
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            // Handle error (print error message, log, etc.)
            std::cerr << "OpenGL error after setting UniformMatrix4fv variable: " << error << std::endl;
        }
    } else {
        // Uniform not found in the map
        std::cout << "FAILED TO SET RAYLIB PROJECTION MATRIX ON TEXT SHADER" << std::endl;
    }
}

//----------------------------------------------------------------------------------
// Text vertices creation functions
//----------------------------------------------------------------------------------

// Draw a part of a texture (defined by a rectangle) with 'pro' parameters
// NOTE: origin is relative to destination rectangle size
void graphics::LabelShader::DrawTexturePro(Rectangle source, Rectangle dest, graphics::Vector2 origin, float rotation) {
    // Check if texture is valid
    if (texture.id > 0) {
        float width = (float)texture.width;
        float height = (float)texture.height;

        bool flipX = false;

        if (source.width < 0) {
            flipX = true;
            source.width *= -1;
        }
        if (source.height < 0) source.y -= source.height;

        graphics::Vector2 topLeft = { 0, 0 };
        graphics::Vector2 topRight = { 0, 0 };
        graphics::Vector2 bottomLeft = { 0, 0 };
        graphics::Vector2 bottomRight = { 0, 0 };

        // Only calculate rotation if needed
        if (rotation == 0.0f) {
            float x = dest.x - origin.x;
            float y = dest.y - origin.y;
            topLeft = { x, y };
            topRight = { x + dest.width, y };
            bottomLeft = { x, y + dest.height };
            bottomRight = { x + dest.width, y + dest.height };
        } else {
            float sinRotation = sinf(rotation * math::PI / 180.0f);
            float cosRotation = cosf(rotation * math::PI / 180.0f);
            float x = dest.x;
            float y = dest.y;
            float dx = -origin.x;
            float dy = -origin.y;

            topLeft.x = x + dx * cosRotation - dy * sinRotation;
            topLeft.y = y + dx * sinRotation + dy * cosRotation;

            topRight.x = x + (dx + dest.width) * cosRotation - dy * sinRotation;
            topRight.y = y + (dx + dest.width) * sinRotation + dy * cosRotation;

            bottomLeft.x = x + dx * cosRotation - (dy + dest.height) * sinRotation;
            bottomLeft.y = y + dx * sinRotation + (dy + dest.height) * cosRotation;

            bottomRight.x = x + (dx + dest.width) * cosRotation - (dy + dest.height) * sinRotation;
            bottomRight.y = y + (dx + dest.width) * sinRotation + (dy + dest.height) * cosRotation;
        }

        // rlSetTexture(texture.id);
        // rlBegin(RL_QUADS);

        // rlColor4ub(tint.r, tint.g, tint.b, opacity);
        // rlNormal3f(0.0f, 0.0f, 1.0f);  // Normal vector pointing towards viewer

        // Top-left corner for texture and quad
        // rlVertex2f(topLeft.x, topLeft.y);
        vertexData.emplace_back(topLeft.x);
        vertexData.emplace_back(topLeft.y);
        vertexData.emplace_back(this->position.z);
        if (flipX) {
            // rlTexCoord2f((source.x + source.width) / width, source.y / height);
            textCoordsData.emplace_back((source.x + source.width) / width);
            textCoordsData.emplace_back(source.y / height);
        } else {
            // rlTexCoord2f(source.x / width, source.y / height);
            textCoordsData.emplace_back(source.x / width);
            textCoordsData.emplace_back(source.y / height);
        }

        // Bottom-left corner for texture and quad
        // rlVertex2f(bottomLeft.x, bottomLeft.y);
        vertexData.emplace_back(bottomLeft.x);
        vertexData.emplace_back(bottomLeft.y);
        vertexData.emplace_back(this->position.z);
        if (flipX) {
            // rlTexCoord2f((source.x + source.width) / width, (source.y + source.height) / height);
            textCoordsData.emplace_back((source.x + source.width) / width);
            textCoordsData.emplace_back((source.y + source.height) / height);
        } else {
            // rlTexCoord2f(source.x / width, (source.y + source.height) / height);
            textCoordsData.emplace_back(source.x / width);
            textCoordsData.emplace_back((source.y + source.height) / height);
        }

        // Bottom-right corner for texture and quad
        // rlVertex2f(bottomRight.x, bottomRight.y);
        vertexData.emplace_back(bottomRight.x);
        vertexData.emplace_back(bottomRight.y);
        vertexData.emplace_back(this->position.z);
        if (flipX) {
            // rlTexCoord2f(source.x / width, (source.y + source.height) / height);
            textCoordsData.emplace_back(source.x / width);
            textCoordsData.emplace_back((source.y + source.height) / height);
        } else {
            // rlTexCoord2f((source.x + source.width) / width, (source.y + source.height) / height);
            textCoordsData.emplace_back((source.x + source.width) / width);
            textCoordsData.emplace_back((source.y + source.height) / height);
        }

        // Top-right corner for texture and quad
        // rlVertex2f(topRight.x, topRight.y);
        vertexData.emplace_back(topRight.x);
        vertexData.emplace_back(topRight.y);
        vertexData.emplace_back(this->position.z);
        if (flipX) {
            // rlTexCoord2f(source.x / width, source.y / height);
            textCoordsData.emplace_back(source.x / width);
            textCoordsData.emplace_back(source.y / height);
        } else {
            // rlTexCoord2f((source.x + source.width) / width, source.y / height);
            textCoordsData.emplace_back((source.x + source.width) / width);
            textCoordsData.emplace_back(source.y / height);
        }

        // rlEnd();
        // rlSetTexture(0);
    }
}

void graphics::LabelShader::DrawTextCodepoint(int codepoint, graphics::Vector2 position) {
    // Character index position in sprite font
    // NOTE: In case a codepoint is not available in the font, index returned points to '?'
    int index = GetGlyphIndex(font, codepoint);
    float scaleFactor = fontSize / font.baseSize;  // Character quad scaling factor
    std::cout << "scaleFactor: " << scaleFactor << std::endl;

    // Character destination rectangle on screen
    // NOTE: We consider glyphPadding on drawing
    Rectangle dstRec = { position.x + font.glyphs[index].offsetX * scaleFactor - (float)font.glyphPadding * scaleFactor,
                         position.y + font.glyphs[index].offsetY * scaleFactor - (float)font.glyphPadding * scaleFactor,
                         (font.recs[index].width + 2.0f * font.glyphPadding) * scaleFactor,
                         (font.recs[index].height + 2.0f * font.glyphPadding) * scaleFactor };

    std::cout << "font.glyphs[index].offsetX: " << font.glyphs[index].offsetX << std::endl;
    std::cout << "font.glyphs[index].offsetY: " << font.glyphs[index].offsetY << std::endl;
    std::cout << "font.glyphPadding: " << (float)font.glyphPadding << std::endl;

    // Character source rectangle from font texture atlas
    // NOTE: We consider chars padding when drawing, it could be required for outline/glow shader effects
    Rectangle srcRec = { font.recs[index].x - (float)font.glyphPadding, font.recs[index].y - (float)font.glyphPadding,
                         font.recs[index].width + 2.0f * font.glyphPadding, font.recs[index].height + 2.0f * font.glyphPadding };

    std::cout << "----------------------------------------------------------" << std::endl;
    // Draw the character texture on the screen
    DrawTexturePro(srcRec, dstRec, { 0, 0 }, 0.0f);
}

// Draw codepoint at specified position in 3D space
void graphics::LabelShader::DrawTextCodepoint3D(int codepoint, graphics::Vector3 position, bool backface) {
    // Character index position in sprite font
    // NOTE: In case a codepoint is not available in the font, index returned points to '?'
    int index = GetGlyphIndex(font, codepoint);
    float scale = fontSize / (float)font.baseSize;

    // Character destination rectangle on screen
    // NOTE: We consider charsPadding on drawing
    position.x += (float)(font.glyphs[index].offsetX - font.glyphPadding) / (float)font.baseSize * scale;
    position.z += (float)(font.glyphs[index].offsetY - font.glyphPadding) / (float)font.baseSize * scale;

    // Character source rectangle from font texture atlas
    // NOTE: We consider chars padding when drawing, it could be required for outline/glow shader effects
    Rectangle srcRec = { font.recs[index].x - (float)font.glyphPadding, font.recs[index].y - (float)font.glyphPadding,
                         font.recs[index].width + 2.0f * font.glyphPadding, font.recs[index].height + 2.0f * font.glyphPadding };

    float width = (float)(font.recs[index].width + 2.0f * font.glyphPadding) / (float)font.baseSize * scale;
    float height = (float)(font.recs[index].height + 2.0f * font.glyphPadding) / (float)font.baseSize * scale;

    if (font.texture.id > 0) {
        const float x = 0.0f;
        const float y = 0.0f;
        const float z = 1.0f;

        // normalized texture coordinates of the glyph inside the font texture (0.0f -> 1.0f)
        const float tx = srcRec.x / font.texture.width;
        const float ty = srcRec.y / font.texture.height;
        const float tw = (srcRec.x + srcRec.width) / font.texture.width;
        const float th = (srcRec.y + srcRec.height) / font.texture.height;

        // if (SHOW_LETTER_BOUNDRY) DrawCubeWiresV((Vector3){ position.x + width / 2, position.y, position.z + height / 2 }, (Vector3){ width, LETTER_BOUNDRY_SIZE, height }, LETTER_BOUNDRY_COLOR);

        // rlCheckRenderBatchLimit(4 + 4 * backface);
        // rlSetTexture(font.texture.id);

        // rlPushMatrix();
        // rlTranslatef(position.x, position.y, position.z);

        // rlBegin(RL_QUADS);
        // rlColor4ub(tint.r, tint.g, tint.b, tint.a);

        // Front Face
        // rlNormal3f(0.0f, 1.0f, 0.0f);  // Normal Pointing Up
        // rlTexCoord2f(tx, ty);
        textCoordsData.emplace_back(tx);
        textCoordsData.emplace_back(ty);
        // rlVertex3f(x, y, z);  // Top Left Of The Texture and Quad
        vertexData.emplace_back(position.x);
        vertexData.emplace_back(position.y);
        vertexData.emplace_back(position.z);
        // rlTexCoord2f(tx, th);
        textCoordsData.emplace_back(tx);
        textCoordsData.emplace_back(th);
        // rlVertex3f(x, y, z + height);  // Bottom Left Of The Texture and Quad
        vertexData.emplace_back(position.x);
        vertexData.emplace_back(position.y);
        vertexData.emplace_back(position.z + height);
        // rlTexCoord2f(tw, th);
        textCoordsData.emplace_back(tw);
        textCoordsData.emplace_back(th);
        // rlVertex3f(x + width, y, z + height);  // Bottom Right Of The Texture and Quad
        vertexData.emplace_back(position.x + width);
        vertexData.emplace_back(position.y);
        vertexData.emplace_back(position.z + height);
        // rlTexCoord2f(tw, ty);
        textCoordsData.emplace_back(tw);
        textCoordsData.emplace_back(ty);
        // rlVertex3f(x + width, y, z);  // Top Right Of The Texture and Quad
        vertexData.emplace_back(position.x + width);
        vertexData.emplace_back(position.y);
        vertexData.emplace_back(position.z);

        if (backface) {
            // Back Face
            // rlNormal3f(0.0f, -1.0f, 0.0f);  // Normal Pointing Down
            // rlTexCoord2f(tx, ty);
            textCoordsData.emplace_back(tx);
            textCoordsData.emplace_back(ty);
            // rlVertex3f(x, y, z);  // Top Right Of The Texture and Quad
            vertexData.emplace_back(x);
            vertexData.emplace_back(y);
            vertexData.emplace_back(z);
            // rlTexCoord2f(tw, ty);
            textCoordsData.emplace_back(tw);
            textCoordsData.emplace_back(ty);
            // rlVertex3f(x + width, y, z);  // Top Left Of The Texture and Quad
            vertexData.emplace_back(x + width);
            vertexData.emplace_back(y);
            vertexData.emplace_back(z);
            // rlTexCoord2f(tw, th);
            textCoordsData.emplace_back(tw);
            textCoordsData.emplace_back(th);
            // rlVertex3f(x + width, y, z + height);  // Bottom Left Of The Texture and Quad
            vertexData.emplace_back(x + width);
            vertexData.emplace_back(y + height);
            vertexData.emplace_back(z);
            // rlTexCoord2f(tx, th);
            textCoordsData.emplace_back(tx);
            textCoordsData.emplace_back(th);
            // rlVertex3f(x, y, z + height);  // Bottom Right Of The Texture and Quad
            vertexData.emplace_back(x);
            vertexData.emplace_back(y + height);
            vertexData.emplace_back(z);
        }
        // rlEnd();
        // rlPopMatrix();

        // rlSetTexture(0);
    }
}

// Draw a 2D text in 3D space
void graphics::LabelShader::DrawText3D(graphics::Vector3 position, bool backface) {
    int length = TextLength(labelText);  // Total length in bytes of the text, scanned by codepoints in loop

    float textOffsetY = 0.0f;  // Offset between lines (on line break '\n')
    float textOffsetX = 0.0f;  // Offset X to next character to draw

    float scale = fontSize / (float)font.baseSize;

    for (int i = 0; i < length;) {
        // Get next codepoint from byte string and glyph index in font
        int codepointByteCount = 0;
        int codepoint = GetCodepointNext(&labelText[i], &codepointByteCount);
        int index = GetGlyphIndex(font, codepoint);

        // NOTE: Normally we exit the decoding sequence as soon as a bad byte is found (and return 0x3f)
        // but we need to draw all of the bad bytes using the '?' symbol moving one byte
        if (codepoint == 0x3f) codepointByteCount = 1;

        if (codepoint == '\n') {
            // NOTE: Fixed line spacing of 1.5 line-height
            // TODO: Support custom line spacing defined by user
            textOffsetY += scale + spacing / (float)font.baseSize * scale;
            textOffsetX = 0.0f;
        } else {
            if ((codepoint != ' ') && (codepoint != '\t')) {
                DrawTextCodepoint3D(codepoint, { position.x + textOffsetX, position.y, position.z + textOffsetY }, backface);
            }

            if (font.glyphs[index].advanceX == 0)
                textOffsetX += (float)(font.recs[index].width + spacing) / (float)font.baseSize * scale;
            else
                textOffsetX += (float)(font.glyphs[index].advanceX + spacing) / (float)font.baseSize * scale;
        }

        i += codepointByteCount;  // Move text bytes counter to next codepoint
    }
}

// Draw text using Font
// NOTE: chars spacing is NOT proportional to fontSize
void graphics::LabelShader::DrawTextEx(graphics::Vector3 position) {
    int size = TextLength(labelText);  // Total size in bytes of the text, scanned by codepoints in loop

    int textOffsetY = 0;       // Offset between lines (on linebreak '\n')
    float textOffsetX = 0.0f;  // Offset X to next character to draw

    float scaleFactor = fontSize / font.baseSize;  // Character quad scaling factor

    for (int i = 0; i < size;) {
        // Get next codepoint from byte string and glyph index in font
        int codepointByteCount = 0;
        int codepoint = GetCodepointNext(&labelText[i], &codepointByteCount);
        int index = GetGlyphIndex(font, codepoint);

        if (codepoint == '\n') {
            // NOTE: Line spacing is a global variable, use SetTextLineSpacing() to setup
            textOffsetY += textLineSpacing;
            textOffsetX = 0.0f;
        } else {
            if ((codepoint != ' ') && (codepoint != '\t')) {
                std::cout << "textOffsetX: " << textOffsetX << std::endl;
                std::cout << "textOffsetY: " << textOffsetY << std::endl;
                DrawTextCodepoint(codepoint, { position.x + textOffsetX, position.y + textOffsetY });
            }

            if (font.glyphs[index].advanceX == 0)
                textOffsetX += ((float)font.recs[index].width * scaleFactor + spacing);
            else
                textOffsetX += ((float)font.glyphs[index].advanceX * scaleFactor + spacing);
        }

        i += codepointByteCount;  // Move text bytes counter to next codepoint
    }
}

// Draw a texture
void graphics::LabelShader::DrawTexture(int posX, int posY, float rotation, float scale) {
    Rectangle source = { 0.0f, 0.0f, (float)texture.width, (float)texture.height };
    Rectangle dest = { position.x, position.y, (float)texture.width * scale, (float)texture.height * scale };
    graphics::Vector2 origin = { 0.0f, 0.0f };

    DrawTexturePro(source, dest, origin, rotation);
}

void graphics::LabelShader::buildIndexBufferData() {
    // from Raylib
    unsigned int k = 0;

    // Indices can be initialized right now
    for (int j = 0; j < vertexData.size(); j += 6) {
        indexData.emplace_back(4 * k);
        indexData.emplace_back(4 * k + 1);
        indexData.emplace_back(4 * k + 2);
        indexData.emplace_back(4 * k);
        indexData.emplace_back(4 * k + 2);
        indexData.emplace_back(4 * k + 3);
        k++;
    }
}

void graphics::LabelShader::buildVertices(graphics::Vector3 fontPosition) {
    // DrawTextEx(fontPosition);
    DrawText3D(fontPosition, false);
    //  DrawTexture(10, 10, 0, 1.0f);
    buildIndexBufferData();
}

// vertexSize is size of point(Vector2, Vector3, etc...) = 2, 3, etc...
void graphics::LabelShader::createTextBuffer(GLenum usage) {
    std::string position = "position";
    std::string vertexTexCoord = "vertexTexCoord";
    int positionLocation = cachedAttributes[position];
    int vertexTexCoordLocation = cachedAttributes[vertexTexCoord];

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &pointsVBO);
    glGenBuffers(1, &textCoordsVBO);

    // Upload vertex data
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, pointsVBO);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), usage);

    glEnableVertexAttribArray(positionLocation);
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, 0, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, textCoordsVBO);
    glBufferData(GL_ARRAY_BUFFER, textCoordsData.size() * sizeof(float), textCoordsData.data(), usage);

    glEnableVertexAttribArray(vertexTexCoordLocation);
    glVertexAttribPointer(vertexTexCoordLocation, 2, GL_FLOAT, 0, 0, 0);

    // build index buffer
    glGenBuffers(1, &indexVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexVBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizei)(indexData.size() * sizeof(unsigned int)), indexData.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
}

//----------------------------------------------------------------------------------
// Rendering Cycle/Drawing functions
//----------------------------------------------------------------------------------

// Use the shader program
void graphics::LabelShader::Use() const {
    glUseProgram(program);
}

void graphics::LabelShader::render() {
    glUseProgram(program);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_SCISSOR_TEST);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glStencilMask(0xffffffff);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glStencilFunc(GL_ALWAYS, 0, 0xffffffff);

    glBindVertexArray(VAO);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture.id);

    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexVBO);
    //  glDrawArrays(GL_TRIANGLES, 0, (GLsizei)(vertexData.size() / 3));

    // glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indexData.size()), GL_UNSIGNED_INT, (GLvoid*)(0 * sizeof(unsigned int)));
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indexData.size()), GL_UNSIGNED_INT, 0);
    print_opengl_error();

    glBindVertexArray(0);
    glUseProgram(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void graphics::LabelShader::renderIndexAttribute(int start, int count) {
    // auto buffer = buffers_.at(indexAttribute);
    // glDrawElements(mode_, count, buffer.type, (GLvoid*)(start * buffer.bytesPerElement));
}

void graphics::LabelShader::destroy() {
    glDeleteProgram(program);
    this->program = -1;
}
