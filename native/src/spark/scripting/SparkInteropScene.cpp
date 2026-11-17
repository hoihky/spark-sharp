#include "spark/scripting/SparkInterop.h"
#include "spark/scripting/SparkInteropInternal.hpp"

#include "spark/ai/AiSubsystem.hpp"
#include "spark/engine/IEngineContext.hpp"
#include "spark/engine/IInput.hpp"
#include "spark/engine/SceneRenderParams.hpp"
#include "spark/ui/runtime/UiScene.hpp"
#include "spark/render/platform/Window.hpp"
#include "spark/physics/PhysicsSubsystem.hpp"
#include "spark/scene/core/GameWorld.hpp"
#include "spark/scene/core/Scene.hpp"
#include "spark/scene/submit/LitSceneSubmitOptions.hpp"
#include "spark/scene/submit/SceneSubmit.hpp"
#include "spark/scene/texture/Texture2D.hpp"
#include "spark/scene/mesh/Mesh.hpp"
#include "spark/core/SharedPtr.hpp"
#include "spark/core/Utf8String.hpp"
#include "spark/physics/3d/CharacterController3D.hpp"

using namespace Spark::Scripting;

namespace {

Spark::FrameTiming ToCppTiming(const SparkFrameTiming& t) {
    return {
            .deltaTimeSeconds = t.deltaTimeSeconds,
            .totalTimeSeconds = t.totalTimeSeconds,
            .frameIndex = t.frameIndex,
    };
}

Spark::SceneSpriteSortMode ToCppSortMode(const SparkSpriteSortMode mode) {
    return static_cast<Spark::SceneSpriteSortMode>(static_cast<std::uint8_t>(mode));
}

Spark::LitSceneSubmitOptions ToCppLitOptions(
        const SparkLitSceneSubmitOptions& in,
        Spark::IEngineContext* context) {
    Spark::LitSceneSubmitOptions out{};
    out.lightDirectionWorld = ToVector3(in.lightDirectionWorld);
    out.lightColor = ToVector3(in.lightColor);
    out.lightIntensity = in.lightIntensity;
    out.ambientColor = ToVector3(in.ambientColor);
    out.enableParticles = in.enableParticles != 0;
    out.particleCameraRight = ToVector3(in.particleCameraRight);
    out.particleCameraUp = ToVector3(in.particleCameraUp);
    out.sceneTimeSeconds = in.sceneTimeSeconds;
    out.spriteSortMode = ToCppSortMode(in.spriteSortMode);
    if (in.useSceneFrustumCulling != 0 && context != nullptr) {
        out.sceneForCulling = context->TryGetScene();
    }
    return out;
}

void FromLitCameraParams(
        const Spark::SceneRenderParams& params,
        SparkMatrix4* outViewProjection,
        SparkVector3* outCameraPositionWorld,
        SparkVector3* outParticleCameraRight,
        SparkVector3* outParticleCameraUp) {
    if (outViewProjection != nullptr) {
        *outViewProjection = FromMatrix4(params.viewProjection);
    }
    if (outCameraPositionWorld != nullptr) {
        *outCameraPositionWorld = FromVector3(params.cameraPositionWorld);
    }
    if (outParticleCameraRight != nullptr) {
        *outParticleCameraRight = FromVector3(params.particleCameraRight);
    }
    if (outParticleCameraUp != nullptr) {
        *outParticleCameraUp = FromVector3(params.particleCameraUp);
    }
}

}  // namespace

