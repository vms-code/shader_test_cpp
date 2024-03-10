#include "setup_window.hpp"

#include <iostream>

// extern GLFWwindow *InitWindow(int windowWidth, int windowHeight, const char *title);
CoreData CORE = { 0 };  // Global CORE state context
GLFWwindow *window;

// Input callbacks events
// Error callback event
static void glfw_error_callback(int error, const char *description);  // GLFW3 Error Callback, runs on GLFW3 error

// Window callbacks events
void framebuffer_size_callback(GLFWwindow *window, int width, int height);

static void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);  // GLFW3 Keyboard Callback, runs on key pressed
static void CharCallback(GLFWwindow *window, unsigned int key);                            // GLFW3 Char Key Callback, runs on key pressed (get char value)
static void MouseButtonCallback(GLFWwindow *window, int button, int action, int mods);     // GLFW3 Mouse Button Callback, runs on mouse button pressed
static void MouseCursorPosCallback(GLFWwindow *window, double x, double y);                // GLFW3 Cursor Position Callback, runs on mouse move
static void MouseScrollCallback(GLFWwindow *window, double xoffset, double yoffset);       // GLFW3 Srolling Callback, runs on mouse wheel
static void CursorEnterCallback(GLFWwindow *window, int enter);                            // GLFW3 Cursor Enter Callback, cursor enters client area

//----------------------------------------------------------------------------------
// Functions to be used on glfw callback
//----------------------------------------------------------------------------------
// GLFW3 Error Callback, runs on GLFW3 error
static void glfw_error_callback(int error, const char *description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// GLFW3 Keyboard Callback, runs on key pressed
static void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key < 0) return;  // Security check, macOS fn key generates -1

    // WARNING: GLFW could return GLFW_REPEAT, we need to consider it as 1
    // to work properly with our implementation (IsKeyDown/IsKeyUp checks)
    if (action == GLFW_RELEASE)
        CORE.Input.Keyboard.currentKeyState[key] = 0;
    else if (action == GLFW_PRESS)
        CORE.Input.Keyboard.currentKeyState[key] = 1;
    else if (action == GLFW_REPEAT)
        CORE.Input.Keyboard.keyRepeatInFrame[key] = 1;

    // Check if there is space available in the key queue
    if ((CORE.Input.Keyboard.keyPressedQueueCount < MAX_KEY_PRESSED_QUEUE) && (action == GLFW_PRESS)) {
        // Add character to the queue
        CORE.Input.Keyboard.keyPressedQueue[CORE.Input.Keyboard.keyPressedQueueCount] = key;
        CORE.Input.Keyboard.keyPressedQueueCount++;
    }

    // Check the exit key to set close window
    if ((key == CORE.Input.Keyboard.exitKey) && (action == GLFW_PRESS)) glfwSetWindowShouldClose(window, GLFW_TRUE);
}

// GLFW3 Char Key Callback, runs on key down (gets equivalent unicode char value)
static void CharCallback(GLFWwindow *window, unsigned int key) {
    // TRACELOG(LOG_DEBUG, "Char Callback: KEY:%i(%c)", key, key);

    // NOTE: Registers any key down considering OS keyboard layout but
    // does not detect action events, those should be managed by user...
    // Ref: https://github.com/glfw/glfw/issues/668#issuecomment-166794907
    // Ref: https://www.glfw.org/docs/latest/input_guide.html#input_char

    // Check if there is space available in the queue
    if (CORE.Input.Keyboard.charPressedQueueCount < MAX_CHAR_PRESSED_QUEUE) {
        // Add character to the queue
        CORE.Input.Keyboard.charPressedQueue[CORE.Input.Keyboard.charPressedQueueCount] = key;
        CORE.Input.Keyboard.charPressedQueueCount++;
    }
}

// GLFW3 Mouse Button Callback, runs on mouse button pressed
static void MouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
    // WARNING: GLFW could only return GLFW_PRESS (1) or GLFW_RELEASE (0) for now,
    // but future releases may add more actions (i.e. GLFW_REPEAT)
    CORE.Input.Mouse.currentButtonState[button] = action;

