#pragma once

#include <Entities/EntitiesManager.h>
#include <filesystem>

namespace FileSystemManager {
    void OpenSystemExplorer(const std::string& path);

    void SetExecutableDir(char* argv0);
    void CreateDirectory(const std::string& path);
    void CreateKeyFile(const std::string& dirPath, const std::string& filename);

    [[nodiscard]] std::filesystem::path GetExecutableDir();
};
