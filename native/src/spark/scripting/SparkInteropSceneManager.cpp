#include "spark/scripting/SparkInterop.h"
#include "spark/scripting/SparkInteropInternal.hpp"

#include "spark/ecs/GameObject.hpp"
#include "spark/scene/core/GameWorld.hpp"
#include "spark/scene/core/SceneManager.hpp"

#include <algorithm>
#include <new>

namespace {

struct SparkSceneManagerImpl {
    Spark::GameWorld* world = nullptr;
    Spark::SceneManager sceneMgr;

    explicit SparkSceneManagerImpl(Spark::GameWorld& inWorld) : world(&inWorld), sceneMgr(inWorld) {}
};

Spark::SceneLoadOptions ToSceneLoadOptions(const SparkSceneLoadOptions* options) {
    Spark::SceneLoadOptions out{};
    if (options == nullptr) {
        return out;
    }
    out.assetsRoot = options->assetsRoot;
    out.additive = options->additive != 0;
    if (options->sceneNameUtf8 != nullptr) {
        out.sceneName = Spark::Utf8String(options->sceneNameUtf8);
    }
    return out;
}

SparkSceneManagerImpl* AsManager(SparkSceneManager* manager) {
    return reinterpret_cast<SparkSceneManagerImpl*>(manager);
}

}  // namespace

