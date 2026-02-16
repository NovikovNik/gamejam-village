#include "FileSystem.h"
#include <Utils/Singleton.h>
#include <Logger/Logger.h>
#include <filesystem>
#include <cstdlib>

class FileSystem : public Singleton<FileSystem>
{
public:
    void OpenSystemExplorer(const std::filesystem::path& relativePath) {
        auto fullPath = executableDirPath / relativePath;
        Logger::Log(fullPath);

        if (!std::filesystem::exists(fullPath))
            return;

#ifdef _WIN32
        std::string cmd = "explorer \"" + fullPath.string() + "\"";
#elif __APPLE__
        std::string cmd = "open \"" + fullPath.string() + "\"";
#else
        std::string cmd = "xdg-open \"" + fullPath.string() + "\"";
#endif
        std::system(cmd.c_str());
    }

    void SetExecutableDir(char* argv0) {
        executableDirPath = std::filesystem::absolute(argv0).parent_path();
    }

private:
    [[maybe_unused]] std::filesystem::path GetWorkingDirectory() const {
        return std::filesystem::current_path();
    }

    private:
        std::filesystem::path executableDirPath;
};

void FileSystemManager::OpenSystemExplorer(const std::string& path) {
    FileSystem::instance().OpenSystemExplorer(path);
}

void FileSystemManager::SetExecutableDir(char* argv0) {
    FileSystem::instance().SetExecutableDir(argv0);
}
