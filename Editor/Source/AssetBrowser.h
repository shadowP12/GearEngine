#pragma once

#include "EditorWindow.h"

class AssetBrowser : public EditorWindow {
public:
    AssetBrowser();

    ~AssetBrowser();

    std::string GetType() override { return "AssetBrowser"; }

    void Draw() override;
};