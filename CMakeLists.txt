cmake_minimum_required(VERSION 3.8)
project(shaders_test LANGUAGES CXX)

#set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS true)

# Set the build type to Debug
#set(CMAKE_BUILD_TYPE Debug)

# Set additional compiler options for Debug build
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g3 -Wall")

# Set C++ standard to C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(external/glfw)
#add_subdirectory(external/freetype)

find_package(OpenGL REQUIRED)

# graphics helper function files
include(CommonVariables.cmake)

add_executable(${PROJECT_NAME} 
    src/main.cpp
    src/Shader.cpp
    src/objects/Label/LabelShader.cpp
    src/objects/Label/helpers.cpp
    src/setup_window.cpp
    src/Events/CameraEvents.cpp

    external/glad/src/gl.c ${GRAPHICS_SOURCES}
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include/objects
        ${CMAKE_CURRENT_SOURCE_DIR}/external
        ${CMAKE_CURRENT_SOURCE_DIR}/external/glad/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE glfw ${OPENGL_LIBRARIES})

# Add the assets directory to the include path
include_directories(assets)

# Copy assets to the build directory
file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})
