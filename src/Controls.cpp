#include "Controls.h"

#include "Application.h"
#include "Raytrace.h"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

void Controls::init(InputMode mode, string recordingFile) {
    glfwGetCursorPos(app.windowManager.getHandle(), &prevXpos, &prevYpos);
    this->mode = mode;
    this->recordingFile = recordingFile;
    if (mode == PLAYBACK) {
        loadRecording(recordingFile);
    }
}

void Controls::update() {
    GLFWwindow *handle = app.windowManager.getHandle();
    if (mode == RECORD || mode == NORMAL) {
        prevHeld = held;
        pressed.clear();
        held.clear();
        released.clear();

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
        if (glfwGetKey(handle, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
            handleEvent(SPRINT);
        }
        if (glfwGetMouseButton(handle, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            handleEvent(PRIMARY_FIRE);
        }
        if (glfwGetMouseButton(handle, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
            handleEvent(SECONDARY_FIRE);
        }
        if (glfwGetKey(handle, GLFW_KEY_UP) == GLFW_PRESS) {
            handleEvent(LIGHT_FORWARD);
        }
        if (glfwGetKey(handle, GLFW_KEY_DOWN) == GLFW_PRESS) {
            handleEvent(LIGHT_BACKWARD);
        }
        if (glfwGetKey(handle, GLFW_KEY_LEFT) == GLFW_PRESS) {
            handleEvent(LIGHT_LEFT);
        }
        if (glfwGetKey(handle, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            handleEvent(LIGHT_RIGHT);
        }
        if (glfwGetKey(handle, GLFW_KEY_Q) == GLFW_PRESS) {
            handleEvent(LIGHT_UP);
        }
        if (glfwGetKey(handle, GLFW_KEY_Z) == GLFW_PRESS) {
            handleEvent(LIGHT_DOWN);
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

        if (mode == RECORD) {
            recordFrame();
        }
    }
    else if (mode == PLAYBACK) {
        if (inputFrames.size() != 0 && inputFrames.front().frame == app.stepCount) {
            InputFrame frame = inputFrames.front();
            mouseDeltaX = frame.mouseDeltaX;
            mouseDeltaY = frame.mouseDeltaY;
            pressed = frame.pressed;
            held = frame.held;
            released = frame.released;
            inputFrames.pop_front();
        }
        else {
            pressed.clear();
            held.clear();
            released.clear();
            mouseDeltaX = 0;
            mouseDeltaY = 0;
        }
    }

    if (glfwGetKey(handle, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        if (mode == RECORD) {
            saveRecording(recordingFile);
        }
        glfwSetWindowShouldClose(handle, true);
    }
    if (glfwGetKey(handle, GLFW_KEY_RIGHT_BRACKET) == GLFW_PRESS) {
        renderRT(400, 200, "screenshot" + to_string(app.stepCount) + ".ppm");
    }
}

void Controls::saveRecording(string filename) {
    ofstream out;
    out.open(filename);
    for (InputFrame frame : inputFrames) {
        out << frame.frame << ";" << hexfloat << frame.mouseDeltaX << ";" << frame.mouseDeltaY << ";";
        for (auto it = frame.pressed.begin(); it != frame.pressed.end(); it++) {
            out << *it;
            if (next(it) != frame.pressed.end()) {
                out << ",";
            }
        }
        out << ";";
        for (auto it = frame.held.begin(); it != frame.held.end(); it++) {
            out << *it;
            if (next(it) != frame.held.end()) {
                out << ",";
            }
        }
        out << ";";
        for (auto it = frame.released.begin(); it != frame.released.end(); it++) {
            out << *it;
            if (next(it) != frame.released.end()) {
                out << ",";
            }
        }
        out << ";" << endl;
    }
    out.close();
}

void Controls::loadRecording(string filename) {
    ifstream in;
    in.open(filename);
    string line;
    while (getline(in, line)) {
        InputFrame frame;
        istringstream iss(line);
        string data;
        iss >> frame.frame;
        iss.ignore();
        iss >> frame.mouseDeltaX;
        iss.ignore();
        iss >> frame.mouseDeltaY;
        iss.ignore();

        int e;
        while (iss >> e) {
            frame.pressed.insert((InputEvent) e);
            if (iss.peek() == ',') {
                iss.ignore();
            }
        }
        iss.clear();
        iss.ignore();
        while (iss >> e) {
            frame.held.insert((InputEvent) e);
            if (iss.peek() == ',') {
                iss.ignore();
            }
        }
        iss.clear();
        iss.ignore();
        while (iss >> e) {
            frame.released.insert((InputEvent) e);
            if (iss.peek() == ',') {
                iss.ignore();
            }
        }

        inputFrames.push_back(frame);
    }
    in.close();
}

void Controls::recordFrame() {
    if (!(mouseDeltaX == 0 && mouseDeltaY == 0 && pressed.size() == 0 && held.size() == 0 && released.size() == 0)) {
        InputFrame frame;
        frame.frame = app.stepCount;
        frame.mouseDeltaX = mouseDeltaX;
        frame.mouseDeltaY = mouseDeltaY;
        frame.pressed = pressed;
        frame.held = held;
        frame.released = released;
        inputFrames.push_back(frame);
    }
}

void Controls::handleEvent(InputEvent e) {
    held.insert(e);
    if (prevHeld.find(e) == prevHeld.end()) {
        pressed.insert(e);
    }
}