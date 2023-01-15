#include "SceneHierarchy.h"
#include "imgui.h"

SceneHierarchy::SceneHierarchy(const std::string &id)
: EditorWindow(id) {
}

SceneHierarchy::~SceneHierarchy() {
}

void SceneHierarchy::Draw() {
    std::string window_name = "SceneHierarchy##" + id;
    ImGui::Begin(window_name.c_str());
    ImGui::End();
}