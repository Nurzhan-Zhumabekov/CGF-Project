#pragma once
#include "Shader.h"

// The two lights of the scene (R4). Filled in once per frame and sent to every lit shader through
// apply(), so the planets, Saturn's rings and the asteroid belt always use the same values.
struct SceneLights {
    glm::vec3 sunPos{0.0f};                    // light 1: point light in the Sun
    glm::vec3 sunColor{1.0f, 0.96f, 0.88f};
    glm::vec3 fillPos{0.0f};                   // light 2: lantern at the camera, toggled with L
    glm::vec3 fillColor{0.35f, 0.45f, 0.65f};
    bool fillOn = true;

    void apply(const Shader& shader) const {   // the shader must be in use
        shader.set("uSunPos", sunPos);
        shader.set("uSunColor", sunColor);
        shader.set("uFillPos", fillPos);
        shader.set("uFillColor", fillColor);
        shader.set("uFillOn", int(fillOn));
    }
};
