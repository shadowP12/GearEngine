#pragma once
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/quaternion.hpp>
#include <gtx/matrix_decompose.hpp>
#include <math.h>

#define PI 3.14159265359f
#define TWO_PI 6.28318530718f
#define INV_PI 0.31830988618f
#define INV_TWO_PI 0.15915494309f

inline glm::vec3 vectorLerp(const glm::vec3& from, const glm::vec3& to, float t)
{
    return from+(to-from)*t;
}

inline glm::vec3 getAxisX(const glm::mat4& mat)
{
	return glm::vec3(mat[0][0], mat[0][1], mat[0][2]);
}

inline glm::vec3 getAxisY(const glm::mat4& mat)
{
	return glm::vec3(mat[1][0], mat[1][1], mat[1][2]);
}

inline glm::vec3 getAxisZ(const glm::mat4& mat)
{
	return glm::vec3(mat[2][0], mat[2][1], mat[2][2]);
}

inline glm::vec3 getTranslate(const glm::mat4& mat)
{
	return glm::vec3(mat[3][0], mat[3][1], mat[3][2]);
}

inline glm::quat fromEulerAngles(const float& xAngle, const float& yAngle, const float& zAngle)
{
    float halfXAngle = xAngle * 0.5f;
    float halfYAngle = yAngle * 0.5f;
    float halfZAngle = zAngle * 0.5f;

    float cx = std::cos(halfXAngle);
    float sx = std::sin(halfXAngle);

    float cy = std::cos(halfYAngle);
    float sy = std::sin(halfYAngle);

    float cz = std::cos(halfZAngle);
    float sz = std::sin(halfZAngle);

    glm::quat quatX(cx, sx, 0.0f, 0.0f);
    glm::quat quatY(cy, 0.0f, sy, 0.0f);
    glm::quat quatZ(cz, 0.0f, 0.0f, sz);

    glm::quat r = quatX * quatZ * quatY;
    return  r;
}

inline glm::quat fromAxisAngle(const glm::vec3& axis, const float& angle)
{
    float halfSin = std::sin(0.5f * angle);
    float halfCos = std::cos(0.5f * angle);

    glm::quat r;
    r.w = halfCos;
    r.x = halfSin * axis.x;
    r.y = halfSin * axis.y;
    r.z = halfSin * axis.z;
    return r;
}

// screen to world space
inline glm::vec3 unProject(const glm::vec4& viewPort, const glm::vec2& point, float depth, const glm::mat4& viewProjectionMatrix)
{
	glm::mat4 inverseViewProjectionMatrix = glm::inverse(viewProjectionMatrix);
	glm::vec4 screen = glm::vec4((point.x - viewPort.x) / viewPort.z, ((viewPort.w - point.y) - viewPort.y) / viewPort.w, depth, 1.0f);

	// to[-1 , 1]
	screen.x = screen.x * 2.0f - 1.0f;
	screen.y = screen.y * 2.0f - 1.0f;
	screen.z = screen.z * 2.0f - 1.0f;

	glm::vec4 worldPoint = inverseViewProjectionMatrix * screen;
	worldPoint.x = worldPoint.x / worldPoint.w;
	worldPoint.y = worldPoint.y / worldPoint.w;
	worldPoint.z = worldPoint.z / worldPoint.w;

	glm::vec3 out = glm::vec3(worldPoint.x, worldPoint.y, worldPoint.z);
	return out;
}

inline void printMatrixInfo(const glm::mat4& mat)
{
	printf("%f   %f   %f   %f\n", mat[0][0], mat[0][1], mat[0][2], mat[0][3]);
	printf("%f   %f   %f   %f\n", mat[1][0], mat[1][1], mat[1][2], mat[1][3]);
	printf("%f   %f   %f   %f\n", mat[2][0], mat[2][1], mat[2][2], mat[2][3]);
	printf("%f   %f   %f   %f\n", mat[3][0], mat[3][1], mat[3][2], mat[3][3]);
}

