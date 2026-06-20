# EOL / Formatting / Include Diff Analysis

## CRLF Consistency

| Scope | EOL | Source |
|-------|-----|--------|
| Project `.cpp`/`.h`/`.hlsl` | **CRLF** | `.editorconfig` + `core.autocrlf=true` |
| `vcpkg_installed/` (3rd party) | LF | Untracked, excluded by `.gitignore` |

All project source files in working tree are CRLF. No violations.

## `.gitattributes`

**Not present.** The project relies on user-level `core.autocrlf=true` (Windows default).
Without `.gitattributes`, the behavior depends on each developer's git config.

**Recommendation:** Add `.gitattributes` to make the policy repo-absolute:
```
*.cpp eol=crlf
*.h eol=crlf
*.hlsl eol=crlf
*.hlsli eol=crlf
```

## `.editorconfig` (exists)

84 lines, mostly VS C++ formatting settings (`cpp_indent_*`, `cpp_space_*`).
Key setting: `end_of_line = crlf` for C++ files.

Note: `.editorconfig` settings (used by VS editor) and `.clang-format` settings are **independent systems**. Some overlap conceptually (brace style, indent width), but they don't conflict because different tooling consumes them.

## `.clang-format` (exists)

Covers the actual formatting rules for clang-format. Matches AGENTS.md conventions:
- Allman braces, 4-space indent, left-aligned pointers
- `SortIncludes: false` — no reordering

## Style-Only Diffs on This Branch

**None found.** All 7 changed files contain only functional changes:
- `DeferredGpuReleaseQueue.h` — new file
- `D3D12HelloTexture.cpp/.h` — refactored to use deferred release queue
- `EnvironmentMap.h` — added `std::move` in `Attach`/`Detach`
- `.vcxproj` / `.vcxproj.filters` — added new file entry
- `docs/EnvMapReloadTestProcedure.md` — new doc

No trailing whitespace, no brace style changes, no include reordering in the diff.

## Summary

| Item | Status |
|------|--------|
| CRLF maintained | ✓ |
| `.gitattributes` | Missing (optional, recommended) |
| `.editorconfig` role | Documented above |
| Style-only diff | None |
