#include "CCamera.h"
#include "GearEngine.h"
#include "Scene/Scene.h"
#include "Scene/Entity.h"
#include "Renderer/Renderer.h"
#include "Renderer/RenderTarget.h"
#include "Renderer/RenderScene.h"
#include "Resource/GpuBuffer.h"
namespace gear {
    CCamera::CCamera(Entity* entity)
            :Component(entity) {
        Renderer* renderer = gEngine.getRenderer();
        mRenderTarget = renderer->getRenderTarget();
        mCameraUniformBuffer = new UniformBuffer(sizeof(FrameUniforms));
    }

    CCamera::~CCamera() {
        // RT的生命周期应该由外部控制
        // SAFE_DELETE(mRenderTarget);
        SAFE_DELETE(mCameraUniformBuffer);
    }

    void CCamera::setRenderTarget(RenderTarget* target) {
        mRenderTarget = target;
    }

    void CCamera::updateCameraBuffer() {
        // TODO
        FrameUniforms ub;
        ub.viewMatrix = glm::mat4(1.0);
        ub.projMatrix = glm::mat4(1.0);
        mCameraUniformBuffer->update(&ub, 0, sizeof(FrameUniforms));
    }
}