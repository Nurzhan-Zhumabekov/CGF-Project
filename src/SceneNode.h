#pragma once
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

// Description of one body. Radii and distances are stylized; periods are in days.
struct BodyParams {
    std::string name;
    float radius = 1.0f;          // visual radius
    float orbitRadius = 0.0f;     // distance from the parent's centre
    float orbitPeriod = 0.0f;     // days per revolution around the parent (0 = does not orbit)
    float spinPeriod = 0.0f;      // days per rotation about its own axis (negative = retrograde)
    float tiltDeg = 0.0f;         // axial tilt
    float orbitPhaseDeg = 0.0f;   // starting angle on the orbit, so the planets do not start in a line
};

// One node of the scene graph. A child orbits the centre of its parent (Sun -> Earth -> Moon).
//
// Each node has two matrices relative to its parent:
//   local = T(orbit position)                    where the body's centre is
//   body  = R(tilt) * R(spin) * S(radius)        how the body is drawn around that centre
// and the world matrices follow the usual scene-graph rule:
//   world = parent.world * local                 passed down to the children
//   model = world * body                         passed to the shader
// Keeping rotation and scale out of `local` means a child is not scaled by its parent, does not
// inherit the parent's spin, and an axial tilt keeps pointing the same way all year (seasons).
class SceneNode {
public:
    explicit SceneNode(BodyParams params, SceneNode* parent = nullptr);

    SceneNode* addChild(BodyParams params);
    // Recomputes the matrices of this node and all descendants at simulation time `days`.
    void update(float days);

    float orbitAngle(float days) const;  // radians, includes the starting phase
    float spinAngle(float days) const;   // radians

    const glm::mat4& localMatrix() const { return local_; }
    const glm::mat4& worldMatrix() const { return world_; }
    const glm::mat4& model() const { return model_; }
    const glm::mat4& frame() const { return world_; }  // same as worldMatrix(), kept for existing callers
    glm::mat4 parentFrame() const { return parent_ ? parent_->world_ : glm::mat4(1.0f); }
    glm::vec3 position() const { return glm::vec3(world_[3]); }
    glm::vec3 spinAxis() const { return glm::normalize(glm::vec3(model_[1])); }  // world-space north pole

    const BodyParams& params() const { return params_; }
    SceneNode* parent() const { return parent_; }
    const std::vector<std::unique_ptr<SceneNode>>& children() const { return children_; }

private:
    BodyParams params_;
    SceneNode* parent_;
    std::vector<std::unique_ptr<SceneNode>> children_;
    glm::mat4 local_{1.0f};
    glm::mat4 world_{1.0f};
    glm::mat4 model_{1.0f};
};
