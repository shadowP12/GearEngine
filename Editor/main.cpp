#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <Blast/Gfx/GfxBuffer.h>
#include <Blast/Gfx/GfxContext.h>
#include <Blast/Utility/ShaderCompiler.h>
#include <Engine/GearEngine.h>
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
#include <Engine/Input/InputSystem.h>
#include <Engine/Utility/FileSystem.h>
#include <Engine/Utility/Event.h>
#include <imgui.h>
#include "ImGuiBackend.h"
#include "GearEditor.h"
#include "TextureImporter.h"
#include "GltfImporter/GltfImporter.h"

struct Vertex {
    float pos[3];
    float uv[2];
};

float vertices[] = {
        -0.5f,  0.5f, 0.0f, 0.0f,
        0.5f, 0.5f, 1.0f, 0.0f,
        0.5f, -0.5f, 1.0f, 1.0f,
        -0.5f, -0.5f, 0.0f, 1.0f
};

unsigned int indices[] = {
        0, 1, 2, 2, 3, 0
};


ImGuiLayout* gImGuiLayout = nullptr;
GLFWwindow* gWindowPtr = nullptr;
uint32_t gWidth = 800;
uint32_t gHeight = 600;
gear::Material* gDefaultMat = nullptr;
gear::Material* gUIMat = nullptr;
gear::MaterialInstance* gDefaultMatIns = nullptr;
gear::MaterialInstance* gUIMatIns = nullptr;
gear::VertexBuffer* gVB = nullptr;
gear::IndexBuffer* gIB = nullptr;
gear::Texture* gTestTex = nullptr;
gear::Entity* gCamera = nullptr;
gear::Entity* gPawn = nullptr;
GltfAsset* gltfAsset = nullptr;

class CameraController {
public:
    CameraController() {
        mEye = glm::vec3(0.0, 0.0, 2.0);
        mEuler.x = 0.0;
        mEuler.y = 0.0;
        mEuler.z = 0.0;
        mOnMousePositionHandle = gear::gEngine.getInputSystem()->getOnMousePositionEvent().bind(CALLBACK_2(CameraController::onMousePosition, this));
        mOnMouseButtonHandle = gear::gEngine.getInputSystem()->getOnMouseButtonEvent().bind(CALLBACK_2(CameraController::onMouseButton, this));
        mOnMouseScrollHandle = gear::gEngine.getInputSystem()->getOnMouseScrollEvent().bind(CALLBACK_1(CameraController::onMouseScroll, this));
    }

    ~CameraController() {
        gear::gEngine.getInputSystem()->getOnMousePositionEvent().unbind(mOnMousePositionHandle);
        gear::gEngine.getInputSystem()->getOnMousePositionEvent().unbind(mOnMouseButtonHandle);
        gear::gEngine.getInputSystem()->getOnMousePositionEvent().unbind(mOnMouseScrollHandle);
    }

    glm::mat4 getCameraMatrix() {
        glm::mat4 R, T;
        R = glm::toMat4(glm::quat(mEuler));
        T = glm::translate(glm::mat4(1.0), mEye);
        return T * R;
    }

private:
    void onMousePosition(float x, float y) {
        update(x, y);
    }

    void onMouseButton(int button, int action) {
        double x, y;
        x = gear::gEngine.getInputSystem()->getMousePosition().x;
        y = gear::gEngine.getInputSystem()->getMousePosition().y;
        switch (action) {
            case 0:
                // Button Up
                if (button == 1) {
                    end();
                }
                break;
            case 1:
                // Button Down
                if (button == 1) {
                    begin(x, y);
                }
                break;
            default:
                break;
        }
    }

    void onMouseScroll(float offset) {
        scroll(offset);
    }

    void begin(int x, int y) {
        mGrabbing = true;
        mStartPoint = glm::vec2(x, y);
    }

    void end() {
        mGrabbing = false;
    }

    void update(int x, int y) {
        if (!mGrabbing) {
            return;
        }
        glm::vec2 del = mStartPoint - glm::vec2(x, y);
        mStartPoint = glm::vec2(x, y);
        mEuler.x += del.y * 0.002f;
        mEuler.y += del.x * 0.002f;
    }

    void scroll(float delta) {
        glm::mat4 R;
        R = glm::toMat4(glm::quat(mEuler));
        glm::vec3 e = glm::vec3(R[2][0], R[2][1], R[2][2]);
        mEye += glm::normalize(glm::vec3(R[2][0], R[2][1], R[2][2])) * delta * -0.2f;
    }

    void updateTarget(float pitch, float yaw) {
        mTarget = mEye + glm::vec3(glm::eulerAngleYXZ(yaw, pitch, 0.0f) * glm::vec4(0.0, 0.0, 1.0, 0.0));
        glm::vec3 dir = glm::vec3(mTarget - mEye);
    }

private:
    bool mGrabbing = false;
    glm::vec3 mEye;
    glm::vec3 mTarget;
    // 只考虑pitch还有yaw
    glm::vec3 mEuler;
    glm::vec2 mStartPoint;
    EventHandle mOnMousePositionHandle;
    EventHandle mOnMouseButtonHandle;
    EventHandle mOnMouseScrollHandle;
};
CameraController* gCamController = nullptr;

static void resizeCB(GLFWwindow* window, int width, int height) {
    gWidth = width;
    gHeight = height;
}

