#include <iostream>
#include <Application/Application.h>
#include <Application/Window.h>
#include <RHI/RHI.h>
#include <RHI/RHIPipelineState.h>
#include <RHI/RHITexture.h>
#include <RHI/RHITextureView.h>
#include <RHI/RHIFramebuffer.h>
#include <Utility/FileSystem.h>

struct Vertex {
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec2 uv;
};

float vertices[] = {
	0.0f,  0.5f, 0.0f, 0.0, 0.0, 0.0, 0.0, 0.0,
	-0.5f, -0.5f, 0.0f, 0.0, 0.0, 0.0, 0.0, 0.0,
	 0.5f, -0.5f, 0.0f, 0.0, 0.0, 0.0, 0.0, 0.0
};

unsigned int indices[] = {
	2, 1, 0
};

class MyApplication : public Application
{
public:
	MyApplication()
		:Application()
	{
	}
	~MyApplication()
	{
	}
	virtual void prepare()
	{
        std::string path = FileSystem::getCurPath();

        RHIDevice* device = RHI::instance().getDevice();

		RHIProgramInfo programInfo;
		programInfo.type = RHIProgramType::Vertex;
		programInfo.entryPoint = "main";
		FileSystem::readFile(path + "/Resource/Shaders/default.vert", programInfo.source);
		mTestVertexProgram = device->createProgram(programInfo);
		mTestVertexProgram->compile();

		programInfo.type = RHIProgramType::Fragment;
		programInfo.entryPoint = "main";
        FileSystem::readFile(path + "/Resource/Shaders/default.frag", programInfo.source);
		mTestFragmentProgram = device->createProgram(programInfo);
		mTestFragmentProgram->compile();

		RHIPipelineStateInfo pipelineInfo;
		pipelineInfo.vertexProgram = mTestVertexProgram;
		pipelineInfo.fragmentProgram = mTestFragmentProgram;
		mTestPipeline = new RHIGraphicsPipelineState(device, pipelineInfo);

		mTestVertexbuffer = device->createVertexBuffer(sizeof(Vertex), 3);
		mTestVertexbuffer->writeData(0,sizeof(vertices), vertices);

		mTestIndexbuffer = device->createIndexBuffer(sizeof(unsigned int),3);
		mTestIndexbuffer->writeData(0,sizeof(indices), indices);
	}

	virtual void runMainLoop()
	{
		while (!glfwWindowShouldClose(mWindow->getWindowPtr()))
		{
			mWindow->beginFrame();
            RHICommandBufferPool* pool = RHI::instance().getDevice()->getHelperCmdBufferPool();
            RHICommandBuffer* cmdBuf = pool->getActiveCmdBuffer();

            cmdBuf->setRenderTarget(mWindow->getFramebuffer());
            cmdBuf->setViewport(glm::vec4(0, 0, 800, 600));
            cmdBuf->setScissor(glm::vec4(0, 0, 800, 600));
            cmdBuf->bindGraphicsPipelineState(mTestPipeline);
            cmdBuf->bindIndexBuffer(mTestIndexbuffer);
            cmdBuf->bindVertexBuffer(mTestVertexbuffer);
            cmdBuf->begin();
            cmdBuf->beginRenderPass(glm::vec4(0,0,800,600));
            cmdBuf->drawIndexed(3,1,0,0,0);
            cmdBuf->endRenderPass();
            cmdBuf->end();
            cmdBuf->submit();

            mWindow->endFrame();
			Input::instance().update();
			glfwPollEvents();
		}
	}
	virtual void finish()
	{
		SAFE_DELETE(mTestCmdBuffer);
        SAFE_DELETE(mTestVertexProgram);
        SAFE_DELETE(mTestFragmentProgram);
        SAFE_DELETE(mTestPipeline);
        SAFE_DELETE(mTestVertexbuffer);
        SAFE_DELETE(mTestIndexbuffer);
	}

private:
	RHICommandBuffer* mTestCmdBuffer = nullptr;
	RHIProgram* mTestVertexProgram = nullptr;
	RHIProgram* mTestFragmentProgram = nullptr;
	RHIGraphicsPipelineState* mTestPipeline = nullptr;
	RHIVertexBuffer* mTestVertexbuffer = nullptr;
	RHIIndexBuffer* mTestIndexbuffer = nullptr;
};

int main()
{
	MyApplication app;
	app.prepare();
	app.runMainLoop();
	app.finish();
	return 0;
}