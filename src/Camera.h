#pragma once
#include <glm/glm.hpp>

// Free-fly camera (WASD + mouse) and follow camera that orbits a moving target.
// Yaw/pitch always describe the viewing direction, so mouse look behaves the same in both modes.
//
// Follow mode keeps the look-at point at
//   focus = target + offset
// where `offset` is the leftover from the previous view and decays as offset *= exp(-k * dt).
// Only the offset is smoothed, never the target itself, so the camera glides to a newly chosen body
// but then stays locked on it at any time speed. Easing the focus towards a moving target instead
// would trail it by speed / k, which is several units for the Moon at the default speed.
//
// A satellite also gets a pivot, the centre of its parent. Each frame the yaw turns by the angle the
// satellite moved around the pivot, so the camera rides along the orbit behind the body: the parent
// stays in view behind it instead of sweeping through the frame and hiding it once per orbit.
class Camera {
public:
    enum class Mode { Free, Follow };

    Camera();
    void setFree();
    // Switch to follow mode (or to another body), keeping the view direction.
    // The wheel cannot bring the camera closer than `minDistance`, so it never ends up inside the body.
    void follow(float distance, float minDistance = 1.0f);
    void setTarget(const glm::vec3& target) { target_ = target; }  // call every frame with the body's world position
    void setPivot(const glm::vec3& pivot);    // call every frame for a satellite, with its parent's world position
    void clearPivot();                        // the followed body orbits the Sun: keep the view direction fixed
    void update(float dt);                    // decays the offset and eases the distance (frame-rate independent)
    void look(float dxPixels, float dyPixels);
    void zoom(float scrollSteps);
    void move(const glm::vec3& axes, float dt, bool fast);  // x = right, y = up, z = forward

    glm::mat4 view() const;
    glm::mat4 projection(float aspect) const;
    glm::vec3 position() const;
    glm::vec3 forward() const;
    Mode mode() const { return mode_; }

private:
    static constexpr float kMaxDistance = 150.0f;

    Mode mode_ = Mode::Free;
    glm::vec3 position_{0.0f, 18.0f, 60.0f};
    glm::vec3 focus_{0.0f}, target_{0.0f}, offset_{0.0f};
    float yaw_ = -90.0f, pitch_ = -15.0f;
    float distance_ = 10.0f, distanceTarget_ = 10.0f, minDistance_ = 1.0f;
    bool retarget_ = false;  // follow() was called: take a fresh offset on the next update()

    glm::vec3 pivot_{0.0f};
    bool hasPivot_ = false, riding_ = false;
    float bearing_ = 0.0f;   // yaw (degrees) that looked from the body at its parent on the previous frame
    float yawGlide_ = 0.0f;  // degrees still to turn to get behind the body after it was chosen
};
