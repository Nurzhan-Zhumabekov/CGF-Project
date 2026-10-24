#include "UI.h"
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

UI::UI(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

UI::~UI() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void UI::draw(UIState& s, float fps, const std::string& cameraInfo) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::Begin("Solar System Explorer", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("FPS: %.0f", fps);
    ImGui::Text("Camera: %s", cameraInfo.c_str());
    ImGui::Separator();
    ImGui::SliderFloat("Time (days/s)", &s.timeScale, 0.0f, 2000.0f, "%.1f", ImGuiSliderFlags_Logarithmic);
    ImGui::Checkbox("Pause (Space)", &s.paused);
    ImGui::Checkbox("Second light (L)", &s.fillLight);
    ImGui::Checkbox("Orbit lines (O)", &s.orbitLines);
    ImGui::Checkbox("VSync", &s.vsync);
    ImGui::Checkbox("Help (H)", &s.showHelp);
    if (s.showHelp) {
        ImGui::Separator();
        ImGui::TextUnformatted("1-8  focus Mercury..Neptune");
        ImGui::TextUnformatted("9    follow the Moon");
        ImGui::TextUnformatted("0/F  free camera");
        ImGui::TextUnformatted("WASD QE fly, Shift = fast");
        ImGui::TextUnformatted("RMB drag look, wheel zoom");
        ImGui::TextUnformatted("[ ]  time x0.5 / x2");
        ImGui::TextUnformatted("Esc  quit");
    }
    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
