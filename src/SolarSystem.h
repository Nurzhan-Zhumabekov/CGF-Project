#pragma once
#include "SceneNode.h"
#include <functional>

// Builds and owns the scene graph: Sun -> 8 planets, Earth -> Moon.
class SolarSystem {
public:
    SolarSystem();
    void update(float days) { sun_->update(days); }
    void forEach(const std::function<void(SceneNode&)>& fn) const;

    SceneNode& sun() const { return *sun_; }
    SceneNode& planet(int i) const { return *planets_.at(i); }  // 0 = Mercury ... 7 = Neptune
    SceneNode& moon() const { return *moon_; }
    int planetCount() const { return int(planets_.size()); }

private:
    std::unique_ptr<SceneNode> sun_;
    std::vector<SceneNode*> planets_;
    SceneNode* moon_ = nullptr;
};
