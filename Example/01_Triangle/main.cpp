#include <iostream>
#include <Application/Application.h>
#include <Application/Window.h>
#include <RHI/RHIDevice.h>
#include <RHI/RHISwapChain.h>
#include <RHI/RHICommandBuffer.h>
#include <RHI/RHISynchronization.h>
#include <RHI/RHIPipeline.h>
#include <RHI/RHIBuffer.h>
#include <RHI/RHIQueue.h>
#include <RHI/RHITexture.h>
#include <RHI/RHIProgram.h>
#include <RHI/RHIFramebuffer.h>
#include <Utility/FileSystem.h>
#include <RHI/Managers/SpirvManager.h>

struct Vertex {
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec2 uv;
};

float vertices[] = {
	0.0f,  0.9f, 0.0f, 0.0, 0.0, 0.0, 0.0, 0.0,
	-0.5f, -0.5f, 0.0f, 0.0, 0.0, 0.0, 0.0, 0.0,
	 0.5f, -0.5f, 0.0f, 0.0, 0.0, 0.0, 0.0, 0.0
};

unsigned int indices[] = {
	2, 1, 0
};

class MyApplication : public Application
{
public:
	MyApplication(uint32_t width, uint32_t height)
		:Application(width, height)
	{
	    SpirvManager::startUp();
	}
	~MyApplication()
	{
	    SpirvManager::shutDown();
	}
	virtual void prepare()
	{
        mDevice = new RHIDevice();

        RHISwapChainInfo swapChainInfo;
        swapChainInfo.width = mWindow->getWidth();
        swapChainInfo.height = mWindow->getHeight();
        swapChainInfo.windowHandle = mWindow->getWindowPtr();
        mSwapChain = new RHISwapChain(mDevice, swapChainInfo);

        RHIProgramInfo programInfo;
        SpirvCompileResult compileResult;
        SpirvCompileInfo spirvCompileInfo;
        spirvCompileInfo.stageType = STAGE_VERTEX;
        spirvCompileInfo.entryPoint = "main";
        FileSystem::readFile("./Resource/Shaders/triangle.vert", spirvCompileInfo.source);
        compileResult = SpirvManager::instance().compile(spirvCompileInfo);
        programInfo.type = PROGRAM_VERTEX;
        programInfo.bytes = compileResult.bytes;
        mVertexProgram = new RHIProgram(mDevice, programInfo);

        spirvCompileInfo.stageType = STAGE_FRAGMENT;
        spirvCompileInfo.entryPoint = "main";
        FileSystem::readFile("./Resource/Shaders/triangle.frag", spirvCompileInfo.source);
        compileResult = SpirvManager::instance().compile(spirvCompileInfo);
        programInfo.type = PROGRAM_FRAGMENT;
        programInfo.bytes = compileResult.bytes;
        mFragmentProgram = new RHIProgram(mDevice, programInfo);

        RHIBufferInfo bufferInfo;
        bufferInfo.size = sizeof(Vertex) * 3;
        bufferInfo.descriptors = DESCRIPTOR_TYPE_VERTEX_BUFFER;
        bufferInfo.memoryUsage = RESOURCE_MEMORY_USAGE_CPU_TO_GPU;
        mVertexbuffer = new RHIBuffer(mDevice, bufferInfo);
		mVertexbuffer->writeData(0,sizeof(vertices), vertices);

        bufferInfo.size = sizeof(unsigned int) * 3;
        bufferInfo.descriptors = DESCRIPTOR_TYPE_INDEX_BUFFER;
        bufferInfo.memoryUsage = RESOURCE_MEMORY_USAGE_CPU_TO_GPU;
        mIndexbuffer = new RHIBuffer(mDevice, bufferInfo);
		mIndexbuffer->writeData(0,sizeof(indices), indices);

		VertexLayout vertexLayout;
		vertexLayout.attribCount = 3;
        vertexLayout.attribs[0].location = 0;
		vertexLayout.attribs[0].binding = 0;
        vertexLayout.attribs[0].offset = offsetof(Vertex, pos);
        vertexLayout.attribs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        vertexLayout.attribs[0].rate = VERTEX_ATTRIB_RATE_VERTEX;
        vertexLayout.attribs[0].semantic = SEMANTIC_POSITION;

        vertexLayout.attribs[1].location = 1;
        vertexLayout.attribs[1].binding = 0;
        vertexLayout.attribs[1].offset = offsetof(Vertex, normal);
        vertexLayout.attribs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        vertexLayout.attribs[1].rate = VERTEX_ATTRIB_RATE_VERTEX;
        vertexLayout.attribs[1].semantic = SEMANTIC_NORMAL;

        vertexLayout.attribs[2].location = 2;
        vertexLayout.attribs[2].binding = 0;
        vertexLayout.attribs[2].offset = offsetof(Vertex, uv);
        vertexLayout.attribs[2].format = VK_FORMAT_R32G32_SFLOAT;
        vertexLayout.attribs[2].rate = VERTEX_ATTRIB_RATE_VERTEX;
        vertexLayout.attribs[2].semantic = SEMANTIC_TEXCOORD0;

        RHIGraphicsPipelineInfo pipelineInfo;
        pipelineInfo.renderPass = mSwapChain->getRenderPass();
        pipelineInfo.renderTargetCount = 1;
        pipelineInfo.descriptorSetCount = 0;
        pipelineInfo.descriptorSets = nullptr;
        pipelineInfo.vertexProgram = mVertexProgram;
        pipelineInfo.fragmentProgram = mFragmentProgram;
        pipelineInfo.vertexLayout = vertexLayout;

		mPipeline = new RHIGraphicsPipeline(mDevice, pipelineInfo);

        mImageAvailableSemaphore = new RHISemaphore(mDevice);
        mRenderFinishedSemaphore = new RHISemaphore(mDevice);
	}

