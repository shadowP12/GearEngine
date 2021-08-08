#include "FileSystem.h"
#include "Utility/Log.h"

#include <direct.h>
#include <iostream>
#include <fstream>
#include <sstream>

namespace gear {
    std::string getCurrentPath() {
        char buffer[1024];
        getcwd(buffer, 1024);
        std::string path(buffer);
        return path;
    }

    std::string readFileData(const std::string& path) {
        std::istream* stream = &std::cin;
        std::ifstream file;

        file.open(path, std::ios_base::binary);
        stream = &file;
        if (file.fail()) {
            LOGW("cannot open input file %s \n", path.c_str());
            return std::string("");
        }
        return std::string((std::istreambuf_iterator<char>(*stream)), std::istreambuf_iterator<char>());
    }
}