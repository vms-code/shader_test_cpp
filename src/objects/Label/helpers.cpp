/******************* RAYLIB FUNCTIONS **********************/
#include <glad/gl.h>
//
#include <stdarg.h>  // Required for: va_list, va_start(), va_end()
#include <stdio.h>   // Required for: FILE, fopen(), fseek(), ftell(), fread(), fwrite(), fprintf(), vprintf(), fclose()
#include <stdlib.h>  // Required for: exit()
#include <string.h>  // Required for: strcpy(), strcat()

#include "Label/helpers.hpp"
#define STB_RECT_PACK_IMPLEMENTATION
#include "stb_rect_pack.h"
//
#include <math.h>
#define STBTT_STATIC
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"  // Required for: ttf font data reading
//
#include "math/Vector2.hpp"

#define MAX_TEXT_BUFFER_LENGTH 1024  // Size of internal static buffers used on some functions:

// Texture parameters (equivalent to OpenGL defines)
#define RL_TEXTURE_WRAP_S 0x2802             // GL_TEXTURE_WRAP_S
#define RL_TEXTURE_WRAP_T 0x2803             // GL_TEXTURE_WRAP_T
#define RL_TEXTURE_MAG_FILTER 0x2800         // GL_TEXTURE_MAG_FILTER
#define RL_TEXTURE_MIN_FILTER 0x2801         // GL_TEXTURE_MIN_FILTER
#define RL_TEXTURE_MIPMAP_BIAS_RATIO 0x4000  // Texture mipmap bias, percentage ratio (custom identifier)

#define RL_TEXTURE_FILTER_NEAREST 0x2600             // GL_NEAREST
#define RL_TEXTURE_FILTER_LINEAR 0x2601              // GL_LINEAR
#define RL_TEXTURE_FILTER_MIP_NEAREST 0x2700         // GL_NEAREST_MIPMAP_NEAREST
#define RL_TEXTURE_FILTER_NEAREST_MIP_LINEAR 0x2702  // GL_NEAREST_MIPMAP_LINEAR
#define RL_TEXTURE_FILTER_LINEAR_MIP_NEAREST 0x2701  // GL_LINEAR_MIPMAP_NEAREST
#define RL_TEXTURE_FILTER_MIP_LINEAR 0x2703          // GL_LINEAR_MIPMAP_LINEAR
#define RL_TEXTURE_FILTER_ANISOTROPIC 0x3000         // Anisotropic filter (custom identifier)
#define RL_TEXTURE_MIPMAP_BIAS_RATIO 0x4000          // Texture mipmap bias, percentage ratio (custom identifier)
// Texture parameters (equivalent to OpenGL defines)
#define RL_TEXTURE_WRAP_S 0x2802      // GL_TEXTURE_WRAP_S
#define RL_TEXTURE_WRAP_T 0x2803      // GL_TEXTURE_WRAP_T
#define RL_TEXTURE_MAG_FILTER 0x2800  // GL_TEXTURE_MAG_FILTER
#define RL_TEXTURE_MIN_FILTER 0x2801  // GL_TEXTURE_MIN_FILTER

// Show trace log messages (LOG_INFO, LOG_WARNING, LOG_ERROR, LOG_DEBUG)
void TraceLog(int logType, const char *text, ...) {
    // Message has level below current threshold, don't emit
    if (logType < logTypeLevel) return;

    char *args;
    va_start(args, text);

#if defined(PLATFORM_ANDROID)
    switch (logType) {
        case LOG_TRACE:
            __android_log_vprint(ANDROID_LOG_VERBOSE, "raylib", text, args);
            break;
        case LOG_DEBUG:
            __android_log_vprint(ANDROID_LOG_DEBUG, "raylib", text, args);
            break;
        case LOG_INFO:
            __android_log_vprint(ANDROID_LOG_INFO, "raylib", text, args);
            break;
        case LOG_WARNING:
            __android_log_vprint(ANDROID_LOG_WARN, "raylib", text, args);
            break;
        case LOG_ERROR:
            __android_log_vprint(ANDROID_LOG_ERROR, "raylib", text, args);
            break;
        case LOG_FATAL:
            __android_log_vprint(ANDROID_LOG_FATAL, "raylib", text, args);
            break;
        default:
            break;
    }
#else
    char buffer[MAX_TRACELOG_MSG_LENGTH] = { 0 };

    switch (logType) {
        case LOG_TRACE:
            strcpy_s(buffer, sizeof(buffer), "TRACE: ");
            break;
        case LOG_DEBUG:
            strcpy_s(buffer, sizeof(buffer), "DEBUG: ");
            break;
        case LOG_INFO:
            strcpy_s(buffer, sizeof(buffer), "INFO: ");
            break;
        case LOG_WARNING:
            strcpy_s(buffer, sizeof(buffer), "WARNING: ");
            break;
        case LOG_ERROR:
            strcpy_s(buffer, sizeof(buffer), "ERROR: ");
            break;
        case LOG_FATAL:
            strcpy_s(buffer, sizeof(buffer), "FATAL: ");
            break;
        default:
            break;
    }

    memset(buffer, 0, MAX_TRACELOG_MSG_LENGTH);
    unsigned int textSize = (unsigned int)strlen(text);
    memcpy(buffer + strlen(buffer), text, (textSize < (MAX_TRACELOG_MSG_LENGTH - 12)) ? textSize : (MAX_TRACELOG_MSG_LENGTH - 12));
    strcat_s(buffer, sizeof(buffer), "\n");
    vprintf(buffer, args);
    fflush(stdout);
#endif

    va_end(args);

    if (logType == LOG_FATAL) exit(EXIT_FAILURE);  // If fatal logging, exit program
}

