#!/usr/bin/env bash
# Run SparkScriptHost with HelloCsGame (or custom runtimeconfig + assembly).
# Usage:
#   SPARK_ENGINE_ROOT=/path/to/spark ./tools/run-spark-script-host.sh [cmake-build-dir] [Debug|Release] [runtimeconfig] [assembly]
set -euo pipefail

SHARP_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${1:-$SHARP_ROOT/build}"
CONFIG="${2:-Debug}"
ENGINE_ROOT="${SPARK_ENGINE_ROOT:-$SHARP_ROOT/../spark}"

case "$(uname -s)" in
  Darwin)  LIB_ENV=(DYLD_LIBRARY_PATH) ;;
  Linux)   LIB_ENV=(LD_LIBRARY_PATH) ;;
  *)       LIB_ENV=() ;;
esac

NATIVE_DIR="$BUILD_DIR/native"
HOST="$NATIVE_DIR/SparkScriptHost"
if [[ ! -x "$HOST" ]]; then
  HOST="$BUILD_DIR/SparkScriptHost"
fi
if [[ ! -x "$HOST" ]]; then
  echo "SparkScriptHost not found under $BUILD_DIR (configure with -DSPARK_SHARP_BUILD_SCRIPT_HOST=ON)" >&2
  exit 1
fi

# Optional override for libSparkInterop.dylib. Must come from the *same* CMake build as
# SparkScriptHost — mixing build/ host with build-interop/ interop duplicates static
# SparkEngine + ImGui and triggers "Mismatched struct layout" in ImGui.
INTEROP_LIB_DIR="${SPARK_INTEROP_NATIVE_DIR:-}"

GAME_DIR="$SHARP_ROOT/scripting/samples/HelloCsGame/bin/$CONFIG/net8.0"
RUNTIME_CONFIG="${3:-$GAME_DIR/HelloCsGame.runtimeconfig.json}"
ASSEMBLY="${4:-$GAME_DIR/HelloCsGame.dll}"

if [[ ! -f "$RUNTIME_CONFIG" || ! -f "$ASSEMBLY" ]]; then
  echo "Missing managed game (build HelloCsGame $CONFIG first):" >&2
  echo "  dotnet build $SHARP_ROOT/scripting/samples/HelloCsGame/HelloCsGame.csproj -c $CONFIG" >&2
  echo "  cmake --build $BUILD_DIR --target SparkHelloCsGameDebug" >&2
  exit 2
fi

if [[ ! -d "$ENGINE_ROOT/assets" ]]; then
  echo "SPARK_ENGINE_ROOT must contain assets/ (got: $ENGINE_ROOT)" >&2
  exit 3
fi

if [[ -n "${INTEROP_LIB_DIR:-}" && "$INTEROP_LIB_DIR" != "$NATIVE_DIR" ]]; then
  echo "warning: SPARK_INTEROP_NATIVE_DIR differs from host native dir; rebuild both from one CMake build to avoid ImGui/engine mismatch." >&2
fi

if ((${#LIB_ENV[@]})); then
  _lib_var="${LIB_ENV[0]}"
  _lib_prev="${!_lib_var:-}"
  _lib_paths="$NATIVE_DIR"
  if [[ -n "${INTEROP_LIB_DIR:-}" && "$INTEROP_LIB_DIR" != "$NATIVE_DIR" ]]; then
    _lib_paths="$INTEROP_LIB_DIR:$_lib_paths"
  fi
  export "$_lib_var=${_lib_paths}${_lib_prev:+:$_lib_prev}"
fi

cd "$ENGINE_ROOT"
exec "$HOST" "$RUNTIME_CONFIG" "$ASSEMBLY" "${@:5}"
