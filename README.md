# Solar System Explorer

Solar System Explorer is an interactive 3D model built with C++17 and OpenGL 3.3 core for the Computer Graphics Fundamentals final project at Astana IT University.

Team: Nurzhan Zhumabekov, Muzaffar Kapezov, Murat Narynbekov, Aknur Ondasyn.

Demo video: https://youtu.be/QA6Noyjfowk

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

Run the unit tests with `ctest --test-dir build` (add `-C Release` with Visual Studio), or run `core_tests` (math, scene graph, time) and `belt_tests` (camera follow mode, asteroid belt data) from the build directory.

## Controls

| Key | Action |
|---|---|
| 1-8 | Camera follows Mercury ... Neptune |
| 9 | Camera follows the Moon, riding along its orbit with the Earth behind it |
| 0 or F | Free camera |
| W A S D, Q E | Fly forward, left, back, right, down, up (Shift = faster) |
| Right mouse button + drag | Look around (the cursor is captured while the button is held) |
| Mouse wheel | Distance to the followed body (stops just above its surface) |
| [ and ] | Time slower / faster by 2x |
| Space | Pause |
| L | Second light (lantern at the camera) |
| O | Orbit lines |
| B | Asteroid belt |
| H | Help in the panel |
| N | Earth relief (normal map) on / off |
| V | VSync on / off |
| Esc | Quit |

The ImGui panel also provides time-speed and VSync controls, the scene toggles and a slider for the number of asteroids drawn (0-5000). Both the panel and the window title show FPS. Keyboard shortcuts are ignored while a panel field is being edited, and the mouse does not move the camera while the cursor is over the panel.

## Features

- Custom shaders in `shaders/*.vert` and `*.frag`, using VAO/VBO/EBO without fixed-function calls (R1).
- Sun, 8 planets and the Moon as scene objects (R2).
- Scene graph: the Moon is a child of the Earth; model/view/projection matrices are built in code with GLM (R3).
- Per-fragment Blinn-Phong with ambient, diffuse and specular terms. Two lights: a point light in the Sun and a switchable lantern at the camera (R4).
- Textures with mipmaps and a cubemap skybox (R5).
- Free-fly and follow cameras, six interactions: focus a body, time speed, pause, second light, orbit lines, asteroid belt (R6). Camera motion is scaled by the frame time, and the follow camera stays locked on its target at any time speed.
- FPS in the window title and in the panel (R7).
- Planet spins are slowed on screen when they are fast (Earth: 30 turns/s at the default speed), so they turn smoothly instead of flickering; the Moon keeps its exact, tidally locked angle. Venus and Uranus spin retrograde.
- Stretch goal: the Earth blends day and night textures by light direction.
- Stretch goal: normal mapping on the Earth. A tangent-space normal map is computed at start-up from the
  brightness of the Earth map (central differences, `src/NormalMap.*`), and the fragment shader turns it into
  world space with the tangent frame of the sphere (east, north, up). `N` switches the relief on and off.
- Stretch goal: Saturn has two-sided, textured transparent rings with its axial tilt,
  radial gaps and a shadow cast by the planet. Rings use both scene lights and render
  after the opaque bodies and skybox, with depth testing but without depth writes.
- Stretch goal: an asteroid belt between Mars and Jupiter drawn with instancing. One rock mesh and
  one `glDrawElementsInstanced` call draw up to 5000 asteroids; the per-asteroid model matrices are
  computed once at start-up and stored in a static buffer read with `glVertexAttribDivisor`.
  The same buffer holds a spin axis and a tint per rock; the orbital motion and the tumbling are
  two uniforms, so nothing is uploaded per frame.

## Structure

