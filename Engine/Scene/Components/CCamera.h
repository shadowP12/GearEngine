#pragma once
#include "Scene/Components/Component.h"
#include "Math/Math.h"

namespace gear {
    class RenderTarget;
    class Entity;
    // TODO: 设置相机UniformBuffer
    class CCamera : public Component {
    public:
        CCamera(Entity* entity);

        virtual ~CCamera();

        static ComponentType getClassType() { return ComponentType::Camera; }

        ComponentType getType() override { return ComponentType::Camera; }

        void setRenderTarget(RenderTarget* target);
    private:
        RenderTarget* mRenderTarget = nullptr;
    };
}