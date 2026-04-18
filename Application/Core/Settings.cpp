#include "Core/PCH.h"
#include "Core/Settings.h"

#include <nlohmann/json.hpp>
#include <filesystem>
#include <fstream>

namespace BA
{

namespace
{

using json = nlohmann::json;

std::filesystem::path GetSettingsFilePath()
{
    wchar_t exePath[MAX_PATH] = {};
    GetModuleFileNameW(nullptr, exePath, MAX_PATH);
    return std::filesystem::path(exePath).parent_path() / "settings.json";
}

WindowSettings ReadWindowSettings(const json& j)
{
    WindowSettings settings;

    if (j.contains("window") && j["window"].is_object())
    {
        const json& w = j["window"];
        settings.positionX = w.value("positionX", settings.positionX);
        settings.positionY = w.value("positionY", settings.positionY);
        settings.clientWidth = w.value("clientWidth", settings.clientWidth);
        settings.clientHeight = w.value("clientHeight", settings.clientHeight);
        settings.isMaximized = w.value("isMaximized", settings.isMaximized);
    }

    return settings;
}

#ifdef BA_EDITOR
EditorSettings ReadEditorSettings(const json& j)
{
    EditorSettings settings;

    if (j.contains("editor") && j["editor"].is_object())
    {
        const json& e = j["editor"];
        settings.consoleFilterLevel = static_cast<LogLevel>(
            e.value("consoleFilterLevel", static_cast<int>(settings.consoleFilterLevel)));
        settings.isConsoleAutoScroll = e.value("isConsoleAutoScroll", settings.isConsoleAutoScroll);
    }

    return settings;
}
#endif // BA_EDITOR

json WriteWindowSettings(const WindowSettings& settings)
{
    return json{
        {"positionX", settings.positionX},
        {"positionY", settings.positionY},
        {"clientWidth", settings.clientWidth},
        {"clientHeight", settings.clientHeight},
        {"isMaximized", settings.isMaximized}
    };
}

#ifdef BA_EDITOR
json WriteEditorSettings(const EditorSettings& settings)
{
    return json{
        {"consoleFilterLevel", static_cast<int>(settings.consoleFilterLevel)},
        {"isConsoleAutoScroll", settings.isConsoleAutoScroll}
    };
}
#endif // BA_EDITOR

} // namespace

AppSettings LoadSettings()
{
    AppSettings appSettings;

    std::filesystem::path filePath = GetSettingsFilePath();

    std::ifstream file(filePath);
    if (!file.is_open())
    {
        BA_LOG_INFO("No settings file found. Using defaults.");
        return appSettings;
    }

    json j = json::parse(file, nullptr, false);
    if (j.is_discarded())
    {
        BA_LOG_WARN("Failed to parse settings file. Using defaults.");
        return appSettings;
    }

    appSettings.window = ReadWindowSettings(j);
#ifdef BA_EDITOR
    appSettings.editor = ReadEditorSettings(j);
#endif // BA_EDITOR

    BA_LOG_INFO("Settings loaded from file.");
    return appSettings;
}

void SaveSettings(const AppSettings& settings)
{
    json j;
    j["window"] = WriteWindowSettings(settings.window);
#ifdef BA_EDITOR
    j["editor"] = WriteEditorSettings(settings.editor);
#endif // BA_EDITOR

    std::filesystem::path filePath = GetSettingsFilePath();

    std::ofstream file(filePath);
    if (!file.is_open())
    {
        BA_LOG_WARN("Failed to open settings file for writing.");
        return;
    }

    file << j.dump(4);
    BA_LOG_INFO("Settings saved to file.");
}

} // namespace BA
