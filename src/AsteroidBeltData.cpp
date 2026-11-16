#include "AsteroidBeltData.h"
#include "Primitives.h"
#include <algorithm>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <random>
#include <stdexcept>

static const float kTwoPi = 6.28318530717959f;

AsteroidBeltParams beltBetween(float innerOrbit, float innerBodyRadius, float outerOrbit, float outerBodyRadius) {
    // The free gap runs from the outer edge of the inner planet to the inner edge of the outer one.
    float gapStart = innerOrbit + innerBodyRadius, gapEnd = outerOrbit - outerBodyRadius;
    if (!(gapEnd > gapStart)) throw std::invalid_argument("no room for a belt between these orbits");
    AsteroidBeltParams p;
    p.innerRadius = glm::mix(gapStart, gapEnd, 0.2f);  // keep a clear lane next to each planet
    p.outerRadius = glm::mix(gapStart, gapEnd, 0.8f);
    return p;
}

std::vector<AsteroidInstance> makeAsteroids(const AsteroidBeltParams& p) {
    if (p.count < 0 || !(p.innerRadius > 0.0f) || !(p.outerRadius > p.innerRadius) || p.halfThickness < 0.0f ||
        !(p.minSize > 0.0f) || p.maxSize < p.minSize)
        throw std::invalid_argument("belt needs count >= 0, 0 < inner < outer radius and 0 < min <= max size");

    std::mt19937 rng(p.seed);
    std::uniform_real_distribution<float> unit(0.0f, 1.0f);
    // The sum of two uniform numbers is triangular: most likely in the middle, rare at the ends.
    // It gives the belt a dense core with thin edges instead of a flat band with a hard border.
    auto triangular = [&] { float a = unit(rng), b = unit(rng); return 0.5f * (a + b); };
    // A direction uniform on the sphere: z uniform in [-1, 1] and any angle around it.
    auto direction = [&] {
        float z = 2.0f * unit(rng) - 1.0f, around = kTwoPi * unit(rng), ring = std::sqrt(1.0f - z * z);
        return glm::vec3(ring * std::cos(around), ring * std::sin(around), z);
    };

    const glm::mat4 I(1.0f);
    std::vector<AsteroidInstance> asteroids;
    asteroids.reserve(size_t(p.count));
    for (int i = 0; i < p.count; ++i) {
        float radius = glm::mix(p.innerRadius, p.outerRadius, triangular());
        float angle = kTwoPi * unit(rng);
        float height = p.halfThickness * (2.0f * triangular() - 1.0f);
        glm::vec3 position(radius * std::cos(angle), height, -radius * std::sin(angle));

        float u = unit(rng);
        float size = glm::mix(p.minSize, p.maxSize, u * u * u);  // cubed: many pebbles, few boulders

        glm::vec3 axis = direction();  // random orientation: any axis, any angle
        float turn = kTwoPi * unit(rng);

        AsteroidInstance a;
        a.model = glm::translate(I, position) * glm::rotate(I, turn, axis) * glm::scale(I, glm::vec3(size));

        // Tumbling: 1 to 5 whole turns per period, either way round.
        float turns = float(1 + std::min(int(unit(rng) * 5.0f), 4));
        a.tumble = glm::vec4(direction(), unit(rng) < 0.5f ? turns : -turns);

        // Colour: every rock shares one texture, so vary its brightness and warmth a little.
        float brightness = glm::mix(0.6f, 1.1f, unit(rng)), warmth = 0.1f * (2.0f * unit(rng) - 1.0f);
        a.tint = brightness * glm::vec3(1.0f + warmth, 1.0f, 1.0f - warmth);

        asteroids.push_back(a);
    }
    return asteroids;
}

// The surface is p(d) = S * r(d) * d for every unit direction d, where
//   r(d) = 1 + sum_i a_i * sin(f_i . d + phase_i)      a few smooth bumps and dents
//   S    = diag(stretch)                               makes the lump longer than it is tall
// r depends only on the direction, so the duplicated seam and pole vertices of the sphere land on
// the same point and the mesh stays closed.
//
// Normal: the surface is the level set |x| - r(x / |x|) = 0, whose gradient at the surface is
// proportional to r * d - gT, with g = sum_i a_i * cos(f_i . d + phase_i) * f_i (gradient of r) and
// gT = g - (g . d) d its part tangent to the sphere. A stretch S maps normals by the inverse
// transpose, which for a diagonal S is a division by the stretch.
MeshData makeRock(int stacks, int sectors, unsigned seed) {
    struct Lobe { glm::vec3 frequency; float amplitude, phase; };
    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> unit(0.0f, 1.0f);
    Lobe lobes[6];
    for (Lobe& l : lobes) {
        float z = 2.0f * unit(rng) - 1.0f, around = kTwoPi * unit(rng), ring = std::sqrt(1.0f - z * z);
        l.frequency = glm::vec3(ring * std::cos(around), ring * std::sin(around), z) * glm::mix(1.5f, 3.5f, unit(rng));
        l.amplitude = glm::mix(0.04f, 0.10f, unit(rng));  // six lobes: r stays within 1 +- 0.6
        l.phase = kTwoPi * unit(rng);
    }
    const glm::vec3 stretch(1.0f, 0.72f, 0.86f);

    MeshData m = makeSphere(stacks, sectors);
    for (Vertex& v : m.vertices) {
        glm::vec3 d = v.pos;  // the unit sphere: position = direction
        float r = 1.0f;
        glm::vec3 g(0.0f);
        for (const Lobe& l : lobes) {
            float t = glm::dot(l.frequency, d) + l.phase;
            r += l.amplitude * std::sin(t);
            g += l.amplitude * std::cos(t) * l.frequency;
        }
        glm::vec3 tangentGradient = g - glm::dot(g, d) * d;
        v.pos = stretch * (r * d);
        v.normal = glm::normalize((r * d - tangentGradient) / stretch);
    }
    return m;
}

// Same rule as the planets: angle = 2 pi * days / period, wrapped in double before the cast so
// the precision does not fall off late in a long, fast run.
static float turnFraction(double days, float period) {
    double turns = std::fmod(days / double(period), 1.0);
    return float(turns < 0.0 ? turns + 1.0 : turns);
}

float beltAngle(double days) { return kTwoPi * turnFraction(days, kBeltPeriodDays); }

float beltTumble(double days) { return turnFraction(days, kTumblePeriodDays); }
