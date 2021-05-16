#include "CCamera.h"
#include "GearEngine.h"
#include "Scene/Scene.h"
#include "Scene/Entity.h"
#include "Renderer/Renderer.h"
#include "Renderer/RenderTarget.h"
namespace gear {
    CCamera::CCamera(Entity* entity)
            :Component(entity) {
        Renderer* renderer = gEngine.getRenderer();
        mRenderTarget = renderer->getRenderTarget();
    }

    CCamera::~CCamera() {
        SAFE_DELETE(mRenderTarget);
    }

    void CCamera::setRenderTarget(RenderTarget* target) {
        mRenderTarget = target;
    }
}