// Load data from file into a buffer
unsigned char *LoadFileData(const char *fileName, int *dataSize) {
    unsigned char *data = NULL;
    *dataSize = 0;

    if (fileName != NULL) {
        FILE *file = fopen(fileName, "rb");

        if (file != NULL) {
            // WARNING: On binary streams SEEK_END could not be found,
            // using fseek() and ftell() could not work in some (rare) cases
            fseek(file, 0, SEEK_END);
            int size = ftell(file);  // WARNING: ftell() returns 'long int', maximum size returned is INT_MAX (2147483647 bytes)
            fseek(file, 0, SEEK_SET);

            if (size > 0) {
                data = (unsigned char *)malloc(size * sizeof(unsigned char));

                if (data != NULL) {
                    // NOTE: fread() returns number of read elements instead of bytes, so we read [1 byte, size elements]
                    size_t count = fread(data, sizeof(unsigned char), size, file);

                    // WARNING: fread() returns a size_t value, usually 'unsigned int' (32bit compilation) and 'unsigned long long' (64bit compilation)
                    // dataSize is unified along raylib as a 'int' type, so, for file-sizes > INT_MAX (2147483647 bytes) we have a limitation
                    if (count > 2147483647) {
                        TRACELOG(LOG_WARNING, "FILEIO: [%s] File is bigger than 2147483647 bytes, avoid using LoadFileData()", fileName);

                        free(data);
                        data = NULL;
                    } else {
                        *dataSize = (int)count;

                        if ((*dataSize) != size)
                            TRACELOG(LOG_WARNING, "FILEIO: [%s] File partially loaded (%i bytes out of %i)", fileName, dataSize, count);
                        else
                            TRACELOG(LOG_INFO, "FILEIO: [%s] File loaded successfully", fileName);
                    }
                } else
                    TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to allocated memory for file reading", fileName);
            } else
                TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to read file", fileName);

            fclose(file);
        } else
            TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to open file", fileName);
    } else
        TRACELOG(LOG_WARNING, "FILEIO: File name provided is not valid");

    return data;
}

