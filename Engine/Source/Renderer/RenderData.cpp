#include "RenderData.h"

namespace gear {
    uint32_t GetVertexLayoutStride(VertexLayoutType vertex_layout) {
        switch(vertex_layout) {
            case VLT_P:
                return 12;
                break;
            case VLT_P_T0:
                return 20;
                break;
            case VLT_UI:
                return 20;
                break;
            case VLT_DEBUG:
                return 28;
                break;
            case VLT_STATIC_MESH:
                return 56;
                break;
            case VLT_SKIN_MESH:
                return 80;
                break;
        }
        return 0;
    }
}