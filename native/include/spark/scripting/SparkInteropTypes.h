#pragma once

/**
 * Blittable types mirroring Spark C++ math / scene enums (layout must match managed mirrors).
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SparkVector2 {
    float x;
    float y;
} SparkVector2;

typedef struct SparkVector3 {
    float x;
    float y;
    float z;
} SparkVector3;

typedef struct SparkVector4 {
    float x;
    float y;
    float z;
    float w;
} SparkVector4;

typedef struct SparkQuaternion {
    float x;
    float y;
    float z;
    float w;
} SparkQuaternion;

/** Column-major 4x4 — mirrors Spark::Matrix4. */
typedef struct SparkMatrix4 {
    float m[16];
} SparkMatrix4;

/** Mirrors SceneMeshSlot. */
typedef enum SparkSceneMeshSlot {
    SparkSceneMeshSlot_UnitCube = 0,
    SparkSceneMeshSlot_GroundPlane = 1,
    SparkSceneMeshSlot_Custom = 2,
} SparkSceneMeshSlot;

/** Mirrors SceneSkyMode. */
typedef enum SparkSceneSkyMode {
    SparkSceneSkyMode_None = 0,
    SparkSceneSkyMode_Box = 1,
    SparkSceneSkyMode_Dome = 2,
    SparkSceneSkyMode_Plane = 3,
} SparkSceneSkyMode;

/** Mirrors SceneSpriteSortMode. */
typedef enum SparkSpriteSortMode {
    SparkSpriteSortMode_SortOrderOnly = 0,
    SparkSpriteSortMode_SortOrderThenWorldY = 1,
} SparkSpriteSortMode;

/** Mirrors Spark::GameFlowState. */
typedef enum SparkGameFlowState {
    SparkGameFlowState_Intro = 0,
    SparkGameFlowState_Playing = 1,
    SparkGameFlowState_Paused = 2,
    SparkGameFlowState_Victory = 3,
    SparkGameFlowState_Defeat = 4,
} SparkGameFlowState;

/** Mirrors Spark::GameFlowTriggerSource. */
typedef enum SparkGameFlowTriggerSource {
    SparkGameFlowTriggerSource_TriggerEnter = 0,
    SparkGameFlowTriggerSource_Died = 1,
    SparkGameFlowTriggerSource_OnGameState = 2,
} SparkGameFlowTriggerSource;

/** Mirrors Spark::ParallaxAxisMode. */
typedef enum SparkParallaxAxisMode {
    SparkParallaxAxisMode_Horizontal = 0,
    SparkParallaxAxisMode_Both = 1,
} SparkParallaxAxisMode;

/** Mirrors Spark::ParallaxDriftMode. */
typedef enum SparkParallaxDriftMode {
    SparkParallaxDriftMode_None = 0,
    SparkParallaxDriftMode_SineHorizontal = 1,
} SparkParallaxDriftMode;

/** Mirrors RigidbodyBodyType2D. */
typedef enum SparkRigidbodyBodyType2D {
    SparkRigidbodyBodyType2D_Kinematic = 0,
    SparkRigidbodyBodyType2D_Static = 1,
    SparkRigidbodyBodyType2D_Dynamic = 2,
} SparkRigidbodyBodyType2D;

/** Mirrors PhysicsWorld2DSettings. */
typedef struct SparkPhysicsWorld2DSettings {
    float gravityY;
    float maxFallSpeed;
    int resolveDynamicVsDynamic;
} SparkPhysicsWorld2DSettings;

/** Mirrors Spark::Camera2D (subset for scripting). */
typedef struct SparkCamera2D {
    SparkVector3 position;
    float rotationRad;
    float halfExtentY;
    float clipNearZ;
    float clipFarZ;
} SparkCamera2D;

/** Mirrors Spark::SpriteAnimationClip. */
typedef struct SparkSpriteAnimationClip {
    uint32_t firstFrame;
    uint32_t frameCount;
    float framesPerSecond;
    int loop;
} SparkSpriteAnimationClip;

/** Mirrors Spark::AnimLoopMode. */
typedef enum SparkAnimLoopMode {
    SparkAnimLoopMode_Loop = 0,
    SparkAnimLoopMode_Once = 1,
    SparkAnimLoopMode_Hold = 2,
} SparkAnimLoopMode;

