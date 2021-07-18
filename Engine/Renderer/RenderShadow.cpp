#include "Renderer.h"
#include "RenderTarget.h"
#include "RenderCache.h"
#include "RenderScene.h"
#include "RenderBuiltinResource.h"
#include "Resource/GpuBuffer.h"
#include "Resource/Material.h"
#include "Resource/Texture.h"
#include <Blast/Gfx/GfxRenderTarget.h>
#include <Blast/Gfx/GfxCommandBuffer.h>
#include <algorithm>
namespace gear {

    struct ShadowMapData {
        glm::mat4 lightProjectionMatrix;
        bool hasVisibleShadows;
    };

    struct SegmentIndex {
        uint8_t v0, v1;
    };

    struct QuadIndex {
        uint8_t v0, v1, v2, v3;
    };

    static constexpr const SegmentIndex sBoxSegments[12] = {
            { 0, 1 }, { 1, 3 }, { 3, 2 }, { 2, 0 },
            { 4, 5 }, { 5, 7 }, { 7, 6 }, { 6, 4 },
            { 0, 4 }, { 1, 5 }, { 3, 7 }, { 2, 6 },
    };

    static constexpr const QuadIndex sBoxQuads[6] = {
            { 2, 0, 1, 3 },  // far
            { 6, 4, 5, 7 },  // near
            { 2, 0, 4, 6 },  // left
            { 3, 1, 5, 7 },  // right
            { 0, 4, 5, 1 },  // bottom
            { 2, 6, 7, 3 },  // top
    };

    // 为了方便调试写死灯光方向
    static glm::vec3 gSunDir = glm::vec3(0.0f, -1.0f, 0.0f);

    // 计算不同坐标系中的near和far
    glm::vec2 computeNearFar(const glm::mat4& view, glm::vec3* wsVertices, size_t count) {
        glm::vec2 nearFar = { std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max() };
        for (size_t i = 0; i < count; i++) {
            // 右手坐标系，看向-z
            float c = TransformPoint(wsVertices[i], view).z;
            nearFar.x = std::max(nearFar.x, c);  // near
            nearFar.y = std::min(nearFar.y, c);  // far
        }
        return nearFar;
    }

    glm::vec2 computeNearFar(const glm::mat4& view, BBox& wsShadowCastersVolume) {
        glm::vec3* wsSceneCastersCorners = wsShadowCastersVolume.getCorners();
        return computeNearFar(view, wsSceneCastersCorners, 8);
    }


    void Renderer::computeSceneCascadeParams(RenderView* view, const CameraInfo& cameraInfo, CascadeParameters& cascadeParams) {
        // 方向光取任何位置都没有问题，这里默认设置为相机当前位置
        cascadeParams.wsLightPosition = cameraInfo.cameraPosition;

        // 计算灯光矩阵
        const glm::mat4 M = glm::lookAt(cascadeParams.wsLightPosition, cascadeParams.wsLightPosition + gSunDir, glm::vec3(0, 1, 0));
        const glm::mat4 Mv = glm::inverse(M);
        const glm::mat4 V = cameraInfo.view;

        // 遍历场景计算出对应的参数
        for (int i = 0; i < view->renderableCount; ++i) {
            Renderable* rb = &view->renderables[i];
            for (int j = 0; j < rb->primitives.size(); ++j) {
                RenderPrimitive* prim = &rb->primitives[j];
                if (prim->castShadow) {
                    cascadeParams.wsShadowCastersVolume.mMin = min(cascadeParams.wsShadowCastersVolume.mMin, prim->bbox.mMin);
                    cascadeParams.wsShadowCastersVolume.mMax = max(cascadeParams.wsShadowCastersVolume.mMax, prim->bbox.mMax);
                    glm::vec2 nf = computeNearFar(Mv, prim->bbox);
                    cascadeParams.lsNearFar.x = std::max(cascadeParams.lsNearFar.x, nf.x);
                    cascadeParams.lsNearFar.y = std::min(cascadeParams.lsNearFar.y, nf.y);
                }

                if (prim->receiveShadow) {
                    cascadeParams.wsShadowReceiversVolume.mMin = min(cascadeParams.wsShadowReceiversVolume.mMin, prim->bbox.mMin);
                    cascadeParams.wsShadowReceiversVolume.mMax = max(cascadeParams.wsShadowReceiversVolume.mMax, prim->bbox.mMax);
                    glm::vec2 nf = computeNearFar(V, prim->bbox);
                    cascadeParams.vsNearFar.x = std::max(cascadeParams.vsNearFar.x, nf.x);
                    cascadeParams.vsNearFar.y = std::min(cascadeParams.vsNearFar.y, nf.y);
                }
            }
        }

    }

