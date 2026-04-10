# Diagnostics — Design

> **Note**: This spec documents already-implemented code.

## Architecture

The diagnostics module sits at the bottom of the dependency graph. Every other module
depends on it transitively through the precompiled header (PCH.h).

```
PCH.h (included by every .cpp)
 ├── Windows SDK / DirectX / STL headers
 ├── Assert.h   — BA_ASSERT macro
 ├── Logger.h   — Logger class + BA_LOG_* macros
 └── Crash.h    — BA_CRASH macro family (depends on BA_LOG_CRITICAL)
```

The module has no runtime dependencies on other application modules. Crash.h depends
on Logger.h at the macro expansion level (BA_CRASH_LOG/BA_CRASH_IF/BA_CRASH_IF_FAILED
call BA_LOG_CRITICAL), but this dependency is resolved through PCH include order.

## Key Decisions

1. **Macros over functions for Assert/Crash**: Macros capture `__FILE__`, `__LINE__`,
   `__func__`, and `#expression` at the call site. Functions cannot do this without
   wrapping in macros anyway.

2. **spdlog as compiled library**: Using `SPDLOG_COMPILED_LIB` reduces compile times
   compared to header-only mode. Combined with `SPDLOG_NO_EXCEPTIONS` to match the
   project-wide no-exceptions policy.

3. **Global unique_ptr ownership**: `g_logger` is a `std::unique_ptr<Logger>` in the
   BA namespace. Lifecycle is explicit (Initialize/Shutdown), not RAII-based, to
   control initialization order with other subsystems.

4. **Two sinks (MSVC + file)**: `msvc_sink_mt` feeds Visual Studio's Output window
   during development. `basic_file_sink_mt` provides persistent logs. Both are
   thread-safe (`_mt` suffix).

5. **Debug-only BA_ASSERT**: Compiles to `(void)0` in release to avoid performance
   cost. Uses `OutputDebugStringA` + `__debugbreak` instead of the CRT `assert()`
   to avoid dialog boxes and provide immediate debugger control.

6. **BA_CRASH release behavior**: Checks `IsDebuggerPresent()` before `__debugbreak()`
   in release builds, so attached debuggers can inspect the crash while standalone
   execution terminates cleanly.

7. **PCH aggregates diagnostics**: Including Assert.h, Logger.h, and Crash.h in PCH.h
   means every translation unit has diagnostic facilities available without explicit
   includes.

## Data Structures

### Logger (class)

```cpp
namespace BA
{
class Logger
{
public:
    void Initialize();
    void Shutdown();
    spdlog::logger* GetInternalLogger() const;

private:
    std::shared_ptr<spdlog::logger> m_internalLogger;
};

extern std::unique_ptr<Logger> g_logger;
}
```

No other types are introduced. Assert and Crash are pure macro definitions with no
associated data structures.

## Interfaces

### Logger Public API

| Method | Signature | Description |
|--------|-----------|-------------|
| Initialize | `void Initialize()` | Creates spdlog logger with MSVC + file sinks, sets pattern/level/flush policy |
| Shutdown | `void Shutdown()` | Calls `spdlog::shutdown()` to flush and release all loggers |
| GetInternalLogger | `spdlog::logger* GetInternalLogger() const` | Returns raw pointer for macro use |

### Logging Macros

| Macro | Description |
|-------|-------------|
| `BA_LOG(level, ...)` | Base macro. Logs with source location via `spdlog::source_loc` |
| `BA_LOG_TRACE(...)` | Trace level log |
| `BA_LOG_DEBUG(...)` | Debug level log |
| `BA_LOG_INFO(...)` | Info level log |
| `BA_LOG_WARN(...)` | Warn level log |
| `BA_LOG_ERROR(...)` | Error level log |
| `BA_LOG_CRITICAL(...)` | Critical level log |

### Assert Macro

| Macro | Description |
|-------|-------------|
| `BA_ASSERT(expression)` | Debug: evaluates expression, outputs to debug output and breaks. Release: `(void)0` |

### Crash Macros

| Macro | Description |
|-------|-------------|
| `BA_CRASH()` | Debug: `__debugbreak` + `ExitProcess(1)`. Release: conditional `__debugbreak` + `ExitProcess(1)` |
| `BA_CRASH_LOG(message)` | Logs critical message, then crashes |
| `BA_CRASH_IF(expression)` | If expression is true, logs expression text and crashes |
| `BA_CRASH_IF_FAILED(hr)` | If `FAILED(hr)`, logs HRESULT as `0x` hex and crashes |

## Cross-Module Dependencies

**Depended on by**: Every module in the project (via PCH.h inclusion).

**Depends on**: Nothing (application-level). Only external dependencies (spdlog,
Windows SDK).

## File Plan

All files already exist. No files to create or modify.

| File | Role |
|------|------|
| `Application/Core/PCH.h` | Precompiled header aggregating SDK, STL, and diagnostic headers |
| `Application/Core/PCH.cpp` | PCH compilation unit (includes PCH.h only) |
| `Application/Core/Assert.h` | BA_ASSERT macro definition |
| `Application/Core/Logger.h` | Logger class declaration + BA_LOG_* macro definitions |
| `Application/Core/Logger.cpp` | Logger class implementation (spdlog setup) |
| `Application/Core/Crash.h` | BA_CRASH macro family definitions |
