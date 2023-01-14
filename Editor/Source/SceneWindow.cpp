#include "SceneWindow.h"
#include <imgui.h>

SceneWindow::SceneWindow() {

}

SceneWindow::~SceneWindow() {

}

void SceneWindow::Draw() {
    if (ImGui::Begin("SceneWindow")) {
        ImGui::Button("TestButton2", ImVec2(200.0f, 50.0f));
    }
    ImGui::End();
}