#pragma once

#include <Entity/Scene.h>

class TestScene {
public:
    TestScene() = default;

    virtual ~TestScene() = default;

    virtual void Load() = 0;

    virtual void Clear() = 0;

    virtual void DrawUI() = 0;

    virtual std::shared_ptr<gear::Scene> GetScene() = 0;
};
