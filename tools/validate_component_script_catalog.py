#!/usr/bin/env python3
"""Validate component-script-catalog.json against engine *Component.hpp declarations."""
from __future__ import annotations

import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import spark_script_bindgen as bindgen  # noqa: E402
from component_script_catalog import (  # noqa: E402
    catalog_path,
    find_method_declaration,
    load_catalog,
    resolve_header_path,
)
from spark_sharp_paths import SHARP_ROOT  # noqa: E402


def main() -> int:
    path = catalog_path(SHARP_ROOT)
    if not path.is_file():
        print(f"error: missing catalog {path}", file=sys.stderr)
        print("Run: python3 tools/export_component_script_catalog.py", file=sys.stderr)
        return 1

    catalog = load_catalog(path)
    errors: list[str] = []
    warnings: list[str] = []

    for comp in catalog.components:
        header_path = resolve_header_path(bindgen.COMPONENT_ROOT, comp)
        if header_path is None:
            errors.append(f"{comp.class_name}: header not found (catalog header={comp.header!r})")
            continue
        text = header_path.read_text(encoding="utf-8", errors="ignore")
        if bindgen.CLASS_RE.search(text) is None:
            errors.append(f"{comp.class_name}: {header_path} is not a final GameComponent")
            continue

        seen_suffix: set[str] = set()
        for entry in comp.methods:
            if entry.suffix in seen_suffix:
                errors.append(f"{comp.class_name}: duplicate suffix {entry.suffix!r}")
            seen_suffix.add(entry.suffix)

            match = find_method_declaration(text, entry.member)
            if match is None:
                errors.append(
                    f"{comp.class_name}: member {entry.member!r} not found in {header_path.name}"
                )
                continue

            method = bindgen.BoundMethod(
                suffix=entry.suffix,
                cpp_name=entry.member,
                return_type=match.group("ret").strip(),
                args=bindgen.parse_args(match.group("args")),
                is_const=match.group("const") is not None,
            )
            if not bindgen.is_bindable_method(method):
                errors.append(
                    f"{comp.class_name}::{entry.member}: not bindable (types unsupported for C ABI)"
                )
            fn = bindgen.spark_fn_name(
                comp.prefix or bindgen.PREFIX_OVERRIDES.get(comp.class_name, bindgen.pascal_to_prefix(comp.class_name)),
                entry.suffix,
            )
            if fn in bindgen.load_manual_spark_functions():
                warnings.append(f"{fn} skipped in bindgen (manual SparkInterop.h)")

    for w in warnings:
        print(f"warning: {w}", file=sys.stderr)
    for e in errors:
        print(f"error: {e}", file=sys.stderr)

    if errors:
        print(f"validate_component_script_catalog: FAILED ({len(errors)} errors)", file=sys.stderr)
        return 1

    method_count = sum(len(c.methods) for c in catalog.components)
    print(
        f"validate_component_script_catalog: OK — {len(catalog.components)} components, "
        f"{method_count} catalogued methods"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
