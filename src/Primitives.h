#pragma once
#include "MeshData.h"

// UV sphere of radius 1. u grows eastwards, v = 1 at the north pole (stb flip on load).
MeshData makeSphere(int stacks, int sectors);
// Unit cube (36 vertices, no indices), used for the skybox.
MeshData makeCube();
// Unit circle in the XZ plane, drawn as GL_LINE_LOOP for orbit lines.
MeshData makeCircle(int segments);
// Annulus in the XZ plane. u runs from inner to outer radius, v around the ring.
MeshData makeRing(int segments, float innerRadius, float outerRadius);
