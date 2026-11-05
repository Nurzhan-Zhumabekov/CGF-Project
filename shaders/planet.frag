#version 330 core
in vec3 vWorldPos;
in vec3 vNormal;
in vec2 vUV;
out vec4 FragColor;

uniform sampler2D uDiffuse;
uniform sampler2D uNight;      // Earth city lights
uniform vec3 uViewPos;
uniform vec3 uSunPos;          // light 1: point light in the Sun
uniform vec3 uSunColor;
uniform vec3 uFillPos;         // light 2: lantern at the camera, toggled with L
uniform vec3 uFillColor;
uniform int uFillOn;
uniform int uEmissive;         // the Sun is drawn unlit
uniform int uUseNight;
uniform float uShininess;
uniform float uSpecStrength;

// Diffuse + specular (Blinn-Phong) of one light; ambient is added once in main().
vec3 blinnPhong(vec3 N, vec3 V, vec3 lightPos, vec3 lightColor, float attenuation, vec3 albedo) {
    vec3 L = normalize(lightPos - vWorldPos);
    vec3 H = normalize(L + V);
    float diff = max(dot(N, L), 0.0);
    float spec = diff > 0.0 ? pow(max(dot(N, H), 0.0), uShininess) * uSpecStrength : 0.0;
    return lightColor * attenuation * (albedo * diff + vec3(spec));
}

void main() {
    vec3 albedo = texture(uDiffuse, vUV).rgb;
    if (uEmissive == 1) {
        FragColor = vec4(albedo * 1.25, 1.0);
        return;
    }
    vec3 N = normalize(vNormal);
    vec3 V = normalize(uViewPos - vWorldPos);

    vec3 color = 0.06 * albedo;  // ambient
    float dSun = length(uSunPos - vWorldPos);
    color += blinnPhong(N, V, uSunPos, uSunColor, 1.0 / (1.0 + 0.0004 * dSun * dSun), albedo);
    if (uFillOn == 1) {
        float d = length(uFillPos - vWorldPos);
        color += blinnPhong(N, V, uFillPos, uFillColor, 1.0 / (1.0 + 0.02 * d * d), albedo);
    }
    if (uUseNight == 1) {  // stretch goal: city lights fade in as the surface turns away from the Sun
        vec3 L = normalize(uSunPos - vWorldPos);
        float night = 1.0 - smoothstep(-0.1, 0.2, dot(N, L));
        color += texture(uNight, vUV).rgb * night;
    }
    FragColor = vec4(color, 1.0);
}
