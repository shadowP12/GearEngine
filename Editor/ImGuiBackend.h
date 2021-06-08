#pragma once
#include <imgui.h>
#include <vector>

struct GLFWwindow;

bool ImGui_ImplGlfw_Init(GLFWwindow* window, bool install_callbacks);
void ImGui_ImplGlfw_Shutdown();
void ImGui_ImplGlfw_NewFrame();

void ImGui_ImplGlfw_MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void ImGui_ImplGlfw_ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void ImGui_ImplGlfw_KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void ImGui_ImplGlfw_CharCallback(GLFWwindow* window, unsigned int c);

namespace gear {
    class Texture;
    class Material;
    class MaterialInstance;
    class VertexBuffer;
    class IndexBuffer;
}

class ImGuiLayout {
public:
    ImGuiLayout();

    ~ImGuiLayout();

private:
    void createBuffers(int numRequiredBuffers);

    void updateBufferData(size_t bufferIndex, size_t vbSizeInBytes, void* vbImguiData, size_t ibSizeInBytes, void* ibImguiData);

private:
    gear::Material* mMaterial = nullptr;
    std::vector<gear::MaterialInstance*> mMaterialInstances;
    std::vector<gear::VertexBuffer*> mVertexBuffers;
    std::vector<gear::IndexBuffer*> mIndexBuffers;
    gear::Texture* mTexture = nullptr;
};
