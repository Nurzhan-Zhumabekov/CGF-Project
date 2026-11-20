#include "AsteroidBelt.h"
#include "AsteroidBeltData.h"
#include <algorithm>
#include <cstddef>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

// Attribute locations 0..2 are position, normal and UV (see Mesh); the per-instance ones follow.
static const GLuint kModelLocation = 3;   // mat4: takes 3, 4, 5 and 6
static const GLuint kTumbleLocation = 7;  // vec4
static const GLuint kTintLocation = 8;    // vec3

// The buffer is read as raw floats, so the struct must not contain padding.
static_assert(sizeof(AsteroidInstance) == 23 * sizeof(float), "AsteroidInstance must be 23 packed floats");

AsteroidBelt::AsteroidBelt(const SceneNode& inner, const SceneNode& outer)
    : shader_("shaders/asteroid.vert", "shaders/planet.frag"),  // same Blinn-Phong lighting as the planets
      rock_(makeRock(6, 10, 7u)) {                               // 100 triangles: enough for a rock a few pixels wide
    const std::vector<AsteroidInstance> asteroids = makeAsteroids(beltBetween(
        inner.params().orbitRadius, inner.params().radius, outer.params().orbitRadius, outer.params().radius));
    capacity_ = int(asteroids.size());

    glGenBuffers(1, &instanceVbo_);
    glBindVertexArray(rock_.vao());  // the instance attributes are stored in the rock's VAO
    glBindBuffer(GL_ARRAY_BUFFER, instanceVbo_);
    glBufferData(GL_ARRAY_BUFFER, GLsizeiptr(asteroids.size() * sizeof(AsteroidInstance)), asteroids.data(),
                 GL_STATIC_DRAW);
    auto instanced = [](GLuint location, GLint components, size_t offset) {
        glEnableVertexAttribArray(location);
        glVertexAttribPointer(location, components, GL_FLOAT, GL_FALSE, sizeof(AsteroidInstance),
                              reinterpret_cast<const void*>(offset));
        glVertexAttribDivisor(location, 1);  // next value for every instance, not for every vertex
    };
    // A vertex attribute holds at most a vec4, so a mat4 takes four consecutive locations, one per column.
    for (GLuint column = 0; column < 4; ++column)
        instanced(kModelLocation + column, 4, offsetof(AsteroidInstance, model) + column * sizeof(glm::vec4));
    instanced(kTumbleLocation, 4, offsetof(AsteroidInstance, tumble));
    instanced(kTintLocation, 3, offsetof(AsteroidInstance, tint));
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

AsteroidBelt::~AsteroidBelt() { glDeleteBuffers(1, &instanceVbo_); }

void AsteroidBelt::draw(int count, double days, const glm::mat4& view, const glm::mat4& projection,
                        const glm::vec3& viewPos, const SceneLights& lights, const Texture& surface) const {
    count = std::clamp(count, 0, capacity_);
    if (count == 0) return;

    // Belt frame: centred on the Sun and rotated about +Y, counter-clockwise like the planets.
    const glm::mat4 belt =
        glm::rotate(glm::translate(glm::mat4(1.0f), lights.sunPos), beltAngle(days), glm::vec3(0, 1, 0));
    shader_.use();
    shader_.set("uBelt", belt);
    shader_.set("uTumble", beltTumble(days));
    shader_.set("uView", view);
    shader_.set("uProj", projection);
    shader_.set("uViewPos", viewPos);
    lights.apply(shader_);
    shader_.set("uEmissive", 0);
    shader_.set("uUseNight", 0);
    shader_.set("uUseNormalMap", 0);
    shader_.set("uShininess", 8.0f);       // rough rock: a wide, weak highlight
    shader_.set("uSpecStrength", 0.04f);
    shader_.set("uDiffuse", 0);
    shader_.set("uNight", 1);
    surface.bind(0);
    rock_.drawInstanced(count);
}
