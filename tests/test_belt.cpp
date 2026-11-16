// Unit tests for the camera follow mode and the OpenGL-free half of the asteroid belt.
#include "AsteroidBeltData.h"
#include "Camera.h"
#include <cmath>
#include <cstdio>
#include <stdexcept>
#include <vector>

static int failures = 0;
#define CHECK(cond)                                                          \
    do {                                                                     \
        if (!(cond)) { std::printf("FAIL %s:%d  %s\n", __FILE__, __LINE__, #cond); ++failures; } \
    } while (0)

static bool near(float a, float b, float eps = 1e-3f) { return std::fabs(a - b) < eps; }

// ---- Camera ----

// The point the follow camera looks at: `distance` in front of the eye.
static glm::vec3 lookedAt(const Camera& c, float distance) { return c.position() + c.forward() * distance; }

// The Moon moves at about 20 units/s at the default time speed. A camera that only eases towards
// the target trails it by speed / easing rate = 3+ units, which is outside the view at this distance.
static void followCameraStaysOnFastTarget() {
    Camera c;
    c.follow(4.0f);
    const float speed = 22.0f, orbit = 13.0f, dt = 1.0f / 60.0f;
    float worst = 0.0f;
    for (int i = 0; i < 300; ++i) {
        float a = speed / orbit * dt * float(i);
        glm::vec3 target(orbit * std::cos(a), 0.0f, -orbit * std::sin(a));
        c.setTarget(target);
        c.update(dt);
        if (i >= 120) worst = std::fmax(worst, glm::length(lookedAt(c, 4.0f) - target));  // after the glide-in
    }
    CHECK(worst < 0.05f);
}

static void switchingTargetGlidesInsteadOfJumping() {
    Camera c;
    c.follow(10.0f);
    c.setTarget({0, 0, 0});
    for (int i = 0; i < 300; ++i) c.update(1.0f / 60.0f);
    glm::vec3 before = c.position();
    c.follow(10.0f);  // key pressed for another planet
    c.setTarget({40, 0, 0});
    c.update(1.0f / 60.0f);
    CHECK(glm::length(c.position() - before) < 5.0f);   // one frame later it has only started moving
    for (int i = 0; i < 300; ++i) c.update(1.0f / 60.0f);
    CHECK(near(glm::length(lookedAt(c, 10.0f) - glm::vec3(40, 0, 0)), 0.0f, 0.05f));
}

static void zoomStopsOutsideTheBody() {
    Camera c;
    c.follow(12.0f, 3.2f);  // a body of radius 2.2 with some clearance
    c.setTarget({0, 0, 0});
    c.zoom(200.0f);         // wheel far past the limit
    for (int i = 0; i < 600; ++i) c.update(1.0f / 60.0f);
    CHECK(near(glm::length(c.position()), 3.2f, 0.02f));
    c.zoom(-400.0f);
    for (int i = 0; i < 600; ++i) c.update(1.0f / 60.0f);
    CHECK(glm::length(c.position()) <= 150.0f + 0.1f);
}

// Horizontal angle in degrees between the view direction and the direction from the body to its parent.
static float bearingError(const Camera& c, glm::vec3 target, glm::vec3 pivot) {
    glm::vec3 f = c.forward(), p = pivot - target;
    float a = std::atan2(f.z, f.x) - std::atan2(p.z, p.x);
    return glm::degrees(std::atan2(std::sin(a), std::cos(a)));
}

// Following a moon: the camera rides along the orbit behind it, so the planet stays in view behind
// the moon and never passes between the moon and the camera.
static void satelliteCameraKeepsParentBehindTheBody() {
    Camera c;
    c.follow(4.0f);
    const float dt = 1.0f / 60.0f, orbit = 2.2f, turnPerSecond = 6.9f;  // the Moon at the default time speed
    for (int i = 0; i < 300; ++i) {
        glm::vec3 pivot(13.0f + 6.7f * dt * float(i), 0.0f, -4.0f);  // the planet moves as well
        float a = turnPerSecond * dt * float(i);
        glm::vec3 target = pivot + glm::vec3(orbit * std::cos(a), 0.0f, -orbit * std::sin(a));
        c.setTarget(target);
        c.setPivot(pivot);
        c.update(dt);
        if (i < 120) continue;  // the first two seconds are the swing into place
        CHECK(glm::length(lookedAt(c, 4.0f) - target) < 0.05f);
        CHECK(std::fabs(bearingError(c, target, pivot)) < 1.0f);
        CHECK(glm::length(c.position() - pivot) > glm::length(target - pivot));  // camera on the outer side
    }
}

static void mouseLookStillWorksWhileRidingAnOrbit() {
    Camera c;
    c.follow(4.0f);
    const float dt = 1.0f / 60.0f;
    const glm::vec3 pivot(0.0f);
    auto step = [&](int i) {
        float a = 3.0f * dt * float(i);
        glm::vec3 target(2.2f * std::cos(a), 0.0f, -2.2f * std::sin(a));
        c.setTarget(target);
        c.setPivot(pivot);
        c.update(dt);
        return target;
    };
    for (int i = 0; i < 240; ++i) step(i);
    c.look(750.0f, 0.0f);  // 750 px * 0.12 deg = a quarter turn to the right
    for (int i = 240; i < 480; ++i) {
        glm::vec3 target = step(i);
        CHECK(near(bearingError(c, target, pivot), 90.0f, 1.0f));  // the offset chosen by the user is kept
    }
    c.clearPivot();  // back to a planet: the view direction stops turning
    glm::vec3 before = c.forward();
    for (int i = 480; i < 540; ++i) {
        c.setTarget(glm::vec3(2.2f * std::cos(0.05f * float(i)), 0.0f, -2.2f * std::sin(0.05f * float(i))));
        c.update(dt);
    }
    CHECK(near(glm::length(c.forward() - before), 0.0f));
}

// ---- Asteroid belt ----

static std::vector<glm::mat4> models(const AsteroidBeltParams& p) {
    std::vector<glm::mat4> m;
    for (const AsteroidInstance& a : makeAsteroids(p)) m.push_back(a.model);
    return m;
}

static void asteroidsStayInsideTheBelt() {
    AsteroidBeltParams p;
    p.count = 2000;
    std::vector<glm::mat4> m = models(p);
    CHECK(m.size() == 2000u);
    for (const glm::mat4& t : m) {
        glm::vec3 pos(t[3]);
        float r = std::sqrt(pos.x * pos.x + pos.z * pos.z);
        CHECK(r >= p.innerRadius - 1e-3f && r <= p.outerRadius + 1e-3f);
        CHECK(std::fabs(pos.y) <= p.halfThickness + 1e-3f);
        CHECK(near(t[3][3], 1.0f) && near(t[0][3], 0.0f) && near(t[1][3], 0.0f) && near(t[2][3], 0.0f));
    }
}

// Rotation times a uniform scale: three perpendicular columns of the same length. The vertex shader
// relies on this to transform normals with mat3(model) instead of an inverse-transpose per vertex.
static void asteroidTransformsAreRotationTimesUniformScale() {
    AsteroidBeltParams p;
    p.count = 500;
    for (const glm::mat4& t : models(p)) {
        glm::vec3 x(t[0]), y(t[1]), z(t[2]);
        float s = glm::length(x);
        CHECK(s >= p.minSize - 1e-4f && s <= p.maxSize + 1e-4f);
        CHECK(near(glm::length(y), s, 1e-4f) && near(glm::length(z), s, 1e-4f));
        CHECK(near(glm::dot(x, y), 0.0f, 1e-4f) && near(glm::dot(y, z), 0.0f, 1e-4f) && near(glm::dot(x, z), 0.0f, 1e-4f));
        CHECK(glm::dot(glm::cross(x, y), z) > 0.0f);  // no mirroring, so face culling still works
    }
}

static void asteroidsFillTheWholeRing() {  // every 30-degree sector gets a fair share
    AsteroidBeltParams p;
    p.count = 3000;
    int sectors[12] = {};
    for (const glm::mat4& t : models(p)) {
        float a = std::atan2(t[3].z, t[3].x) + 3.14159265f;  // [0, 2 pi]
        ++sectors[int(a / 6.2831853f * 12.0f) % 12];
    }
    for (int n : sectors) CHECK(n > 150 && n < 350);  // 250 expected
}

// The panel draws the first N matrices, so a shorter belt must be a prefix of a longer one.
static void beltIsDeterministicAndPrefixStable() {
    AsteroidBeltParams small, large;
    small.count = 100;
    large.count = 400;
    std::vector<glm::mat4> a = models(small), b = models(small);
    std::vector<glm::mat4> c = models(large);
    CHECK(a == b);
    for (size_t i = 0; i < a.size(); ++i) CHECK(a[i] == c[i]);
    small.seed += 1;
    CHECK(models(small) != a);
}

static void asteroidsTumbleAndDifferInColour() {
    AsteroidBeltParams p;
    p.count = 1000;
    std::vector<AsteroidInstance> a = makeAsteroids(p);
    bool clockwise = false, counterClockwise = false;
    float darkest = 10.0f, brightest = 0.0f;
    for (const AsteroidInstance& i : a) {
        CHECK(near(glm::length(glm::vec3(i.tumble)), 1.0f));
        float turns = i.tumble.w;
        CHECK(turns == std::round(turns) && turns != 0.0f && std::fabs(turns) <= 5.0f);  // whole turns: no jump at the wrap
        (turns > 0.0f ? counterClockwise : clockwise) = true;
        CHECK(i.tint.x > 0.4f && i.tint.x < 1.3f && i.tint.y >= 0.6f && i.tint.y <= 1.1f && i.tint.z > 0.4f && i.tint.z < 1.3f);
        darkest = std::fmin(darkest, i.tint.y);
        brightest = std::fmax(brightest, i.tint.y);
    }
    CHECK(clockwise && counterClockwise);
    CHECK(brightest - darkest > 0.3f);
    CHECK(near(beltTumble(0.0), 0.0f) && near(beltTumble(kTumblePeriodDays * 0.25), 0.25f));
    CHECK(near(beltTumble(kTumblePeriodDays * 5000.5), 0.5f));
    CHECK(beltTumble(-1.0) >= 0.0f && beltTumble(-1.0) < 1.0f);
}

static void beltRejectsBadParameters() {
    auto rejected = [](AsteroidBeltParams p) {
        try { models(p); } catch (const std::invalid_argument&) { return true; }
        return false;
    };
    AsteroidBeltParams p;
    p.count = -1;
    CHECK(rejected(p));
    p = AsteroidBeltParams();
    p.outerRadius = p.innerRadius;
    CHECK(rejected(p));
    p = AsteroidBeltParams();
    p.minSize = 0.0f;
    CHECK(rejected(p));
    p = AsteroidBeltParams();
    p.count = 0;
    CHECK(!rejected(p) && models(p).empty());
}

static void beltFitsBetweenMarsAndJupiter() {  // the scene's values: orbit radius and body radius
    AsteroidBeltParams p = beltBetween(17.0f, 0.55f, 24.0f, 2.2f);
    CHECK(p.innerRadius > 17.0f + 0.55f + p.maxSize);
    CHECK(p.outerRadius < 24.0f - 2.2f - p.maxSize);
    CHECK(p.outerRadius > p.innerRadius);
    bool rejected = false;
    try { beltBetween(24.0f, 2.2f, 17.0f, 0.55f); } catch (const std::invalid_argument&) { rejected = true; }
    CHECK(rejected);
}

static void beltAngleFollowsThePeriod() {
    CHECK(near(beltAngle(0.0), 0.0f));
    CHECK(near(beltAngle(kBeltPeriodDays / 4.0), 3.14159265f / 2.0f));
    CHECK(near(beltAngle(kBeltPeriodDays * 1000.25), 3.14159265f / 2.0f));  // wraps without losing precision
    CHECK(beltAngle(-10.0) >= 0.0f);
}

static void rockIsAClosedOutwardFacingLump() {
    MeshData m = makeRock(6, 10, 7u);
    CHECK(!m.vertices.empty() && m.indices.size() % 3 == 0);
    float minR = 1e9f, maxR = 0.0f;
    for (const Vertex& v : m.vertices) {
        CHECK(near(glm::length(v.normal), 1.0f));
        CHECK(glm::dot(v.normal, v.pos) > 0.0f);  // normals point away from the centre
        minR = std::fmin(minR, glm::length(v.pos));
        maxR = std::fmax(maxR, glm::length(v.pos));
    }
    CHECK(minR > 0.25f && maxR < 1.75f);
    CHECK(maxR - minR > 0.1f);  // it is a lump, not a sphere
    for (size_t i = 0; i + 2 < m.indices.size(); i += 3) {
        const Vertex& a = m.vertices.at(m.indices[i]);
        const Vertex& b = m.vertices.at(m.indices[i + 1]);
        const Vertex& c = m.vertices.at(m.indices[i + 2]);
        glm::vec3 face = glm::cross(b.pos - a.pos, c.pos - a.pos);
        CHECK(glm::dot(face, a.pos + b.pos + c.pos) > 0.0f);  // CCW seen from outside
        CHECK(glm::dot(face, a.normal + b.normal + c.normal) > 0.0f);  // shading normals agree with the faces
    }
    // The seam column repeats the first one, so the surface has no crack there.
    const int columns = 11;
    for (int row = 0; row <= 6; ++row)
        CHECK(near(glm::length(m.vertices[row * columns].pos - m.vertices[row * columns + 10].pos), 0.0f));
    CHECK(makeRock(6, 10, 7u).vertices[20].pos == m.vertices[20].pos);
    CHECK(makeRock(6, 10, 8u).vertices[20].pos != m.vertices[20].pos);
}

int main() {
    followCameraStaysOnFastTarget();
    switchingTargetGlidesInsteadOfJumping();
    zoomStopsOutsideTheBody();
    satelliteCameraKeepsParentBehindTheBody();
    mouseLookStillWorksWhileRidingAnOrbit();
    asteroidsStayInsideTheBelt();
    asteroidTransformsAreRotationTimesUniformScale();
    asteroidsFillTheWholeRing();
    beltIsDeterministicAndPrefixStable();
    asteroidsTumbleAndDifferInColour();
    beltRejectsBadParameters();
    beltFitsBetweenMarsAndJupiter();
    beltAngleFollowsThePeriod();
    rockIsAClosedOutwardFacingLump();
    if (failures == 0) std::printf("all belt and camera tests passed\n");
    return failures == 0 ? 0 : 1;
}
