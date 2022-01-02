#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <imgui.h>

#include <Window/BaseWindow.h>
#include <Application/BaseApplication.h>
#include <GearEngine.h>
#include <Renderer/Renderer.h>
#include <View/View.h>
#include <Entity/Scene.h>
#include <Entity/Entity.h>
#include <Entity/EntityManager.h>
#include <Entity/Components/CLight.h>
#include <Entity/Components/CCamera.h>
#include <Entity/Components/CTransform.h>
#include <Entity/Components/CMesh.h>
#include <Entity/Components/CSkybox.h>
#include <Resource/GpuBuffer.h>
#include <Resource/Texture.h>
#include <Resource/Material.h>
#include <Resource/BuiltinResources.h>
#include <MaterialCompiler/MaterialCompiler.h>
#include <Input/InputSystem.h>
#include <UI/Canvas.h>

#include <map>

#include "CameraController.h"
#include "TextureImporter.h"
#include "GltfImporter.h"

struct Vertex {
    float pos[3];
    float uv[2];
};

float vertices[] = {
        -0.5f,  0.5f, 0.0f, 0.0f, 0.0f,
        0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
        0.5f, -0.5f, 0.0f, 1.0f, 1.0f,
        -0.5f, -0.5f, 0.0f, 0.0f, 1.0f
};

unsigned int indices[] = {
        0, 1, 2, 2, 3, 0
};

class Window;
static std::map<GLFWwindow*, Window*> glfw_window_table;

class Window : public gear::BaseWindow {
public:
    Window(uint32_t w, uint32_t h) {
        glfw_window = glfwCreateWindow(w, h, "GearEditor", nullptr, nullptr);
        width = w;
        height = h;
        window_ptr = glfwGetWin32Window(glfw_window);
        glfwSetFramebufferSizeCallback(glfw_window, this->WindowSizeCallback);
        glfwSetCursorPosCallback(glfw_window, CursorPositionCallback);
        glfwSetMouseButtonCallback(glfw_window, MouseButtonCallback);
        glfwSetScrollCallback(glfw_window, MouseScrollCallback);
        glfw_window_table[glfw_window] = this;
    }

    ~Window() {
        glfwDestroyWindow(glfw_window);
    }

    bool ShouldClose() {
        return glfwWindowShouldClose(glfw_window);
    };

    static void WindowSizeCallback(GLFWwindow* window, int w, int h) {
        glfw_window_table[window]->InternalWindowSizeCallback(w, h);
    }

    static void CursorPositionCallback(GLFWwindow* window, double pos_x, double pos_y) {
        glfw_window_table[window]->InternalCursorPositionCallback(pos_x, pos_y);
    }

    static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
        glfw_window_table[window]->InternalMouseButtonCallback(button, action, mods);
    }

    static void MouseScrollCallback(GLFWwindow* window, double offset_x, double offset_y) {
        glfw_window_table[window]->InternalMouseScrollCallback(offset_x, offset_y);
    }

private:
    void InternalWindowSizeCallback(int w, int h) {
        width = w;
        height = h;
    }

    void InternalCursorPositionCallback(double pos_x, double pos_y) {
        gear::gEngine.GetInputSystem()->OnMousePosition(pos_x, pos_y);
    }

    void InternalMouseButtonCallback(int button, int action, int mods) {
        gear::gEngine.GetInputSystem()->OnMouseButton(button, action);
    }

    void InternalMouseScrollCallback(double offset_x, double offset_y) {
        gear::gEngine.GetInputSystem()->OnMouseScroll(offset_y);
    }

private:
    friend class Application;
    GLFWwindow* glfw_window = nullptr;
};

class Application : public gear::BaseApplication {
public:
    Application() {

    }

    ~Application() {

    }

