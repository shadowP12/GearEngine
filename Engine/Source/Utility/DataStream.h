#pragma once

#include <memory>

class DataStream {
public:
    DataStream() = default;

    DataStream(uint32_t capacity);

    ~DataStream();

    uint8_t* Read(uint32_t pos);

    void Write(uint8_t* in_data, uint32_t size);

private:
    void ReallocIfNeeded(uint32_t size);

    void Realloc(uint32_t size);

private:
    uint32_t cursor = 0;
    uint32_t max_size = 0;
    uint8_t* data = nullptr;
};