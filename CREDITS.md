# Credits

## Библиотеки (в `third_party/`)

| Библиотека | Версия | Автор | Лицензия | Ссылка |
|---|---|---|---|---|
| GLFW | 3.4 | Marcus Geelnard, Camilla Löwy и участники | zlib/libpng | https://www.glfw.org |
| GLM | 1.0.1 | G-Truc Creation | MIT (Happy Bunny) | https://github.com/g-truc/glm |
| Dear ImGui | 1.90.1 | Omar Cornut | MIT | https://github.com/ocornut/imgui |
| stb_image | 2.30 | Sean Barrett | Public domain / MIT | https://github.com/nothings/stb |
| GLAD 2 (сгенерированный загрузчик GL 3.3 core) | glad2 | David Herberth | MIT, спецификации Khronos | https://github.com/Dav1dde/glad |

Исходники GLFW, GLM и Dear ImGui взяты из пакетов Ubuntu (archive.ubuntu.com, pool/universe), stb_image из репозитория nothings/stb.

## Текстуры и ассеты

Все текстуры в `assets/textures/` созданы скриптом `tools/gen_textures.py` (процедурный шум), сторонних ассетов нет. Если в проект попадут внешние текстуры или модели, добавьте строку: название, автор, ссылка, лицензия.

## Туториалы и код по мотивам

Подход к skybox (`gl_Position = pos.xyww`, `GL_LEQUAL`) и Blinn-Phong описан в LearnOpenGL (Joey de Vries, https://learnopengl.com, CC BY-NC 4.0). Код написан заново, не скопирован.
