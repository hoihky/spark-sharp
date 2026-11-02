# C# script API (spark-sharp)

This document maps the **Spark C++ engine API** to **Spark.Bindings** / **Spark.Scripting**, notes coverage gaps for demo ports, and describes the **object-oriented facet layout** (SOLID-friendly, minimal static surface).

## Recommended entry points

| C++ concept | C# type | Notes |
|-------------|---------|--------|
| `Spark::Game` | `Spark.Bindings.Game` / `Spark.Scripting.SparkGame` | Override lifecycle hooks |
| `Spark::IEngineContext` | `IEngineContext` | Input, scene, window |
| `Spark::Scene` | `Scene` | `GetWorld()` |
| `Spark::GameWorld` | `GameWorld` | Use **facets** below (preferred) |
| `Spark::GameObject` | `GameObject` | Hierarchy + `Add*` / `Get*` components |
| Components | `Spark.Bindings.Components.*` | ~120 generated mirrors |

### GameWorld facets (instance API)

Prefer facet properties over flat `GameWorld` methods and static helpers:

```csharp
GameWorld world = context.TryGetScene()!.GetWorld();

world.Entities.Create("Player");
world.Entities.SetParent(child, parent);
world.Entities.FindWithTag("Enemy", includeInactive: false);
world.AssetCache.LoadGltf(path);
world.AssetCache.Pump();
world.Physics.Simulate3D(timing);
world.Physics.Layers.DefaultQueryFilter();
world.Simulation.UpdateGameObjects(timing, context);
world.Rendering.SubmitStandardLit(context, viewProj, ...);
world.RenderLayers.Register("Foreground", 100);
world.Query.CollectWithTag("Pickup", buffer);
world.Scenes.BeginLoadAsync("levels/hub.sparkscene");
world.Scenes.Pump();
player.EmitSignal(SignalId.UserSignal(0));
```

| Facet | Mirrors C++ |
|-------|-------------|
| `Entities` | Create/destroy/find/reparent, immediate destroy |
| `AssetCache` | Load/register GLTF & textures, pump, readiness |
| `Physics` | 2D/3D step, character controllers, overlap queries, layer masks |
| `Simulation` | `UpdateGameObjects`, VFX, sound, AI |
| `Rendering` | Standard-lit submit / param fill |
| `RenderLayers` | `RenderLayerRegistry` |
| `Query` | `QueryGameObjects` (by tag, batched) |
| `Scenes` | `SceneManager` load/unload, spawn points, prefabs |

`SparkGame` exposes the same facets as `protected` members (`Entities`, `AssetCache`, …).

### Signals

| C++ | C# |
|-----|-----|
| `GameObject::EmitSignal` | `gameObject.EmitSignal(SignalId, …)` |
| `SignalId` / `UserBase + n` | `SignalId` enum / `GameObject.UserSignal(offset)` |
| `OnSignal` on components | Not exposed (no managed callback interop yet) |

### Deprecated static helpers

These remain for compatibility but are marked `[Obsolete]`:

- `SparkMath.Mul` → `Matrix4.Multiply` / `operator *`
- `CollisionFilter2D.*` → `world.Physics.Layers.*`
- `RenderLayer.*` → `world.RenderLayers.*`
- `SceneSubmit.StandardLit` → `world.Rendering.SubmitStandardLit` or `StandardLitSceneSubmit`

## Reusable gameplay library (`Spark.Scripting.Gameplay`)

| Type | Purpose |
|------|---------|
| `GameplayVectorMath` | Instance math (look-at, quats, wrap angle) — inject or use `Default` |
| `CharacterCameraRig` | Third/first-person camera + WASD walk (port of C++ rig) |
| `ICharacterMovementBounds` | Optional room clamp (implement in your game, e.g. dungeon bounds) |
| `InputKeyCodes` | GLFW key constants |

## Native interop added (non-component)

Recent `SparkInterop` exports aligned with `GameWorld.hpp`:

- `FindGameObjectWithTag` (with `includeInactive`)
- `FindGameObjectByName`
- `SetParent` (world-level)
- `DestroyGameObjectImmediate`
- `PumpAssets`, `IsGltfReady`, `IsSkinnedGltfReady`, pending load count
- `QueryCollectByTag`

Regenerate P/Invoke companions after editing `SparkInterop.h`:

```bash
python3 tools/generate_native_pinvoke_companion.py
```

## Coverage gaps (demos / subsystems — not yet script-bound)

These C++ areas still need `SPARK_SCRIPT_BIND` / interop before C# can mirror them cleanly:

| Area | C++ examples | Status |
|------|----------------|--------|
| **SceneManager** | additive load, prefabs | `world.Scenes` facet |
| **Signals** | `OnSignal` on components | Emit only; no managed `OnSignal` |
| **SceneLoadSession** | async load helper | Use `world.Scenes.BeginLoadAsync` + `Pump` |
| **Game flow / save** | `GameFlowCoordinator`, `GameSave` | No C API |
| **Async asset callbacks** | `OnGltfReady`, `RequestGltf` | Partial (pump + `Is*Ready` only) |
| **Editor / ImGui** | `EditorApplication`, `IImGuiLayer` | Not exposed |
| **Tilemap edit** | `TilemapEditSession`, TMX tools | Components only |
| **Physics 3D queries** | raycast/overlap (non-character) | Limited |
| **GameplayInventoryCollector**, **PhysicsContactListener3D** | components | No interop prefix |
| **Hierarchy signals / tags batch** | full `QueryGameObjects` filters | Tag query only |
| **Subsystems as objects** | `PhysicsSubsystem`, `NavigationSubsystem` | World methods only |

Component mirrors are the strongest layer (~120 types, ~1260 `spark_*` symbols). Subsystems and editor tooling are the main gap for full **SparkDemo** parity.

## Porting checklist (C++ demo → C#)

1. Rebuild **spark-sharp** `SparkInterop` + `Spark.Bindings` after pulling binding changes.
2. Subclass `SparkGame`; use `World` facets in `OnAttach` / `OnUpdate` / `OnRender`.
3. Map C++ `GameWorld::` calls to the matching facet (table above).
4. Map components via `gameObject.GetOrAdd*` / generated properties.
5. For shared camera/math, use `Spark.Scripting.Gameplay` instead of copying into the game project.
6. Run `binding-coverage.json` + `scan-cpp-component-api.py` when adding new `SPARK_SCRIPT_BIND` on engine headers.

## See also

- [Bindings pipeline](BINDINGS_COVERAGE.md) (component coverage)
- [SparkSharp docs site](index.html)
