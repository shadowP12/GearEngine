#pragma once
#include <imgui.h>
#include <vector>

struct GLFWwindow;

namespace gear {
    class RenderPipeline;
}

namespace ImGui {

    void ImGuiInit(GLFWwindow* window);

    void GetRenderPipeline(gear::RenderPipeline** pipeline);

    void BeginUI();

    void EndUI();

    void ImGuiTerminate();
}
