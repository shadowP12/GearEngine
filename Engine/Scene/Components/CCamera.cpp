#include "CCamera.h"
#include "Scene/Scene.h"
#include "Scene/Entity.h"
#include "Renderer/Renderer.h"

namespace gear {
    CCamera::CCamera(Entity* entity)
            :Component(entity) {
    }

    CCamera::~CCamera() {
    }
}