#pragma once
#include "Scene/Components/Component.h"
#include "Math/Math.h"

namespace gear {
    class RenderTarget;
    class UniformBuffer;
    class Entity;
    class CCamera : public Component {
    public:
        CCamera(Entity* entity);

        virtual ~CCamera();

        static ComponentType getClassType() { return ComponentType::Camera; }

        ComponentType getType() override { return ComponentType::Camera; }

        void setRenderTarget(RenderTarget* target);

        void setLayer(uint32_t layer);

    private:
        void updateCameraBuffer();

    private:
        friend class RenderScene;
        RenderTarget* mRenderTarget = nullptr;
        UniformBuffer* mCameraUniformBuffer = nullptr;
        // 因为灯光需要根据每个相机进行排序，所以灯光的UniformBuffer放在此处管理
        UniformBuffer* mLightUniformBuffer = nullptr;
        glm::mat4 mProjMatrix;
        uint32_t mLayer = 0;
    };
}