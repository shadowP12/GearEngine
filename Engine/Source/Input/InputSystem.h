#pragma once
#include "Core/GearDefine.h"
#include "Utility/Event.h"
#include "Math/Math.h"

namespace gear {
    class InputSystem {
    public:
        InputSystem();

        ~InputSystem();

        void Reset();

        float GetMouseScrollWheel() { return _mouse_scroll_wheel; }

        bool GetMouseButtonHeld(uint8_t i) { return _mouse_button_held[i]; }

        bool GetMouseButtonDown(uint8_t i) { return _mouse_button_down[i]; }

        bool GetMouseButtonUp(uint8_t i) { return _mouse_button_up[i]; }

        glm::vec2 GetMousePosition() { return _mouse_position; }

        void OnMousePosition(float x, float y);

        void OnMouseButton(int button, int action);

        void OnMouseScroll(float offset);

        Event<void, float, float>& GetOnMousePositionEvent();

        Event<void, int, int>& GetOnMouseButtonEvent();

        Event<void, float>& GetOnMouseScrollEvent();

    private:
        float _mouse_scroll_wheel;
        bool _mouse_button_held[3];
        bool _mouse_button_down[3];
        bool _mouse_button_up[3];
        glm::vec2 _mouse_position;
        Event<void, float, float> _on_mouse_position_event;
        Event<void, int, int> _on_mouse_button_event;
        Event<void, float> _on_mouse_scroll_event;
    };
}
