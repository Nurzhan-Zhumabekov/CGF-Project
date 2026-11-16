#pragma once
#include "MeshData.h"
#include <glm/glm.hpp>
#include <vector>

// CPU half of the asteroid belt: where every rock sits and what one rock looks like.
// Kept free of OpenGL so it can be unit-tested; AsteroidBelt uploads the result to the GPU.

constexpr int kMaxAsteroids = 5000;      // instances generated and uploaded once at start-up
constexpr int kDefaultAsteroids = 3000;  // how many of them are drawn until the panel slider is moved
constexpr float kBeltPeriodDays = 1682.0f;  // one revolution of the belt: the real period of Ceres
constexpr float kTumblePeriodDays = 60.0f;  // slowest rock turns once in this time (stylized, to be visible)

struct AsteroidBeltParams {
    int count = kMaxAsteroids;
    float innerRadius = 18.4f;    // distance from the Sun, scene units
    float outerRadius = 21.0f;
    float halfThickness = 0.45f;  // how far a rock can be above or below the orbital plane
    float minSize = 0.035f;       // rock radius; the planets are 0.4 .. 2.2
    float maxSize = 0.16f;
    unsigned seed = 2420u;        // fixed, so the belt looks the same on every run
};

// Everything that differs from one asteroid to the next. The layout (23 floats, no padding) is the
// layout of the instance buffer, see AsteroidBelt.
struct AsteroidInstance {
    glm::mat4 model;   // relative to the Sun: T(position) * R(random axis, angle) * S(size), uniform scale
    glm::vec4 tumble;  // xyz: unit axis the rock spins about (in its own frame); w: whole turns per
                       // kTumblePeriodDays, signed and never 0
    glm::vec3 tint;    // multiplies the surface colour: darker / lighter, slightly warmer / cooler
};

// Belt occupying the middle of the free gap between two bodies, given their orbit and body radii.
// Throws std::invalid_argument if the gap is empty.
AsteroidBeltParams beltBetween(float innerOrbit, float innerBodyRadius, float outerOrbit, float outerBodyRadius);

// One instance per asteroid. The scale is uniform, so mat3(model) is also valid for normals. The
// first N instances of a longer list equal a list generated with count = N, so drawing a prefix
// gives an evenly thinned belt.
// Throws std::invalid_argument for a negative count, an empty radius range or non-positive sizes.
std::vector<AsteroidInstance> makeAsteroids(const AsteroidBeltParams& params);

// Lumpy rock of radius about 1: a UV sphere whose radius varies with direction, with exact normals.
MeshData makeRock(int stacks, int sectors, unsigned seed);

// Rotation of the whole belt about the Sun at simulation time `days`, in radians [0, 2 pi).
float beltAngle(double days);

// Fraction of the tumble period completed at `days`, in [0, 1). A rock has turned
// 2 pi * tumble.w * beltTumble(days); tumble.w is a whole number, so the wrap from 1 to 0 is invisible.
float beltTumble(double days);
