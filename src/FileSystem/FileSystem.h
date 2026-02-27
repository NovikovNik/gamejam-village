#pragma once

#include <Entities/EntitiesManager.h>
#include <filesystem>

namespace FileSystemManager {
    void OpenSystemExplorer(const std::string& path);

    void SetExecutableDir(char* argv0);
    void CreateDirectory(const std::string& path);
    void CreateKeyFile(const std::string& dirPath, const std::string& filename);
    void DeleteKeyFile(const std::string& dirPath, const std::string& filename);

    [[nodiscard]] std::filesystem::path GetExecutableDir();

    // Возвращает базовый путь до Assets с учётом платформы и .app bundle на macOS.
    [[nodiscard]] std::filesystem::path GetAssetsBaseDir();

    // Удобный хелпер: Assets/<subDir> (например "dialogs", "audio", "textures", "fonts").
    [[nodiscard]] std::filesystem::path GetAssetsSubDir(const std::string& subDir);

    // Путь до папки village (на macOS — рядом с .app, на остальных платформах — рядом с exe).
    [[nodiscard]] std::filesystem::path GetVillageDir();
};