// Load font data for further use
// NOTE: Requires TTF font memory data and can generate SDF data
GlyphInfo *LoadFontData(const unsigned char *fileData, int dataSize, int fontSize, int *codepoints, int codepointCount, int type) {
    // NOTE: Using some SDF generation default values,
    // trades off precision with ability to handle *smaller* sizes
#ifndef FONT_SDF_CHAR_PADDING
#define FONT_SDF_CHAR_PADDING 4  // SDF font generation char padding
#endif
#ifndef FONT_SDF_ON_EDGE_VALUE
#define FONT_SDF_ON_EDGE_VALUE 128  // SDF font generation on edge value
#endif
#ifndef FONT_SDF_PIXEL_DIST_SCALE
#define FONT_SDF_PIXEL_DIST_SCALE 64.0f  // SDF font generation pixel distance scale
#endif
#ifndef FONT_BITMAP_ALPHA_THRESHOLD
#define FONT_BITMAP_ALPHA_THRESHOLD 80  // Bitmap (B&W) font generation alpha threshold
#endif

    GlyphInfo *chars = NULL;

    // Load font data (including pixel data) from TTF memory file
    // NOTE: Loaded information should be enough to generate font image atlas, using any packaging method
    if (fileData != NULL) {
        bool genFontChars = false;
        stbtt_fontinfo fontInfo = { 0 };

        if (stbtt_InitFont(&fontInfo, (unsigned char *)fileData, 0))  // Initialize font for data reading
        {
            // Calculate font scale factor
            float scaleFactor = stbtt_ScaleForPixelHeight(&fontInfo, (float)fontSize);

            // Calculate font basic metrics
            // NOTE: ascent is equivalent to font baseline
            int ascent, descent, lineGap;
            stbtt_GetFontVMetrics(&fontInfo, &ascent, &descent, &lineGap);

            // In case no chars count provided, default to 95
            codepointCount = (codepointCount > 0) ? codepointCount : 95;

            // Fill fontChars in case not provided externally
            // NOTE: By default we fill glyphCount consecutively, starting at 32 (Space)
            if (codepoints == NULL) {
                codepoints = (int *)malloc(codepointCount * sizeof(int));
                for (int i = 0; i < codepointCount; i++) codepoints[i] = i + 32;
                genFontChars = true;
            }

            chars = (GlyphInfo *)calloc(codepointCount, sizeof(GlyphInfo));

            // NOTE: Using simple packaging, one char after another
            for (int i = 0; i < codepointCount; i++) {
                int chw = 0, chh = 0;    // Character width and height (on generation)
                int ch = codepoints[i];  // Character value to get info for
                chars[i].value = ch;

                //  Render a unicode codepoint to a bitmap
                //      stbtt_GetCodepointBitmap()           -- allocates and returns a bitmap
                //      stbtt_GetCodepointBitmapBox()        -- how big the bitmap must be
                //      stbtt_MakeCodepointBitmap()          -- renders into bitmap you provide

                // Check if a glyph is available in the font
                // WARNING: if (index == 0), glyph not found, it could fallback to default .notdef glyph (if defined in font)
                int index = stbtt_FindGlyphIndex(&fontInfo, ch);

                if (index > 0) {
                    switch (type) {
                        case FONT_DEFAULT:
                        case FONT_BITMAP:
                            chars[i].image.data = stbtt_GetCodepointBitmap(&fontInfo, scaleFactor, scaleFactor, ch, &chw, &chh, &chars[i].offsetX, &chars[i].offsetY);
                            break;
                        case FONT_SDF:
                            if (ch != 32) chars[i].image.data = stbtt_GetCodepointSDF(&fontInfo, scaleFactor, ch, FONT_SDF_CHAR_PADDING, FONT_SDF_ON_EDGE_VALUE, FONT_SDF_PIXEL_DIST_SCALE, &chw, &chh, &chars[i].offsetX, &chars[i].offsetY);
                            break;
                        default:
                            break;
                    }

                    if (chars[i].image.data != NULL)  // Glyph data has been found in the font
                    {
                        stbtt_GetCodepointHMetrics(&fontInfo, ch, &chars[i].advanceX, NULL);
                        chars[i].advanceX = (int)((float)chars[i].advanceX * scaleFactor);

                        // Load characters images
                        chars[i].image.width = chw;
                        chars[i].image.height = chh;
                        chars[i].image.mipmaps = 1;
                        chars[i].image.format = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE;

                        chars[i].offsetY += (int)((float)ascent * scaleFactor);
                    }

                    // NOTE: We create an empty image for space character,
                    // it could be further required for atlas packing
                    if (ch == 32) {
                        stbtt_GetCodepointHMetrics(&fontInfo, ch, &chars[i].advanceX, NULL);
                        chars[i].advanceX = (int)((float)chars[i].advanceX * scaleFactor);

                        Image imSpace = {
                            .data = calloc(chars[i].advanceX * fontSize, 2),
                            .width = chars[i].advanceX,
                            .height = fontSize,
                            .mipmaps = 1,
                            .format = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE
                        };

                        chars[i].image = imSpace;
                    }

                    if (type == FONT_BITMAP) {
                        // Aliased bitmap (black & white) font generation, avoiding anti-aliasing
                        // NOTE: For optimum results, bitmap font should be generated at base pixel size
                        for (int p = 0; p < chw * chh; p++) {
                            if (((unsigned char *)chars[i].image.data)[p] < FONT_BITMAP_ALPHA_THRESHOLD)
                                ((unsigned char *)chars[i].image.data)[p] = 0;
                            else
                                ((unsigned char *)chars[i].image.data)[p] = 255;
                        }
                    }
                } else {
                    // TODO: Use some fallback glyph for codepoints not found in the font
                }
            }
        } else
            TRACELOG(LOG_WARNING, "FONT: Failed to process TTF font data");

        if (genFontChars) free(codepoints);
    }

    return chars;
}