inline glm::mat4 createPerspective(float fov, float aspect, float near, float far)
{
	float thetaY = (fov)*3.1415926f/180.0f*0.5f;
	float tanThetaY = std::tan(thetaY);

	glm::mat4 ret = glm::mat4(0.0f);
	ret[0][0] = 1.0f / (aspect * tanThetaY);
	ret[1][1] = 1.0f / (tanThetaY);
	ret[2][2] = -(far + near)/(far - near);
	ret[2][3] = -2.0f*far*near/(far - near);
	ret[3][2] = -1.0f;
	ret = glm::transpose(ret);
	return ret;
}

inline glm::mat4 createOrthographic(float left, float right, float bottom, float top, float near, float far)
{
	glm::mat4 ret(1);
	ret[0][0] = 2.0f / (right - left);
	ret[1][1] = 2.0f / (top - bottom);
	ret[2][2] = -2.0f / (far - near);
	ret[3][0] = -(right + left) / (right - left);
	ret[3][1] = -(top + bottom) / (top - bottom);
	ret[3][2] = -(far + near) / (far - near);
	return ret;
}

inline glm::mat4 createOrthographic(float width, float height, float near, float far)
{
	float halfWidth = width / 2.0f;
	float halfHeight = height / 2.0f;
	return createOrthographic(-halfWidth, halfWidth, -halfHeight, halfHeight, near, far);
}

inline glm::mat4 lookAt(const glm::vec3& eye, const glm::vec3& center, const glm::vec3& up)
{
	glm::vec3 f(glm::normalize(center - eye));
	glm::vec3 s(normalize(cross(f, up)));
	glm::vec3 u(cross(s, f));

	glm::mat4 ret(1);
	ret[0][0] = s.x;
	ret[1][0] = s.y;
	ret[2][0] = s.z;
	ret[0][1] = u.x;
	ret[1][1] = u.y;
	ret[2][1] = u.z;
	ret[0][2] = -f.x;
	ret[1][2] = -f.y;
	ret[2][2] = -f.z;
	ret[3][0] = -dot(s, eye);
	ret[3][1] = -dot(u, eye);
	ret[3][2] = dot(f, eye);
	return ret;
}

inline float getRandom(unsigned int *seed0, unsigned int *seed1)
{
    *seed0 = 36969 * ((*seed0) & 65535) + ((*seed0) >> 16);
    *seed1 = 18000 * ((*seed1) & 65535) + ((*seed1) >> 16);

    unsigned int ires = ((*seed0) << 16) + (*seed1);

    union {
        float f;
        unsigned int ui;
    } res;

    res.ui = (ires & 0x007fffff) | 0x40000000;
    return (res.f - 2.0f) / 2.0f;
}

inline glm::vec3 TransformPoint(const glm::vec3& point, const glm::mat4& inMat)
{
//    glm::vec4 p = glm::vec4(point.x, point.y, point.z, 1.0f);
//    glm::vec4 r = inMat * p;
//    return glm::vec3(r.x, r.y, r.z);
    glm::mat4 mat = glm::transpose(inMat);
    float x = point.x, y = point.y, z = point.z;
    float xp = mat[0][0] * x + mat[0][1] * y + mat[0][2] * z + mat[0][3];
    float yp = mat[1][0] * x + mat[1][1] * y + mat[1][2] * z + mat[1][3];
    float zp = mat[2][0] * x + mat[2][1] * y + mat[2][2] * z + mat[2][3];
    float wp = mat[3][0] * x + mat[3][1] * y + mat[3][2] * z + mat[3][3];
    assert(wp != 0);

    if (wp == 1.0f)
        return glm::vec3(xp, yp, zp);
    else
        return glm::vec3(xp, yp, zp) / wp;
}

inline glm::vec4 TransformPoint(const glm::vec4& point, const glm::mat4& inMat)
{
    glm::mat4 mat = glm::transpose(inMat);
    float x = point.x, y = point.y, z = point.z, w = point.w;
    float xp = mat[0][0] * x + mat[0][1] * y + mat[0][2] * z + mat[0][3] * w;
    float yp = mat[1][0] * x + mat[1][1] * y + mat[1][2] * z + mat[1][3] * w;
    float zp = mat[2][0] * x + mat[2][1] * y + mat[2][2] * z + mat[2][3] * w;
    float wp = mat[3][0] * x + mat[3][1] * y + mat[3][2] * z + mat[3][3] * w;

    return glm::vec4(xp, yp, zp, wp);
}

