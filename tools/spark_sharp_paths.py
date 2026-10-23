"""Shared paths for spark-sharp tooling (engine checkout + native interop)."""
from __future__ import annotations

import os
from pathlib import Path

SHARP_ROOT = Path(os.environ.get("SPARK_SHARP_ROOT", Path(__file__).resolve().parents[1]))
ENGINE_ROOT = Path(os.environ.get("SPARK_ENGINE_ROOT", SHARP_ROOT.parent / "spark"))

NATIVE_INCLUDE = SHARP_ROOT / "native/include"
NATIVE_SRC = SHARP_ROOT / "native/src"

INTEROP_H = NATIVE_INCLUDE / "spark/scripting/SparkInterop.h"
INTEROP_GEN_H = NATIVE_INCLUDE / "spark/scripting/SparkInteropComponentBindings.generated.h"
INTEROP_KINDS_H = NATIVE_INCLUDE / "spark/scripting/SparkInteropComponentKinds.h"
INTEROP_FACTORY_CPP = NATIVE_SRC / "spark/scripting/SparkInteropComponentFactory.generated.cpp"

ENGINE_COMPONENTS = ENGINE_ROOT / "include/spark/ecs/components"
ENGINE_ECS_HPP = ENGINE_ROOT / "include/spark/ecs/Ecs.hpp"

BINDINGS_OUT = SHARP_ROOT / "scripting/bindings/generated/Spark.Bindings"
BINDINGS_COVERAGE = SHARP_ROOT / "scripting/bindings/binding-coverage.json"
BINDINGS_MANIFEST = SHARP_ROOT / "scripting/bindings/spark-interop-bindings.json"