// Generate image font atlas using chars info
// NOTE: Packing method: 0-Default, 1-Skyline
Image GenImageFontAtlas(const GlyphInfo *glyphs, Rectangle **glyphRecs, int glyphCount, int fontSize, int padding, int packMethod) {
    Image atlas = { 0 };

    if (glyphs == NULL) {
        TRACELOG(LOG_WARNING, "FONT: Provided chars info not valid, returning empty image atlas");
        return atlas;
    }

    *glyphRecs = NULL;

    // In case no chars count provided we suppose default of 95
    glyphCount = (glyphCount > 0) ? glyphCount : 95;

    // NOTE: Rectangles memory is loaded here!
    Rectangle *recs = (Rectangle *)malloc(glyphCount * sizeof(Rectangle));

    // Calculate image size based on total glyph width and glyph row count
    int totalWidth = 0;
    int maxGlyphWidth = 0;

    for (int i = 0; i < glyphCount; i++) {
        if (glyphs[i].image.width > maxGlyphWidth) maxGlyphWidth = glyphs[i].image.width;
        totalWidth += glyphs[i].image.width + 2 * padding;
    }

    int paddedFontSize = fontSize + 2 * padding;
    // No need for a so-conservative atlas generation
    float totalArea = totalWidth * paddedFontSize * 1.2f;
    float imageMinSize = sqrtf(totalArea);
    int imageSize = (int)powf(2, ceilf(logf(imageMinSize) / logf(2)));

    if (totalArea < ((imageSize * imageSize) / 2)) {
        atlas.width = imageSize;       // Atlas bitmap width
        atlas.height = imageSize / 2;  // Atlas bitmap height
    } else {
        atlas.width = imageSize;   // Atlas bitmap width
        atlas.height = imageSize;  // Atlas bitmap height
    }

    atlas.data = (unsigned char *)calloc(1, atlas.width * atlas.height);  // Create a bitmap to store characters (8 bpp)
    atlas.format = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE;
    atlas.mipmaps = 1;

    // DEBUG: We can see padding in the generated image setting a gray background...
    // for (int i = 0; i < atlas.width*atlas.height; i++) ((unsigned char *)atlas.data)[i] = 100;

    if (packMethod == 0)  // Use basic packing algorithm
    {
        int offsetX = padding;
        int offsetY = padding;

        // NOTE: Using simple packaging, one char after another
        for (int i = 0; i < glyphCount; i++) {
            // Check remaining space for glyph
            if (offsetX >= (atlas.width - glyphs[i].image.width - 2 * padding)) {
                offsetX = padding;

                // NOTE: Be careful on offsetY for SDF fonts, by default SDF
                // use an internal padding of 4 pixels, it means char rectangle
                // height is bigger than fontSize, it could be up to (fontSize + 8)
                offsetY += (fontSize + 2 * padding);

                if (offsetY > (atlas.height - fontSize - padding)) {
                    for (int j = i + 1; j < glyphCount; j++) {
                        TRACELOG(LOG_WARNING, "FONT: Failed to package character (%i)", j);
                        // Make sure remaining recs contain valid data
                        recs[j].x = 0;
                        recs[j].y = 0;
                        recs[j].width = 0;
                        recs[j].height = 0;
                    }
                    break;
                }
            }

            // Copy pixel data from glyph image to atlas
            for (int y = 0; y < glyphs[i].image.height; y++) {
                for (int x = 0; x < glyphs[i].image.width; x++) {
                    ((unsigned char *)atlas.data)[(offsetY + y) * atlas.width + (offsetX + x)] = ((unsigned char *)glyphs[i].image.data)[y * glyphs[i].image.width + x];
                }
            }

            // Fill chars rectangles in atlas info
            recs[i].x = (float)offsetX;
            recs[i].y = (float)offsetY;
            recs[i].width = (float)glyphs[i].image.width;
            recs[i].height = (float)glyphs[i].image.height;

            // Move atlas position X for next character drawing
            offsetX += (glyphs[i].image.width + 2 * padding);
        }
    } else if (packMethod == 1)  // Use Skyline rect packing algorithm (stb_pack_rect)
    {
        stbrp_context *context = (stbrp_context *)malloc(sizeof(*context));
        stbrp_node *nodes = (stbrp_node *)malloc(glyphCount * sizeof(*nodes));

        stbrp_init_target(context, atlas.width, atlas.height, nodes, glyphCount);
        stbrp_rect *rects = (stbrp_rect *)malloc(glyphCount * sizeof(stbrp_rect));

        // Fill rectangles for packaging
        for (int i = 0; i < glyphCount; i++) {
            rects[i].id = i;
            rects[i].w = glyphs[i].image.width + 2 * padding;
            rects[i].h = glyphs[i].image.height + 2 * padding;
        }

        // Package rectangles into atlas
        stbrp_pack_rects(context, rects, glyphCount);

        for (int i = 0; i < glyphCount; i++) {
            // It returns char rectangles in atlas
            recs[i].x = rects[i].x + (float)padding;
            recs[i].y = rects[i].y + (float)padding;
            recs[i].width = (float)glyphs[i].image.width;
            recs[i].height = (float)glyphs[i].image.height;

            if (rects[i].was_packed) {
                // Copy pixel data from fc.data to atlas
                for (int y = 0; y < glyphs[i].image.height; y++) {
                    for (int x = 0; x < glyphs[i].image.width; x++) {
                        ((unsigned char *)atlas.data)[(rects[i].y + padding + y) * atlas.width + (rects[i].x + padding + x)] = ((unsigned char *)glyphs[i].image.data)[y * glyphs[i].image.width + x];
                    }
                }
            } else
                TRACELOG(LOG_WARNING, "FONT: Failed to package character (%i)", i);
        }

        free(rects);
        free(nodes);
        free(context);
    }

#if defined(SUPPORT_FONT_ATLAS_WHITE_REC)
    // Add a 3x3 white rectangle at the bottom-right corner of the generated atlas,
    // useful to use as the white texture to draw shapes with raylib, using this rectangle
    // shapes and text can be backed into a single draw call: SetShapesTexture()
    for (int i = 0, k = atlas.width * atlas.height - 1; i < 3; i++) {
        ((unsigned char *)atlas.data)[k - 0] = 255;
        ((unsigned char *)atlas.data)[k - 1] = 255;
        ((unsigned char *)atlas.data)[k - 2] = 255;
        k -= atlas.width;
    }
#endif

    // Convert image data from GRAYSCALE to GRAY_ALPHA
    unsigned char *dataGrayAlpha = (unsigned char *)malloc(atlas.width * atlas.height * sizeof(unsigned char) * 2);  // Two channels

    for (int i = 0, k = 0; i < atlas.width * atlas.height; i++, k += 2) {
        dataGrayAlpha[k] = 255;
        dataGrayAlpha[k + 1] = ((unsigned char *)atlas.data)[i];
    }

    free(atlas.data);
    atlas.data = dataGrayAlpha;
    atlas.format = PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA;

    *glyphRecs = recs;

    return atlas;
}

