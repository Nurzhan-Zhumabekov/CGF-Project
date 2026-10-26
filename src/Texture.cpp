#define STB_IMAGE_IMPLEMENTATION
#include "Texture.h"
#include "Utils.h"
#include <stb_image.h>
#include <memory>
#include <stdexcept>

using Image = std::unique_ptr<unsigned char, decltype(&stbi_image_free)>;

Texture::Texture(const std::string& path, GLenum wrapS) {
    stbi_set_flip_vertically_on_load(true);  // image row 0 = top = v 1
    int w, h, ch;
    // Normalize grayscale, gray+alpha, RGB and RGBA to the same upload format.
    Image px(stbi_load(res::path(path).c_str(), &w, &h, &ch, STBI_rgb_alpha), stbi_image_free);
    if (!px) throw std::runtime_error("cannot load texture " + path);
    glGenTextures(1, &id_);
    glBindTexture(GL_TEXTURE_2D, id_);
    GLint alignment;
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignment);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, px.get());
    glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

Texture::~Texture() { glDeleteTextures(1, &id_); }

void Texture::bind(unsigned unit) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, id_);
}

Cubemap::Cubemap(const std::array<std::string, 6>& faces) {
    stbi_set_flip_vertically_on_load(false);
    std::array<Image, 6> images = {{
        {nullptr, stbi_image_free}, {nullptr, stbi_image_free}, {nullptr, stbi_image_free},
        {nullptr, stbi_image_free}, {nullptr, stbi_image_free}, {nullptr, stbi_image_free}}};
    int size = 0;
    for (unsigned i = 0; i < 6; ++i) {
        int w, h, ch;
        images[i].reset(stbi_load(res::path(faces[i]).c_str(), &w, &h, &ch, STBI_rgb_alpha));
        if (!images[i]) throw std::runtime_error("cannot load cubemap face " + faces[i]);
        if (w != h || (i > 0 && w != size))
            throw std::runtime_error("cubemap faces must be square and have equal sizes: " + faces[i]);
        size = w;
    }
    // Validate all six images before allocating GPU storage (also safe on load failure).
    glGenTextures(1, &id_);
    glBindTexture(GL_TEXTURE_CUBE_MAP, id_);
    GLint alignment;
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignment);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    for (unsigned i = 0; i < 6; ++i)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA8, size, size, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, images[i].get());
    glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

Cubemap::~Cubemap() { glDeleteTextures(1, &id_); }

void Cubemap::bind(unsigned unit) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, id_);
}
