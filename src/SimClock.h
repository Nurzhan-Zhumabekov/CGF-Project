#pragma once
#include <string>

// Simulation time in days since J2000 (1 January 2000, 12:00), the epoch of the orbit phases.
//
//   days += dt * scale        once per frame, skipped while paused
//   angle = 2π * days / period  computed fresh from `days` for every body (SceneNode)
//
// Angles are never accumulated frame by frame, so changing the speed or pausing cannot make a
// planet jump: the speed only changes how fast `days` grows, not where a planet is at a given day.
//
// `days` is a double on purpose. A float has 24 bits of precision: after 30 s at the top speed
// (60 000 days) the gap between neighbouring floats is 0.004 days, larger than one frame's step at
// the lowest speed (0.0017 days), so slowing down would freeze the whole scene.
class SimClock {
public:
    static constexpr float kMinScale = 0.1f;       // simulated days per real second
    static constexpr float kMaxScale = 2000.0f;
    static constexpr float kDefaultScale = 30.0f;

    void advance(float dtSeconds);                 // no-op while paused or for dt <= 0
    void faster() { setScale(scale_ * 2.0f); }     // key ]
    void slower() { setScale(scale_ * 0.5f); }     // key [
    void setScale(float daysPerSecond);            // clamped to [kMinScale, kMaxScale]
    void togglePause() { paused_ = !paused_; }     // Space
    void setPaused(bool paused) { paused_ = paused; }
    void setDays(double days) { days_ = days; }

    double days() const { return days_; }
    float scale() const { return scale_; }
    bool paused() const { return paused_; }
    std::string date() const;                      // calendar date of `days`, e.g. "2000-01-01"

private:
    double days_ = 0.0;
    float scale_ = kDefaultScale;
    bool paused_ = false;
};
