# Solar System Explorer: team guide

Team: Nurzhan Zhumabekov, Muzaffar Kapezov, Murat Narynbekov, Aknur Ondasyn.
Course: Computer Graphics Fundamentals, instructor Nazira Shaimerdenova.
The final project accounts for 40% of the course grade. Submit it on Moodle 3 days before the defense, no later than 23:59 (Asia/Almaty).

Deliver Solar System Explorer in C++ / OpenGL 3.3 core with R1-R8 and three stretch goals. The submission also includes a report using the course template, 8-12 slides and a 3-6 minute demo video.

## 1. Project requirements

Core features:
- The Sun as a light source.
- 8 textured planets orbiting with a hierarchy (Earth -> Moon).
- A skybox.
- Keys 1-8 focus the camera on a planet.
- Time-speed control.

Stretch goals (each working one gives 4 points in A5, maximum 8):
1. The Earth blends day and night textures by light direction.
2. Saturn's rings with transparency.
3. An asteroid belt with instancing.

End the video with the camera following the Moon around the Earth as night falls over the continents.

Mandatory R1-R8 (a project missing any item is capped at 60% for Technical depth):

| R | Requirement |
|---|---|
| R1 | OpenGL 3.3 core, VAO/VBO/EBO, own vertex/fragment shaders, no glBegin/glLight |
| R2 | At least 5 objects in the 3D scene |
| R3 | Own model/view/projection matrices, a hierarchy with a parent-child relation |
| R4 | Per-fragment Phong or Blinn-Phong: ambient, diffuse, specular; 2+ lights, one of which moves or can be toggled |
| R5 | Textures with correct UVs and mipmaps, or a cubemap, normal map or procedural texture |
| R6 | FPS or orbit camera (keyboard and mouse) and at least 2 scene interactions. Full marks in C2 need 3+ |
| R7 | Stable 30+ FPS at 1280x720 on the TA machine, on-screen FPS counter. Full marks in B3 need 60+ |
| R8 | Builds and runs the same way as the course assignments, README lists the exact build steps |

Permitted libraries are GLFW or SDL2, GLAD or GLEW, GLM, stb_image, tinyobjloader, Dear ImGui, miniaudio or OpenAL. Bundle any library absent from the course assignments and build it with the project. Game engines and high-level renderers are prohibited.

## 2. Grading and submission

1. Build. The assumed setup is CMake with GLFW, GLAD and GLM, as in LearnOpenGL. Verify it against the course assignments in step 0 and adapt the project if needed. R8 and D1 (3 points) require the same build procedure.
2. Performance. FPS will be measured on the TA machine. Leave room for the asteroid belt and large textures; target 60+ FPS on the team's laptops.
3. Licenses. List the author, link and license of each texture, model and tutorial snippet in CREDITS.md. Missing credits can cost 10 points or reduce the project grade to 0, with an academic integrity report.
4. Code knowledge. During the defense, the instructor may ask any member to explain any code section or run the program. A6, F1, F2 and F4 apply individually. An inability to explain the code raises an integrity concern.
5. Commit history. D4 (2 points) requires regular contributions from every member throughout the project. Use individual accounts and avoid leaving all work to a single final commit.
6. Presentation. Section F is graded individually and carries 10 points. Sections A-E and G share a team grade.
7. Peer evaluation. Confidential feedback within the team may adjust individual scores.
8. Deadlines. Send the topic and team list to n.shaimerdenova@astanait.edu.kz in MS Teams within 3 days of the document's release. Progress checks fall in weeks 7, 8 and 9; final submission is 3 days before the defense. A delay of up to 24 h costs 10%; 24-48 h costs 20%. Later submissions require prior written approval or receive 0.

## 3. Proposed responsibilities

