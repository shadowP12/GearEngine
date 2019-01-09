#include "Renderer.h"

Renderer::Renderer()
{
	std::vector<VkFormat> colors;
	colors.push_back(VK_FORMAT_D32_SFLOAT);
	mMainRenderPass = std::shared_ptr<RenderPass>(new RenderPass(colors, true, true));
}

Renderer::~Renderer()
{
}