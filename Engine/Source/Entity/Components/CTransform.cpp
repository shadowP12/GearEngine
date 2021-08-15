#include "CTransform.h"
#include "../Entity.h"

namespace gear {
    CTransform::CTransform(Entity* entity)
            :Component(entity) {
    }

    CTransform::~CTransform() {
        SetParent(nullptr);

        for (int i = 0; i < _children.size(); ++i) {
            _children[i]->GetComponent<CTransform>()->SetParent(nullptr);
        }
        _children.clear();
    }

    void CTransform::SetParent(Entity* new_parent) {
        Entity* old_parent = _parent;

        if (old_parent == new_parent)
            return;

        if (old_parent) {
            CTransform* old_parent_transform = old_parent->GetComponent<CTransform>();
            for (auto iter = old_parent_transform->_children.begin(); iter != old_parent_transform->_children.end();) {
                if (iter == old_parent_transform->_children.end()) {
                    break;
                }

                if (*iter == this->_entity) {
                    old_parent_transform->_children.erase(iter);
                    break;
                }
            }
        }

        _parent = new_parent;
        if (new_parent) {
            CTransform* new_parent_transform = new_parent->GetComponent<CTransform>();
            new_parent_transform->_children.push_back(this->_entity);
        }

        UpdateTransform();
    }

    Entity* CTransform::GetParent() {
        return _parent;
    }

    const std::vector<Entity*>& CTransform::GetChildren() {
        return _children;
    }

    void CTransform::SetTransform(const glm::mat4& local_transform) {
        _local = local_transform;
        UpdateTransform();
    }

    const glm::mat4& CTransform::GetTransform() {
        return _local;
    }

    const glm::mat4& CTransform::GetWorldTransform() {
        return _world;
    }

    void CTransform::UpdateTransform() {
        _world = _local;
        if (_parent) {
            _world = _parent->GetComponent<CTransform>()->_world * _local;
        }

        for (int i = 0; i < _children.size(); ++i) {
            _children[i]->GetComponent<CTransform>()->UpdateTransform();
        }
    }
}