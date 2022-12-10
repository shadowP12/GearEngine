#include "BaseWindow.h"
#include "GearEngine.h"
#include "Renderer/Renderer.h"

#include <GfxDefine.h>
#include <GfxDevice.h>

namespace gear {
    BaseWindow::BaseWindow() {
    }

    BaseWindow::~BaseWindow() {
        if (swapchain) {
            SAFE_DELETE(swapchain);
        }
    }

    blast::GfxSwapChain* BaseWindow::GetSwapChain() {
        // 如果当前窗口尺寸与交换链不一致则重新创建交换链，若当前窗口最小化则返回nullptr
        if (width == 0 ||height == 0) {
            return nullptr;
        }

        bool should_recreate = false;
        if (!swapchain) {
            should_recreate = true;
        } else {
            if (swapchain->desc.width != width || swapchain->desc.height != height) {
                should_recreate = true;
            }
        }

        if (should_recreate) {
            blast::GfxSwapChainDesc swapchain_desc;
            swapchain_desc.window = window_ptr;
            swapchain_desc.width = width;
            swapchain_desc.height = height;
            swapchain = gEngine.GetRenderer()->GetDevice()->CreateSwapChain(swapchain_desc, swapchain);
        }

        return swapchain;
    }
}