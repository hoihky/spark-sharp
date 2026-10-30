#include "spark/scripting/SparkInterop.h"
#include "spark/scripting/SparkInteropInternal.hpp"

#include "spark/demo/DemoAssetLoad.hpp"
#include "spark/demo/ShellDemoSceneUtil.hpp"
#include "spark/ecs/GameObject.hpp"
#include "spark/ecs/components/rendering/MaterialComponent.hpp"
#include "spark/ecs/components/rendering/TerrainComponent.hpp"
#include "spark/engine/IEngineContext.hpp"
#include "spark/engine/SceneRenderParams.hpp"
#include "spark/render/lighting/SceneLightingProfile.hpp"
#include "spark/scene/core/GameWorld.hpp"
#include "spark/scene/mesh/TerrainGeneratorSettings.hpp"
#include "spark/scene/submit/SceneSubmit.hpp"
#include "spark/scene/texture/Texture2D.hpp"

using namespace Spark::Scripting;

namespace {

Spark::GameObject* ToObject(SparkGameObject* object) noexcept {
    return reinterpret_cast<Spark::GameObject*>(object);
}

Spark::GameWorld* ToWorld(SparkGameWorld* world) noexcept {
    return reinterpret_cast<Spark::GameWorld*>(world);
}

Spark::TerrainComponent* ToTerrain(SparkGameComponent* component) noexcept {
    return AsComponent<Spark::TerrainComponent>(component, Spark::ComponentKind::Terrain);
}

}  // namespace

extern "C" {

int spark_world_register_terrain_demo_ground(SparkGameWorld* world) {
    if (world == nullptr) {
        return 0;
    }
    Spark::GameWorld& w = *ToWorld(world);
    auto groundTex = Spark::MakeShared<Spark::Texture2D>(Spark::Utf8String("TerrainGround"));
    if (!Spark::DemoAssets::TryLoadTerrainDemoSoilTexture(*groundTex)) {
        *groundTex = Spark::Texture2D::CreateVariedSoilGroundPattern(1024, 1024);
    }
    w.RegisterTexture(groundTex, "spark/terrain/ground");
    return 1;
}

SparkGameComponent* spark_object_add_terrain_with_settings(
        SparkGameObject* object,
        const SparkTerrainGeneratorSettings* settings,
        const SparkVector3* meshAlbedoRgb) {
    if (object == nullptr || settings == nullptr || meshAlbedoRgb == nullptr) {
        return nullptr;
    }
    Spark::TerrainGeneratorSettings ts{};
    ts.subdivX = settings->subdivX;
    ts.subdivZ = settings->subdivZ;
    ts.halfExtentX = settings->halfExtentX;
    ts.halfExtentZ = settings->halfExtentZ;
    ts.heightScale = settings->heightScale;
    ts.noiseScale = settings->noiseScale;
    ts.octaves = settings->octaves;
    ts.persistence = settings->persistence;
    ts.lacunarity = settings->lacunarity;
    ts.seed = settings->seed;
    ts.worldUnitsPerTextureRepeat = settings->worldUnitsPerTextureRepeat;
    return reinterpret_cast<SparkGameComponent*>(ToObject(object)->AddComponent<Spark::TerrainComponent>(
            ts, ToVector3(*meshAlbedoRgb)));
}

void spark_terrain_reset_heights_to_procedural(
        SparkGameComponent* terrain,
        SparkGameObject* owner) {
    if (terrain == nullptr || owner == nullptr) {
        return;
    }
    Spark::TerrainComponent* t = ToTerrain(terrain);
    if (t != nullptr) {
        t->ResetHeightsToProcedural(*ToObject(owner));
    }
}

int spark_terrain_try_raycast_world(
        SparkGameComponent* terrain,
        SparkGameObject* owner,
        const SparkVector3* rayOriginWorld,
        const SparkVector3* rayDirWorld,
        const float maxDistance,
        SparkVector3* outHitWorld) {
    if (terrain == nullptr || owner == nullptr || rayOriginWorld == nullptr || rayDirWorld == nullptr ||
        outHitWorld == nullptr) {
        return 0;
    }
    Spark::TerrainComponent* t = ToTerrain(terrain);
    if (t == nullptr) {
        return 0;
    }
    Spark::Vector3 hit{};
    if (!t->TryRaycastWorld(
                *ToObject(owner),
                ToVector3(*rayOriginWorld),
                ToVector3(*rayDirWorld),
                maxDistance,
                hit)) {
        return 0;
    }
    *outHitWorld = FromVector3(hit);
    return 1;
}

int spark_demo_terrain_screen_to_world_ray(
        const int fbW,
        const int fbH,
        const float px,
        const float py,
        const SparkMatrix4* invViewProj,
        SparkVector3* outOrigin,
        SparkVector3* outDir) {
    if (invViewProj == nullptr || outOrigin == nullptr || outDir == nullptr) {
        return 0;
    }
    Spark::Vector3 ro{};
    Spark::Vector3 rd{};
    if (!Spark::TerrainScreenToWorldRay(fbW, fbH, px, py, ToMatrix4(*invViewProj), ro, rd)) {
        return 0;
    }
    *outOrigin = FromVector3(ro);
    *outDir = FromVector3(rd);
    return 1;
}

void spark_scene_submit_terrain_demo_from_world(
        SparkGameWorld* world,
        SparkEngineContext* context,
        const SparkMatrix4* viewProjection,
        const SparkVector3* cameraPositionWorld,
        const SparkVector3* lightDirectionWorld,
        const SparkVector3* lightColor,
        const float lightIntensity,
        const SparkVector3* ambientColor,
        const int enableParticles,
        const SparkVector3* particleCameraRight,
        const SparkVector3* particleCameraUp,
        const float sceneTimeSeconds,
        const SparkSpriteSortMode spriteSortMode,
        const int ssaoEnabled) {
    if (world == nullptr || context == nullptr || viewProjection == nullptr || cameraPositionWorld == nullptr ||
        lightDirectionWorld == nullptr || lightColor == nullptr || ambientColor == nullptr ||
        particleCameraRight == nullptr || particleCameraUp == nullptr) {
        return;
    }
    Spark::SceneRenderParams params{};
    Spark::FillStandardLitSceneFromWorld(
            *ToWorld(world),
            *reinterpret_cast<Spark::IEngineContext*>(context),
            ToMatrix4(*viewProjection),
            ToVector3(*cameraPositionWorld),
            ToVector3(*lightDirectionWorld),
            ToVector3(*lightColor),
            lightIntensity,
            ToVector3(*ambientColor),
            enableParticles != 0,
            ToVector3(*particleCameraRight),
            ToVector3(*particleCameraUp),
            sceneTimeSeconds,
            params,
            static_cast<Spark::SceneSpriteSortMode>(static_cast<std::uint8_t>(spriteSortMode)));
    params.lightingProfile = Spark::SceneLightingProfile::Outdoor;
    params.useTimeOfDay = true;
    params.timeOfDay = 0.48F;
    params.shadowCascadeFar = 900.0F;
    params.worldClearColorEnabled = true;
    params.worldClearColor = {0.42F, 0.62F, 0.92F};
    params.ssaoEnabled = ssaoEnabled != 0;
    params.Sanitize();
    reinterpret_cast<Spark::IEngineContext*>(context)->SetSceneRenderParams(params);
}

}  // extern "C"
