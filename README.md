# Solar System Explorer

Интерактивная 3D-модель Солнечной системы на C++17 и OpenGL 3.3 core. Финальный проект курса Computer Graphics Fundamentals (Astana IT University).

Команда: Nurzhan Zhumabekov, Muzaffar Kapezov, Murat Narynbekov, Aknur Ondasyn.

## Сборка

Нужны компилятор C++17 и CMake 3.10+. GLFW, GLAD, GLM, Dear ImGui и stb_image лежат в `third_party/`, ставить ничего не нужно.

```
cmake -S . -B build
cmake --build build --config Release
```

Запуск (шейдеры и текстуры копируются рядом с исполняемым файлом при сборке):

- Linux, macOS: `./build/solar_system`
- Windows (Visual Studio): `build\Release\solar_system.exe`

Linux: нужны заголовки X11, например `sudo apt install build-essential cmake libgl-dev libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev`.

Тесты математики и сцены: `ctest --test-dir build` (или `./build/core_tests`).

## Управление

| Клавиша | Действие |
|---|---|
| 1-8 | Камера следует за Меркурием ... Нептуном |
| 9 | Камера следует за Луной |
| 0 или F | Свободная камера |
| W A S D, Q E | Полёт вперёд, влево, назад, вправо, вниз, вверх (Shift быстрее) |
| ПКМ + движение мыши | Обзор |
| Колесо мыши | Расстояние до планеты в режиме следования |
| [ и ] | Время медленнее и быстрее в 2 раза |
| Space | Пауза |
| L | Второй свет (фонарь у камеры) |
| O | Орбитальные линии |
| H | Справка в панели |
| Esc | Выход |

Скорость времени, VSync и переключатели есть в панели ImGui. FPS показан в панели и в заголовке окна.

## Что реализовано

- Свои шейдеры в `shaders/*.vert` и `*.frag`, VAO/VBO/EBO, без fixed-function вызовов (R1).
- Солнце, 8 планет и Луна как объекты сцены (R2).
- Граф сцены: Луна дочерняя к Земле, матрицы model/view/projection считаются в коде через GLM (R3).
- Blinn-Phong на фрагмент: ambient, diffuse, specular. Два света: точечный в Солнце и переключаемый фонарь у камеры (R4).
- Текстуры с mipmap, skybox из cubemap (R5).
- Камера свободного полёта и камера слежения, 5 интерактивов (R6).
- FPS в заголовке окна и в панели (R7).
- Дополнительно: Земля смешивает дневную и ночную текстуры по направлению света.

Запланировано: кольца Сатурна с прозрачностью, пояс астероидов через instancing.

## Структура

```
src/        main, App (цикл и рендер), Window, Shader, Mesh, Texture, Skybox, UI,
            Camera, SceneNode, SolarSystem, Primitives
shaders/    planet, skybox, line
assets/     текстуры (PNG)
tools/      gen_textures.py: процедурные текстуры-заглушки
tests/      тесты геометрии, графа сцены и камеры без OpenGL
third_party/ GLFW, GLAD, GLM, Dear ImGui, stb_image
docs/       инструкция для команды
```

## Текстуры

Текущие текстуры созданы скриптом `tools/gen_textures.py` (процедурный шум), это заглушки. Их можно заменить реальными: положить PNG с теми же именами в `assets/textures/` и записать источник в `CREDITS.md`.

## Зоны ответственности

План распределения задач в команде:

| Участник | Зона |
|---|---|
| Nurzhan Zhumabekov | Сборка, окно, шейдеры, меши, FPS, интеграция |
| Muzaffar Kapezov | Сцена, иерархия, орбиты, время |
| Murat Narynbekov | Освещение, текстуры, skybox, день и ночь Земли, кольца |
| Aknur Ondasyn | Камера, интерфейс, пояс астероидов |
