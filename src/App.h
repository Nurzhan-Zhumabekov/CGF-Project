#pragma once
#include "Camera.h"
#include "Mesh.h"
#include "SaturnRings.h"
#include "Shader.h"
#include "Skybox.h"
#include "SolarSystem.h"
#include "Texture.h"
#include "UI.h"
#include "Window.h"
#include <map>
#include <string>

// Owns the window, scene, camera and renderer, and runs the main loop.
class App {
public:
    App();
    void run();

private:
    void handleInput(float dt);
    void focusOn(SceneNode* node);
    void render();
    void drawBodies();
    void drawOrbitLines(const glm::mat4& view, const glm::mat4& proj);
    const Texture& texture(const std::string& name) const { return textures_.at(name); }

    Window window_;
    SolarSystem system_;
    Camera camera_;
    UIState state_;
    Shader planetShader_, lineShader_;
    Mesh sphere_, circle_;
    Skybox skybox_;
    SaturnRings rings_;
    std::map<std::string, Texture> textures_;
    UI ui_;

    SceneNode* followed_ = nullptr;
    float simDays_ = 0.0f;
    float fps_ = 0.0f;
};
