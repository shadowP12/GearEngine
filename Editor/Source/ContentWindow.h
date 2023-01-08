#pragma once

#include "EditorWindow.h"

class ContentWindow : public EditorWindow {
public:
    ContentWindow();

    ~ContentWindow();

    std::string GetID() override { return "ContentWindow"; }

    void Draw() override;
};
