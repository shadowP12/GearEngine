#include "ImGuiBackend.h"
#include <Engine/GearEngine.h>
#include <Engine/Utility/FileSystem.h>
#include <Engine/Renderer/Renderer.h>
#include <Engine/Renderer/RenderScene.h>
#include <Engine/Resource/Material.h>
#include <Engine/Resource/GpuBuffer.h>
#include <Engine/Resource/Texture.h>
#include <Engine/Scene/Scene.h>
#include <Engine/Scene/Entity.h>
#include <Engine/Scene/Components/CCamera.h>
#include <Engine/Scene/Components/CTransform.h>
#include <Engine/Scene/Components/CRenderable.h>
#include <Engine/MaterialCompiler/MaterialCompiler.h>

// GLFW
#include <GLFW/glfw3.h>
#ifdef _WIN32
#undef APIENTRY
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif
#define GLFW_HAS_WINDOW_TOPMOST       (GLFW_VERSION_MAJOR * 1000 + GLFW_VERSION_MINOR * 100 >= 3200) // 3.2+ GLFW_FLOATING
#define GLFW_HAS_WINDOW_HOVERED       (GLFW_VERSION_MAJOR * 1000 + GLFW_VERSION_MINOR * 100 >= 3300) // 3.3+ GLFW_HOVERED
#define GLFW_HAS_WINDOW_ALPHA         (GLFW_VERSION_MAJOR * 1000 + GLFW_VERSION_MINOR * 100 >= 3300) // 3.3+ glfwSetWindowOpacity
#define GLFW_HAS_PER_MONITOR_DPI      (GLFW_VERSION_MAJOR * 1000 + GLFW_VERSION_MINOR * 100 >= 3300) // 3.3+ glfwGetMonitorContentScale
#define GLFW_HAS_VULKAN               (GLFW_VERSION_MAJOR * 1000 + GLFW_VERSION_MINOR * 100 >= 3200) // 3.2+ glfwCreateWindowSurface
#ifdef GLFW_RESIZE_NESW_CURSOR  // let's be nice to people who pulled GLFW between 2019-04-16 (3.4 define) and 2019-11-29 (cursors defines) // FIXME: Remove when GLFW 3.4 is released?
#define GLFW_HAS_NEW_CURSORS          (GLFW_VERSION_MAJOR * 1000 + GLFW_VERSION_MINOR * 100 >= 3400) // 3.4+ GLFW_RESIZE_ALL_CURSOR, GLFW_RESIZE_NESW_CURSOR, GLFW_RESIZE_NWSE_CURSOR, GLFW_NOT_ALLOWED_CURSOR
#else
#define GLFW_HAS_NEW_CURSORS          (0)
#endif

static double gTime = 0.0;
static bool gMouseJustPressed[ImGuiMouseButton_COUNT] = {};
static GLFWcursor* gMouseCursors[ImGuiMouseCursor_COUNT] = {};

static const char* ImguiGetClipboardTextCB(void* data) {
    return glfwGetClipboardString((GLFWwindow*)data);
}

static void ImguiSetClipboardTextCB(void* data, const char* text) {
    glfwSetClipboardString((GLFWwindow*)data, text);
}

void ImguiMouseButtonCB(GLFWwindow* window, int button, int action, int mods) {
    if (action == GLFW_PRESS && button >= 0 && button < IM_ARRAYSIZE(gMouseJustPressed))
        gMouseJustPressed[button] = true;
}

void ImguiScrollCB(GLFWwindow* window, double xoffset, double yoffset) {
    ImGuiIO& io = ImGui::GetIO();
    io.MouseWheelH += (float)xoffset;
    io.MouseWheel += (float)yoffset;
}

void ImguiKeyCB(GLFWwindow* window, int key, int scancode, int action, int mods) {
    ImGuiIO& io = ImGui::GetIO();
    if (key >= 0 && key < IM_ARRAYSIZE(io.KeysDown))
    {
        if (action == GLFW_PRESS)
            io.KeysDown[key] = true;
        if (action == GLFW_RELEASE)
            io.KeysDown[key] = false;
    }

    // Modifiers are not reliable across systems
    io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
    io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
    io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
#ifdef _WIN32
    io.KeySuper = false;
#else
    io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];
#endif
}

void ImguiCharCB(GLFWwindow* window, unsigned int c) {
    ImGuiIO& io = ImGui::GetIO();
    io.AddInputCharacter(c);
}

