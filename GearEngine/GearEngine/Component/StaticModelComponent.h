#ifndef STATIC_MODEL_COMPONENT_H
#define STATIC_MODEL_COMPONENT_H
#include "Component.h"
#include "../Renderer/RenderObject/StaticModel.h"
#include "../Renderer/Renderer.h"

class StaticModelComponent : public Component
{
public:
	StaticModelComponent(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> mat, std::shared_ptr<Node> node)
	{
		mType = "StaticModelComponent";
		mStaticModel = std::shared_ptr<StaticModel>(new StaticModel());
		mStaticModel->setMesh(mesh);
		mStaticModel->setMaterial(mat);
		mStaticModel->setNode(node);
		Renderer::instance().getRenderScene()->registerStaticModel(mStaticModel);
	}
	virtual ~StaticModelComponent()
	{
	}
	virtual void destroyImp()
	{
		mStaticModel->destroy();
	}
	virtual void setEnabledImp(bool enable)
	{
		mStaticModel->setEnabled(enable);
	}

private:
	std::shared_ptr<StaticModel> mStaticModel;
};
#endif