    void computeFrustumCorners(glm::vec3* out, const glm::mat4& projectionViewInverse, const glm::vec2 csNearFar) {
        // ndc -> camera -> world
        float near = csNearFar.x;
        float far = csNearFar.y;
        glm::vec3 csViewFrustumCorners[8] = {
                { -1, -1,  far },
                {  1, -1,  far },
                { -1,  1,  far },
                {  1,  1,  far },
                { -1, -1,  near },
                {  1, -1,  near },
                { -1,  1,  near },
                {  1,  1,  near },
        };
        for (glm::vec3 c : csViewFrustumCorners) {
            *out++ = TransformPoint(c, projectionViewInverse);
        }
    }

    bool intersectSegmentWithTriangle(glm::vec3& p, glm::vec3 s0, glm::vec3 s1, glm::vec3 t0, glm::vec3 t1, glm::vec3 t2) {
        constexpr const float EPSILON = 1.0f / 65536.0f;  // ~1e-5
        const auto e1 = t1 - t0;
        const auto e2 = t2 - t0;
        const auto d = s1 - s0;
        const auto q = glm::cross(d, e2);
        const auto a = glm::dot(e1, q);
        if (std::abs(a) < EPSILON) {
            // 无法构成三角形
            return false;
        }
        const auto s = s0 - t0;
        const auto u = dot(s, q) * glm::sign(a);
        const auto r = cross(s, e1);
        const auto v = dot(d, r) * glm::sign(a);
        if (u < 0 || v < 0 || u + v > std::abs(a)) {
            // 射线没有和三角形相交
            return false;
        }
        const auto t = dot(e2, r) * glm::sign(a);
        if (t < 0 || t > std::abs(a)) {
            // 射线相交但是不在线段内
            return false;
        }

        // 计算相交点位置
        p = s0 + d * (t / std::abs(a));
        return true;
    }

    bool intersectSegmentWithPlanarQuad(glm::vec3& p, glm::vec3 s0, glm::vec3 s1, glm::vec3 t0, glm::vec3 t1, glm::vec3 t2, glm::vec3 t3) {
        bool hit = intersectSegmentWithTriangle(p, s0, s1, t0, t1, t2) ||
                   intersectSegmentWithTriangle(p, s0, s1, t0, t2, t3);
        return hit;
    }

    void intersectFrustum(glm::vec3* out, uint32_t& vertexCount, glm::vec3 const* segmentsVertices, glm::vec3 const* quadsVertices) {
        for (const SegmentIndex segment : sBoxSegments) {
            const glm::vec3 s0{ segmentsVertices[segment.v0] };
            const glm::vec3 s1{ segmentsVertices[segment.v1] };
            // 每条线段最多与两个quad相交
            int maxVertexCount = vertexCount + 2;
            for (int j = 0; j < 6 && vertexCount < maxVertexCount; ++j) {
                const QuadIndex quad = sBoxQuads[j];
                const glm::vec3 t0{ quadsVertices[quad.v0] };
                const glm::vec3 t1{ quadsVertices[quad.v1] };
                const glm::vec3 t2{ quadsVertices[quad.v2] };
                const glm::vec3 t3{ quadsVertices[quad.v3] };
                if (intersectSegmentWithPlanarQuad(out[vertexCount], s0, s1, t0, t1, t2, t3)) {
                    vertexCount++;
                }
            }
        }
    }

