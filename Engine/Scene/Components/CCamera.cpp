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
        Scene* scene = gEngine.getScene();
        Renderer* renderer = gEngine.getRenderer();
        mRenderTarget = renderer->getRenderTarget();
        mCameraUniformBuffer = new UniformBuffer(sizeof(FrameUniforms));

        // 注册场景相机
        scene->registerCamera(mEntity);

        // TODO: 硬编码投影矩阵
        mProjMatrix = glm::perspective(glm::radians(45.0), 800.0 / 600.0, 0.1, 100.0);
        mProjMatrix[1][1] *= -1;
    }

    CCamera::~CCamera() {
        // 注销场景相机
        Scene* scene = gEngine.getScene();
        scene->unregisterCamera(mEntity);

        // RT的生命周期应该由外部控制
        // SAFE_DELETE(mRenderTarget);
        SAFE_DELETE(mCameraUniformBuffer);
    }

    void CCamera::setRenderTarget(RenderTarget* target) {
        mRenderTarget = target;
    }

    void CCamera::setLayer(uint32_t layer) {
        mLayer = layer;
    }

    void CCamera::updateCameraBuffer() {
        glm::mat4 modelMatrix = mEntity->getComponent<CTransform>()->getWorldTransform();
        FrameUniforms ub;
        ub.viewMatrix = glm::inverse(modelMatrix);
        ub.projMatrix = mProjMatrix;
        mCameraUniformBuffer->update(&ub, 0, sizeof(FrameUniforms));
    }
}