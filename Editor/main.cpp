#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <Blast/Gfx/GfxBuffer.h>
#include <Blast/Gfx/GfxContext.h>
#include <Blast/Utility/ShaderCompiler.h>
#include <Engine/GearEngine.h>
#include <Engine/Renderer/Renderer.h>
#include <Engine/Renderer/RenderScene.h>
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

Blast::GfxShader* vertShader = nullptr;
Blast::GfxShader* fragShader = nullptr;
Blast::GfxBuffer* meshIndexBuffer = nullptr;
Blast::GfxBuffer* meshVertexBuffer = nullptr;

GLFWwindow* gWindowPtr = nullptr;

void getSurfaceSize(int* w, int* h) {
    glfwGetFramebufferSize(gWindowPtr, w, h);
}

static std::string readFileData(const std::string& path);

int main()
{
    // 测试材质编辑器
    std::string materialCode = readFileData("./BuiltinResources/Materials/default.mat");
    gear::MaterialCompiler materialCompiler;
    materialCompiler.compile("./BuiltinResources/Materials/default.mat");
    return 0;
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    gWindowPtr = glfwCreateWindow(800, 600, "GearEditor", nullptr, nullptr);
    glfwSetWindowUserPointer(gWindowPtr, nullptr);
    gear::gEngine.getRenderer()->initSurface(glfwGetWin32Window(gWindowPtr));
    gear::gEngine.getRenderer()->setSurfaceSizeFunc(getSurfaceSize);
    gear::gEngine.getRenderer()->resize(800, 600);

    gear::RenderView* mainView = gear::gEngine.getRenderer()->getScene()->genView();

    Blast::GfxBufferDesc bufferDesc;
    bufferDesc.size = sizeof(Vertex) * 4;
    bufferDesc.type = Blast::RESOURCE_TYPE_VERTEX_BUFFER;
    bufferDesc.usage = Blast::RESOURCE_USAGE_CPU_TO_GPU;
    meshVertexBuffer = gear::gEngine.getRenderer()->getContext()->createBuffer(bufferDesc);
    meshVertexBuffer->writeData(0,sizeof(vertices), vertices);

    bufferDesc.size = sizeof(unsigned int) * 6;
    bufferDesc.type = Blast::RESOURCE_TYPE_INDEX_BUFFER;
    bufferDesc.usage = Blast::RESOURCE_USAGE_CPU_TO_GPU;
    meshIndexBuffer = gear::gEngine.getRenderer()->getContext()->createBuffer(bufferDesc);
    meshIndexBuffer->writeData(0, sizeof(indices), indices);


    {
        Blast::ShaderCompileDesc compileDesc;
        compileDesc.code = readFileData("./Resource/Shaders/triangle.vert");
        compileDesc.stage = Blast::SHADER_STAGE_VERT;
        Blast::ShaderCompileResult compileResult = gear::gEngine.getRenderer()->getShaderCompiler()->compile(compileDesc);

        Blast::GfxShaderDesc shaderDesc;
        shaderDesc.stage = Blast::SHADER_STAGE_VERT;
        shaderDesc.bytes = compileResult.bytes;
        vertShader = gear::gEngine.getRenderer()->getContext()->createShader(shaderDesc);
    }

    {
        Blast::ShaderCompileDesc compileDesc;
        compileDesc.code = readFileData("./Resource/Shaders/triangle.frag");
        compileDesc.stage = Blast::SHADER_STAGE_FRAG;
        Blast::ShaderCompileResult compileResult = gear::gEngine.getRenderer()->getShaderCompiler()->compile(compileDesc);

        Blast::GfxShaderDesc shaderDesc;
        shaderDesc.stage = Blast::SHADER_STAGE_FRAG;
        shaderDesc.bytes = compileResult.bytes;
        fragShader = gear::gEngine.getRenderer()->getContext()->createShader(shaderDesc);
    }

    gear::RenderPrimitive* p = gear::gEngine.getRenderer()->getScene()->genPrimitive();
    p->vertexBuffer = meshVertexBuffer;
    p->indexBuffer = meshIndexBuffer;
    p->offset = 0;
    p->vertexShader = vertShader;
    p->pixelShader = fragShader;
    while (!glfwWindowShouldClose(gWindowPtr)) {
        gear::gEngine.getRenderer()->render();
        glfwPollEvents();
    }
    gear::gEngine.getRenderer()->getScene()->deleteView(mainView);

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