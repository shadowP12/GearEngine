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
namespace gear {

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

        // 为了方便调试写死灯光方向
        const glm::vec3 dir = glm::vec3(0.0f, -1.0f, 0.0f);
        // 计算灯光矩阵
        const glm::mat4 M = glm::lookAt(cascadeParams.wsLightPosition, cascadeParams.wsLightPosition + dir, glm::vec3(0, 1, 0));
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

    void updateShadowMapData() {
        // 重新设置相机投影矩阵，为了让视锥体更加紧凑
        // 开始计算灯光投影矩阵
        // 1.计算灯光矩阵，使用lookat函数
        // 2.计算相机视锥体在世界坐标的顶点
        // 3.计算接收阴影包围盒与视锥体的相交的顶点
        // 4.计算灯光空间的包围盒
        // 5.由灯光空间的包围盒计算出灯光视锥体的远近平面
        // 6.远近平面计算出灯光的投影矩阵
        // 7.可选内容，通过扭曲灯光投影矩阵的方法去改善阴影贴图
        
        
        
    }

    void Renderer::renderShadow(RenderView* view) {
        CameraInfo cameraInfo = view->cameraInfo;
        // 更新cascade的参数
        CascadeParameters cascadeParams;
        computeSceneCascadeParams(view, cameraInfo, cascadeParams);

        float splitPercentages[SHADOW_CASCADE_COUNT + 1];
        for (int i = 0; i < SHADOW_CASCADE_COUNT + 1; ++i) {
            splitPercentages[i] = i * 0.3f;
        }
        splitPercentages[SHADOW_CASCADE_COUNT] = 1.0f;

        float splitsWs[SHADOW_CASCADE_COUNT + 1];
        float splitsCs[SHADOW_CASCADE_COUNT + 1];
        for (int i = 0; i < SHADOW_CASCADE_COUNT + 1; i++) {
            splitsWs[i] = p.near + (p.far - p.near) * p.splitPositions[s];
            splitsCs[i] = mat4f::project(p.proj, float3(0.0f, 0.0f, mSplitsWs[s])).z;
        }

        // 
        
    }
}