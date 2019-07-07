#ifndef DEBUG_CAMERA_H
#define DEBUG_CAMERA_H
#include "../Math/GMath.h"
#include "../Input/Input.h"

class DebugCamera 
{
public:
	DebugCamera(const glm::vec3& pos, float yaw = -90.0f, float pitch = 0.0f) 
	{
		mPos = pos;
		mYaw = yaw;
		mPitch = pitch;
		mFront = glm::vec3(0, 0, -1);
		mLastPos = Input::instance().getMousePosition();
		updateVectors();
	}

	void tick() 
	{
		glm::vec2 offset = Input::instance().getMousePosition() - mLastPos;
		mLastPos = Input::instance().getMousePosition();
		if (Input::instance().getMouseButton(MouseButton::MouseRight)) 
		{
			mYaw += offset.x * 0.1f;
			mPitch += -offset.y * 0.1f;
			updateVectors();
		}
		float sw = Input::instance().getMouseScrollWheel();
		mPos += mFront * sw * 0.1f;
	}

	glm::mat4 getViewMatrix() 
	{
		return glm::lookAt(mPos, mPos + mFront, mUp);
	}
private:
	void updateVectors() 
	{
		glm::vec3 front;
		front.x = cos(glm::radians(mYaw)) * cos(glm::radians(mPitch));
		front.y = sin(glm::radians(mPitch));
		front.z = sin(glm::radians(mYaw)) * cos(glm::radians(mPitch));
		mFront = glm::normalize(front);
		mRight = glm::normalize(glm::cross(mFront, glm::vec3(0, 1, 0)));
		mUp = glm::normalize(glm::cross(mRight, mFront));
	}
private:
	glm::vec3 mPos;
	glm::vec3 mFront;
	glm::vec3 mUp;
	glm::vec3 mRight;

	float mYaw;
	float mPitch;

	glm::vec2 mLastPos;
};

#endif // DEBUG_CAMERA_H