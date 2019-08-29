#include <iostream>
#include "Application/Application.h"
#include "Application/Window.h"
#include "RHI/RHI.h"
#include "RHI/RHIPipelineState.h"
#include "RHI/RHITexture.h"
#include "RHI/RHITextureView.h"
#include "RHI/RHIFramebuffer.h"
#include "Utility/FileSystem.h"

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
		RHIDevice* device = RHI::instance().getDevice();
		// 创建cmd buffer
		mTestCmdBuffer = device->allocCommandBuffer(CommandBufferType::GRAPHICS, true);

		// 创建pipeline state
		RHIProgramInfo programInfo;
		programInfo.type = RHIProgramType::Vertex;
		programInfo.entryPoint = "main";
		readFile("D:/GearEngine/GearEngine/Resource/Shaders/default.vert", programInfo.source);
		mTestVertexProgram = device->createProgram(programInfo);
		mTestVertexProgram->compile();

		programInfo.type = RHIProgramType::Fragment;
		programInfo.entryPoint = "main";
		readFile("D:/GearEngine/GearEngine/Resource/Shaders/default.frag", programInfo.source);
		mTestFragmentProgram = device->createProgram(programInfo);
		mTestFragmentProgram->compile();

		RHIPipelineStateInfo pipelineInfo;
		pipelineInfo.vertexProgram = mTestVertexProgram;
		pipelineInfo.fragmentProgram = mTestFragmentProgram;
		// 快速验证直接创建pso对象
		mTestPipeline = new RHIGraphicsPipelineState(device, pipelineInfo);

		// 创建render target
		RHIColorAttachmentInfo color;
		color.format = VK_FORMAT_R8G8B8A8_UNORM;
		color.numSample = 1;
		RHIRenderPassInfo passInfo;
		passInfo.color[0] = color;
		passInfo.hasDepth = false;
		passInfo.numColorAttachments = 1;
		mTestRenderpass = new RHIRenderPass(device, passInfo);

		RHITextureInfo texInfo;
		texInfo.extent.width = 800;
		texInfo.extent.height = 600;
		texInfo.extent.depth = 1;
		texInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
		texInfo.mipLevels = 1;
		texInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		texInfo.arrayLayers = 1;
		texInfo.type = VK_IMAGE_TYPE_2D;
		texInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		mTestTex = new RHITexture(device, texInfo);

		RHITextureViewInfo texViewInfo;
		texViewInfo.texture = mTestTex;
		texViewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
		texViewInfo.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		texViewInfo.baseArrayLayer = 0;
		texViewInfo.baseMipLevel = 0;
		texViewInfo.layerCount = 1;
		texViewInfo.levelCount = 1;
		
		mTestTexView = new RHITextureView(device, texViewInfo);

		RHIFramebufferInfo fbInfo;
		fbInfo.color[0] = mTestTexView;
		fbInfo.depth = nullptr;
		fbInfo.width = 800;
		fbInfo.height = 600;
		fbInfo.renderpass = mTestRenderpass;
		fbInfo.layers = 1;
		fbInfo.numColorAttachments = 1;
		fbInfo.hasDepth = false;

		mTestFramebuffer = new RHIFramebuffer(device, fbInfo);
	}
	virtual void runMainLoop()
	{
		while (!glfwWindowShouldClose(mWindow->getWindowPtr()))
		{
			mWindow->beginFrame();
			// 插入执行代码
			
			mWindow->endFrame();
			Input::instance().update();
			glfwPollEvents();
		}
	}
	virtual void finish()
	{
		delete mTestCmdBuffer;
		delete mTestVertexProgram;
		delete mTestFragmentProgram;
		delete mTestPipeline;
		delete mTestRenderpass;
		delete mTestFramebuffer;
		delete mTestTex;
		delete mTestTexView;
	}

private:
	RHICommandBuffer* mTestCmdBuffer;
	RHIProgram* mTestVertexProgram;
	RHIProgram* mTestFragmentProgram;
	RHIGraphicsPipelineState* mTestPipeline;
	RHIRenderPass* mTestRenderpass;
	RHIFramebuffer* mTestFramebuffer;
	RHITexture* mTestTex;
	RHITextureView* mTestTexView;
};

int main()
{
	MyApplication app;
	app.prepare();
	app.runMainLoop();
	app.finish();
	return 0;
}