#pragma once

#include <unordered_set>
#include <glm/glm.hpp>

class Controls
{
public:
    enum InputEvent {PRIMARY_FIRE, SECONDARY_FIRE, USE, JUMP, FORWARD, BACKWARD, LEFT, RIGHT};
    float mouseSensitivity = 3;
    void update();
    bool isPressed(InputEvent e) { return pressed.find(e) != pressed.end(); }
    bool isHeld(InputEvent e) { return held.find(e) != held.end(); }
    bool isReleased(InputEvent e) { return released.find(e) != released.end(); }
    float getMouseDeltaX() { return mouseDeltaX; }
    float getMouseDeltaY() { return mouseDeltaY; }
    void init();
private:
    void handleEvent(InputEvent e);
    std::unordered_set<InputEvent> pressed;
    std::unordered_set<InputEvent> held;
    std::unordered_set<InputEvent> released;
    std::unordered_set<InputEvent> prevHeld;
    double prevXpos, prevYpos;
    float mouseDeltaX = 0, mouseDeltaY = 0;
};