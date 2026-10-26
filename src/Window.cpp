#include "Window.h"
#include <stdexcept>

Window::Window(int width, int height, const std::string& title) {
    if (!glfwInit()) throw std::runtime_error("glfwInit failed");
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    glfwWindowHint(GLFW_SAMPLES, 4);
    win_ = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!win_) {
        glfwTerminate();
        throw std::runtime_error("cannot create a window with an OpenGL 3.3 core context");
    }
    // Keep short key taps until pollEvents reads them, even if released between frames.
    glfwSetInputMode(win_, GLFW_STICKY_KEYS, GLFW_TRUE);
    glfwMakeContextCurrent(win_);
    if (!gladLoadGL(glfwGetProcAddress)) throw std::runtime_error("gladLoadGL failed");
    glfwSetWindowUserPointer(win_, this);
    glfwSetScrollCallback(win_, scrollCallback);  // set before ImGui so it chains to us
    glfwSwapInterval(1);
}

Window::~Window() {
    if (win_) glfwDestroyWindow(win_);
    glfwTerminate();
}

void Window::scrollCallback(GLFWwindow* w, double, double dy) {
    static_cast<Window*>(glfwGetWindowUserPointer(w))->scroll_ += float(dy);
}

float Window::takeScroll() {
    float s = scroll_;
    scroll_ = 0.0f;
    return s;
}

void Window::pollEvents() {
    prev_ = cur_;
    glfwPollEvents();
    for (int k = GLFW_KEY_SPACE; k <= GLFW_KEY_LAST; ++k) cur_[k] = glfwGetKey(win_, k) == GLFW_PRESS;
    double x, y;
    glfwGetCursorPos(win_, &x, &y);
    delta_ = first_ ? glm::vec2(0.0f) : glm::vec2(float(x - lastX_), float(y - lastY_));
    lastX_ = x;
    lastY_ = y;
    first_ = false;
}