void cursorPositionCB(GLFWwindow * window, double posX, double posY) {
    gear::gEngine.getInputSystem()->onMousePosition(posX, posY);
}

void mouseButtonCB(GLFWwindow * window, int button, int action, int mods) {
    gear::gEngine.getInputSystem()->onMouseButton(button, action);
}

void mouseScrollCB(GLFWwindow * window, double offsetX, double offsetY) {
    gear::gEngine.getInputSystem()->onMouseScroll(offsetY);
}

void createTestScene() {
    // 初始化测试资源
    gear::MaterialCompiler materialCompiler;
    std::string materialCode = gear::readFileData("./BuiltinResources/Materials/default.mat");
    gUIMat = materialCompiler.compile(materialCode);
    gUIMatIns = gUIMat->createInstance();

    gear::VertexBuffer::Builder vbBuilder;
    vbBuilder.vertexCount(4);
    vbBuilder.attribute(Blast::SEMANTIC_POSITION, Blast::FORMAT_R32G32_FLOAT);
    vbBuilder.attribute(Blast::SEMANTIC_TEXCOORD0, Blast::FORMAT_R32G32_FLOAT);
    gVB = vbBuilder.build();
    gVB->update(vertices, 0, sizeof(vertices));

    gear::IndexBuffer::Builder ibBuilder;
    ibBuilder.indexCount(6);
    ibBuilder.indexType(Blast::INDEX_TYPE_UINT32);
    gIB = ibBuilder.build();
    gIB->update(indices, 0, sizeof(indices));

    // 初始化相机
    {
        gCamera = gear::gEngine.getScene()->createEntity();
        gear::CTransform* ct = gCamera->addComponent<gear::CTransform>();
        ct->setTransform(glm::mat4(1.0));
        gCamera->addComponent<gear::CCamera>()->setProjection(gear::ProjectionType::PERSPECTIVE, 0.0, 800.0, 600.0, 0.0, 0.1, 100.0);
    }

    // 初始化Pawn
    {
        gPawn = gear::gEngine.getScene()->createEntity();
        gear::CTransform* ct = gPawn->addComponent<gear::CTransform>();
        ct->setTransform(glm::mat4(1.0));

//        gear::RenderPrimitive primitive;
//        primitive.count = 6;
//        primitive.offset = 0;
//        primitive.type = Blast::PRIMITIVE_TOPO_TRI_LIST;
//        primitive.materialInstance = gUIMatIns;
//        primitive.vertexBuffer = gVB;
//        primitive.indexBuffer = gIB;
//        gPawn->addComponent<gear::CRenderable>()->addPrimitive(primitive);
    }

    gTestTex = gEditor.getTextureImporter()->importTexture2D("./BuiltinResources/Textures/test.png");

    Blast::GfxSamplerDesc samplerDesc;
    gUIMatIns->setParameter("albedo_texture", gTestTex, samplerDesc);
    gltfAsset = gEditor.getGltfImporter()->import("./BuiltinResources/GltfFiles/test.gltf");
}

void destroyTestScene() {
    gEditor.getGltfImporter()->destroyGltfAsset(gltfAsset);
    SAFE_DELETE(gDefaultMat);
    SAFE_DELETE(gDefaultMatIns);
    SAFE_DELETE(gUIMat);
    SAFE_DELETE(gUIMatIns);
    SAFE_DELETE(gVB);
    SAFE_DELETE(gIB);
    SAFE_DELETE(gTestTex);
    gear::gEngine.getScene()->destroyEntity(gCamera);
    gear::gEngine.getScene()->destroyEntity(gPawn);
}

int main()
{
    // 初始化glfw
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    gWindowPtr = glfwCreateWindow(gWidth, gHeight, "GearEditor", nullptr, nullptr);
    glfwSetWindowUserPointer(gWindowPtr, nullptr);
    glfwSetFramebufferSizeCallback(gWindowPtr, resizeCB);
    glfwSetMouseButtonCallback(gWindowPtr, mouseButtonCB);
    glfwSetCursorPosCallback(gWindowPtr, cursorPositionCB);
    glfwSetScrollCallback(gWindowPtr, mouseScrollCB);

    // 初始化imgui
    gImGuiLayout = new ImGuiLayout(gWindowPtr);

    gear::gEngine.getRenderer()->initSurface(glfwGetWin32Window(gWindowPtr));

    gCamController = new CameraController();
    createTestScene();

    while (!glfwWindowShouldClose(gWindowPtr)) {
        glfwPollEvents();
        // 更新相机
        gCamera->getComponent<gear::CTransform>()->setTransform(gCamController->getCameraMatrix());

        // 每一帧的开始都需要获取当前屏幕信息
//        gImGuiLayout->beginFrame();
//
//        bool show_demo_window = true;
//        ImGui::ShowDemoWindow(&show_demo_window);
//
//        gImGuiLayout->endFrame();

        gear::gEngine.getRenderer()->beginFrame(gWidth, gHeight);
        gear::gEngine.getRenderer()->endFrame();

        // 重置input状态
        gear::gEngine.getInputSystem()->reset();
    }
    // 确保渲染器结束所有的工作
    gear::gEngine.getRenderer()->terminate();
    destroyTestScene();
    delete gCamController;

    // 销毁imgui
    SAFE_DELETE(gImGuiLayout);

    // 销毁glfw
    glfwTerminate();
    return 0;
}