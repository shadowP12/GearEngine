#pragma once
#include <Blast/Gfx/GfxDefine.h>

namespace gear {
    class Texture;
    class RenderUtility {
    public:
        RenderUtility();

        ~RenderUtility();

        Texture* EquirectangularMapToCubemap(Texture* equirectangular_map, uint32_t face_size);
    };
}