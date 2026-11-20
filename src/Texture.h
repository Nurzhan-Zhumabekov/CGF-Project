#pragma once
#include <array>
#include <glad/gl.h>
#include <string>

// 2D texture with generated mipmaps. Loaded once at start-up, never per frame.
class Texture {
public:
    explicit Texture(const std::string& path, GLenum wrapS = GL_REPEAT);
    // Tangent-space normal map computed at start-up from the luminance of the image at `path`
    // (see NormalMap.h). Rows are flipped like the colour texture, so both share the same UVs.
    static Texture normalMapFromImage(const std::string& path, float strength);
    ~Texture();
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
    Texture(Texture&& o) noexcept : id_(o.id_) { o.id_ = 0; }
    void bind(unsigned unit) const;

private:
    Texture() = default;
    void upload(int width, int height, const unsigned char* rgba, GLenum wrapS);
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
