#pragma once
#include <glm/glm.hpp>
#include <vector>

struct Vertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 uv;
};

// CPU-side geometry. Kept free of OpenGL so it can be unit-tested.
struct MeshData {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;  // empty means draw vertices in order
};
