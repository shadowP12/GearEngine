#pragma once
#include "Components/Component.h"
#include <vector>

namespace gear {
    class Component;
    class Entity {
    public:
        Entity();
        ~Entity();

        template<class T, class... Args>
        T* addComponent(Args &&... args) {
            T* component = new T(this, std::forward<Args>(args)...);
            mComponents.push_back(component);
            return component;
        }

        template<class T>
        void removeComponent() {
            T* component = getComponent<T>();
            if (component == nullptr) {
                return;
            }

            for (auto iter = mComponents.begin(); iter != mComponents.end();) {
                if (iter == mComponents.end())
                    break;
                if (*iter == component) {
                    mComponents.erase(iter);
                    delete component;
                    component = nullptr;
                    break;
                }
            }
        }

        template<class T>
        T* getComponent() {
            for (int i = 0; i < mComponents.size(); ++i) {
                if(mComponents[i]->getType() == T::getClassType()) {
                    return (T*)mComponents[i];
                }
            }
            return nullptr;
        }
    protected:
        friend class Scene;
        std::vector<Component*> mComponents;
    };
}