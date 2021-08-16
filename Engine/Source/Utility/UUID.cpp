#include "Utility/UUID.h"

#include <windows.h>
#include <rpc.h>

std::string GenerateUUID() {
    UUID uuid{};
    RPC_CSTR str = nullptr;

    UuidCreate(&uuid);
    UuidToStringA(&uuid, &str);

    std::string result(reinterpret_cast<const char*>(str));
    return result;
}