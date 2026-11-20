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
      belt_(system_.planet(3), system_.planet(4)),  // between Mars and Jupiter
      ui_(window_.handle()) {
    system_.forEach([this](SceneNode& n) {
        std::string key = lower(n.params().name);
        textures_.emplace(key, Texture("assets/textures/" + key + ".jpg"));
    });
    textures_.emplace("earth_night", Texture("assets/textures/earth_night.jpg"));
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    system_.update(clock_.days());
}

void App::focusOn(SceneNode* node) {
    followed_ = node;
    const float radius = node->params().radius;
    camera_.follow(radius * 4.0f + 3.0f, radius * 1.3f + 0.3f);  // the wheel stops just above the surface
}

void App::handleInput(float dt) {
    ImGuiIO& io = ImGui::GetIO();
    if (window_.keyPressed(GLFW_KEY_ESCAPE)) window_.close();

    // Scene interactions (R6): focus a planet, change time, toggle light / orbit lines / belt / pause.
    // Skipped while a panel field has keyboard focus, so typing a number there does not move the camera.
    const bool keys = !io.WantCaptureKeyboard;
    auto pressed = [&](int key) { return keys && window_.keyPressed(key); };
    for (int i = 0; i < system_.planetCount(); ++i)
        if (pressed(GLFW_KEY_1 + i)) focusOn(&system_.planet(i));
    if (pressed(GLFW_KEY_9)) focusOn(&system_.moon());
    if (pressed(GLFW_KEY_0) || pressed(GLFW_KEY_F)) {
        followed_ = nullptr;
        camera_.setFree();
    }
    if (pressed(GLFW_KEY_SPACE)) clock_.togglePause();
    if (pressed(GLFW_KEY_L)) state_.fillLight = !state_.fillLight;
    if (pressed(GLFW_KEY_O)) state_.orbitLines = !state_.orbitLines;
    if (pressed(GLFW_KEY_B)) state_.asteroids = !state_.asteroids;
    if (pressed(GLFW_KEY_H)) state_.showHelp = !state_.showHelp;
    if (pressed(GLFW_KEY_V)) state_.vsync = !state_.vsync;
    if (pressed(GLFW_KEY_RIGHT_BRACKET)) clock_.faster();
    if (pressed(GLFW_KEY_LEFT_BRACKET)) clock_.slower();

    // Camera: speeds scale with dt, so motion is the same at any frame rate (C1).
    // Mouse look starts with the right button outside the panel and lasts until the button is released.
    // The cursor is captured meanwhile, so a long turn is not cut short by the edge of the screen.
    const bool rightButton = window_.mouseDown(GLFW_MOUSE_BUTTON_RIGHT);
    if (rightButton && !looking_ && !io.WantCaptureMouse) {
        looking_ = true;
        window_.captureCursor(true);
    } else if (!rightButton && looking_) {
        looking_ = false;
        window_.captureCursor(false);
    } else if (looking_) {
        camera_.look(window_.mouseDelta().x, window_.mouseDelta().y);
    }
    const float scroll = window_.takeScroll();
    if (!io.WantCaptureMouse) camera_.zoom(scroll);
    auto held = [&](int key) { return keys && window_.keyDown(key); };
    glm::vec3 axes(0.0f);
    axes.z = float(held(GLFW_KEY_W)) - float(held(GLFW_KEY_S));
    axes.x = float(held(GLFW_KEY_D)) - float(held(GLFW_KEY_A));
    axes.y = float(held(GLFW_KEY_E)) - float(held(GLFW_KEY_Q));
    camera_.move(axes, dt, held(GLFW_KEY_LEFT_SHIFT));

    clock_.advance(dt);
    system_.update(clock_.days(), dt);  // dt lets fast spins slow down on screen
    if (followed_) {
        camera_.setTarget(followed_->position());
        const SceneNode* parent = followed_->parent();
        if (parent && parent->parent()) camera_.setPivot(parent->position());  // a moon: ride along its orbit
        else camera_.clearPivot();
    }
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
    SceneLights lights;
    lights.sunPos = system_.sun().position();
    lights.fillPos = camera_.position();  // the lantern travels with the camera
    lights.fillOn = state_.fillLight;

    planetShader_.use();
    planetShader_.set("uView", view);
    planetShader_.set("uProj", proj);
    planetShader_.set("uViewPos", camera_.position());
    lights.apply(planetShader_);
    drawBodies();
    if (state_.asteroids)  // opaque, so it goes with the planets, before the skybox and the transparent rings
        belt_.draw(state_.asteroidCount, clock_.days(), view, proj, camera_.position(), lights, texture("moon"));
    if (state_.orbitLines) drawOrbitLines(view, proj);
    skybox_.draw(view, proj);
    rings_.draw(system_.planet(5), view, proj, lights);
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
        ui_.draw(state_, clock_, fps_, cam);
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
