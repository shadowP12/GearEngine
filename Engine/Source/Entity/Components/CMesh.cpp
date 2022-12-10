#include "CMesh.h"
#include "CTransform.h"
#include "Entity/Entity.h"
#include "GearEngine.h"
#include "Renderer/Renderer.h"
#include "Resource/Mesh.h"

namespace gear {
    CMesh::CMesh(Entity* entity)
            :Component(entity) {
    }

    CMesh::~CMesh() {
    }

    void CMesh::SetMesh(std::shared_ptr<Mesh> mesh) {
        // Clear material list
        material_list.clear();
        material_list.resize(mesh->GetSubMeshs().size());
        this->mesh = mesh;
    }

    void CMesh::SetMaterial(uint32_t idx, std::shared_ptr<MaterialInstance> material) {
        if (idx >= material_list.size()) {
            return;
        }
        material_list[idx] = material;
    }

    std::shared_ptr<MaterialInstance> CMesh::GetMaterial(uint32_t idx) {
        if (idx >= material_list.size()) {
            return nullptr;
        }
        return material_list[idx];
    }

    void CMesh::SetMaterials(std::vector<std::shared_ptr<MaterialInstance>> materials) {
        if (material_list.size() != materials.size()) {
            return;
        }
        material_list = materials;
    }

    std::vector<std::shared_ptr<MaterialInstance>> CMesh::GetMaterials() {
        return material_list;
    }

    void CMesh::SetCastShadow(bool cast_shadow) {
        this->cast_shadow = cast_shadow;
    }

    void CMesh::SetReceiveShadow(bool receive_shadow) {
        this->receive_shadow = receive_shadow;
    }

    void CMesh::SetBoundBox(gear::BBox bbox) {
        this->bbox = bbox;
    }

    BBox CMesh::GetBoundBox() {
        return this->bbox;
    }
}