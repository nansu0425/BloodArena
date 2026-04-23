#pragma once

#include "Core/Logger.h"
#include "Graphics/Gizmo/Gizmo.h"

namespace BA
{

constexpr size_t kEditorConsoleInputBufferSize = 256;
constexpr size_t kEditorSceneNameBufferSize = 64;

struct EditorSettings
{
    LogLevel consoleFilterLevel = LogLevel::Trace;
    bool isConsoleAutoScroll = true;
};

struct ConsoleEntry
{
    std::string message;
    LogLevel level;
};

class EditorState
{
public:
    void Initialize();
    void Shutdown();

    uint32_t GetSelectedGameObjectId() const;
    void SetSelectedGameObjectId(uint32_t id);

    void AddConsoleEntry(std::string message, LogLevel level);
    void ClearConsole();
    const std::vector<ConsoleEntry>& GetConsoleEntries() const;

    LogLevel GetConsoleFilterLevel() const;
    void SetConsoleFilterLevel(LogLevel level);

    bool GetConsoleAutoScroll() const;
    void SetConsoleAutoScroll(bool autoScroll);

    EditorSettings GetEditorSettings() const;
    void SetEditorSettings(const EditorSettings& settings);

    char* GetConsoleInputBuffer();
    void ClearConsoleInputBuffer();

    char* GetSceneNameBuffer();
    void ClearSceneNameBuffer();
    void SetSceneNameBuffer(const char* name);

    bool IsViewportFlying() const;
    void SetViewportFlying(bool isFlying);

    Gizmo::Mode GetGizmoMode() const;
    void SetGizmoMode(Gizmo::Mode mode);
    Gizmo::Space GetGizmoSpace() const;
    void SetGizmoSpace(Gizmo::Space space);

private:
    uint32_t m_selectedGameObjectId = 0;

    std::vector<ConsoleEntry> m_consoleEntries;
    LogLevel m_consoleFilterLevel = LogLevel::Trace;
    bool m_consoleAutoScroll = true;

    char m_consoleInputBuffer[kEditorConsoleInputBufferSize] = {};
    char m_sceneNameBuffer[kEditorSceneNameBufferSize] = {};
    bool m_isViewportFlying = false;
    Gizmo::Mode m_gizmoMode = Gizmo::Mode::Translate;
    Gizmo::Space m_gizmoSpace = Gizmo::Space::World;
};

extern std::unique_ptr<EditorState> g_editorState;

} // namespace BA
