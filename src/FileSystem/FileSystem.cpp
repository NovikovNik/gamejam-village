#include "FileSystem.h"
#include <Utils/Singleton.h>
#include <Logger/Logger.h>
#include <filesystem>
#include <cstdlib>
#include <fstream>

#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#undef CreateDirectory
#endif

class FileSystem : public Singleton<FileSystem>
{
public:
    void OpenSystemExplorer(const std::filesystem::path& relativePath) {
#if defined(__APPLE__)
        // На macOS открываем папки рядом с .app (на уровне build/Debug),
        // а не внутри Contents/MacOS.
        std::filesystem::path root = executableDirPath / "../../..";
#else
        // На Windows/Linux используем директорию исполняемого файла.
        std::filesystem::path root = executableDirPath;
#endif
        auto fullPath = root / relativePath;

        if (!std::filesystem::exists(fullPath)) {
            std::filesystem::create_directories(fullPath);
        }

#ifdef _WIN32
        // Open folder via ShellExecuteW to avoid slow std::system/console startup
        const auto widePath = fullPath.wstring();
        HINSTANCE result = ShellExecuteW(
            nullptr,
            L"open",
            widePath.c_str(),
            nullptr,
            nullptr,
            SW_SHOWNORMAL
        );
        if (reinterpret_cast<UINT_PTR>(result) <= 32) {
            Logger::Err("[FS] ShellExecuteW failed for path: " + fullPath.string());
        }
#elif __APPLE__
        std::string cmd = "open \"" + fullPath.string() + "\"";
#else
        std::string cmd = "xdg-open \"" + fullPath.string() + "\"";
#endif

#if !defined(_WIN32)
        std::system(cmd.c_str());
#endif

        Logger::Debug("[FS] Folder opened: " + fullPath.string());
    }

    void CreateDirectory(const std::string& path) {
        auto fullPath = executableDirPath / path;
        if (!std::filesystem::exists(fullPath)) {
            std::filesystem::create_directories(fullPath);
        }
    }

    void CreateKeyFile(const std::string& dirPath, const std::string& filename) {
#if defined(__APPLE__)
        // Для относительных путей создаём файлы рядом с .app;
        // для абсолютных (dirPath как полный путь) поведение std::filesystem::operator/
        // оставит только абсолютный путь и проигнорирует root.
        std::filesystem::path root = executableDirPath / "../../..";
#else
        std::filesystem::path root = executableDirPath;
#endif
        auto fullDir = root / dirPath;
        if (!std::filesystem::exists(fullDir)) {
            std::filesystem::create_directories(fullDir);
        }
        auto filePath = fullDir / filename;
        if (!std::filesystem::exists(filePath)) {
            std::ofstream file(filePath);
            if (file) {
                Logger::Debug("[FS] Created file: " + filePath.string());
            } else {
                Logger::Debug("[FS] Failed to create file: " + filePath.string());
            }
        } else {
            Logger::Debug("[FS] File already exists: " + filePath.string());
        }
    }

    void DeleteKeyFile(const std::string& dirPath, const std::string& filename) {
#if defined(__APPLE__)
        // Аналогично CreateKeyFile: относительные пути трактуем как внешние к .app.
        std::filesystem::path root = executableDirPath / "../../..";
#else
        std::filesystem::path root = executableDirPath;
#endif
        auto fullDir = root / dirPath;
        if (!std::filesystem::exists(fullDir)) {
            return;
        }
        auto filePath = fullDir / filename;
        if (std::filesystem::exists(filePath)) {
            std::filesystem::remove(filePath);
            Logger::Debug("[FS] Deleted file: " + filePath.string());
        } else {
            Logger::Debug("[FS] File does not exist: " + filePath.string());
        }
    }

    void SetExecutableDir(char* argv0) {
        executableDirPath = std::filesystem::absolute(argv0).parent_path();
    }

    std::filesystem::path GetExecutableDir() const {
        return executableDirPath;
    }

    std::filesystem::path GetAssetsBaseDir() const {
#if defined(__APPLE__)
        // В бандле AAAB.app исполняемый файл лежит в Contents/MacOS,
        // ассеты — в Contents/Resources/Assets.
        return executableDirPath / "../Resources/Assets";
#else
        // Для Windows/Linux ассеты лежат рядом с exe в папке assets.
        return executableDirPath / "assets";
#endif
    }

    std::filesystem::path GetAssetsSubDir(const std::string& subDir) const {
        return GetAssetsBaseDir() / subDir;
    }

    std::filesystem::path GetVillageDir() const {
#if defined(__APPLE__)
        // exe: <path>/AAAB.app/Contents/MacOS
        // village: рядом с AAAB.app → <path>/village
        std::filesystem::path root = executableDirPath / "../../..";
#else
        // На Windows/Linux village лежит рядом с exe
        std::filesystem::path root = executableDirPath;
#endif
        return (root / "village").lexically_normal();
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

void FileSystemManager::CreateDirectory(const std::string& path) {
    FileSystem::instance().CreateDirectory(path);
}

void FileSystemManager::CreateKeyFile(const std::string& dirPath, const std::string& filename) {
    FileSystem::instance().CreateKeyFile(dirPath, filename);
}

void FileSystemManager::DeleteKeyFile(const std::string& dirPath, const std::string& filename) {
    FileSystem::instance().DeleteKeyFile(dirPath, filename);
}

std::filesystem::path FileSystemManager::GetExecutableDir() {
    return FileSystem::instance().GetExecutableDir();
}

std::filesystem::path FileSystemManager::GetAssetsBaseDir() {
    return FileSystem::instance().GetAssetsBaseDir();
}

std::filesystem::path FileSystemManager::GetAssetsSubDir(const std::string& subDir) {
    return FileSystem::instance().GetAssetsSubDir(subDir);
}

std::filesystem::path FileSystemManager::GetVillageDir() {
    return FileSystem::instance().GetVillageDir();
}
