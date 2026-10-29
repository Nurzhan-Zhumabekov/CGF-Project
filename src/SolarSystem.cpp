#include "SolarSystem.h"

// Periods, rotation periods and axial tilts are real values (NASA planetary fact sheet, rounded).
// Radii and orbit distances are stylized: real proportions would make Mercury a sub-pixel dot
// next to the Sun and put Neptune 75x farther out than Mercury.
// Orbit phase = mean longitude at J2000 (1 Jan 2000), so the planets start spread out as they really were.
SolarSystem::SolarSystem() {
    sun_ = std::make_unique<SceneNode>(BodyParams{"Sun", 3.0f, 0.0f, 0.0f, 25.4f, 7.25f, 0.0f});
    const BodyParams planets[] = {
        //  name       radius orbitR  orbitDays  spinDays   tilt    phase
        {"Mercury",    0.40f,  6.0f,     87.97f,   58.65f,   0.03f, 252.25f},
        {"Venus",      0.90f,  9.0f,    224.70f, -243.02f, 177.40f, 181.98f},
        {"Earth",      1.00f, 13.0f,    365.25f,    1.00f,  23.44f, 100.46f},
        {"Mars",       0.55f, 17.0f,    686.98f,    1.03f,  25.19f, 355.45f},
        {"Jupiter",    2.20f, 24.0f,   4332.59f,    0.41f,   3.13f,  34.40f},
        {"Saturn",     1.90f, 32.0f,  10759.22f,    0.44f,  26.73f,  49.94f},
        {"Uranus",     1.30f, 39.0f,  30688.50f,   -0.72f,  97.77f, 313.23f},
        {"Neptune",    1.25f, 45.0f,  60182.00f,    0.67f,  28.32f, 304.88f}};
    for (const auto& p : planets) planets_.push_back(sun_->addChild(p));

    // The Moon is a child of the Earth: its orbit is measured from the Earth's centre.
    // Spin period = orbit period (tidal locking), so the same side always faces the Earth.
    moon_ = planets_[2]->addChild({"Moon", 0.27f, 2.2f, 27.32f, 27.32f, 6.68f, 218.32f});
}

SceneNode* SolarSystem::find(const std::string& name) const {
    SceneNode* found = nullptr;
    forEach([&](SceneNode& n) {
        if (!found && n.params().name == name) found = &n;
    });
    return found;
}

void SolarSystem::forEach(const std::function<void(SceneNode&)>& fn) const {
    std::function<void(SceneNode&)> walk = [&](SceneNode& n) {
        fn(n);
        for (auto& c : n.children()) walk(*c);
    };
    walk(*sun_);
}
