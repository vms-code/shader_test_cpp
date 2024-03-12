#include <glad/gl.h>
//    Will drag system OpenGL headers
#include <GLFW/glfw3.h>
#include <stdio.h>

#include <iostream>
#include <sstream>
#include <string>

#define GLT_IMPLEMENTATION
#include "gltext.h"

//
#include "Events/CameraEvents.hpp"
#include "Label/LabelShader.hpp"
#include "Shader.hpp"
#include "cameras/PerspectiveCamera.hpp"
#include "filepath.hpp"
#include "setup_window.hpp"

extern CoreData CORE;

// Main code
int main(int, char**) {
    int windowWidth = 1500;
    int windowHeight = 900;

    // Create window with graphics context
    GLFWwindow* window = InitWindow(windowWidth, windowHeight, "Dear ImGui GLFW+OpenGL3 example");
    if (window == nullptr)
        return -1;

    int version = gladLoadGL(glfwGetProcAddress);
    if (version == 0) {
        printf("Failed to initialize OpenGL context\n");
        return -1;
    }

    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    float aspectRatio = static_cast<float>(mode->width) / static_cast<float>(mode->height);

    auto camera = graphics::PerspectiveCamera::create(75, aspectRatio, 0.1f, 100);
    camera->position.z = 5;

    struct backgroundColor {
        float r = 0.0f;
        float g = 0.0f;
        float b = 0.0f;
        float a = 1.0f;
    };

    backgroundColor bgColor = { 0.5f, 0.5f, 0.5f, 1.0f };

    // Specify the relative path to the shader file
    std::string relativeVertexShaderPath = "shaders/basic.vert";
    std::string relativeFragmentShaderPath = "shaders/basic.frag";

    // Construct the absolute path to the shader file
    std::optional<std::string> absoluteVertexShaderPath = GetAssetsPath(relativeVertexShaderPath);
    std::optional<std::string> absoluteFragmentShaderPath = GetAssetsPath(relativeFragmentShaderPath);

    //  Create a shared pointer to a Program instance
    Shader shader(*absoluteVertexShaderPath, *absoluteFragmentShaderPath);

    std::vector<float> vertices{
        -0.5f, -0.5f, 0,  // left
        0.5f, -0.5f, 0,   // right
        0.0f, 0.5f, 0     // top
    };

    // create buffer for triangle
    shader.createBuffer("position", vertices, GL_STATIC_DRAW, 3, 0, 0);
    shader.set_glUniformMatrix4fv("projection", camera->projectionMatrix);  // setup shader projection matrix
    glUseProgram(0);

    // SETUP TEXT SHADER
    std::string relativeFontPath = "fonts/anonymous_pro_bold.ttf";
    // std::optional<std::string> absoluteFontPath = GetAssetsPath(relativeFontPath);
    std::optional<std::string> absoluteFontPath = "C:\\Users\\vitor\\Documents\\CODE\\C++\\ShaderTraining\\simple_test_project\\build\\assets\\fonts\\anonymous_pro_bold.ttf";

    // Specify the relative path to the shader file
    std::string relativeTextVertexShaderPath = "shaders/text.vert";
    std::string relativeTextFragmentShaderPath = "shaders/sdf.frag";

    // Construct the absolute path to the shader file
    std::optional<std::string> absoluteTextVertexShaderPath = GetAssetsPath(relativeTextVertexShaderPath);
    std::optional<std::string> absoluteTextFragmentShaderPath = GetAssetsPath(relativeTextFragmentShaderPath);

    Color labelColor{ 1.0, 1.0, 0.0 };
    graphics::LabelShader textShader("}()?;&*+-/[]@#$%'\"^~:_=.\n this is a new line", *absoluteTextVertexShaderPath, *absoluteTextFragmentShaderPath, *absoluteFontPath, labelColor);
    textShader.set_glUniformMatrix4fv("projection", camera->projectionMatrix);  // setup textShader projection matrix
    glUseProgram(0);

    // TEXT FOR CAMERA INFO UPDATE
    // Initialize glText
    if (!gltInit()) {
        fprintf(stderr, "Failed to initialize glText\n");
        glfwTerminate();
        return EXIT_FAILURE;
    }
    // Creating text
    GLTtext* text = gltCreateText();

    int viewportWidth, viewportHeight;
    double time;

    std::ostringstream cameraLog;
    // textShader.rotateX(180.0f);
    //   textShader.translateY(-2.0f);

    double rotation = 0.0;
    graphics::Vector3 text_position{ 0, 0, 0 };
    // textShader.rotateX(180);
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        time = glfwGetTime();

        glfwGetFramebufferSize(window, &viewportWidth, &viewportHeight);
        glViewport(0, 0, viewportWidth, viewportHeight);

        glClearColor(bgColor.r, bgColor.g, bgColor.b, bgColor.a);
        glClear(GL_COLOR_BUFFER_BIT);

        camera_zoom(camera, viewportWidth, viewportHeight);
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
            camera_pan(camera, viewportWidth, viewportHeight);
        else if (IsMouseButtonUp(MOUSE_BUTTON_LEFT))
            camera_mouse_up();

        /*
        textShader.rotateX(rotation);
        if (rotation > 180) {
            rotation -= 0.000001;
        } else {
            rotation += 0.000001;
        }
        */

        // textShader.position.copy(text_position);
        //  text_position.y -= 0.01f;

        // update camera matrices and frustum
        textShader.updateMatrixWorld();
        if (camera->parent == nullptr) camera->updateMatrixWorld();
        shader.set_glUniformMatrix4fv("modelView", camera->matrixWorldInverse);
        glUseProgram(0);

        // textShader.matrixWorld->lookAt(textShader.position, camera->position, { 0.0, 1.0, 0.0 });
        // textShader.matrixWorld->makeRotationX(180.0f);
        textShader.modelViewMatrix.multiplyMatrices(camera->matrixWorldInverse, *textShader.matrixWorld);
        textShader.normalMatrix.getNormalMatrix(textShader.modelViewMatrix);
        textShader.set_glUniformMatrix4fv("modelView", textShader.modelViewMatrix);
        glUseProgram(0);

        // rendering goes here
        // shader.render(GL_TRIANGLES, 0, 3);
        textShader.render();

        gltBeginDraw();
        // update camera position display
        cameraLog << "Camera x: " << camera->position.x << " Camera y: " << camera->position.y << " Camera z: " << camera->position.z;

        gltSetText(text, cameraLog.str().c_str());

        cameraLog.str("");
        cameraLog.clear();

        // sprintf(str, "Time: %.4f", time);
        // gltSetText(text, str);
        // gltColor(cosf((float)time) * 0.5f + 0.5f, sinf((float)time) * 0.5f + 0.5f, 1.0f, 1.0f);

        gltDrawText2DAligned(text, 0.0f, (GLfloat)viewportHeight, 2.0f, GLT_LEFT, GLT_BOTTOM);

        gltEndDraw();

        glfwSwapBuffers(window);
    }

    gltDeleteText(text);
    gltTerminate();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
