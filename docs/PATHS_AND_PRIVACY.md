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
- **CLion** shared run configs under `.idea/runConfigurations/` use `$PROJECT_DIR$` and `tools/clion-run-hello-cs-game-debug.sh` (not absolute paths).
- **VS Code** uses `${workspaceFolder}` and a prompt for `SPARK_ENGINE_ROOT` (default `../spark`).

## Before pushing

```bash
./tools/check-paths-privacy.sh
git status   # ensure no build/, bin/, obj/, compile_commands.json
```

Optional manual scan:

```bash
git grep -E '/Users/|/home/|\\\\Users\\\\' -- ':!build' ':!cmake-build-*' ':!**/obj/**' ':!**/bin/**'
```

If you accidentally committed build artifacts, remove them from the index (`git rm -r --cached …`) and rotate any exposed secrets.
