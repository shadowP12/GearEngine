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
    }

    CCamera::~CCamera() {
        // 注销场景相机
        Scene* scene = gEngine.getScene();
        scene->unregisterCamera(mEntity);

        // RT的生命周期应该由外部控制
        // SAFE_DELETE(mRenderTarget);
        SAFE_DELETE(mCameraUniformBuffer);
    }

    void CCamera::setProjection(ProjectionType type, double left, double right, double bottom, double top, double near, double far) {
        mNear = near;
        mFar = far;
        if (type == ProjectionType::PERSPECTIVE) {
            double fov = glm::radians(45.0);//glm::atan((right - left) / 2.0 / near);
            double aspect = (right - left) / (bottom - top);
            mProjMatrix = glm::perspective(fov, aspect, near, far);
            // 翻转y轴
            mProjMatrix[1][1] *= -1;
        } else {
            mProjMatrix = glm::ortho(left, right, top, bottom, near, far);
        }
    }

    void CCamera::setRenderTarget(RenderTarget* target) {
        mRenderTarget = target;
    }

    void CCamera::setLayer(RenderLayer layer) {
        mLayer = layer;
    }

    void CCamera::updateCameraBuffer() {
        glm::mat4 modelMatrix = mEntity->getComponent<CTransform>()->getWorldTransform();
        FrameUniforms ub;
        ub.viewMatrix = glm::inverse(modelMatrix);
        ub.projMatrix = mProjMatrix;
        glm::vec4 r = mProjMatrix * glm::vec4(100, 100, 0, 1);
        mCameraUniformBuffer->update(&ub, 0, sizeof(FrameUniforms));
    }

    glm::mat4 CCamera::getViewMatrix() {
        glm::mat4 modelMatrix = mEntity->getComponent<CTransform>()->getWorldTransform();
        return glm::inverse(modelMatrix);
    }

    glm::mat4 CCamera::getModelMatrix() {
        return mEntity->getComponent<CTransform>()->getWorldTransform();
    }

    glm::mat4 CCamera::getProjMatrix() {
        return mProjMatrix;
    }

    float CCamera::getNear() {
        return mNear;
    }

    float CCamera::getFar() {
        return mFar;
    }
}