#if defined(SUPPORT_GESTURES_SYSTEM) && defined(SUPPORT_MOUSE_GESTURES)
    // Process mouse events as touches to be able to use mouse-gestures
    GestureEvent gestureEvent = { 0 };

    // Register touch actions
    if ((CORE.Input.Mouse.currentButtonState[button] == 1) && (CORE.Input.Mouse.previousButtonState[button] == 0))
        gestureEvent.touchAction = TOUCH_ACTION_DOWN;
    else if ((CORE.Input.Mouse.currentButtonState[button] == 0) && (CORE.Input.Mouse.previousButtonState[button] == 1))
        gestureEvent.touchAction = TOUCH_ACTION_UP;

    // NOTE: TOUCH_ACTION_MOVE event is registered in MouseCursorPosCallback()

    // Assign a pointer ID
    gestureEvent.pointId[0] = 0;

    // Register touch points count
    gestureEvent.pointCount = 1;

    // Register touch points position, only one point registered
    gestureEvent.position[0] = GetMousePosition();

    // Normalize gestureEvent.position[0] for CORE.Window.screen.width and CORE.Window.screen.height
    gestureEvent.position[0].x /= (float)GetScreenWidth();
    gestureEvent.position[0].y /= (float)GetScreenHeight();

    // Gesture data is sent to gestures-system for processing
    // Prevent calling ProcessGestureEvent() when Emscripten is present and there's a touch gesture, so EmscriptenTouchCallback() can handle it itself
    if (GetMouseX() != 0 || GetMouseY() != 0) ProcessGestureEvent(gestureEvent);

#endif
}

// GLFW3 Cursor Position Callback, runs on mouse move
static void MouseCursorPosCallback(GLFWwindow *window, double x, double y) {
    CORE.Input.Mouse.currentPosition.x = (float)x;
    CORE.Input.Mouse.currentPosition.y = (float)y;
    CORE.Input.Touch.position[0] = CORE.Input.Mouse.currentPosition;

    // std::cout << "mouse position callback X: " << CORE.Input.Mouse.currentPosition.x << std::endl;

#if defined(SUPPORT_GESTURES_SYSTEM) && defined(SUPPORT_MOUSE_GESTURES)
    // Process mouse events as touches to be able to use mouse-gestures
    GestureEvent gestureEvent = { 0 };

    gestureEvent.touchAction = TOUCH_ACTION_MOVE;

    // Assign a pointer ID
    gestureEvent.pointId[0] = 0;

    // Register touch points count
    gestureEvent.pointCount = 1;

    // Register touch points position, only one point registered
    gestureEvent.position[0] = touch.position[0];

    // Normalize gestureEvent.position[0] for CORE.Window.screen.width and CORE.Window.screen.height
    gestureEvent.position[0].x /= (float)GetScreenWidth();
    gestureEvent.position[0].y /= (float)GetScreenHeight();

    // Gesture data is sent to gestures-system for processing
    ProcessGestureEvent(gestureEvent);
#endif
}

// GLFW3 Scrolling Callback, runs on mouse wheel
static void MouseScrollCallback(GLFWwindow *window, double xoffset, double yoffset) {
    // CORE.Input.Mouse.previousWheelMove = CORE.Input.Mouse.currentWheelMove;
    // CORE.Input.Mouse.currentWheelMove = { (float)xoffset, (float)yoffset };
    CORE.Input.Mouse.currentWheelMove.x = (float)xoffset;
    CORE.Input.Mouse.currentWheelMove.y = (float)yoffset;
}

// GLFW3 CursorEnter Callback, when cursor enters the window
static void CursorEnterCallback(GLFWwindow *window, int enter) {
    if (enter)
        CORE.Input.Mouse.cursorOnScreen = true;
    else
        CORE.Input.Mouse.cursorOnScreen = false;
}

