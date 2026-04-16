#pragma once

namespace BA
{

inline std::wstring ResolveAssetPath(const wchar_t* relativePath)
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

} // namespace BA
