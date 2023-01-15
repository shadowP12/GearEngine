#pragma once

#include "EditorWindow.h"

class Inspector : public EditorWindow {
public:
    Inspector();

    ~Inspector();

    std::string GetType() override { return "Inspector"; }

    void Draw() override;
};