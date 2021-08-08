#pragma once
#include <imgui.h>
#include <vector>
#include <Engine/Utility/Event.h>

struct GLFWwindow;

namespace gear {
    class Entity;
    class Texture;
    class Material;
    class MaterialInstance;
    class VertexBuffer;
    class IndexBuffer;
}

class ImGuiLayout {
public:
    ImGuiLayout(GLFWwindow* window);

    ~ImGuiLayout();

    void beginFrame();

    void endFrame();

private:
    void onMousePosition(float x, float y);

    void onMouseButton(int button, int action);

    void onMouseScroll(float offset);

    void processImGuiCommands();

    void createBuffers(int numRequiredBuffers);

    void updateBufferData(size_t bufferIndex, size_t vbSizeInBytes, void* vbImguiData, size_t ibSizeInBytes, void* ibImguiData);

private:
    GLFWwindow* mWindow = nullptr;
    gear::Entity* mUICamera = nullptr;
    gear::Entity* mUIPawn = nullptr;
    gear::Material* mMaterial = nullptr;
    std::vector<gear::MaterialInstance*> mMaterialInstances;
    std::vector<gear::VertexBuffer*> mVertexBuffers;
    std::vector<gear::IndexBuffer*> mIndexBuffers;
    gear::Texture* mTexture = nullptr;
    EventHandle mOnMousePositionHandle;
    EventHandle mOnMouseButtonHandle;
    EventHandle mOnMouseScrollHandle;
};
