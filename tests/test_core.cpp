// Minimal assert-based unit tests for the OpenGL-free core (geometry, scene graph, camera).
#include "Camera.h"
#include "Primitives.h"
#include "SimClock.h"
#include "SolarSystem.h"
#include <cmath>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <vector>

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

static void ringsFollowSaturnWithFixedTilt() {
    SolarSystem s;
    SceneNode& saturn = s.planet(5);
    MeshData ring = makeRing(64, 1.25f, 2.4f);
    s.update(0.0f);
    glm::vec3 axis0 = saturn.spinAxis();
    for (float days : {0.0f, 100.0f, saturn.params().orbitPeriod / 4.0f}) {
        s.update(days);
        CHECK(near(glm::dot(saturn.spinAxis(), axis0), 1.0f));
        for (const auto& v : ring.vertices) {
            glm::vec3 offset = glm::vec3(saturn.model() * glm::vec4(v.pos, 1.0f)) - saturn.position();
            CHECK(near(glm::dot(offset, axis0), 0.0f));  // ring stays in Saturn's equatorial plane
            CHECK(near(glm::length(offset), glm::length(v.pos) * saturn.params().radius));
        }
    }
}

// ---- Time: SimClock, and objective 5 ("everything moves; doubling timeScale doubles all speeds") ----

static const float kPiF = 3.14159265f;

// Orbit angle of a body around its parent, measured from positions (x = r cos a, z = -r sin a).
static float measuredOrbitAngle(const SceneNode& n) {
    glm::vec3 p = n.position() - n.parent()->position();
    return std::atan2(-p.z, p.x);
}

static float wrapAngle(float a) {  // to (-pi, pi]
    while (a > kPiF) a -= 2.0f * kPiF;
    while (a <= -kPiF) a += 2.0f * kPiF;
    return a;
}

// Angle the body turned about its own spin axis between two model matrices, measured from the
// body's x axis. Positive = counter-clockwise seen from the north pole, negative = retrograde.
static float measuredSpin(const glm::mat4& m0, const glm::mat4& m1, glm::vec3 axis) {
    glm::vec3 a = glm::normalize(glm::vec3(m0[0])), b = glm::normalize(glm::vec3(m1[0]));
    return std::atan2(glm::dot(axis, glm::cross(a, b)), glm::dot(a, b));
}

static std::vector<std::string> bodyNames() {
    std::vector<std::string> names;
    SolarSystem s;
    s.forEach([&](SceneNode& n) { names.push_back(n.params().name); });
    return names;
}

// Runs `frames` frames of 1/60 s on a fresh system and returns how far one body orbited and spun.
struct Swept { float orbit = 0.0f, spin = 0.0f; };
static Swept sweep(const std::string& name, float scale, int frames, double startDay = 1000.0) {
    SolarSystem s;
    SimClock clock;
    clock.setDays(startDay);
    clock.setScale(scale);
    SceneNode& n = *s.find(name);
    s.update(clock.days());
    float orbit0 = n.parent() ? measuredOrbitAngle(n) : 0.0f;
    glm::mat4 model0 = n.model();
    for (int i = 0; i < frames; ++i) {
        clock.advance(1.0f / 60.0f);
        s.update(clock.days());
    }
    Swept r;
    if (n.parent()) r.orbit = wrapAngle(measuredOrbitAngle(n) - orbit0);
    r.spin = measuredSpin(model0, n.model(), n.spinAxis());
    return r;
}

static void clockAdvancesByDtTimesScale() {
    SimClock c;
    CHECK(near(c.scale(), SimClock::kDefaultScale));
    c.advance(0.5f);
    CHECK(near(float(c.days()), 0.5f * SimClock::kDefaultScale));
    c.advance(-1.0f);  // a negative frame time never runs the clock backwards
    CHECK(near(float(c.days()), 0.5f * SimClock::kDefaultScale));
}

static void clockPauseFreezesTime() {
    SimClock c;
    c.advance(1.0f);
    double before = c.days();
    c.togglePause();
    for (int i = 0; i < 100; ++i) c.advance(1.0f / 60.0f);
    CHECK(c.days() == before);
    c.togglePause();
    c.advance(1.0f);
    CHECK(c.days() > before);
}

static void clockSpeedKeysDoubleAndHalveWithinLimits() {
    SimClock c;
    c.faster();
    CHECK(near(c.scale(), 2.0f * SimClock::kDefaultScale));
    c.slower();
    c.slower();
    CHECK(near(c.scale(), 0.5f * SimClock::kDefaultScale));
    for (int i = 0; i < 40; ++i) c.slower();
    CHECK(near(c.scale(), SimClock::kMinScale));  // never 0 or negative
    for (int i = 0; i < 40; ++i) c.faster();
    CHECK(near(c.scale(), SimClock::kMaxScale));
    c.setScale(std::nanf(""));
    CHECK(near(c.scale(), SimClock::kMaxScale));  // invalid input is ignored
}

static void clockKeepsPrecisionAfterLongFastRun() {  // a float accumulator froze here
    SimClock c;
    c.setScale(SimClock::kMaxScale);
    for (int i = 0; i < 60 * 300; ++i) c.advance(1.0f / 60.0f);  // 5 minutes at top speed
    double before = c.days();
    c.setScale(SimClock::kMinScale);
    for (int i = 0; i < 60; ++i) c.advance(1.0f / 60.0f);       // 1 second at the lowest speed
    CHECK(near(float(c.days() - before), SimClock::kMinScale, 1e-4f));
}

