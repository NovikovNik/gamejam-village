#pragma once

#include <Entities/EntitiesManager.h>

namespace FileSystemManager {
    void OpenSystemExplorer(const std::string& path);

    void SetExecutableDir(char* argv0);
    void CreateDirectory(const std::string& path);
    void CreateKeyFile(const std::string& dirPath, const std::string& filename);
};
