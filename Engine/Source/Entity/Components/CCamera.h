#pragma once
#include "Scene/Components/Component.h"
#include "Renderer/RenderScene.h"
#include "Math/Math.h"

namespace gear {
    class RenderTarget;
    class UniformBuffer;
    class Entity;

    enum class ProjectionType {
        PERSPECTIVE,
        ORTHO
    };

    class CCamera : public Component {
    public:
        CCamera(Entity* entity);

        virtual ~CCamera();

        static ComponentType getClassType() { return ComponentType::Camera; }

        ComponentType getType() override { return ComponentType::Camera; }

        void setProjection(ProjectionType type, double left, double right, double bottom, double top, double near, double far);

        void setRenderTarget(RenderTarget* target);

        void setLayer(RenderLayer layer);

    private:
        void updateCameraBuffer();

        glm::mat4 getViewMatrix();

        glm::mat4 getModelMatrix();

        glm::mat4 getProjMatrix();

        float getNear();

        float getFar();
    private:
        friend class RenderScene;
        RenderTarget* mRenderTarget = nullptr;
        UniformBuffer* mCameraUniformBuffer = nullptr;
        // 因为灯光需要根据每个相机进行排序，所以灯光的UniformBuffer放在此处管理
        UniformBuffer* mLightUniformBuffer = nullptr;
        float mNear = 0.0f;
        float mFar = 100.0f;
        glm::mat4 mProjMatrix;
        RenderLayer mLayer = RenderLayer::CORE;
    };
}