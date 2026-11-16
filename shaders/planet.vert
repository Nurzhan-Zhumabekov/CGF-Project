#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;
uniform mat3 uNormalMat;  // transpose(inverse(model)), computed on the CPU once per body

out vec3 vWorldPos;
out vec3 vNormal;
out vec2 vUV;
out vec3 vTint;  // colour multiplier; only the asteroid shader varies it

void main() {
    vec4 world = uModel * vec4(aPos, 1.0);
    vWorldPos = world.xyz;
    vNormal = normalize(uNormalMat * aNormal);
    vUV = aUV;
    vTint = vec3(1.0);
    gl_Position = uProj * uView * world;
}
