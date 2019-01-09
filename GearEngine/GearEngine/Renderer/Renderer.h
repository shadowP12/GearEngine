#ifndef RENDERER_H
#define RENDERER_H
#include "../Utility/Module.h"
#include "../RenderAPI/Framebuffer.h"
#include "../RenderAPI/RenderPass.h"

class Renderer : public Module<Renderer>
{
public:
	Renderer();
	~Renderer();
	std::shared_ptr<RenderPass> getRenderPass() { return mMainRenderPass; }
private:
	std::shared_ptr<RenderPass> mMainRenderPass;
};
#endif