    void Init() override {
        gear::gEngine.GetBuiltinResources()->Initialize("./BuiltinResources/");

        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        // 初始化ImGui
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
        io.Fonts->AddFontFromFileTTF("./BuiltinResources/Fonts/Roboto-Medium.ttf", 16.0f);

        mouse_position_cb_handle = gear::gEngine.GetInputSystem()->GetOnMousePositionEvent().Bind(ImGuiMousePositionCB, 100);
        mouse_button_cb_handle = gear::gEngine.GetInputSystem()->GetOnMouseButtonEvent().Bind(ImGuiMouseButtonCB, 100);
        mouse_scroll_cb_handle = gear::gEngine.GetInputSystem()->GetOnMouseScrollEvent().Bind(ImGuiMouseScrollCB, 100);

        main_window = new Window(800, 600);
        scene_view = new gear::View();
        scene = new gear::Scene();
        {
            int tex_width, tex_height;
            unsigned char* pixels = nullptr;
            io.Fonts->GetTexDataAsRGBA32(&pixels, &tex_width, &tex_height);
            gear::Texture::Builder tex_builder;
            tex_builder.SetWidth(tex_width);
            tex_builder.SetHeight(tex_height);
            tex_builder.SetFormat(blast::FORMAT_R8G8B8A8_UNORM);
            font_texture = tex_builder.Build();
            font_texture->UpdateData(pixels);

            imgui_ma = gear::gEngine.GetMaterialCompiler()->Compile("./BuiltinResources/Materials/ui.mat", true);

            camera = gear::gEngine.GetEntityManager()->CreateEntity();
            camera->AddComponent<gear::CTransform>()->SetTransform(glm::mat4(1.0f));
            camera->GetComponent<gear::CTransform>()->SetPosition(glm::vec3(0.0f, 0.0f, 5.0f));
            //camera->GetComponent<gear::CTransform>()->SetEuler(glm::vec3(-0.358f, 0.46f, 0.0f));
            camera->AddComponent<gear::CCamera>()->SetProjection(gear::ProjectionType::PERSPECTIVE, 0.0, 800.0, 600.0, 0.0, 0.1, 100.0);
            //camera->AddComponent<gear::CCamera>()->SetProjection(gear::ProjectionType::ORTHO, -1.0, 1.0, 1.0, -1.0, 0.0, 1.0);
            scene->AddEntity(camera);

            sun = gear::gEngine.GetEntityManager()->CreateEntity();
            sun->AddComponent<gear::CTransform>()->SetTransform(glm::mat4(1.0f));
            sun->GetComponent<gear::CTransform>()->SetPosition(glm::vec3(0.0f, 10.0f, 0.0f));
            sun->GetComponent<gear::CTransform>()->SetEuler(glm::vec3(glm::radians(120.0f), 0.0f, 0.0f));
            sun->AddComponent<gear::CLight>();
            scene->AddEntity(sun);

            canvas = new gear::Canvas();

            camera_controller = new CameraController();
            camera_controller->SetCamera(camera);

            gltf_asset = ImportGltfAsset("./BuiltinResources/GltfFiles/mech_drone/scene.gltf");
            for (uint32_t i = 0; i < gltf_asset->entities.size(); ++i) {
                if (i == 1) {
                    gltf_asset->entities[i]->GetComponent<gear::CTransform>()->SetScale(glm::vec3(0.03f));
                }

                if (gltf_asset->entities[i]->HasComponent<gear::CMesh>()) {
                    gltf_asset->entities[i]->GetComponent<gear::CMesh>()->SetCastShadow(true);
                    gltf_asset->entities[i]->GetComponent<gear::CMesh>()->SetReceiveShadow(true);
                }
                scene->AddEntity(gltf_asset->entities[i]);
            }

            // 加载天空盒以及IBL资源
            gear::Texture* equirectangular_map = ImportTexture2DWithFloat("./BuiltinResources/Textures/Ridgecrest_Road_Ref.hdr");
            skybox_map = gear::gEngine.GetRenderer()->EquirectangularMapToCubemap(equirectangular_map, 512);
            irradiance_map = gear::gEngine.GetRenderer()->ComputeIrradianceMap(skybox_map);
            prefiltered_map = gear::gEngine.GetRenderer()->ComputePrefilteredMap(skybox_map);
            brdf_lut = gear::gEngine.GetRenderer()->ComputeBRDFLut();
            SAFE_DELETE(equirectangular_map);

            sun->AddComponent<gear::CSkybox>()->SetCubeMap(skybox_map);
            ibl = gear::gEngine.GetEntityManager()->CreateEntity();
            ibl->AddComponent<gear::CTransform>()->SetTransform(glm::mat4(1.0f));
            ibl->AddComponent<gear::CLight>();
            ibl->GetComponent<gear::CLight>()->SetLightType(gear::CLight::LightType::IBL);
            ibl->GetComponent<gear::CLight>()->SetIrradianceMap(irradiance_map);
            ibl->GetComponent<gear::CLight>()->SetPrefilteredMap(prefiltered_map);
            ibl->GetComponent<gear::CLight>()->SetBRDFLut(brdf_lut);
            scene->AddEntity(ibl);
        }
    }

