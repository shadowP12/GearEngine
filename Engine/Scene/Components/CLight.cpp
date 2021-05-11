#include "Scene/Components/CLight.h"
#include "Scene/Scene.h"
#include "Scene/Entity.h"
#include "Renderer/Renderer.h"

namespace gear {
    CLight::CLight(Entity* entity)
            :Component(entity) {
    }

    CLight::~CLight() {
    }

}