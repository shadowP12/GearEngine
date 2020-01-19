#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
namespace FileSystem
{
    std::string getCurPath();
    bool readFile(const std::string& dstFileName, std::string& outData);
}
#endif