#include "Entity.h"

Entity::Entity()
{
	mActive = true;
	mDestroyed = false;
	mAncestorActive = false;
}

Entity::~Entity()
{
}

void Entity::activate()
{
	if (mActive)
		return;
	mActive = true;
	notifyChildren(true);
}

void Entity::deactivate()
{
	if (!mActive)
		return;
	mActive = false;
	notifyChildren(false);
}

void Entity::destroy()
{
	if (!mDestroyed)
		return;

	for (std::vector<std::shared_ptr<Node>>::iterator iter = mChildren.begin(); iter != mChildren.end(); )
	{
		if (iter == mChildren.end())
			break;
		std::shared_ptr<Entity> child = std::dynamic_pointer_cast<Entity>(*iter);
		mChildren.erase(iter);
		child->mParent = NULL;
		child->destroy();
	}

	for (ComponentMap::iterator iter = mComponents.begin(); iter != mComponents.end(); )
	{
		if (iter == mComponents.end())
			break;
		mComponents.erase(iter);
		iter->second->destroy();
	}

	mDestroyed = true;
}

bool Entity::getDestroyed()
{
	return mDestroyed;
}

bool Entity::getActive()
{
	return mActive;
}

void Entity::notifyChildren(bool active)
{
	for (int i = 0; i < mChildren.size(); ++i) 
	{
		std::shared_ptr<Entity> child = std::dynamic_pointer_cast<Entity>(mChildren[i]);
		child->mAncestorActive = active;
		if (child->mActive) 
		{
			child->mActive = active;
			child->notifyChildren(active);
		}
	}
}

bool Entity::getActiveInHierarchy()
{
	return mActive && mAncestorActive;
}

std::string Entity::getName()
{
	return mName;
}

void Entity::setName(std::string name)
{
	mName = name;
}

std::shared_ptr<Component> Entity::getComponent(std::string type)
{
	std::shared_ptr<Component> com;
	ComponentMap::iterator nameIt = mComponents.find(type);
	if (nameIt != mComponents.end()) {
		com = nameIt->second;
	}
	return com;
}

void Entity::addComponent(std::shared_ptr<Component> newComponent)
{
	std::pair<ComponentMap::iterator, bool> result;
	result = mComponents.insert(ComponentMap::value_type(newComponent->getType(), newComponent));
	//todo:more Imformation
	if (result.second) 
	{
	}
}

void Entity::removeComponent(std::string type)
{
	ComponentMap::iterator nameIt = mComponents.find(type);
	if (nameIt != mComponents.end()) 
	{
		mComponents.erase(nameIt);
	}
}
