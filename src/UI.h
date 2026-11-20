#pragma once
#include "AsteroidBeltData.h"
#include <string>

class SimClock;

struct GLFWwindow;

// State shared between the ImGui panel and the simulation.
struct UIState {
    bool fillLight = true;     // second light (lantern at the camera)
    bool orbitLines = true;
    bool asteroids = true;     // asteroid belt (instanced)
    int asteroidCount = kDefaultAsteroids;  // how many rocks of the belt are drawn
    bool vsync = true;
    bool normalMap = true;     // Earth relief (normal mapping)
    bool showHelp = true;
};

class UI {
public:
    explicit UI(GLFWwindow* window);
    ~UI();
    void draw(UIState& s, SimClock& clock, float fps, const std::string& cameraInfo);  // builds and renders one frame
};
