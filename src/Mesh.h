#pragma once
#include "MeshData.h"
#include <glad/gl.h>

// GPU mesh: one VAO with position(0), normal(1), uv(2). Uploaded once, drawn many times.
class Mesh {
public:
    explicit Mesh(const MeshData& data, GLenum mode = GL_TRIANGLES);
    ~Mesh();
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh(Mesh&& o) noexcept;

    void draw() const;
    void drawInstanced(int count) const;  // for the asteroid belt (stretch goal)
    GLuint vao() const { return vao_; }

private:
    GLuint vao_ = 0, vbo_ = 0, ebo_ = 0;
    GLsizei count_ = 0;
    GLenum mode_ = GL_TRIANGLES;
    bool indexed_ = false;
};
