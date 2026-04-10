# Diagnostics

> **Note**: This spec documents already-implemented code.

## Context

The diagnostics module is the foundational layer of the BloodArena engine. It provides
logging, debug assertions, and crash handling that all other modules depend on. It also
defines the precompiled header (PCH) that aggregates platform SDK, standard library,
and diagnostic headers for the entire project.

Every module includes PCH.h, which transitively provides access to Assert, Logger, and
Crash facilities. This makes the diagnostics module a zero-dependency foundation that
the rest of the codebase builds upon.

## Functional Requirements

1. **Logging**: The application must have a centralized logging system that writes
   messages to both the MSVC debug output window and a log file (`Logs/Application.log`).

2. **Log Levels**: Log messages must support severity levels: trace, debug, info,
   warn, error, and critical.

3. **Log Source Location**: Each log message must include source file, line number,
   and function name.

4. **Log Format**: Log output must follow the pattern
   `[YYYY-MM-DD HH:MM:SS] [file:line function] [level] message`.

5. **Log Flush Policy**: The logger must automatically flush on warn level and above
   to ensure critical messages are not lost.

6. **Log File Truncation**: The log file must be truncated on each application start,
   not appended to.

7. **Debug Assertion**: A debug-only assertion macro (`BA_ASSERT`) must evaluate an
   expression, output the failed expression with file and line to the MSVC debug
   output, and break into the debugger. In release builds, the macro must be a no-op.

8. **Crash Termination**: A crash macro (`BA_CRASH`) must terminate the process. In
   debug builds, it must break into the debugger before exiting. In release builds,
   it must only break if a debugger is attached.

9. **Crash with Log**: A `BA_CRASH_LOG` macro must log a critical message before
   crashing.

10. **Conditional Crash**: A `BA_CRASH_IF` macro must crash when a boolean expression
    is true, logging the expression text.

11. **HRESULT Crash**: A `BA_CRASH_IF_FAILED` macro must crash when a COM HRESULT
    indicates failure, logging the HRESULT value in hex format.

12. **Precompiled Header**: A PCH must aggregate Windows SDK headers (Windows.h,
    D3D11, DXGI, D3DCompiler, WRL), CRT debug headers (debug builds only), standard
    library headers, and diagnostic headers (Assert, Logger, Crash).

## Non-Functional Requirements

1. The logging system must use spdlog as a compiled library (not header-only) with
   exceptions disabled (`SPDLOG_NO_EXCEPTIONS`).

2. All diagnostic macros must be usable without namespace qualification (global macro
   scope).

3. CRT memory leak detection (`_CRTDBG_MAP_ALLOC`, `_CRTDBG_LEAK_CHECK_DF`) must be
   enabled in debug builds via the PCH.

## Out of Scope

- Runtime log level configuration (currently hardcoded to trace).
- Log rotation or size limits.
- Custom log sinks beyond MSVC debug output and file.
- Assertion dialogs or user-facing error messages.

## Dependencies

- **External**: spdlog (vcpkg, compiled library mode, no exceptions)
- **Platform**: Windows SDK (Windows.h, OutputDebugStringA, IsDebuggerPresent,
  ExitProcess, __debugbreak)
- **Other Specs**: None. This is the foundation that all other specs depend on.

## Acceptance Criteria

1. `BA_LOG_INFO("test")` writes to both MSVC debug output and `Logs/Application.log`.
2. Log output contains timestamp, source location, level, and message.
3. `BA_ASSERT(false)` breaks into the debugger in debug builds.
4. `BA_ASSERT(false)` is a no-op in release builds.
5. `BA_CRASH()` terminates the process after breaking into the debugger (debug) or
   conditionally breaking (release).
6. `BA_CRASH_IF_FAILED(E_FAIL)` logs the HRESULT and terminates.
7. `BA_CRASH_IF(true)` logs the expression and terminates.
8. All modules compile successfully with PCH enabled.
