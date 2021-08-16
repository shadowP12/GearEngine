#include "InputSystem.h"

namespace gear {
    InputSystem::InputSystem() {
        _mouse_position = glm::vec2(0.0f, 0.0f);
        memset(_mouse_button_held, 0, sizeof(_mouse_button_held));
        memset(_mouse_button_up, 0, sizeof(_mouse_button_up));
        memset(_mouse_button_down, 0, sizeof(_mouse_button_down));
        _mouse_scroll_wheel = 0.0;
    }

    InputSystem::~InputSystem() {
    }

    void InputSystem::Reset() {
        memset(_mouse_button_up, 0, sizeof(_mouse_button_up));
        memset(_mouse_button_down, 0, sizeof(_mouse_button_down));
        _mouse_scroll_wheel = 0.0;
    }

    void InputSystem::OnMousePosition(float x, float y) {
        _mouse_position = glm::vec2(x, y);
        _on_mouse_position_event.Dispatch(x, y);
    }

    void InputSystem::OnMouseButton(int button, int action) {
        switch (action) {
            case 0:
                _mouse_button_up[button] = true;
                _mouse_button_held[button] = false;
                break;
            case 1:
                _mouse_button_down[button] = true;
                _mouse_button_held[button] = true;
                break;
            default:
                break;
        }
        _on_mouse_button_event.Dispatch(button, action);
    }

    void InputSystem::OnMouseScroll(float offset) {
        _mouse_scroll_wheel = offset;
        _on_mouse_scroll_event.Dispatch(offset);
    }

    Event<void, float, float> & InputSystem::GetOnMousePositionEvent() {
        return _on_mouse_position_event;
    }

    Event<void, int, int> & InputSystem::GetOnMouseButtonEvent() {
        return _on_mouse_button_event;
    }

    Event<void, float> & InputSystem::GetOnMouseScrollEvent() {
        return _on_mouse_scroll_event;
    }
}