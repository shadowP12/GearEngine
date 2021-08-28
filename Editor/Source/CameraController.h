#pragma once
#include <Math/Math.h>
#include <Utility/Event.h>

namespace gear {
    class Entity;
}

class CameraController {
public:
    CameraController();

    ~CameraController();

    void SetCamera(gear::Entity* camera);

private:
    void Begin(int x, int y) {
        _grabbing = true;
        _start_point = glm::vec2(x, y);
    }

    void End() {
        _grabbing = false;
    }

    void OnMousePosition(float x, float y);

    void OnMouseButton(int button, int action);

    void OnMouseScroll(float offset);

private:
    gear::Entity* _camera = nullptr;
    bool _grabbing = false;
    glm::vec2 _start_point;
    EventHandle _on_mouse_position_handle;
    EventHandle _on_mouse_button_handle;
    EventHandle _on_mouse_scroll_handle;
};