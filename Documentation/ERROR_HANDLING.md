# Error Handling Policy

## Core Principles

- **Unified HRESULT error code system**
- **No exceptions** — Exceptions are not used for game performance and control flow predictability.
- Unrecoverable errors are **logged and then immediately terminated**.

## Error Code System

### HRESULT Unification

Win32 API and DirectX/COM API have different error code systems, so we unify on HRESULT.

| API | Original Error System | Conversion Method |
|---|---|---|
| DirectX / COM | HRESULT (use as-is) | No conversion needed |
| Win32 API | DWORD (GetLastError) | `HRESULT_FROM_WIN32(GetLastError())` |

### Error Checking

Use the `FAILED()` macro for explicit checking every time.

```cpp
HRESULT hr = SomeFunction();
if (FAILED(hr))
{
    // Handle error
}
```

For Win32 API:

```cpp
HWND hWnd = CreateWindowEx(...);
if (hWnd == nullptr)
{
    HRESULT hr = HRESULT_FROM_WIN32(GetLastError());
    // Handle error
}
```

## Termination Policy

Termination methods are distinguished by phase.

### Initialization Phase (Before Game Loop)

Return `HRESULT` and exit via `return` in `WinMain`.

- Window creation, DX11 device/swap chain creation, shader compilation, resource loading, etc.
- Call depth is shallow, so error propagation overhead is minimal.

```cpp
HRESULT hr = InitDevice(...);
if (FAILED(hr))
{
    // Display error via MessageBox
    return -1;
}
```

### Runtime Phase (After Game Loop Entry)

The Fatal function logs the error and immediately terminates via `ExitProcess`.

- Adding error propagation code to every function degrades productivity and readability.
- The OS cleans up main memory and GPU resources on process termination, so there is no memory leak risk from immediate termination.

```cpp
void Fatal(const wchar_t* context, HRESULT hr)
{
    // Log output (file, OutputDebugString, etc.)
    ExitProcess(static_cast<UINT>(hr));
}
```

## Error Output

### Initialization Phase

Since window/DirectX creation may not be complete, display errors to the user via `MessageBox`.

### Runtime Phase (Future)

After game loop entry, the following output targets will be used (to be detailed when the logging system is introduced):

- File log
- `OutputDebugString` (Debug build)
- In-game console

## References

- Commercial engines (Unreal Engine, CryEngine, etc.) also use error codes + Fatal function instead of exceptions.
