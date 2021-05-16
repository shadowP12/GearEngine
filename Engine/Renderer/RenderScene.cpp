#include "RenderScene.h"
#include "Renderer.h"
#include "GearEngine.h"
#include "Scene/Scene.h"
#include "Scene/Entity.h"
#include "Scene/Components/CCamera.h"
#include "Scene/Components/CRenderable.h"
#include "Scene/Components/CLight.h"
#include "Scene/Components/CTransform.h"
#include "Resource/Material.h"
#include "Resource/GpuBuffer.h"
#include <Blast/Gfx/GfxContext.h>

namespace gear {
    RenderScene::RenderScene(Renderer* renderer) {
        mRenderer = renderer;
        mViews.resize(8);
    }

    RenderScene::~RenderScene() {
        mViews.clear();
        mRenderables.clear();
    }

    /**
     * TODO: 灯光排序
     * TODO: 灯光收集
     */
    void RenderScene::prepare() {
        Scene* scene = gEngine.getScene();
        mViewCount = 0;
        mRenderableCount = 0;
        if (mRenderables.size() < scene->mEntities.size()) {
            mRenderables.resize(scene->mEntities.size());
        }

        for (int i = 0; i < scene->mEntities.size(); ++i) {
            if (scene->mEntities[i]->getComponent<CRenderable>()) {
                CRenderable* cr = scene->mEntities[i]->getComponent<CRenderable>();
                cr->updateRenderableBuffer();
                mRenderables[mRenderableCount].vertexBuffer = cr->mVertexBuffer;
                mRenderables[mRenderableCount].indexBuffer = cr->mIndexBufferr;
                mRenderables[mRenderableCount].renderableUB = cr->mRenderableBuffer;
                mRenderables[mRenderableCount].boneUB = cr->mBoneBuffer;
                mRenderables[mRenderableCount].materialInstance = cr->mMaterialInstance;
                mRenderables[mRenderableCount].type = cr->mPrimitive.type;
                mRenderables[mRenderableCount].offset = cr->mPrimitive.offset;
                mRenderables[mRenderableCount].count = cr->mPrimitive.count;
                mRenderableCount++;
            }

            // 最多支持8个RenderView
            if (scene->mEntities[i]->getComponent<CCamera>()) {
                CCamera* cc = scene->mEntities[i]->getComponent<CCamera>();
                cc->updateCameraBuffer();
                mViews[mViewCount].cameraUB = cc->mCameraUniformBuffer;
                mViews[mViewCount].lightUB = cc->mLightUniformBuffer;
                mViews[mViewCount].renderTarget = cc->mRenderTarget;
                mViewCount++;
            }
        }
    }
}