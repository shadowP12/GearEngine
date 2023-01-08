#pragma once

#include "EditorWindow.h"

class SceneWindow : public EditorWindow {
public:
    SceneWindow();

    ~SceneWindow();

    std::string GetID() override { return "SceneWindow"; }

    void Draw() override;
};