#include "System.h"

System::System()
{
	mNextID = 0;
}

System::~System()
{
}

void System::updataComponentState()
{
	for (S_ComponentMap::iterator iter = mComponents.begin(); iter != mComponents.end(); )
	{
		if (iter == mComponents.end())
			break;
		if (iter->second->getDestroyed())
			mComponents.erase(iter);
		//todo:destroy event
	}
}
