#ifndef NODE_H
#define NODE_H
#include <memory>
#include <vector>
#include "../Math/GMath.h"

class Node : public std::enable_shared_from_this<Node>
{
public:
	Node() 
	{
		mParent = nullptr;
		mLPos = glm::vec3(0,0,0);
		mLScale = glm::vec3(1,1,1);
		mLRot = glm::quat(1,0,0,0);
	}
	virtual ~Node() 
	{
	}
	std::shared_ptr<Node> getParent() 
	{
		return mParent;
	}
	std::vector<std::shared_ptr<Node>> getChildren()
	{
		return mChildren;
	}
	void setParent(std::shared_ptr<Node> newParent);
	void appendChildren(std::shared_ptr<Node> newChildren);
	void removeChildren(std::shared_ptr<Node> Children);
	glm::mat4 getLocalMatrix();
	glm::mat4 getWorldMatrix();
public:
	std::shared_ptr<Node> mParent;
	std::vector<std::shared_ptr<Node>> mChildren;
	glm::vec3 mLPos;
	glm::vec3 mLScale;
	glm::quat mLRot;

};

#endif