/** Mirrors Spark::Sprite2DAnimLocomotionSource. */
typedef enum SparkSprite2DAnimLocomotionSource {
    SparkSprite2DAnimLocomotionSource_HorizontalAbsVelX = 0,
    SparkSprite2DAnimLocomotionSource_SpeedSq = 1,
} SparkSprite2DAnimLocomotionSource;

/** Result of <c>spark_world_register_platformer2d_demo_textures</c> (Kenney paths under assets/). */
typedef struct SparkPlatformer2DAssetsInfo {
    int usingKenneyTilesheet;
    int usingKenneyPlayerAtlas;
    int usingKenneyGem;
    uint32_t playerAtlasColumns;
} SparkPlatformer2DAssetsInfo;

/** Mirrors Spark::TerrainGeneratorSettings for demo terrain setup. */
typedef struct SparkTerrainGeneratorSettings {
    int32_t subdivX;
    int32_t subdivZ;
    float halfExtentX;
    float halfExtentZ;
    float heightScale;
    float noiseScale;
    int32_t octaves;
    float persistence;
    float lacunarity;
    uint32_t seed;
    float worldUnitsPerTextureRepeat;
} SparkTerrainGeneratorSettings;

typedef struct SparkGameObject SparkGameObject;

/** Mirrors <c>Spark::PhysicsQueryFilter2D</c>. */
typedef struct SparkPhysicsQueryFilter2D {
    uint16_t queryCategoryBits;
    uint16_t queryMaskBits;
    int hitSolids;
    int hitTriggers;
} SparkPhysicsQueryFilter2D;

/** Mirrors <c>Spark::PhysicsQueryHit2D</c>. */
typedef struct SparkPhysicsQueryHit2D {
    uint32_t staticColliderIndex;
    SparkGameObject* owner;
} SparkPhysicsQueryHit2D;

/** Mirrors Spark::CharacterController3DSettings (subset used by script games). */
typedef struct SparkCharacterController3DSettings {
    float gravityY;
    float maxFallSpeed;
    int slideIterations;
    int sweepBinaryIterations;
    float broadPhaseCellSize;
} SparkCharacterController3DSettings;

/** Mirrors Spark::SignalPayload. */
typedef struct SparkSignalPayload {
    const void* ptr;
    uint64_t a;
    uint64_t b;
} SparkSignalPayload;

/** Mirrors Spark::SignalId (engine-reserved values; game code uses SparkSignalId_UserBase + n). */
typedef enum SparkSignalId {
    SparkSignalId_None = 0,
    SparkSignalId_TransformChanged = 1,
    SparkSignalId_MeshDirty = 2,
    SparkSignalId_CollisionBoundsDirty = 3,
    SparkSignalId_Physics2DTriggerOverlap = 4,
    SparkSignalId_Physics3DTriggerEnter = 5,
    SparkSignalId_Physics3DTriggerExit = 6,
    SparkSignalId_AnimationEvent = 7,
    SparkSignalId_DamageApplied = 8,
    SparkSignalId_Died = 9,
    SparkSignalId_Physics2DTriggerEnter = 10,
    SparkSignalId_Physics2DTriggerExit = 11,
    SparkSignalId_Physics2DTriggerStay = 12,
    SparkSignalId_SpriteAnimationEvent = 13,
    SparkSignalId_InputActionTriggered = 14,
    SparkSignalId_GameStateChanged = 15,
    SparkSignalId_ItemCollected = 16,
    SparkSignalId_Physics3DTriggerStay = 17,
    SparkSignalId_Physics3DContact = 18,
    SparkSignalId_UserBase = 0x10000,
} SparkSignalId;

/** Mirrors Spark::SceneLoadOptions (subset for scripting). */
typedef struct SparkSceneLoadOptions {
    const char* assetsRoot;
    int additive;
    const char* sceneNameUtf8;
} SparkSceneLoadOptions;

/** Mirrors Spark::SceneSpawnPose. */
typedef struct SparkSceneSpawnPose {
    int found;
    SparkVector3 position;
    SparkQuaternion rotation;
    SparkGameObject* spawnObject;
} SparkSceneSpawnPose;

/** Summary of Spark::PrefabInstantiateResult (root handles via separate out array). */
typedef struct SparkPrefabInstantiateResult {
    uint64_t instanceId;
    int ready;
    uint32_t rootCount;
} SparkPrefabInstantiateResult;

#ifdef __cplusplus
}
#endif
