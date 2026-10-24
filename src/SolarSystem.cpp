#include "SolarSystem.h"

SolarSystem::SolarSystem() {
    sun_ = std::make_unique<SceneNode>(BodyParams{"Sun", 3.0f, 0.0f, 0.0f, 25.0f, 0.0f});
    //                        name       radius orbitR  period    spin   tilt
    const BodyParams planets[] = {
        {"Mercury", 0.40f, 6.0f, 88.0f, 58.6f, 0.0f},
        {"Venus", 0.90f, 9.0f, 225.0f, -243.0f, 177.0f},
        {"Earth", 1.00f, 13.0f, 365.25f, 1.0f, 23.4f},
        {"Mars", 0.55f, 17.0f, 687.0f, 1.03f, 25.2f},
        {"Jupiter", 2.20f, 24.0f, 4333.0f, 0.41f, 3.1f},
        {"Saturn", 1.90f, 32.0f, 10759.0f, 0.45f, 26.7f},
        {"Uranus", 1.30f, 39.0f, 30687.0f, -0.72f, 97.8f},
        {"Neptune", 1.25f, 45.0f, 60190.0f, 0.67f, 28.3f}};
    for (const auto& p : planets) planets_.push_back(sun_->addChild(p));
    moon_ = planets_[2]->addChild({"Moon", 0.27f, 2.2f, 27.3f, 27.3f, 6.7f});  // child of Earth
}

void SolarSystem::forEach(const std::function<void(SceneNode&)>& fn) const {
    std::function<void(SceneNode&)> walk = [&](SceneNode& n) {
        fn(n);
        for (auto& c : n.children()) walk(*c);
    };
    walk(*sun_);
}
