#pragma once
#include "Core/GearDefine.h"

namespace gear {
    template<class T>
    class Singleton {
    public:
        static T *Instance() {
            static T instance;
            return &instance;
        }

    private:
        Singleton(Singleton const &) {
        }

        Singleton &operator=(Singleton const &) {
            return *this;
        }

    protected:
        Singleton() {}

        ~Singleton() {}
    };
}