    void Exit() override {
        DestroyGltfAsset(gltf_asset);
        gear::gEngine.GetEntityManager()->DestroyEntity(sun);
        gear::gEngine.GetEntityManager()->DestroyEntity(ibl);
        gear::gEngine.GetEntityManager()->DestroyEntity(camera);
        SAFE_DELETE(font_texture);
        SAFE_DELETE(skybox_map);
        SAFE_DELETE(irradiance_map);
        SAFE_DELETE(prefiltered_map);
        SAFE_DELETE(brdf_lut);
        for (int i = 0; i < imgui_mis.size(); ++i) {
            SAFE_DELETE(imgui_mis[i]);
        }
        imgui_mis.clear();
        SAFE_DELETE(imgui_ma);
        SAFE_DELETE(main_window);
        SAFE_DELETE(scene_view);
        SAFE_DELETE(canvas);
        SAFE_DELETE(scene);
        SAFE_DELETE(camera_controller);

        ImGui::DestroyContext();
        glfwTerminate();
    };

    bool ShouldClose() override {
        if (main_window) {
            return main_window->ShouldClose();
        }
        return true;
    };

protected:
    void Tick(float dt) override {
        glfwPollEvents();

        // 构建ImGui
        ImGuiIO& io = ImGui::GetIO();
        io.DeltaTime = dt;
        io.DisplaySize = ImVec2((float)main_window->GetWidth(), (float)main_window->GetHeight());
        io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
        io.MousePos.x = gear::gEngine.GetInputSystem()->GetMousePosition().x;
        io.MousePos.y = gear::gEngine.GetInputSystem()->GetMousePosition().y;
        io.MouseWheel = gear::gEngine.GetInputSystem()->GetMouseScrollWheel();
        for (int i = 0; i < 3; i++) {
            io.MouseDown[i] = gear::gEngine.GetInputSystem()->GetMouseButtonDown(i) || gear::gEngine.GetInputSystem()->GetMouseButtonHeld(i);
        }

        ImGui::NewFrame();
        bool show_demo_window = true;
        ImGui::ShowDemoWindow(&show_demo_window);
        ImGui::Render();

        canvas->Begin();
        ImDrawData* commands = ImGui::GetDrawData();
        uint32_t num_prims = 0;
        for (int cmdListIndex = 0; cmdListIndex < commands->CmdListsCount; cmdListIndex++) {
            const ImDrawList* cmds = commands->CmdLists[cmdListIndex];
            num_prims += cmds->CmdBuffer.size();
        }

        uint32_t previous_size = imgui_mis.size();
        if (num_prims > imgui_mis.size()) {
            imgui_mis.resize(num_prims);
            for (size_t i = previous_size; i < imgui_mis.size(); i++) {
                imgui_mis[i] = imgui_ma->CreateInstance();
                imgui_mis[i]->SetTexture("albedo_texture", font_texture);
                blast::GfxSamplerDesc sampler_desc;
                imgui_mis[i]->SetSampler("albedo_sampler", sampler_desc);
            }
        }

        uint32_t prim_index = 0;
        for (uint32_t cmd_list_index = 0; cmd_list_index < commands->CmdListsCount; cmd_list_index++) {
            const ImDrawList* cmds = commands->CmdLists[cmd_list_index];
            uint32_t index_offset = 0;

            gear::Canvas::Batch canvas_batch;
            canvas_batch.vertex_data = (uint8_t*)(cmds->VtxBuffer.Data);
            canvas_batch.vertex_count = cmds->VtxBuffer.Size;
            canvas_batch.vertex_layout = gear::VLT_UI;
            canvas_batch.index_data = (uint8_t*)(cmds->IdxBuffer.Data);
            canvas_batch.index_count = cmds->IdxBuffer.Size;
            canvas_batch.index_type = blast::INDEX_TYPE_UINT16;

            for (const auto& pcmd : cmds->CmdBuffer) {
                if (pcmd.UserCallback) {
                    pcmd.UserCallback(cmds, &pcmd);
                } else {
                    gear::Canvas::Element canvas_element;
                    gear::MaterialInstance* imgui_mi = imgui_mis[prim_index];
                    imgui_mi->SetScissor( pcmd.ClipRect.x, pcmd.ClipRect.y, pcmd.ClipRect.z, pcmd.ClipRect.w);
                    canvas_element.mi = imgui_mi;
                    canvas_element.count = pcmd.ElemCount;
                    canvas_element.offset = index_offset;
                    canvas_batch.elements.push_back(canvas_element);
                    prim_index++;
                }
                index_offset += pcmd.ElemCount;
            }
            canvas->AddBatch(canvas_batch);
        }
        canvas->End();

        // 绘制场景
        scene_view->SetSize(main_window->GetWidth(), main_window->GetHeight());
        scene_view->SetViewport(0, 0, main_window->GetWidth(), main_window->GetHeight());
        gear::gEngine.GetRenderer()->RenderScene(scene, scene_view);

        // 绘制到window上
        gear::View* views[] = { scene_view };
        gear::Canvas* canvases[] = { canvas };
        gear::gEngine.GetRenderer()->RenderWindow(main_window, 1, views, 1, canvases);
    };