    void intersectFrustumWithBox(const glm::vec3* wsFrustumCorners, const BBox& wsBox, glm::vec3* outVertices, uint32_t& outVertexCount) {
        /*
         * 算法:
         * 1.计算视锥体处于包围盒的顶点
         * 2.计算包围盒处于视锥体的顶点
         * 3.计算包围盒的边和视锥体平面之间的相交
         * 4.计算视锥体的边和包围盒平面之间的相交
         */
        uint32_t vertexCount = 0;

        for (size_t i = 0; i < 8; i++) {
            glm::vec3 p = wsFrustumCorners[i];
            outVertices[vertexCount] = p;
            if ((p.x >= wsBox.mMin.x && p.x <= wsBox.mMax.x) &&
                (p.y >= wsBox.mMin.y && p.y <= wsBox.mMax.y) &&
                (p.z >= wsBox.mMin.z && p.z <= wsBox.mMax.z)) {
                vertexCount++;
            }
        }
        const bool someFrustumVerticesAreInTheBox = vertexCount > 0;
        constexpr const float EPSILON = 1.0f / 8192.0f; // ~0.012 mm

        // 如果顶点数量等于8说明视锥体被完全覆盖，不需要优化
        if (vertexCount < 8) {
            Frustum frustum(wsFrustumCorners);
            glm::vec4* wsFrustumPlanes = frustum.getNormalizedPlanes();

            // 包围盒的8个顶点
            const glm::vec3* wsSceneReceiversCorners = wsBox.getCorners();

            for (int i = 0; i < 8; ++i) {
                glm::vec3 p = wsSceneReceiversCorners[i];
                outVertices[vertexCount] = p;
                // l/b/r/t/f/n分别表示到视锥体对应平面的距离
                float l = glm::dot(glm::vec3(wsFrustumPlanes[0]), p) + wsFrustumPlanes[0].w;
                float b = glm::dot(glm::vec3(wsFrustumPlanes[1]), p) + wsFrustumPlanes[1].w;
                float r = glm::dot(glm::vec3(wsFrustumPlanes[2]), p) + wsFrustumPlanes[2].w;
                float t = glm::dot(glm::vec3(wsFrustumPlanes[3]), p) + wsFrustumPlanes[3].w;
                float f = glm::dot(glm::vec3(wsFrustumPlanes[4]), p) + wsFrustumPlanes[4].w;
                float n = glm::dot(glm::vec3(wsFrustumPlanes[5]), p) + wsFrustumPlanes[5].w;
                if ((l <= EPSILON) && (b <= EPSILON) &&
                    (r <= EPSILON) && (t <= EPSILON) &&
                    (f <= EPSILON) && (n <= EPSILON)) {
                    ++vertexCount;
                }
            }

            // 如果视锥体没有被包围盒完全包围，或者没有包围整个包围盒，则说明两者边界存在交点
            if (someFrustumVerticesAreInTheBox || vertexCount < 8) {
                intersectFrustum(outVertices, vertexCount, wsSceneReceiversCorners, wsFrustumCorners);

                intersectFrustum(outVertices, vertexCount, wsFrustumCorners, wsSceneReceiversCorners);
            }
        }
    }

    void updateShadowMapData(const CameraInfo& cameraInfo, const CascadeParameters& cascadeParams, ShadowMapData& data) {
//        projection[2][2] = f / (n - f);
//        projection[3][2] = -(f * n) / (f - n);
        // 开始计算灯光投影矩阵
        if (cascadeParams.wsShadowCastersVolume.isEmpty() || cascadeParams.wsShadowReceiversVolume.isEmpty()) {
            data.hasVisibleShadows = false;
            return;
        }

        // 1.计算灯光矩阵，使用lookat函数
        const glm::vec3 lightPosition = cascadeParams.wsLightPosition;
        const glm::mat4 lightViewMatrix = glm::lookAt(lightPosition, lightPosition + gSunDir, glm::vec3{ 0, 1, 0 });
        const glm::mat4 lightModelMatrix = glm::inverse(lightViewMatrix);

        // 2.计算相机视锥体在世界坐标的顶点
        glm::vec3 wsViewFrustumVertices[8];
        computeFrustumCorners(wsViewFrustumVertices, cameraInfo.model * glm::inverse(cameraInfo.projection), cascadeParams.csNearFar);

        // 3.计算接收阴影包围盒与视锥体的相交的顶点
        glm::vec3 wsClippedShadowReceiverVolume[64];
        uint32_t vertexCount = 0;
        intersectFrustumWithBox(wsViewFrustumVertices, cascadeParams.wsShadowReceiversVolume, wsClippedShadowReceiverVolume, vertexCount);
        data.hasVisibleShadows = vertexCount >= 2;
        if (!data.hasVisibleShadows) {
            return;
        }

        // 4.计算灯光空间的包围盒
        BBox lsLightFrustumBounds;
        for (size_t i = 0; i < vertexCount; ++i) {
            // 利用相机视锥体与物体相交信息求出灯光包围盒
            glm::vec3 v = TransformPoint(wsClippedShadowReceiverVolume[i], lightModelMatrix);
            lsLightFrustumBounds.mMin.z = std::min(lsLightFrustumBounds.mMin.z, v.z);
            lsLightFrustumBounds.mMax.z = std::max(lsLightFrustumBounds.mMax.z, v.z);
        }

        // 5.由灯光空间的包围盒计算出灯光视锥体的远近平面
        const float znear = -lsLightFrustumBounds.mMax.z;
        const float zfar = -lsLightFrustumBounds.mMin.z;
        if (znear >= zfar) {
            data.hasVisibleShadows = false;
            return;
        }

        // 6.远近平面计算出灯光的投影矩阵，方向光使用正交投影
        data.lightProjectionMatrix = glm::mat4(1.0f);
        data.lightProjectionMatrix[2][2] = zfar / (znear - zfar);
        data.lightProjectionMatrix[3][2] = -(zfar * znear) / (zfar - znear);

        // 7.可选内容，通过扭曲灯光投影矩阵的方法去改善阴影贴图
        
    }

