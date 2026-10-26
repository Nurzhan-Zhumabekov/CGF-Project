#include "Skybox.h"
#include "Primitives.h"

static std::array<std::string, 6> faces() {
    return {"assets/textures/sky_px.png", "assets/textures/sky_nx.png", "assets/textures/sky_py.png",
            "assets/textures/sky_ny.png", "assets/textures/sky_pz.png", "assets/textures/sky_nz.png"};
}

Skybox::Skybox() : cubemap_(faces()), cube_(makeCube()), shader_("shaders/skybox.vert", "shaders/skybox.frag") {}

void Skybox::draw(const glm::mat4& view, const glm::mat4& projection) const {
    GLint depthFunc;
    GLboolean depthMask;
    glGetIntegerv(GL_DEPTH_FUNC, &depthFunc);
    glGetBooleanv(GL_DEPTH_WRITEMASK, &depthMask);
    const GLboolean cull = glIsEnabled(GL_CULL_FACE);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);
    shader_.use();
    shader_.set("uView", glm::mat4(glm::mat3(view)));
    shader_.set("uProj", projection);
    shader_.set("uSky", 0);
    cubemap_.bind(0);
    cube_.draw();
    glDepthFunc(depthFunc);
    glDepthMask(depthMask);
    if (cull) glEnable(GL_CULL_FACE);
}
