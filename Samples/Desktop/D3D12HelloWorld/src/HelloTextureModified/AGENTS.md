## DirectX12 Conventions

- Follow project's .clang-format.
- Do not reorder includes.
- Prefer auto only when type is obvious.
- Use Allman braces.
- Use 4 spaces indentation.
- EOL should be CRLF.
- Do not use emoji (✅, ❌, etc.) in any output; use text markers like [x], [ ], [*] instead.

## CLI Flags (for AI-driven automation)

Flags are parsed in `DXSample::ParseCommandLineArgs()` (`DXSample.cpp:107`).

| Flag | Example | Description |
|------|---------|-------------|
| `-LogToFile` | `-LogToFile d3d12_debug.log` | On startup, dump all accumulated D3D12 Debug Layer messages (via `ID3D12InfoQueue`) to the specified file. Also appends new messages as they arrive (polled each frame). |
| `-LogFPS` | `-LogFPS 60` | Log CPU FPS to the same log file every N frames. Only active when `-LogToFile` is also specified. Value is `1000.0f / cpuFrameTimeMs`. |
| `-AutoSelectGltfDamagedHelmet` | `-AutoSelectGltfDamagedHelmet` | On startup, automatically select the "glTF Viewer > DamagedHelmet" scene and switch to Running mode without user interaction. Equivalent to calling `OpenSelectedScene()` after setting `m_selectedSceneIndex = 0`. |

## D3D12 Debug Layer Check

Use the Debug build with CLI automation when checking whether a change introduced D3D12 Debug Layer errors.

From this directory:

```powershell
.\bin\x64\Debug\D3D12HelloTextureModified.exe -AutoSelectGltfDamagedHelmet -LogToFile d3d12_debug.log -LogFPS 120
```

Recommended check:

```powershell
Select-String -LiteralPath d3d12_debug.log -Pattern "\[ERROR\]|\[WARNING\]|D3D12"
```

For automated verification, start the exe, let it run for a few seconds, close it, then inspect `d3d12_debug.log`.
An empty log, or a log without `[ERROR]`, means no D3D12 Debug Layer error was captured for that run.
Do not commit generated log files such as `d3d12_debug.log`.
