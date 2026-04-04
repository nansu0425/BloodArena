# Debug Infrastructure Roadmap

Items to be implemented before or alongside core game systems. Each section describes its purpose, implementation outline, and priority.

> **Status**: Planning — no items implemented yet.

## Table of Contents

1. [Assert Macro](#1-assert-macro)
2. [Logging System (spdlog)](#2-logging-system-spdlog)
3. [CRT Memory Leak Detection](#3-crt-memory-leak-detection)
4. [Address Sanitizer](#4-address-sanitizer)
5. [DirectX Debug Layer](#5-directx-debug-layer)
6. [Tracy Profiler](#6-tracy-profiler)
7. [Crash Dump (MiniDump)](#7-crash-dump-minidump)

---

## 1. Assert Macro

**Priority**: High — implement first, used across all systems.

### Purpose

Improve the debugging workflow over the standard `assert` (`<cassert>`). Key differences from `assert`:

| | Standard `assert` | Custom `BA_ASSERT` |
|---|---|---|
| Break location | Inside CRT (`ucrtbased.dll`) — must walk up the call stack to find the failing line | Directly on the failing line via `__debugbreak()` |
| Dialog | Abort / Retry / Ignore popup on every failure — requires a click before debugging | No popup — breaks immediately |
| Customization | None | Can integrate with the logging system, custom output format |

The standard `assert` already outputs file/line/expression to the VS Debug Output window via `OutputDebugString`, so output destination is not a differentiator. The practical gains are breaking at the exact source line without a dialog popup, which matters in a game loop where asserts may fire repeatedly.

### Implementation Outline

- Custom `BA_ASSERT(expr)` macro in a shared header (included via `PCH.h`)
- **Debug build** (`_DEBUG`):
  - Evaluates the expression
  - On failure: outputs file, line, and expression string via `OutputDebugString`
  - Triggers `__debugbreak()` to hit a breakpoint in the attached debugger
- **Release build** (`NDEBUG`):
  - Expands to nothing (completely removed by preprocessor)

### Example Usage

```cpp
BA_ASSERT(index < arraySize);
BA_ASSERT(pDevice != nullptr);
```

---

## 2. Logging System (spdlog)

**Priority**: High — foundational for all runtime diagnostics.

### Purpose

Provide structured, multi-target log output for runtime diagnostics using [spdlog](https://github.com/gabime/spdlog). Complements the error handling policy defined in [ERROR_HANDLING.md](ERROR_HANDLING.md).

### Why spdlog

Covers all planned requirements out of the box — multi-sink output, compile-time level filtering, automatic metadata — without needing a custom implementation. Also provides async logging, log rotation, and fmt-based formatting for free.

### Implementation Outline

- **Severity levels**: trace / debug / info / warn / err / critical (maps to the planned Fatal~Debug levels)
- **Sinks** (multi-sink logger):
  | Sink | Build | Notes |
  |---|---|---|
  | `rotating_file_sink` | All | File log with automatic rotation |
  | `msvc_sink` | Debug | Outputs to VS Output window via `OutputDebugString` |
  | Custom sink | All | Route log entries to ImGui in-game console (after integration) |
- **Automatic metadata**: Use `SPDLOG_LOGGER_CALL` macros — `__FILE__`, `__LINE__`, timestamp included automatically
- **Compile-time filtering**: Define `SPDLOG_ACTIVE_LEVEL` to strip low-severity logs at compile time (e.g., Release omits debug/trace)
- **`Fatal()` integration**: Log at critical level, then call `ExitProcess` as defined in ERROR_HANDLING.md

### Build Configuration

- **No exceptions**: Define `SPDLOG_NO_EXCEPTIONS` — this project has exceptions disabled (`ExceptionHandling>false`)
- **Compiled mode**: Use compiled (non-header-only) mode to minimize compile time impact
- **fmt**: Bundled with spdlog by default — no separate dependency needed
- **Integration**: Add via vcpkg or Git submodule

### References

- [spdlog GitHub](https://github.com/gabime/spdlog)

---

## 3. CRT Memory Leak Detection

**Priority**: Medium — useful once dynamic allocations begin.

### Purpose

Detect memory leaks in Debug builds using the MSVC CRT debug heap.

### Implementation Outline

- **Enable at startup** (Debug build only):
  ```cpp
  #ifdef _DEBUG
  #define _CRTDBG_MAP_ALLOC
  #include <crtdbg.h>
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
  #endif
  ```
- Automatically reports leaked allocations with file/line info to the VS Output window at process exit
- Add `_CRTDBG_MAP_ALLOC` and `<crtdbg.h>` to `PCH.h` under `#ifdef _DEBUG`
- No impact on Release builds

---

## 4. Address Sanitizer (ASan)

**Priority**: Medium — useful once dynamic allocations begin.

### Purpose

Detect memory access errors at runtime via compiler instrumentation.

### Implementation Outline

- **Compiler option**: `/fsanitize=address` in Debug builds
- Detects out-of-bounds access, use-after-free, double-free at runtime with immediate error reports
- **Usage pattern**: Not always-on — enable selectively when investigating memory bugs or run periodically in CI. ~2x memory and speed overhead makes it impractical for daily iteration
- **CRT conflict**: ASan and CRT debug heap both intercept allocations — test compatibility before enabling both simultaneously

---

## 5. DirectX Debug Layer

**Priority**: High — implement at DX11 device creation time.

### Purpose

Catch DirectX API misuse, resource leaks, and performance warnings at development time.

### Implementation Outline

- **Device creation**: Pass `D3D11_CREATE_DEVICE_DEBUG` flag in Debug builds
- **Info queue**: Query `ID3D11InfoQueue` from the device to:
  - Filter messages by severity (suppress noise, escalate errors)
  - Optionally break on corruption/error via `SetBreakOnSeverity`
- **DXGI debug**: Use `DXGIGetDebugInterface` + `IDXGIDebug::ReportLiveObjects` at shutdown to detect GPU resource leaks
- **Conditional compilation**: All debug layer code wrapped in `#ifdef _DEBUG` — zero overhead in Release

### Required Libraries

- `d3d11sdklayers.h` (ID3D11Debug, ID3D11InfoQueue)
- `dxgidebug.h` (IDXGIDebug)

---

## 6. Tracy Profiler

**Priority**: High — integrate at game loop creation time for early performance baselines.

### Purpose

Real-time, nanosecond-resolution profiler. Replaces a hand-rolled frame timer and provides far broader instrumentation:

| Capability | Details |
|---|---|
| Frame timing | `FrameMark` macro — automatic frame time and FPS in the viewer |
| CPU zone profiling | `ZoneScoped` / `ZoneNamed` — per-function and per-scope timing with call stacks |
| GPU profiling | `TracyD3D11Zone` — D3D11 render pass timing without manual timestamp queries |
| Memory profiling | Allocation/free tracking with per-callsite statistics |
| Lock contention | `TracyLockable(std::mutex, m)` — visualize thread contention |

All data is streamed to a separate **Tracy Viewer** application for timeline, histogram, and statistical analysis.

### Implementation Outline

- **Integration**: Add Tracy as a Git submodule or vcpkg package
- **Macro insertion**:
  - `FrameMark` at the end of each frame
  - `ZoneScoped` in functions to profile
  - `TracyD3D11Context` / `TracyD3D11Zone` for GPU profiling
- **Conditional compilation**: Wrap with `#ifdef TRACY_ENABLE` — define only in profiling builds so Release ships with zero overhead
- **Viewer**: Separate application (`Tracy/profiler/build/`) — connects to the running game over localhost

### References

- [Tracy GitHub](https://github.com/wolfpld/tracy)
- [Tracy manual (PDF)](https://github.com/wolfpld/tracy/releases/latest/download/tracy.pdf)

---

## 7. Crash Dump (MiniDump)

**Priority**: Low — most valuable once Release builds are distributed.

### Purpose

Generate `.dmp` files on unhandled crashes for post-mortem analysis with Visual Studio.

### Implementation Outline

- **Exception filter**: Register via `SetUnhandledExceptionFilter` at startup
- **Dump generation**: Call `MiniDumpWriteDump` (from `<DbgHelp.h>`) inside the filter
  - Write `.dmp` to a known directory (e.g., `logs/` or user's AppData)
  - Include thread and module information at minimum
- **PDB availability**: Release builds already have `GenerateDebugInformation=true` — ensure matching PDB files are archived alongside each build for symbol resolution
- **Active in all builds**: Crash dumps are useful in both Debug and Release

### Required Libraries

- `dbghelp.lib` / `<DbgHelp.h>`

---

## Implementation Order (Suggested)

```
Phase 1 (Core — before game systems):
  ├── Assert Macro
  └── Logging System (spdlog)

Phase 2 (Memory — once dynamic allocations begin):
  ├── CRT Memory Leak Detection
  └── Address Sanitizer

Phase 3 (Graphics — at DX11 init):
  ├── DirectX Debug Layer
  └── Tracy Profiler

Phase 4 (Polish — as needed):
  └── Crash Dump
```

## References

- [ERROR_HANDLING.md](ERROR_HANDLING.md) — Error code system and termination policy
- [WinMain.cpp](../Application/WinMain.cpp) — Current `ShowLastError()` utility
- [PCH.h](../Application/PCH.h) — Target for Assert macro and CRT debug includes
