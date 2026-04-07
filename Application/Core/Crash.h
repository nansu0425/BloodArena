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

#define BA_CRASH_LOG(message)                           \
    do                                                  \
    {                                                   \
        BA_LOG_CRITICAL(                                \
            "BA_CRASH_LOG({})",                         \
            message                                     \
        );                                              \
        BA_CRASH();                                     \
    }                                                   \
    while (0)

#define BA_CRASH_IF(expression)                         \
    do                                                  \
    {                                                   \
        if (expression)                                 \
        {                                               \
            BA_LOG_CRITICAL(                            \
                "BA_CRASH_IF({})",                      \
                #expression                             \
            );                                          \
            BA_CRASH();                                 \
        }                                               \
    }                                                   \
    while (0)

#define BA_CRASH_IF_FAILED(hr)                          \
    do                                                  \
    {                                                   \
        if (FAILED(hr))                                 \
        {                                               \
            BA_LOG_CRITICAL(                            \
                "BA_CRASH_IF_FAILED({:#010x})",         \
                static_cast<uint32_t>(hr)               \
            );                                          \
            BA_CRASH();                                 \
        }                                               \
    }                                                   \
    while (0)
