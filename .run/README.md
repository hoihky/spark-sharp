# CLion run configurations

## Why “Executable is not specified”?

| CMake target | What it is |
|--------------|------------|
| **SparkHelloCsGameDebug** | **Build only** — runs `dotnet build` (no `.exe` / app to run) |
| **SparkRunHelloCsGameDebug** | **Shell script** — build + launch (use the **Shell Script** run config, not CMake Application) |
| **SparkScriptHost** | **The real executable** — CoreCLR host that loads HelloCsGame.dll |

Do **not** create a **CMake Application** run config with target `SparkHelloCsGameDebug`.

## Recommended: SparkScriptHost (HelloCsGame Debug)

1. **Settings → CMake** → enable presets → profile **`debug`** (build dir `build/`).
2. Reload CMake; build **SparkScriptHost** and **SparkHelloCsGameDebug** once (CMake tool window).
3. Run **`SparkScriptHost (HelloCsGame Debug)`** from the run dropdown (`.run/SparkScriptHost (HelloCsGame Debug).run.xml`).

If your build directory is **`cmake-build-debug`** instead of `build/`, edit that run config:

- **DYLD_LIBRARY_PATH** → `$PROJECT_DIR$/cmake-build-debug/native`
- Build **SparkScriptHost** under that profile first.

**Working directory** must be the **engine** repo (`../spark`, with `assets/`).

## Alternative: SparkRunHelloCsGameDebug

**Shell Script** run config → executes `tools/clion-run-hello-cs-game-debug.sh` (not a CMake Application).

- Auto-picks the first existing build tree: `cmake-build-debug`, then `build`, then `cmake-build-release`.
- Override with env **`SPARK_SHARP_BUILD_DIR`** (CMake sets this when you run target `SparkRunHelloCsGameDebug` from the CMake tool window).
- **Script path:** `tools/clion-run-hello-cs-game-debug.sh` (relative — not `$ProjectFileDir$/…`, CLion does not resolve that macro for Shell Script).
- **Working directory:** `$PROJECT_DIR$` (spark-sharp repo root).
- If you see **Shell script not found**, open the **spark-sharp** folder as the project root, or browse to the script once with the `…` button.

Reload CMake after pulling changes so the script is executable (`chmod +x tools/clion-run-hello-cs-game-debug.sh` if needed).

## Manual CMake Application

- **Target:** `SparkScriptHost` (executable appears automatically)
- **Program arguments:**  
  `.../HelloCsGame.runtimeconfig.json`  
  `.../HelloCsGame.dll`  
  (Debug paths under `scripting/samples/HelloCsGame/bin/Debug/net8.0/`)
- **Working directory:** engine root (`SPARK_ENGINE_ROOT`)
- **Environment:** `DYLD_LIBRARY_PATH=$PROJECT_DIR$/build/native`

Before first run, build target **SparkHelloCsGameDebug** from the CMake tool window.
