#include "CameraController.h"
#include <GearEngine.h>
#include <Utility/FileSystem.h>
#include <Utility/Log.h>
#include <Entity/Scene.h>
#include <Entity/Entity.h>
#include <Entity/Components/CCamera.h>
#include <Entity/Components/CTransform.h>
#include <Entity/Components/CMesh.h>
#include <MaterialCompiler/MaterialCompiler.h>
#include <Input/InputSystem.h>

CameraController::CameraController() {
    _on_mouse_position_handle = gear::gEngine.GetInputSystem()->GetOnMousePositionEvent().Bind(CALLBACK_2(CameraController::OnMousePosition, this));
    _on_mouse_button_handle = gear::gEngine.GetInputSystem()->GetOnMouseButtonEvent().Bind(CALLBACK_2(CameraController::OnMouseButton, this));
    _on_mouse_scroll_handle = gear::gEngine.GetInputSystem()->GetOnMouseScrollEvent().Bind(CALLBACK_1(CameraController::OnMouseScroll, this));
}

CameraController::~CameraController() {
    gear::gEngine.GetInputSystem()->GetOnMousePositionEvent().Unbind(_on_mouse_position_handle);
    gear::gEngine.GetInputSystem()->GetOnMouseButtonEvent().Unbind(_on_mouse_button_handle);
    gear::gEngine.GetInputSystem()->GetOnMouseScrollEvent().Unbind(_on_mouse_scroll_handle);
}

void CameraController::SetCamera(gear::Entity* camera) {
    if (_camera == camera) {
        return;
    }
    _camera = camera;
}

void CameraController::OnMousePosition(float x, float y) {
    if (!_grabbing) {
        return;
    }

    glm::vec2 offset = _start_point - glm::vec2(x, y);
    _start_point = glm::vec2(x, y);

    glm::vec3 euler = _camera->GetComponent<gear::CTransform>()->GetEuler();
    euler.x += offset.y * 0.001f;
    euler.y += offset.x * 0.001f;
    _camera->GetComponent<gear::CTransform>()->SetEuler(euler);
}

void CameraController::OnMouseButton(int button, int action) {
    double x, y;
    x = gear::gEngine.GetInputSystem()->GetMousePosition().x;
    y = gear::gEngine.GetInputSystem()->GetMousePosition().y;
    switch (action) {
        case 0:
            // Button Up
            if (button == 1) {
                End();
            }
            break;
        case 1:
            // Button Down
            if (button == 1) {
                Begin(x, y);
            }
            break;
        default:
            break;
    }
}

void CameraController::OnMouseScroll(float offset) {
    glm::vec3 camera_pos = _camera->GetComponent<gear::CTransform>()->GetPosition();
    glm::vec3 camera_front = _camera->GetComponent<gear::CTransform>()->GetFrontVector();
    camera_pos -= camera_front * offset * 0.2f;
    _camera->GetComponent<gear::CTransform>()->SetPosition(camera_pos);
}