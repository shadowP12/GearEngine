#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <Blast/Gfx/GfxBuffer.h>
#include <Blast/Gfx/GfxContext.h>
#include <Blast/Utility/ShaderCompiler.h>
#include <Engine/GearEngine.h>
#include <Engine/Renderer/Renderer.h>
#include <Engine/Resource/Material.h>
#include <Engine/Resource/GpuBuffer.h>
#include <Engine/Scene/Scene.h>
#include <Engine/Scene/Entity.h>
#include <Engine/Scene/Components/CCamera.h>
#include <Engine/Scene/Components/CTransform.h>
#include <Engine/Scene/Components/CRenderable.h>
#include <Engine/MaterialCompiler/MaterialCompiler.h>
#include <Utility/Log.h>
#include <iostream>
#include <fstream>

struct Vertex {
    float pos[3];
    float uv[2];
};

float vertices[] = {
        -0.5f,  -0.5f, 0.0f, 0.0f, 0.0f,
        0.5f, -0.5f, 1.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.0f, 1.0f, 1.0f,
        -0.5f, 0.5f, 0.0f, 0.0f, 1.0f
};

unsigned int indices[] = {
        0, 1, 2, 2, 3, 0
};


GLFWwindow* gWindowPtr = nullptr;
uint32_t gWidth = 800;
uint32_t gHeight = 600;
gear::Material* gDefaultMat = nullptr;
gear::MaterialInstance* gDefaultMatIns = nullptr;
gear::VertexBuffer* gVB = nullptr;
gear::IndexBuffer* gIB = nullptr;
gear::Entity* gCamera = nullptr;
gear::Entity* gPawn = nullptr;

static std::string readFileData(const std::string& path);

static void resizeCB(GLFWwindow* window, int width, int height) {
    gWidth = width;
    gHeight = height;
}

void createTestScene() {
    // 初始化测试资源
    gear::MaterialCompiler materialCompiler;
    std::string materialCode = readFileData("./BuiltinResources/Materials/default.mat");
    gDefaultMat = materialCompiler.compile(materialCode);
    gDefaultMatIns = gDefaultMat->createInstance();

    gear::VertexBuffer::Builder vbBuilder;
    vbBuilder.vertexCount(4);
    vbBuilder.attribute(Blast::SEMANTIC_POSITION, Blast::FORMAT_R32G32B32_FLOAT);
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
        ct->setTransform(glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 0, 1), glm::vec3(0, 1, 0)));

        gear::CCamera* cc = gCamera->addComponent<gear::CCamera>();
    }

    // 初始化Pawn
    {
        gPawn = gear::gEngine.getScene()->createEntity();
        gear::CTransform* ct = gPawn->addComponent<gear::CTransform>();
        ct->setTransform(glm::mat4(1.0));

        gear::CRenderable* cr = gPawn->addComponent<gear::CRenderable>();
        cr->setPrimitive({Blast::PRIMITIVE_TOPO_TRI_LIST , 0, 6});
        cr->setVertexBuffer(gVB);
        cr->setIndexBuffer(gIB);
        cr->setMaterialInstance(gDefaultMatIns);
    }
}

void destroyTestScene() {
    SAFE_DELETE(gDefaultMat);
    SAFE_DELETE(gDefaultMatIns);
    SAFE_DELETE(gVB);
    SAFE_DELETE(gIB);
    gear::gEngine.getScene()->destroyEntity(gCamera);
    gear::gEngine.getScene()->destroyEntity(gPawn);
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    gWindowPtr = glfwCreateWindow(gWidth, gHeight, "GearEditor", nullptr, nullptr);
    glfwSetWindowUserPointer(gWindowPtr, nullptr);
    glfwSetFramebufferSizeCallback(gWindowPtr, resizeCB);

    gear::gEngine.getRenderer()->initSurface(glfwGetWin32Window(gWindowPtr));

    createTestScene();

    while (!glfwWindowShouldClose(gWindowPtr)) {
        gear::gEngine.getRenderer()->beginFrame(gWidth, gHeight);
        gear::gEngine.getRenderer()->endFrame();
        glfwPollEvents();
    }

    destroyTestScene();

    glfwDestroyWindow(gWindowPtr);
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