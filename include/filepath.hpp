
#include <filesystem>
#include <iostream>
#include <optional>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

std::string GetExecutablePath() {
#ifdef _WIN32
    // Buffer to hold the executable path
    TCHAR buffer[MAX_PATH];
    // Get the full path to the executable
    GetModuleFileName(nullptr, buffer, MAX_PATH);

    return std::filesystem::path(buffer).string();
#else
    // For non-Windows platforms, use the Linux approach
    return std::filesystem::canonical("/proc/self/exe").string();
#endif
}

std::optional<std::string> GetAssetsPath(const std::string& relativePath) {
    // Get the directory containing the executable
    std::filesystem::path executablePath = GetExecutablePath();

    // Check if the executable path is empty (indicating an error)
    if (executablePath.empty()) {
        std::cerr << "Failed to determine executable path." << std::endl;
        return std::nullopt;  // Return an empty optional
    }

    // Construct the absolute path to the assets folder
    std::filesystem::path assetsPath = executablePath.parent_path().parent_path() / "assets" / relativePath;

    // Check if the assets path exists
    if (!std::filesystem::exists(assetsPath)) {
        std::cerr << "Assets path does not exist: " << assetsPath << std::endl;
        return std::nullopt;  // Return an empty optional
    }

    return assetsPath.string();
}