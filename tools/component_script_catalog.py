"""Load/save component script binding catalog (manifest-first interop)."""
from __future__ import annotations

import json
import re
from dataclasses import dataclass, field
from pathlib import Path

SCHEMA_VERSION = 1

CLASS_RE = re.compile(
    r"class\s+(?P<name>\w+Component)\s+final\s*:\s*public\s+GameComponent",
    re.MULTILINE,
)

# Return type must contain a word character (avoids matching call sites like `SetFoo(x);`).
_CPP_RET = r"[\w:<>,*&]+(?:\s+[\w:<>,*&]+)*"

METHOD_LINE_RE = re.compile(
    r"^\s+(?:(?:\[\[.*?\]\]\s+)*)"
    r"(?:(?:virtual\s+))?"
    r"(?:(?:explicit\s+))?"
    r"(?:(?:static\s+))?"
    rf"(?P<ret>{_CPP_RET})\s+"
    r"(?P<name>~?\w+)\s*"
    r"\((?P<args>[^)]*)\)\s*"
    r"(?P<const>const)?\s*(?:override)?\s*(?:noexcept)?\s*"
    r"(?:\{[^{}]*\}\s*)?;?\s*$"
)

BIND_SUFFIX_RE = re.compile(r"SPARK_SCRIPT_BIND\(\s*(?P<suffix>[\w]+)\s*\)")


@dataclass
class CatalogMethod:
    suffix: str
    member: str


@dataclass
class CatalogComponent:
    class_name: str
    prefix: str | None = None
    header: str | None = None
    methods: list[CatalogMethod] = field(default_factory=list)


@dataclass
class Catalog:
    schema_version: int
    components: list[CatalogComponent]

    def sorted(self) -> Catalog:
        comps = sorted(self.components, key=lambda c: c.class_name)
        for c in comps:
            c.methods = sorted(c.methods, key=lambda m: (m.suffix, m.member))
        return Catalog(schema_version=self.schema_version, components=comps)


def catalog_path(sharp_root: Path) -> Path:
    return sharp_root / "scripting/bindings/component-script-catalog.json"


def load_catalog(path: Path) -> Catalog:
    data = json.loads(path.read_text(encoding="utf-8"))
    version = int(data.get("schemaVersion", 0))
    if version != SCHEMA_VERSION:
        raise ValueError(f"unsupported catalog schemaVersion {version} (expected {SCHEMA_VERSION})")
    components: list[CatalogComponent] = []
    for entry in data.get("components") or []:
        if not isinstance(entry, dict):
            continue
        class_name = entry.get("class")
        if not isinstance(class_name, str) or not class_name:
            continue
        prefix = entry.get("prefix")
        header = entry.get("header")
        methods: list[CatalogMethod] = []
        for raw in entry.get("methods") or []:
            if not isinstance(raw, dict):
                continue
            suffix = raw.get("suffix")
            member = raw.get("member")
            if not isinstance(suffix, str) or not suffix:
                continue
            if not isinstance(member, str) or not member:
                raise ValueError(f"{class_name}: method suffix {suffix!r} requires non-empty member")
            methods.append(CatalogMethod(suffix=suffix, member=member))
        if methods:
            components.append(
                CatalogComponent(
                    class_name=class_name,
                    prefix=prefix if isinstance(prefix, str) else None,
                    header=header if isinstance(header, str) else None,
                    methods=methods,
                )
            )
    return Catalog(schema_version=version, components=components).sorted()


def write_catalog(path: Path, catalog: Catalog) -> None:
    catalog = catalog.sorted()
    payload = {
        "schemaVersion": catalog.schema_version,
        "components": [
            {
                "class": c.class_name,
                **({"prefix": c.prefix} if c.prefix else {}),
                **({"header": c.header} if c.header else {}),
                "methods": [{"suffix": m.suffix, "member": m.member} for m in c.methods],
            }
            for c in catalog.components
        ],
    }
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(payload, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")


def resolve_header_path(component_root: Path, comp: CatalogComponent) -> Path | None:
    if comp.header:
        candidate = component_root.parent.parent / "include" / comp.header
        if candidate.is_file():
            return candidate
    direct = component_root / f"{comp.class_name}.hpp"
    if direct.is_file():
        return direct
    matches = list(component_root.rglob(f"{comp.class_name}.hpp"))
    if len(matches) == 1:
        return matches[0]
    if matches:
        return sorted(matches)[0]
    return None


def header_relative_include(header: Path, engine_root: Path) -> str:
    return header.relative_to(engine_root / "include").as_posix()


def find_method_declaration(header_text: str, member_name: str) -> re.Match[str] | None:
    for line in header_text.splitlines():
        match = METHOD_LINE_RE.match(line)
        if match is not None and match.group("name") == member_name:
            return match
    # Multi-line declarations (parameters wrapped across lines).
    pattern = re.compile(
        r"^\s+(?:(?:\[\[.*?\]\]\s+)*)"
        r"(?:(?:virtual\s+))?"
        r"(?:(?:explicit\s+))?"
        r"(?:(?:static\s+))?"
        rf"(?P<ret>{_CPP_RET})\s+"
        + re.escape(member_name)
        + r"\s*\((?P<args>[^)]*)\)\s*"
        r"(?P<const>const)?\s*(?:override)?\s*(?:noexcept)?\s*"
        r"(?:\{|\s*;)",
        re.MULTILINE,
    )
    return pattern.search(header_text)


def export_from_legacy_markers(
    component_root: Path,
    engine_root: Path,
    prefix_overrides: dict[str, str],
    pascal_to_prefix_fn,
    *,
    is_bindable=None,
    parse_args_fn=None,
) -> Catalog:
    """Build catalog from SPARK_SCRIPT_BIND markers (one-time / refresh)."""
    components: list[CatalogComponent] = []
    for path in sorted(component_root.rglob("*Component.hpp")):
        text = path.read_text(encoding="utf-8", errors="ignore")
        cm = CLASS_RE.search(text)
        if not cm:
            continue
        class_name = cm.group("name")
        prefix = prefix_overrides.get(class_name, pascal_to_prefix_fn(class_name))
        lines = text.splitlines()
        methods: list[CatalogMethod] = []
        seen: set[tuple[str, str]] = set()
        for i, line in enumerate(lines):
            suffix_match = BIND_SUFFIX_RE.search(line)
            if suffix_match is None:
                continue
            suffix = suffix_match.group("suffix")
            decl = None
            for j in range(i + 1, min(i + 6, len(lines))):
                candidate = lines[j]
                if "SPARK_SCRIPT_BIND" in candidate:
                    continue
                decl = METHOD_LINE_RE.match(candidate)
                if decl is not None:
                    break
            if decl is None:
                continue
            member = decl.group("name")
            if member.startswith("~") or member == "Kind":
                continue
            if is_bindable is not None and parse_args_fn is not None:
                from spark_script_bindgen import BoundMethod  # local import avoids cycle at module load

                method = BoundMethod(
                    suffix=suffix,
                    cpp_name=member,
                    return_type=decl.group("ret").strip(),
                    args=parse_args_fn(decl.group("args")),
                    is_const=decl.group("const") is not None,
                )
                if not is_bindable(method):
                    continue
            key = (suffix, member)
            if key in seen:
                continue
            seen.add(key)
            methods.append(CatalogMethod(suffix=suffix, member=member))
        if not methods:
            continue
        components.append(
            CatalogComponent(
                class_name=class_name,
                prefix=prefix,
                header=header_relative_include(path, engine_root),
                methods=methods,
            )
        )
    return Catalog(schema_version=SCHEMA_VERSION, components=components).sorted()
