#pragma once

#include <Entities/EntitiesManager.h>

namespace FileSystemManager {
    void OpenSystemExplorer(const std::string& path);
    void SetExecutableDir(char* argv0);
};