extern "C" {

void spark_world_simulate_game_ai(
        SparkGameWorld* world,
        const SparkFrameTiming* timing,
        SparkEngineContext* context) {
    if (world == nullptr || timing == nullptr || context == nullptr) {
        return;
    }
    Spark::AiSubsystem::RunFrame(
            *reinterpret_cast<Spark::GameWorld*>(world),
            ToCppTiming(*timing),
            *reinterpret_cast<Spark::IEngineContext*>(context));
}

void spark_world_physics_simulate_2d(
        SparkGameWorld* world,
        const SparkFrameTiming* timing,
        const SparkPhysicsWorld2DSettings* settings) {
    if (world == nullptr || timing == nullptr) {
        return;
    }
    Spark::PhysicsWorld2DSettings cpp{};
    if (settings != nullptr) {
        cpp.gravityY = settings->gravityY;
        cpp.maxFallSpeed = settings->maxFallSpeed;
        cpp.resolveDynamicVsDynamic = settings->resolveDynamicVsDynamic != 0;
    }
    Spark::PhysicsSubsystem physics{};
    physics.GetWorld2D().SetSettings(cpp);
    physics.Simulate2D(*reinterpret_cast<Spark::GameWorld*>(world), ToCppTiming(*timing));
}

void spark_world_physics_simulate_3d(SparkGameWorld* world, const SparkFrameTiming* timing) {
    if (world == nullptr || timing == nullptr) {
        return;
    }
    Spark::PhysicsSubsystem physics{};
    physics.Simulate3D(*reinterpret_cast<Spark::GameWorld*>(world), ToCppTiming(*timing));
}

void spark_world_simulate_character_controllers_3d(
        SparkGameWorld* world,
        const SparkFrameTiming* timing,
        const SparkCharacterController3DSettings* settings) {
    if (world == nullptr || timing == nullptr) {
        return;
    }
    Spark::CharacterController3DSettings cpp{};
    if (settings != nullptr) {
        cpp.gravityY = settings->gravityY;
        cpp.maxFallSpeed = settings->maxFallSpeed;
        cpp.slideIterations = settings->slideIterations;
        cpp.sweepBinaryIterations = settings->sweepBinaryIterations;
        cpp.broadPhaseCellSize = settings->broadPhaseCellSize;
    }
    Spark::SimulateCharacterControllers3D(
            *reinterpret_cast<Spark::GameWorld*>(world),
            ToCppTiming(*timing),
            cpp);
}

int spark_world_load_gltf(SparkGameWorld* world, const char* path) {
    if (world == nullptr || path == nullptr) {
        return 0;
    }
    auto* cppWorld = reinterpret_cast<Spark::GameWorld*>(world);
    Spark::GltfAsset asset{};
    return cppWorld->Assets().AwaitGltf(path, asset) ? 1 : 0;
}

int spark_world_load_skinned_gltf(SparkGameWorld* world, const char* path) {
    if (world == nullptr || path == nullptr) {
        return 0;
    }
    auto* cppWorld = reinterpret_cast<Spark::GameWorld*>(world);
    Spark::SkinnedGltfAsset asset{};
    return cppWorld->Assets().AwaitSkinnedGltf(path, asset) ? 1 : 0;
}

int spark_world_load_texture(SparkGameWorld* world, const char* path) {
    if (world == nullptr || path == nullptr) {
        return 0;
    }
    return reinterpret_cast<Spark::GameWorld*>(world)->Assets().LoadTexture(path) ? 1 : 0;
}

void spark_world_pump_assets(SparkGameWorld* world) {
    if (world == nullptr) {
        return;
    }
    reinterpret_cast<Spark::GameWorld*>(world)->Assets().Pump();
}

int spark_world_is_gltf_ready(SparkGameWorld* world, const char* path) {
    if (world == nullptr || path == nullptr) {
        return 0;
    }
    return reinterpret_cast<Spark::GameWorld*>(world)->Assets().IsGltfReady(path) ? 1 : 0;
}

int spark_world_is_skinned_gltf_ready(SparkGameWorld* world, const char* path) {
    if (world == nullptr || path == nullptr) {
        return 0;
    }
    return reinterpret_cast<Spark::GameWorld*>(world)->Assets().IsSkinnedGltfReady(path) ? 1 : 0;
}

std::uint32_t spark_world_get_pending_asset_load_count(SparkGameWorld* world) {
    if (world == nullptr) {
        return 0;
    }
    return static_cast<std::uint32_t>(
            reinterpret_cast<Spark::GameWorld*>(world)->Assets().PendingLoadCount());
}

int spark_world_register_sky_sphere_mesh(
        SparkGameWorld* world,
        const char* cacheKey,
        const float radius,
        const int latitudeSegments,
        const int longitudeSegments) {
    if (world == nullptr || cacheKey == nullptr || cacheKey[0] == '\0') {
        return 0;
    }
    if (latitudeSegments < 2 || longitudeSegments < 3) {
        return 0;
    }
    auto* cppWorld = reinterpret_cast<Spark::GameWorld*>(world);
    Spark::SharedPtr<Spark::Mesh> skyMesh =
            Spark::MakeShared<Spark::Mesh>(Spark::Utf8String("ScriptSkySphere"));
    *skyMesh = Spark::Mesh::CreateSkySphere(radius, latitudeSegments, longitudeSegments);
    cppWorld->Assets().RegisterMesh(skyMesh, cacheKey);
    return 1;
}

int spark_world_register_texture_from_file(
        SparkGameWorld* world,
        const char* filePath,
        const char* cacheKey) {
    if (world == nullptr || filePath == nullptr || filePath[0] == '\0' || cacheKey == nullptr ||
        cacheKey[0] == '\0') {
        return 0;
    }
    Spark::Texture2D decoded{};
    if (!Spark::Texture2D::TryLoadFromFile(filePath, decoded)) {
        return 0;
    }
    auto shared = Spark::MakeShared<Spark::Texture2D>(Spark::MoveTemp(decoded));
    reinterpret_cast<Spark::GameWorld*>(world)->Assets().RegisterTexture(shared, cacheKey);
    reinterpret_cast<Spark::GameWorld*>(world)->Assets().RegisterTexture(shared, filePath);
    return 1;
}

int spark_world_register_checkerboard_texture(
        SparkGameWorld* world,
        const char* cacheKey,
        const uint32_t size,
        const uint32_t tilePixels,
        const SparkVector3* colorA,
        const SparkVector3* colorB) {
    if (world == nullptr || cacheKey == nullptr || colorA == nullptr || colorB == nullptr) {
        return 0;
    }
    Spark::Texture2D tex = Spark::Texture2D::CreateCheckerboard(
            size,
            tilePixels,
            ToVector3(*colorA),
            ToVector3(*colorB));
    auto shared = Spark::MakeShared<Spark::Texture2D>(std::move(tex));
  reinterpret_cast<Spark::GameWorld*>(world)->Assets().RegisterTexture(shared, cacheKey);
    return 1;
}

void spark_scene_fill_standard_lit_from_world(
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
        void* outParams,
        const std::uint32_t outParamsByteSize,
        const SparkSpriteSortMode spriteSortMode) {
    if (world == nullptr || context == nullptr || viewProjection == nullptr || cameraPositionWorld == nullptr ||
        lightDirectionWorld == nullptr || lightColor == nullptr || ambientColor == nullptr ||
        particleCameraRight == nullptr || particleCameraUp == nullptr || outParams == nullptr) {
        return;
    }
    if (outParamsByteSize != sizeof(Spark::SceneRenderParams)) {
        return;
    }
    auto& params = *static_cast<Spark::SceneRenderParams*>(outParams);
    Spark::FillStandardLitSceneFromWorld(
            *reinterpret_cast<Spark::GameWorld*>(world),
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
            ToCppSortMode(spriteSortMode));
}

void spark_scene_submit_standard_lit_from_world(
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
            *reinterpret_cast<Spark::GameWorld*>(world),
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
            ToCppSortMode(spriteSortMode));
    params.ssaoEnabled = ssaoEnabled != 0;
    params.Sanitize();
    reinterpret_cast<Spark::IEngineContext*>(context)->SetSceneRenderParams(params);
}

