#version 330 core
in vec3 vWorldPos;
in vec3 vNormal;
in vec2 vUV;
out vec4 FragColor;

uniform sampler2D uDiffuse;
uniform vec3 uSunPos;
uniform vec3 uSunColor;
uniform vec3 uFillPos;
uniform vec3 uFillColor;
uniform int uFillOn;
uniform vec3 uSaturnPos;
uniform float uSaturnRadius;

// Test the segment from this fragment to the light against Saturn's sphere.
float visibility(vec3 lightPos) {
    vec3 toLight = lightPos - vWorldPos;
    float distanceToLight = length(toLight);
    vec3 L = toLight / max(distanceToLight, 0.0001);
    vec3 toCenter = uSaturnPos - vWorldPos;
    float along = dot(toCenter, L);
    float perpendicular2 = max(dot(toCenter, toCenter) - along * along, 0.0);
    float radius2 = uSaturnRadius * uSaturnRadius;
    if (perpendicular2 >= radius2) return 1.0;
    float halfChord = sqrt(radius2 - perpendicular2);
    return along + halfChord > 0.0 && along - halfChord < distanceToLight ? 0.0 : 1.0;
}

vec3 lighting(vec3 N, vec3 lightPos, vec3 lightColor, float falloff) {
    vec3 toLight = lightPos - vWorldPos;
    float distance2 = dot(toLight, toLight);
    // Thin particles scatter light on both sides of the ring plane.
    float diffuse = abs(dot(N, normalize(toLight)));
    return lightColor * diffuse * visibility(lightPos) / (1.0 + falloff * distance2);
}

void main() {
    vec4 texel = texture(uDiffuse, vUV);
    if (texel.a < 0.005) discard;
    vec3 N = normalize(vNormal);
    vec3 light = vec3(0.06) + lighting(N, uSunPos, uSunColor, 0.0004);
    if (uFillOn == 1) light += lighting(N, uFillPos, uFillColor, 0.02);
    FragColor = vec4(texel.rgb * light, texel.a);
}