// Formatting of text with variables to 'embed'
// WARNING: String returned will expire after this function is called MAX_TEXTFORMAT_BUFFERS times
const char *TextFormat(const char *text, ...) {
#ifndef MAX_TEXTFORMAT_BUFFERS
#define MAX_TEXTFORMAT_BUFFERS 4  // Maximum number of static buffers for text formatting
#endif

    // We create an array of buffers so strings don't expire until MAX_TEXTFORMAT_BUFFERS invocations
    static char buffers[MAX_TEXTFORMAT_BUFFERS][MAX_TEXT_BUFFER_LENGTH] = { 0 };
    static int index = 0;

    char *currentBuffer = buffers[index];
    memset(currentBuffer, 0, MAX_TEXT_BUFFER_LENGTH);  // Clear buffer before using

    va_list args;
    va_start(args, text);
    int requiredByteCount = vsnprintf(currentBuffer, MAX_TEXT_BUFFER_LENGTH, text, args);
    va_end(args);

    // If requiredByteCount is larger than the MAX_TEXT_BUFFER_LENGTH, then overflow occured
    if (requiredByteCount >= MAX_TEXT_BUFFER_LENGTH) {
        // Inserting "..." at the end of the string to mark as truncated
        char *truncBuffer = buffers[index] + MAX_TEXT_BUFFER_LENGTH - 4;  // Adding 4 bytes = "...\0"
        sprintf_s(truncBuffer, sizeof(truncBuffer), "...");
    }

    index += 1;  // Move to next buffer for next function call
    if (index >= MAX_TEXTFORMAT_BUFFERS) index = 0;

    return currentBuffer;
}

// Get pixel data size in bytes (image or texture)
// NOTE: Size depends on pixel format
int rlGetPixelDataSize(int width, int height, int format) {
    int dataSize = 0;  // Size in bytes
    int bpp = 0;       // Bits per pixel

    switch (format) {
        case RL_PIXELFORMAT_UNCOMPRESSED_GRAYSCALE:
            bpp = 8;
            break;
        case RL_PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA:
        case RL_PIXELFORMAT_UNCOMPRESSED_R5G6B5:
        case RL_PIXELFORMAT_UNCOMPRESSED_R5G5B5A1:
        case RL_PIXELFORMAT_UNCOMPRESSED_R4G4B4A4:
            bpp = 16;
            break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8:
            bpp = 32;
            break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8:
            bpp = 24;
            break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R32:
            bpp = 32;
            break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32:
            bpp = 32 * 3;
            break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32:
            bpp = 32 * 4;
            break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R16:
            bpp = 16;
            break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16:
            bpp = 16 * 3;
            break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16A16:
            bpp = 16 * 4;
            break;
        case RL_PIXELFORMAT_COMPRESSED_DXT1_RGB:
        case RL_PIXELFORMAT_COMPRESSED_DXT1_RGBA:
        case RL_PIXELFORMAT_COMPRESSED_ETC1_RGB:
        case RL_PIXELFORMAT_COMPRESSED_ETC2_RGB:
        case RL_PIXELFORMAT_COMPRESSED_PVRT_RGB:
        case RL_PIXELFORMAT_COMPRESSED_PVRT_RGBA:
            bpp = 4;
            break;
        case RL_PIXELFORMAT_COMPRESSED_DXT3_RGBA:
        case RL_PIXELFORMAT_COMPRESSED_DXT5_RGBA:
        case RL_PIXELFORMAT_COMPRESSED_ETC2_EAC_RGBA:
        case RL_PIXELFORMAT_COMPRESSED_ASTC_4x4_RGBA:
            bpp = 8;
            break;
        case RL_PIXELFORMAT_COMPRESSED_ASTC_8x8_RGBA:
            bpp = 2;
            break;
        default:
            break;
    }

    dataSize = width * height * bpp / 8;  // Total data size in bytes

    // Most compressed formats works on 4x4 blocks,
    // if texture is smaller, minimum dataSize is 8 or 16
    if ((width < 4) && (height < 4)) {
        if ((format >= RL_PIXELFORMAT_COMPRESSED_DXT1_RGB) && (format < RL_PIXELFORMAT_COMPRESSED_DXT3_RGBA))
            dataSize = 8;
        else if ((format >= RL_PIXELFORMAT_COMPRESSED_DXT3_RGBA) && (format < RL_PIXELFORMAT_COMPRESSED_ASTC_8x8_RGBA))
            dataSize = 16;
    }

    return dataSize;
}

