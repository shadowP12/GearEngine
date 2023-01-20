#pragma once
#include <Utility/Singleton.h>
#include <string>

class EditorSettings : gear::Singleton<EditorSettings> {
public:
    void LoadSettings();

    void SaveSettings();

    std::string GetProjectDir();

    void SetProjectDir(const std::string& dir);

private:
    std::string project_dir;
};