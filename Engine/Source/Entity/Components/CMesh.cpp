#include "CMesh.h"
#include "CTransform.h"
#include "Entity/Entity.h"
#include "Entity/EntityManager.h"
#include "GearEngine.h"
#include "Renderer/Renderer.h"
#include "Resource/GpuBuffer.h"

namespace gear {
    CMesh::CMesh(Entity* entity)
            :Component(entity) {
    }

    CMesh::~CMesh() {
    }

    void CMesh::ResetSubMeshs() {
        _sub_meshs.clear();
    }

    void CMesh::AddSubMesh(const SubMesh& sub_mesh) {
        _sub_meshs.push_back(sub_mesh);
    }

    void CMesh::SetCastShadow(bool cast_shadow) {
        for (int i = 0; i < _sub_meshs.size(); ++i) {
            _sub_meshs[i].cast_shadow = cast_shadow;
        }
    }

    void CMesh::SetReceiveShadow(bool receive_shadow) {
        for (int i = 0; i < _sub_meshs.size(); ++i) {
            _sub_meshs[i].receive_shadow = receive_shadow;
        }
    }

}