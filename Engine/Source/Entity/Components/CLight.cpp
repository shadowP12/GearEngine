#include "CLight.h"
#include "Entity/Entity.h"

namespace gear {
    CLight::CLight(Entity* entity)
            :Component(entity) {
    }

    CLight::~CLight() {
    }

}