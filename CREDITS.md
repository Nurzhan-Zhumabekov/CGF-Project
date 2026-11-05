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

The script `tools/gen_textures.py` creates the planet textures and skybox in `assets/textures/` using procedural noise.
`tools/gen_ring_texture.py` creates `saturn_rings.png`, an original procedural RGBA radial profile
with bands, transparent gaps and soft edges. Ring geometry is generated in `makeRing`.
No third-party assets are included. Record the name, author, link and license here whenever an external texture or model is added.

## Tutorials and references

LearnOpenGL describes the skybox technique (`gl_Position = pos.xyww`, `GL_LEQUAL`) and Blinn-Phong lighting (Joey de Vries, https://learnopengl.com, CC BY-NC 4.0). The implementation was written from scratch rather than copied from the tutorial.
