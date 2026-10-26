#include "Primitives.h"
#include <cmath>
#include <stdexcept>

static const float kPi = 3.14159265358979f;

MeshData makeSphere(int stacks, int sectors) {
    MeshData m;
    for (int i = 0; i <= stacks; ++i) {
        float phi = kPi * float(i) / float(stacks);  // 0 at the north pole
        float y = std::cos(phi), r = std::sin(phi);
        for (int j = 0; j <= sectors; ++j) {
            float theta = 2.0f * kPi * float(j) / float(sectors);
            glm::vec3 p(r * std::cos(theta), y, -r * std::sin(theta));
            m.vertices.push_back({p, p, {float(j) / sectors, 1.0f - float(i) / stacks}});
        }
    }
    for (int i = 0; i < stacks; ++i) {
        for (int j = 0; j < sectors; ++j) {
            unsigned a = i * (sectors + 1) + j, b = a + sectors + 1;
            m.indices.insert(m.indices.end(), {a, b, a + 1, a + 1, b, b + 1});  // CCW from outside
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

MeshData makeRing(int segments, float innerRadius, float outerRadius) {
    if (segments < 3 || !std::isfinite(innerRadius) || !std::isfinite(outerRadius) ||
        innerRadius <= 0.0f || outerRadius <= innerRadius)
        throw std::invalid_argument("ring needs 3+ segments and 0 < inner radius < outer radius");
    MeshData m;
    for (int i = 0; i <= segments; ++i) {
        float v = float(i) / float(segments);
        float angle = 2.0f * kPi * v;
        glm::vec3 direction(std::cos(angle), 0.0f, -std::sin(angle));
        m.vertices.push_back({direction * innerRadius, {0, 1, 0}, {0, v}});
        m.vertices.push_back({direction * outerRadius, {0, 1, 0}, {1, v}});
        if (i < segments) {
            unsigned a = 2 * i;
            m.indices.insert(m.indices.end(), {a, a + 1, a + 2, a + 2, a + 1, a + 3});
        }
    }
    return m;
}
