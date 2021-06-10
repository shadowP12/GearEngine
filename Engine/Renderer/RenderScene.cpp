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
    }

    RenderScene::~RenderScene() {
    }

    /**
     * TODO: 灯光排序
     * TODO: 灯光收集
     */
    void RenderScene::prepare() {
        Scene* scene = gEngine.getScene();

        mViewCount = 0;
        for (int i = 0; i < scene->mCameraEntities.size(); ++i) {
            if (i >= 8) {
                // 最多只需要8个RenderView
                break;
            }
            CCamera* cc = scene->mCameraEntities[i]->getComponent<CCamera>();
            cc->updateCameraBuffer();
            mViews[mViewCount].cameraUB = cc->mCameraUniformBuffer;
            mViews[mViewCount].lightUB = cc->mLightUniformBuffer;
            if (cc->mRenderTarget == nullptr) {
                mViews[mViewCount].renderTarget = mRenderer->getRenderTarget();
            } else {
                mViews[mViewCount].renderTarget = cc->mRenderTarget;
            }
            mViews[mViewCount].layer = cc->mLayer;
            mViews[mViewCount].renderableCount = 0;
            if (mViews[mViewCount].renderables.size() < scene->mEntities.size()) {
                mViews[mViewCount].renderables.resize(scene->mEntities.size());
            }
            mViewCount++;
        }

        for (int i = 0; i < scene->mEntities.size(); ++i) {
            if (scene->mEntities[i]->getComponent<CRenderable>()) {
                CRenderable* cr = scene->mEntities[i]->getComponent<CRenderable>();
                for (int j = 0; j < mViewCount; ++j) {
                    if (mViews[j].layer == cr->mLayer) {
                        cr->updateRenderableBuffer();
                        mViews[j].renderables[mViews[j].renderableCount].renderableUB = cr->mRenderableBuffer;
                        mViews[j].renderables[mViews[j].renderableCount].boneUB = cr->mBoneBuffer;
                        mViews[j].renderables[mViews[j].renderableCount].primitives.clear();
                        for (int k = 0; k < cr->mPrimitives.size(); ++k) {
                            mViews[j].renderables[mViews[j].renderableCount].primitives.push_back(cr->mPrimitives[k]);
                        }
                        mViews[j].renderableCount++;
                    }
                }
            }
        }
    }
}