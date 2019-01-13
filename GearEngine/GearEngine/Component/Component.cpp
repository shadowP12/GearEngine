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
	//may be more ...
	destroyImp();
}

bool Component::getEnabled()
{
	return mEnabled;
}

void Component::setEnabled(bool enable)
{
	if (mEnabled != enable)
		mEnabled = enable;
	//may be more
	setEnabledImp(enable);
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

std::string Component::getType()
{
	return mType;
}
