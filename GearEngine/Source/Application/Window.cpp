#include "Window.h"
#include "RHI/RHI.h"
#include "RHI/RHISynchronization.h"

Window::Window(int width, int height)
	:mWidth(width),mHeight(height)
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	mWindow = glfwCreateWindow(mWidth, mHeight, "Gear", nullptr, nullptr);
	glfwSetWindowUserPointer(mWindow, this);
	glfwSetFramebufferSizeCallback(mWindow, resizeCallback);
	glfwSetKeyCallback(mWindow, keyCallback);
	glfwSetMouseButtonCallback(mWindow, mouseButtonCallback);
	glfwSetCursorPosCallback(mWindow, cursorPosCallback);
	glfwSetScrollCallback(mWindow, mouseScrollCallback);

	//初始化交换链
	auto res = glfwCreateWindowSurface(RHI::instance().getInstance(), mWindow, nullptr, &mSurface);
	if (res != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create window surface!");
	}

	//缺少检查会崩溃
	VkBool32 supportsPresent;
	uint32_t graphicsFamily = RHI::instance().getDevice()->getGraphicsQueue()->getFamilyIndex();
	vkGetPhysicalDeviceSurfaceSupportKHR(RHI::instance().getDevice()->getPhyDevice(), graphicsFamily, mSurface, &supportsPresent);

	if (!supportsPresent)
	{
		throw std::runtime_error("Cannot find a graphics queue that also supports present operations.");
	}

	mSwapChain = new RHISwapChain(RHI::instance().getDevice(), mSurface, width, height);

	// 改变SwapChain中framebuffer的image layout
	std::vector<VkImageMemoryBarrier> barriers;
    for (int i = 0; i < mSwapChain->mSwapChainImages.size(); ++i)
    {
        VkImageMemoryBarrier imageMemoryBarrier = {};
        imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.image = mSwapChain->mSwapChainImages[i];
        imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
        imageMemoryBarrier.subresourceRange.levelCount = 1;
        imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
        imageMemoryBarrier.subresourceRange.layerCount = 1;
        imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageMemoryBarrier.srcAccessMask = 0;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                                           VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
        barriers.push_back(imageMemoryBarrier);
    }

    RHICommandBufferPool* pool = RHI::instance().getDevice()->getHelperCmdBufferPool();
    RHICommandBuffer* cmdBuf = pool->getActiveCmdBuffer();
    cmdBuf->begin();
    vkCmdPipelineBarrier(cmdBuf->getHandle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, barriers.size(), barriers.data());
    cmdBuf->end();
    cmdBuf->submit();

	// 创建显示信号量
    mImageAvailableSemaphore = RHI::instance().getDevice()->createSemaphore();
    mRenderFinishedSemaphore = RHI::instance().getDevice()->createSemaphore();
}

Window::~Window()
{
    SAFE_DELETE(mImageAvailableSemaphore);
    SAFE_DELETE(mRenderFinishedSemaphore);
	glfwDestroyWindow(mWindow);
	if (mSwapChain)
		delete mSwapChain;
}

void Window::beginFrame()
{
	VkResult result = vkAcquireNextImageKHR(RHI::instance().getDevice()->getDevice(), mSwapChain->getHandle(), std::numeric_limits<uint64_t>::max(), mImageAvailableSemaphore->getHandle(), VK_NULL_HANDLE, &mFrameIndex);
	if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) 
	{
		throw std::runtime_error("failed to acquire swap chain image!");
	}

    RHICommandBufferPool* pool = RHI::instance().getDevice()->getHelperCmdBufferPool();
    RHICommandBuffer* cmdBuf = pool->getActiveCmdBuffer();
    cmdBuf->addWaitSemaphore(mImageAvailableSemaphore);
    cmdBuf->begin();
    cmdBuf->end();
    cmdBuf->submit();
}

// note:必须保证渲染流程不为空
void Window::endFrame()
{
    RHICommandBufferPool* pool = RHI::instance().getDevice()->getHelperCmdBufferPool();
    RHICommandBuffer* cmdBuf = pool->getActiveCmdBuffer();
    cmdBuf->addSignalSemaphore(mRenderFinishedSemaphore);
    cmdBuf->begin();
    cmdBuf->end();
    cmdBuf->submit();

    VkSemaphore waitSemaphores[] = { mRenderFinishedSemaphore->getHandle() };
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = waitSemaphores;
	VkSwapchainKHR swapChains[] = { mSwapChain->getHandle() };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &mFrameIndex;
	
	VkResult result = vkQueuePresentKHR(RHI::instance().getDevice()->getGraphicsQueue()->getHandle(), &presentInfo);

	if (result != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to present swap chain image!");
	}

	// 万金油做法
	vkQueueWaitIdle(RHI::instance().getDevice()->getGraphicsQueue()->getHandle());
}

RHIFramebuffer* Window::getFramebuffer()
{
	return mSwapChain->getFramebuffer(mFrameIndex);
}

GLFWwindow * Window::getWindowPtr()
{
	return mWindow;
}

int Window::getWidth()
{
	return mWidth;
}

int Window::getHeight()
{
	return mHeight;
}

void Window::reset(uint32_t width, uint32_t height)
{
	mWidth = width;
	mHeight = height;
	if (mSwapChain)
		delete mSwapChain;
	mSwapChain = new RHISwapChain(RHI::instance().getDevice(), mSurface, width, height);
}

void resizeCallback(GLFWwindow * window, int width, int height)
{
	Window * win = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
	win->reset(width, height);
	
}

void cursorPosCallback(GLFWwindow * window, double xPos, double yPos)
{
	Input::instance().setMousePosition(glm::vec2(xPos, yPos));
}

void keyCallback(GLFWwindow * window, int key, int scancode, int action, int mods)
{
	int curKey = 0;
	if (key >= 48 && key <= 57) 
	{
		curKey = key - 48;
	}
	else if (key >= 65 && key <= 90) 
	{
		curKey = key - 55;
	}
	else 
	{
		return;
	}

	switch (action) 
	{
	case GLFW_PRESS:
		Input::instance().setKeyDown(curKey);
		Input::instance().setKey(curKey, true);
		break;
	case GLFW_RELEASE:
		Input::instance().setKeyUp(curKey);
		Input::instance().setKey(curKey, false);
		break;
	default:
		break;
	}
}

void mouseButtonCallback(GLFWwindow * window, int button, int action, int mods)
{
	switch (action) 
	{
	case GLFW_PRESS:
		Input::instance().setMouseButtonDown(button);
		Input::instance().setMouseButton(button, true);
		break;
	case GLFW_RELEASE:
		Input::instance().setMouseButtonUp(button);
		Input::instance().setMouseButton(button, false);
		break;
	default:
		break;
	}
}

void mouseScrollCallback(GLFWwindow * window, double xOffset, double yOffset)
{
	Input::instance().setMouseScrollWheel((float)yOffset);
}


