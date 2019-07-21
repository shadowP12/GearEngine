#ifndef HASH_H
#define HASH_H
#include <functional>
/**
  std::hash属于c++14的标准,后续可能为了兼容c++11修改hash函数
*/

template <class T>
size_t Hash(const T& v)
{
	std::hash<T> hasher;
	return hasher(v);
}

template <class T>
void Hash(std::size_t& seed, const T& v)
{
	std::hash<T> hasher;
	seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}
#endif
