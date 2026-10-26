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
    CHECK(m.indices.size() == 16u * 32u * 6u);
    for (const auto& v : m.vertices) CHECK(near(glm::length(v.normal), 1.0f));
    for (const auto& v : m.vertices) CHECK(v.uv.x >= 0.0f && v.uv.x <= 1.0f && v.uv.y >= 0.0f && v.uv.y <= 1.0f);
}

static void sphereTrianglesFaceOutward() {  // CCW winding seen from outside, needed for face culling
    MeshData m = makeSphere(16, 32);
    int inward = 0;
    for (size_t i = 0; i + 2 < m.indices.size(); i += 3) {
        glm::vec3 a = m.vertices[m.indices[i]].pos, b = m.vertices[m.indices[i + 1]].pos, c = m.vertices[m.indices[i + 2]].pos;
        glm::vec3 n = glm::cross(b - a, c - a);
        if (glm::length(n) < 1e-6f) continue;  // degenerate triangles at the poles
        if (glm::dot(n, a + b + c) < 0.0f) ++inward;
    }
    CHECK(inward == 0);
}

static void ringHasHoleAndConsistentWinding() {
    MeshData m = makeRing(64, 1.25f, 2.4f);
    CHECK(m.vertices.size() == 130u);
    CHECK(m.indices.size() == 384u);
    for (const auto& v : m.vertices) {
        CHECK(near(v.pos.y, 0.0f));
        CHECK(near(glm::length(v.pos), v.uv.x == 0.0f ? 1.25f : 2.4f));
        CHECK(v.uv.y >= 0.0f && v.uv.y <= 1.0f);
        CHECK(near(v.normal.y, 1.0f));
    }
    for (size_t i = 0; i < m.indices.size(); i += 3) {
        CHECK(m.indices[i] < m.vertices.size() && m.indices[i + 1] < m.vertices.size() &&
              m.indices[i + 2] < m.vertices.size());
        glm::vec3 a = m.vertices.at(m.indices[i]).pos, b = m.vertices.at(m.indices[i + 1]).pos;
        glm::vec3 c = m.vertices.at(m.indices[i + 2]).pos;
        CHECK(glm::cross(b - a, c - a).y > 0.0f);
        CHECK(glm::length((a + b + c) / 3.0f) > 1.2f);  // no triangles across the central hole
    }
    CHECK(near(glm::length(m.vertices[0].pos - m.vertices[128].pos), 0.0f));
    CHECK(near(glm::length(m.vertices[1].pos - m.vertices[129].pos), 0.0f));
    bool rejected = false;
    try { makeRing(2, 1.25f, 2.4f); } catch (const std::invalid_argument&) { rejected = true; }
    CHECK(rejected);
    rejected = false;
    try { makeRing(64, 2.4f, 1.25f); } catch (const std::invalid_argument&) { rejected = true; }
    CHECK(rejected);
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
    ringHasHoleAndConsistentWinding();
    moonOrbitsEarthNotSun();
    earthOrbitRadiusAndPeriod();
    childrenAreNotScaledByParent();
    followCameraConvergesToTarget();
    cameraSmoothingIsFrameRateIndependent();
    if (failures == 0) std::printf("all core tests passed\n");
    return failures == 0 ? 0 : 1;
}
