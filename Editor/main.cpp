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
#include <Utility/Log.h>
#include <iostream>
#include <fstream>
#define STB_IMAGE_IMPLEMENTATION
#define STBIR_FLAG_ALPHA_PREMULTIPLIED
#include <stb_image.h>
#include <imgui.h>
#include "ImGuiBackend.h"

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

class CameraController {
public:
    CameraController() {
        mEye = glm::vec3(0.0);
        mEuler.x = 0.0;
        mEuler.y = 0.0;
        mEuler.z = 0.0;
    }

    ~CameraController() = default;

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

    glm::mat4 getCameraMatrix() {
        glm::mat4 R, T;
        R = glm::toMat4(glm::quat(mEuler));
        T = glm::translate(glm::mat4(1.0), mEye);
        return T * R;
    }

private:
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
};
CameraController* gCamController = nullptr;

static std::string readFileData(const std::string& path);

static void resizeCB(GLFWwindow* window, int width, int height) {
    gWidth = width;
    gHeight = height;
}

void cursorPositionCB(GLFWwindow * window, double posX, double posY) {
    gCamController->update(posX, posY);
}

void mouseButtonCB(GLFWwindow * window, int button, int action, int mods) {
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    switch (action) {
        case 0:
            // Button Up
            if (button == 1) {
                gCamController->end();
            }
            break;
        case 1:
            // Button Down
            if (button == 1) {
                gCamController->begin(x, y);
            }
            break;
        default:
            break;
    }
}

void mouseScrollCB(GLFWwindow * window, double offsetX, double offsetY) {
    gCamController->scroll(offsetY);
}

void createTestScene() {
    // 初始化测试资源
    gear::MaterialCompiler materialCompiler;
    std::string materialCode = readFileData("./BuiltinResources/Materials/default.mat");
//    gDefaultMat = materialCompiler.compile(materialCode);
//    gDefaultMatIns = gDefaultMat->createInstance();

    materialCode = readFileData("./BuiltinResources/Materials/ui.mat");
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
        gear::CCamera* cc = gCamera->addComponent<gear::CCamera>();
    }

    // 初始化Pawn
    {
        gPawn = gear::gEngine.getScene()->createEntity();
        gear::CTransform* ct = gPawn->addComponent<gear::CTransform>();
        ct->setTransform(glm::mat4(1.0));

        gear::RenderPrimitive primitive;
        primitive.count = 6;
        primitive.offset = 0;
        primitive.type = Blast::PRIMITIVE_TOPO_TRI_LIST;
        primitive.materialInstance = gUIMatIns;
        gear::CRenderable* cr = gPawn->addComponent<gear::CRenderable>();
        cr->addPrimitive(primitive);
        cr->setVertexBuffer(gVB);
        cr->setIndexBuffer(gIB);
    }

    // 加载测试纹理
    int texWidth, texHeight, texChannels;
    std::string imagePath = "./BuiltinResources/Textures/test.png";
    unsigned char* pixels = stbi_load(imagePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

    // 做预乘处理
    for (int i = 0; i < texWidth; ++i) {
        for (int j = 0; j < texHeight; ++j) {
            unsigned bytePerPixel = texChannels;
            unsigned char* pixelOffset = pixels + (i + texWidth * j) * bytePerPixel;
            float alpha = pixelOffset[3] / 255.0f;
            pixelOffset[0] *= alpha;
            pixelOffset[1] *= alpha;
            pixelOffset[2] *= alpha;
        }
    }

    uint32_t imageSize = texWidth * texHeight * texChannels;
    gear::Texture::Builder texBuild;
    texBuild.width(texWidth);
    texBuild.height(texHeight);
    texBuild.format(Blast::FORMAT_R8G8B8A8_UNORM);
    gTestTex = texBuild.build();
    gTestTex->setData(0, pixels, imageSize);
    stbi_image_free(pixels);

    Blast::GfxSamplerDesc samplerDesc;
    gUIMatIns->setParameter("albedo_texture", gTestTex, samplerDesc);
}

void destroyTestScene() {
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
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_Init(gWindowPtr, true);

    gear::gEngine.getRenderer()->initSurface(glfwGetWin32Window(gWindowPtr));

    gCamController = new CameraController();
    createTestScene();

    while (!glfwWindowShouldClose(gWindowPtr)) {
        // 更新相机
        gear::CTransform* ct = gCamera->getComponent<gear::CTransform>();
        ct->setTransform(gCamController->getCameraMatrix());

        // 每一帧的开始都需要获取当前屏幕信息
        // ImGui_ImplGlfw_NewFrame();


        gear::gEngine.getRenderer()->beginFrame(gWidth, gHeight);
        gear::gEngine.getRenderer()->endFrame();
        glfwPollEvents();
    }
    // 确保渲染器结束所有的工作
    gear::gEngine.getRenderer()->terminate();
    destroyTestScene();
    delete gCamController;

    // 销毁imgui
    ImGui::DestroyContext();
    ImGui_ImplGlfw_Shutdown();

    // 销毁glfw
    glfwTerminate();
    return 0;
}

std::string readFileData(const std::string& path) {
    std::istream* stream = &std::cin;
    std::ifstream file;

    file.open(path, std::ios_base::binary);
    stream = &file;
    if (file.fail()) {
        LOGW("cannot open input file %s \n", path.c_str());
        return std::string("");
    }
    return std::string((std::istreambuf_iterator<char>(*stream)), std::istreambuf_iterator<char>());
}