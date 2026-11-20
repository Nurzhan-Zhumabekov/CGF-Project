#pragma once
#include <vector>

// Tangent-space normal map built from a height field (no OpenGL, so it can be unit tested).
//
// The height of a texel is the luminance of an RGBA image, h = (0.299 r + 0.587 g + 0.114 b) / 255.
// Its slopes are central differences, wrapping around in u (the longitude seam of a planet map)
// and clamped in v (the poles):
//   dh/du = (h[x + 1] - h[x - 1]) / 2        dh/dv = (h[y + 1] - h[y - 1]) / 2
// The surface z = strength * h has the normal n = normalize(-strength * dh/du, -strength * dh/dv, 1)
// in tangent space (x along +u, y along +v, z out of the surface). It is stored as RGB = (n + 1) / 2,
// so a flat texel is (128, 128, 255), and alpha is 255.
//
// `rgba` has width * height texels, row 0 first; rows are kept in the same order, so the result
// lines up with the colour texture it was made from. Throws std::invalid_argument for an empty
// image, a too short buffer or a negative strength.
std::vector<unsigned char> normalMapFromHeight(const unsigned char* rgba, int width, int height, float strength);
