#include "DataStream.h"

uint32_t AlignTo(uint32_t value, uint32_t alignment) {
    return ((value + alignment - 1) / alignment) * alignment;
}

DataStream::DataStream(uint32_t capacity) {
    Realloc(capacity);
}

DataStream::~DataStream() {
    free(data);
}

uint8_t* DataStream::Read(uint32_t pos) {
    return data + pos;
}

void DataStream::Write(uint8_t* in_data, uint32_t size) {
    uint32_t pos = cursor + size;
    ReallocIfNeeded(pos);
    memcpy(data, in_data, size);
    cursor += size;
}

void DataStream::ReallocIfNeeded(uint32_t size) {
    if (size <= max_size) {
        return;
    }
    Realloc(size);
}

void DataStream::Realloc(uint32_t size) {
    size = AlignTo(size, 8);
    uint8_t* old_data = data;
    uint8_t* new_data = (uint8_t*)malloc(size);
    if (old_data) {
        memcpy(new_data, old_data, max_size);
        free(old_data);
    }
    data = new_data;
    max_size = size;
}