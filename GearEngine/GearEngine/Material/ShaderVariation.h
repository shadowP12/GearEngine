#ifndef SHADER_VARIATION_H
#define SHADER_VARIATION_H
#include <unordered_map>
#include <string>
class  ShaderDefines
{
public:

	void set(const std::string& name, const std::string& value)
	{
		mDefines[name] = value;
	}

	std::unordered_map<std::string, std::string> get() const { return mDefines; }

	void clear() { mDefines.clear(); }

protected:
	std::unordered_map<std::string, std::string> mDefines;
};

class ShaderVariation
{
public:
	ShaderVariation();
	~ShaderVariation();
private:
	
};

#endif