ImGuiLayout::ImGuiLayout(GLFWwindow* window) {
    mWindow = window;
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
    io.BackendPlatformName = "glfw";

    io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;
    io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
    io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;
    io.KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN;
    io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
    io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
    io.KeyMap[ImGuiKey_Insert] = GLFW_KEY_INSERT;
    io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
    io.KeyMap[ImGuiKey_Space] = GLFW_KEY_SPACE;
    io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
    io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
    io.KeyMap[ImGuiKey_KeyPadEnter] = GLFW_KEY_KP_ENTER;
    io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
    io.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
    io.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
    io.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
    io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
    io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;

    io.SetClipboardTextFn = ImguiSetClipboardTextCB;
    io.GetClipboardTextFn = ImguiGetClipboardTextCB;
    io.ClipboardUserData = mWindow;
#if defined(_WIN32)
    io.ImeWindowHandle = (void*)glfwGetWin32Window(mWindow);
#endif

    glfwSetMouseButtonCallback(window, ImguiMouseButtonCB);
    glfwSetScrollCallback(window, ImguiScrollCB);
    glfwSetKeyCallback(window, ImguiKeyCB);
    glfwSetCharCallback(window, ImguiCharCB);

    ImGui::GetIO().Fonts->AddFontFromFileTTF("./BuiltinResources/Fonts/Roboto-Medium.ttf", 16.0f);
    {
        // 初始化UI相机
        mUICamera = gear::gEngine.getScene()->createEntity();
        gear::CTransform* ct = mUICamera->addComponent<gear::CTransform>();
        ct->setTransform(glm::mat4(1.0));
        mUICamera->addComponent<gear::CCamera>()->setLayer(2);
    }

    {
        // 初始化UIPawn
        mUIPawn = gear::gEngine.getScene()->createEntity();
        gear::CTransform* ct = mUIPawn->addComponent<gear::CTransform>();
        ct->setTransform(glm::mat4(1.0));
        mUIPawn->addComponent<gear::CRenderable>()->setLayer(2);
    }

    // 加载图片资源
    unsigned char* pixels;
    int width, height;
    ImGui::GetIO().Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    uint32_t size = width * height * 4;
    gear::Texture::Builder texBuild;
    texBuild.width(width);
    texBuild.height(height);
    texBuild.format(Blast::FORMAT_R8G8B8A8_UNORM);
    mTexture = texBuild.build();
    mTexture->setData(0, pixels, size);

    // 加载材质
    std::string materialCode = gear::readFileData("./BuiltinResources/Materials/ui.mat");
    mMaterial = gear::gEngine.getMaterialCompiler()->compile(materialCode);
}

ImGuiLayout::~ImGuiLayout() {
    ImGui::DestroyContext();

    SAFE_DELETE(mMaterial);
    SAFE_DELETE(mTexture);
    for (int i = 0; i < mMaterialInstances.size(); ++i) {
        SAFE_DELETE(mMaterialInstances[i]);
    }
    mMaterialInstances.clear();
    for (int i = 0; i < mVertexBuffers.size(); ++i) {
        SAFE_DELETE(mVertexBuffers[i]);
    }
    mVertexBuffers.clear();
    for (int i = 0; i < mIndexBuffers.size(); ++i) {
        SAFE_DELETE(mIndexBuffers[i]);
    }
    mIndexBuffers.clear();
    gear::gEngine.getScene()->destroyEntity(mUICamera);
    gear::gEngine.getScene()->destroyEntity(mUIPawn);
}

void ImGuiLayout::beginFrame() {
    ImGuiIO& io = ImGui::GetIO();
    int w, h;
    int displayW, displayH;
    glfwGetWindowSize(mWindow, &w, &h);
    glfwGetFramebufferSize(mWindow, &displayW, &displayH);
    io.DisplaySize = ImVec2((float)w, (float)h);
    if (w > 0 && h > 0)
        io.DisplayFramebufferScale = ImVec2((float)displayW / w, (float)displayH / h);

    // 更新鼠标位置以及按钮状态
    for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++) {
        io.MouseDown[i] = gMouseJustPressed[i] || glfwGetMouseButton(mWindow, i) != 0;
        gMouseJustPressed[i] = false;
    }

    const ImVec2 mousePosBackup = io.MousePos;
    io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
    const bool focused = glfwGetWindowAttrib(mWindow, GLFW_FOCUSED) != 0;
    if (focused) {
        if (io.WantSetMousePos) {
            glfwSetCursorPos(mWindow, (double)mousePosBackup.x, (double)mousePosBackup.y);
        } else {
            double mouseX, mouseY;
            glfwGetCursorPos(mWindow, &mouseX, &mouseY);
            io.MousePos = ImVec2((float)mouseX, (float)mouseY);
        }
    }

    // 设置dt
    double currentTime = glfwGetTime();
    io.DeltaTime = gTime > 0.0 ? (float)(currentTime - gTime) : (float)(1.0f / 60.0f);
    gTime = currentTime;

    ImGui::NewFrame();
    mUICamera->getComponent<gear::CCamera>()->setProjection(gear::ProjectionType::ORTHO, 0.0, io.DisplaySize.x, io.DisplaySize.y, 0.0, 0.0, 1.0);
}

void ImGuiLayout::endFrame() {
    ImGui::Render();
    processImGuiCommands();
}

