#include "UI.h"
#include <GearEngine.h>
#include <Utility/FileSystem.h>
#include <Utility/Log.h>
#include <RenderPipeline/RenderPipeline.h>
#include <Resource/Material.h>
#include <Resource/GpuBuffer.h>
#include <Resource/Texture.h>
#include <Entity/Scene.h>
#include <Entity/Entity.h>
#include <Entity/EntityManager.h>
#include <Entity/Components/CCamera.h>
#include <Entity/Components/CTransform.h>
#include <Entity/Components/CRenderable.h>
#include <MaterialCompiler/MaterialCompiler.h>
#include <Input/InputSystem.h>

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

static GLFWwindow* g_window = nullptr;
static double g_time = 0.0;
gear::RenderPipeline* g_ui_pipeline = nullptr;
gear::Scene* g_ui_scene = nullptr;
gear::Entity* g_ui_camera = nullptr;
gear::Entity* g_ui_pawn = nullptr;
gear::Material* g_ui_ma = nullptr;
std::vector<gear::MaterialInstance*> g_ui_mis;
std::vector<gear::VertexBuffer*> g_vbs;
std::vector<gear::IndexBuffer*> g_ibs;
gear::Texture* g_ui_texture = nullptr;
EventHandle g_mouse_position_cb_handle;
EventHandle g_mouse_button_cb_handle;
EventHandle g_mouse_scroll_cb_handle;

const char* ImguiGetClipboardTextCB(void* data) {
    return glfwGetClipboardString((GLFWwindow*)data);
}

