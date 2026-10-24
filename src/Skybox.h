#pragma once
#include "Mesh.h"
#include "Shader.h"
#include "Texture.h"

// Star-field cube drawn last with depth func LEQUAL; the view matrix loses its translation.
class Skybox {
public:
    Skybox();
    void draw(const glm::mat4& view, const glm::mat4& projection) const;

private:
    Cubemap cubemap_;
    Mesh cube_;
    Shader shader_;
};
