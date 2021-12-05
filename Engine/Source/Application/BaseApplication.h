#pragma once
#include "Core/GearDefine.h"

#include <vector>

namespace gear {
    class BaseApplication {
    public:
        BaseApplication();

        ~BaseApplication();

        void Run(float dt);

        virtual bool ShouldClose() { return true; };

        virtual void Init() {};

        virtual void Exit() {};

    protected:
        virtual void Tick(float dt) {};
    };
}
