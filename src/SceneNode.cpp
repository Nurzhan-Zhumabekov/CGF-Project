#include "SceneNode.h"
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

static const float kTwoPi = 6.28318530717959f;

SceneNode::SceneNode(BodyParams params, SceneNode* parent) : params_(std::move(params)), parent_(parent) {}

SceneNode* SceneNode::addChild(BodyParams params) {
    children_.push_back(std::make_unique<SceneNode>(std::move(params), this));
    return children_.back().get();
}

float SceneNode::orbitAngle(float days) const {
    float phase = glm::radians(params_.orbitPhaseDeg);
    return params_.orbitPeriod > 0.0f ? phase + kTwoPi * days / params_.orbitPeriod : phase;
}

float SceneNode::spinAngle(float days) const {
    // Starting the spin at the orbit phase makes a tidally locked body (spin period = orbit period,
    // like the Moon) face its parent from the first frame.
    float phase = glm::radians(params_.orbitPhaseDeg);
    return params_.spinPeriod != 0.0f ? phase + kTwoPi * days / params_.spinPeriod : phase;
}

void SceneNode::update(float days) {
    const glm::mat4 I(1.0f);
    const glm::vec3 up(0, 1, 0), forward(0, 0, 1);

    // Orbit: the point at angle a on a circle of radius r in the parent's XZ plane (counter-clockwise
    // seen from +Y). Rotating (r, 0, 0) about +Y by a gives (r cos a, 0, -r sin a).
    float a = orbitAngle(days), r = params_.orbitRadius;
    local_ = glm::translate(I, glm::vec3(r * std::cos(a), 0.0f, -r * std::sin(a)));
    world_ = parentFrame() * local_;

    // Body: tilt the axis (fixed in space), spin about the tilted axis, then scale to the radius.
    glm::mat4 body = glm::rotate(I, glm::radians(params_.tiltDeg), forward) *
                     glm::rotate(I, spinAngle(days), up) *
                     glm::scale(I, glm::vec3(params_.radius));
    model_ = world_ * body;

    for (auto& c : children_) c->update(days);
}
