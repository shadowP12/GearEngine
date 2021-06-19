#include "InputSystem.h"

namespace gear {
    InputSystem::InputSystem() {
        mMousePosition = glm::vec2(0.0f, 0.0f);
        memset(mMouseButtonHeld, 0, sizeof(mMouseButtonHeld));
        memset(mMouseButtonUp, 0, sizeof(mMouseButtonUp));
        memset(mMouseButtonDown, 0, sizeof(mMouseButtonDown));
        mMouseScrollWheel = 0.0;
    }

    InputSystem::~InputSystem() {
    }

    void InputSystem::reset() {
        memset(mMouseButtonUp, 0, sizeof(mMouseButtonUp));
        memset(mMouseButtonDown, 0, sizeof(mMouseButtonDown));
        mMouseScrollWheel = 0.0;
    }

    void InputSystem::onMousePosition(float x, float y) {
        mMousePosition = glm::vec2(x, y);
        mOnMousePositionEvent.dispatch(x, y);
    }

    void InputSystem::onMouseButton(int button, int action) {
        switch (action) {
            case 0:
                mMouseButtonUp[button] = true;
                mMouseButtonHeld[button] = false;
                break;
            case 1:
                mMouseButtonDown[button] = true;
                mMouseButtonHeld[button] = true;
                break;
            default:
                break;
        }
        mOnMouseButtonEvent.dispatch(button, action);
    }

    void InputSystem::onMouseScroll(float offset) {
        mMouseScrollWheel = offset;
        mOnMouseScrollEvent.dispatch(offset);
    }

    Event<void, float, float> & InputSystem::getOnMousePositionEvent() {
        return mOnMousePositionEvent;
    }

    Event<void, int, int> & InputSystem::getOnMouseButtonEvent() {
        return mOnMouseButtonEvent;
    }

    Event<void, float> & InputSystem::getOnMouseScrollEvent() {
        return mOnMouseScrollEvent;
    }
}