// Get OpenGL internal formats and data type from raylib PixelFormat
void rlGetGlTextureFormats(int format, unsigned int *glInternalFormat, unsigned int *glFormat, unsigned int *glType) {
    *glInternalFormat = 0;
    *glFormat = 0;
    *glType = 0;

    switch (format) {
        case RL_PIXELFORMAT_UNCOMPRESSED_GRAYSCALE:
            *glInternalFormat = GL_R8;
            *glFormat = GL_RED;
            *glType = GL_UNSIGNED_BYTE;
            break;
        case RL_PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA:
            *glInternalFormat = GL_RG8;
            *glFormat = GL_RG;
            *glType = GL_UNSIGNED_BYTE;
            break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R5G6B5:
            *glInternalFormat = GL_RGB565;
            *glFormat = GL_RGB;
            *glType = GL_UNSIGNED_SHORT_5_6_5;
            break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8:
            *glInternalFormat = GL_RGB8;
            *glFormat = GL_RGB;
            *glType = GL_UNSIGNED_BYTE;
            break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R5G5B5A1:
            *glInternalFormat = GL_RGB5_A1;
            *glFormat = GL_RGBA;
            *glType = GL_UNSIGNED_SHORT_5_5_5_1;
            break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R4G4B4A4:
            *glInternalFormat = GL_RGBA4;
            *glFormat = GL_RGBA;
            *glType = GL_UNSIGNED_SHORT_4_4_4_4;
            break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8:
            *glInternalFormat = GL_RGBA8;
            *glFormat = GL_RGBA;
            *glType = GL_UNSIGNED_BYTE;
            break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R32:
            // if (RLGL.ExtSupported.texFloat32) *glInternalFormat = GL_R32F;
            *glFormat = GL_RED;
            *glType = GL_FLOAT;
            break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32:
            // if (RLGL.ExtSupported.texFloat32) *glInternalFormat = GL_RGB32F;
            *glFormat = GL_RGB;
            *glType = GL_FLOAT;
            break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32:
            // if (RLGL.ExtSupported.texFloat32) *glInternalFormat = GL_RGBA32F;
            *glFormat = GL_RGBA;
            *glType = GL_FLOAT;
            break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R16:
            // if (RLGL.ExtSupported.texFloat16) *glInternalFormat = GL_R16F;
            *glFormat = GL_RED;
            *glType = GL_HALF_FLOAT;
            break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16:
            // if (RLGL.ExtSupported.texFloat16) *glInternalFormat = GL_RGB16F;
            *glFormat = GL_RGB;
            *glType = GL_HALF_FLOAT;
            break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16A16:
            // if (RLGL.ExtSupported.texFloat16) *glInternalFormat = GL_RGBA16F;
            *glFormat = GL_RGBA;
            *glType = GL_HALF_FLOAT;
            break;
        default:
            TRACELOG(RL_LOG_WARNING, "TEXTURE: Current format not supported (%i)", format);
            break;
    }
}

