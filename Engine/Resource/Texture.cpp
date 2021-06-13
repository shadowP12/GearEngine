#include "Resource/Texture.h"
#include "GearEngine.h"
#include "Renderer/Renderer.h"
#include "Renderer/CopyEngine.h"
#include <Blast/Gfx/GfxContext.h>
#include <Blast/Gfx/GfxBuffer.h>
#include <Blast/Gfx/GfxTexture.h>
#include <Blast/Gfx/GfxCommandBuffer.h>

namespace gear {
    void Texture::Builder::width(uint32_t width) {
        mWidth = width;
    }

    void Texture::Builder::height(uint32_t height) {
        mHeight = height;
    }

    void Texture::Builder::depth(uint32_t depth) {
        mDepth = depth;
    }

    void Texture::Builder::array(uint32_t array) {
        mArray = array;
    }

    void Texture::Builder::format(Blast::Format format) {
        mFormat = format;
    }

    Texture * Texture::Builder::build() {
        return new Texture(this);
    }

    Texture::Texture(Builder* builder) {
        mWidth = builder->mWidth;
        mHeight = builder->mHeight;
        mDepth = builder->mDepth;
        mArray = builder->mArray;
        mFormat = builder->mFormat;
        for (int i = 0; i < mArray; ++i) {
            Blast::GfxContext* context = gEngine.getRenderer()->getContext();
            uint8_t* data = new uint8_t[mWidth * mHeight * mDepth * context->getFormatStride(mFormat)];
            mDatas.push_back(data);
        }

        Blast::GfxContext* context = gEngine.getRenderer()->getContext();
        Blast::GfxTextureDesc textureDesc;
        textureDesc.width = mWidth;
        textureDesc.height = mHeight;
        textureDesc.format = mFormat;
        textureDesc.type = Blast::RESOURCE_TYPE_TEXTURE | Blast::RESOURCE_TYPE_RW_TEXTURE;
        textureDesc.usage = Blast::RESOURCE_USAGE_GPU_ONLY;
        mInternelTexture = context->createTexture(textureDesc);
    }

    Texture::~Texture() {
        for (int i = 0; i < mDatas.size(); ++i) {
            SAFE_DELETE_ARRAY(mDatas[i]);
        }
        mDatas.clear();

        Renderer* renderer = gEngine.getRenderer();
        CopyEngine* copyEngine = renderer->getCopyEngine();
        copyEngine->destroy(mInternelTexture);
    }

    void Texture::setData(uint32_t i, void* data, uint32_t size) {
        uint8_t* dst = mDatas[i];
        uint8_t* src = (uint8_t*)data;
        memcpy(dst, src, size);

        Blast::GfxContext* context = gEngine.getRenderer()->getContext();
        uint32_t imageSize = mWidth * mHeight * mDepth * context->getFormatStride(mFormat);
        uint32_t totalSize = imageSize * mArray;
        Blast::GfxBufferDesc bufferDesc;
        bufferDesc.size = totalSize;
        bufferDesc.type = Blast::RESOURCE_TYPE_RW_BUFFER;
        bufferDesc.usage = Blast::RESOURCE_USAGE_CPU_TO_GPU;
        Blast::GfxBuffer* stagingBuffer = context->createBuffer(bufferDesc);
        uint32_t offset = 0;
        for (int i = 0; i < mArray; ++i) {
            stagingBuffer->writeData(offset, imageSize, mDatas[i]);
            offset += imageSize;
        }

        CopyEngine* copyEngine = gEngine.getRenderer()->getCopyEngine();
        CopyCommand* copyCommand = copyEngine->getActiveCommand();
        copyCommand->cmd->begin();
        {
            // 设置纹理为读写状态
            Blast::GfxTextureBarrier barrier;
            barrier.texture = mInternelTexture;
            barrier.newState = Blast::RESOURCE_STATE_COPY_DEST;
            copyCommand->cmd->setBarrier(0, nullptr, 1, &barrier);
        }

        offset = 0;
        Blast::GfxCopyToImageHelper helper;
        helper.bufferOffset = 0;
        helper.layer = 0;
        helper.level = 0;
        for (int i = 0; i < mArray; ++i) {
            helper.bufferOffset = offset;
            helper.layer = i;
            helper.level = 0;
            copyCommand->cmd->copyToImage(stagingBuffer, mInternelTexture, helper);
            offset += imageSize;
        }

        {
            // 设置纹理为Shader可读状态
            Blast::GfxTextureBarrier barrier;
            barrier.texture = mInternelTexture;
            barrier.newState = Blast::RESOURCE_STATE_SHADER_RESOURCE;
            copyCommand->cmd->setBarrier(0, nullptr, 1, &barrier);
        }
        copyCommand->cmd->end();

        copyCommand->callbacks.push_back([stagingBuffer, copyEngine]() {
            copyEngine->releaseStage(nullptr);
            delete stagingBuffer;
        });

        Blast::GfxSubmitInfo submitInfo;
        submitInfo.cmdBufCount = 1;
        submitInfo.cmdBufs = &copyCommand->cmd;
        submitInfo.signalFence = copyCommand->fence;
        submitInfo.waitSemaphoreCount = 0;
        submitInfo.waitSemaphores = nullptr;
        submitInfo.signalSemaphoreCount = 0;
        submitInfo.signalSemaphores = nullptr;
        gEngine.getRenderer()->getQueue()->submit(submitInfo);
    }
}