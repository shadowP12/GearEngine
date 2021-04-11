#pragma once

#include "Core/GearDefine.h"
#include <vector>
#include <map>
#include <string>
#include <memory>
namespace gear {
    enum class ResourceType {
        NONE = 0,
        MESH = 1,
        TEXTURE = 2,
        MATERIAL = 3,
        MAX_RESOURCE_TYPES
    };

    class Resource : public std::enable_shared_from_this<Resource> {
    public:
        Resource() {}
        virtual ~Resource() {}
        void setType(ResourceType type) { mType = type; }
        ResourceType getType() { return mType; }
        void setId(const std::string& id) { mId = id; }
        const std::string& getId() { return mId; }
        void setUUID(const std::string& uuid) { mUUID = uuid; }
        const std::string& getUUID() { return mUUID; }
    protected:
        friend class ResourceManager;
        ResourceType mType = ResourceType::NONE;
        std::string mId;
        std::string mUUID;
    };
}