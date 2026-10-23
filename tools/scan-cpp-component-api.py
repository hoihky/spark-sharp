#!/usr/bin/env python3
"""Scan *Component.hpp public methods and compare to spark_* interop (coverage report)."""
from __future__ import annotations

import json
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

from spark_sharp_paths import (  # noqa: E402
    BINDINGS_COVERAGE,
    ENGINE_COMPONENTS,
    INTEROP_GEN_H,
    INTEROP_H,
)

METHOD_RE = re.compile(
    r"^\s+(?:virtual\s+)?(?:[\w:<>,\s&]+\s+)?(\w+)\s*\([^;]*\)\s*(?:const)?\s*(?:override)?\s*;",
    re.MULTILINE,
)

SPARK_FN_RE = re.compile(r"\b(spark_[a-z0-9_]+)\s*\(", re.MULTILINE)

SKIP_METHODS = {
    "Kind",
    "OnAttach",
    "OnDetach",
    "OnUpdate",
    "OnSignal",
    "UpdatePriority",
    "SubsystemTick",
}


def component_kind_from_path(path: Path) -> str:
    name = path.stem
    if name.endswith("Component"):
        name = name[: -len("Component")]
    return name


def pascal_to_prefix(kind: str) -> str:
    parts: list[str] = []
    buf = ""
    for ch in kind:
        if ch.isupper() and buf:
            parts.append(buf.lower())
            buf = ch
        else:
            buf += ch
    if buf:
        parts.append(buf.lower())
    return "_".join(parts)


def load_interop_functions() -> set[str]:
    text = INTEROP_H.read_text(encoding="utf-8", errors="ignore")
    if INTEROP_GEN_H.is_file():
        text += INTEROP_GEN_H.read_text(encoding="utf-8", errors="ignore")
    return set(SPARK_FN_RE.findall(text))


def scan_headers() -> dict[str, list[str]]:
    result: dict[str, list[str]] = {}
    for path in ENGINE_COMPONENTS.rglob("*Component.hpp"):
        text = path.read_text(encoding="utf-8", errors="ignore")
        methods = []
        for m in METHOD_RE.finditer(text):
            name = m.group(1)
            if name in SKIP_METHODS:
                continue
            methods.append(name)
        if methods:
            result[component_kind_from_path(path)] = sorted(set(methods))
    return result


def main() -> int:
    if not ENGINE_COMPONENTS.is_dir():
        print(f"error: {ENGINE_COMPONENTS} not found", file=sys.stderr)
        return 1
    headers = scan_headers()
    interop_fns = load_interop_functions()
    report = {
        "componentsWithPublicMethods": len(headers),
        "interopFunctionCount": len(interop_fns),
        "kindsWithoutInteropPrefix": [],
        "notes": "Per-method coverage requires SPARK_SCRIPT_BIND + bindgen; kinds listed lack any spark_<prefix>_ symbol.",
    }
    for kind, methods in sorted(headers.items()):
        prefix = pascal_to_prefix(kind)
        has_any = any(fn.startswith(f"spark_{prefix}_") for fn in interop_fns)
        if not has_any:
            report["kindsWithoutInteropPrefix"].append(
                {"kind": kind, "expectedPrefix": prefix, "methodCount": len(methods), "sampleMethods": methods[:10]}
            )
    BINDINGS_COVERAGE.parent.mkdir(parents=True, exist_ok=True)
    BINDINGS_COVERAGE.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(
        f"Wrote {BINDINGS_COVERAGE} "
        f"({len(report['kindsWithoutInteropPrefix'])} kinds without interop prefix, "
        f"{report['interopFunctionCount']} spark_* functions)"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
