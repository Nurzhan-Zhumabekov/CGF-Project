#include "Primitives.h"
#include <cmath>
#include <stdexcept>

static const float kPi = 3.14159265358979f;

// Grid of (stacks + 1) rows x (sectors + 1) columns on the unit sphere.
//   phi   = pi * i / stacks        latitude angle from the north pole (row i)
//   theta = 2 pi * j / sectors     longitude (column j)
//   p     = (sin phi cos theta, cos phi, -sin phi sin theta)
// The normal of a unit sphere is the position itself. The last column repeats the first with u = 1,
// so the texture does not wrap backwards across the seam.
MeshData makeSphere(int stacks, int sectors) {
    if (stacks < 2 || sectors < 3) throw std::invalid_argument("sphere needs 2+ stacks and 3+ sectors");
    MeshData m;
    m.vertices.reserve(size_t(stacks + 1) * size_t(sectors + 1));
    for (int i = 0; i <= stacks; ++i) {
        float phi = kPi * float(i) / float(stacks);
        bool pole = (i == 0 || i == stacks);
        float y = pole ? (i == 0 ? 1.0f : -1.0f) : std::cos(phi);  // exact poles, no rounding error
        float r = pole ? 0.0f : std::sin(phi);
        for (int j = 0; j <= sectors; ++j) {
            float theta = 2.0f * kPi * float(j) / float(sectors);
            glm::vec3 p(r * std::cos(theta), y, -r * std::sin(theta));
            // A pole vertex serves one triangle per sector; centring its u in that sector stops
            // the texture from twisting into a swirl at the poles.
            float u = pole ? (float(j) + 0.5f) / float(sectors) : float(j) / float(sectors);
            m.vertices.push_back({p, p, {std::fmin(u, 1.0f), 1.0f - float(i) / float(stacks)}});
        }
    }
    // Each quad between rows i and i + 1 is two CCW triangles (seen from outside, for face culling).
    // At the poles the two top (or bottom) corners are the same point, so one triangle per sector is
    // enough, and it uses pole vertex j, whose u is the centre of sector j.
    m.indices.reserve(size_t(6) * size_t(sectors) * size_t(stacks - 1));
    for (int i = 0; i < stacks; ++i) {
        for (int j = 0; j < sectors; ++j) {
            unsigned a = unsigned(i * (sectors + 1) + j), b = a + unsigned(sectors + 1);
            if (i == 0) m.indices.insert(m.indices.end(), {a, b, b + 1});                // north cap
            else if (i == stacks - 1) m.indices.insert(m.indices.end(), {a, b, a + 1});  // south cap
            else m.indices.insert(m.indices.end(), {a, b, a + 1, a + 1, b, b + 1});
        }
    }
    return m;
}

MeshData makeCube() {
    static const float v[] = {
        -1, 1, -1, -1, -1, -1, 1, -1, -1, 1, -1, -1, 1, 1, -1, -1, 1, -1,
        -1, -1, 1, -1, -1, -1, -1, 1, -1, -1, 1, -1, -1, 1, 1, -1, -1, 1,
        1, -1, -1, 1, -1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1, 1, -1, -1,
        -1, -1, 1, -1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1, 1, -1, -1, 1,
        -1, 1, -1, 1, 1, -1, 1, 1, 1, 1, 1, 1, -1, 1, 1, -1, 1, -1,
        -1, -1, -1, -1, -1, 1, 1, -1, -1, 1, -1, -1, -1, -1, 1, 1, -1, 1};
    MeshData m;
    for (int i = 0; i < 36; ++i) m.vertices.push_back({{v[3 * i], v[3 * i + 1], v[3 * i + 2]}, {0, 0, 0}, {0, 0}});
    return m;
}

MeshData makeCircle(int segments) {
    MeshData m;
    for (int i = 0; i < segments; ++i) {
        float a = 2.0f * kPi * float(i) / float(segments);
        m.vertices.push_back({{std::cos(a), 0.0f, std::sin(a)}, {0, 1, 0}, {0, 0}});
    }
    return m;
}