// Initialize platform: graphics, inputs and more
GLFWwindow *InitWindow(int windowWidth, int windowHeight, const char *title) {
    glfwSetErrorCallback(glfw_error_callback);

    // Initialize GLFW internal global state
    int result = glfwInit();
    if (result == GLFW_FALSE) {
        // TRACELOG(LOG_WARNING, "GLFW: Failed to initialize GLFW");
        return nullptr;
    }

    // Initialize graphic device: display/window and graphic context
    //----------------------------------------------------------------------------
    glfwDefaultWindowHints();  // Set default windows hints
    // glfwWindowHint(GLFW_RED_BITS, 8);             // Framebuffer red color component bits
    // glfwWindowHint(GLFW_GREEN_BITS, 8);           // Framebuffer green color component bits
    // glfwWindowHint(GLFW_BLUE_BITS, 8);            // Framebuffer blue color component bits
    // glfwWindowHint(GLFW_ALPHA_BITS, 8);           // Framebuffer alpha color component bits
    // glfwWindowHint(GLFW_DEPTH_BITS, 24);          // Depthbuffer bits
    // glfwWindowHint(GLFW_REFRESH_RATE, 0);         // Refresh rate for fullscreen window
    // glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API); // OpenGL API to use. Alternative: GLFW_OPENGL_ES_API
    // glfwWindowHint(GLFW_AUX_BUFFERS, 0);          // Number of auxiliar buffers

    // NOTE: When asking for an OpenGL context version, most drivers provide the highest supported version
    // with backward compatibility to older OpenGL versions.
    // For example, if using OpenGL 1.1, driver can provide a 4.3 backwards compatible context.

    // helpful for debugging
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

    // GL 3.0 + GLSL 130
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only

    // Create window with graphics context
    window = glfwCreateWindow(windowWidth, windowHeight, title, nullptr, nullptr);
    if (window == nullptr) {
        glfwTerminate();
        return nullptr;
    }

    // Set input callback events
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCharCallback(window, CharCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    glfwSetCursorPosCallback(window, MouseCursorPosCallback);  // Track mouse position changes
    glfwSetScrollCallback(window, MouseScrollCallback);
    glfwSetCursorEnterCallback(window, CursorEnterCallback);

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glfwSwapInterval(1);  // Enable vsync

    return window;
}

//----------------------------------------------------------------------------------
// Module Functions Definition: Input Handling: Keyboard
//----------------------------------------------------------------------------------

// Check if a key has been pressed once
bool IsKeyPressed(int key) {
    bool pressed = false;

    if ((key > 0) && (key < MAX_KEYBOARD_KEYS)) {
        if ((CORE.Input.Keyboard.previousKeyState[key] == 0) && (CORE.Input.Keyboard.currentKeyState[key] == 1)) pressed = true;
    }

    return pressed;
}

// Check if a key has been pressed again
bool IsKeyPressedRepeat(int key) {
    bool repeat = false;

    if ((key > 0) && (key < MAX_KEYBOARD_KEYS)) {
        if (CORE.Input.Keyboard.keyRepeatInFrame[key] == 1) repeat = true;
    }

    return repeat;
}

// Check if a key is being pressed (key held down)
bool IsKeyDown(int key) {
    bool down = false;

    if ((key > 0) && (key < MAX_KEYBOARD_KEYS)) {
        if (CORE.Input.Keyboard.currentKeyState[key] == 1) down = true;
    }

    return down;
}

// Check if a key has been released once
bool IsKeyReleased(int key) {
    bool released = false;

    if ((key > 0) && (key < MAX_KEYBOARD_KEYS)) {
        if ((CORE.Input.Keyboard.previousKeyState[key] == 1) && (CORE.Input.Keyboard.currentKeyState[key] == 0)) released = true;
    }

    return released;
}

// Check if a key is NOT being pressed (key not held down)
bool IsKeyUp(int key) {
    bool up = false;

    if ((key > 0) && (key < MAX_KEYBOARD_KEYS)) {
        if (CORE.Input.Keyboard.currentKeyState[key] == 0) up = true;
    }

    return up;
}

// Get the last key pressed
int GetKeyPressed(void) {
    int value = 0;

    if (CORE.Input.Keyboard.keyPressedQueueCount > 0) {
        // Get character from the queue head
        value = CORE.Input.Keyboard.keyPressedQueue[0];

        // Shift elements 1 step toward the head
        for (int i = 0; i < (CORE.Input.Keyboard.keyPressedQueueCount - 1); i++)
            CORE.Input.Keyboard.keyPressedQueue[i] = CORE.Input.Keyboard.keyPressedQueue[i + 1];

        // Reset last character in the queue
        CORE.Input.Keyboard.keyPressedQueue[CORE.Input.Keyboard.keyPressedQueueCount - 1] = 0;
        CORE.Input.Keyboard.keyPressedQueueCount--;
    }

    return value;
}

// Get the last char pressed
int GetCharPressed(void) {
    int value = 0;

    if (CORE.Input.Keyboard.charPressedQueueCount > 0) {
        // Get character from the queue head
        value = CORE.Input.Keyboard.charPressedQueue[0];

        // Shift elements 1 step toward the head
        for (int i = 0; i < (CORE.Input.Keyboard.charPressedQueueCount - 1); i++)
            CORE.Input.Keyboard.charPressedQueue[i] = CORE.Input.Keyboard.charPressedQueue[i + 1];

        // Reset last character in the queue
        CORE.Input.Keyboard.charPressedQueue[CORE.Input.Keyboard.charPressedQueueCount - 1] = 0;
        CORE.Input.Keyboard.charPressedQueueCount--;
    }

    return value;
}

// Set a custom key to exit program
// NOTE: default exitKey is set to ESCAPE
void SetExitKey(int key) {
    CORE.Input.Keyboard.exitKey = key;
}

//----------------------------------------------------------------------------------
// Module Functions Definition: Input Handling: Gamepad
//----------------------------------------------------------------------------------

// NOTE: Functions with a platform-specific implementation on rcore_<platform>.c
// int SetGamepadMappings(const char *mappings)

// Check if a gamepad is available
bool IsGamepadAvailable(int gamepad) {
    bool result = false;

    if ((gamepad < MAX_GAMEPADS) && CORE.Input.Gamepad.ready[gamepad]) result = true;

    return result;
}

// Get gamepad internal name id
const char *GetGamepadName(int gamepad) {
    return CORE.Input.Gamepad.name[gamepad];
}

// Check if a gamepad button has been pressed once
bool IsGamepadButtonPressed(int gamepad, int button) {
    bool pressed = false;

    if ((gamepad < MAX_GAMEPADS) && CORE.Input.Gamepad.ready[gamepad] && (button < MAX_GAMEPAD_BUTTONS) &&
        (CORE.Input.Gamepad.previousButtonState[gamepad][button] == 0) && (CORE.Input.Gamepad.currentButtonState[gamepad][button] == 1)) pressed = true;

    return pressed;
}

// Check if a gamepad button is being pressed
bool IsGamepadButtonDown(int gamepad, int button) {
    bool down = false;

    if ((gamepad < MAX_GAMEPADS) && CORE.Input.Gamepad.ready[gamepad] && (button < MAX_GAMEPAD_BUTTONS) &&
        (CORE.Input.Gamepad.currentButtonState[gamepad][button] == 1)) down = true;

    return down;
}

// Check if a gamepad button has NOT been pressed once
bool IsGamepadButtonReleased(int gamepad, int button) {
    bool released = false;

    if ((gamepad < MAX_GAMEPADS) && CORE.Input.Gamepad.ready[gamepad] && (button < MAX_GAMEPAD_BUTTONS) &&
        (CORE.Input.Gamepad.previousButtonState[gamepad][button] == 1) && (CORE.Input.Gamepad.currentButtonState[gamepad][button] == 0)) released = true;

    return released;
}

// Check if a gamepad button is NOT being pressed
bool IsGamepadButtonUp(int gamepad, int button) {
    bool up = false;

    if ((gamepad < MAX_GAMEPADS) && CORE.Input.Gamepad.ready[gamepad] && (button < MAX_GAMEPAD_BUTTONS) &&
        (CORE.Input.Gamepad.currentButtonState[gamepad][button] == 0)) up = true;

    return up;
}

// Get the last gamepad button pressed
int GetGamepadButtonPressed(void) {
    return CORE.Input.Gamepad.lastButtonPressed;
}

// Get gamepad axis count
int GetGamepadAxisCount(int gamepad) {
    return CORE.Input.Gamepad.axisCount[gamepad];
}

// Get axis movement vector for a gamepad
float GetGamepadAxisMovement(int gamepad, int axis) {
    float value = 0;

    if ((gamepad < MAX_GAMEPADS) && CORE.Input.Gamepad.ready[gamepad] && (axis < MAX_GAMEPAD_AXIS) &&
        (fabsf(CORE.Input.Gamepad.axisState[gamepad][axis]) > 0.1f)) value = CORE.Input.Gamepad.axisState[gamepad][axis];  // 0.1f = GAMEPAD_AXIS_MINIMUM_DRIFT/DELTA

    return value;
}

//----------------------------------------------------------------------------------
// Module Functions Definition: Input Handling: Mouse
//----------------------------------------------------------------------------------

// NOTE: Functions with a platform-specific implementation on rcore_<platform>.c
// Set mouse position XY
void SetMousePosition(int x, int y) {
    CORE.Input.Mouse.currentPosition = { (float)x, (float)y };
    CORE.Input.Mouse.previousPosition = CORE.Input.Mouse.currentPosition;

    // NOTE: emscripten not implemented
    glfwSetCursorPos(window, CORE.Input.Mouse.currentPosition.x, CORE.Input.Mouse.currentPosition.y);
}
// void SetMouseCursor(int cursor)
// Set mouse cursor
void SetMouseCursor(int cursor) {
    CORE.Input.Mouse.cursor = cursor;
    if (cursor == MOUSE_CURSOR_DEFAULT)
        glfwSetCursor(window, NULL);
    else {
        // NOTE: We are relating internal GLFW enum values to our MouseCursor enum values
        glfwSetCursor(window, glfwCreateStandardCursor(0x00036000 + cursor));
    }
}

// Check if a mouse button has been pressed once
bool IsMouseButtonPressed(int button) {
    bool pressed = false;

    if ((CORE.Input.Mouse.currentButtonState[button] == 1) && (CORE.Input.Mouse.previousButtonState[button] == 0)) pressed = true;

    // Map touches to mouse buttons checking
    if ((CORE.Input.Touch.currentTouchState[button] == 1) && (CORE.Input.Touch.previousTouchState[button] == 0)) pressed = true;

    return pressed;
}

// Check if a mouse button is being pressed
bool IsMouseButtonDown(int button) {
    bool down = false;

    if (CORE.Input.Mouse.currentButtonState[button] == 1) down = true;

    // NOTE: Touches are considered like mouse buttons
    if (CORE.Input.Touch.currentTouchState[button] == 1) down = true;

    return down;
}

// Check if a mouse button has been released once
bool IsMouseButtonReleased(int button) {
    bool released = false;

    if ((CORE.Input.Mouse.currentButtonState[button] == 0) && (CORE.Input.Mouse.previousButtonState[button] == 1)) released = true;

    // Map touches to mouse buttons checking
    if ((CORE.Input.Touch.currentTouchState[button] == 0) && (CORE.Input.Touch.previousTouchState[button] == 1)) released = true;

    return released;
}

// Check if a mouse button is NOT being pressed
bool IsMouseButtonUp(int button) {
    bool up = false;

    if (CORE.Input.Mouse.currentButtonState[button] == 0) up = true;

    // NOTE: Touches are considered like mouse buttons
    if (CORE.Input.Touch.currentTouchState[button] == 0) up = true;

    return up;
}

// Get mouse position X
int GetMouseX(void) {
    return (int)((CORE.Input.Mouse.currentPosition.x + CORE.Input.Mouse.offset.x) * CORE.Input.Mouse.scale.x);
}

// Get mouse position Y
int GetMouseY(void) {
    return (int)((CORE.Input.Mouse.currentPosition.y + CORE.Input.Mouse.offset.y) * CORE.Input.Mouse.scale.y);
}

// Get mouse position XY
RVec2 GetMousePosition(void) {
    RVec2 position = { 0 };

    position.x = (CORE.Input.Mouse.currentPosition.x + CORE.Input.Mouse.offset.x) * CORE.Input.Mouse.scale.x;
    position.y = (CORE.Input.Mouse.currentPosition.y + CORE.Input.Mouse.offset.y) * CORE.Input.Mouse.scale.y;

    return CORE.Input.Mouse.currentPosition;
}

// Get mouse delta between frames
RVec2 GetMouseDelta(void) {
    RVec2 delta = { 0 };

    delta.x = CORE.Input.Mouse.currentPosition.x - CORE.Input.Mouse.previousPosition.x;
    delta.y = CORE.Input.Mouse.currentPosition.y - CORE.Input.Mouse.previousPosition.y;

    return delta;
}

// Set mouse offset
// NOTE: Useful when rendering to different size targets
void SetMouseOffset(int offsetX, int offsetY) {
    CORE.Input.Mouse.offset = { (float)offsetX, (float)offsetY };
}

// Set mouse scaling
// NOTE: Useful when rendering to different size targets
void SetMouseScale(float scaleX, float scaleY) {
    CORE.Input.Mouse.scale = { scaleX, scaleY };
}

// Get mouse wheel movement Y
float GetMouseWheelMove(void) {
    float result = 0.0f;

    if (fabsf(CORE.Input.Mouse.currentWheelMove.x) > fabsf(CORE.Input.Mouse.currentWheelMove.y))
        result = (float)CORE.Input.Mouse.currentWheelMove.x;
    else
        result = (float)CORE.Input.Mouse.currentWheelMove.y;

    CORE.Input.Mouse.currentWheelMove = { 0, 0 };

    return result;
}

// Get mouse wheel movement X/Y as a vector
RVec2 GetMouseWheelMoveV(void) {
    RVec2 result = { 0 };

    result = CORE.Input.Mouse.currentWheelMove;

    return result;
}

//----------------------------------------------------------------------------------
// Module Functions Definition: Input Handling: Touch
//----------------------------------------------------------------------------------

// Get touch position X for touch point 0 (relative to screen size)
int GetTouchX(void) {
    return (int)CORE.Input.Touch.position[0].x;
}

// Get touch position Y for touch point 0 (relative to screen size)
int GetTouchY(void) {
    return (int)CORE.Input.Touch.position[0].y;
}

// Get touch position XY for a touch point index (relative to screen size)
// TODO: Touch position should be scaled depending on display size and render size
RVec2 GetTouchPosition(int index) {
    RVec2 position = { -1.0f, -1.0f };

    if (index < MAX_TOUCH_POINTS)
        position = CORE.Input.Touch.position[index];
    else
        // TRACELOG(LOG_WARNING, "INPUT: Required touch point out of range (Max touch points: %i)", MAX_TOUCH_POINTS);
        printf("GetTouchPosition Error");

    return position;
}

// Get touch point identifier for given index
int GetTouchPointId(int index) {
    int id = -1;

    if (index < MAX_TOUCH_POINTS) id = CORE.Input.Touch.pointId[index];

    return id;
}

// Get number of touch points
int GetTouchPointCount(void) {
    return CORE.Input.Touch.pointCount;
}

// Get current screen width
int GetScreenWidth(void) {
    return CORE.Window.screen.width;
}

// Get current screen height
int GetScreenHeight(void) {
    return CORE.Window.screen.height;
}
