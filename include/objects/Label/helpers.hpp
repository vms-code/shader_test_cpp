#ifndef GRAPHICS_LABELSHADER_HELPERS_HPP
#define GRAPHICS_LABELSHADER_HELPERS_HPP
/******************* GOT IT FROM RAYLIB **********************/
#include "setup_window.hpp"

// static int textLineSpacing = 15;  // Text vertical line spacing in pixels
void TraceLog(int logType, const char *text, ...);
#define MAX_TRACELOG_MSG_LENGTH 256
#define TRACELOG(level, ...) TraceLog(level, __VA_ARGS__)

static int logTypeLevel = LOG_INFO;  // Minimum log type level
// Trace log level
// NOTE: Organized by priority level
typedef enum {
    RL_LOG_ALL = 0,  // Display all logs
    RL_LOG_TRACE,    // Trace logging, intended for internal use only
    RL_LOG_DEBUG,    // Debug logging, used for internal debugging, it should be disabled on release builds
    RL_LOG_INFO,     // Info logging, used for program execution info
    RL_LOG_WARNING,  // Warning logging, used on recoverable failures
    RL_LOG_ERROR,    // Error logging, used on unrecoverable failures
    RL_LOG_FATAL,    // Fatal logging, used to abort program: exit(EXIT_FAILURE)
    RL_LOG_NONE      // Disable logging
} rlTraceLogLevel;
// Pixel formats
// NOTE: Support depends on OpenGL version and platform
typedef enum {
    PIXELFORMAT_UNCOMPRESSED_GRAYSCALE = 1,  // 8 bit per pixel (no alpha)
    PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA,     // 8*2 bpp (2 channels)
    PIXELFORMAT_UNCOMPRESSED_R5G6B5,         // 16 bpp
    PIXELFORMAT_UNCOMPRESSED_R8G8B8,         // 24 bpp
    PIXELFORMAT_UNCOMPRESSED_R5G5B5A1,       // 16 bpp (1 bit alpha)
    PIXELFORMAT_UNCOMPRESSED_R4G4B4A4,       // 16 bpp (4 bit alpha)
    PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,       // 32 bpp
    PIXELFORMAT_UNCOMPRESSED_R32,            // 32 bpp (1 channel - float)
    PIXELFORMAT_UNCOMPRESSED_R32G32B32,      // 32*3 bpp (3 channels - float)
    PIXELFORMAT_UNCOMPRESSED_R32G32B32A32,   // 32*4 bpp (4 channels - float)
    PIXELFORMAT_UNCOMPRESSED_R16,            // 16 bpp (1 channel - half float)
    PIXELFORMAT_UNCOMPRESSED_R16G16B16,      // 16*3 bpp (3 channels - half float)
    PIXELFORMAT_UNCOMPRESSED_R16G16B16A16,   // 16*4 bpp (4 channels - half float)
    PIXELFORMAT_COMPRESSED_DXT1_RGB,         // 4 bpp (no alpha)
    PIXELFORMAT_COMPRESSED_DXT1_RGBA,        // 4 bpp (1 bit alpha)
    PIXELFORMAT_COMPRESSED_DXT3_RGBA,        // 8 bpp
    PIXELFORMAT_COMPRESSED_DXT5_RGBA,        // 8 bpp
    PIXELFORMAT_COMPRESSED_ETC1_RGB,         // 4 bpp
    PIXELFORMAT_COMPRESSED_ETC2_RGB,         // 4 bpp
    PIXELFORMAT_COMPRESSED_ETC2_EAC_RGBA,    // 8 bpp
    PIXELFORMAT_COMPRESSED_PVRT_RGB,         // 4 bpp
    PIXELFORMAT_COMPRESSED_PVRT_RGBA,        // 4 bpp
    PIXELFORMAT_COMPRESSED_ASTC_4x4_RGBA,    // 8 bpp
    PIXELFORMAT_COMPRESSED_ASTC_8x8_RGBA     // 2 bpp
} PixelFormat;
// Texture pixel formats
// NOTE: Support depends on OpenGL version
typedef enum {
    RL_PIXELFORMAT_UNCOMPRESSED_GRAYSCALE = 1,  // 8 bit per pixel (no alpha)
    RL_PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA,     // 8*2 bpp (2 channels)
    RL_PIXELFORMAT_UNCOMPRESSED_R5G6B5,         // 16 bpp
    RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8,         // 24 bpp
    RL_PIXELFORMAT_UNCOMPRESSED_R5G5B5A1,       // 16 bpp (1 bit alpha)
    RL_PIXELFORMAT_UNCOMPRESSED_R4G4B4A4,       // 16 bpp (4 bit alpha)
    RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,       // 32 bpp
    RL_PIXELFORMAT_UNCOMPRESSED_R32,            // 32 bpp (1 channel - float)
    RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32,      // 32*3 bpp (3 channels - float)
    RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32,   // 32*4 bpp (4 channels - float)
    RL_PIXELFORMAT_UNCOMPRESSED_R16,            // 16 bpp (1 channel - half float)
    RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16,      // 16*3 bpp (3 channels - half float)
    RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16A16,   // 16*4 bpp (4 channels - half float)
    RL_PIXELFORMAT_COMPRESSED_DXT1_RGB,         // 4 bpp (no alpha)
    RL_PIXELFORMAT_COMPRESSED_DXT1_RGBA,        // 4 bpp (1 bit alpha)
    RL_PIXELFORMAT_COMPRESSED_DXT3_RGBA,        // 8 bpp
    RL_PIXELFORMAT_COMPRESSED_DXT5_RGBA,        // 8 bpp
    RL_PIXELFORMAT_COMPRESSED_ETC1_RGB,         // 4 bpp
    RL_PIXELFORMAT_COMPRESSED_ETC2_RGB,         // 4 bpp
    RL_PIXELFORMAT_COMPRESSED_ETC2_EAC_RGBA,    // 8 bpp
    RL_PIXELFORMAT_COMPRESSED_PVRT_RGB,         // 4 bpp
    RL_PIXELFORMAT_COMPRESSED_PVRT_RGBA,        // 4 bpp
    RL_PIXELFORMAT_COMPRESSED_ASTC_4x4_RGBA,    // 8 bpp
    RL_PIXELFORMAT_COMPRESSED_ASTC_8x8_RGBA     // 2 bpp
} rlPixelFormat;
// Font type, defines generation method
typedef enum {
    FONT_DEFAULT = 0,  // Default font generation, anti-aliased
    FONT_BITMAP,       // Bitmap font generation, no anti-aliasing
    FONT_SDF           // SDF font generation, requires external shader
} FontType;
// Texture parameters: filter mode
// NOTE 1: Filtering considers mipmaps if available in the texture
// NOTE 2: Filter is accordingly set for minification and magnification
typedef enum {
    TEXTURE_FILTER_POINT = 0,        // No filter, just pixel approximation
    TEXTURE_FILTER_BILINEAR,         // Linear filtering
    TEXTURE_FILTER_TRILINEAR,        // Trilinear filtering (linear with mipmaps)
    TEXTURE_FILTER_ANISOTROPIC_4X,   // Anisotropic filtering 4x
    TEXTURE_FILTER_ANISOTROPIC_8X,   // Anisotropic filtering 8x
    TEXTURE_FILTER_ANISOTROPIC_16X,  // Anisotropic filtering 16x
} TextureFilter;
// Image, pixel data stored in CPU memory (RAM)
typedef struct Image {
    void *data;   // Image raw data
    int width;    // Image base width
    int height;   // Image base height
    int mipmaps;  // Mipmap levels, 1 by default
    int format;   // Data format (PixelFormat type)
} Image;

