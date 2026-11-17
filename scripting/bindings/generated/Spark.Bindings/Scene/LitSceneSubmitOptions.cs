// Companion — mirrors C++ LitSceneSubmitOptions (LitSceneSubmitter inputs).
#nullable enable

namespace Spark.Bindings;

/// <summary>Per-frame lit submit bundle (C++ <c>LitSceneSubmitOptions</c>).</summary>
public struct LitSceneSubmitOptions
{
    public Vector3 LightDirectionWorld;
    public Vector3 LightColor;
    public float LightIntensity;
    public Vector3 AmbientColor;
    public bool EnableParticles;
    public Vector3 ParticleCameraRight;
    public Vector3 ParticleCameraUp;
    public float SceneTimeSeconds;
    public SparkSpriteSortMode SpriteSortMode;
    /// <summary>When true, native fill uses the active <c>Scene</c> for frustum cull (BVH / OctTree / QuadTree).</summary>
    public bool UseSceneFrustumCulling;

    public static LitSceneSubmitOptions Default3D => new()
    {
        LightDirectionWorld = new Vector3 { X = 0.35f, Y = 0.92f, Z = 0.18f },
        LightColor = new Vector3 { X = 1f, Y = 0.97f, Z = 0.9f },
        LightIntensity = 0.92f,
        AmbientColor = new Vector3 { X = 0.05f, Y = 0.055f, Z = 0.07f },
        EnableParticles = false,
        ParticleCameraRight = new Vector3 { X = 1f, Y = 0f, Z = 0f },
        ParticleCameraUp = new Vector3 { X = 0f, Y = 1f, Z = 0f },
        SceneTimeSeconds = 0f,
        SpriteSortMode = SparkSpriteSortMode.SparkSpriteSortMode_SortOrderOnly,
        UseSceneFrustumCulling = false,
    };

    public LitSceneSubmitOptions WithDirectionalLight(in Vector3 directionWorld, in Vector3 color, float intensity)
    {
        LightDirectionWorld = directionWorld;
        LightColor = color;
        LightIntensity = intensity;
        return this;
    }

    public LitSceneSubmitOptions WithAmbient(in Vector3 color)
    {
        AmbientColor = color;
        return this;
    }

    public LitSceneSubmitOptions WithParticles(bool enabled, in Vector3 cameraRight, in Vector3 cameraUp)
    {
        EnableParticles = enabled;
        ParticleCameraRight = cameraRight;
        ParticleCameraUp = cameraUp;
        return this;
    }

    public LitSceneSubmitOptions WithSceneTime(float seconds)
    {
        SceneTimeSeconds = seconds;
        return this;
    }

    public LitSceneSubmitOptions WithSpriteSortMode(SparkSpriteSortMode mode)
    {
        SpriteSortMode = mode;
        return this;
    }

    public LitSceneSubmitOptions WithSceneFrustumCulling(bool enabled = true)
    {
        UseSceneFrustumCulling = enabled;
        return this;
    }

    internal unsafe SparkLitSceneSubmitOptions ToNative()
    {
        return new SparkLitSceneSubmitOptions
        {
            lightDirectionWorld = LightDirectionWorld.ToNative(),
            lightColor = LightColor.ToNative(),
            lightIntensity = LightIntensity,
            ambientColor = AmbientColor.ToNative(),
            enableParticles = EnableParticles ? 1 : 0,
            particleCameraRight = ParticleCameraRight.ToNative(),
            particleCameraUp = ParticleCameraUp.ToNative(),
            sceneTimeSeconds = SceneTimeSeconds,
            spriteSortMode = SpriteSortMode,
            useSceneFrustumCulling = UseSceneFrustumCulling ? 1 : 0,
        };
    }
}