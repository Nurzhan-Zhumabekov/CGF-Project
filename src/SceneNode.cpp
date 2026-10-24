#include "SceneNode.h"
#include <glm/gtc/matrix_transform.hpp>

static const float kTwoPi = 6.28318530717959f;

SceneNode::SceneNode(BodyParams params, SceneNode* parent) : params_(std::move(params)), parent_(parent) {}

SceneNode* SceneNode::addChild(BodyParams params) {
    children_.push_back(std::make_unique<SceneNode>(std::move(params), this));
    return children_.back().get();
}

void SceneNode::update(float days) {
    const glm::vec3 up(0, 1, 0), forward(0, 0, 1);
    float orbitAngle = params_.orbitPeriod > 0.0f ? kTwoPi * days / params_.orbitPeriod : 0.0f;
    float spinAngle = params_.spinPeriod != 0.0f ? kTwoPi * days / params_.spinPeriod : 0.0f;

    // Orbit: rotate around the parent's centre, then push out along +X.
    frame_ = parentFrame() * glm::rotate(glm::mat4(1.0f), orbitAngle, up) *
             glm::translate(glm::mat4(1.0f), glm::vec3(params_.orbitRadius, 0, 0));
    // Body: tilt the axis, spin about it, then scale. Scale stays out of `frame_`, so children are unscaled.
    model_ = frame_ * glm::rotate(glm::mat4(1.0f), glm::radians(params_.tiltDeg), forward) *
             glm::rotate(glm::mat4(1.0f), spinAngle, up) *
             glm::scale(glm::mat4(1.0f), glm::vec3(params_.radius));

    for (auto& c : children_) c->update(days);
}
