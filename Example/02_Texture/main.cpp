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
#include <RHI/RHIDescriptorSet.h>
#include <Utility/FileSystem.h>
#include <RHI/Managers/SpirvManager.h>
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
#include <algorithm>
#include <chrono>

struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

struct Vertex {
	glm::vec3 pos;
	glm::vec2 uv;
};

float vertices[] = {
	-0.5f,  -0.5f, 0.0f, 1.0f, 0.0f,
	0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
	 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
    -0.5f, 0.5f, 0.0f, 1.0f, 1.0f
};

unsigned int indices[] = {
	0, 1, 2, 2, 3, 0
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
        FileSystem::readFile("./Resource/Shaders/texture.vert", spirvCompileInfo.source);
        compileResult = SpirvManager::instance().compile(spirvCompileInfo);
        programInfo.type = PROGRAM_VERTEX;
        programInfo.bytes = compileResult.bytes;
        mVertexProgram = new RHIProgram(mDevice, programInfo);

        spirvCompileInfo.stageType = STAGE_FRAGMENT;
        spirvCompileInfo.entryPoint = "main";
        FileSystem::readFile("./Resource/Shaders/texture.frag", spirvCompileInfo.source);
        compileResult = SpirvManager::instance().compile(spirvCompileInfo);
        programInfo.type = PROGRAM_FRAGMENT;
        programInfo.bytes = compileResult.bytes;
        mFragmentProgram = new RHIProgram(mDevice, programInfo);

        RHIBufferInfo bufferInfo;
        bufferInfo.size = sizeof(Vertex) * 4;
        bufferInfo.descriptors = DESCRIPTOR_TYPE_VERTEX_BUFFER;
        bufferInfo.memoryUsage = RESOURCE_MEMORY_USAGE_CPU_TO_GPU;
        mVertexbuffer = new RHIBuffer(mDevice, bufferInfo);
		mVertexbuffer->writeData(0,sizeof(vertices), vertices);

        bufferInfo.size = sizeof(unsigned int) * 6;
        bufferInfo.descriptors = DESCRIPTOR_TYPE_INDEX_BUFFER;
        bufferInfo.memoryUsage = RESOURCE_MEMORY_USAGE_CPU_TO_GPU;
        mIndexbuffer = new RHIBuffer(mDevice, bufferInfo);
		mIndexbuffer->writeData(0, sizeof(indices), indices);

        bufferInfo.size = sizeof(UniformBufferObject);
        bufferInfo.descriptors = DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        bufferInfo.memoryUsage = RESOURCE_MEMORY_USAGE_CPU_TO_GPU;
        mUniformBuffer = new RHIBuffer(mDevice, bufferInfo);

        {
            // load texture
            int texWidth, texHeight, texChannels;
            unsigned char* pixels = stbi_load("./Resource/Textures/test.png", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
            uint32_t imageSize = texWidth * texHeight * 4;
            bufferInfo.size = imageSize;
            bufferInfo.descriptors = DESCRIPTOR_TYPE_BUFFER;
            bufferInfo.memoryUsage = RESOURCE_MEMORY_USAGE_CPU_TO_GPU;
            RHIBuffer* stagingBuffer = new RHIBuffer(mDevice, bufferInfo);
            stagingBuffer->writeData(0, imageSize, pixels);
            stbi_image_free(pixels);

            RHITextureInfo textureInfo;
            textureInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
            textureInfo.descriptors = DESCRIPTOR_TYPE_TEXTURE;
            textureInfo.width = texWidth;
            textureInfo.height = texHeight;
            textureInfo.depth = 1;
            textureInfo.mipLevels = 1;
            textureInfo.arrayLayers = 1;
            mDebugTexture = new RHITexture(mDevice, textureInfo);

            SubresourceDataInfo subresourceDataInfo;
            subresourceDataInfo.mipLevel = 0;
            subresourceDataInfo.arrayLayer = 0;

            RHICommandBuffer* cmdBuf = mDevice->getGraphicsCommandPool()->getActiveCmdBuffer();
            cmdBuf->begin();
            RHITextureBarrier barrier = { mDebugTexture, RESOURCE_STATE_COPY_DEST };
            cmdBuf->setResourceBarrier(0, nullptr, 1, &barrier);
            cmdBuf->updateSubresource(mDebugTexture, stagingBuffer, subresourceDataInfo);
            barrier = {mDebugTexture, RESOURCE_STATE_SHADER_RESOURCE };
            cmdBuf->setResourceBarrier(0, nullptr, 1, &barrier);
            cmdBuf->end();
            RHIQueueSubmitInfo submitInfo;
            submitInfo.cmdBuf = cmdBuf;
            mDevice->getGraphicsQueue()->submit(submitInfo);
            mDevice->getGraphicsQueue()->waitIdle();
            SAFE_DELETE(stagingBuffer);
        }

        RHISamplerInfo samplerInfo;
        mSampler = new RHISampler(mDevice, samplerInfo);

        RHIDescriptorSetInfo descriptorSetInfo;
        descriptorSetInfo.set = 0;
        descriptorSetInfo.bindingCount = 2;
        descriptorSetInfo.bindings[0].binding = 0;
        descriptorSetInfo.bindings[0].descriptorCount = 1;
        descriptorSetInfo.bindings[0].type = DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorSetInfo.bindings[0].stage = PROGRAM_VERTEX;
        descriptorSetInfo.bindings[1].binding = 1;
        descriptorSetInfo.bindings[1].descriptorCount = 1;
        descriptorSetInfo.bindings[1].type = DESCRIPTOR_TYPE_TEXTURE;
        descriptorSetInfo.bindings[1].stage = PROGRAM_FRAGMENT;
        mDescriptorSet = new RHIDescriptorSet(mDevice, descriptorSetInfo);
        mDescriptorSet->updateBuffer(0, DESCRIPTOR_TYPE_UNIFORM_BUFFER, mUniformBuffer, sizeof(UniformBufferObject), 0);
        mDescriptorSet->updateTexture(1, DESCRIPTOR_TYPE_TEXTURE, mDebugTexture, mSampler);

		VertexLayout vertexLayout;
		vertexLayout.attribCount = 2;
        vertexLayout.attribs[0].location = 0;
		vertexLayout.attribs[0].binding = 0;
        vertexLayout.attribs[0].offset = offsetof(Vertex, pos);
        vertexLayout.attribs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        vertexLayout.attribs[0].rate = VERTEX_ATTRIB_RATE_VERTEX;
        vertexLayout.attribs[0].semantic = SEMANTIC_POSITION;

        vertexLayout.attribs[1].location = 1;
        vertexLayout.attribs[1].binding = 0;
        vertexLayout.attribs[1].offset = offsetof(Vertex, uv);
        vertexLayout.attribs[1].format = VK_FORMAT_R32G32_SFLOAT;
        vertexLayout.attribs[1].rate = VERTEX_ATTRIB_RATE_VERTEX;
        vertexLayout.attribs[1].semantic = SEMANTIC_TEXCOORD0;

        RHIGraphicsPipelineInfo pipelineInfo;
        pipelineInfo.renderPass = mSwapChain->getRenderPass();
        pipelineInfo.renderTargetCount = 1;
        pipelineInfo.descriptorSetCount = 1;
        pipelineInfo.descriptorSets = &mDescriptorSet;
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
        updateUniformBuffer();
        RHICommandBuffer* cmdBuf = mDevice->getGraphicsCommandPool()->getActiveCmdBuffer();
        cmdBuf->begin();
        RHITexture* colorTarget = mSwapChain->getColorTexture(imageIndex);
        RHITexture* depthTarget = mSwapChain->getDepthStencilTexture(imageIndex);
        RHITextureBarrier barriers[] = { { colorTarget, RESOURCE_STATE_RENDER_TARGET },
                                         { depthTarget, RESOURCE_STATE_DEPTH_WRITE } };
        cmdBuf->setResourceBarrier(0, nullptr, 2, barriers);
        // drawing
        cmdBuf->bindFramebuffer(mSwapChain->getFramebuffer(imageIndex));
        cmdBuf->bindGraphicsPipeline(mPipeline, &mDescriptorSet, 1);
        cmdBuf->setViewport(0, 0, 800, 600);
        cmdBuf->setScissor(0, 0, 800, 600);
        cmdBuf->bindIndexBuffer(mIndexbuffer, 0, VK_INDEX_TYPE_UINT32);
        cmdBuf->bindVertexBuffer(mVertexbuffer, 0);
        cmdBuf->drawIndexed(6, 1, 0, 0, 0);
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
	    SAFE_DELETE(mSampler);
	    SAFE_DELETE(mUniformBuffer);
	    SAFE_DELETE(mDebugTexture)
        SAFE_DELETE(mDescriptorSet)
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

	void updateUniformBuffer()
    {
        static auto startTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        UniformBufferObject ubo{};
        ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.proj = glm::perspective(glm::radians(45.0f), mWindow->getWidth() / (float) mWindow->getHeight(), 0.1f, 10.0f);
        ubo.proj[1][1] *= -1;

        mUniformBuffer->writeData(0, sizeof(ubo), &ubo);
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
    RHIBuffer* mUniformBuffer = nullptr;
    RHISampler* mSampler = nullptr;
    RHITexture* mDebugTexture = nullptr;
    RHIDescriptorSet* mDescriptorSet = nullptr;
};

int main()
{
	MyApplication app(800, 600);
	app.prepare();
	app.runMainLoop();
	app.finish();
	return 0;
}