inline glm::vec3 TransformVector(const glm::vec3& vec, const glm::mat4& inMat)
{
    glm::mat4 mat = glm::transpose(inMat);
    float x = vec.x, y = vec.y, z = vec.z;

    return glm::vec3(mat[0][0] * x + mat[0][1] * y + mat[0][2] * z,
                     mat[1][0] * x + mat[1][1] * y + mat[1][2] * z,
                     mat[2][0] * x + mat[2][1] * y + mat[2][2] * z);
}

inline glm::vec3 TransformNormal(const glm::vec3& norm, const glm::mat4& inMat)
{
    return glm::mat3(glm::transpose(glm::inverse(inMat))) * norm;
    glm::mat4 mat = glm::transpose(inMat);
    mat = glm::inverse(mat);

    float x = norm.x, y = norm.y, z = norm.z;

    return glm::vec3(mat[0][0] * x + mat[1][0] * y + mat[2][0] * z,
                     mat[0][1] * x + mat[1][1] * y + mat[2][1] * z,
                     mat[0][2] * x + mat[1][2] * y + mat[2][2] * z);
}

inline glm::vec2 projectPointToVector(const glm::vec2& point, const glm::vec2& proj)
{
    glm::vec2 ret;
    const float mul = glm::dot(point, proj) / glm::dot(proj, proj);
    ret.x = mul * proj.x;
    ret.y = mul * proj.y;
    return ret;
}

inline glm::vec3 projectPointToVector(const glm::vec3& point, const glm::vec3& proj)
{
    glm::vec3 ret;
    const float mul = glm::dot(point, proj) / glm::dot(proj, proj);
    ret.x = mul * proj.x;
    ret.y = mul * proj.y;
    ret.z = mul * proj.z;
    return ret;
}

inline glm::vec3 matToEuler(const glm::mat4& inm)
{
    glm::mat4 m = glm::transpose(inm);
    float m21 = m[2][1];
    if (m21 < 1)
    {
        if (m21 > -1)
        {
            float xAngle = glm::radians(std::asin(m21));
            float yAngle = std::atan2(-m[2][0], m[2][2]);
            float zAngle = std::atan2(-m[0][1], m[1][1]);
            return glm::vec3(xAngle, yAngle, zAngle);
        }
        else
        {
            // Note: Not an unique solution.
            float xAngle = glm::radians(-3.1415926*0.5);
            float yAngle = glm::radians(0.0f);
            float zAngle = -std::atan2(m[0][2], m[0][0]);

            return glm::vec3(xAngle, yAngle, zAngle);
        }
    }
    else
    {
        // Note: Not an unique solution.
        float xAngle = glm::radians(3.1415926*0.5);
        float yAngle = glm::radians(0.0f);
        float zAngle = std::atan2(m[0][2], m[0][0]);
        return glm::vec3(xAngle, yAngle, zAngle);
    }
}

inline glm::quat eulerToQuat(const glm::vec3& euler)
{
    float halfXAngle = euler.x * 0.5f;
    float halfYAngle = euler.y * 0.5f;
    float halfZAngle = euler.z * 0.5f;

    float cx = std::cos(halfXAngle);
    float sx = std::sin(halfXAngle);

    float cy = std::cos(halfYAngle);
    float sy = std::sin(halfYAngle);

    float cz = std::cos(halfZAngle);
    float sz = std::sin(halfZAngle);

    glm::quat quatX(cx, sx, 0.0f, 0.0f);
    glm::quat quatY(cy, 0.0f, sy, 0.0f);
    glm::quat quatZ(cz, 0.0f, 0.0f, sz);

    return quatZ * (quatX * quatY);
}

inline glm::vec3 quatToEuler(const glm::quat& rot)
{
    glm::mat4 matRot = glm::toMat4(rot);
    return matToEuler(matRot);
}
