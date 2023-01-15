#include "Inspector.h"
#include "imgui.h"

Inspector::Inspector() {
}

Inspector::~Inspector() {
}

void Inspector::Draw() {
    std::string window_name = "Inspector##" + id;
    ImGui::Begin(window_name.c_str());
    ImGui::End();
}