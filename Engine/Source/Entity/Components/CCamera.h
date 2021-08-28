#pragma once
#include "Component.h"
#include "Resource/RenderTarget.h"
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

        static ComponentType GetClassType() { return ComponentType::Camera; }

        ComponentType GetType() override { return ComponentType::Camera; }

        void SetProjection(ProjectionType type, double left, double right, double bottom, double top, double near, double far);

        void SetRenderTarget(RenderTarget* target);

        RenderTarget* GetRenderTarget();

        glm::mat4 GetViewMatrix();

        glm::mat4 GetModelMatrix();

        glm::mat4 GetProjMatrix();

        float GetNear();

        float GetFar();

        void SetMain(bool enable);

        bool GetMain();

        void SetDisplay(bool enable);

        bool GetDisplay();

    private:
        float _near = 0.0f;
        float _far = 100.0f;
        glm::mat4 _proj_matrix;
        bool _is_main = true;
        bool _is_display = true;
        RenderTarget* _render_target = nullptr;
    };
}