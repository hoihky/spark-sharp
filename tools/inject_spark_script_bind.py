#!/usr/bin/env python3
"""Insert SPARK_SCRIPT_BIND markers on script-safe public methods in *Component.hpp."""
from __future__ import annotations

import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

from spark_sharp_paths import (  # noqa: E402
    BINDINGS_MANIFEST,
    ENGINE_COMPONENTS,
    INTEROP_GEN_H,
    INTEROP_H,
    ENGINE_ROOT,
)

COMPONENT_ROOT = ENGINE_COMPONENTS
INTEROP = INTEROP_H
INTEROP_GEN = INTEROP_GEN_H
BIND_INCLUDE = '#include "spark/scripting/SparkScriptBind.hpp"'
MANIFEST = BINDINGS_MANIFEST


def load_prefix_overrides() -> dict[str, str]:
    overrides: dict[str, str] = {
        "Sprite2DCharacterAnimFsmComponent": "sprite_2d_fsm",
        "Character3DAnimFsmComponent": "char_3d_fsm",
        "AnimationEventReceiverComponent": "anim_event_receiver",
        "FoliageInstancedMeshComponent": "foliage_instanced",
        "GameStateComponent": "game_state",
        "ParallaxLayerComponent": "parallax",
        "ScreenShakeComponent": "screen_shake",
        "GameFlowTriggerComponent": "game_flow_trigger",
        "WindEnvironmentComponent": "wind_environment",
        "GrassFieldComponent": "grass_field",
        "InputActionMapComponent": "input_action_map",
        "PlayerInputComponent": "player_input",
        "AnimationLayerStackComponent": "animation_layer_stack",
        "CombatHurtFlash3DComponent": "combat_hurt_flash_3d",
        "Character3DBodyComponent": "character_3d_body",
        "Character3DVisualComponent": "character_3d_visual",
        "PhysicsAttachedBody3DComponent": "physics_attached_body_3d",
        "GameplayInventoryCollectorComponent": "gameplay_inventory_collector",
        "PhysicsContactListener3DComponent": "physics_contact_listener_3d",
    }
    if MANIFEST.is_file():
        import json

        data = json.loads(MANIFEST.read_text(encoding="utf-8"))
        for prefix, class_name in (data.get("classOverrides") or {}).items():
            if isinstance(class_name, str):
                overrides[class_name] = prefix
        for prefix, binding in (data.get("prefixToClass") or {}).items():
            if isinstance(binding, dict) and "class" in binding:
                overrides.setdefault(binding["class"], prefix)
    return overrides


PREFIX_OVERRIDES = load_prefix_overrides()

MARKERS = (
    "_get_",
    "_set_",
    "_is_",
    "_has_",
    "_add_",
    "_clear_",
    "_bind_",
    "_find_",
    "_play_",
    "_stop_",
    "_tick_",
    "_push_",
    "_pop_",
    "_apply_",
    "_import_",
    "_remove_",
    "_request_",
    "_configure_",
    "_prepare_",
    "_scatter_",
    "_bounds_",
)

SKIP_METHODS = {
    "Kind",
    "OnAttach",
    "OnDetach",
    "OnUpdate",
    "OnSignal",
    "UpdatePriority",
    "SubsystemTick",
    "SimulateCharacterControllers3D",
    "SimulateTriggerVolumes3D",
    "Refresh",
    "SyncRuntimeStates",
}

SKIP_TYPE_FRAGMENTS = (
    "Function<",
    "SharedPtr",
    "UniquePtr",
    "Array<",
    "SignalPayload",
    "IEngineContext",
    "GameWorld",
    "FrameTiming",
    "std::function",
    "Callback",
    "Applicator",
    "Resolver",
    "const char*&",
)

METHOD_RE = re.compile(
    r"^(\s+)(?:(?:\[\[.*?\]\]\s+)*)"
    r"(?:(?:virtual\s+))?"
    r"(?:explicit\s+)?"
    r"(?P<ret>[\w:<>,\s*&]+?)\s+"
    r"(?P<name>~?\w+)\s*"
    r"\((?P<args>[^)]*)\)\s*"
    r"(?P<const>const)?\s*(?:override)?\s*(?:noexcept)?\s*"
    r"(?:\{[^{}]*\}\s*)?;?\s*$"
)

CLASS_RE = re.compile(
    r"class\s+(?P<name>\w+Component)\s+final\s*:\s*public\s+GameComponent"
)


def extract_prefix(fn: str) -> str | None:
    if not fn.startswith("spark_"):
        return None
    body = fn[6:]
    for marker in MARKERS:
        if marker in body:
            return body[: body.index(marker)]
    return None


def existing_spark_functions() -> set[str]:
    text = INTEROP.read_text(encoding="utf-8")
    if INTEROP_GEN.is_file():
        text += INTEROP_GEN.read_text(encoding="utf-8")
    return set(re.findall(r"\b(spark_[a-z0-9_]+)\s*\(", text))


def pascal_to_prefix(class_name: str) -> str:
    kind = class_name[: -len("Component")] if class_name.endswith("Component") else class_name
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


