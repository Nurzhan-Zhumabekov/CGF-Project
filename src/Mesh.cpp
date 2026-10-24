#include "Mesh.h"
#include <cstddef>

Mesh::Mesh(const MeshData& d, GLenum mode) : mode_(mode), indexed_(!d.indices.empty()) {
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, GLsizeiptr(d.vertices.size() * sizeof(Vertex)), d.vertices.data(), GL_STATIC_DRAW);
    if (indexed_) {
        glGenBuffers(1, &ebo_);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, GLsizeiptr(d.indices.size() * sizeof(unsigned)), d.indices.data(), GL_STATIC_DRAW);
    }
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
    glBindVertexArray(0);
    count_ = GLsizei(indexed_ ? d.indices.size() : d.vertices.size());
}

Mesh::Mesh(Mesh&& o) noexcept
    : vao_(o.vao_), vbo_(o.vbo_), ebo_(o.ebo_), count_(o.count_), mode_(o.mode_), indexed_(o.indexed_) {
    o.vao_ = o.vbo_ = o.ebo_ = 0;
}

Mesh::~Mesh() {
    glDeleteBuffers(1, &vbo_);
    glDeleteBuffers(1, &ebo_);
    glDeleteVertexArrays(1, &vao_);
}

void Mesh::draw() const {
    glBindVertexArray(vao_);
    if (indexed_) glDrawElements(mode_, count_, GL_UNSIGNED_INT, nullptr);
    else glDrawArrays(mode_, 0, count_);
}

void Mesh::drawInstanced(int n) const {
    glBindVertexArray(vao_);
    if (indexed_) glDrawElementsInstanced(mode_, count_, GL_UNSIGNED_INT, nullptr, n);
    else glDrawArraysInstanced(mode_, 0, count_, n);
}
