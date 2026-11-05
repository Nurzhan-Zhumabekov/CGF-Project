#include "SaturnRings.h"
#include "Primitives.h"
#include <glm/gtc/matrix_inverse.hpp>

SaturnRings::SaturnRings()
    : shader_("shaders/planet.vert", "shaders/ring.frag"),
      mesh_(makeRing(256, 1.25f, 2.4f)),
      texture_("assets/textures/saturn_rings.png", GL_CLAMP_TO_EDGE) {}

void SaturnRings::draw(const SceneNode& saturn, const glm::mat4& view, const glm::mat4& projection,
                       const glm::vec3& sunPos, const glm::vec3& viewPos, bool fillLight) const {
    shader_.use();
    // The same model gives the rings Saturn's centre, axial tilt and radius.
    shader_.set("uModel", saturn.model());
    shader_.set("uNormalMat", glm::inverseTranspose(glm::mat3(saturn.model())));
    shader_.set("uView", view);
    shader_.set("uProj", projection);
    shader_.set("uSunPos", sunPos);
    shader_.set("uSunColor", glm::vec3(1.0f, 0.96f, 0.88f));
    shader_.set("uFillPos", viewPos);
    shader_.set("uFillColor", glm::vec3(0.35f, 0.45f, 0.65f));
    shader_.set("uFillOn", int(fillLight));
    shader_.set("uSaturnPos", saturn.position());
    shader_.set("uSaturnRadius", saturn.params().radius);
    shader_.set("uDiffuse", 0);
    texture_.bind(0);

    const GLboolean cull = glIsEnabled(GL_CULL_FACE), blend = glIsEnabled(GL_BLEND);
    GLboolean depthMask;
    GLint srcRGB, dstRGB, srcAlpha, dstAlpha, equationRGB, equationAlpha;
    glGetBooleanv(GL_DEPTH_WRITEMASK, &depthMask);
    glGetIntegerv(GL_BLEND_SRC_RGB, &srcRGB);
    glGetIntegerv(GL_BLEND_DST_RGB, &dstRGB);
    glGetIntegerv(GL_BLEND_SRC_ALPHA, &srcAlpha);
    glGetIntegerv(GL_BLEND_DST_ALPHA, &dstAlpha);
    glGetIntegerv(GL_BLEND_EQUATION_RGB, &equationRGB);
    glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &equationAlpha);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);  // retain depth testing against Saturn, but do not hide the background
    mesh_.draw();
    glDepthMask(depthMask);
    glBlendFuncSeparate(srcRGB, dstRGB, srcAlpha, dstAlpha);
    glBlendEquationSeparate(equationRGB, equationAlpha);
    if (!blend) glDisable(GL_BLEND);
    if (cull) glEnable(GL_CULL_FACE);
}
