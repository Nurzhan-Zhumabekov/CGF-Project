#include "Camera.h"
#include <algorithm>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera() = default;

static float wrapDegrees(float a) {  // to [-180, 180)
    a = std::fmod(a + 180.0f, 360.0f);
    return a < 0.0f ? a + 180.0f : a - 180.0f;
}

void Camera::setPivot(const glm::vec3& pivot) {
    pivot_ = pivot;
    hasPivot_ = true;
}

void Camera::clearPivot() {
    hasPivot_ = riding_ = false;
    yawGlide_ = 0.0f;
}

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
    clearPivot();
}

void Camera::follow(float distance, float minDistance) {
    if (mode_ == Mode::Free) {
        focus_ = position_ + forward() * distanceTarget_;  // start in front of us, then glide to the target
        distance_ = distanceTarget_;
        mode_ = Mode::Follow;
    }
    minDistance_ = std::clamp(minDistance, 0.2f, kMaxDistance);
    distanceTarget_ = std::clamp(distance, minDistance_, kMaxDistance);
    retarget_ = true;
}

void Camera::update(float dt) {
    if (mode_ != Mode::Follow) return;
    if (retarget_) offset_ = focus_ - target_;  // a new body was chosen: start from where we are looking now
    float keep = std::exp(-6.0f * dt);          // fraction left after dt; the same after 1 s at 30 or 144 FPS

    const glm::vec3 toPivot = pivot_ - target_;
    if (hasPivot_ && (toPivot.x != 0.0f || toPivot.z != 0.0f)) {
        float bearing = glm::degrees(std::atan2(toPivot.z, toPivot.x));  // forward() has this yaw when it points at the pivot
        if (retarget_ || !riding_) yawGlide_ = wrapDegrees(bearing - yaw_);  // first swing round behind the body
        else yaw_ += wrapDegrees(bearing - bearing_);                        // then turn exactly as far as it moved
        bearing_ = bearing;
        riding_ = true;
    }
    retarget_ = false;
    float turn = yawGlide_ * (1.0f - keep);
    yaw_ = wrapDegrees(yaw_ + turn);
    yawGlide_ -= turn;

    offset_ *= keep;
    focus_ = target_ + offset_;
    distance_ = glm::mix(distanceTarget_, distance_, keep);
}

void Camera::look(float dx, float dy) {
    yaw_ += dx * 0.12f;
    pitch_ = std::clamp(pitch_ - dy * 0.12f, -89.0f, 89.0f);
}

void Camera::zoom(float steps) {
    distanceTarget_ = std::clamp(distanceTarget_ * std::pow(0.9f, steps), minDistance_, kMaxDistance);
}

void Camera::move(const glm::vec3& axes, float dt, bool fast) {
    if (mode_ != Mode::Free) return;
    glm::vec3 f = forward(), r = glm::normalize(glm::cross(f, glm::vec3(0, 1, 0)));
    float speed = (fast ? 40.0f : 12.0f) * dt;
    position_ += (r * axes.x + glm::vec3(0, 1, 0) * axes.y + f * axes.z) * speed;
}

glm::mat4 Camera::view() const { return glm::lookAt(position(), position() + forward(), glm::vec3(0, 1, 0)); }

glm::mat4 Camera::projection(float aspect) const { return glm::perspective(glm::radians(45.0f), aspect, 0.1f, 600.0f); }