static void clockDateCountsFromJ2000() {
    SimClock c;
    CHECK(c.date() == "2000-01-01");
    c.setDays(59.0);
    CHECK(c.date() == "2000-02-29");  // 2000 is a leap year
    c.setDays(366.0);
    CHECK(c.date() == "2001-01-01");
    c.setDays(9132.0);
    CHECK(c.date() == "2025-01-01");
    c.setDays(-1.0);
    CHECK(c.date() == "1999-12-31");
}

static void everyBodyOrbitsAndSpins() {  // objective 5, first half: 8 planets + Moon around the Sun and their axes
    int orbiting = 0;
    for (const std::string& name : bodyNames()) {
        SolarSystem s;
        const BodyParams& p = s.find(name)->params();
        if (p.orbitPeriod > 0.0f) {
            // 1/8 of an orbit: run at 1/8 period per second for 60 frames
            float scale = p.orbitPeriod / 8.0f > SimClock::kMaxScale ? SimClock::kMaxScale : p.orbitPeriod / 8.0f;
            int frames = int(std::lround(60.0 * (p.orbitPeriod / 8.0) / scale));
            Swept r = sweep(name, scale, frames);
            CHECK(near(r.orbit, 2.0f * kPiF / 8.0f, 0.01f));  // counter-clockwise, 45 degrees
            ++orbiting;
        }
        float spinScale = std::fabs(p.spinPeriod) / 8.0f;
        int spinFrames = 60;
        if (spinScale < SimClock::kMinScale) {
            spinFrames = int(std::lround(60.0 * (std::fabs(p.spinPeriod) / 8.0) / SimClock::kMinScale));
            spinScale = SimClock::kMinScale;
        }
        Swept r = sweep(name, spinScale, spinFrames);
        float expected = (p.spinPeriod > 0.0f ? 1.0f : -1.0f) * 2.0f * kPiF / 8.0f;  // retrograde: Venus, Uranus
        if (!near(r.spin, expected, 0.01f)) std::printf("  spin of %s: %f, expected %f\n", name.c_str(), r.spin, expected);
        CHECK(near(r.spin, expected, 0.01f));
    }
    CHECK(orbiting == 9);  // 8 planets + the Moon
}

static void doublingTimeScaleDoublesEverySpeed() {  // objective 5, second half
    for (const std::string& name : bodyNames()) {
        SolarSystem s;
        const BodyParams& p = s.find(name)->params();
        for (float period : {p.orbitPeriod, std::fabs(p.spinPeriod)}) {
            if (period <= 0.0f) continue;
            // Same real time for both runs; the slow run covers 1/16 of a turn, the fast one 1/8.
            float scale = period / 16.0f;
            if (scale < SimClock::kMinScale) scale = SimClock::kMinScale;
            if (scale > SimClock::kMaxScale / 2.0f) scale = SimClock::kMaxScale / 2.0f;
            int frames = int(std::lround(60.0 * (period / 16.0) / scale));
            Swept slow = sweep(name, scale, frames), fast = sweep(name, 2.0f * scale, frames);
            float a = period == p.orbitPeriod ? slow.orbit : slow.spin;
            float b = period == p.orbitPeriod ? fast.orbit : fast.spin;
            CHECK(std::fabs(a) > 0.1f);
            CHECK(near(b / a, 2.0f, 0.01f));
        }
    }
}

static void changingSpeedOrPausingDoesNotJump() {  // guide 5.2 item 6
    SolarSystem s;
    SimClock clock;
    SceneNode& moon = s.moon();
    s.update(clock.days());
    auto step = [&]() {  // angle the Moon moves around the Earth in one 1/60 s frame
        float before = measuredOrbitAngle(moon);
        clock.advance(1.0f / 60.0f);
        s.update(clock.days());
        return wrapAngle(measuredOrbitAngle(moon) - before);
    };
    for (int i = 0; i < 30; ++i) step();
    float normal = step();
    CHECK(normal > 0.0f);
    clock.faster();
    CHECK(near(step() / normal, 2.0f, 1e-3f));  // twice as far, never a jump to another spot
    clock.togglePause();
    glm::vec3 frozen = moon.position();
    for (int i = 0; i < 10; ++i) CHECK(step() == 0.0f);
    CHECK(moon.position() == frozen);
    clock.togglePause();
    CHECK(near(step() / normal, 2.0f, 1e-3f));  // resumes from where it stopped
}

static void slowMotionStillMovesLateInTheRun() {  // float angles stepped in 0.25 rad chunks here
    const double lateDay = 600000.0;  // about 5 minutes at the top speed
    Swept r = sweep("Earth", SimClock::kMinScale, 6, lateDay);  // 0.1 s at 0.1 days/s = 0.01 days
    CHECK(near(r.spin, 2.0f * kPiF * 0.01f, 1e-3f));
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
    ringsFollowSaturnWithFixedTilt();
    clockAdvancesByDtTimesScale();
    clockPauseFreezesTime();
    clockSpeedKeysDoubleAndHalveWithinLimits();
    clockKeepsPrecisionAfterLongFastRun();
    clockDateCountsFromJ2000();
    everyBodyOrbitsAndSpins();
    doublingTimeScaleDoublesEverySpeed();
    changingSpeedOrPausingDoesNotJump();
    slowMotionStillMovesLateInTheRun();
    followCameraConvergesToTarget();
    cameraSmoothingIsFrameRateIndependent();
    if (failures == 0) std::printf("all core tests passed\n");
    return failures == 0 ? 0 : 1;
}
