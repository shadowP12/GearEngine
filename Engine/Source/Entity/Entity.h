#pragma once
#include "Components/Component.h"
#include <vector>
#include <string>

namespace gear {
    class Component;
    class Entity : public std::enable_shared_from_this<Entity> {
    public:
        Entity(const std::string& name);
        
        ~Entity();

        static std::shared_ptr<Entity> Create(const std::string& name);

        std::string GetName();

        template<class T, class... Args>
        T* AddComponent(Args &&... args) {
            T* component = new T(this, std::forward<Args>(args)...);
            _components.push_back(component);
            return component;
        }

        template<class T>
        void RemoveComponent() {
            T* component = getComponent<T>();
            if (component == nullptr) {
                return;
            }

            for (auto iter = _components.begin(); iter != _components.end();) {
                if (iter == _components.end())
                    break;
                if (*iter == component) {
                    _components.erase(iter);
                    delete component;
                    component = nullptr;
                    break;
                }
            }
        }

        template<class T>
        T* GetComponent() {
            for (int i = 0; i < _components.size(); ++i) {
                if(_components[i]->GetType() == T::GetClassType()) {
                    return (T*)_components[i];
                }
            }
            return nullptr;
        }

        template<class T>
        bool HasComponent() {
            for (int i = 0; i < _components.size(); ++i) {
                if(_components[i]->GetType() == T::GetClassType()) {
                    return true;
                }
            }
            return false;
        }
        
    protected:
        friend class Scene;
        std::string _name;
        std::vector<Component*> _components;
    };
}