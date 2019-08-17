#ifndef INPUT_H
#define INPUT_H
#include "../Utility/Module.h"
#include "KeyCode.h"
#include "../Math/GMath.h"
#include <memory.h>

class Input :public Module<Input> 
{
public:
	Input() 
	{
		memset(mKeyDown, 0, sizeof(mKeyDown));
		memset(mKeyUp, 0, sizeof(mKeyUp));
		memset(mMouseButtonDown, 0, sizeof(mMouseButtonDown));
		memset(mMouseButtonUp, 0, sizeof(mMouseButtonUp));
		mMouseScrollWheel = 0;
		memset(mMouseButtonHeld, 0, sizeof(mMouseButtonHeld));
		memset(mKey, 0, sizeof(mKey));
	}

	void update() 
	{
		memset(mKeyDown, 0, sizeof(mKeyDown));
		memset(mKeyUp, 0, sizeof(mKeyUp));
		memset(mMouseButtonDown, 0, sizeof(mMouseButtonDown));
		memset(mMouseButtonUp, 0, sizeof(mMouseButtonUp));
		mMouseScrollWheel = 0;
	}

	bool getKey(KeyCode key) 
	{
		return mKey[(int)key];
	}

	bool getKeyDown(KeyCode key) 
	{
		return mKeyDown[(int)key];
	}

	bool getKeyUp(KeyCode key) 
	{
		return mKeyUp[(int)key];
	}

	bool getMouseButton(MouseButton mouse) 
	{
		return mMouseButtonHeld[(int)mouse];
	}

	bool getMouseButtonDown(MouseButton mouse) 
	{
		return mMouseButtonDown[(int)mouse];
	}

	bool getMouseButtonUp(MouseButton mouse) 
	{
		return mMouseButtonUp[(int)mouse];
	}

	glm::vec2 getMousePosition() 
	{
		return mMousePosition;
	}

	float getMouseScrollWheel() 
	{
		return mMouseScrollWheel;
	}

	void setMouseButton(int idx, bool state) 
	{
		if (idx > 2 || idx < 0) 
		{
			return;
		}
		mMouseButtonHeld[idx] = state;
	}

	void setMouseButtonDown(int idx) 
	{
		if (idx > 2 || idx < 0) 
		{
			return;
		}
		mMouseButtonDown[idx] = true;
	}

	void setMouseButtonUp(int idx) 
	{
		if (idx > 2 || idx < 0) 
		{
			return;
		}
		mMouseButtonUp[idx] = true;
	}

	void setKey(int idx, bool state) 
	{
		if (idx > ((int)KeyCode::Count) - 1 || idx < 0) {
			return;
		}

		mKey[idx] = state;
	}

	void setKeyDown(int idx) 
	{
		if (idx > ((int)KeyCode::Count) - 1 || idx < 0) 
		{
			return;
		}

		mKeyDown[idx] = true;
	}

	void setKeyUp(int idx) 
	{
		if (idx > ((int)KeyCode::Count) - 1 || idx < 0) 
		{
			return;
		}

		mKeyUp[idx] = true;
	}

	void setMousePosition(const glm::vec2& pos) 
	{
		mMousePosition = pos;
	}

	void setMouseScrollWheel(float offset) 
	{
		mMouseScrollWheel = offset;
	}

private:
	float mMouseScrollWheel;
	glm::vec2 mMousePosition;
	bool mMouseButtonHeld[3];
	bool mMouseButtonDown[3];
	bool mMouseButtonUp[3];
	bool mKeyDown[(int)KeyCode::Count];
	bool mKey[(int)KeyCode::Count];
	bool mKeyUp[(int)KeyCode::Count];
};

#endif // INPUT_H