def camel_to_snake(name: str) -> str:
    out: list[str] = []
    buf = ""
    for ch in name:
        if ch.isupper() and buf:
            out.append(buf.lower())
            buf = ch
        else:
            buf += ch
    if buf:
        out.append(buf.lower())
    return "_".join(out)


def method_to_suffix(name: str, ret: str) -> str | None:
    if name.startswith("~") or name.endswith("Component"):
        return None
    ret_norm = ret.replace("const ", "").replace("&", "").strip()
    if name.startswith("Get") and len(name) > 3:
        return "get_" + camel_to_snake(name[3:])
    if name.startswith("Set") and len(name) > 3:
        return "set_" + camel_to_snake(name[3:])
    if name.startswith("Is") and len(name) > 2 and ret_norm == "bool":
        # Pair with set_is_* (e.g. IsTrigger + SetIsTrigger → get_is_trigger / set_is_trigger).
        return "get_is_" + camel_to_snake(name[2:])
    if name.startswith("Is") and len(name) > 2:
        return "is_" + camel_to_snake(name[2:])
    if name.startswith("Has") and len(name) > 3:
        return "has_" + camel_to_snake(name[2:])
    if ret_norm == "bool":
        return "is_" + camel_to_snake(name)
    if ret_norm == "void":
        return camel_to_snake(name)
    if ret_norm in {"float", "int", "double"} or "uint" in ret_norm:
        return "get_" + camel_to_snake(name)
    if ret_norm.endswith("Mode") or ret_norm.endswith("State") or ret_norm.endswith("Source"):
        return "get_" + camel_to_snake(name)
    return camel_to_snake(name)


def is_bindable(ret: str, name: str, args: str) -> bool:
    if name in SKIP_METHODS or name.startswith("~"):
        return False
    blob = f"{ret} {args}"
    if any(frag in blob for frag in SKIP_TYPE_FRAGMENTS):
        return False
    if "GameObject&" in args or "const GameObject&" in args:
        return False
    if "ColliderFilter" in args:
        return False
    if "ContactHandler" in args or "Function<" in ret:
        return False
    if "GameplayTeamId" in ret and "Get" in name:
        return False
    if "GameplayTeamId" in args and "SetGameplayTeam" in name:
        return False
    if "Skeleton&" in args or "AnimationJointMask" in ret:
        return False
    if "UiTheme" in ret or "UiTheme" in args or "IUiElement" in ret or "IUiRenderer" in args:
        return False
    if "UiFrameInput" in ret or "UiFrameInput" in args or "IInput&" in args:
        return False
    if "operator" in name:
        return False
    return method_to_suffix(name, ret) is not None


def ensure_include(text: str) -> str:
    if BIND_INCLUDE in text:
        return text
    lines = text.splitlines(keepends=True)
    last_include = 0
    for i, line in enumerate(lines):
        if line.strip().startswith("#include"):
            last_include = i
    lines.insert(last_include + 1, BIND_INCLUDE + "\n")
    return "".join(lines)


def inject_file(path: Path, existing_fns: set[str]) -> int:
    text = path.read_text(encoding="utf-8")
    cm = CLASS_RE.search(text)
    if not cm:
        return 0
    class_name = cm.group("name")
    prefix = PREFIX_OVERRIDES.get(class_name, pascal_to_prefix(class_name))

    lines = text.splitlines(keepends=True)
    out: list[str] = []
    added = 0
    depth = 0
    in_public = False
    seen_private = False

    for line in lines:
        stripped = line.strip()
        if stripped.startswith("class ") and "GameComponent" in stripped:
            depth = 0
            in_public = False
            seen_private = False
        if stripped == "public:":
            in_public = True
        elif stripped == "private:" or stripped == "protected:":
            in_public = False
            seen_private = True
        if "{" in line:
            depth += line.count("{")
        if "}" in line:
            depth -= line.count("}")

        if (
            in_public
            and not seen_private
            and depth <= 1
            and "SPARK_SCRIPT_BIND" not in line
        ):
            m = METHOD_RE.match(line.rstrip("\n"))
            if m and is_bindable(m.group("ret"), m.group("name"), m.group("args")):
                suffix = method_to_suffix(m.group("name"), m.group("ret"))
                fn = f"spark_{prefix}_{suffix}"
                if suffix and fn not in existing_fns and f"SPARK_SCRIPT_BIND({suffix})" not in text:
                    indent = m.group(1)
                    out.append(f'{indent}SPARK_SCRIPT_BIND({suffix})\n')
                    added += 1
        out.append(line)

    if added == 0:
        return 0
    new_text = ensure_include("".join(out))
    path.write_text(new_text, encoding="utf-8")
    return added


def main() -> int:
    if not COMPONENT_ROOT.is_dir():
        print(f"error: engine components not found at {COMPONENT_ROOT}", file=sys.stderr)
        return 1
    existing_fns = existing_spark_functions()
    total = 0
    files = 0
    for path in sorted(COMPONENT_ROOT.rglob("*Component.hpp")):
        n = inject_file(path, existing_fns)
        if n:
            files += 1
            total += n
            print(f"{path.relative_to(COMPONENT_ROOT.parent.parent.parent)}: +{n} bindings")
    print(f"inject_spark_script_bind: {total} markers in {files} headers")
    return 0


if __name__ == "__main__":
    sys.exit(main())
