#include "SceneViewer.h"
#include "imgui.h"

SceneViewer::SceneViewer(const std::string &id)
: EditorWindow(id) {

}

SceneViewer::~SceneViewer() {
}

void SceneViewer::Draw() {
    std::string window_name = "SceneViewer##" + id;
    ImGui::Begin(window_name.c_str());
    ImGui::End();
}