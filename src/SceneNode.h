#pragma once
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

// Physical-ish description of one body. Distances and radii are stylized, periods are in days.
struct BodyParams {
    std::string name;
    float radius = 1.0f;        // visual radius
    float orbitRadius = 0.0f;   // distance from the parent
    float orbitPeriod = 0.0f;   // days per revolution around the parent (0 = does not orbit)
    float spinPeriod = 0.0f;    // days per rotation about its own axis (negative = retrograde)
    float tiltDeg = 0.0f;       // axial tilt
};

// One node of the scene graph. A child orbits the centre of its parent (Earth -> Moon).
class SceneNode {
public:
    explicit SceneNode(BodyParams params, SceneNode* parent = nullptr);

    SceneNode* addChild(BodyParams params);
    // Recomputes frame/model for this node and all descendants at simulation time `days`.
    void update(float days);

    const glm::mat4& model() const { return model_; }  // includes scale, spin and tilt
    const glm::mat4& frame() const { return frame_; }  // orbit frame: translation = body centre
    glm::mat4 parentFrame() const { return parent_ ? parent_->frame_ : glm::mat4(1.0f); }
    glm::vec3 position() const { return glm::vec3(frame_[3]); }
    const BodyParams& params() const { return params_; }
    SceneNode* parent() const { return parent_; }
    const std::vector<std::unique_ptr<SceneNode>>& children() const { return children_; }

private:
    BodyParams params_;
    SceneNode* parent_;
    std::vector<std::unique_ptr<SceneNode>> children_;
    glm::mat4 frame_{1.0f};
    glm::mat4 model_{1.0f};
};
