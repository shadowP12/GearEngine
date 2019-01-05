#ifndef RESOURCE_H
#define RESOURCE_H
#include <string>
class Resource
{
public:
	Resource() {}
	virtual ~Resource() {}
	void setName(std::string name) { mName = name; }
	std::string getName() { return mName; }
private:
	//todo:uuid
	std::string mName;
};

#endif