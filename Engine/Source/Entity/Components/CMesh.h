#pragma once
#include "Component.h"
#include "Math/Math.h"
#include "Math/Geometry.h"
#include "Renderer/RenderData.h"

#include <Blast/Gfx/GfxDefine.h>

#include <vector>

namespace gear {
    class Entity;
    class VertexBuffer;
    class IndexBuffer;
    class UniformBuffer;
    class MaterialInstance;
    class Skeleton;

    struct SubMesh {
        bool cast_shadow = false;
        bool receive_shadow = true;
        uint32_t count = 0;
        uint32_t offset = 0;
        BBox bbox;
        MaterialInstance* mi = nullptr;
        VertexBuffer* vb = nullptr;
        IndexBuffer* ib = nullptr;
        blast::PrimitiveTopology topo = blast::PrimitiveTopology::PRIMITIVE_TOPO_TRI_LIST;
    };

    class CMesh : public Component {
    public:
        CMesh(Entity* entity);

        virtual ~CMesh();

        static ComponentType GetClassType() { return ComponentType::Mesh; }

        ComponentType GetType() override { return ComponentType::Mesh; }

        void ResetSubMeshs();

        void AddSubMesh(const SubMesh& sub_mesh);

        const std::vector<SubMesh>& GetSubMeshs() {
            return _sub_meshs;
        }

        void SetSkeleton(Skeleton* skeleton) {
            this->skeleton = skeleton;
        }

        void SetCastShadow(bool castShadow);

        void SetReceiveShadow(bool receiveShadow);

        RenderableType GetRenderableType() { return _renderable_type; }

        void SetRenderableType(RenderableType type) { _renderable_type = type; }

    private:
        friend class Scene;
        RenderableType _renderable_type = RENDERABLE_COMMON;
        std::vector<SubMesh> _sub_meshs;
        Skeleton* skeleton = nullptr;
    };
}