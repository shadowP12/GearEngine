#include "Scene/Components/Component.h"
#include "Scene/Scene.h"
EFFECTS_NAMESPACE_BEGIN

Component::Component(SceneNode* node) {
    mType = ComponentType::None;
    mNode = node;
}

Component::~Component()
{}

EFFECTS_NAMESPACE_END