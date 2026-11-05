#pragma once
#include "Mesh.h"
#include "Shader.h"
#include "Texture.h"

// Star-field cube drawn before transparency with LEQUAL and no depth writes; view has no translation.
class Skybox {
public:
    Skybox();
    void draw(const glm::mat4& view, const glm::mat4& projection) const;

private:
    Cubemap cubemap_;
    Mesh cube_;
    Shader shader_;
};