// Get name string for pixel format
const char *rlGetPixelFormatName(unsigned int format) {
    switch (format) {
        case RL_PIXELFORMAT_UNCOMPRESSED_GRAYSCALE:
            return "GRAYSCALE";
            break;  // 8 bit per pixel (no alpha)
        case RL_PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA:
            return "GRAY_ALPHA";
            break;  // 8*2 bpp (2 channels)
        case RL_PIXELFORMAT_UNCOMPRESSED_R5G6B5:
            return "R5G6B5";
            break;  // 16 bpp
        case RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8:
            return "R8G8B8";
            break;  // 24 bpp
        case RL_PIXELFORMAT_UNCOMPRESSED_R5G5B5A1:
            return "R5G5B5A1";
            break;  // 16 bpp (1 bit alpha)
        case RL_PIXELFORMAT_UNCOMPRESSED_R4G4B4A4:
            return "R4G4B4A4";
            break;  // 16 bpp (4 bit alpha)
        case RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8:
            return "R8G8B8A8";
            break;  // 32 bpp
        case RL_PIXELFORMAT_UNCOMPRESSED_R32:
            return "R32";
            break;  // 32 bpp (1 channel - float)
        case RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32:
            return "R32G32B32";
            break;  // 32*3 bpp (3 channels - float)
        case RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32:
            return "R32G32B32A32";
            break;  // 32*4 bpp (4 channels - float)
        case RL_PIXELFORMAT_UNCOMPRESSED_R16:
            return "R16";
            break;  // 16 bpp (1 channel - half float)
        case RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16:
            return "R16G16B16";
            break;  // 16*3 bpp (3 channels - half float)
        case RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16A16:
            return "R16G16B16A16";
            break;  // 16*4 bpp (4 channels - half float)
        case RL_PIXELFORMAT_COMPRESSED_DXT1_RGB:
            return "DXT1_RGB";
            break;  // 4 bpp (no alpha)
        case RL_PIXELFORMAT_COMPRESSED_DXT1_RGBA:
            return "DXT1_RGBA";
            break;  // 4 bpp (1 bit alpha)
        case RL_PIXELFORMAT_COMPRESSED_DXT3_RGBA:
            return "DXT3_RGBA";
            break;  // 8 bpp
        case RL_PIXELFORMAT_COMPRESSED_DXT5_RGBA:
            return "DXT5_RGBA";
            break;  // 8 bpp
        case RL_PIXELFORMAT_COMPRESSED_ETC1_RGB:
            return "ETC1_RGB";
            break;  // 4 bpp
        case RL_PIXELFORMAT_COMPRESSED_ETC2_RGB:
            return "ETC2_RGB";
            break;  // 4 bpp
        case RL_PIXELFORMAT_COMPRESSED_ETC2_EAC_RGBA:
            return "ETC2_RGBA";
            break;  // 8 bpp
        case RL_PIXELFORMAT_COMPRESSED_PVRT_RGB:
            return "PVRT_RGB";
            break;  // 4 bpp
        case RL_PIXELFORMAT_COMPRESSED_PVRT_RGBA:
            return "PVRT_RGBA";
            break;  // 4 bpp
        case RL_PIXELFORMAT_COMPRESSED_ASTC_4x4_RGBA:
            return "ASTC_4x4_RGBA";
            break;  // 8 bpp
        case RL_PIXELFORMAT_COMPRESSED_ASTC_8x8_RGBA:
            return "ASTC_8x8_RGBA";
            break;  // 2 bpp
        default:
            return "UNKNOWN";
            break;
    }
}

// Set texture parameters (wrap mode/filter mode)
void rlTextureParameters(unsigned int id, int param, int value) {
    glBindTexture(GL_TEXTURE_2D, id);

    switch (param) {
        case RL_TEXTURE_WRAP_S:
        case RL_TEXTURE_WRAP_T:
            glTexParameteri(GL_TEXTURE_2D, param, value);
            break;
        case RL_TEXTURE_MAG_FILTER:
        case RL_TEXTURE_MIN_FILTER:
            glTexParameteri(GL_TEXTURE_2D, param, value);
            break;
        case RL_TEXTURE_MIPMAP_BIAS_RATIO:
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, value / 100.0f);
        default:
            break;
    }

    glBindTexture(GL_TEXTURE_2D, 0);
}

// Set texture scaling filter mode
void SetTextureFilter(Texture texture, int filter) {
    switch (filter) {
        case TEXTURE_FILTER_POINT: {
            if (texture.mipmaps > 1) {
                // RL_TEXTURE_FILTER_MIP_NEAREST - tex filter: POINT, mipmaps filter: POINT (sharp switching between mipmaps)
                rlTextureParameters(texture.id, RL_TEXTURE_MIN_FILTER, RL_TEXTURE_FILTER_MIP_NEAREST);

                // RL_TEXTURE_FILTER_NEAREST - tex filter: POINT (no filter), no mipmaps
                rlTextureParameters(texture.id, RL_TEXTURE_MAG_FILTER, RL_TEXTURE_FILTER_NEAREST);
            } else {
                // RL_TEXTURE_FILTER_NEAREST - tex filter: POINT (no filter), no mipmaps
                rlTextureParameters(texture.id, RL_TEXTURE_MIN_FILTER, RL_TEXTURE_FILTER_NEAREST);
                rlTextureParameters(texture.id, RL_TEXTURE_MAG_FILTER, RL_TEXTURE_FILTER_NEAREST);
            }
        } break;
        case TEXTURE_FILTER_BILINEAR: {
            if (texture.mipmaps > 1) {
                // RL_TEXTURE_FILTER_LINEAR_MIP_NEAREST - tex filter: BILINEAR, mipmaps filter: POINT (sharp switching between mipmaps)
                // Alternative: RL_TEXTURE_FILTER_NEAREST_MIP_LINEAR - tex filter: POINT, mipmaps filter: BILINEAR (smooth transition between mipmaps)
                rlTextureParameters(texture.id, RL_TEXTURE_MIN_FILTER, RL_TEXTURE_FILTER_LINEAR_MIP_NEAREST);

                // RL_TEXTURE_FILTER_LINEAR - tex filter: BILINEAR, no mipmaps
                rlTextureParameters(texture.id, RL_TEXTURE_MAG_FILTER, RL_TEXTURE_FILTER_LINEAR);
            } else {
                // RL_TEXTURE_FILTER_LINEAR - tex filter: BILINEAR, no mipmaps
                rlTextureParameters(texture.id, RL_TEXTURE_MIN_FILTER, RL_TEXTURE_FILTER_LINEAR);
                rlTextureParameters(texture.id, RL_TEXTURE_MAG_FILTER, RL_TEXTURE_FILTER_LINEAR);
            }
        } break;
        case TEXTURE_FILTER_TRILINEAR: {
            if (texture.mipmaps > 1) {
                // RL_TEXTURE_FILTER_MIP_LINEAR - tex filter: BILINEAR, mipmaps filter: BILINEAR (smooth transition between mipmaps)
                rlTextureParameters(texture.id, RL_TEXTURE_MIN_FILTER, RL_TEXTURE_FILTER_MIP_LINEAR);

                // RL_TEXTURE_FILTER_LINEAR - tex filter: BILINEAR, no mipmaps
                rlTextureParameters(texture.id, RL_TEXTURE_MAG_FILTER, RL_TEXTURE_FILTER_LINEAR);
            } else {
                TRACELOG(LOG_WARNING, "TEXTURE: [ID %i] No mipmaps available for TRILINEAR texture filtering", texture.id);

                // RL_TEXTURE_FILTER_LINEAR - tex filter: BILINEAR, no mipmaps
                rlTextureParameters(texture.id, RL_TEXTURE_MIN_FILTER, RL_TEXTURE_FILTER_LINEAR);
                rlTextureParameters(texture.id, RL_TEXTURE_MAG_FILTER, RL_TEXTURE_FILTER_LINEAR);
            }
        } break;
        case TEXTURE_FILTER_ANISOTROPIC_4X:
            rlTextureParameters(texture.id, RL_TEXTURE_FILTER_ANISOTROPIC, 4);
            break;
        case TEXTURE_FILTER_ANISOTROPIC_8X:
            rlTextureParameters(texture.id, RL_TEXTURE_FILTER_ANISOTROPIC, 8);
            break;
        case TEXTURE_FILTER_ANISOTROPIC_16X:
            rlTextureParameters(texture.id, RL_TEXTURE_FILTER_ANISOTROPIC, 16);
            break;
        default:
            break;
    }
}

