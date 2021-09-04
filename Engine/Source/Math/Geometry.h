#pragma once	
#include "Core/GearDefine.h"
#include "Math.h"

namespace gear {
    class BBox {
    public:
        BBox() {
            bb_min = glm::vec3(GEAR_INF, GEAR_INF, GEAR_INF);
            bb_max = glm::vec3(GEAR_NEG_INF, GEAR_NEG_INF, GEAR_NEG_INF);
        }

        BBox::BBox(const glm::vec3& p) {
            bb_min = p;
            bb_max = p;
        }

        BBox::BBox(const glm::vec3& p1, const glm::vec3& p2) {
            bb_min = glm::vec3(glm::min(p1.x, p2.x), glm::min(p1.y, p2.y), glm::min(p1.z, p2.z));
            bb_max = glm::vec3(glm::max(p1.x, p2.x), glm::max(p1.y, p2.y), glm::max(p1.z, p2.z));
        }

        glm::vec3 BBox::Center() {
            return (bb_max + bb_min) * 0.5f;
        }

        glm::vec3 BBox::Diagonal() {
            return (bb_max - bb_min);
        }

        float BBox::SurfaceArea() {
            glm::vec3 d = Diagonal();
            return (d.x * d.y + d.x * d.z + d.y * d.z) * 2;
        }

        float BBox::Volume() {
            glm::vec3 d = Diagonal();
            return d.x * d.y * d.z;
        }

        int BBox::MaximumExtent() {
            glm::vec3 diag = Diagonal();
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

        bool IsEmpty() const noexcept {
            bool ret = false;
            for (int i = 0; i < 3; ++i) {
                ret |= bb_min[i] >= bb_max[i];
            }
            return ret;
        }

        struct Corners {
            glm::vec3 vertices[8];
        };

        Corners GetCorners() const {
            Corners corners;
            corners.vertices[0] = { bb_min.x, bb_min.y, bb_min.z };
            corners.vertices[1] = { bb_max.x, bb_min.y, bb_min.z };
            corners.vertices[2] = { bb_min.x, bb_max.y, bb_min.z };
            corners.vertices[3] = { bb_max.x, bb_max.y, bb_min.z };
            corners.vertices[4] = { bb_min.x, bb_min.y, bb_max.z };
            corners.vertices[5] = { bb_max.x, bb_min.y, bb_max.z };
            corners.vertices[6] = { bb_min.x, bb_max.y, bb_max.z };
            corners.vertices[7] = { bb_max.x, bb_max.y, bb_max.z };
            return corners;
        }

        void BBox::Grow(const BBox &bbox) {
            bb_min.x = glm::min(bb_min.x, bbox.bb_min.x);
            bb_min.y = glm::min(bb_min.y, bbox.bb_min.y);
            bb_min.z = glm::min(bb_min.z, bbox.bb_min.z);
            bb_max.x = glm::max(bb_max.x, bbox.bb_max.x);
            bb_max.y = glm::max(bb_max.y, bbox.bb_max.y);
            bb_max.z = glm::max(bb_max.z, bbox.bb_max.z);
        }

        void BBox::Grow(const glm::vec3& point) {
            bb_min.x = glm::min(bb_min.x, point.x);
            bb_min.y = glm::min(bb_min.y, point.y);
            bb_min.z = glm::min(bb_min.z, point.z);
            bb_max.x = glm::max(bb_max.x, point.x);
            bb_max.y = glm::max(bb_max.y, point.y);
            bb_max.z = glm::max(bb_max.z, point.z);
        }

        static BBox BBox::Grow(const BBox &bbox1, const BBox &bbox2) {
            BBox ret;
            ret.bb_min.x = glm::min(bbox1.bb_min.x, bbox2.bb_min.x);
            ret.bb_min.y = glm::min(bbox1.bb_min.y, bbox2.bb_min.y);
            ret.bb_min.z = glm::min(bbox1.bb_min.z, bbox2.bb_min.z);
            ret.bb_max.x = glm::max(bbox1.bb_max.x, bbox2.bb_max.x);
            ret.bb_max.y = glm::max(bbox1.bb_max.y, bbox2.bb_max.y);
            ret.bb_max.z = glm::max(bbox1.bb_max.z, bbox2.bb_max.z);
            return ret;
        }

        static BBox BBox::Grow(const BBox &bbox, const glm::vec3 &point) {
            BBox ret;
            ret.bb_min.x = glm::min(bbox.bb_min.x, point.x);
            ret.bb_min.y = glm::min(bbox.bb_min.y, point.y);
            ret.bb_min.z = glm::min(bbox.bb_min.z, point.z);
            ret.bb_max.x = glm::max(bbox.bb_max.x, point.x);
            ret.bb_max.y = glm::max(bbox.bb_max.y, point.y);
            ret.bb_max.z = glm::max(bbox.bb_max.z, point.z);
            return ret;
        }

    public:
        glm::vec3 bb_min;
        glm::vec3 bb_max;
    };

    class Frustum {
    public:
        Frustum() = default;

        Frustum(const glm::vec3 in_corners[8]) {
            for (uint32_t i = 0; i < 8; ++i) {
                corners[i] = in_corners[i];
            }
            
            glm::vec3 a = corners[0];
            glm::vec3 b = corners[1];
            glm::vec3 c = corners[2];
            glm::vec3 d = corners[3];
            glm::vec3 e = corners[4];
            glm::vec3 f = corners[5];
            glm::vec3 g = corners[6];
            glm::vec3 h = corners[7];

            //     c----d
            //    /|   /|
            //   g----h |
            //   | a--|-b
            //   |/   |/
            //   e----f

            auto plane = [](glm::vec3 p1, glm::vec3 p2, glm::vec3 p3) {
                auto v12 = p2 - p1;
                auto v23 = p3 - p2;
                auto n = glm::normalize(glm::cross(v12, v23));
                return glm::vec4{n, -glm::dot(n, p1)};
            };

            planes[0] = plane(a, e, g);   // left
            planes[1] = plane(f, b, d);   // right
            planes[2] = plane(a, b, f);   // bottom
            planes[3] = plane(g, h, d);   // top
            planes[4] = plane(a, c, d);   // far
            planes[5] = plane(e, f, h);   // near
        }
        
    public:
        glm::vec3 corners[8];
        glm::vec4 planes[6];
    };
}
