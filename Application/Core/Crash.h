#pragma once

#ifdef _DEBUG
#define BA_CRASH()                  \
    do                              \
    {                               \
        __debugbreak();             \
        ExitProcess(1);             \
    }                               \
    while (0)
#else // Release
#define BA_CRASH()                  \
    do                              \
    {                               \
        if (IsDebuggerPresent())    \
        {                           \
            __debugbreak();         \
        }                           \
        ExitProcess(1);             \
    }                               \
    while (0)
#endif // _DEBUG
