cmake_minimum_required(VERSION 3.10)
project(SolarSystemExplorer LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Third-party code lives in third_party/ so nothing has to be installed on the TA machine.
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "" FORCE)
add_subdirectory(third_party/glfw)

add_library(glad STATIC third_party/glad/src/gl.c)
target_include_directories(glad PUBLIC third_party/glad/include)

add_library(imgui STATIC
  third_party/imgui/imgui.cpp
  third_party/imgui/imgui_draw.cpp
  third_party/imgui/imgui_tables.cpp
  third_party/imgui/imgui_widgets.cpp
  third_party/imgui/imgui_impl_glfw.cpp
  third_party/imgui/imgui_impl_opengl3.cpp)
target_include_directories(imgui PUBLIC third_party/imgui)
target_link_libraries(imgui PUBLIC glfw)

add_library(glm INTERFACE)
target_include_directories(glm INTERFACE third_party/glm)

# Pure math/geometry code (no OpenGL): shared by the app and the unit tests.
add_library(core STATIC
  src/Primitives.cpp
  src/SceneNode.cpp
  src/SolarSystem.cpp
  src/Camera.cpp)
target_include_directories(core PUBLIC src)
target_link_libraries(core PUBLIC glm)

add_executable(solar_system
  src/main.cpp
  src/App.cpp
  src/Window.cpp
  src/Shader.cpp
  src/Mesh.cpp
  src/Texture.cpp
  src/Skybox.cpp
  src/UI.cpp)
target_include_directories(solar_system PRIVATE third_party/stb)
target_link_libraries(solar_system PRIVATE core glfw glad imgui)

# Shaders and textures are copied next to the executable after every build.
add_custom_command(TARGET solar_system POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:solar_system>/shaders
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:solar_system>/assets)

enable_testing()
add_executable(core_tests tests/test_core.cpp)
target_link_libraries(core_tests PRIVATE core)
add_test(NAME core_tests COMMAND core_tests)
