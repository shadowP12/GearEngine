#pragma once
#include "Component.h"
#include "Math/Math.h"
#include "Math/Geometry.h"
#include "Renderer/RenderData.h"
#include <GfxDefine.h>
#include <vector>

namespace gear {
    class Entity;
    class Mesh;
    class MaterialInstance;
    class Skeleton;

    struct SubMesh {

    };

    class CMesh : public Component {
    public:
        CMesh(Entity* entity);

        virtual ~CMesh();

        static ComponentType GetClassType() { return ComponentType::Mesh; }

        ComponentType GetType() override { return ComponentType::Mesh; }

        void SetMesh(std::shared_ptr<Mesh> mesh);

        void SetMaterial(uint32_t idx, std::shared_ptr<MaterialInstance> material);

        std::shared_ptr<MaterialInstance> GetMaterial(uint32_t idx);

        void SetMaterials(std::vector<std::shared_ptr<MaterialInstance>> materials);

        std::vector<std::shared_ptr<MaterialInstance>> GetMaterials();

        void SetSkeleton(std::shared_ptr<Skeleton> skeleton) {
            this->skeleton = skeleton;
        }

        std::shared_ptr<Skeleton> GetSkeleton() {
            return this->skeleton;
        }

        void SetCastShadow(bool castShadow);

        void SetReceiveShadow(bool receiveShadow);

        void SetBoundBox(BBox bbox);

        BBox GetBoundBox();

    private:
        friend class Scene;
        bool cast_shadow = false;
        bool receive_shadow = true;
        BBox bbox;
        std::shared_ptr<Mesh> mesh = nullptr;
        std::vector<std::shared_ptr<MaterialInstance>> material_list;
        std::shared_ptr<Skeleton> skeleton = nullptr;
    };
}