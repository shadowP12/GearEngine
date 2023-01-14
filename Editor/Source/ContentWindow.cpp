#include "ContentWindow.h"
#include <imgui.h>

ContentWindow::ContentWindow() {

}

ContentWindow::~ContentWindow() {

}
void ContentWindow::Draw() {
    bool open = true;
    if (ImGui::Begin("ContentWindow", &open)) {
        ImGui::Button("TestButton1", ImVec2(200.0f, 50.0f));
    }
    ImGui::End();
}