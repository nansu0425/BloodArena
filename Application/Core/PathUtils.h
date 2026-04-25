#pragma once

namespace BA
{

// Resolve a path relative to the Application root (project dir under debugger,
// executable directory in standalone runs).
inline std::wstring ResolveApplicationPath(const wchar_t* relativePath)
{
    if (IsDebuggerPresent())
    {
        return relativePath;
    }

    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(nullptr, exePath, MAX_PATH);

    std::wstring resolved(exePath);
    resolved = resolved.substr(0, resolved.rfind(L'\\') + 1);
    resolved += relativePath;

    return resolved;
}

inline std::wstring ResolveAssetPath(const wchar_t* relativePath)
{
    return ResolveApplicationPath(relativePath);
}

} // namespace BA
