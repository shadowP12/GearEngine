#include "CTransform.h"
#include "Scene/Entity.h"

namespace gear {
    CTransform::CTransform(Entity* entity)
            :Component(entity) {
    }

    CTransform::~CTransform() {
    }

}