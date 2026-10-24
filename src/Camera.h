#pragma once
#include <glm/glm.hpp>

// Free-fly camera (WASD + mouse) and follow camera that orbits a moving target.
// Yaw/pitch always describe the viewing direction, so mouse look behaves the same in both modes.
class Camera {
public:
    enum class Mode { Free, Follow };

    Camera();
    void setFree();
    void follow(float distance);              // switch to follow mode, keep the view direction
    void setTarget(const glm::vec3& target) { target_ = target; }
    void update(float dt);                    // smooths focus point and distance (frame-rate independent)
    void look(float dxPixels, float dyPixels);
    void zoom(float scrollSteps);
    void move(const glm::vec3& axes, float dt, bool fast);  // x = right, y = up, z = forward

    glm::mat4 view() const;
    glm::mat4 projection(float aspect) const;
    glm::vec3 position() const;
    glm::vec3 forward() const;
    Mode mode() const { return mode_; }

private:
    Mode mode_ = Mode::Free;
    glm::vec3 position_{0.0f, 18.0f, 60.0f};
    glm::vec3 focus_{0.0f}, target_{0.0f};
    float yaw_ = -90.0f, pitch_ = -15.0f;
    float distance_ = 10.0f, distanceTarget_ = 10.0f;
};
