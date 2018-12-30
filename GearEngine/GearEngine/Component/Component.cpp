#include "Component.h"

Component::Component()
{
	mEnabled = true;
	mDestroyed = false;
}

Component::~Component()
{
}

void Component::destroy()
{
	if (!mDestroyed)
		return;
	mDestroyed = true;
	
}

bool Component::getEnabled()
{
	return mEnabled;
}

void Component::setEnabled(bool enable)
{
	if (mEnabled != enable)
		mEnabled = enable;
}

bool Component::getDestroyed()
{
	return mDestroyed;
}

int Component::getID()
{
	return mID;
}

void Component::setID(int id)
{
	mID = id;
}

std::shared_ptr<Node> Component::getNode()
{
	return mNode;
}

void Component::setNode(std::shared_ptr<Node> node)
{
	mNode = node;
}

std::string Component::getType()
{
	return mType;
}
