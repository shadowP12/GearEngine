#include "CCamera.h"
#include "CTransform.h"
#include "GearEngine.h"
#include "Entity/Entity.h"
#include "Resource//RenderTarget.h"
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
            _proj_matrix = glm::perspective(fov, aspect, near, far);
            // 翻转y轴
            _proj_matrix[1][1] *= -1;
        } else {
            _proj_matrix = glm::ortho(left, right, top, bottom, near, far);
        }
    }

    void CCamera::SetRenderTarget(RenderTarget* target) {
        _render_target = target;
    }

    RenderTarget* CCamera::GetRenderTarget() {
        return _render_target;
    }

    glm::mat4 CCamera::GetViewMatrix() {
        glm::mat4 modelMatrix = _entity->GetComponent<CTransform>()->GetWorldTransform();
        return glm::inverse(modelMatrix);
    }

    glm::mat4 CCamera::GetModelMatrix() {
        return _entity->GetComponent<CTransform>()->GetWorldTransform();
    }

    glm::mat4 CCamera::GetProjMatrix() {
        return _proj_matrix;
    }

    float CCamera::GetNear() {
        return _near;
    }

    float CCamera::GetFar() {
        return _far;
    }

    void CCamera::SetMain(bool enable) {
        _is_main = enable;
    }

    bool CCamera::GetMain() { return _is_main; }

    void CCamera::SetDisplay(bool enable) {
        _is_display = enable;
    }

    bool CCamera::GetDisplay() { return _is_display; }
}