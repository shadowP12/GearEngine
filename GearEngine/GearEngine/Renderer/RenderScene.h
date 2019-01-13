#ifndef RENDER_SCENE_H
#define RENDER_SCENE_H
#include "RenderObject/StaticModel.h"
#include <map>

//todo:views,renderables,lights

struct SceneInfo
{
	std::vector<std::shared_ptr<StaticModel>> staticModels;
};

class RenderScene
{
public:
	RenderScene()
	{
	}
	~RenderScene()
	{
	}
	void registerStaticModel(std::shared_ptr<StaticModel> model)
	{
		for (size_t i = 0; i < mSceneInfo.staticModels.size(); i++)
		{
			if (mSceneInfo.staticModels[i] == model)
			{
				return;
			}
		}
		mSceneInfo.staticModels.push_back(model);
	}
	void updateStaticModel()
	{
		for (std::vector<std::shared_ptr<StaticModel>>::iterator iter = mSceneInfo.staticModels.begin(); iter != mSceneInfo.staticModels.end(); )
		{
			if (iter == mSceneInfo.staticModels.end())
				break;
			if ((*iter)->getDestroyed())
			{
				mSceneInfo.staticModels.erase(iter);
			}
		}
	}
	SceneInfo& getSceneInfo() { return mSceneInfo; }
private:
	SceneInfo mSceneInfo;
};

#endif
