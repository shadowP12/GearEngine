#ifndef RENDERER_H
#define RENDERER_H
#include "../Utility/Module.h"
#include "../RenderAPI/Framebuffer.h"
#include "../RenderAPI/RenderPass.h"
#include "../RenderAPI/SwapChain.h"
#include "../RenderAPI/CommandBuffer.h"
#include "../Debugger/DebugCamera.h"
#include "RenderScene.h"

class Renderer : public Module<Renderer>
{
public:
	Renderer()
	{
	}
	~Renderer()
	{
	}
	void init(uint32_t w, uint32_t h)
	{
		mWidth = w;
		mHeight = h;
		std::vector<VkFormat> colors;
		colors.push_back(VK_FORMAT_B8G8R8A8_UNORM);
		mMainRenderPass = std::shared_ptr<RenderPass>(new RenderPass(colors, true, true));

		VkExtent2D extent;
		extent.width = mWidth;
		extent.height = mHeight;
		mSwapChain = std::shared_ptr<SwapChain>(new SwapChain(extent, mMainRenderPass));

		mMainCamera = std::shared_ptr<DebugCamera>(new DebugCamera(glm::vec3(0,0,0),-90,0));

		mScene = std::shared_ptr<RenderScene>(new RenderScene());

		createSemaphores();
	}
	void reSize(uint32_t w, uint32_t h)
	{
		mWidth = w;
		mHeight = h;
		VkExtent2D extent;
		extent.width = mWidth;
		extent.height = mHeight;
		mSwapChain->reCreateSwapChain(extent,mMainRenderPass);
	}

	void draw();

	std::shared_ptr<RenderPass> getRenderPass() { return mMainRenderPass; }
	std::shared_ptr<RenderScene> getRenderScene() { return mScene; }
private:
	void buildCommandBuffer(CommandBuffer& commandBuffer);
	void createSemaphores();
private:
	std::shared_ptr<RenderPass> mMainRenderPass;
	std::shared_ptr<SwapChain> mSwapChain;
	std::shared_ptr<DebugCamera> mMainCamera;//hard code:camera should be an component
	std::shared_ptr<RenderScene> mScene;
	VkSemaphore mImageAvailableSemaphore;
	VkSemaphore mRenderFinishedSemaphore;
	uint32_t mCurFrameIndex;
	uint32_t mWidth;
	uint32_t mHeight;
};
#endif