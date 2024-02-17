#include <iostream>

#include "setup_window.hpp"
//
#include "events/CameraEvents.hpp"
#include "math/Vector3.hpp"

// can't increase from camera z position because it causes recursion, so need another variable to keep track of the first z position and zoom in and out based on that
float zoom_baseline = 0;
float scaleFactor = 1;
float newScaleX = 1;
float newScaleY = 1;
float newScaleZ = 1;
float zoomIncrement = 0.250f;

// panning
float cached_mouse_pan_ndc_x = 0;
float cached_mouse_pan_ndc_y = 0;

void camera_zoom(std::shared_ptr<graphics::PerspectiveCamera> camera, int windowWidth, int windowHeight) {
    float wheel = GetMouseWheelMove();
    // Zoom based on mouse wheel
    if (wheel != 0) {
        scaleFactor = wheel * zoomIncrement;

        RVec2 mousePosition = GetMousePosition();
        newScaleZ = camera->position.z - scaleFactor;

        // Convert mouse position to NDC
        float mouse_ndc_x = static_cast<float>((2.0 * mousePosition.x) / static_cast<float>(windowWidth) - 1.0);
        float mouse_ndc_y = static_cast<float>(1.0 - (2.0 * mousePosition.y) / static_cast<float>(windowHeight));

        graphics::Vector3 mouseWorld(mouse_ndc_x, mouse_ndc_y, 0);
        mouseWorld.unproject(*camera);  // convert mouse ndc coordinates to camera view coordinates
        mouseWorld.z = newScaleZ;       // z position is based on scaleFactor and is already on view coordinates

        graphics::Vector3 direction = mouseWorld.sub(camera->position);

        direction.normalize();

        newScaleX = camera->position.x + direction.x * scaleFactor;
        newScaleY = camera->position.y + direction.y * scaleFactor;
        // float newScaleZ = camera->position.z + direction.z;  //* scaleFactor;

        graphics::Vector3 newScale3(newScaleX, newScaleY, newScaleZ);

        camera->position.copy(newScale3);
    }
};

void camera_pan(std::shared_ptr<graphics::PerspectiveCamera> camera, int windowWidth, int windowHeight) {
    RVec2 mousePosition = GetMousePosition();

    // Convert mouse position to NDC
    float mouse_pan_ndc_x = static_cast<float>((2.0 * mousePosition.x) / static_cast<float>(windowWidth) - 1.0);
    float mouse_pan_ndc_y = static_cast<float>(1.0 - (2.0 * mousePosition.y) / static_cast<float>(windowHeight));

    if (cached_mouse_pan_ndc_x == 0) cached_mouse_pan_ndc_x = mouse_pan_ndc_x;
    if (cached_mouse_pan_ndc_y == 0) cached_mouse_pan_ndc_y = mouse_pan_ndc_y;

    float pan_mouse_x = mouse_pan_ndc_x - cached_mouse_pan_ndc_x;
    float pan_mouse_y = mouse_pan_ndc_y - cached_mouse_pan_ndc_y;

    graphics::Vector3 mouseWorld(pan_mouse_x, pan_mouse_y, 0);
    mouseWorld.unproject(*camera);  // convert mouse ndc coordinates to camera view coordinates

    float panX = camera->position.x - pan_mouse_x * 6.8;
    float panY = camera->position.y - pan_mouse_y * 3.3;
    //  float newScaleZ = camera->position.z + direction.z;  //* scaleFactor;

    graphics::Vector3 newPanPosition(panX, panY, camera->position.z);

    camera->position.copy(newPanPosition);

    std::cout << "mouseworld x: " << mouse_pan_ndc_x << " "
              << "mouseworld y: " << mouse_pan_ndc_y << std::endl;

    cached_mouse_pan_ndc_x = mouse_pan_ndc_x;
    cached_mouse_pan_ndc_y = mouse_pan_ndc_y;
}

void camera_mouse_up() {
    cached_mouse_pan_ndc_x = 0;
    cached_mouse_pan_ndc_y = 0;
}
