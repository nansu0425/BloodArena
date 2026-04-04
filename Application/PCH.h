#pragma once

#include <Windows.h>
#include <cstdio>

#ifdef _DEBUG
#define BA_ASSERT(expression)                                                                                                  \
    do                                                                                                                         \
    {                                                                                                                          \
        if (!(expression))                                                                                                     \
        {                                                                                                                      \
            CHAR buffer[512] = {};                                                                                             \
            sprintf_s(buffer, "Assertion failed!\n\nExpression: %s\nFile: %s\nLine: %d\n\n", #expression, __FILE__, __LINE__); \
            OutputDebugStringA(buffer);                                                                                        \
            __debugbreak();                                                                                                    \
        }                                                                                                                      \
    }                                                                                                                          \
    while (0)
#else
#define BA_ASSERT(expression)
#endif
