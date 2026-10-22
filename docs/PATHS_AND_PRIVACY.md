# Paths and privacy (contributors)

## Do not commit

| Artifact | Why |
|----------|-----|
| `build/`, `cmake-build-debug/`, `_deps/` | CMake outputs embed **absolute paths** to this machine |
| `**/bin/`, `**/obj/` | MSBuild / NuGet files contain **home directory** paths (`~/.nuget`) |
| `.env`, keys, `credentials.json` | Secrets |

All of the above are listed in **`.gitignore`**.

## Source and scripts (OK)

- **CMake / presets** use `${sourceDir}`, `SPARK_ENGINE_ROOT`, `CMAKE_BINARY_DIR` — no hardcoded user paths.
- **Shell / Python tools** resolve roots from `$(dirname "$0")` or `SPARK_ENGINE_ROOT` / `SPARK_SHARP_ROOT` env vars.
- **CLion / VS Code** run configs use `$PROJECT_DIR$` / `$ProjectFileDir$` only.

## Before pushing

```bash
git status   # ensure no build/, bin/, obj/, compile_commands.json
git grep -E '/Users/|/home/|\\\\Users\\\\' -- ':!build' ':!cmake-build-*' ':!**/obj/**' ':!**/bin/**'
```

If you accidentally committed build artifacts, remove them from the index (`git rm -r --cached …`) and rotate any exposed secrets.
