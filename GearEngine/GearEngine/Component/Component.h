#ifndef COMPONENT_H
#define COMPONENT_H
#include <string>

class Component : public std::enable_shared_from_this<Component>
{
public:
	Component();
	~Component();
	void destroy();
	bool getEnabled();
	void setEnabled(bool enable);
	bool getDestroyed();
	std::string getType();
private:
	std::string mType;
	bool mEnabled;
	bool mDestroyed;
};

#endif