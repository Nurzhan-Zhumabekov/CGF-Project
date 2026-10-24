# Solar System Explorer

Solar System Explorer is an interactive 3D model built with C++17 and OpenGL 3.3 core for the Computer Graphics Fundamentals final project at Astana IT University.

Team: Nurzhan Zhumabekov, Muzaffar Kapezov, Murat Narynbekov, Aknur Ondasyn.

## Build

Build with a C++17 compiler and CMake 3.10+. The repository includes GLFW, GLAD, GLM, Dear ImGui and stb_image in `third_party/`; these libraries need no separate installation.

```
cmake -S . -B build
cmake --build build --config Release
```

The build copies shaders and textures beside the executable. Run it with:

- Linux, macOS: `./build/solar_system`
- Windows (Visual Studio): `build\Release\solar_system.exe`

On Linux, install the X11 development headers: `sudo apt install build-essential cmake libgl-dev libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev`.

Run the math and scene-graph unit tests with `ctest --test-dir build` or `./build/core_tests`.

## Controls

| Key | Action |
|---|---|
| 1-8 | Camera follows Mercury ... Neptune |
| 9 | Camera follows the Moon |
| 0 or F | Free camera |
| W A S D, Q E | Fly forward, left, back, right, down, up (Shift = faster) |
| Right mouse button + drag | Look around |
| Mouse wheel | Distance to the followed body |
| [ and ] | Time slower / faster by 2x |
| Space | Pause |
| L | Second light (lantern at the camera) |
| O | Orbit lines |
| H | Help in the panel |
| Esc | Quit |

The ImGui panel also provides time-speed and VSync controls, along with the scene toggles. Both the panel and the window title show FPS.

## Features

- Custom shaders in `shaders/*.vert` and `*.frag`, using VAO/VBO/EBO without fixed-function calls (R1).
- Sun, 8 planets and the Moon as scene objects (R2).
- Scene graph: the Moon is a child of the Earth; model/view/projection matrices are built in code with GLM (R3).
- Per-fragment Blinn-Phong with ambient, diffuse and specular terms. Two lights: a point light in the Sun and a switchable lantern at the camera (R4).
- Textures with mipmaps and a cubemap skybox (R5).
- Free-fly and follow cameras, five interactions (R6).
- FPS in the window title and in the panel (R7).
- Stretch goal: the Earth blends day and night textures by light direction.

Saturn's transparent rings and an instanced asteroid belt are planned additions.

## Structure

```
src/         main, App (loop and rendering), Window, Shader, Mesh, Texture, Skybox, UI,
             Camera, SceneNode, SolarSystem, Primitives
shaders/     planet, skybox, line
assets/      textures (PNG)
tools/       gen_textures.py: procedural placeholder textures
tests/       OpenGL-free tests for geometry, scene graph and camera
third_party/ GLFW, GLAD, GLM, Dear ImGui, stb_image
docs/        team guide
```

## Textures

The PNG files currently use procedural-noise placeholders from `tools/gen_textures.py`. Replace a texture by keeping its filename in `assets/textures/` and recording the new source in `CREDITS.md`.

## Team responsibilities

Planned areas of responsibility:

| Member | Area |
|---|---|
| Nurzhan Zhumabekov | Build, window, shaders, meshes, FPS, integration |
| Muzaffar Kapezov | Scene, hierarchy, orbits, time |
| Murat Narynbekov | Lighting, textures, skybox, Earth day/night, rings |
| Aknur Ondasyn | Camera, interface, asteroid belt |
