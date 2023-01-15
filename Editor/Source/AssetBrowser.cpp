#include "AssetBrowser.h"
#include "imgui.h"

AssetBrowser::AssetBrowser() {
}

AssetBrowser::~AssetBrowser() {
}

void AssetBrowser::Draw() {
    std::string window_name = "AssetBrowser##" + id;
    ImGui::Begin(window_name.c_str());
    ImGui::End();
}