void ImGuiLayout::processImGuiCommands() {
    ImDrawData* commands = ImGui::GetDrawData();
    const ImGuiIO& io = ImGui::GetIO();

    int fbwidth = (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
    int fbheight = (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y);
    if (fbwidth == 0 || fbheight == 0)
        return;
    commands->ScaleClipRects(io.DisplayFramebufferScale);

    createBuffers(commands->CmdListsCount);

    // 计算当前有多少prim
    size_t nPrims = 0;
    for (int cmdListIndex = 0; cmdListIndex < commands->CmdListsCount; cmdListIndex++) {
        const ImDrawList* cmds = commands->CmdLists[cmdListIndex];
        nPrims += cmds->CmdBuffer.size();
    }

    // 为新的prim创建一个材质实例
    size_t previousSize = mMaterialInstances.size();
    if (nPrims > mMaterialInstances.size()) {
        mMaterialInstances.resize(nPrims);
        for (size_t i = previousSize; i < mMaterialInstances.size(); i++) {
            mMaterialInstances[i] = mMaterial->createInstance();
            Blast::GfxSamplerDesc samplerDesc;
            mMaterialInstances[i]->setParameter("albedo_texture", mTexture, samplerDesc);
        }
    }

    // 清空renderable component的prim，并重新设置
    gear::CRenderable* crenderable = mUIPawn->getComponent<gear::CRenderable>();
    crenderable->resetPrimitives();
    int bufferIndex = 0;
    int primIndex = 0;
    for (int cmdListIndex = 0; cmdListIndex < commands->CmdListsCount; cmdListIndex++) {
        const ImDrawList* cmds = commands->CmdLists[cmdListIndex];
        size_t indexOffset = 0;
        updateBufferData(bufferIndex,
                         cmds->VtxBuffer.Size * sizeof(ImDrawVert), cmds->VtxBuffer.Data,
                         cmds->IdxBuffer.Size * sizeof(ImDrawIdx), cmds->IdxBuffer.Data);
        for (const auto& pcmd : cmds->CmdBuffer) {
            if (pcmd.UserCallback) {
                pcmd.UserCallback(cmds, &pcmd);
            } else {
                // 设置材质属性
                gear::MaterialInstance* materialInstance = mMaterialInstances[primIndex];
                // TODO: 后续提供scissor接口
                // materialInstance->setScissor( pcmd.ClipRect.x, fbheight - pcmd.ClipRect.w,
                //                               (uint16_t) (pcmd.ClipRect.z - pcmd.ClipRect.x),
                //                               (uint16_t) (pcmd.ClipRect.w - pcmd.ClipRect.y));
                if (pcmd.TextureId) {
                    Blast::GfxSamplerDesc samplerDesc;
                    materialInstance->setParameter("albedo_texture", (gear::Texture*)pcmd.TextureId, samplerDesc);
                }

                // 创建对应的prim
                gear::RenderPrimitive primitive;
                primitive.count = pcmd.ElemCount;
                primitive.offset = indexOffset;
                primitive.type = Blast::PRIMITIVE_TOPO_TRI_LIST;
                primitive.materialInstance = materialInstance;
                primitive.vertexBuffer = mVertexBuffers[bufferIndex];
                primitive.indexBuffer = mIndexBuffers[bufferIndex];
                crenderable->addPrimitive(primitive);
                primIndex++;
            }
            indexOffset += pcmd.ElemCount;
        }
        bufferIndex++;
    }
}

void ImGuiLayout::createBuffers(int numRequiredBuffers) {
    if (numRequiredBuffers > mVertexBuffers.size()) {
        size_t previousSize = mVertexBuffers.size();
        mVertexBuffers.resize(numRequiredBuffers, nullptr);
        for (size_t i = previousSize; i < mVertexBuffers.size(); i++) {
            gear::VertexBuffer::Builder vbBuilder;
            vbBuilder.vertexCount(10000);
            vbBuilder.attribute(Blast::SEMANTIC_POSITION, Blast::FORMAT_R32G32_FLOAT);
            vbBuilder.attribute(Blast::SEMANTIC_TEXCOORD0, Blast::FORMAT_R32G32_FLOAT);
            vbBuilder.attribute(Blast::SEMANTIC_COLOR, Blast::FORMAT_R8G8B8A8_UNORM);
            mVertexBuffers[i] = vbBuilder.build();
        }
    }
    if (numRequiredBuffers > mIndexBuffers.size()) {
        size_t previousSize = mIndexBuffers.size();
        mIndexBuffers.resize(numRequiredBuffers, nullptr);
        for (size_t i = previousSize; i < mIndexBuffers.size(); i++) {
            gear::IndexBuffer::Builder builder;
            builder.indexCount(5000);
            builder.indexType(Blast::INDEX_TYPE_UINT16);
            mIndexBuffers[i] = builder.build();
        }
    }
}

void ImGuiLayout::updateBufferData(size_t bufferIndex, size_t vbSizeInBytes, void* vbImguiData, size_t ibSizeInBytes, void* ibImguiData) {
    mVertexBuffers[bufferIndex]->update(vbImguiData, 0, vbSizeInBytes);
    mIndexBuffers[bufferIndex]->update(ibImguiData, 0, ibSizeInBytes);
}