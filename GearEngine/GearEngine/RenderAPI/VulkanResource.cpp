#include "VulkanResource.h"

VulkanResource::VulkanResource(VulkanResourceManager* manager)
{
	mManager = manager;
}

VulkanResource::~VulkanResource()
{
}

void VulkanResource::destroy()
{
	mManager->destroy(this);
}

VulkanResourceManager::VulkanResourceManager()
{
}

VulkanResourceManager::~VulkanResourceManager()
{
}

void VulkanResourceManager::destroy(VulkanResource * resource)
{
	delete resource;
}