void spark_scene_fill_standard_lit_from_world_options(
        SparkGameWorld* world,
        SparkEngineContext* context,
        const SparkMatrix4* viewProjection,
        const SparkVector3* cameraPositionWorld,
        const SparkLitSceneSubmitOptions* options,
        void* outParams,
        const std::uint32_t outParamsByteSize) {
    if (world == nullptr || context == nullptr || viewProjection == nullptr || cameraPositionWorld == nullptr ||
        options == nullptr || outParams == nullptr) {
        return;
    }
    if (outParamsByteSize != sizeof(Spark::SceneRenderParams)) {
        return;
    }
    auto& params = *static_cast<Spark::SceneRenderParams*>(outParams);
    auto* ctx = reinterpret_cast<Spark::IEngineContext*>(context);
    Spark::LitSceneSubmitOptions cppOpts = ToCppLitOptions(*options, ctx);
    Spark::FillStandardLitSceneFromWorld(
            *reinterpret_cast<Spark::GameWorld*>(world),
            *ctx,
            ToMatrix4(*viewProjection),
            ToVector3(*cameraPositionWorld),
            cppOpts,
            params);
}

void spark_scene_submit_standard_lit_from_world_options(
        SparkGameWorld* world,
        SparkEngineContext* context,
        const SparkMatrix4* viewProjection,
        const SparkVector3* cameraPositionWorld,
        const SparkLitSceneSubmitOptions* options,
        const int ssaoEnabled) {
    if (world == nullptr || context == nullptr || viewProjection == nullptr || cameraPositionWorld == nullptr ||
        options == nullptr) {
        return;
    }
    auto* ctx = reinterpret_cast<Spark::IEngineContext*>(context);
    Spark::LitSceneSubmitOptions cppOpts = ToCppLitOptions(*options, ctx);
    Spark::SceneRenderParams params{};
    Spark::FillStandardLitSceneFromWorld(
            *reinterpret_cast<Spark::GameWorld*>(world),
            *ctx,
            ToMatrix4(*viewProjection),
            ToVector3(*cameraPositionWorld),
            cppOpts,
            params);
    params.ssaoEnabled = ssaoEnabled != 0;
    params.Sanitize();
    ctx->SetSceneRenderParams(params);
}

