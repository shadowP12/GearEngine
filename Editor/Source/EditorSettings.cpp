#include "EditorSettings.h"
#include "EditorMisc.h"
#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>

void EditorSettings::LoadSettings() {

}

void EditorSettings::SaveSettings() {
}

std::string EditorSettings::GetProjectDir() {
    return project_dir;
}

void EditorSettings::SetProjectDir(const std::string &dir) {
    project_dir = dir;
}
