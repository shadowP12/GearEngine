#include "CCamera.h"
#include "CTransform.h"
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
        // TODO: 硬编码投影矩阵
        mProjMatrix = glm::perspective(glm::radians(45.0), 800.0 / 600.0, 0.1, 100.0);
        mProjMatrix[1][1] *= -1;
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
        glm::mat4 modelMatrix = mEntity->getComponent<CTransform>()->getWorldTransform();
        FrameUniforms ub;
        ub.viewMatrix = glm::inverse(modelMatrix);
        ub.projMatrix = mProjMatrix;
        mCameraUniformBuffer->update(&ub, 0, sizeof(FrameUniforms));
    }
}