#pragma once
#include "MeshData.h"

// UV sphere of radius 1 with normals and UVs. u grows eastwards, v = 1 at the north pole (stb flip on load).
// Throws std::invalid_argument for stacks < 2 or sectors < 3.
MeshData makeSphere(int stacks, int sectors);
// Unit cube (36 vertices, no indices), used for the skybox.
MeshData makeCube();
// Unit circle in the XZ plane, drawn as GL_LINE_LOOP for orbit lines.
MeshData makeCircle(int segments);
