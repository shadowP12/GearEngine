#include "LevelEditor.h"
#include <imgui.h>
#include <imgui_internal.h>

LevelEditor::LevelEditor() {
    inspector = std::make_shared<Inspector>();
    asset_browser = std::make_shared<AssetBrowser>();
    scene_viewer = std::make_shared<SceneViewer>("LevelEditor");
    scene_hierarchy = std::make_shared<SceneHierarchy>("LevelEditor");
}

LevelEditor::~LevelEditor() {

}

void LevelEditor::Draw() {
    std::string window_name = "LevelEditor";
    ImGui::Begin(window_name.c_str());
    ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_NoWindowMenuButton | ImGuiDockNodeFlags_NoCloseButton;
    ImGui::DockSpace(ImGui::GetID("LevelEditor"), ImVec2(0, 0), dockspace_flags);
    ImGui::End();

    inspector->Draw();
    asset_browser->Draw();
    scene_viewer->Draw();
    scene_hierarchy->Draw();
}