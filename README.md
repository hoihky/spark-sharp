> **Disclaimer:** This project is an experimental, work-in-progress prototype built with the help of "vibe coding". Things will break. Features are currently missing, and the build scripts might not work at all. It is shared purely for educational or entertainment purposes.

# SparkSharp

C# scripting and interop for the [Spark](https://github.com/your-org/spark) C++ engine.

This repository owns:

- **SparkInterop** — C ABI shared library (`native/`) built against **SparkEngine**
- **Spark.Bindings** — generated P/Invoke + component mirrors (`scripting/bindings/`)
- **Spark.Scripting** — managed game bootstrap (`scripting/Spark.Scripting/`)
- **Tools** — bindgen and C# codegen (`tools/`)

The engine repo keeps only `include/spark/scripting/SparkScriptBind.hpp` (annotation macros for codegen).

## Prerequisites

- CMake ≥ 3.28, C++23, Vulkan SDK (same as Spark engine)
- **.NET 8 SDK**
- Sibling checkout: `spark-sharp` next to `spark`, or set **`SPARK_ENGINE_ROOT`**

## Build

```bash
export SPARK_ENGINE_ROOT=/path/to/spark   # optional if ../spark exists

cmake --preset debug
cmake --build build --target SparkInterop SparkScriptHost SparkHelloCsGameDebug
```

| Preset | Output |
|--------|--------|
| **debug** | `SparkInterop`, `SparkScriptHost`, bindings (Release), HelloCsGame Debug via target |
| **interop-only** | `SparkInterop` only (no host) |

Interop-only:

```bash
cmake --preset interop-only
cmake --build build-interop --target SparkInterop
```

## Run HelloCsGame

Working directory must be the **engine** tree (`SPARK_ENGINE_ROOT`) so `assets/` resolve.

```bash
cmake --build build --target SparkRunHelloCsGameDebug
```

Or manually:

```bash
./tools/run-spark-script-host.sh build Debug
```

## Regenerate C# API

After changing `SPARK_SCRIPT_BIND` markers or `SparkInterop.h`:

```bash
export SPARK_ENGINE_ROOT=/path/to/spark
./tools/generate-csharp-bindings.sh
```

Codegen reads component headers from **SPARK_ENGINE_ROOT** (`include/spark/ecs/components`).

## Debugging C# (HelloCsGame)

Games are **class libraries** loaded by native **SparkScriptHost** (CoreCLR via hostfxr). Breakpoints live in your `.cs` files; the debugger attaches to **SparkScriptHost**.

### Quick run (no IDE)

```bash
cmake --preset debug
cmake --build build --target SparkRunHelloCsGameDebug
```

### VS Code / Cursor

1. Open the **spark-sharp** folder.
2. Set **`SPARK_ENGINE_ROOT`** when prompted (default `../spark`).
3. **Run and Debug** → **SparkScriptHost (HelloCsGame Debug)** — builds host + Debug HelloCsGame, launches under LLDB (native).
4. For **C# breakpoints**: start the host (step 3 or `SparkRunHelloCsGameDebug`), then **Attach .NET to SparkScriptHost**.

Requires **C# Dev Kit** (or C# extension) for the attach configuration.

If your build directory is not `build/` (e.g. CLion `cmake-build-debug`), edit `.vscode/launch.json` paths or run:

```bash
./tools/run-spark-script-host.sh cmake-build-debug Debug
```

### JetBrains Rider

1. Open `scripting/samples/HelloCsGame/HelloCsGame.csproj` (or the repo root).
2. **Native** run configuration: executable `build/native/SparkScriptHost`, program arguments = Debug `HelloCsGame.runtimeconfig.json` and `.dll`, working directory = `SPARK_ENGINE_ROOT`, environment `DYLD_LIBRARY_PATH=build/native`.
3. Enable managed debugging on the same session.

### CLion

1. Load CMake preset **debug** (`SPARK_SHARP_BUILD_SCRIPT_HOST=ON`).
2. Build target **SparkRunHelloCsGameDebug** (custom target), or use run config **SparkRunHelloCsGameDebug** under `.run/`.
3. Set env **`SPARK_ENGINE_ROOT`** to your engine checkout.

For C# stepping, attach a .NET debugger to `SparkScriptHost` while the game is running.

### Custom game DLL

Build your library (same pattern as HelloCsGame: `GameBootstrap.Factory`, `GameEntry.Initialize`). Pass paths to the host:

```bash
./tools/run-spark-script-host.sh build Debug \
  /path/to/YourGame.runtimeconfig.json \
  /path/to/YourGame.dll \
  YourAsm.YourEntry, YourAsm \
  Initialize
```

(Extra args are forwarded to `SparkScriptHost`.)

## Documentation

Introduction page (same style as the Spark engine): **[docs/index.html](docs/index.html)** — open in a browser locally or publish via GitHub Pages.

## Paths and privacy

Do not commit `build/`, `bin/`, or `obj/` (machine-specific absolute paths). See **[docs/PATHS_AND_PRIVACY.md](docs/PATHS_AND_PRIVACY.md)**.

## Layout

| Path | Role |
|------|------|
| `native/include/spark/scripting/` | Public C ABI headers |
| `native/src/spark/scripting/` | Interop + host implementation |
| `scripting/bindings/` | C# projects + generator |
| `scripting/samples/HelloCsGame/` | Sample managed game |
| `CMakePresets.json` | `debug` / `interop-only` |
| `.vscode/` | Launch + build tasks |
| `tools/run-spark-script-host.sh` | Host launcher (Debug/Release, `SPARK_ENGINE_ROOT`) |

## CMake targets

| Target | Purpose |
|--------|---------|
| `SparkInterop` | Shared interop library |
| `SparkBindingsBuild` | Release build of Spark.Bindings + Spark.Scripting |
| `SparkHelloCsGameDebug` | Debug build of HelloCsGame |
| `SparkScriptHost` | CoreCLR + engine executable |
| `SparkRunHelloCsGameDebug` | Build + run sample (Debug) |

## License

SparkSharp is licensed under the [MIT License](LICENSE) with the [Commons Clause](https://commonsclause.com) restriction, which prohibits selling the software itself (same terms as the Spark C++ engine).
