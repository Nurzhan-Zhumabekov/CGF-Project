#pragma once
#include "Lights.h"
#include "Mesh.h"
#include "SceneNode.h"
#include "Shader.h"
#include "Texture.h"

// Asteroid belt drawn with instancing: one rock mesh, one draw call for thousands of rocks.
//
// The model matrix, spin and tint of every asteroid are computed once at start-up and stored in a
// static vertex buffer. glVertexAttribDivisor(location, 1) makes the GPU step through that buffer once per
// instance instead of once per vertex, so each copy of the rock gets its own values. Nothing is
// uploaded per frame: the orbital motion and the tumbling are two uniforms.
class AsteroidBelt {
public:
    // The belt fills the gap between the orbits of `inner` and `outer` (Mars and Jupiter).
    AsteroidBelt(const SceneNode& inner, const SceneNode& outer);
    ~AsteroidBelt();
    AsteroidBelt(const AsteroidBelt&) = delete;
    AsteroidBelt& operator=(const AsteroidBelt&) = delete;

    // Draws the first `count` asteroids (clamped to capacity()) around the Sun at time `days`.
    void draw(int count, double days, const glm::mat4& view, const glm::mat4& projection, const glm::vec3& viewPos,
              const SceneLights& lights, const Texture& surface) const;
    int capacity() const { return capacity_; }

private:
    Shader shader_;
    Mesh rock_;
    GLuint instanceVbo_ = 0;
    int capacity_ = 0;
};
