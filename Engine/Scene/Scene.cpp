#include "Scene.h"
#include "Entity.h"

namespace gear {
    Scene::Scene() {}

    Scene::~Scene() {}

    Entity* Scene::createEntity() {
        Entity* entity = new Entity();
        mEntities.push_back(entity);
        return entity;
    }

    void Scene::destroyEntity(Entity* entity) {
        for (auto iter = mEntities.begin(); iter != mEntities.end();) {
            if (iter == mEntities.end())
                break;
            if (*iter == entity) {
                mEntities.erase(iter);
                delete entity;
                entity = nullptr;
                break;
            }
        }
    }
}
EFFECTS_NAMESPACE_BEGIN

SceneNode::SceneNode() {
}

SceneNode::~SceneNode() {
}

SceneNode* SceneNode::create(const std::string& id) {
    SceneNode* node = Scene::instance().addNode();
    node->mId = id;
    node->mUUID = generateUUID();
    node->mParent = nullptr;
    node->mChildren.clear();
    node->mComponents.clear();
    node->mLPos = glm::vec3(0,0,0);
    node->mLScale = glm::vec3(1,1,1);
    node->mLRot = glm::quat(1,0,0,0);
    node->mLEuler = glm::vec3(0.0);
    node->mDirtyFlag = true;
    return node;
}

void SceneNode::destroy(SceneNode* node) {
    node->destroyInternal();
}

void SceneNode::destroyInternal() {
    setParent(nullptr);

    for (int i = 0; i < mChildren.size(); ++i) {
        mChildren[i]->destroyInternal();
    }
    mChildren.clear();

    for (int i = 0; i < mComponents.size(); ++i) {
        SAFE_DELETE(mComponents[i]);
    }
    mComponents.clear();

    Scene::instance().removeNode(this);
}

void SceneNode::initialized() {
}

void SceneNode::update(float dt) {
    for (int i = 0; i < mComponents.size(); ++i) {
        if (mDirtyFlag) {
            mComponents[i]->onNodeDirty();
        }
        mComponents[i]->update(dt);
    }
    if (mDirtyFlag) {
        mDirtyFlag = false;
    }
}

void SceneNode::setParent(SceneNode* newParent) {
    SceneNode* oldParent = mParent;

    if (oldParent == newParent)
        return;

    SceneNode* curNode = newParent;
    while (curNode) {
        if (curNode == this)
            return;
        curNode = curNode->getParent();
    }

    if (oldParent) {
        for (auto iter = oldParent->mChildren.begin(); iter != oldParent->mChildren.end(); ) {
            if (iter == oldParent->mChildren.end()) {
                break;
            }

            if (*iter == this) {
                oldParent->mChildren.erase(iter);
                break;
            }
        }
    }

    mParent = newParent;
    if (newParent) {
        newParent->mChildren.push_back(this);
    }
    mDirtyFlag = true;
}

void SceneNode::appendChildren(SceneNode* newChildren) {
    if (!newChildren)
        return;

    SceneNode* curNode = this;
    while (curNode)
    {
        if (curNode == newChildren)
            return;
        curNode = curNode->getParent();
    }

    SceneNode* oldParent = newChildren->mParent;
    if (oldParent) {
        for (auto iter = oldParent->mChildren.begin(); iter != oldParent->mChildren.end(); ) {
            if (iter == oldParent->mChildren.end()) {
                break;
            }

            if (*iter == newChildren) {
                oldParent->mChildren.erase(iter);
                break;
            }
        }
    }

    newChildren->mParent = this;
    newChildren->mDirtyFlag = true;
    mChildren.push_back(newChildren);
}

void SceneNode::removeChildren(SceneNode* children) {
    for (auto iter = mChildren.begin(); iter != mChildren.end(); ) {
        if (iter == mChildren.end())
            break;

        if (*iter == children) {
            mChildren.erase(iter);
            children->mParent = nullptr;
            children->mDirtyFlag = true;
        }
    }
}

void SceneNode::setTransform(const glm::vec3 &pos, const glm::vec3 &scale, const glm::quat &rot) {
    mLPos = pos;
    mLScale = scale;
    mLRot = rot;
    mDirtyFlag = true;
}

void SceneNode::setPosition(const glm::vec3& pos) {
    mLPos = pos;
    setDirty();
}

void SceneNode::setRotation(const glm::quat& rot) {
    mLRot = rot;
    setDirty();
}

void SceneNode::setScale(const glm::vec3& scale) {
    mLScale = scale;
    setDirty();
}

void SceneNode::rotate(const glm::vec3 axis, const float &angle) {
    glm::quat r = fromAxisAngle(axis, glm::radians(angle));
    mLRot = mLRot * r;
    setDirty();
}

glm::mat4 SceneNode::getLocalMatrix() {
    //RTS
    glm::mat4 R, T, S;
    R = glm::toMat4(mLRot);
    T = glm::translate(glm::mat4(1.0),mLPos);
    S = glm::scale(glm::mat4(1.0),mLScale);
    mLocalMatrix = T * R * S;
    return mLocalMatrix;
}

glm::mat4 SceneNode::getWorldMatrix() {
    glm::mat4 out = getLocalMatrix();
    SceneNode* cur = mParent;
    while (cur) {
        out = cur->getLocalMatrix() * out;
        cur = cur->mParent;
    }
    mWorldMatrix = out;
    return mWorldMatrix;
}

glm::vec3 SceneNode::getRightVector() {
    return getAxisX(getWorldMatrix());
}

glm::vec3 SceneNode::getUpVector() {
    return getAxisY(getWorldMatrix());
}

glm::vec3 SceneNode::getFrontVector() {
    return getAxisZ(getWorldMatrix());
}

void SceneNode::setEuler(const glm::vec3& euler) {
    mLEuler = euler;
    setRotation(glm::quat(mLEuler));
}


Scene::Scene(){
}

Scene::~Scene(){
    for (int i = 0; i < mNodes.size(); ++i) {
        SAFE_DELETE(mNodes[i]);
    }
    mNodes.clear();
}

SceneNode* Scene::addNode() {
    SceneNode* node = new SceneNode();
    mNodes.push_back(node);
    return node;
}

void Scene::removeNode(SceneNode* node) {
    for (auto iter = mNodes.begin(); iter != mNodes.end(); ++iter) {
        if (iter == mNodes.end()) {
            break;
        }

        if (*iter == node) {
            mNodes.erase(iter);
            break;
        }
    }

    SAFE_DELETE(node);
}

void Scene::update(float dt) {
    for (int i = 0; i < mNodes.size(); ++i) {
        mNodes[i]->update(dt);
    }
}

EFFECTS_NAMESPACE_END