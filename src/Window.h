#pragma once
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <array>
#include <glm/glm.hpp>
#include <string>

// GLFW window with an OpenGL 3.3 core context plus polled keyboard/mouse state.
class Window {
public:
    Window(int width, int height, const std::string& title);
    ~Window();
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    GLFWwindow* handle() const { return win_; }
    bool shouldClose() const { return glfwWindowShouldClose(win_); }
    void close() { glfwSetWindowShouldClose(win_, GLFW_TRUE); }
    void pollEvents();  // call once per frame: updates key edges, mouse delta
    void swap() { glfwSwapBuffers(win_); }
    void setVsync(bool on) { glfwSwapInterval(on ? 1 : 0); }
    void setTitle(const std::string& t) { glfwSetWindowTitle(win_, t.c_str()); }
    void framebufferSize(int& w, int& h) const { glfwGetFramebufferSize(win_, &w, &h); }

    bool keyDown(int key) const { return cur_[key]; }
    bool keyPressed(int key) const { return cur_[key] && !prev_[key]; }  // true on the first frame only
    bool mouseDown(int button) const { return glfwGetMouseButton(win_, button) == GLFW_PRESS; }
    glm::vec2 mouseDelta() const { return delta_; }
    float takeScroll();

private:
    static void scrollCallback(GLFWwindow* w, double dx, double dy);
    GLFWwindow* win_ = nullptr;
    std::array<bool, GLFW_KEY_LAST + 1> prev_{}, cur_{};
    glm::vec2 delta_{0.0f};
    double lastX_ = 0.0, lastY_ = 0.0;
    bool first_ = true;
    float scroll_ = 0.0f;
};
