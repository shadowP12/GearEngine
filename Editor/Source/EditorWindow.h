#pragma once

#include <string>

class EditorWindow {
public:
    EditorWindow(const std::string& id = "Default") {
        this->id = id;
    }

    ~EditorWindow() = default;

    std::string GetID() { return id; }

    virtual std::string GetType() { return "EditorWindow"; }

    virtual void Draw() = 0;

protected:
    std::string id;
};