#pragma once

// NOMINMAX is defined at the project level (Application.vcxproj PreprocessorDefinitions)
// to prevent <Windows.h> from defining min/max macros that collide with std::min/std::max.
#include <Windows.h>

#include <d3d11.h>
#include <dxgi1_3.h>
#include <d3dcompiler.h>
#include <wrl/client.h>
#include <directxtk/SimpleMath.h>

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include <crtdbg.h>
#endif // _DEBUG

#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <format>
#include <functional>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include "Core/Assert.h"
#include "Core/Logger.h"
#include "Core/Crash.h"

