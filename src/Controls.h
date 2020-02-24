#pragma once

#include <unordered_set>
#include <list>
#include <glm/glm.hpp>
#include <string>

class Controls
{
public:
    enum InputEvent {PRIMARY_FIRE, SECONDARY_FIRE, USE, JUMP, FORWARD, BACKWARD, LEFT, RIGHT, SPRINT};
    enum InputMode {NORMAL, RECORD, PLAYBACK};
    struct InputFrame {
        int frame;
        std::unordered_set<Controls::InputEvent> pressed, released, held;
        float mouseDeltaX, mouseDeltaY;
    };

    float mouseSensitivity = 3;
    void update();
    bool isPressed(InputEvent e) { return pressed.find(e) != pressed.end(); }
    bool isHeld(InputEvent e) { return held.find(e) != held.end(); }
    bool isReleased(InputEvent e) { return released.find(e) != released.end(); }
    float getMouseDeltaX() { return mouseDeltaX; }
    float getMouseDeltaY() { return mouseDeltaY; }
    void init(InputMode mode = NORMAL, std::string recordingFile = "");
private:
    void handleEvent(InputEvent e);
    void recordFrame();
    void saveRecording(std::string filename);
    void loadRecording(std::string filename);
    std::unordered_set<InputEvent> pressed;
    std::unordered_set<InputEvent> held;
    std::unordered_set<InputEvent> released;
    std::unordered_set<InputEvent> prevHeld;
    double prevXpos, prevYpos;
    float mouseDeltaX = 0, mouseDeltaY = 0;

    InputMode mode;
    std::string recordingFile;
    std::list<InputFrame> inputFrames;
};