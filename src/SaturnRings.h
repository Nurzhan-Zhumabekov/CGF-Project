#pragma once
#include "Mesh.h"
#include "SceneNode.h"
#include "Shader.h"
#include "Texture.h"

// A two-sided transparent annulus, drawn after opaque bodies and the skybox.
class SaturnRings {
public:
    SaturnRings();
    void draw(const SceneNode& saturn, const glm::mat4& view, const glm::mat4& projection,
              const glm::vec3& sunPos, const glm::vec3& viewPos, bool fillLight) const;

private:
    Shader shader_;
    Mesh mesh_;
    Texture texture_;
};
