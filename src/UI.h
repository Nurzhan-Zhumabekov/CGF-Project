#pragma once
#include <string>

struct GLFWwindow;

// State shared between the ImGui panel and the simulation.
struct UIState {
    bool paused = false;
    bool fillLight = true;     // second light (lantern at the camera)
    bool orbitLines = true;
    bool vsync = true;
    float timeScale = 30.0f;   // simulated days per real second
    bool showHelp = true;
};

class UI {
public:
    explicit UI(GLFWwindow* window);
    ~UI();
    void draw(UIState& s, float fps, const std::string& cameraInfo);  // builds and renders one frame
};
