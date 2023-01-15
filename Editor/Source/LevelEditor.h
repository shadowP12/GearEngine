#pragma once

#include "EditorWindow.h"
#include "Inspector.h"
#include "AssetBrowser.h"
#include "SceneViewer.h"
#include "SceneHierarchy.h"
#include <memory>

class LevelEditor : public EditorWindow {
public:
    LevelEditor();

    ~LevelEditor();

    std::string GetType() override { return "LevelEditor"; }

    void Draw() override;

private:
    std::shared_ptr<Inspector> inspector;
    std::shared_ptr<AssetBrowser> asset_browser;
    std::shared_ptr<SceneViewer> scene_viewer;
    std::shared_ptr<SceneHierarchy> scene_hierarchy;
};