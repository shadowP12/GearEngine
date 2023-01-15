#pragma once

#include "EditorWindow.h"

class SceneViewer : public EditorWindow {
public:
    SceneViewer(const std::string& id);

    ~SceneViewer();

    std::string GetType() override { return "SceneViewer"; }

    void Draw() override;
};