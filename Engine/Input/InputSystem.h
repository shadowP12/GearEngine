#pragma once
#include "Core/GearDefine.h"
#include "Utility/Event.h"
#include "Math/Math.h"

namespace gear {
    class InputSystem {
    public:
        InputSystem();

        ~InputSystem();

        void reset();

        float getMouseScrollWheel() { return mMouseScrollWheel; }

        bool getMouseButtonHeld(uint8_t i) { return mMouseButtonHeld[i]; }

        bool getMouseButtonDown(uint8_t i) { return mMouseButtonDown[i]; }

        bool getMouseButtonUp(uint8_t i) { return mMouseButtonUp[i]; }

        glm::vec2 getMousePosition() { return mMousePosition; }

        void onMousePosition(float x, float y);

        void onMouseButton(int button, int action);

        void onMouseScroll(float offset);

        Event<void, float, float>& getOnMousePositionEvent();

        Event<void, int, int>& getOnMouseButtonEvent();

        Event<void, float>& getOnMouseScrollEvent();

    private:
        float mMouseScrollWheel;
        bool mMouseButtonHeld[3];
        bool mMouseButtonDown[3];
        bool mMouseButtonUp[3];
        glm::vec2 mMousePosition;
        Event<void, float, float> mOnMousePositionEvent;
        Event<void, int, int> mOnMouseButtonEvent;
        Event<void, float> mOnMouseScrollEvent;
    };
}
