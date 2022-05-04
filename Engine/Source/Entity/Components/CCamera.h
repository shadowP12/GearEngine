#pragma once
#include "Component.h"
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

        glm::mat4 GetViewMatrix();

        glm::mat4 GetModelMatrix();

        glm::mat4 GetProjMatrix();

        float GetNear();

        float GetFar();

        void SetMain(bool enable);

        bool GetMain();

        void SetDisplay(bool enable);

        bool GetDisplay();

        void SetAperture(float aperture) { this->aperture = aperture; }

        float GetAperture() { return aperture; }

        void SetShutterSpeed(float shutter_speed) { this->shutter_speed = shutter_speed; }

        float GetShutterSpeed() { return shutter_speed; }

        void SetSensitivity(float sensitivity) { this->sensitivity = sensitivity; }

        float GetSensitivity() { return sensitivity; }

    private:
        float _near = 0.0f;
        float _far = 100.0f;
        glm::mat4 proj_matrix;
        bool is_main = true;
        bool is_display = true;
        float aperture = 16.0f;
        float shutter_speed = 1.0f / 125.0f;
        float sensitivity = 100.0f;
    };
}