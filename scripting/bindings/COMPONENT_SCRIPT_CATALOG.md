# Component script catalog (`component-script-catalog.json`)

Manifest-first list of C++ `*Component` methods exposed to C# via the Spark interop ABI.

## Source of truth

| Artifact | Role |
|----------|------|
| **`component-script-catalog.json`** (this folder) | Declares `class`, optional `prefix`/`header`, and `{ suffix, member }` rows used by `spark_script_bindgen.py` |
| Engine `include/spark/ecs/components/**/*Component.hpp` | C++ declarations; catalog `member` names must match real methods |
| `SparkInteropComponentBindings.generated.{h,cpp}` | Generated C thunks (do not edit) |

Engine headers no longer carry binding markers; edit the JSON when adding script surface.

## Workflow

### Add or change a scriptable API

1. Implement the method on the C++ component in the **spark** repo.
2. Edit **`component-script-catalog.json`**: add `{ "suffix": "verb_noun", "member": "VerbNoun" }` under the component (and `prefix` override if needed — see `spark-interop-bindings.json` / `PREFIX_OVERRIDES` in bindgen).
3. From **spark-sharp** root:

   ```bash
   export SPARK_ENGINE_ROOT=/path/to/spark
   python3 tools/validate_component_script_catalog.py
   python3 tools/spark_script_bindgen.py
   ./tools/generate-csharp-bindings.sh   # or full CI-equivalent pipeline
   ```

Validation checks that each catalogued `member` exists in the header and passes the same `is_bindable_method` rules as bindgen (supported return/parameter types for the C ABI).

## Tools

| Script | Purpose |
|--------|---------|
| `tools/component_script_catalog.py` | Load/save JSON, resolve headers, find method declarations |
| `tools/validate_component_script_catalog.py` | Fail CI if catalog disagrees with engine headers |
| `tools/spark_script_bindgen.py` | Emit `SparkInteropComponentBindings.generated.*` from catalog |
| `tools/export_component_script_catalog.py` | **Archived:** one-time migration from legacy header markers (old branches only) |

`./tools/generate-csharp-bindings.sh` runs **validate → bindgen → …**.