// Get next codepoint in a byte sequence and bytes processed
int GetCodepointNext(const char *text, int *codepointSize) {
    const char *ptr = text;
    int codepoint = 0x3f;  // Codepoint (defaults to '?')
    *codepointSize = 1;

    // Get current codepoint and bytes processed
    if (0xf0 == (0xf8 & ptr[0])) {
        // 4 byte UTF-8 codepoint
        if (((ptr[1] & 0xC0) ^ 0x80) || ((ptr[2] & 0xC0) ^ 0x80) || ((ptr[3] & 0xC0) ^ 0x80)) {
            return codepoint;
        }  // 10xxxxxx checks
        codepoint = ((0x07 & ptr[0]) << 18) | ((0x3f & ptr[1]) << 12) | ((0x3f & ptr[2]) << 6) | (0x3f & ptr[3]);
        *codepointSize = 4;
    } else if (0xe0 == (0xf0 & ptr[0])) {
        // 3 byte UTF-8 codepoint */
        if (((ptr[1] & 0xC0) ^ 0x80) || ((ptr[2] & 0xC0) ^ 0x80)) {
            return codepoint;
        }  // 10xxxxxx checks
        codepoint = ((0x0f & ptr[0]) << 12) | ((0x3f & ptr[1]) << 6) | (0x3f & ptr[2]);
        *codepointSize = 3;
    } else if (0xc0 == (0xe0 & ptr[0])) {
        // 2 byte UTF-8 codepoint
        if ((ptr[1] & 0xC0) ^ 0x80) {
            return codepoint;
        }  // 10xxxxxx checks
        codepoint = ((0x1f & ptr[0]) << 6) | (0x3f & ptr[1]);
        *codepointSize = 2;
    } else if (0x00 == (0x80 & ptr[0])) {
        // 1 byte UTF-8 codepoint
        codepoint = ptr[0];
        *codepointSize = 1;
    }

    return codepoint;
}

// Get index position for a unicode character on font
// NOTE: If codepoint is not found in the font it fallbacks to '?'
int GetGlyphIndex(Font font, int codepoint) {
    int index = 0;

    int fallbackIndex = 0;  // Get index of fallback glyph '?'

    // Look for character index in the unordered charset
    for (int i = 0; i < font.glyphCount; i++) {
        if (font.glyphs[i].value == 63) fallbackIndex = i;

        if (font.glyphs[i].value == codepoint) {
            index = i;
            break;
        }
    }

    if ((index == 0) && (font.glyphs[0].value != codepoint)) index = fallbackIndex;

    return index;
}

//----------------------------------------------------------------------------------
// Text strings management functions
//----------------------------------------------------------------------------------
// Get text length in bytes, check for \0 character
unsigned int TextLength(const char *text) {
    unsigned int length = 0;

    if (text != NULL) {
        // NOTE: Alternative: use strlen(text)

        while (*text++) length++;
    }

    return length;
}
