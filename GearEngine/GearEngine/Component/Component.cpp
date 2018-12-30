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

std::string Component::getType()
{
	return mType;
}
