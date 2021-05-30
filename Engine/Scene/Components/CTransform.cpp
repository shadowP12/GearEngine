#include "CTransform.h"
#include "Scene/Entity.h"

namespace gear {
    CTransform::CTransform(Entity* entity)
            :Component(entity) {
    }

    CTransform::~CTransform() {
        setParent(nullptr);

        for (int i = 0; i < mChildren.size(); ++i) {
            mChildren[i]->getComponent<CTransform>()->setParent(nullptr);
        }
        mChildren.clear();
    }

    void CTransform::setParent(Entity* newParent) {
        Entity* oldParent = mParent;

        if (oldParent == newParent)
            return;

        if (oldParent) {
            CTransform* oldParentTransform = oldParent->getComponent<CTransform>();
            for (auto iter = oldParentTransform->mChildren.begin(); iter != oldParentTransform->mChildren.end();) {
                if (iter == oldParentTransform->mChildren.end()) {
                    break;
                }

                if (*iter == this->mEntity) {
                    oldParentTransform->mChildren.erase(iter);
                    break;
                }
            }
        }

        mParent = newParent;
        if (newParent) {
            CTransform* newParentTransform = newParent->getComponent<CTransform>();
            newParentTransform->mChildren.push_back(this->mEntity);
        }

        updateTransform();
    }

    Entity* CTransform::getParent() {
        return mParent;
    }

    const std::vector<Entity*>& CTransform::getChildren() {
        return mChildren;
    }

    void CTransform::setTransform(const glm::mat4& localTransform) {
        mLocal = localTransform;
        updateTransform();
    }

    const glm::mat4& CTransform::getTransform() {
        return mLocal;
    }

    const glm::mat4& CTransform::getWorldTransform() {
        return mWorld;
    }

    void CTransform::updateTransform() {
        if (mParent) {
            mWorld = mParent->getComponent<CTransform>()->mWorld * mLocal;
        }

        for (int i = 0; i < mChildren.size(); ++i) {
            mChildren[i]->getComponent<CTransform>()->updateTransform();
        }
    }
}