	virtual void run()
	{
        uint32_t imageIndex;
        mSwapChain->acquireNextImage(mImageAvailableSemaphore, nullptr, imageIndex);
        RHICommandBuffer* cmdBuf = mDevice->getGraphicsCommandPool()->getActiveCmdBuffer();
        cmdBuf->begin();
        RHITexture* colorTarget = mSwapChain->getColorTexture(imageIndex);
        RHITexture* depthTarget = mSwapChain->getDepthStencilTexture(imageIndex);
        RHITextureBarrier barriers[] = { { colorTarget, RESOURCE_STATE_RENDER_TARGET },
                                         { depthTarget, RESOURCE_STATE_DEPTH_WRITE } };
        cmdBuf->setResourceBarrier(0, nullptr, 2, barriers);
        // drawing
        cmdBuf->bindFramebuffer(mSwapChain->getFramebuffer(imageIndex));
        cmdBuf->bindGraphicsPipeline(mPipeline, nullptr, 0);
        cmdBuf->setViewport(0, 0, 800, 600);
        cmdBuf->setScissor(0, 0, 800, 600);
        cmdBuf->bindIndexBuffer(mIndexbuffer, 0, VK_INDEX_TYPE_UINT32);
        cmdBuf->bindVertexBuffer(mVertexbuffer, 0);
        cmdBuf->drawIndexed(3, 1, 0, 0, 0);
        cmdBuf->unbindFramebuffer();
        RHITextureBarrier finalBarriers[] = { { colorTarget, RESOURCE_STATE_PRESENT },
                                         { depthTarget, RESOURCE_STATE_DEPTH_WRITE } };
        cmdBuf->setResourceBarrier(0, nullptr, 2, finalBarriers);
        cmdBuf->end();
        RHIQueueSubmitInfo submitInfo;
        submitInfo.cmdBuf = cmdBuf;
        submitInfo.waitSemaphores = &mImageAvailableSemaphore;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.signalSemaphores = &mRenderFinishedSemaphore;
        submitInfo.signalSemaphoreCount = 1;
        mDevice->getGraphicsQueue()->submit(submitInfo);
        RHIQueuePresentInfo presentInfo;
        presentInfo.index = imageIndex;
        presentInfo.swapChain = mSwapChain;
        presentInfo.waitSemaphores = &mRenderFinishedSemaphore;
        presentInfo.waitSemaphoreCount = 1;
        mDevice->getGraphicsQueue()->Present(presentInfo);
	}

	virtual void finish()
	{
	    SAFE_DELETE(mImageAvailableSemaphore);
        SAFE_DELETE(mRenderFinishedSemaphore);
        SAFE_DELETE(mVertexProgram);
        SAFE_DELETE(mFragmentProgram);
        SAFE_DELETE(mVertexbuffer);
        SAFE_DELETE(mIndexbuffer);
        SAFE_DELETE(mPipeline);
        SAFE_DELETE(mSwapChain);
        SAFE_DELETE(mDevice);
	}

private:
    RHIDevice* mDevice = nullptr;
	RHIProgram* mVertexProgram = nullptr;
	RHIProgram* mFragmentProgram = nullptr;
	RHIBuffer* mVertexbuffer = nullptr;
	RHIBuffer* mIndexbuffer = nullptr;
	RHIGraphicsPipeline* mPipeline = nullptr;
	RHISwapChain* mSwapChain = nullptr;
    RHISemaphore* mImageAvailableSemaphore = nullptr;
    RHISemaphore* mRenderFinishedSemaphore = nullptr;
};

int main()
{
	MyApplication app(800, 600);
	app.prepare();
	app.runMainLoop();
	app.finish();
	return 0;
}