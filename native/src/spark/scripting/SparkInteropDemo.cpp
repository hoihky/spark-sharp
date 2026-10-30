#include "spark/scripting/SparkInterop.h"
#include "spark/scripting/SparkInteropInternal.hpp"

#include "spark/config.hpp"
#include "spark/core/Utf8String.hpp"
#include "spark/demo/DemoAssetLoad.hpp"
#include "spark/render/scene/SceneGroundExtent.hpp"
#include "spark/scene/assets/gltf/GltfAssetBindings.hpp"
#include "spark/scene/assets/GameWorldAssetCache.hpp"
#include "spark/ecs/GameObject.hpp"
#include "spark/scene/core/GameWorld.hpp"
#include "spark/scene/mesh/Mesh.hpp"
#include "spark/scene/texture/Texture2D.hpp"

#include <algorithm>
#include <cstring>

using namespace Spark::Scripting;

namespace {

Spark::GameObject* ToObject(SparkGameObject* object) noexcept {
    return reinterpret_cast<Spark::GameObject*>(object);
}

Spark::GameWorld* ToWorld(SparkGameWorld* world) noexcept {
    return reinterpret_cast<Spark::GameWorld*>(world);
}

[[nodiscard]] bool CopyUtf8Label(const Spark::Utf8String& label, char* outLabel, const int labelCap) noexcept {
    if (outLabel == nullptr || labelCap <= 0) {
        return false;
    }
    const char* src = label.CStr();
    if (src == nullptr) {
        outLabel[0] = '\0';
        return true;
    }
    std::strncpy(outLabel, src, static_cast<std::size_t>(labelCap - 1));
    outLabel[labelCap - 1] = '\0';
    return true;
}

[[nodiscard]] bool CopyUtf8Key(const char* key, char* outKey, const int keyCap) noexcept {
    if (outKey == nullptr || keyCap <= 0 || key == nullptr) {
        return false;
    }
    std::strncpy(outKey, key, static_cast<std::size_t>(keyCap - 1));
    outKey[keyCap - 1] = '\0';
    return true;
}

bool TryLoadSkyHdrTexture(
        Spark::GameWorld& w,
        Spark::SharedPtr<Spark::Texture2D>& outTex,
        Spark::Utf8String& outLabel) {
    Spark::Texture2D decoded;
    if (Spark::Texture2D::TryLoadFromFile(SPARK_SKY_TEXTURE_PATH, decoded)) {
        outTex = Spark::MakeShared<Spark::Texture2D>(Spark::MoveTemp(decoded));
        w.RegisterTexture(outTex, "spark/demo/sky_equirect");
        outLabel = Spark::Utf8String("HDR (build default)");
        return true;
    }

    static constexpr const char* kRelativeHdrPaths[] = {
            "/textures/sky/venice_sunset_1k.hdr",
            "/textures/sky/studio_small_08_1k.hdr",
            "/textures/sky/equirect_sky_1k.hdr",
    };
    static constexpr const char* kLabels[] = {
            "venice_sunset_1k",
            "studio_small_08_1k",
            "equirect_sky_1k",
    };
    static constexpr const char* kRoots[] = {SPARK_ASSETS_DIR, SPARK_BUILD_ASSETS_DIR, "assets", nullptr};

    for (std::size_t pi = 0; pi < sizeof(kRelativeHdrPaths) / sizeof(kRelativeHdrPaths[0]); ++pi) {
        for (std::size_t ri = 0; kRoots[ri] != nullptr; ++ri) {
            Spark::Utf8String path(kRoots[ri]);
            path.AppendUtf8(kRelativeHdrPaths[pi]);
            if (Spark::Texture2D::TryLoadFromFile(path.CStr(), decoded)) {
                outTex = Spark::MakeShared<Spark::Texture2D>(Spark::MoveTemp(decoded));
                w.RegisterTexture(outTex, "spark/demo/sky_equirect");
                outLabel = Spark::Utf8String(kLabels[pi]);
                return true;
            }
        }
    }
    return false;
}

}  // namespace

