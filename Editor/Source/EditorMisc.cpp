#include "EditorMisc.h"
#include <filesystem/path.h>

namespace EditorMisc {
    const std::string GetEditorResourcesDir() {
        static std::string editor_resources_dir = "";
        if (editor_resources_dir == "") {
            filesystem::path root_path = filesystem::path::getcwd();
            editor_resources_dir = (root_path / "EditorResources").str(filesystem::path::path_type::posix_path);
        }
        return editor_resources_dir;
    }

    const std::string GetEngineResourcesDir() {
        static std::string engine_resources_dir = "";
        if (engine_resources_dir == "") {
            filesystem::path root_path = filesystem::path::getcwd();
            engine_resources_dir = (root_path / "EngineResources").str(filesystem::path::path_type::posix_path);
        }
        return engine_resources_dir;
    }
};