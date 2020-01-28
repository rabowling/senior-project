#include "Controls.h"

#include "Application.h"

void Controls::init() {
    glfwGetCursorPos(app.windowManager.getHandle(), &prevXpos, &prevYpos);
}

void Controls::update() {
    prevHeld = held;
    pressed.clear();
    held.clear();
    released.clear();

    GLFWwindow *handle = app.windowManager.getHandle();
    if (glfwGetKey(handle, GLFW_KEY_W) == GLFW_PRESS) {
        handleEvent(FORWARD);
    }
    if (glfwGetKey(handle, GLFW_KEY_S) == GLFW_PRESS) {
        handleEvent(BACKWARD);
    }
    if (glfwGetKey(handle, GLFW_KEY_A) == GLFW_PRESS) {
        handleEvent(LEFT);
    }
    if (glfwGetKey(handle, GLFW_KEY_D) == GLFW_PRESS) {
        handleEvent(RIGHT);
    }
    if (glfwGetKey(handle, GLFW_KEY_SPACE) == GLFW_PRESS) {
        handleEvent(JUMP);
    }
    if (glfwGetKey(handle, GLFW_KEY_E) == GLFW_PRESS) {
        handleEvent(USE);
    }
    if (glfwGetMouseButton(handle, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        handleEvent(PRIMARY_FIRE);
    }
    if (glfwGetMouseButton(handle, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        handleEvent(SECONDARY_FIRE);
    }

    for (InputEvent e : prevHeld) {
        if (held.find(e) == held.end()) {
            released.insert(e);
        }
    }

    double xpos, ypos;
    glfwGetCursorPos(handle, &xpos, &ypos);
    int width, height;
    glfwGetFramebufferSize(handle, &width, &height);
    mouseDeltaX = (prevXpos - xpos) / height * mouseSensitivity;
    mouseDeltaY = (prevYpos - ypos) / height * mouseSensitivity;
    prevXpos = xpos;
    prevYpos = ypos;
}

void Controls::handleEvent(InputEvent e) {
    if (prevHeld.find(e) != prevHeld.end()) {
        held.insert(e);
    }
    else {
        pressed.insert(e);
        held.insert(e);
    }
}