extern "C" {

int spark_world_register_three_d_demo_textures(
        SparkGameWorld* world,
        char* outBrickTextureKey,
        const int brickKeyCap) {
    if (world == nullptr) {
        return 0;
    }
    Spark::GameWorld& w = *ToWorld(world);

    auto checkerTex = Spark::MakeShared<Spark::Texture2D>(Spark::Utf8String("DemoGroundTex"));
    if (!Spark::DemoAssets::TryLoadGroundDirtTexture(*checkerTex)) {
        *checkerTex = Spark::Texture2D::CreateCheckerboard(
                256,
                32,
                Spark::Vector3{0.92F, 0.86F, 0.72F},
                Spark::Vector3{0.25F, 0.35F, 0.55F});
        checkerTex->GetName() = Spark::Utf8String("DemoCheckerFallback");
    }
    w.RegisterTexture(checkerTex, "spark/demo/checker");

    auto brickTex = Spark::MakeShared<Spark::Texture2D>(Spark::Utf8String("BrickPattern"));
    const char* brickKey = "spark/demo/brick_fallback";
    if (!Spark::DemoAssets::TryLoadBrickTexture(*brickTex)) {
        *brickTex = Spark::Texture2D::CreateBrickPattern(256, 256);
        w.RegisterTexture(brickTex, "spark/demo/brick_fallback");
    } else {
        w.RegisterTexture(brickTex, "spark/demo/bricks");
        brickKey = "spark/demo/bricks";
    }

    static_cast<void>(CopyUtf8Key(brickKey, outBrickTextureKey, brickKeyCap));
    return 1;
}

int spark_world_register_sky_demo_ground(
        SparkGameWorld* world,
        char* outTextureKey,
        const int textureKeyCap,
        float* outRoughness) {
    if (world == nullptr || outRoughness == nullptr) {
        return 0;
    }
    Spark::GameWorld& w = *ToWorld(world);

    auto groundDiffTex = Spark::MakeShared<Spark::Texture2D>(Spark::Utf8String("SkyDemoGroundDiffuse"));
    const char* textureKey = "spark/demo/ground_baked";
    if (Spark::DemoAssets::TryLoadTerrainDemoSoilTexture(*groundDiffTex)) {
        w.RegisterTexture(groundDiffTex, "spark/demo/ground_soil");
        textureKey = "spark/demo/ground_soil";
        *outRoughness = 0.93F;
    } else {
        const Spark::DemoAssets::RandomSoilGroundResult baked =
                Spark::DemoAssets::BuildJitteredSoilWithGrassPatchesTexture(768U, 0x5F3759DFU);
        *groundDiffTex = Spark::MoveTemp(baked.texture);
        w.RegisterTexture(groundDiffTex, "spark/demo/ground_baked");
        *outRoughness = 0.91F;
    }

    const float groundWorldSpan =
            Spark::DemoAssets::ProceduralTextureSpanWorldUnits(Spark::kSceneGroundHalfExtent);
    auto groundAsset = Spark::MakeShared<Spark::Mesh>(Spark::Utf8String("SkyDemoGround"));
    *groundAsset = Spark::Mesh::CreateGroundPlane(Spark::kSceneGroundHalfExtent, groundWorldSpan);
    w.RegisterMesh(groundAsset, "spark/demo/ground");

    static_cast<void>(CopyUtf8Key(textureKey, outTextureKey, textureKeyCap));
    return 1;
}

int spark_demo_try_register_sky_hdr(SparkGameWorld* world, char* outLabel, const int labelCap) {
    if (world == nullptr) {
        return 0;
    }
    Spark::GameWorld& w = *ToWorld(world);
    auto skyEquirectTex = Spark::MakeShared<Spark::Texture2D>(Spark::Utf8String("SkyEquirect"));
    Spark::Utf8String label;
    if (!TryLoadSkyHdrTexture(w, skyEquirectTex, label)) {
        return 0;
    }
    static_cast<void>(CopyUtf8Label(label, outLabel, labelCap));
    return 1;
}

int spark_gltf_try_measure_rigid_layout(
        SparkGameWorld* world,
        const char* gltfPath,
        const float targetMaxExtent,
        const float defaultUniformScale,
        float* outUniformScale,
        float* outBoundsMinY) {
    if (world == nullptr || gltfPath == nullptr || outUniformScale == nullptr || outBoundsMinY == nullptr) {
        return 0;
    }
    Spark::GltfAsset asset{};
    if (!ToWorld(world)->AwaitGltf(gltfPath, asset) || !asset.mesh) {
        return 0;
    }

    float uniformScale = defaultUniformScale;
    Spark::Vector3 boundsMin{};
    Spark::Vector3 boundsMax{};
    if (asset.mesh->TryComputeAxisAlignedBounds(boundsMin, boundsMax)) {
        const float dx = boundsMax.x - boundsMin.x;
        const float dy = boundsMax.y - boundsMin.y;
        const float dz = boundsMax.z - boundsMin.z;
        const float maxExt = std::max({dx, dy, dz});
        if (maxExt > 1.0e-4F) {
            uniformScale = targetMaxExtent / maxExt;
        }
        *outBoundsMinY = boundsMin.y;
    } else {
        *outBoundsMinY = 0.0F;
    }
    *outUniformScale = uniformScale;
    return 1;
}

int spark_object_bind_rigid_gltf_mesh(
        SparkGameObject* object,
        SparkGameWorld* world,
        const char* gltfPath) {
    if (object == nullptr || world == nullptr || gltfPath == nullptr) {
        return 0;
    }
    Spark::GltfAsset asset{};
    if (!ToWorld(world)->AwaitGltf(gltfPath, asset) || !asset.mesh) {
        return 0;
    }
    Spark::GltfAssetBinder::BindRigidMesh(
            *ToObject(object),
            asset,
            Spark::SceneMeshSlot::Custom,
            Spark::Vector3{1.0F, 1.0F, 1.0F},
            gltfPath);
    return 1;
}

int spark_object_bind_skinned_gltf_mesh(
        SparkGameObject* object,
        SparkGameWorld* world,
        const char* gltfPath) {
    if (object == nullptr || world == nullptr || gltfPath == nullptr) {
        return 0;
    }
    const Spark::SkinnedGltfAsset asset = ToWorld(world)->LoadSkinnedGltf(gltfPath);
    if (!asset.mesh) {
        return 0;
    }
    Spark::GltfAssetBinder::BindSkinnedMesh(
            *ToObject(object), asset, Spark::Vector3{1.0F, 1.0F, 1.0F}, gltfPath);
    return 1;
}

}  // extern "C"
