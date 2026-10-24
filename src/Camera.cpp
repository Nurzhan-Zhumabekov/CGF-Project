#include "Camera.h"
#include <algorithm>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera() = default;

glm::vec3 Camera::forward() const {
    float y = glm::radians(yaw_), p = glm::radians(pitch_);
    return glm::normalize(glm::vec3(std::cos(y) * std::cos(p), std::sin(p), std::sin(y) * std::cos(p)));
}

glm::vec3 Camera::position() const {
    return mode_ == Mode::Free ? position_ : focus_ - forward() * distance_;
}

void Camera::setFree() {
    if (mode_ == Mode::Free) return;
    position_ = position();
    mode_ = Mode::Free;
}

void Camera::follow(float distance) {
    if (mode_ == Mode::Free) {
        focus_ = position_ + forward() * distanceTarget_;  // start in front of us, then glide to the target
        distance_ = distanceTarget_;
        mode_ = Mode::Follow;
    }
    distanceTarget_ = distance;
}

void Camera::update(float dt) {
    if (mode_ != Mode::Follow) return;
    float a = 1.0f - std::exp(-6.0f * dt);
    focus_ = glm::mix(focus_, target_, a);
    distance_ = glm::mix(distance_, distanceTarget_, a);
}

void Camera::look(float dx, float dy) {
    yaw_ += dx * 0.12f;
    pitch_ = std::clamp(pitch_ - dy * 0.12f, -89.0f, 89.0f);
}

void Camera::zoom(float steps) {
    distanceTarget_ = std::clamp(distanceTarget_ * std::pow(0.9f, steps), 1.0f, 150.0f);
}

void Camera::move(const glm::vec3& axes, float dt, bool fast) {
    if (mode_ != Mode::Free) return;
    glm::vec3 f = forward(), r = glm::normalize(glm::cross(f, glm::vec3(0, 1, 0)));
    float speed = (fast ? 40.0f : 12.0f) * dt;
    position_ += (r * axes.x + glm::vec3(0, 1, 0) * axes.y + f * axes.z) * speed;
}

glm::mat4 Camera::view() const { return glm::lookAt(position(), position() + forward(), glm::vec3(0, 1, 0)); }

glm::mat4 Camera::projection(float aspect) const { return glm::perspective(glm::radians(45.0f), aspect, 0.1f, 600.0f); }
