// Minimal assert-based unit tests for the OpenGL-free core (geometry, scene graph, camera).
#include "Camera.h"
#include "Primitives.h"
#include "SolarSystem.h"
#include <cmath>
#include <cstdio>
#include <stdexcept>

static int failures = 0;
#define CHECK(cond)                                                          \
    do {                                                                     \
        if (!(cond)) { std::printf("FAIL %s:%d  %s\n", __FILE__, __LINE__, #cond); ++failures; } \
    } while (0)

static bool near(float a, float b, float eps = 1e-3f) { return std::fabs(a - b) < eps; }

static void sphereHasExpectedTopology() {
    MeshData m = makeSphere(16, 32);
    CHECK(m.vertices.size() == 17u * 33u);
    CHECK(m.indices.size() == 32u * 6u * 15u);  // 14 middle bands x 2 triangles + 2 caps x 1 triangle
    for (const auto& v : m.vertices) CHECK(near(glm::length(v.normal), 1.0f));
    for (const auto& v : m.vertices) CHECK(v.uv.x >= 0.0f && v.uv.x <= 1.0f && v.uv.y >= 0.0f && v.uv.y <= 1.0f);
}

static void sphereTrianglesFaceOutward() {  // CCW winding seen from outside, needed for face culling
    MeshData m = makeSphere(16, 32);
    int inward = 0;
    for (size_t i = 0; i + 2 < m.indices.size(); i += 3) {
        glm::vec3 a = m.vertices[m.indices[i]].pos, b = m.vertices[m.indices[i + 1]].pos, c = m.vertices[m.indices[i + 2]].pos;
        glm::vec3 n = glm::cross(b - a, c - a);
        if (glm::dot(n, a + b + c) < 0.0f) ++inward;
    }
    CHECK(inward == 0);
}

static void sphereHasNoZeroAreaTriangles() {  // the pole caps used to emit one flat triangle per sector
    MeshData m = makeSphere(16, 32);
    int flat = 0;
    for (size_t i = 0; i + 2 < m.indices.size(); i += 3) {
        glm::vec3 a = m.vertices[m.indices[i]].pos, b = m.vertices[m.indices[i + 1]].pos, c = m.vertices[m.indices[i + 2]].pos;
        if (glm::length(glm::cross(b - a, c - a)) < 1e-6f) ++flat;
    }
    CHECK(flat == 0);
}

static void sphereRejectsBadArguments() {
    int thrown = 0;
    for (auto args : {glm::ivec2(1, 32), glm::ivec2(16, 2), glm::ivec2(0, 0)}) {
        try { makeSphere(args.x, args.y); } catch (const std::invalid_argument&) { ++thrown; }
    }
    CHECK(thrown == 3);
}

static void worldIsParentWorldTimesLocal() {  // the scene-graph rule, checked on every node
    SolarSystem s;
    for (float days : {0.0f, 17.0f, 500.0f}) {
        s.update(days);
        s.forEach([&](SceneNode& n) {
            glm::mat4 expected = n.parentFrame() * n.localMatrix();
            for (int c = 0; c < 4; ++c)
                for (int r = 0; r < 4; ++r) CHECK(near(n.worldMatrix()[c][r], expected[c][r]));
        });
    }
}

static void moonOrbitsEarthNotSun() {
    SolarSystem s;
    for (float days : {0.0f, 10.0f, 123.4f, 400.0f}) {
        s.update(days);
        float d = glm::length(s.moon().position() - s.planet(2).position());
        CHECK(near(d, 2.2f));  // distance to Earth stays constant
    }
}

static void earthOrbitRadiusAndPeriod() {
    SolarSystem s;
    s.update(0.0f);
    glm::vec3 p0 = s.planet(2).position();
    CHECK(near(glm::length(p0), 13.0f));
    s.update(365.25f);  // one full year
    CHECK(near(glm::length(s.planet(2).position() - p0), 0.0f, 1e-2f));
    s.update(365.25f / 2.0f);  // half a year: opposite side of the Sun
    CHECK(near(glm::length(s.planet(2).position() + p0), 0.0f, 1e-2f));
}

static void childrenAreNotScaledByParent() {
    SolarSystem s;
    s.update(50.0f);
    CHECK(near(glm::length(glm::vec3(s.moon().model()[0])), 0.27f));  // Moon radius, not Earth radius * 0.27
}

static void axialTiltIsFixedInSpace() {  // the axis must not swing around with the orbit (seasons)
    SolarSystem s;
    SceneNode& earth = s.planet(2);
    s.update(0.0f);
    glm::vec3 axis0 = earth.spinAxis();
    CHECK(near(glm::degrees(std::acos(axis0.y)), 23.44f, 0.01f));
    for (float days : {91.3f, 182.6f, 300.0f}) {
        s.update(days);
        CHECK(near(glm::dot(earth.spinAxis(), axis0), 1.0f, 1e-4f));
    }
}

static void moonReturnsAfterOneSiderealMonth() {  // its orbit must not inherit the Earth's orbital turn
    SolarSystem s;
    s.update(40.0f);
    glm::vec3 offset0 = s.moon().position() - s.planet(2).position();
    s.update(40.0f + 27.32f);
    CHECK(near(glm::length(s.moon().position() - s.planet(2).position() - offset0), 0.0f, 1e-2f));
}

static void moonIsTidallyLocked() {  // the same side of the Moon faces the Earth at all times
    SolarSystem s;
    for (float days : {0.0f, 5.0f, 13.7f, 100.0f}) {
        s.update(days);
        glm::vec3 toEarth = glm::normalize(s.planet(2).position() - s.moon().position());
        glm::vec3 nearSide = glm::normalize(glm::vec3(s.moon().model() * glm::vec4(-1, 0, 0, 0)));
        CHECK(glm::dot(nearSide, toEarth) > 0.99f);
    }
}

static void planetsStartSpreadOut() {  // with orbit phases they are not lined up along +X
    SolarSystem s;
    s.update(0.0f);
    int onPositiveX = 0;
    for (int i = 0; i < s.planetCount(); ++i) {
        glm::vec3 p = glm::normalize(s.planet(i).position());
        if (p.x > 0.99f) ++onPositiveX;
        CHECK(near(p.y, 0.0f));
    }
    CHECK(onPositiveX <= 1);
}

static void findBodiesByName() {
    SolarSystem s;
    CHECK(s.find("Sun") == &s.sun());
    CHECK(s.find("Saturn") == &s.planet(5));
    CHECK(s.find("Moon") == &s.moon());
    CHECK(s.find("Pluto") == nullptr);
}

static void followCameraConvergesToTarget() {
    Camera c;
    c.follow(10.0f);
    c.setTarget({50, 0, 0});
    for (int i = 0; i < 600; ++i) c.update(1.0f / 60.0f);  // 10 s at 60 FPS
    glm::vec3 toTarget = glm::vec3(50, 0, 0) - c.position();
    CHECK(near(glm::length(toTarget), 10.0f, 0.05f));
}

static void cameraSmoothingIsFrameRateIndependent() {
    Camera a, b;
    a.follow(10.0f); b.follow(10.0f);
    a.setTarget({30, 0, 0}); b.setTarget({30, 0, 0});
    for (int i = 0; i < 60; ++i) a.update(1.0f / 60.0f);
    for (int i = 0; i < 144; ++i) b.update(1.0f / 144.0f);
    CHECK(near(glm::length(a.position() - b.position()), 0.0f, 0.1f));  // same wall-clock second
}

int main() {
    sphereHasExpectedTopology();
    sphereTrianglesFaceOutward();
    sphereHasNoZeroAreaTriangles();
    sphereRejectsBadArguments();
    worldIsParentWorldTimesLocal();
    moonOrbitsEarthNotSun();
    earthOrbitRadiusAndPeriod();
    childrenAreNotScaledByParent();
    axialTiltIsFixedInSpace();
    moonReturnsAfterOneSiderealMonth();
    moonIsTidallyLocked();
    planetsStartSpreadOut();
    findBodiesByName();
    followCameraConvergesToTarget();
    cameraSmoothingIsFrameRateIndependent();
    if (failures == 0) std::printf("all core tests passed\n");
    return failures == 0 ? 0 : 1;
}
