#include "App.h"
#include "Primitives.h"
#include <algorithm>
#include <cctype>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>

namespace {
std::string lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
    return s;
}
}  // namespace

App::App()
    : window_(1280, 720, "Solar System Explorer"),
      planetShader_("shaders/planet.vert", "shaders/planet.frag"),
      lineShader_("shaders/line.vert", "shaders/line.frag"),
      sphere_(makeSphere(48, 96)),
      circle_(makeCircle(180), GL_LINE_LOOP),
      ui_(window_.handle()) {
    system_.forEach([this](SceneNode& n) {
        std::string key = lower(n.params().name);
        textures_.emplace(key, Texture("assets/textures/" + key + ".png"));
    });
    textures_.emplace("earth_night", Texture("assets/textures/earth_night.png"));
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_MULTISAMPLE);
    system_.update(simDays_);
}

void App::focusOn(SceneNode* node) {
    followed_ = node;
    camera_.follow(node->params().radius * 4.0f + 3.0f);
}

void App::handleInput(float dt) {
    ImGuiIO& io = ImGui::GetIO();
    if (window_.keyPressed(GLFW_KEY_ESCAPE)) window_.close();

    // Scene interactions (R6): focus a planet, change time, toggle light / orbit lines / pause.
    for (int i = 0; i < system_.planetCount(); ++i)
        if (window_.keyPressed(GLFW_KEY_1 + i)) focusOn(&system_.planet(i));
    if (window_.keyPressed(GLFW_KEY_9)) focusOn(&system_.moon());
    if (window_.keyPressed(GLFW_KEY_0) || window_.keyPressed(GLFW_KEY_F)) {
        followed_ = nullptr;
        camera_.setFree();
    }
    if (window_.keyPressed(GLFW_KEY_SPACE)) state_.paused = !state_.paused;
    if (window_.keyPressed(GLFW_KEY_L)) state_.fillLight = !state_.fillLight;
    if (window_.keyPressed(GLFW_KEY_O)) state_.orbitLines = !state_.orbitLines;
    if (window_.keyPressed(GLFW_KEY_H)) state_.showHelp = !state_.showHelp;
    if (window_.keyPressed(GLFW_KEY_RIGHT_BRACKET)) state_.timeScale = std::clamp(state_.timeScale * 2.0f, 0.1f, 2000.0f);
    if (window_.keyPressed(GLFW_KEY_LEFT_BRACKET)) state_.timeScale = std::clamp(state_.timeScale * 0.5f, 0.1f, 2000.0f);

    // Camera: speeds scale with dt, so motion is the same at any frame rate (C1).
    if (!io.WantCaptureMouse) {
        if (window_.mouseDown(GLFW_MOUSE_BUTTON_RIGHT)) camera_.look(window_.mouseDelta().x, window_.mouseDelta().y);
        camera_.zoom(window_.takeScroll());
    } else {
        window_.takeScroll();
    }
    glm::vec3 axes(0.0f);
    axes.z = float(window_.keyDown(GLFW_KEY_W)) - float(window_.keyDown(GLFW_KEY_S));
    axes.x = float(window_.keyDown(GLFW_KEY_D)) - float(window_.keyDown(GLFW_KEY_A));
    axes.y = float(window_.keyDown(GLFW_KEY_E)) - float(window_.keyDown(GLFW_KEY_Q));
    camera_.move(axes, dt, window_.keyDown(GLFW_KEY_LEFT_SHIFT));

    if (!state_.paused) simDays_ += dt * state_.timeScale;
    system_.update(simDays_);
    if (followed_) camera_.setTarget(followed_->position());
    camera_.update(dt);
}

void App::drawBodies() {
    planetShader_.set("uDiffuse", 0);
    planetShader_.set("uNight", 1);
    system_.forEach([this](SceneNode& n) {
        const std::string name = n.params().name;
        const bool earth = name == "Earth";
        texture(lower(name)).bind(0);
        if (earth) texture("earth_night").bind(1);
        planetShader_.set("uEmissive", int(name == "Sun"));
        planetShader_.set("uUseNight", int(earth));
        planetShader_.set("uShininess", earth ? 48.0f : 16.0f);
        planetShader_.set("uSpecStrength", earth ? 0.5f : 0.08f);
        planetShader_.set("uModel", n.model());
        planetShader_.set("uNormalMat", glm::mat3(glm::transpose(glm::inverse(n.model()))));
        sphere_.draw();
    });
}

void App::drawOrbitLines(const glm::mat4& view, const glm::mat4& proj) {
    lineShader_.use();
    lineShader_.set("uColor", glm::vec3(0.30f, 0.36f, 0.48f));
    system_.forEach([&](SceneNode& n) {
        if (!n.parent()) return;
        glm::mat4 model = n.parentFrame() * glm::scale(glm::mat4(1.0f), glm::vec3(n.params().orbitRadius));
        lineShader_.set("uMVP", proj * view * model);
        circle_.draw();
    });
}

void App::render() {
    int w, h;
    window_.framebufferSize(w, h);
    glViewport(0, 0, w, h);
    glClearColor(0.0f, 0.0f, 0.02f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 view = camera_.view(), proj = camera_.projection(h > 0 ? float(w) / float(h) : 1.0f);
    planetShader_.use();
    planetShader_.set("uView", view);
    planetShader_.set("uProj", proj);
    planetShader_.set("uViewPos", camera_.position());
    planetShader_.set("uSunPos", system_.sun().position());
    planetShader_.set("uSunColor", glm::vec3(1.0f, 0.96f, 0.88f));
    planetShader_.set("uFillOn", int(state_.fillLight));
    planetShader_.set("uFillPos", camera_.position());
    planetShader_.set("uFillColor", glm::vec3(0.35f, 0.45f, 0.65f));
    drawBodies();
    if (state_.orbitLines) drawOrbitLines(view, proj);
    skybox_.draw(view, proj);
}

void App::run() {
    double last = glfwGetTime(), fpsClock = last;
    int frames = 0;
    bool vsync = state_.vsync;
    while (!window_.shouldClose()) {
        double now = glfwGetTime();
        float dt = std::min(float(now - last), 0.1f);  // clamp so a stall does not teleport the camera
        last = now;

        window_.pollEvents();
        handleInput(dt);
        render();
        std::string cam = followed_ ? "following " + followed_->params().name : "free fly";
        ui_.draw(state_, fps_, cam);
        window_.swap();

        if (state_.vsync != vsync) {
            vsync = state_.vsync;
            window_.setVsync(vsync);
        }
        ++frames;
        if (now - fpsClock >= 0.5) {
            fps_ = float(frames / (now - fpsClock));
            window_.setTitle("Solar System Explorer | FPS: " + std::to_string(int(fps_ + 0.5f)));
            frames = 0;
            fpsClock = now;
        }
    }
}
