#include "SceneNode.h"
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

static const float kTwoPi = 6.28318530717959f;

SceneNode::SceneNode(BodyParams params, SceneNode* parent) : params_(std::move(params)), parent_(parent) {}

SceneNode* SceneNode::addChild(BodyParams params) {
    children_.push_back(std::make_unique<SceneNode>(std::move(params), this));
    return children_.back().get();
}

// Fraction of a full turn completed after `days`, in [0, 1). Computed in double and wrapped before
// converting to float: 2π * days / period itself can reach millions of radians, where a float can
// only step in quarter-radian increments and a fast-spinning planet would visibly stutter.
static float turnFraction(double days, float period) {
    double turns = std::fmod(days / double(period), 1.0);
    return float(turns < 0.0 ? turns + 1.0 : turns);  // negative period = retrograde, same rotation
}

// angle = phase + 2π * days / period
float SceneNode::orbitAngle(double days) const {
    float phase = glm::radians(params_.orbitPhaseDeg);
    return params_.orbitPeriod > 0.0f ? phase + kTwoPi * turnFraction(days, params_.orbitPeriod) : phase;
}

float SceneNode::spinAngle(double days) const {
    // Starting the spin at the orbit phase makes a tidally locked body (spin period = orbit period,
    // like the Moon) face its parent from the first frame.
    float phase = glm::radians(params_.orbitPhaseDeg);
    return params_.spinPeriod != 0.0f ? phase + kTwoPi * turnFraction(days, params_.spinPeriod) : phase;
}

// Spin drawn on screen. Without a frame time it is exactly spinAngle(days). With one, the turn made
// since the previous frame is slowed when it is fast:
//   shown rate = rate / (1 + |rate| / kMaxShownSpinTurnsPerSecond)      (turns per real second)
// Slow spins stay almost exact, fast ones keep their direction and order but never exceed the limit,
// so nothing turns far enough in one frame to flicker. Pausing (rate 0) still stops every body.
// A tidally locked body (spin period = orbit period, the Moon) keeps the exact angle, so it goes on
// facing its parent at any speed.
float SceneNode::shownSpinAngle(double days, double dtSeconds) {
    const double period = params_.spinPeriod;
    const bool locked = params_.spinPeriod == params_.orbitPeriod;
    if (period == 0.0 || locked || dtSeconds <= 0.0 || !spinStarted_) {
        spinTurns_ = turnFraction(days, params_.spinPeriod == 0.0f ? 1.0f : params_.spinPeriod);
    } else {
        double step = (days - lastDays_) / period;  // turns since the previous frame
        double rate = step / dtSeconds;
        step /= 1.0 + std::fabs(rate) / kMaxShownSpinTurnsPerSecond;
        spinTurns_ = std::fmod(spinTurns_ + step, 1.0);
        if (spinTurns_ < 0.0) spinTurns_ += 1.0;
    }
    spinStarted_ = true;
    lastDays_ = days;
    const float phase = glm::radians(params_.orbitPhaseDeg);
    return period == 0.0 ? phase : phase + kTwoPi * float(spinTurns_);
}

void SceneNode::update(double days, double dtSeconds) {
    const glm::mat4 I(1.0f);
    const glm::vec3 up(0, 1, 0), forward(0, 0, 1);

    // Orbit: the point at angle a on a circle of radius r in the parent's XZ plane (counter-clockwise
    // seen from +Y). Rotating (r, 0, 0) about +Y by a gives (r cos a, 0, -r sin a).
    float a = orbitAngle(days), r = params_.orbitRadius;
    local_ = glm::translate(I, glm::vec3(r * std::cos(a), 0.0f, -r * std::sin(a)));
    world_ = parentFrame() * local_;

    // Body: tilt the axis (fixed in space), spin about the tilted axis, then scale to the radius.
    glm::mat4 body = glm::rotate(I, glm::radians(params_.tiltDeg), forward) *
                     glm::rotate(I, shownSpinAngle(days, dtSeconds), up) *
                     glm::scale(I, glm::vec3(params_.radius));
    model_ = world_ * body;

    for (auto& c : children_) c->update(days, dtSeconds);
}
