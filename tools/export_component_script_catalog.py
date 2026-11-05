#!/usr/bin/env python3
"""Export component-script-catalog.json from legacy SPARK_SCRIPT_BIND markers in engine headers."""
from __future__ import annotations

import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import spark_script_bindgen as bindgen  # noqa: E402
from component_script_catalog import (  # noqa: E402
    catalog_path,
    export_from_legacy_markers,
    write_catalog,
)
from spark_sharp_paths import ENGINE_ROOT, SHARP_ROOT  # noqa: E402


def main() -> int:
    if not bindgen.COMPONENT_ROOT.is_dir():
        print(f"error: engine components not found at {bindgen.COMPONENT_ROOT}", file=sys.stderr)
        return 1
    catalog = export_from_legacy_markers(
        bindgen.COMPONENT_ROOT,
        ENGINE_ROOT,
        bindgen.PREFIX_OVERRIDES,
        bindgen.pascal_to_prefix,
        is_bindable=bindgen.is_bindable_method,
        parse_args_fn=bindgen.parse_args,
    )
    out = catalog_path(SHARP_ROOT)
    write_catalog(out, catalog)
    method_count = sum(len(c.methods) for c in catalog.components)
    print(
        f"export_component_script_catalog: {len(catalog.components)} components, "
        f"{method_count} methods -> {out.relative_to(SHARP_ROOT)}"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
