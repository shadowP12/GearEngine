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

void Entity::destroyed()
{
	if (!mActive)
		return;
	mActive = false;
	notifyChildren(false);
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