| Member | Area | Files | Criteria |
|---|---|---|---|
| Nurzhan Zhumabekov | Engine and build: CMake, window, main loop, Shader, Mesh, FPS counter, integration, README | `CMakeLists.txt`, `src/main.cpp`, `src/App.*`, `src/Shader.*`, `src/Mesh.*`, `src/Window.*` | R1, R7, R8, A1, B3, B4, D1, D2, D3 |
| Muzaffar Kapezov | Scene and hierarchy: SceneNode, sphere generator, data for the 8 planets and the Moon, orbits, time | `src/SceneNode.*`, `src/SolarSystem.*`, `src/Primitives.*` | R2, R3, A2 |
| Murat Narynbekov | Light and textures: Blinn-Phong shader, Sun + second light, Texture (mipmaps), Skybox, Earth day/night, Saturn's rings | `shaders/*.vert`, `shaders/*.frag`, `src/Texture.*`, `src/Skybox.*` | R4, R5, A3, A4, B1, B2 |
| Aknur Ondasyn | Camera and interface: Camera (free + follow), keys 1-8, ImGui panel, on-screen help, asteroid belt (instancing) | `src/Camera.*`, `src/UI.*`, new `src/AsteroidBelt.*` | R6, C1, C2, C3, B4 |

Each member writes and presents their report sections and must understand the relevant formulas. Section 8 assigns the report sections and slides.

## 4. First-day setup for all four members

1. Confirm the topic registration. Nurzhan sends "Solar System Explorer" and the surname, name and group of all four members to the instructor in MS Teams. Keep the reply as confirmation. A maximum of 2 teams can take the same topic; choose another immediately if both places are taken.
2. Open your previous assignments and check how they were built (CMake? Visual Studio? Makefile?) and which libraries they used. Post it in the chat. Result: one line "build: ...". If it is not CMake, adapt the project structure to it.
3. Clone the repository:
   ```
   git clone https://github.com/Nurzhan-Zhumabekov/CGF-Project
   cd CGF-Project
   git config user.name "Your Name"
   git config user.email "email linked to your GitHub"
   ```
   Result: `git log` shows the shared history. If push is rejected, the repository owner (Nurzhan) adds you under Settings -> Collaborators.
4. Nurzhan adds the TA to the repository as a member (Settings -> Collaborators). This is a requirement of the assignment.
5. Install a C++17 compiler and CMake 3.10+ (Windows: Visual Studio 2022 or MinGW; Linux: `sudo apt install build-essential cmake libgl-dev libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev`; macOS: `xcode-select --install` and `brew install cmake`). Check: `cmake --version` and `g++ --version` print versions.
6. Build the skeleton: `cmake -S . -B build && cmake --build build` and run `./build/solar_system`. Result: a window with the Sun, planets, orbit lines and an ImGui panel. If it fails, post the full error in the chat.

## 5. Work order by role

### 5.1 Nurzhan: engine (weeks 1-2, the others depend on it)

1. Keep the libraries in `third_party/` (GLFW, GLM, Dear ImGui, stb_image.h, GLAD generated for 3.3 core). Nothing has to be installed on the TA machine.
2. `CMakeLists.txt`: build the `solar_system` executable from `src/*.cpp`, copy `shaders/` and `assets/` next to the executable.
3. Window 1280x720, GL 3.3 core, GLAD, main loop, `deltaTime`, FPS in the window title (updated every 0.5 s).
4. Classes `Shader` (compile from `.vert`/`.frag`, print the error log) and `Mesh` (VAO/VBO/EBO).
5. Check: `cmake -S . -B build && cmake --build build -j && ./build/solar_system` shows the scene with FPS in the title.
6. If the build fails: read the CMake log; the usual causes are missing X11 packages (Linux) or no generator selected (Windows). Post the whole error in the chat.
7. Commit and push to `main`. The others pull and continue.

Done when: every one of the four builds the project from scratch following the README with one command.

### 5.2 Muzaffar: scene and hierarchy (weeks 1-3)

1. `makeSphere`: stacks/sectors generator with positions, normals, UVs. Normals = normalized positions.
2. `SceneNode`: local matrix, list of children, `worldMatrix = parent.world * local`. The Earth is the parent of the Moon.
3. Body data: radius, orbit distance, orbital period, rotation period, axial tilt. Scales are stylized, not real (otherwise Neptune is not visible).
4. Time: `simTime += dt * timeScale`, orbit angle = `2π * simTime / period`. Time speed: keys `[` and `]`, pause on Space.
5. Check: 8 planets and the Moon move around the Sun and their axes; doubling `timeScale` doubles all speeds.
6. If planets "jump" when the speed changes, you compute the angle as `angle += speed` without `dt` or recompute the phase. Keep the accumulated `simTime`.

