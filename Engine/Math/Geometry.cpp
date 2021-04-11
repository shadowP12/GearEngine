#include "Geometry.h"

namespace gear {
    void printRayInfo(const Ray& ray) {
        printf("ray orig:%f  %f  %f\n", ray.orig.x, ray.orig.y, ray.orig.z);
        printf("ray dir:%f  %f  %f\n", ray.dir.x, ray.dir.y, ray.dir.z);
    }

    Ray pickRay(const glm::vec4& viewport, const glm::vec2& point, const glm::mat4& view_matrix,
                const glm::mat4& projection_matrix) {
        glm::mat4 view_projection_matrix = projection_matrix * view_matrix;
        glm::vec3 near_point = unProject(viewport, point, 0.0f, view_projection_matrix);
        glm::vec3 far_point = unProject(viewport, point, 1.0f, view_projection_matrix);
        glm::vec3 dir = glm::normalize(far_point - near_point);
        glm::vec3 orig = getTranslate(glm::inverse(view_matrix));
        Ray ray;
        ray.dir = dir;
        ray.orig = orig;
        return ray;
    }

    float intersect(const Ray& ray, const Plane& plane) {
        float denom = glm::dot(plane.normal, ray.dir);
        if (std::abs(denom) > 1e-6) {
            glm::vec3 difference = plane.center - ray.orig;
            float t = glm::dot(plane.normal, difference) / denom;
            if (t >= 0)
                return t;
            return -1;
        }
        return -1;
    }

    bool intersect(const Ray& ray, const Triangle& triangle, float& t, glm::vec2 uv) {
        const glm::vec3& v0 = triangle.v0;
        const glm::vec3& v1 = triangle.v1;
        const glm::vec3& v2 = triangle.v2;
        glm::vec3 e1 = v1 - v0;
        glm::vec3 e2 = v2 - v0;
        glm::vec3 p = glm::cross(ray.dir, e2);
        float a = glm::dot(e1, p);
        if (std::fabs(a) < 1e-5f)
            return false;

        float f = 1.0f / a;
        glm::vec3 s = ray.orig - v0;
        float u = f * glm::dot(s, p);

        if (u < 0.0f || u > 1.0f)
            return false;

        glm::vec3 q = cross(s, e1);
        float v = f * glm::dot(ray.dir, q);

        if (v < 0.0f || (u + v) > 1.0f)
            return false;

        t = f * dot(e2, q);
        uv.x = u;
        uv.y = v;
        return true;
    }

    bool intersect(const Ray& ray, const Triangle& triangle, IntersectData* isect) {
        glm::vec3 e1 = triangle.v1 - triangle.v0;
        glm::vec3 e2 = triangle.v2 - triangle.v0;
        glm::vec3 p = glm::cross(ray.dir, e2);

        float a = dot(e1, p);

        if (fabs(a) < 0.0001)
            return false;
        float f = 1.0f / a;
        glm::vec3 s = ray.orig - triangle.v0;
        float u = f * dot(s, p);

        if (u < 0.0f || u > 1.0f)
            return false;
        glm::vec3 q = cross(s, e1);
        float v = f * dot(ray.dir, q);
        if (v < 0.0f || (u + v) > 1.0f)
            return false;

        float t = dot(e2, q) * f;

        //printf("%f\n",t);
        if (t > 0.0 && t < isect->t) {
            isect->hit = true;
            isect->t = t;
            isect->pos = isect->ray.orig + isect->ray.dir * t;
            isect->normal = normalize(u * triangle.n1 + v * triangle.n2 + (1.0f - u - v) * triangle.n0);
            isect->texcoord = u * triangle.t1 + v * triangle.t2 + (1.0f - u - v) * triangle.t0;
            return true;
        }
        return false;
    }

}