    static void ImGuiMousePositionCB(float x, float y) {
        // 当鼠标处于imgui控件内，则截断输入对应的输入事件
        if (ImGui::IsWindowHovered(1 << 2)) {
            gear::gEngine.GetInputSystem()->GetOnMousePositionEvent().Block();
        }
    }

    static void ImGuiMouseButtonCB(int button, int action) {
        // 当鼠标处于imgui控件内，则截断输入对应的输入事件
        if (ImGui::IsWindowHovered(1 << 2)) {
            gear::gEngine.GetInputSystem()->GetOnMouseButtonEvent().Block();
        }
    }

    static void ImGuiMouseScrollCB(float offset) {
        // 当鼠标处于imgui控件内，则截断输入对应的输入事件
        if (ImGui::IsWindowHovered(1 << 2)) {
            gear::gEngine.GetInputSystem()->GetOnMouseScrollEvent().Block();
        }
    }

private:
    Window* main_window = nullptr;
    gear::Scene* scene = nullptr;
    gear::Canvas* canvas = nullptr;
    gear::View* scene_view = nullptr;
    gear::Entity* camera = nullptr;
    gear::Entity* sun = nullptr;
    gear::Entity* ibl = nullptr;
    gear::Material* imgui_ma = nullptr;
    std::vector<gear::MaterialInstance*> imgui_mis;
    gear::Texture* font_texture = nullptr;
    gear::Texture* skybox_map = nullptr;
    gear::Texture* irradiance_map = nullptr;
    gear::Texture* prefiltered_map = nullptr;
    gear::Texture* brdf_lut = nullptr;
    CameraController* camera_controller = nullptr;
    GltfAsset* gltf_asset = nullptr;
    EventHandle mouse_position_cb_handle;
    EventHandle mouse_button_cb_handle;
    EventHandle mouse_scroll_cb_handle;
};

int main() {
    double time = 0.0;
    Application app;
    app.Init();
    while (!app.ShouldClose()) {
        double current_time = glfwGetTime();
        float dt = time > 0.0 ? (float)(current_time - time) : (float)(1.0f / 60.0f);
        time = current_time;
        app.Run(dt);
    }
    app.Exit();
    return 0;
}