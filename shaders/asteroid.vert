#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;
// Per-instance attributes (divisor 1): one value for each asteroid instead of each vertex.
layout(location = 3) in mat4 aInstance;  // model matrix, takes locations 3-6
layout(location = 7) in vec4 aTumble;    // xyz: spin axis, w: whole turns per tumble period
layout(location = 8) in vec3 aTint;      // colour multiplier

uniform mat4 uBelt;     // rotation of the whole belt about the Sun
uniform float uTumble;  // fraction of the tumble period, [0, 1)
uniform mat4 uView;
uniform mat4 uProj;

out vec3 vWorldPos;
out vec3 vNormal;
out vec2 vUV;
out vec3 vTint;
out vec3 vTangent;  // the asteroids have no normal map, but planet.frag reads this varying

// Rodrigues' formula: v turned by angle about the unit axis.
vec3 turn(vec3 v, vec3 axis, float angle) {
    return v * cos(angle) + cross(axis, v) * sin(angle) + axis * dot(axis, v) * (1.0 - cos(angle));
}

void main() {
    // The rock spins in its own frame first; aTumble.w is a whole number, so uTumble wrapping
    // from 1 back to 0 changes the angle by full turns only.
    float angle = 6.28318530718 * aTumble.w * uTumble;
    mat4 model = uBelt * aInstance;
    vec4 world = model * vec4(turn(aPos, aTumble.xyz, angle), 1.0);
    vWorldPos = world.xyz;
    // The instance matrices are rotation * uniform scale, so mat3(model) differs from the
    // inverse-transpose only by a constant factor, which normalize() removes.
    vNormal = normalize(mat3(model) * turn(aNormal, aTumble.xyz, angle));
    vUV = aUV;
    vTint = aTint;
    vTangent = vec3(0.0);
    gl_Position = uProj * uView * world;
}
