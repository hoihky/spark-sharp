# C# script API (spark-sharp)

This document maps the **Spark C++ engine API** to **Spark.Bindings** / **Spark.Scripting**, notes coverage gaps for demo ports, and describes the **object-oriented facet layout** (SOLID-friendly, minimal static surface).

## Recommended entry points

| C++ concept | C# type | Notes |
|-------------|---------|--------|
| `Spark::Game` | `Spark.Bindings.Game` / `Spark.Scripting.SparkGame` | Override lifecycle hooks |
| `Spark::IEngineContext` | `IEngineContext` | Input, scene, window |
| `Spark::Scene` | `Scene` | `GetWorld()` |
| `Spark::GameWorld` | `GameWorld` | Use **facets** below (preferred; mirrors C++ `Entities()` / `Simulation()` / …) |
| `Spark::LitScene` | `LitScene` + `LitScene.Builder` | Options-based submit; matches `LitSceneSubmitter` pipeline |
| `Spark::LitSceneSubmitOptions` | `LitSceneSubmitOptions` | Fluent `With*` helpers; `UseSceneFrustumCulling` for spatial cull |
| `Spark::ScenePartitionKind` | `ScenePartitionKind` | On `SceneSpatialPolicyComponent.Partition` |
| `Spark::AiSubsystem` | `WorldSimulation.RunAi` | Full frame via `AiSubsystem::RunFrame` in native |
| `Spark::SoundSubsystem` | `WorldSimulation.ProcessAudio` | `SoundSubsystem::ProcessFrame` in native |
| `Spark::GameObject` | `GameObject` | Hierarchy + `Add*` / `Get*` components |
| Components | `Spark.Bindings.Components.*` | ~120 generated mirrors |

### GameWorld facets (instance API)

Prefer facet properties over flat `GameWorld` methods and static helpers:

```csharp
GameWorld world = context.TryGetScene()!.GetWorld();

world.Entities.Create("Player");
world.Entities.SetParent(child, parent);
world.Entities.FindWithTag("Enemy", includeInactive: false);
world.Assets.LoadGltf(path);
world.Assets.Pump();
world.Physics.Simulate3D(timing);
world.Physics.Layers.DefaultQueryFilter();
world.Simulation.RunDefaultSlice(timing, context); // or UpdateGameObjects + ProcessVfx + ProcessAudio
world.Rendering.SubmitStandardLit(context, viewProj, ...);
world.RenderLayers.Register("Foreground", 100);
world.Query.CollectWithTag("Pickup", buffer);
world.Entities.Query.CollectFresh(pickups, GameObjectQueryFilter.WithTag("Pickup"));
var all = world.Query.Query(GameObjectQueryFilter.WithComponent(SparkComponentKind.Mesh));
world.Scenes.BeginLoadAsync("levels/hub.sparkscene");
world.Scenes.Pump();
player.EmitSignal(SignalId.UserSignal(0));

var listener = goalTrigger.GetOrAddScriptSignalListener();
listener.ListenSignal = SignalId.Physics2DTriggerEnter;
listener.SetCallback((owner, id, payload) =>
{
    var other = payload.TryGetObjectFromPointer();
});
```

| Facet | Mirrors C++ |
|-------|-------------|
| `Entities` | Create/destroy/find/reparent, immediate destroy |
| `Assets` | Load/register GLTF & textures, pump, readiness (`WorldAssets` / C++ `world.Assets()`) |
| `Physics` | 2D/3D step, character controllers, overlap queries, layer masks |
| `Simulation` | `UpdateGameObjects`, VFX, sound, AI |
| `Rendering` | Standard-lit submit / param fill |
| `RenderLayers` | `RenderLayerRegistry` |
| `Query` | `WorldQuery`: `CollectWithTag`, `CollectWithComponent`, `Collect(filter, span)`, `Query(filter)`, `FindWithTag` / `FindByName` |
| `Scenes` | `SceneManager` load/unload, spawn points, prefabs |

`SparkGame` exposes the same facets as `protected` members (`Entities`, `Assets`, …).

### Signals

| C++ | C# |
|-----|-----|
| `GameObject::EmitSignal` | `gameObject.EmitSignal(SignalId, …)` |
| `GameComponent::OnSignal` | `gameObject.GetOrAddScriptSignalListener().SetCallback(...)` |
| `SignalId` / `UserBase + n` | `SignalId` enum / `GameObject.UserSignal(offset)` |

### Deprecated static helpers

These remain for compatibility but are marked `[Obsolete]`:

