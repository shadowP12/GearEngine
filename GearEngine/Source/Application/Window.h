#ifndef WINDOW_H
#define WINDOW_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include "Input/Input.h"
#include "RHI/RHISwapChain.h"

static void resizeCallback(GLFWwindow *window, int width, int height);
static void cursorPosCallback(GLFWwindow * window, double xPos, double yPos);
static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
static void mouseButtonCallback(GLFWwindow * window, int button, int action, int mods);
static void mouseScrollCallback(GLFWwindow * window, double xOffset, double yOffset);

class RHICommandBuffer;

class Window
{
public:
	Window(int width = 800, int height = 600);
	~Window();
	GLFWwindow* getWindowPtr();
	int getWidth();
	int getHeight();
	void reset(uint32_t width, uint32_t height);
	std::vector<const char*> getRequiredExtensions(bool enableValidationLayers);
	void beginFrame();
	void endFrame();
private:
	int mWidth;
	int mHeight;
	GLFWwindow* mWindow;
	VkSurfaceKHR mSurface;
	RHISwapChain* mSwapChain;
	RHICommandBuffer* mPresentCmdBuffer;
	// 这里使用比较讨巧的方法
	// 将gpu数据绘制到屏幕时独自发送一条命令
	VkSemaphore mImageAvailableSemaphore;
	VkSemaphore mRenderFinishedSemaphore;
	uint32_t mFrameIndex;
};

#endif