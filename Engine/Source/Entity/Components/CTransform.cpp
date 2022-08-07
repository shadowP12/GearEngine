#include "CTransform.h"
#include "../Entity.h"

namespace gear {
    CTransform::CTransform(Entity* entity)
            :Component(entity) {
    }

    CTransform::~CTransform() {
        SetParent(nullptr);

        std::vector<Entity*> remove_children = _children;
        for (int i = 0; i < remove_children.size(); ++i) {
            remove_children[i]->GetComponent<CTransform>()->SetParent(nullptr);
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

    void CTransform::SetPosition(const glm::vec3& pos) {
        _translation = pos;
        UpdateLocalTransform();
        UpdateTransform();
    }

    void CTransform::SetScale(const glm::vec3& scale) {
        _scale = scale;
        UpdateLocalTransform();
        UpdateTransform();
    }

    void CTransform::SetEuler(const glm::vec3& euler) {
        _euler = euler;
        _rot = glm::quat(euler);
        UpdateLocalTransform();
        UpdateTransform();
    }

    void CTransform::SetTransform(const glm::mat4& local_transform) {
        glm::vec3 skew;
        glm::vec4 perspective;
        glm::decompose(local_transform, _scale, _rot, _translation, skew,perspective);
        _euler = glm::eulerAngles(_rot) * 3.14159f / 180.f;

        _local = local_transform;
        UpdateTransform();
    }

    glm::mat4 CTransform::GetTransform() {
        return _local;
    }

    glm::mat4 CTransform::GetWorldTransform() {
        return _world;
    }

    glm::vec3 CTransform::GetRightVector() {
        return GetAxisX(_world);
    }

    glm::vec3 CTransform::GetUpVector() {
        return GetAxisY(_world);
    }

    glm::vec3 CTransform::GetFrontVector() {
        return GetAxisZ(_world);
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

    void CTransform::UpdateLocalTransform() {
        glm::mat4 r, t, s;
        r = glm::toMat4(_rot);
        t = glm::translate(glm::mat4(1.0), _translation);
        s = glm::scale(glm::mat4(1.0), _scale);
        _local = t * r * s;
    }
}