```
src/         main, App (loop and rendering), Window, Shader, Mesh, Texture, Skybox, SaturnRings, UI,
             Camera, SceneNode, SolarSystem, Primitives, AsteroidBelt (instanced drawing),
             AsteroidBeltData (rock mesh and placement, no OpenGL), Lights (light values shared by all shaders)
shaders/     planet, ring, skybox, line, asteroid (instanced vertex shader)
assets/      textures (planet maps JPEG, skybox and ring PNG)
tests/       OpenGL-free tests for geometry, scene graph, time, camera and asteroid belt data
third_party/ GLFW, GLAD, GLM, Dear ImGui, stb_image
docs/        team guide
```

## Textures

The Sun, the eight planets, the Moon and the Earth night map are 2048x1024 equirectangular maps from
Solar System Scope (CC BY 4.0), stored as JPEG in `assets/textures/` under the body's name
(`earth.jpg`, `earth_night.jpg`, ...). The asteroids reuse `moon.jpg` with a per-rock tint.
`CREDITS.md` lists the source, author and license of every file.

The skybox faces (`sky_*.png`) and `saturn_rings.png` are the team's own procedural images.
`saturn_rings.png` is a radial RGBA profile: its horizontal axis runs from the inner to the outer
radius and alpha controls the gaps and density.

The repository contains C++ and GLSL only; every image is committed, so nothing has to be generated
to build or run. Resource-only edits are copied on each build, including builds of the
`solar_system` target.

## Lighting and rings verification

- Press `6` to follow Saturn, then `Space` to pause. The far part of the rings is hidden
  by Saturn, the near part crosses in front, and stars remain visible through the gaps.
- Toggle `L`: both Saturn and its rings respond to the camera lantern. With it off,
  Saturn casts a dark shadow on the rings on the side away from the Sun.
- Press `3` to follow Earth. Unpause and lower time speed to inspect the smooth transition
  between the lit surface and emissive night lights. `L` does not change the Sun-based night mask.
- Use RMB + drag to inspect both sides of the rings and the skybox; the sky follows
  camera rotation but not translation. `0` returns to free flight.
- Run `ctest --test-dir <build-directory> --output-on-failure`. Geometry tests cover
  the ring's central hole, winding, UV seam and invalid parameters in addition to existing tests.

## Camera and asteroid belt verification

- Press `9`: the camera swings round behind the Moon and then rides along its orbit, so the Moon
  stays in the centre with the Earth behind it. Slow time down with `[` to watch night fall over
  the continents; RMB + drag picks another angle, which is then kept.
- Press `5`, then roll the mouse wheel forward as far as it goes: the camera stops above Jupiter's
  surface instead of passing through it.
- Press `1` ... `8` in turn: the camera glides from one planet to the next without a jump.
- Press `V` (VSync off) and compare the FPS with the belt on and off (`B`), and with the
  Asteroids slider at 0 and at 5000. Camera speed and the glide between planets do not change with FPS.
- Press `4` to follow Mars: the belt is the band of rocks outside its orbit. It turns slowly around
  the Sun, every rock tumbles about its own axis and has its own shade, and the rocks are lit by
  the Sun and by the camera lantern (`L`) like the planets.
- Hold RMB and drag far past the window edge: the camera keeps turning, and the cursor reappears
  where it was when the button is released.

## AI use

The team designed the application and wrote most of the code: the build, window and shader classes, the scene graph, orbits and simulation time, the lighting, textures, skybox, Earth day/night blending and Saturn’s rings. An AI coding assistant was used for selected parts: the first version of the asteroid belt and of the follow camera with their tests, normal mapping for the Earth, and a fix of the planet spin direction and flicker. It also helped with the report (parts of the literature review, references and screenshots), the demo video recording and the slide deck. The team reviewed, built and tested every AI-written part.

## Team responsibilities

Planned areas of responsibility:

| Member | Area |
|---|---|
| Nurzhan Zhumabekov | Build, window, shaders, meshes, FPS, integration |
| Muzaffar Kapezov | Scene, hierarchy, orbits, time |
| Murat Narynbekov | Lighting, textures, skybox, Earth day/night, rings |
| Aknur Ondasyn | Camera, interface, asteroid belt |
