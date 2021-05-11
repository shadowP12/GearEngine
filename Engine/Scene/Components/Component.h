#pragma once
#include <memory>
namespace gear {
    enum class ComponentType {
        None,
        Transform,
        Renderable,
        Camera,
        Light
    };

    class Entity;

    class Component
    {
    public:
        Component(Entity* node);
        ~Component();
        virtual void initialized() {}
        virtual void update(float dt) {}
        virtual void destroyed() {}
        virtual void onNodeDirty() {}
        static ComponentType getClassType() { return ComponentType::None; }
        virtual ComponentType getType() { return mType; }

    protected:
        Entity* mEntity;
        ComponentType mType;
    };
}