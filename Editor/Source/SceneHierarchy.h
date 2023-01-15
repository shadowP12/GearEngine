#pragma once

#include "EditorWindow.h"

class SceneHierarchy : public EditorWindow {
public:
    SceneHierarchy(const std::string& id);

    ~SceneHierarchy();

    std::string GetType() override { return "SceneHierarchy"; }

    void Draw() override;
};