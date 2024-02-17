#include "cameras/PerspectiveCamera.hpp"

void camera_zoom(std::shared_ptr<graphics::PerspectiveCamera> camera, int windowWidth, int windowHeight);
void camera_pan(std::shared_ptr<graphics::PerspectiveCamera> camera, int windowWidth, int windowHeight);
void camera_mouse_up();
