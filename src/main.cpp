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
#include "Shader.hpp"
#include "cameras/PerspectiveCamera.hpp"
#include "filepath.hpp"
#include "setup_window.hpp"

extern CoreData CORE;

// Main code
int main(int, char**) {
    int windowWidth = 1900;
    int windowHeight = 1200;

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

    auto camera = graphics::PerspectiveCamera::create(75, static_cast<float>(mode->width) / static_cast<float>(mode->height), 0.1f, 100);
    camera->position.z = 5;

    struct backgroundColor {
        float r = 0.0f;
        float g = 0.0f;
        float b = 0.0f;
        float a = 1.0f;
    };

    backgroundColor bgColor;

    // Specify the relative path to the shader file
    std::string relativeVertexShaderPath = "shaders/basic.vert";
    std::string relativeFragmentShaderPath = "shaders/basic.frag";

    // Construct the absolute path to the shader file
    std::optional<std::string> absoluteVertexShaderPath = GetAssetsPath(relativeVertexShaderPath);
    std::optional<std::string> absoluteFragmentShaderPath = GetAssetsPath(relativeFragmentShaderPath);

    // Specify relative paths for vertex and fragment shaders
    // std::string vertexShaderPath = "../assets/shaders/meshbasic.vert";
    // std::string fragmentShaderPath = "../assets/shaders/meshbasic.frag";

    // Program mesh_program = Program::Program(mesh_for_program, vertexShaderPath, fragmentShaderPath);
    //  Create a shared pointer to a Program instance
    Shader shader(*absoluteVertexShaderPath, *absoluteFragmentShaderPath);

    std::vector<float> vertices{
        -0.5f, -0.5f, 0,  // left
        0.5f, -0.5f, 0,   // right
        0.0f, 0.5f, 0     // top
    };

    // create buffer for triangle
    shader.createBuffer("position", vertices, GL_STATIC_DRAW, 3, 0, 0);
    shader.Use();

    shader.set_glUniformMatrix4fv("projection", camera->projectionMatrix);  // setup shader projection matrix

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

        // update camera matrices and frustum
        if (camera->parent == nullptr) camera->updateMatrixWorld();
        shader.set_glUniformMatrix4fv("modelView", camera->matrixWorldInverse);

        // rendering goes here
        shader.render(GL_TRIANGLES, 0, 3);

        gltBeginDraw();
        // update camera position display
        cameraLog << "Camera x: " << camera->position.x << " Camera y: " << camera->position.y << " Camera z: " << camera->position.z;

        gltSetText(text, cameraLog.str().c_str());

        cameraLog.str("");
        cameraLog.clear();

        // sprintf(str, "Time: %.4f", time);
        // gltSetText(text, str);
        gltColor(cosf((float)time) * 0.5f + 0.5f, sinf((float)time) * 0.5f + 0.5f, 1.0f, 1.0f);

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