- `SparkMath.Mul` → `Matrix4.Multiply` / `operator *`
- `CollisionFilter2D.*` → `world.Physics.Layers.*`
- `RenderLayer.*` → `world.RenderLayers.*`
- `GameWorld.LoadGltf` / `LoadTexture` / `AssetCache` → `world.Assets.*`
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
- `Assets.Pump`, `Assets.IsGltfReady`, `Assets.IsSkinnedGltfReady`, `Assets.PendingLoadCount` (C ABI: `spark_world_pump_assets`, …)
- `QueryCollectByTag`

Regenerate P/Invoke companions after editing `SparkInterop.h`:

```bash
python3 tools/generate_native_pinvoke_companion.py
```

## Coverage gaps (demos / subsystems — not yet script-bound)

These C++ areas still need catalog + interop entries before C# can mirror them cleanly:

| Area | C++ examples | Status |
|------|----------------|--------|
| **SceneManager** | additive load, prefabs | `world.Scenes` facet |
| **Signals** | `OnSignal` on components | `ScriptSignalListenerComponent` + `SetCallback` |
| **SceneLoadSession** | async load helper | Use `world.Scenes.BeginLoadAsync` + `Pump` |
| **Game flow / save** | `GameFlowCoordinator`, `GameSave` | No C API |
| **Async asset callbacks** | `OnGltfReady`, `RequestGltf` | Partial (pump + `Is*Ready` only) |
| **Editor / ImGui** | `EditorApplication`, `IImGuiLayer` | Not exposed |
| **Tilemap edit** | `TilemapEditSession`, TMX tools | Components only |
| **Scene submit internals** | `LitSceneSubmitter`, `SceneDrawListSorter`, `spark/scene/spatial/*` | Run inside native fill; not script API |
| **Physics 3D queries** | raycast/overlap (non-character) | Limited |
| **GameplayInventoryCollector**, **PhysicsContactListener3D** | components | No interop prefix |
| **Hierarchy signals / tags batch** | full `QueryGameObjects` filters | Tag query only |
| **Subsystems as objects** | `PhysicsSubsystem`, `NavigationSubsystem` | World methods only |

Component mirrors are the strongest layer (~120 types, ~1260 `spark_*` symbols). Subsystems and editor tooling are the main gap for full **SparkDemo** parity.

## Lit scene submit (C++ refactor parity)

```csharp
var opts = LitSceneSubmitOptions.Default3D
    .WithDirectionalLight(
        new Vector3 { X = 0.35f, Y = 0.92f, Z = 0.18f },
        new Vector3 { X = 1f, Y = 0.97f, Z = 0.9f },
        0.92f)
    .WithSceneFrustumCulling(true)
    .WithSpriteSortMode(SparkSpriteSortMode.SparkSpriteSortMode_SortOrderThenWorldY);

// Spatial policy on a GameObject (Octree → native OctTree):
worldObject.GetOrAddSceneSpatialPolicy()!.Partition = ScenePartitionKind.Octree;

new LitScene.Builder(world, context)
    .WithCameraFromWorld()
    .WithOptions(opts)
    .Submit();

// Or facet + auto camera:
LitScene.SubmitWithCamera(world, context, opts);
```

Positional overloads (`world.Rendering.SubmitStandardLit` with many vectors) still call the same native `LitSceneSubmitter` path.

**`Vector3`:** generated bindings expose `X` / `Y` / `Z` properties only — use object initializers in options and lighting helpers, not a three-float constructor.

**Paths / privacy:** set `SPARK_ENGINE_ROOT` to your engine checkout; run `./tools/check-paths-privacy.sh` before push ([PATHS_AND_PRIVACY.md](PATHS_AND_PRIVACY.md)). Engine factoring log: [SCENE_SUBMIT_AND_SPATIAL_REFACTOR](https://github.com/hoihky/spark/blob/main/docs/SCENE_SUBMIT_AND_SPATIAL_REFACTOR.md).

## Porting checklist (C++ demo → C#)

1. Rebuild **spark-sharp** `SparkInterop` + `Spark.Bindings` after pulling binding changes.
2. Subclass `SparkGame`; use `World` facets in `OnAttach` / `OnUpdate` / `OnRender`.
3. Map C++ `GameWorld::` calls to the matching facet (table above).
4. Map components via `gameObject.GetOrAdd*` / generated properties.
5. For shared camera/math, use `Spark.Scripting.Gameplay` instead of copying into the game project.
6. Run `binding-coverage.json` + `scan-cpp-component-api.py` when adding catalog entries for new component methods.

## See also

- [Component script catalog](../scripting/bindings/COMPONENT_SCRIPT_CATALOG.md)
- [Spark engine: bindings coverage](https://github.com/hoihky/spark/blob/main/docs/BINDINGS_COVERAGE.md) (audit gaps)
- [SparkSharp docs site](index.html)
