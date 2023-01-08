#pragma once

#include <string>

class EditorWindow {
public:
    EditorWindow() = default;

    ~EditorWindow() = default;

    virtual std::string GetID() { return "EditorWindow"; }

    virtual void Draw() = 0;
};