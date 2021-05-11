#include "Scene/Components/CRenderable.h"
#include "Scene/Scene.h"
#include "Scene/Entity.h"
#include "Renderer/Renderer.h"

namespace gear {
    CRenderable::CRenderable(Entity* entity)
            :Component(entity) {
    }

    CRenderable::~CRenderable() {
    }

}