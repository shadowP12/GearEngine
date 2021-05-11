#include "Entity.h"

namespace gear {
    Entity::Entity() {
    }

    Entity::~Entity() {
        for (int i = 0; i < mComponents.size(); ++i) {
            delete mComponents[i];
            mComponents[i] = nullptr;
        }
        mComponents.clear();
    }
}