    void Renderer::renderShadow(RenderView* view) {
        CameraInfo cameraInfo = view->cameraInfo;
        // 更新cascade的参数
        CascadeParameters cascadeParams;
        computeSceneCascadeParams(view, cameraInfo, cascadeParams);

        // 更新分割变量
        float vsNear = cascadeParams.vsNearFar.x;
        float vsFar = cascadeParams.vsNearFar.y;

        float splitPercentages[SHADOW_CASCADE_COUNT + 1];
        for (int i = 0; i < SHADOW_CASCADE_COUNT + 1; ++i) {
            splitPercentages[i] = i * 0.3f;
        }
        splitPercentages[SHADOW_CASCADE_COUNT] = 1.0f;

        float splitsWs[SHADOW_CASCADE_COUNT + 1];
        float splitsCs[SHADOW_CASCADE_COUNT + 1];
        for (int i = 0; i < SHADOW_CASCADE_COUNT + 1; i++) {
            splitsWs[i] = vsNear + (vsFar - vsNear) * splitPercentages[i];
            splitsCs[i] = TransformPoint(cameraInfo.projection, glm::vec3(0.0f, 0.0f, splitsWs[i])).z;
        }

        // 更新shadow map的投影矩阵
        ShadowMapData datas[SHADOW_CASCADE_COUNT];
        for (int i = 0; i < SHADOW_CASCADE_COUNT; i++) {
            cascadeParams.csNearFar = { splitsCs[i], splitsCs[i + 1] };
            updateShadowMapData(cameraInfo, cascadeParams, datas[i]);
        }

        // 生成绘制命令并执行
        for (int i = 0; i < SHADOW_CASCADE_COUNT; ++i) {
            uint32_t shadowDrawCallHead = mDrawCallHead;
            uint32_t shadowDrawCallCount = 0;
            for (int i = 0; i < view->renderableCount; ++i) {
                Renderable* rb = &view->renderables[i];
                for (int j = 0; j < rb->primitives.size(); ++j) {
                    RenderPrimitive* rp = &rb->primitives[j];
                    DrawCall& dc = mDrawCalls[shadowDrawCallHead + shadowDrawCallCount];
                    dc.variant = MaterialVariant::DEPTH;




//                    mDrawCalls[shadowDrawCallHead + shadowDrawCallCount].variant = 0;
//                    mDrawCalls[shadowDrawCallHead + shadowDrawCallCount].renderableUB = rb->renderableUB;
//                    mDrawCalls[shadowDrawCallHead + shadowDrawCallCount].boneUB = rb->boneUB;
//                    mDrawCalls[shadowDrawCallHead + shadowDrawCallCount].vertexBuffer = rp->vertexBuffer;
//                    mDrawCalls[shadowDrawCallHead + shadowDrawCallCount].indexBuffer = rp->indexBuffer;
//                    mDrawCalls[shadowDrawCallHead + shadowDrawCallCount].count = rp->count;
//                    mDrawCalls[shadowDrawCallHead + shadowDrawCallCount].offset = rp->offset;
//                    mDrawCalls[shadowDrawCallHead + shadowDrawCallCount].type = rp->type;
//                    mDrawCalls[shadowDrawCallHead + shadowDrawCallCount].materialInstance = rp->materialInstance;
//                    mDrawCalls[shadowDrawCallHead + shadowDrawCallCount].blendState = rp->materialInstance->getMaterial()->mBlendState;
//                    mDrawCalls[shadowDrawCallHead + shadowDrawCallCount].depthState = rp->materialInstance->getMaterial()->mDepthState;
//                    mDrawCalls[shadowDrawCallHead + shadowDrawCallCount].rasterizerState = rp->materialInstance->getMaterial()->mRasterizerState;
                    shadowDrawCallCount++;
                }
            }
            mDrawCallHead = shadowDrawCallCount;
            // 排序
            std::sort(&mDrawCalls[shadowDrawCallHead], &mDrawCalls[shadowDrawCallHead] + shadowDrawCallCount);
        }

        
    }
}