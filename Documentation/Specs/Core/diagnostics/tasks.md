# Diagnostics — Tasks

> **Note**: This spec documents already-implemented code. All tasks are marked [done].

## Task 1: [done] Create precompiled header with platform and standard library includes

**Files:** `Application/Core/PCH.h`, `Application/Core/PCH.cpp`

**Description:** Create a precompiled header that aggregates Windows SDK headers
(Windows.h, d3d11.h, dxgi1_3.h, d3dcompiler.h, wrl/client.h), CRT debug headers
(debug builds only: _CRTDBG_MAP_ALLOC, cstdlib, crtdbg.h), and standard library
headers (algorithm, array, cassert, chrono, cmath, cstdint, cstdio, format,
functional, memory, span, string, string_view, unordered_map, vector). PCH.cpp
includes only PCH.h.

**Acceptance Criteria:**
- PCH.h includes all listed platform and standard library headers
- Debug builds define `_CRTDBG_MAP_ALLOC` and include CRT debug headers
- Release builds exclude CRT debug headers
- PCH.cpp contains only `#include "Core/PCH.h"`
- Project compiles with precompiled headers enabled

---

## Task 2: [done] Implement debug assertion macro

**Depends on:** Task 1

**Files:** `Application/Core/Assert.h`

**Description:** Define `BA_ASSERT(expression)` macro. In debug builds, it evaluates
the expression, formats a diagnostic message with expression text, file, and line
number, outputs it via `OutputDebugStringA`, and calls `__debugbreak()`. In release
builds, it expands to `(void)0`.

**Acceptance Criteria:**
- `BA_ASSERT(false)` outputs expression, file, and line to MSVC debug output in debug builds
- `BA_ASSERT(false)` breaks into the debugger in debug builds
- `BA_ASSERT(false)` is a no-op in release builds
- `BA_ASSERT(true)` does nothing in all builds

---

## Task 3: [done] Implement logging system with spdlog

**Depends on:** Task 1

**Files:** `Application/Core/Logger.h`, `Application/Core/Logger.cpp`

**Description:** Create a `Logger` class in the `BA` namespace with Initialize,
Shutdown, and GetInternalLogger methods. Initialize creates a multi-sink spdlog
logger with msvc_sink_mt and basic_file_sink_mt (writing to `Logs/Application.log`,
truncate on open). Set log pattern to
`[%Y-%m-%d %H:%M:%S] [%s:%# %!] [%l] %v`, level to trace, and flush on warn.
Register the logger globally with spdlog. Define `BA_LOG` base macro using
`spdlog::source_loc` and convenience macros for each level (TRACE through CRITICAL).
Declare `g_logger` as `extern std::unique_ptr<Logger>`.

**Acceptance Criteria:**
- `BA_LOG_INFO("test")` writes to both MSVC debug output and `Logs/Application.log`
- Log output contains timestamp, source location (file:line function), level, and message
- Logger flushes automatically on warn level and above
- Log file is truncated (not appended) on each application start
- All six level macros (TRACE, DEBUG, INFO, WARN, ERROR, CRITICAL) work correctly

---

## Task 4: [done] Implement crash macro family

**Depends on:** Task 3 (BA_CRASH_LOG/BA_CRASH_IF/BA_CRASH_IF_FAILED use BA_LOG_CRITICAL)

**Files:** `Application/Core/Crash.h`

**Description:** Define four crash macros. `BA_CRASH()`: in debug, `__debugbreak` +
`ExitProcess(1)`; in release, conditional `__debugbreak` (only if debugger attached)
+ `ExitProcess(1)`. `BA_CRASH_LOG(message)`: logs message at critical level, then
calls `BA_CRASH()`. `BA_CRASH_IF(expression)`: if expression is true, logs the
stringified expression at critical level, then crashes.
`BA_CRASH_IF_FAILED(hr)`: if `FAILED(hr)`, logs the HRESULT as hex (`{:#010x}`),
then crashes.

**Acceptance Criteria:**
- `BA_CRASH()` terminates the process in all builds
- `BA_CRASH()` breaks into the debugger in debug builds unconditionally
- `BA_CRASH()` breaks into the debugger in release builds only when `IsDebuggerPresent()` is true
- `BA_CRASH_LOG("msg")` logs "msg" at critical level before terminating
- `BA_CRASH_IF(true)` logs the expression text and terminates
- `BA_CRASH_IF(false)` does nothing
- `BA_CRASH_IF_FAILED(E_FAIL)` logs the HRESULT in hex format and terminates
- `BA_CRASH_IF_FAILED(S_OK)` does nothing

---

## Task 5: [done] Add diagnostic headers to precompiled header

**Depends on:** Tasks 2, 3, 4

**Files:** `Application/Core/PCH.h`

**Description:** Add includes for `Core/Assert.h`, `Core/Logger.h`, and `Core/Crash.h`
at the end of PCH.h, after all platform and standard library includes. This makes
diagnostic facilities available to every translation unit without explicit includes.

**Acceptance Criteria:**
- PCH.h includes Assert.h, Logger.h, and Crash.h
- All translation units can use BA_ASSERT, BA_LOG_*, and BA_CRASH_* without additional includes
- Include order is correct: Assert.h and Logger.h before Crash.h (Crash macros depend on BA_LOG_CRITICAL)
