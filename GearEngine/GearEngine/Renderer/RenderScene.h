#ifndef RENDER_SCENE_H
#define RENDER_SCENE_H
#include "RenderObject/StaticModel.h"
#include <map>

class RenderScene
{
public:
	RenderScene()
	{
	}
	~RenderScene()
	{
	}
	
private:
	uint32_t mNextId;
};

#endif
