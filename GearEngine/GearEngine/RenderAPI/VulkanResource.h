#ifndef VULKAN_RESOURCE_H
#define VULKAN_RESOURCE_H
#include "VulkanPrerequisites.h"
class VulkanResourceManager;

class VulkanResource
{
public:
	VulkanResource(VulkanResourceManager* manager);
	virtual ~VulkanResource();
	void destroy();
	//todo
private:
	VulkanResourceManager* mManager;
};

class VulkanResourceManager
{
public:
	VulkanResourceManager();
	~VulkanResourceManager();

	template<class Type, class... Args>
	Type* create(Args &&...args)
	{
		Type* resource = new Type(this, std::forward<Args>(args)...);
		return resource;
	}

private:
	friend VulkanResource;
	void destroy(VulkanResource* resource);
};

#endif