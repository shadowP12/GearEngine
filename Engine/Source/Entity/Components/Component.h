#pragma once
#include <memory>
namespace gear {
    enum class ComponentType {
        None,
        Transform,
        Renderable,
        Camera,
        Light,
        Mesh,
        Skybox,
        Animation,
        Atmosphere,
    };

    class Entity;

    class Component {
    public:
        Component(Entity*);

        virtual ~Component();

        virtual void Initialized() {}

        virtual void Update(float dt) {}

        virtual void Destroyed() {}

        virtual void OnNodeDirty() {}

        static ComponentType GetClassType() { return ComponentType::None; }

        virtual ComponentType GetType() { return _type; }

    protected:
        Entity* _entity;
        ComponentType _type;
    };
}