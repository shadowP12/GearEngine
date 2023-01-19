#include "FileSystem.h"
#include "Utility/Log.h"
#include <stdio.h>

namespace gear {
    int SaveFile(const std::string& path, void* data, uint32_t size) {
        FILE* file = fopen(path.c_str(), "wt");
        if (!file) {
            return -1;
        }

        fwrite(data, size, 1, file);
        fclose(file);
        return 0;
    }

    int LoadFile(const std::string& path, void** data, uint32_t& size) {
        FILE* file = fopen(path.c_str(), "rb");
        if (!file) {
            return -1;
        }

        fseek(file, 0, SEEK_END);
        long file_size = ftell(file);
        if (file_size < 0) {
            fclose(file);
            return -1;
        }
        fseek(file, 0, SEEK_SET);

        void* file_data = malloc(file_size);
        if (!file_data) {
            fclose(file);
            return -1;
        }

        uint32_t read_size = fread(file_data, 1, file_size, file);

        fclose(file);

        if (read_size != file_size) {
            free(file_data);
            return -1;
        }

        *data = file_data;
        size = file_size;
        return 0;
    }
}