Done when: the Earth -> Moon hierarchy works and all 8 planets are distinguishable.

### 5.3 Murat: light, textures, skybox (weeks 2-4)

1. Blinn-Phong in the fragment shader: `ambient + diffuse * max(dot(N,L),0) + specular * pow(max(dot(N,H),0), shininess)`.
2. Light 1: point light at the Sun (the Sun itself is drawn emissive, unlit). Light 2: a weak fill lantern at the camera, toggled with `L` (R4 needs a moving or switchable second source).
3. Normals under non-uniform scale: `mat3(transpose(inverse(model)))`. Compute it on the CPU and pass a uniform; this is also B4.
4. `Texture`: `stbi_load`, `glGenerateMipmap`, `GL_LINEAR_MIPMAP_LINEAR`, `GL_REPEAT`. Load once at start-up, never per frame.
5. Skybox: cubemap from 6 faces, shader with `gl_Position = pos.xyww` and `glDepthFunc(GL_LEQUAL)`, view matrix without translation.
6. Stretch goals: the Earth uses two samples (day and night) and `mix(night, day, smoothstep(-0.1, 0.2, dot(N,L)))` (already in the skeleton). Saturn's rings: a flat disc with an alpha texture, `glEnable(GL_BLEND)`, drawn after the opaque objects.
7. Planet textures: download manually from solarsystemscope.com/textures (the site states a CC BY 4.0 license, check it on the page before use) and add them to `CREDITS.md`. Use the 2k versions. If the site is unavailable, look for NASA sources and check the license.
8. Check: the Sun lights the side of each planet that faces it, the far side is dark but not black (ambient), a highlight is visible on the oceans, no seams on the sphere.
9. If a texture is stretched or rotated, check `u = sector / sectors` in the sphere and `stbi_set_flip_vertically_on_load(true)`. If half a planet is black, the normals are not normalized.

Done when: every planet has its own texture and the skybox and both lights are visible.

### 5.4 Aknur: camera, interface, asteroid belt (weeks 2-5)

1. `Camera`: free-fly mode (WASD + mouse, scaled by `deltaTime` so speed does not depend on FPS: C1) and follow mode.
2. Keys `1`-`8` follow a planet (orbit camera around its world position, the mouse wheel changes the distance). Key `0` returns to free fly. Key `9` follows the Moon (for the Wow moment).
3. Mouse: hold the right button to look around (do not react while the cursor is over the ImGui panel).
4. ImGui panel: FPS, time speed (slider), checkboxes "orbit lines", "second light", "pause", list of keys (C3: on-screen help).
5. At least 3 meaningful interactions (for C2): focus a planet 1-8, time speed, toggle the second light or the orbit lines.
6. Stretch: asteroid belt. One mesh + `glDrawElementsInstanced`, matrices in a buffer with `glVertexAttribDivisor`, 2000-5000 rocks between Mars and Jupiter. Compute the matrices once at start-up.
7. Check: the camera moves smoothly between planets without jerks, the speed is the same at 30 and 144 FPS (check with `glfwSwapInterval(0)` and `(1)`).
8. If the camera "flies away" while following, use the planet's world matrix, not the local one.

Done when: keys 1-9 and the panel work, the belt is visible and keeps FPS above 60 on your machine.

## 6. Git workflow for D4

1. Branches: `feature/engine` (Nurzhan), `feature/scene` (Muzaffar), `feature/lighting` (Murat), `feature/camera-ui` (Aknur). Create with `git checkout -b feature/name`.
2. Commit at least 2-3 times a week in small pieces. Messages: what was done, one line.
3. Merge into `main` through a Pull Request, a second person reads the diff. Before the PR: `git pull origin main`, the build passes.
4. Do not commit `build/`, binaries or IDE files. `.gitignore` is already in the repository.
5. Shaders only in separate `.vert`/`.frag` files, no large commented-out blocks, no file longer than about 500 lines (D2).
6. Merge conflict: open the file, keep both needed parts, `git add`, `git commit`. If it does not work out, do not delete someone else's code, ask the author.

