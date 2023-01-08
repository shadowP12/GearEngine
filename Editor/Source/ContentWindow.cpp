#include "ContentWindow.h"
#include <imgui.h>

ContentWindow::ContentWindow() {

}

ContentWindow::~ContentWindow() {

}

void ContentWindow::Draw() {
    if (ImGui::Begin("ContentWindow")) {
    }
    ImGui::End();
}