extern "C" {

SparkSceneManager* spark_scene_manager_create(SparkGameWorld* world) {
    if (world == nullptr) {
        return nullptr;
    }
    auto* cppWorld = reinterpret_cast<Spark::GameWorld*>(world);
    return reinterpret_cast<SparkSceneManager*>(new (std::nothrow) SparkSceneManagerImpl(*cppWorld));
}

void spark_scene_manager_destroy(SparkSceneManager* manager) {
    delete AsManager(manager);
}

uint64_t spark_scene_manager_load_from_file(
        SparkSceneManager* manager,
        const char* path,
        const SparkSceneLoadOptions* options) {
    SparkSceneManagerImpl* mgr = AsManager(manager);
    if (mgr == nullptr || path == nullptr) {
        return static_cast<uint64_t>(Spark::kInvalidSceneInstanceId);
    }
    const Spark::SceneInstanceId id = mgr->sceneMgr.LoadSceneFromFile(path, ToSceneLoadOptions(options));
    return static_cast<uint64_t>(id);
}

uint64_t spark_scene_manager_begin_load_async(
        SparkSceneManager* manager,
        const char* path,
        const SparkSceneLoadOptions* options) {
    SparkSceneManagerImpl* mgr = AsManager(manager);
    if (mgr == nullptr || path == nullptr) {
        return static_cast<uint64_t>(Spark::kInvalidSceneInstanceId);
    }
    const Spark::SceneInstanceId id = mgr->sceneMgr.BeginLoadSceneAsync(path, ToSceneLoadOptions(options));
    return static_cast<uint64_t>(id);
}

void spark_scene_manager_pump(SparkSceneManager* manager) {
    SparkSceneManagerImpl* mgr = AsManager(manager);
    if (mgr == nullptr) {
        return;
    }
    mgr->sceneMgr.Pump();
}

int spark_scene_manager_is_scene_ready(SparkSceneManager* manager, const uint64_t instanceId) {
    SparkSceneManagerImpl* mgr = AsManager(manager);
    if (mgr == nullptr) {
        return 0;
    }
    return mgr->sceneMgr.IsSceneReady(static_cast<Spark::SceneInstanceId>(instanceId)) ? 1 : 0;
}

int spark_scene_manager_has_scene_failed(SparkSceneManager* manager, const uint64_t instanceId) {
    SparkSceneManagerImpl* mgr = AsManager(manager);
    if (mgr == nullptr) {
        return 0;
    }
    return mgr->sceneMgr.HasSceneFailed(static_cast<Spark::SceneInstanceId>(instanceId)) ? 1 : 0;
}

void spark_scene_manager_unload_scene(SparkSceneManager* manager, const uint64_t instanceId) {
    SparkSceneManagerImpl* mgr = AsManager(manager);
    if (mgr == nullptr) {
        return;
    }
    mgr->sceneMgr.UnloadScene(static_cast<Spark::SceneInstanceId>(instanceId));
}

void spark_scene_manager_unload_all_scenes(SparkSceneManager* manager) {
    SparkSceneManagerImpl* mgr = AsManager(manager);
    if (mgr == nullptr) {
        return;
    }
    mgr->sceneMgr.UnloadAllScenes();
}

uint32_t spark_scene_manager_get_loaded_scene_count(SparkSceneManager* manager) {
    SparkSceneManagerImpl* mgr = AsManager(manager);
    if (mgr == nullptr) {
        return 0;
    }
    return static_cast<uint32_t>(mgr->sceneMgr.GetLoadedSceneCount());
}

uint64_t spark_scene_manager_load_at_spawn_point(
        SparkSceneManager* manager,
        const char* scenePath,
        const char* spawnPointName,
        SparkSceneSpawnPose* outSpawnPose,
        const SparkSceneLoadOptions* options) {
    SparkSceneManagerImpl* mgr = AsManager(manager);
    if (mgr == nullptr || scenePath == nullptr) {
        if (outSpawnPose != nullptr) {
            *outSpawnPose = SparkSceneSpawnPose{};
        }
        return static_cast<uint64_t>(Spark::kInvalidSceneInstanceId);
    }
    Spark::SceneSpawnPose cppPose{};
    const Spark::SceneInstanceId id = mgr->sceneMgr.LoadSceneAtSpawnPoint(
            scenePath,
            spawnPointName,
            outSpawnPose != nullptr ? &cppPose : nullptr,
            ToSceneLoadOptions(options));
    if (outSpawnPose != nullptr) {
        outSpawnPose->found = cppPose.found ? 1 : 0;
        outSpawnPose->position = Spark::Scripting::FromVector3(cppPose.position);
        outSpawnPose->rotation = Spark::Scripting::FromQuaternion(cppPose.rotation);
        outSpawnPose->spawnObject = reinterpret_cast<SparkGameObject*>(cppPose.object);
    }
    return static_cast<uint64_t>(id);
}

int spark_scene_manager_instantiate_prefab(
        SparkSceneManager* manager,
        const char* prefabPath,
        SparkGameObject* parent,
        const SparkVector3* position,
        const SparkQuaternion* rotation,
        const char* assetsRoot,
        const int additive,
        const int pumpUntilReady,
        SparkPrefabInstantiateResult* outResult,
        SparkGameObject** outRoots,
        const uint32_t outRootsCapacity) {
    SparkSceneManagerImpl* mgr = AsManager(manager);
    if (mgr == nullptr || prefabPath == nullptr) {
        return 0;
    }

    Spark::PrefabInstantiateOptions opts{};
    opts.parent = reinterpret_cast<Spark::GameObject*>(parent);
    if (position != nullptr) {
        opts.position = Spark::Scripting::ToVector3(*position);
    }
    if (rotation != nullptr) {
        opts.rotation = Spark::Scripting::ToQuaternion(*rotation);
    }
    opts.assetsRoot = assetsRoot;
    opts.additive = additive != 0;
    opts.pumpUntilReady = pumpUntilReady != 0;

    const Spark::PrefabInstantiateResult result = mgr->sceneMgr.InstantiatePrefab(prefabPath, opts);

    if (outResult != nullptr) {
        outResult->instanceId = static_cast<uint64_t>(result.instanceId);
        outResult->ready = result.ready ? 1 : 0;
        outResult->rootCount = static_cast<uint32_t>(result.rootObjects.GetSize());
    }

    if (outRoots != nullptr && outRootsCapacity > 0) {
        const std::size_t n = std::min(
                static_cast<std::size_t>(outRootsCapacity),
                result.rootObjects.GetSize());
        for (std::size_t i = 0; i < n; ++i) {
            outRoots[i] = reinterpret_cast<SparkGameObject*>(result.rootObjects[i]);
        }
    }

    return 1;
}

}  // extern "C"
