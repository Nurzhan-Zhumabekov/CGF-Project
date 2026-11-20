#include "NormalMap.h"
#include <cmath>
#include <stdexcept>

std::vector<unsigned char> normalMapFromHeight(const unsigned char* rgba, int width, int height, float strength) {
    if (!rgba || width < 1 || height < 1 || !(strength >= 0.0f))
        throw std::invalid_argument("normal map needs an image of at least 1 x 1 texels and strength >= 0");
    const size_t texels = size_t(width) * size_t(height);

    std::vector<float> h(texels);
    for (size_t i = 0; i < texels; ++i) {
        const unsigned char* p = rgba + 4 * i;
        h[i] = (0.299f * p[0] + 0.587f * p[1] + 0.114f * p[2]) / 255.0f;
    }
    auto at = [&](int x, int y) {
        x = (x % width + width) % width;                    // wrap across the longitude seam
        y = y < 0 ? 0 : (y >= height ? height - 1 : y);     // clamp at the poles
        return h[size_t(y) * size_t(width) + size_t(x)];
    };

    std::vector<unsigned char> out(4 * texels);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float du = 0.5f * (at(x + 1, y) - at(x - 1, y));
            float dv = 0.5f * (at(x, y + 1) - at(x, y - 1));
            float nx = -strength * du, ny = -strength * dv, nz = 1.0f;
            float len = std::sqrt(nx * nx + ny * ny + nz * nz);
            unsigned char* o = &out[4 * (size_t(y) * size_t(width) + size_t(x))];
            o[0] = (unsigned char)std::lround((nx / len * 0.5f + 0.5f) * 255.0f);
            o[1] = (unsigned char)std::lround((ny / len * 0.5f + 0.5f) * 255.0f);
            o[2] = (unsigned char)std::lround((nz / len * 0.5f + 0.5f) * 255.0f);
            o[3] = 255;
        }
    }
    return out;
}
