#include "SaturnRings.h"
#include "Primitives.h"
#include <glm/gtc/matrix_inverse.hpp>

SaturnRings::SaturnRings()
    : shader_("shaders/planet.vert", "shaders/ring.frag"),
      mesh_(makeRing(256, 1.25f, 2.4f)),
      texture_("assets/textures/saturn_rings.png", GL_CLAMP_TO_EDGE) {}

void SaturnRings::draw(const SceneNode& saturn, const glm::mat4& view, const glm::mat4& projection,
                       const SceneLights& lights) const {
    shader_.use();
    // The same model gives the rings Saturn's centre, axial tilt and radius.
    shader_.set("uModel", saturn.model());
    shader_.set("uNormalMat", glm::inverseTranspose(glm::mat3(saturn.model())));
    shader_.set("uView", view);
    shader_.set("uProj", projection);
    lights.apply(shader_);
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
