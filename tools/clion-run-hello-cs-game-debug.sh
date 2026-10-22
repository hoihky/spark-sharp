#!/usr/bin/env bash
# CLion / terminal: build SparkScriptHost + HelloCsGame (Debug) and run.
set -euo pipefail

SHARP_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$SHARP_ROOT"

cache_has_script_host() {
  [[ -f "$1/CMakeCache.txt" ]] && grep -q 'SPARK_SHARP_BUILD_SCRIPT_HOST:BOOL=ON' "$1/CMakeCache.txt"
}

detect_build_dir() {
  if [[ -n "${SPARK_SHARP_BUILD_DIR:-}" ]]; then
    if [[ -f "${SPARK_SHARP_BUILD_DIR}/CMakeCache.txt" ]]; then
      echo "$(cd "${SPARK_SHARP_BUILD_DIR}" && pwd)"
      return 0
    fi
    echo "SPARK_SHARP_BUILD_DIR is set but has no CMakeCache.txt: ${SPARK_SHARP_BUILD_DIR}" >&2
    exit 1
  fi
  local candidate dir
  # Prefer a tree configured with SparkScriptHost (preset debug).
  for candidate in cmake-build-debug build cmake-build-release build-debug; do
    dir="$SHARP_ROOT/$candidate"
    if cache_has_script_host "$dir"; then
      echo "$dir"
      return 0
    fi
  done
  for candidate in cmake-build-debug build cmake-build-release build-debug; do
    dir="$SHARP_ROOT/$candidate"
    if [[ -f "$dir/CMakeCache.txt" ]]; then
      echo "$dir"
      echo "Warning: $candidate was not configured with SPARK_SHARP_BUILD_SCRIPT_HOST=ON." >&2
      echo "In CLion: enable CMake preset 'debug' or set SPARK_SHARP_BUILD_SCRIPT_HOST=ON, then Reload CMake." >&2
      return 0
    fi
  done
  echo "No CMake build tree found under $SHARP_ROOT." >&2
  echo "Configure in CLion (preset debug → build/) or: cmake --preset debug" >&2
  exit 1
}

BUILD_DIR="$(detect_build_dir)"
export SPARK_ENGINE_ROOT="${SPARK_ENGINE_ROOT:-$SHARP_ROOT/../spark}"

if [[ ! -d "$SPARK_ENGINE_ROOT/assets" ]]; then
  echo "SPARK_ENGINE_ROOT must contain assets/: $SPARK_ENGINE_ROOT" >&2
  echo "Set SPARK_ENGINE_ROOT to your Spark C++ engine checkout." >&2
  exit 1
fi

echo "Using build dir: $BUILD_DIR"
echo "Using engine root: $SPARK_ENGINE_ROOT"

if ! cache_has_script_host "$BUILD_DIR"; then
  echo "Reconfigure required: SPARK_SHARP_BUILD_SCRIPT_HOST is OFF in $BUILD_DIR" >&2
  echo "  cmake --preset debug -S \"$SHARP_ROOT\"" >&2
  echo "Or in CLion: CMake profile with SPARK_SHARP_BUILD_SCRIPT_HOST=ON, then Reload CMake Project." >&2
  exit 1
fi

cmake --build "$BUILD_DIR" --target SparkScriptHost SparkHelloCsGameDebug
exec "$SHARP_ROOT/tools/run-spark-script-host.sh" "$BUILD_DIR" Debug