## 7. Schedule

Set calendar dates from the course schedule for the milestones below.

| When | What is ready | Who |
|---|---|---|
| 3 days after release | Topic and members registered | Nurzhan |
| Week 6 | Step 0, window, camera, first sphere | Nurzhan, Aknur, Muzaffar |
| Week 7 (check) | Window, camera, first objects, Sun and planets | everyone |
| Week 8 (check) | Light, textures, skybox, interaction | Murat, Aknur |
| Week 9 (check) | Almost finished project, stretch goals | everyone |
| 10 days before submission | Code freeze, only bug fixes after this | everyone |
| 7-5 days before | Report, slides, video | see below |
| 3 days before, by 23:59 | Moodle submission: repository link with tag v1.0, report PDF, slides, video | Nurzhan |

## 8. Report, presentation, video

Use `Report_template` for the report: Introduction, Literature Review, Project Description, Technical Implementation, Graphics and Rendering, Testing and Evaluation (including the FPS table and hardware), Conclusion and Future Work, References.

| Section | Author |
|---|---|
| Introduction, Project Description, Conclusion, assembling and formatting the report | Nurzhan |
| Technical Implementation (hierarchy, matrices, time), formulas | Muzaffar |
| Graphics and Rendering (Blinn-Phong, textures, skybox, screenshots of every feature) | Murat |
| Testing and Evaluation (FPS table, camera, interactions, bugs) | Aknur |
| Literature Review: 15+ Q1-Q2 sources, 4 per person, compare them, do not just list them | everyone |

Check each source in Scopus, IEEE Xplore or ACM DL and use a consistent IEEE or APA reference style. Give every figure a caption and a reference in the text, such as "Figure 1 illustrates...".

Prepare 8-12 slides for a 7-10 minute presentation in which every member speaks. Cover the problem, rendering pipeline, architecture, one key shader, results and conclusions. Explain each part without reading the slides and be ready to discuss the formulas.

Record a 3-6 minute demo at 1080p with the FPS counter visible throughout the real-time screen capture. Name each feature in the voice-over or captions and demonstrate R1-R7 and every stretch goal. Finish with the camera following the Moon while the Earth enters night. Murat records and edits the video; everyone contributes to the script.

## 9. Submission checklist

- [ ] A fresh clone builds following the README without manual library installation.
- [ ] Window 1280x720, FPS in the title, 60+ FPS (at least 30) on the weakest laptop in the team.
- [ ] 8 planets and the Moon with textures, the Moon orbits the Earth, the skybox is visible.
- [ ] Two lights, the second one toggled with a key.
- [ ] Keys 1-8 focus the camera, time speed changes, on-screen help.
- [ ] At least 2 stretch goals declared and working (all three for a safety margin).
- [ ] Shaders in `.vert`/`.frag`, no files longer than ~500 lines, no commented-out blocks.
- [ ] README: build steps, controls, features, team responsibilities.
- [ ] CREDITS.md: every texture, model and tutorial snippet with author, link, license.
- [ ] The history has commits from all four members over the whole period.
- [ ] Tag `v1.0` on the submission day, TA added to the repository.
- [ ] Report PDF: all template sections, 15+ sources, FPS table with hardware.
- [ ] Slides 8-12, video 3-6 minutes with FPS on screen.
- [ ] Everyone can explain any part of the code and the formulas (Blinn-Phong, matrix hierarchy, mipmaps, instancing).

## 10. Main risks

| Risk | What to do |
|---|---|
| The build differs from the course assignments | Find out in step 0, adapt CMake in the first week |
| Two other teams took the topic | Get the instructor's confirmation on the first day |
| Textures without a license | Use only textures with an explicit license, record them in CREDITS.md immediately |
| One member does not commit or does not understand the code | Review each other's PRs, everyone explains their part to the team a week before the defense |
| FPS below 30 on the TA machine | Smaller textures, fewer asteroids, test on a weak machine early |
| Recording the video on the last day | Record 5-7 days before submission, leave time for retakes |