// Rectangle, 4 components
typedef struct Rectangle {
    float x;       // Rectangle top-left corner position x
    float y;       // Rectangle top-left corner position y
    float width;   // Rectangle width
    float height;  // Rectangle height
} Rectangle;

// Texture, tex data stored in GPU memory (VRAM)
typedef struct Texture {
    unsigned int id;  // OpenGL texture id
    int width;        // Texture base width
    int height;       // Texture base height
    int mipmaps;      // Mipmap levels, 1 by default
    int format;       // Data format (PixelFormat type)
} Texture;

// GlyphInfo, font characters glyphs info
typedef struct GlyphInfo {
    int value;     // Character value (Unicode)
    int offsetX;   // Character offset X when drawing
    int offsetY;   // Character offset Y when drawing
    int advanceX;  // Character advance position X
    Image image;   // Character image data
} GlyphInfo;

// Font, font texture and GlyphInfo array data
typedef struct Font {
    int baseSize;       // Base size (default chars height)
    int glyphCount;     // Number of glyph characters
    int glyphPadding;   // Padding around the glyph characters
    Texture texture;    // Texture atlas containing the glyphs
    Rectangle *recs;    // Rectangles in texture for the glyphs
    GlyphInfo *glyphs;  // Glyphs info data
} Font;

/******** FUNCTIONS *********/

unsigned char *LoadFileData(const char *fileName, int *dataSize);
// Load font data for further use
// NOTE: Requires TTF font memory data and can generate SDF data
GlyphInfo *LoadFontData(const unsigned char *fileData, int dataSize, int fontSize, int *codepoints, int codepointCount, int type);
Image GenImageFontAtlas(const GlyphInfo *glyphs, Rectangle **glyphRecs, int glyphCount, int fontSize, int padding, int packMethod);
int rlGetPixelDataSize(int width, int height, int format);
void rlGetGlTextureFormats(int format, unsigned int *glInternalFormat, unsigned int *glFormat, unsigned int *glType);
const char *rlGetPixelFormatName(unsigned int format);
// Set texture parameters (wrap mode/filter mode)
void rlTextureParameters(unsigned int id, int param, int value);
// Set texture scaling filter mode
void SetTextureFilter(Texture texture, int filter);

int GetCodepointNext(const char *text, int *codepointSize);
int GetGlyphIndex(Font font, int codepoint);
unsigned int TextLength(const char *text);
void SetTextLineSpacing(int spacing);

#endif
