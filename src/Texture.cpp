#define STB_IMAGE_IMPLEMENTATION
#include "Texture.h"
#include "Utils.h"
#include <stb_image.h>
#include <stdexcept>

static GLenum formatFor(int channels) { return channels == 4 ? GL_RGBA : channels == 3 ? GL_RGB : GL_RED; }

Texture::Texture(const std::string& path) {
    stbi_set_flip_vertically_on_load(true);  // image row 0 = top = v 1
    int w, h, ch;
    unsigned char* px = stbi_load(res::path(path).c_str(), &w, &h, &ch, 0);
    if (!px) throw std::runtime_error("cannot load texture " + path);
    glGenTextures(1, &id_);
    glBindTexture(GL_TEXTURE_2D, id_);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GLint(formatFor(ch)), w, h, 0, formatFor(ch), GL_UNSIGNED_BYTE, px);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(px);
}

Texture::~Texture() { glDeleteTextures(1, &id_); }

void Texture::bind(unsigned unit) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, id_);
}

Cubemap::Cubemap(const std::array<std::string, 6>& faces) {
    glGenTextures(1, &id_);
    glBindTexture(GL_TEXTURE_CUBE_MAP, id_);
    stbi_set_flip_vertically_on_load(false);
    for (unsigned i = 0; i < 6; ++i) {
        int w, h, ch;
        unsigned char* px = stbi_load(res::path(faces[i]).c_str(), &w, &h, &ch, 3);
        if (!px) throw std::runtime_error("cannot load cubemap face " + faces[i]);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, px);
        stbi_image_free(px);
    }
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