void ImguiSetClipboardTextCB(void* data, const char* text) {
    glfwSetClipboardString((GLFWwindow*)data, text);
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

void ImguiMousePositionCB(float x, float y) {
    // 当鼠标处于imgui控件内，则截断输入对应的输入事件
    if (ImGui::IsWindowHovered(1 << 2)) {
        gear::gEngine.GetInputSystem()->GetOnMousePositionEvent().Block();
    }
}

void ImguiMouseButtonCB(int button, int action) {
    // 当鼠标处于imgui控件内，则截断输入对应的输入事件
    if (ImGui::IsWindowHovered(1 << 2)) {
        gear::gEngine.GetInputSystem()->GetOnMouseButtonEvent().Block();
    }
}

void ImGuiMouseScrollCB(float offset) {
    // 当鼠标处于imgui控件内，则截断输入对应的输入事件
    if (ImGui::IsWindowHovered(1 << 2)) {
        gear::gEngine.GetInputSystem()->GetOnMouseScrollEvent().Block();
    }
}

namespace ImGui {
    void ProcessImGuiCommands();

    void CreateBuffers(uint32_t num_required_buffers);

    void UpdateBufferData(uint32_t buffer_index, uint32_t vb_size_in_bytes, void* vb_imgui_data, size_t ib_size_in_bytes, void* ib_imgui_Data);

    void GetRenderPipeline(gear::RenderPipeline** pipeline) {
        *pipeline = g_ui_pipeline;
    }

    void ImGuiInit(GLFWwindow* window) {
        g_window = window;

        g_mouse_position_cb_handle = gear::gEngine.GetInputSystem()->GetOnMousePositionEvent().Bind(ImguiMousePositionCB, 100);
        g_mouse_button_cb_handle = gear::gEngine.GetInputSystem()->GetOnMouseButtonEvent().Bind(ImguiMouseButtonCB, 100);
        g_mouse_scroll_cb_handle = gear::gEngine.GetInputSystem()->GetOnMouseScrollEvent().Bind(ImGuiMouseScrollCB, 100);

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
        io.ClipboardUserData = g_window;
#if defined(_WIN32)
        io.ImeWindowHandle = (void*)glfwGetWin32Window(g_window);
#endif

        glfwSetKeyCallback(window, ImguiKeyCB);
        glfwSetCharCallback(window, ImguiCharCB);

        ImGui::GetIO().Fonts->AddFontFromFileTTF("./BuiltinResources/Fonts/Roboto-Medium.ttf", 16.0f);

        // 创建ui场景
        g_ui_scene = gear::gEngine.CreateScene();

        // 初始化ui相机
        g_ui_camera = gear::gEngine.GetEntityManager()->CreateEntity();
        g_ui_camera->AddComponent<gear::CTransform>()->SetTransform(glm::mat4(1.0));
        g_ui_camera->AddComponent<gear::CCamera>();
        g_ui_scene->AddEntity(g_ui_camera);

        // 初始化ui实例
        g_ui_pawn = gear::gEngine.GetEntityManager()->CreateEntity();
        g_ui_pawn->AddComponent<gear::CTransform>()->SetTransform(glm::mat4(1.0));
        g_ui_pawn->AddComponent<gear::CRenderable>();
        g_ui_scene->AddEntity(g_ui_pawn);

        // 创建ui管线
        g_ui_pipeline = gear::gEngine.CreateRenderPipeline();
        g_ui_pipeline->SetScene(g_ui_scene);

        // 加载图片资源
        unsigned char* pixels;
        int width, height;
        ImGui::GetIO().Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
        uint32_t size = width * height * 4;
        gear::Texture::Builder tex_builder;
        tex_builder.SetWidth(width);
        tex_builder.SetHeight(height);
        tex_builder.SetFormat(blast::FORMAT_R8G8B8A8_UNORM);
        g_ui_texture = tex_builder.build();
        g_ui_texture->SetData(0, pixels, size);

        // 加载材质
        std::string material_code = gear::ReadFileData("./BuiltinResources/Materials/ui.mat");
        g_ui_ma = gear::gEngine.GetMaterialCompiler()->Compile(material_code);
    }


    void ImGuiTerminate() {
        gear::gEngine.GetInputSystem()->GetOnMousePositionEvent().Unbind(g_mouse_position_cb_handle);
        gear::gEngine.GetInputSystem()->GetOnMousePositionEvent().Unbind(g_mouse_button_cb_handle);
        gear::gEngine.GetInputSystem()->GetOnMousePositionEvent().Unbind(g_mouse_scroll_cb_handle);

        ImGui::DestroyContext();

        SAFE_DELETE(g_ui_ma);
        SAFE_DELETE(g_ui_texture);

        for (int i = 0; i < g_ui_mis.size(); ++i) {
            SAFE_DELETE(g_ui_mis[i]);
        }
        g_ui_mis.clear();

        for (int i = 0; i < g_vbs.size(); ++i) {
            SAFE_DELETE(g_vbs[i]);
        }
        g_vbs.clear();

        for (int i = 0; i < g_ibs.size(); ++i) {
            SAFE_DELETE(g_ibs[i]);
        }
        g_ibs.clear();

        gear::gEngine.GetEntityManager()->DestroyEntity(g_ui_pawn);
        gear::gEngine.GetEntityManager()->DestroyEntity(g_ui_camera);

        gear::gEngine.DestroyScene(g_ui_scene);
        gear::gEngine.DestroyRenderPipeline(g_ui_pipeline);
    }

    void BeginUI() {
        ImGuiIO& io = ImGui::GetIO();
        int w, h;
        int display_w, display_h;
        glfwGetWindowSize(g_window, &w, &h);
        glfwGetFramebufferSize(g_window, &display_w, &display_h);
        io.DisplaySize = ImVec2((float)w, (float)h);
        if (w > 0 && h > 0)
            io.DisplayFramebufferScale = ImVec2((float)display_w / w, (float)display_h / h);

        // 更新imgui现在的输入状态
        io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
        io.MousePos.x = gear::gEngine.GetInputSystem()->GetMousePosition().x;
        io.MousePos.y = gear::gEngine.GetInputSystem()->GetMousePosition().y;

        for (int i = 0; i < 3; i++) {
            io.MouseDown[i] = gear::gEngine.GetInputSystem()->GetMouseButtonDown(i) || gear::gEngine.GetInputSystem()->GetMouseButtonHeld(i);
        }

        io.MouseWheel = gear::gEngine.GetInputSystem()->GetMouseScrollWheel();

        // 设置dt
        double currentTime = glfwGetTime();
        io.DeltaTime = g_time > 0.0 ? (float)(currentTime - g_time) : (float)(1.0f / 60.0f);
        g_time = currentTime;

        ImGui::NewFrame();
        g_ui_camera->GetComponent<gear::CCamera>()->SetProjection(gear::ProjectionType::ORTHO, 0.0, io.DisplaySize.x, io.DisplaySize.y, 0.0, 0.0, 1.0);
    }

    void EndUI() {
        ImGui::Render();
        ProcessImGuiCommands();
    }

    void ProcessImGuiCommands() {
        ImDrawData* commands = ImGui::GetDrawData();
        const ImGuiIO& io = ImGui::GetIO();

        int fbwidth = (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
        int fbheight = (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y);
        if (fbwidth == 0 || fbheight == 0)
            return;
        commands->ScaleClipRects(io.DisplayFramebufferScale);

        CreateBuffers(commands->CmdListsCount);

        // 计算当前有多少prim
        uint32_t num_prims = 0;
        for (int cmdListIndex = 0; cmdListIndex < commands->CmdListsCount; cmdListIndex++) {
            const ImDrawList* cmds = commands->CmdLists[cmdListIndex];
            num_prims += cmds->CmdBuffer.size();
        }

        // 为新的prim创建一个材质实例
        uint32_t previous_size = g_ui_mis.size();
        if (num_prims > g_ui_mis.size()) {
            g_ui_mis.resize(num_prims);
            for (size_t i = previous_size; i < g_ui_mis.size(); i++) {
                g_ui_mis[i] = g_ui_ma->CreateInstance();
                blast::GfxSamplerDesc sampler_desc;
                g_ui_mis[i]->SetTexture("albedo_texture", g_ui_texture, sampler_desc);
            }
        }

        // 清空renderable component的prim，并重新设置
        gear::CRenderable* crenderable = g_ui_pawn->GetComponent<gear::CRenderable>();
        crenderable->ResetPrimitives();
        uint32_t buffer_index = 0;
        uint32_t prim_index = 0;
        for (uint32_t cmd_list_index = 0; cmd_list_index < commands->CmdListsCount; cmd_list_index++) {
            const ImDrawList* cmds = commands->CmdLists[cmd_list_index];
            uint32_t index_offset = 0;
            UpdateBufferData(buffer_index,
                             cmds->VtxBuffer.Size * sizeof(ImDrawVert), cmds->VtxBuffer.Data,
                             cmds->IdxBuffer.Size * sizeof(ImDrawIdx), cmds->IdxBuffer.Data);

            for (const auto& pcmd : cmds->CmdBuffer) {
                if (pcmd.UserCallback) {
                    pcmd.UserCallback(cmds, &pcmd);
                } else {
                    // 设置材质属性
                    gear::MaterialInstance* mi = g_ui_mis[prim_index];
                    // TODO: 后续提供scissor接口
                    // materialInstance->setScissor( pcmd.ClipRect.x, fbheight - pcmd.ClipRect.w,
                    //                               (uint16_t) (pcmd.ClipRect.z - pcmd.ClipRect.x),
                    //                               (uint16_t) (pcmd.ClipRect.w - pcmd.ClipRect.y));
                    if (pcmd.TextureId) {
                        blast::GfxSamplerDesc sampler_desc;
                        mi->SetTexture("albedo_texture", (gear::Texture*)pcmd.TextureId, sampler_desc);
                    }

                    // 创建对应的prim
                    gear::RenderPrimitive primitive;
                    primitive.count = pcmd.ElemCount;
                    primitive.offset = index_offset;
                    primitive.topo = blast::PRIMITIVE_TOPO_TRI_LIST;
                    primitive.mi = mi;
                    primitive.vb = g_vbs[buffer_index];
                    primitive.ib = g_ibs[buffer_index];
                    crenderable->AddPrimitive(primitive);
                    prim_index++;
                }
                index_offset += pcmd.ElemCount;
            }
            buffer_index++;
        }
    }

    void CreateBuffers(uint32_t num_required_buffers) {
        if (num_required_buffers > g_vbs.size()) {
            uint32_t previous_size = g_vbs.size();
            g_vbs.resize(num_required_buffers, nullptr);
            for (uint32_t i = previous_size; i < g_vbs.size(); i++) {
                gear::VertexBuffer::Builder builder;
                builder.SetVertexCount(10000);
                builder.SetAttribute(blast::SEMANTIC_POSITION, blast::FORMAT_R32G32_FLOAT);
                builder.SetAttribute(blast::SEMANTIC_TEXCOORD0, blast::FORMAT_R32G32_FLOAT);
                builder.SetAttribute(blast::SEMANTIC_COLOR, blast::FORMAT_R8G8B8A8_UNORM);
                g_vbs[i] = builder.Build();
            }
        }

        if (num_required_buffers > g_ibs.size()) {
            uint32_t previous_size = g_ibs.size();
            g_ibs.resize(num_required_buffers, nullptr);
            for (uint32_t i = previous_size; i < g_ibs.size(); i++) {
                gear::IndexBuffer::Builder builder;
                builder.SetIndexCount(5000);
                builder.SetIndexType(blast::INDEX_TYPE_UINT16);
                g_ibs[i] = builder.Build();
            }
        }
    }

    void UpdateBufferData(uint32_t buffer_index, uint32_t vb_size_in_bytes, void* vb_imgui_data, size_t ib_size_in_bytes, void* ib_imgui_Data) {
        if (g_vbs[buffer_index]->GetSize() < vb_size_in_bytes) {
            SAFE_DELETE(g_vbs[buffer_index]);
            uint32_t require_count = (vb_size_in_bytes / sizeof(ImDrawVert)) * 2;
            gear::VertexBuffer::Builder builder;
            builder.SetVertexCount(require_count);
            builder.SetAttribute(blast::SEMANTIC_POSITION, blast::FORMAT_R32G32_FLOAT);
            builder.SetAttribute(blast::SEMANTIC_TEXCOORD0, blast::FORMAT_R32G32_FLOAT);
            builder.SetAttribute(blast::SEMANTIC_COLOR, blast::FORMAT_R8G8B8A8_UNORM);
            g_vbs[buffer_index] = builder.Build();
        }
        g_vbs[buffer_index]->Update(vb_imgui_data, 0, vb_size_in_bytes);

        if (g_ibs[buffer_index]->GetSize() < ib_size_in_bytes) {
            SAFE_DELETE(g_ibs[buffer_index]);
            uint32_t require_count = (ib_size_in_bytes / sizeof(ImDrawIdx)) * 2;
            gear::IndexBuffer::Builder builder;
            builder.SetIndexCount(require_count);
            builder.SetIndexType(blast::INDEX_TYPE_UINT16);
            g_ibs[buffer_index] = builder.Build();
        }
        g_ibs[buffer_index]->Update(ib_imgui_Data, 0, ib_size_in_bytes);
    }
}