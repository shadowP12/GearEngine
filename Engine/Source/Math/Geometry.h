#pragma once	
#include "Core/GearDefine.h"
#include "Math.h"

namespace gear {
    struct Ray {
        static glm::vec3 PointAt(const Ray& ray, float t) {
            return ray.origin + ray.direction * t;
        }
        
        glm::vec3 origin;
        glm::vec3 direction;
    };
    
    struct Plane {
        float distance;
        glm::vec3 normal;
    };

    struct BBox {
        static glm::vec3 BBox::Center(const BBox& bbox) {
            return (bbox.bb_max + bbox.bb_min) * 0.5f;
        }

        static glm::vec3 BBox::Diagonal(const BBox& bbox) {
            return (bbox.bb_max - bbox.bb_min);
        }

        static float BBox::SurfaceArea(const BBox& bbox) {
            glm::vec3 d = Diagonal(bbox);
            return (d.x * d.y + d.x * d.z + d.y * d.z) * 2;
        }

        static float BBox::Volume(const BBox& bbox) {
            glm::vec3 d = Diagonal(bbox);
            return d.x * d.y * d.z;
        }

        static int BBox::MaximumExtent(const BBox& bbox) {
            glm::vec3 diag = Diagonal(bbox);
            if (diag.x > diag.y && diag.x > diag.z) {
                return 0;
            }
            else if (diag.y > diag.z) {
                return 1;
            }
            else {
                return 2;
            }
        }

        static bool IsEmpty(const BBox& bbox) {
            bool ret = false;
            for (int i = 0; i < 3; ++i) {
                ret |= bbox.bb_min[i] >= bbox.bb_max[i];
            }
            return ret;
        }

        static glm::vec3* Corners(const BBox& bbox) {
            glm::vec3 corners[8];
            corners[0] = { bbox.bb_min.x, bbox.bb_min.y, bbox.bb_min.z };
            corners[1] = { bbox.bb_max.x, bbox.bb_min.y, bbox.bb_min.z };
            corners[2] = { bbox.bb_min.x, bbox.bb_max.y, bbox.bb_min.z };
            corners[3] = { bbox.bb_max.x, bbox.bb_max.y, bbox.bb_min.z };
            corners[4] = { bbox.bb_min.x, bbox.bb_min.y, bbox.bb_max.z };
            corners[5] = { bbox.bb_max.x, bbox.bb_min.y, bbox.bb_max.z };
            corners[6] = { bbox.bb_min.x, bbox.bb_max.y, bbox.bb_max.z };
            corners[7] = { bbox.bb_max.x, bbox.bb_max.y, bbox.bb_max.z };
            return corners;
        }

        static BBox Grow(const BBox &bbox1, const BBox &bbox2) {
            BBox ret;
            ret.bb_min.x = glm::min(bbox1.bb_min.x, bbox2.bb_min.x);
            ret.bb_min.y = glm::min(bbox1.bb_min.y, bbox2.bb_min.y);
            ret.bb_min.z = glm::min(bbox1.bb_min.z, bbox2.bb_min.z);
            ret.bb_max.x = glm::max(bbox1.bb_max.x, bbox2.bb_max.x);
            ret.bb_max.y = glm::max(bbox1.bb_max.y, bbox2.bb_max.y);
            ret.bb_max.z = glm::max(bbox1.bb_max.z, bbox2.bb_max.z);
            return ret;
        }

        static BBox Grow(const BBox &bbox, const glm::vec3 &point) {
            BBox ret;
            ret.bb_min.x = glm::min(bbox.bb_min.x, point.x);
            ret.bb_min.y = glm::min(bbox.bb_min.y, point.y);
            ret.bb_min.z = glm::min(bbox.bb_min.z, point.z);
            ret.bb_max.x = glm::max(bbox.bb_max.x, point.x);
            ret.bb_max.y = glm::max(bbox.bb_max.y, point.y);
            ret.bb_max.z = glm::max(bbox.bb_max.z, point.z);
            return ret;
        }

        glm::vec3 bb_min = glm::vec3(GEAR_INF, GEAR_INF, GEAR_INF);
        glm::vec3 bb_max = glm::vec3(GEAR_NEG_INF, GEAR_NEG_INF, GEAR_NEG_INF);
    };

    struct Frustum {
        static void Generate(Frustum& frustum, const glm::vec3 corners[8]) {
            glm::vec3 a = corners[0];
            glm::vec3 b = corners[1];
            glm::vec3 c = corners[2];
            glm::vec3 d = corners[3];
            glm::vec3 e = corners[4];
            glm::vec3 f = corners[5];
            glm::vec3 g = corners[6];
            glm::vec3 h = corners[7];

            frustum.corners[0] = corners[0];
            frustum.corners[1] = corners[1];
            frustum.corners[2] = corners[2];
            frustum.corners[3] = corners[3];
            frustum.corners[4] = corners[4];
            frustum.corners[5] = corners[5];
            frustum.corners[6] = corners[6];
            frustum.corners[7] = corners[7];

            //     c----d
            //    /|   /|
            //   g----h |
            //   | a--|-b
            //   |/   |/
            //   e----f

            auto gen_plane_func = [](glm::vec3 p1, glm::vec3 p2, glm::vec3 p3) {
                auto v12 = p2 - p1;
                auto v23 = p3 - p2;
                auto n = glm::normalize(glm::cross(v12, v23));
                Plane plane;
                plane.distance = -glm::dot(n, p1);
                plane.normal = n;
                return plane;
            };

            frustum.planes[0] = gen_plane_func(a, e, g);   // left
            frustum.planes[1] = gen_plane_func(f, b, d);   // right
            frustum.planes[2] = gen_plane_func(a, b, f);   // bottom
            frustum.planes[3] = gen_plane_func(g, h, d);   // top
            frustum.planes[4] = gen_plane_func(a, c, d);   // far
            frustum.planes[5] = gen_plane_func(e, f, h);   // near
        }

        glm::vec3 corners[8];
        Plane planes[6];
    };
}
