#ifndef RESOURCE_H
#define RESOURCE_H
#include <string>
class ResourceManager;
class Resource
{
public:
	Resource() {}
	virtual ~Resource() {}
	void setName(std::string name) { mName = name; }
	std::string getName() { return mName; }
private:
	//todo:uuid
	friend ResourceManager;
	std::string mName;
};

#endif