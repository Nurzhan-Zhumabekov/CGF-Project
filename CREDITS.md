# Credits

## Libraries (in `third_party/`)

| Library | Version | Author | License | Link |
|---|---|---|---|---|
| GLFW | 3.4 | Marcus Geelnard, Camilla Löwy and contributors | zlib/libpng | https://www.glfw.org |
| GLM | 1.0.1 | G-Truc Creation | MIT (Happy Bunny) | https://github.com/g-truc/glm |
| Dear ImGui | 1.90.1 | Omar Cornut | MIT | https://github.com/ocornut/imgui |
| stb_image | 2.30 | Sean Barrett | Public domain / MIT | https://github.com/nothings/stb |
| GLAD 2 (generated GL 3.3 core loader) | glad2 | David Herberth | MIT, Khronos specifications | https://github.com/Dav1dde/glad |

GLFW, GLM and Dear ImGui were obtained from Ubuntu packages at archive.ubuntu.com, pool/universe. The stb_image source comes from nothings/stb.

## Textures and assets

### Planet maps: Solar System Scope

Author: Solar System Scope (INOVE), https://www.solarsystemscope.com/textures/
License: Creative Commons Attribution 4.0 International (CC BY 4.0), https://creativecommons.org/licenses/by/4.0/
Changes: files renamed; images not modified. The maps are based on NASA elevation and imagery data.

| File in `assets/textures/` | Original file |
|---|---|
| `sun.jpg` | `2k_sun.jpg` |
| `mercury.jpg` | `2k_mercury.jpg` |
| `venus.jpg` | `2k_venus_atmosphere.jpg` |
| `earth.jpg` | `2k_earth_daymap.jpg` |
| `earth_night.jpg` | `2k_earth_nightmap.jpg` |
| `moon.jpg` (also used for the asteroids) | `2k_moon.jpg` |
| `mars.jpg` | `2k_mars.jpg` |
| `jupiter.jpg` | `2k_jupiter.jpg` |
| `saturn.jpg` | `2k_saturn.jpg` |
| `uranus.jpg` | `2k_uranus.jpg` |
| `neptune.jpg` | `2k_neptune.jpg` |

Each original is at `https://www.solarsystemscope.com/textures/download/<original file>`.

### Own assets

The skybox faces `sky_*.png` and `saturn_rings.png` (an RGBA radial profile with bands, transparent
gaps and soft edges) are original procedural images made by the team. The scripts that generated
them were removed from the repository because the project is C++ only; they remain in the Git
history under `tools/`. Ring geometry is generated in `makeRing` and the asteroid mesh in `makeRock`
(`src/AsteroidBeltData.cpp`); no third-party models are included.

Record the name, author, link and license here whenever an external texture or model is added.

## Tutorials and references

LearnOpenGL describes the skybox technique (`gl_Position = pos.xyww`, `GL_LEQUAL`) and Blinn-Phong lighting (Joey de Vries, https://learnopengl.com, CC BY-NC 4.0). The implementation was written from scratch rather than copied from the tutorial.

The asteroid belt follows the instancing technique described in the LearnOpenGL chapter "Instancing" (a `mat4` vertex attribute spread over four locations with `glVertexAttribDivisor`, drawn with `glDrawElementsInstanced`; same author and license). The code in `src/AsteroidBelt.cpp` and `shaders/asteroid.vert` was written for this project, not copied.

AI assistance is declared in the "AI use" section of `README.md`.
