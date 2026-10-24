#pragma once
#include <array>
#include <glad/gl.h>
#include <string>

// 2D texture with generated mipmaps. Loaded once at start-up, never per frame.
class Texture {
public:
    explicit Texture(const std::string& path);
    ~Texture();
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
    Texture(Texture&& o) noexcept : id_(o.id_) { o.id_ = 0; }
    void bind(unsigned unit) const;

private:
    GLuint id_ = 0;
};

// Six faces in the order +X, -X, +Y, -Y, +Z, -Z.
class Cubemap {
public:
    explicit Cubemap(const std::array<std::string, 6>& faces);
    ~Cubemap();
    Cubemap(const Cubemap&) = delete;
    Cubemap& operator=(const Cubemap&) = delete;
    void bind(unsigned unit) const;

private:
    GLuint id_ = 0;
};
