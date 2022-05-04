#include "CCamera.h"
#include "CTransform.h"
#include "GearEngine.h"
#include "Entity/Entity.h"

namespace gear {
    CCamera::CCamera(Entity* entity)
            :Component(entity) {
    }

    CCamera::~CCamera() {
    }

    void CCamera::SetProjection(ProjectionType type, double left, double right, double bottom, double top, double near, double far) {
        _near = near;
        _far = far;
        if (type == ProjectionType::PERSPECTIVE) {
            // 后续暴露fov参数
            double fov = glm::radians(45.0);
            double aspect = (right - left) / (bottom - top);
            proj_matrix = glm::perspective(fov, aspect, near, far);
            // 翻转y轴
            proj_matrix[1][1] *= -1;
        } else {
            proj_matrix = glm::ortho(left, right, top, bottom, near, far);
        }
    }

    glm::mat4 CCamera::GetViewMatrix() {
        glm::mat4 modelMatrix = _entity->GetComponent<CTransform>()->GetWorldTransform();
        return glm::inverse(modelMatrix);
    }

    glm::mat4 CCamera::GetModelMatrix() {
        return _entity->GetComponent<CTransform>()->GetWorldTransform();
    }

    glm::mat4 CCamera::GetProjMatrix() {
        return proj_matrix;
    }

    float CCamera::GetNear() {
        return _near;
    }

    float CCamera::GetFar() {
        return _far;
    }

    void CCamera::SetMain(bool enable) {
        is_main = enable;
    }

    bool CCamera::GetMain() { return is_main; }

    void CCamera::SetDisplay(bool enable) {
        is_display = enable;
    }

    bool CCamera::GetDisplay() { return is_display; }
}