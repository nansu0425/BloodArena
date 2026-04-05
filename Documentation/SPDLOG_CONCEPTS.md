# spdlog Concepts & Reference

Practical reference for implementing the BloodArena logging system. Covers the core concepts needed to set up an spdlog-based logger with VS Debug Output (msvc_sink).

> **Prerequisite**: spdlog v1.16.0 is already integrated via vcpkg (static lib, compiled mode, no-exceptions).

## Table of Contents

1. [Architecture Overview](#1-architecture-overview)
2. [Compiled Mode](#2-compiled-mode)
3. [No-Exceptions Mode](#3-no-exceptions-mode)
4. [Log Levels](#4-log-levels)
5. [Compile-Time Level Filtering](#5-compile-time-level-filtering)
6. [Source Location Logging](#6-source-location-logging)
7. [Sinks](#7-sinks)
8. [Pattern Strings](#8-pattern-strings)
9. [Logger Lifecycle](#9-logger-lifecycle)
10. [Integration with BloodArena](#10-integration-with-bloodarena)

---

## 1. Architecture Overview

spdlog has three core components:

| Component | Role |
|-----------|------|
| **Logger** | Entry point for log calls. Holds a name, a log level, and one or more sinks. |
| **Sink** | Output destination. Each sink writes formatted messages to a specific target (file, debug output, console, etc.). |
| **Formatter** | Converts a log message into a string. Each sink has its own formatter, so different sinks can use different formats. |

### Data Flow

```
  Log macro call
  (SPDLOG_INFO, etc.)
        |
        v
+-----------------+
| Compile-Time    |  SPDLOG_ACTIVE_LEVEL check
| Filter          |  Below active level -> (void)0, never compiled
+--------+--------+
         |  passes
         v
+-----------------+
| Runtime Filter  |  logger->level() check
| (Logger Level)  |  Below logger level -> early return, no formatting
+--------+--------+
         |  passes
         v
+-----------------+
| Sink Dispatch   |  Logger iterates over all sinks
+--------+--------+
         |  for each sink
         v
+-----------------+
| Runtime Filter  |  sink->level() check
| (Sink Level)    |  Below sink level -> skip this sink
+--------+--------+
         |  passes
         v
+-----------------+
| Formatter       |  pattern_formatter formats the message
+--------+--------+
         |
         v
+-----------------+
| Output          |  OutputDebugStringA, file write, etc.
+-----------------+
```

Key takeaway: there are **two filtering layers** (compile-time and runtime), and runtime filtering happens at **both** the logger level and the per-sink level. The formatter only runs after all filters pass, so filtered messages have zero formatting overhead.

---

## 2. Compiled Mode

spdlog supports two build modes:

| Mode | Define | How it works |
|------|--------|--------------|
| **Header-only** | (none) | All code is in headers via `SPDLOG_INLINE` = `inline`. Simple to set up but increases compile time. |
| **Compiled** | `SPDLOG_COMPILED_LIB` | spdlog is built as a static/dynamic library. `SPDLOG_INLINE` = empty, so implementations live in `.cpp` files. Faster incremental builds. |

BloodArena uses **compiled mode** -- the `SPDLOG_COMPILED_LIB` define is set in both the vcpkg build (via triplet) and the application project (via vcxproj preprocessor definitions). This means:

- Include spdlog headers as usual (`#include <spdlog/spdlog.h>`)
- Link against the pre-built static library (handled automatically by vcpkg)
- Do **not** include `spdlog/spdlog-inl.h` or other `-inl.h` files

---

## 3. No-Exceptions Mode

The project disables C++ exceptions globally. spdlog must be told about this via `SPDLOG_NO_EXCEPTIONS`.

### Macro Substitutions

| Macro | With Exceptions | With `SPDLOG_NO_EXCEPTIONS` |
|-------|----------------|-----------------------------|
| `SPDLOG_TRY` | `try` | *(empty)* |
| `SPDLOG_THROW(ex)` | `throw(ex)` | `printf("spdlog fatal error: %s\n", ex.what()); std::abort();` |
| `SPDLOG_CATCH_STD` | `catch (const std::exception&) {}` | *(empty)* |

### The Double-Define Requirement

`SPDLOG_NO_EXCEPTIONS` must be defined in **two places**:

```
vcpkg custom triplet                     Application project (vcxproj)
(x64-windows-noexcept.cmake)             (Preprocessor Definitions)
-----------------------------            -----------------------------
set(VCPKG_CXX_FLAGS                      SPDLOG_COMPILED_LIB
    "/DSPDLOG_NO_EXCEPTIONS")            SPDLOG_NO_EXCEPTIONS
         |                                        |
         v                                        v
  spdlog static library                    Application code
  compiled without exceptions              includes spdlog headers
                                           with matching defines
```

**Why both?** The triplet define affects how the spdlog **library** is compiled. The vcxproj define affects how the spdlog **headers** are interpreted by the application code. If these mismatch, the application code may try to call exception-throwing functions that do not exist in the library, causing linker errors or undefined behavior.

### Custom Error Handler

The default `SPDLOG_THROW` calls `std::abort()`, which bypasses the project termination policy. Override this with:

```cpp
spdlog::set_error_handler([](const std::string& msg)
{
    // Project-specific error handling instead of abort
});
```

This is a **global** setting -- applies to all loggers. The signature is `void(const std::string&)`.

Note: this error handler is for **spdlog internal errors** (e.g., failed file operations, formatting errors), not for application-level errors. Normal log calls do not trigger it.

---

## 4. Log Levels

spdlog defines 7 levels, from most verbose to most severe:

| Value | Enum | Macro Suffix | Typical Use |
|-------|------|--------------|-------------|
| 0 | `trace` | `TRACE` | Per-frame verbose diagnostics (positions, states) |
| 1 | `debug` | `DEBUG` | Development-time diagnostics (init steps, resource loading) |
| 2 | `info` | `INFO` | System lifecycle events (subsystem started, config loaded) |
| 3 | `warn` | `WARN` | Recoverable anomalies (fallback path taken, degraded mode) |
| 4 | `err` | `ERROR` | Failed operations that the system can survive |
| 5 | `critical` | `CRITICAL` | Unrecoverable errors -- maps to `Fatal()` in BloodArena |
| 6 | `off` | -- | Disables all logging |

### Runtime Filtering

```cpp
logger->set_level(spdlog::level::debug);   // Logger-level filter
sink->set_level(spdlog::level::warn);      // Per-sink filter (independent)
```

A message must pass **both** the logger-level filter and the sink-level filter to be output. This allows, for example, a file sink to capture everything from `debug` up while the MSVC sink only shows `warn` and above.

---

## 5. Compile-Time Level Filtering

`SPDLOG_ACTIVE_LEVEL` controls which log levels are **compiled into the binary**. Log calls below this level are replaced with `(void)0` by the preprocessor -- zero overhead, not even argument evaluation.

### How It Works

```cpp
// If SPDLOG_ACTIVE_LEVEL is SPDLOG_LEVEL_INFO (2):

SPDLOG_TRACE(...)    // -> (void)0  (compiled out, level 0 < 2)
SPDLOG_DEBUG(...)    // -> (void)0  (compiled out, level 1 < 2)
SPDLOG_INFO(...)     // -> SPDLOG_LOGGER_CALL(default_logger, info, ...)  (kept)
SPDLOG_WARN(...)     // -> SPDLOG_LOGGER_CALL(default_logger, warn, ...)  (kept)
SPDLOG_ERROR(...)    // -> kept
SPDLOG_CRITICAL(...) // -> kept
```

### Where to Define

`SPDLOG_ACTIVE_LEVEL` must be defined **before** any spdlog header is included. The recommended approach is to define it in the project preprocessor definitions (vcxproj), so it applies globally:

| Build | Value | Effect |
|-------|-------|--------|
| Debug | `SPDLOG_LEVEL_TRACE` (0) | All levels compiled in |
| Release | `SPDLOG_LEVEL_INFO` (2) | trace/debug stripped at compile time |

### Default

If `SPDLOG_ACTIVE_LEVEL` is not defined, spdlog defaults to `SPDLOG_LEVEL_INFO` (2).

---

## 6. Source Location Logging

spdlog can automatically capture the file name, line number, and function name of each log call -- but **only** when using the macro API.

### Macro vs Direct API

| Approach | Source Location | Compile-Time Filtering |
|----------|:--------------:|:---------------------:|
| `SPDLOG_INFO("msg")` | Yes | Yes |
| `SPDLOG_LOGGER_INFO(logger, "msg")` | Yes | Yes |
| `logger->info("msg")` | No | No |
| `spdlog::info("msg")` | No | No |

The macros work by expanding to `SPDLOG_LOGGER_CALL`, which captures `__FILE__`, `__LINE__`, and `SPDLOG_FUNCTION` (= `__FUNCTION__` on MSVC) into a `source_loc` struct and passes it to `logger->log()`.

### The Two Macro Families

| Family | Target | Example |
|--------|--------|---------|
| `SPDLOG_LOGGER_xxx` | Specific logger | `SPDLOG_LOGGER_INFO(myLogger, "msg")` |
| `SPDLOG_xxx` | Default logger | `SPDLOG_INFO("msg")` -- uses `spdlog::default_logger_raw()` internally |

**Rule**: Always use one of these macro families. Never call `logger->info()` directly if you want source location or compile-time filtering.

---

## 7. Sinks

### Sink Basics

A sink is an output destination. Each sink:
- Has its own **formatter** (can format messages differently per sink)
- Has its own **level filter** (can filter independently from the logger)
- Is either thread-safe (`_mt`) or single-threaded (`_st`)

| Suffix | Mutex Type | Use Case |
|--------|-----------|----------|
| `_mt` | `std::mutex` | Multiple threads may log concurrently |
| `_st` | `null_mutex` (no-op) | Only one thread logs -- avoids lock overhead |

### msvc_sink (Primary Target)

`msvc_sink` writes log messages to the Visual Studio Output window via `OutputDebugStringA`.

Key behaviors:
- **Debugger check**: By default, `check_debugger_present_` is `true`. When no debugger is attached, `IsDebuggerPresent()` returns false and the sink **skips output entirely** -- no wasted formatting or API calls.
- **Null terminator**: The sink appends `\0` to the formatted buffer before calling `OutputDebugStringA`.
- **Flush**: `flush_()` is a no-op -- `OutputDebugStringA` writes are immediate.

Available types:

| Type | Alias |
|------|-------|
| `msvc_sink_mt` | `windebug_sink_mt` |
| `msvc_sink_st` | `windebug_sink_st` |

Constructor:

```cpp
msvc_sink()                                // check_debugger_present_ = true
msvc_sink(bool check_debugger_present)     // explicit control
```

### Other Sinks (Future Use)

| Sink | Purpose | When to Add |
|------|---------|-------------|
| `rotating_file_sink` | File log with size-based rotation | When file logging is needed |
| `callback_sink` | Custom callback per log message | For ImGui in-game console integration |

A logger can hold **multiple sinks simultaneously** -- messages are dispatched to all sinks that pass their level filter.

---

## 8. Pattern Strings

The formatter converts log messages into strings using a pattern string with `%` flags.

### Key Format Flags

| Flag | Description | Example Output |
|------|-------------|----------------|
| `%v` | Log message content | `Device created successfully` |
| `%l` | Log level (full) | `info` |
| `%L` | Log level (short) | `I` |
| `%n` | Logger name | `BloodArena` |
| `%t` | Thread ID | `12345` |
| `%s` | Source filename (short, basename only) | `WinMain.cpp` |
| `%g` | Source filename (full path) | `C:\...\WinMain.cpp` |
| `%#` | Source line number | `42` |
| `%!` | Source function name | `WinMain` |
| `%@` | Source location shorthand (`file:line`) | `WinMain.cpp:42` |
| `%H:%M:%S` | Time (hour:min:sec) | `14:30:05` |
| `%e` | Milliseconds | `123` |
| `%Y-%m-%d` | Date | `2026-04-05` |
| `%+` | Default full pattern | *(see below)* |

### Default Pattern

`%+` expands to the **full formatter** pattern:

```
[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] [%s:%#] %v
```

Example output:

```
[2026-04-05 14:30:05.123] [BloodArena] [info] [WinMain.cpp:42] Device created successfully
```

### Setting a Custom Pattern

```cpp
// Apply to a specific sink
sink->set_pattern("[%H:%M:%S.%e] [%l] %v");

// Apply to a logger (applies to all its sinks)
logger->set_pattern("[%H:%M:%S.%e] [%l] [%s:%#] %v");
```

---

## 9. Logger Lifecycle

### Creation

A logger is constructed with a name and one or more sinks:

```cpp
auto logger = std::make_shared<spdlog::logger>("LoggerName", sinks_init_list);
```

Where `sinks_init_list` is an `std::initializer_list<spdlog::sink_ptr>` (or begin/end iterators).

### Default Logger

spdlog maintains a **global default logger** that the `SPDLOG_xxx` macros (without `_LOGGER_`) use:

```cpp
spdlog::set_default_logger(logger);    // Register as default
spdlog::default_logger_raw();          // Get raw pointer to default logger
```

After setting the default logger, `SPDLOG_INFO("msg")` routes to it automatically.

### Shutdown

```cpp
spdlog::shutdown();    // Flushes all loggers, drops all loggers from the registry
```

Call at application exit to ensure all buffered messages are flushed. After `shutdown()`, further log calls are silently dropped.

### Default Error Handler Behavior

If no custom error handler is set, spdlog built-in handler writes to `stderr` with rate limiting (at most once per second):

```
[*** LOG ERROR #0001 ***] [2026-04-05 14:30:05] [BloodArena] error message
```

---

## 10. Integration with BloodArena

### Fatal() Function

The `critical` log level maps to the project `Fatal()` termination pattern ([ERROR_HANDLING.md](ERROR_HANDLING.md)):

```
Fatal() flow:
  1. Log at critical level (via SPDLOG_CRITICAL or SPDLOG_LOGGER_CRITICAL)
  2. spdlog::shutdown() to flush all sinks
  3. ExitProcess(errorCode)
```

### BA_ASSERT vs Logging

These are complementary, not overlapping:

| | BA_ASSERT | Logging |
|---|-----------|---------|
| **Purpose** | Catch programmer errors (invariant violations) | Record runtime events and diagnostics |
| **Active in** | Debug only | Both Debug and Release (level-dependent) |
| **On failure** | `__debugbreak()` -- halts in debugger | Writes to output targets, continues (unless Fatal) |
| **Output** | `OutputDebugString` (file + line) | Multi-sink (file, debug output, console) |

### Logger Initialization Timing

The logger should be initialized **early in WinMain**, before any subsystem that may log:

```
WinMain entry
  +-- Logger::Init()          <-- Here (first)
  +-- Window creation
  +-- DX11 device creation
  +-- Resource loading
  +-- Game loop entry
```

During the initialization phase, both `MessageBox` (for user-facing errors) and the logger (for developer diagnostics) can coexist.

---

## References

- [spdlog GitHub](https://github.com/gabime/spdlog)
- [spdlog Wiki](https://github.com/gabime/spdlog/wiki)
- [ERROR_HANDLING.md](ERROR_HANDLING.md) -- Error code system and termination policy
- [DEBUG_INFRASTRUCTURE.md](DEBUG_INFRASTRUCTURE.md) -- Debug infrastructure roadmap (Section 2: Logging System)