int spark_scene_try_resolve_lit_camera(
        const SparkGameWorld* world,
        const int framebufferWidth,
        const int framebufferHeight,
        SparkMatrix4* outViewProjection,
        SparkVector3* outCameraPositionWorld,
        SparkVector3* outParticleCameraRight,
        SparkVector3* outParticleCameraUp) {
    if (world == nullptr || outViewProjection == nullptr || outCameraPositionWorld == nullptr) {
        return 0;
    }
    Spark::SceneRenderParams scratch{};
    if (!Spark::TryFillSceneCameraFromWorld(
                *reinterpret_cast<const Spark::GameWorld*>(world),
                static_cast<float>(framebufferWidth),
                static_cast<float>(framebufferHeight),
                scratch)) {
        return 0;
    }
    FromLitCameraParams(scratch, outViewProjection, outCameraPositionWorld, outParticleCameraRight, outParticleCameraUp);
    return 1;
}

int spark_scene_submit_standard_lit_with_camera(
        SparkGameWorld* world,
        SparkEngineContext* context,
        const SparkLitSceneSubmitOptions* options,
        const int ssaoEnabled) {
    if (world == nullptr || context == nullptr || options == nullptr) {
        return 0;
    }
    auto* ctx = reinterpret_cast<Spark::IEngineContext*>(context);
    int fbW = 0;
    int fbH = 0;
    ctx->GetFramebufferSize(fbW, fbH);
    if (fbW <= 0) {
        fbW = 1;
    }
    if (fbH <= 0) {
        fbH = 1;
    }
    Spark::SceneRenderParams params{};
    if (!Spark::TryFillSceneCameraFromWorld(
                *reinterpret_cast<Spark::GameWorld*>(world),
                static_cast<float>(fbW),
                static_cast<float>(fbH),
                params)) {
        return 0;
    }
    Spark::LitSceneSubmitOptions cppOpts = ToCppLitOptions(*options, ctx);
    if (cppOpts.enableParticles) {
        cppOpts.particleCameraRight = params.particleCameraRight;
        cppOpts.particleCameraUp = params.particleCameraUp;
    }
    Spark::FillStandardLitSceneFromWorld(
            *reinterpret_cast<Spark::GameWorld*>(world),
            *ctx,
            params.viewProjection,
            params.cameraPositionWorld,
            cppOpts,
            params);
    params.ssaoEnabled = ssaoEnabled != 0;
    params.Sanitize();
    ctx->SetSceneRenderParams(params);
    return 1;
}

void spark_ui_process_canvases_input(
        SparkGameWorld* world,
        SparkInput* input,
        const int framebufferWidth,
        const int framebufferHeight) {
    if (world == nullptr || input == nullptr) {
        return;
    }
    Spark::ProcessUiCanvasesInput(
            *reinterpret_cast<Spark::GameWorld*>(world),
            *reinterpret_cast<Spark::IInput*>(input),
            framebufferWidth,
            framebufferHeight);
}

void spark_ui_paint_canvases(
        SparkGameWorld* world,
        void* params,
        const std::uint32_t paramsByteSize,
        const int framebufferWidth,
        const int framebufferHeight) {
    if (world == nullptr || params == nullptr || paramsByteSize != sizeof(Spark::SceneRenderParams)) {
        return;
    }
    Spark::PaintUiCanvases(
            *reinterpret_cast<Spark::GameWorld*>(world),
            *static_cast<Spark::SceneRenderParams*>(params),
            framebufferWidth,
            framebufferHeight);
}

int spark_ui_consumes_game_pointer(void) {
    return Spark::UiConsumesGamePointer() ? 1 : 0;
}

void spark_context_process_ui_input(SparkEngineContext* context) {
    if (context == nullptr) {
        return;
    }
    auto* ctx = reinterpret_cast<Spark::IEngineContext*>(context);
    Spark::Scene* scene = ctx->TryGetScene();
    if (scene == nullptr) {
        return;
    }
    int w = 0;
    int h = 0;
    ctx->GetFramebufferSize(w, h);
    float sx = 1.0F;
    float sy = 1.0F;
    ctx->GetWindow().GetContentScale(sx, sy);
    Spark::ProcessUiCanvasesInput(*scene, ctx->GetInput(), w, h, sx, sy);
}

}  // extern "C"
