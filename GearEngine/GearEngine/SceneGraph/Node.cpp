#include "Node.h"

Node::Node()
{
}

Node::~Node()
{
}

void Node::setParent(std::shared_ptr<Node> newParent)
{
	std::shared_ptr<Node> oldParent = mParent;
	if (oldParent == newParent)
		return;

	std::shared_ptr<Node> curNode = newParent;
	while (curNode)
	{
		if (curNode == shared_from_this())
			return;
		curNode = curNode->getParent();
	}

	if (oldParent)
	{
		for (std::vector<std::shared_ptr<Node>>::iterator iter = oldParent->mChildren.begin(); iter != oldParent->mChildren.end(); )
		{
			if (iter == oldParent->mChildren.end())
				break;
			if (*iter == shared_from_this())
				oldParent->mChildren.erase(iter);
		}
	}

	mParent = newParent;
	if (newParent)
	{
		newParent->mChildren.push_back(shared_from_this());
	}
}

void Node::appendChildren(std::shared_ptr<Node> newChildren) 
{
	if (!newChildren)
		return;

	std::shared_ptr<Node> curNode = shared_from_this();
	while (curNode)
	{
		if (curNode == newChildren)
			return;
		curNode = curNode->getParent();
	}

	std::shared_ptr<Node> oldParent = newChildren->mParent;
	if (oldParent)
	{
		for (std::vector<std::shared_ptr<Node>>::iterator iter = oldParent->mChildren.begin(); iter != oldParent->mChildren.end(); )
		{
			if (iter == oldParent->mChildren.end())
				break;
			if (*iter == newChildren)
				oldParent->mChildren.erase(iter);
		}
	}
	
	newChildren->mParent = shared_from_this();
	mChildren.push_back(newChildren);
}

void Node::removeChildren(std::shared_ptr<Node> Children)
{
	for (std::vector<std::shared_ptr<Node>>::iterator iter = mChildren.begin(); iter != mChildren.end(); )
	{
		if (iter == mChildren.end())
			break;
		if (*iter == Children)
		{
			mChildren.erase(iter);
			Children->mParent = NULL;
		}
	}
}

glm::mat4 Node::getLocalMatrix()
{
	//RTS
	glm::mat4 R, T, S;
	R = glm::toMat4(mLRot);
	T = glm::translate(T,mLPos);
	S = glm::scale(S,mLScale);
	return R*T*S;
}

glm::mat4 Node::getWorldMatrix()
{
	glm::mat4 out = getLocalMatrix();
	std::shared_ptr<Node> cur = mParent;
	while (cur)
	{
		out = cur->getLocalMatrix() * out;
	}
	return out;
}

