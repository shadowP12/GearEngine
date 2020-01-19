#include "FileSystem.h"
#include <direct.h>
namespace FileSystem
{
    std::string getCurPath()
    {
        char buffer[1024];
        if(_getcwd(buffer, 1024)!= nullptr)
        {
            return std::string(buffer);
        }
        return std::string();
    }

    bool readFile(const std::string& dstFileName, std::string& outData)
    {
        std::istream* stream = &std::cin;
        std::ifstream inputFile;

        inputFile.open(dstFileName, std::ios_base::binary);
        stream = &inputFile;
        if (inputFile.fail())
        {
            std::cout << "error: cannot open input file: '" << dstFileName << std::endl;
            return false;
        }
        outData = std::string((std::istreambuf_iterator<char>(*stream)), std::istreambuf_iterator<char>());
        return true;
    }
}
