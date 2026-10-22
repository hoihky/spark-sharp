#!/usr/bin/env bash
# Regenerates C# bindings from SparkInterop headers via ClangSharp.
# Requires: Spark engine checkout at SPARK_ENGINE_ROOT, dotnet 8+.
set -euo pipefail

SHARP_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
ENGINE_ROOT="${SPARK_ENGINE_ROOT:-${SHARP_ROOT}/../spark}"
export SPARK_SHARP_ROOT="${SHARP_ROOT}"
export SPARK_ENGINE_ROOT="${ENGINE_ROOT}"

CONFIG="${SHARP_ROOT}/scripting/bindings/bindings.config.json"
GENERATOR="${SHARP_ROOT}/scripting/bindings/generator/Spark.Bindings.Generator.csproj"
OUT="${SHARP_ROOT}/scripting/bindings/generated/Spark.Bindings"

cd "${SHARP_ROOT}"
python3 "${SHARP_ROOT}/tools/inject_spark_script_bind.py"
python3 "${SHARP_ROOT}/tools/spark_script_bindgen.py"
python3 "${SHARP_ROOT}/tools/generate_interop_manifest.py"
dotnet tool restore >/dev/null
export PATH="${PATH}:${HOME}/.dotnet/tools"

dotnet run --project "${GENERATOR}" -- "${CONFIG}"

"${SHARP_ROOT}/tools/sync-component-kind-bindings.sh"
python3 "${SHARP_ROOT}/tools/scan-cpp-component-api.py" || true
python3 "${SHARP_ROOT}/tools/generate_native_pinvoke_companion.py"
python3 "${SHARP_ROOT}/tools/generate-csharp-component-registry.py"

if [[ ! -f "${OUT}/Native.g.cs" ]]; then
  echo "error: Native.g.cs was not generated" >&2
  exit 1
fi

echo "Bindings written to ${OUT}"
