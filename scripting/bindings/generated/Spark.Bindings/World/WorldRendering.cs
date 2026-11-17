// Companion — scene submit / standard lit rendering (LitSceneSubmitter pipeline).
#nullable enable

namespace Spark.Bindings;

/// <summary>Scene render submission (mirrors <c>SceneSubmit.hpp</c> + <c>LitSceneSubmitter</c>).</summary>
public sealed class WorldRendering : WorldFacet
{
    public WorldRendering(GameWorld world)
        : base(world) { }

    public void SubmitStandardLit(
        IEngineContext context,
        in Matrix4 viewProjection,
        in Vector3 cameraPositionWorld,
        in LitSceneSubmitOptions options,
        bool ssaoEnabled = false)
    {
        unsafe
        {
            var vp = viewProjection.ToNative();
            var cam = cameraPositionWorld.ToNative();
            var nativeOpts = options.ToNative();
            Native.spark_scene_submit_standard_lit_from_world_options(
                InteropPtr.World(World.Handle),
                InteropPtr.Context(context.Handle),
                &vp,
                &cam,
                &nativeOpts,
                ssaoEnabled ? 1 : 0);
        }
    }

    public void SubmitStandardLit(
        IEngineContext context,
        in Matrix4 viewProjection,
        in Vector3 cameraPositionWorld,
        in Vector3 lightDirectionWorld,
        in Vector3 lightColor,
        float lightIntensity,
        in Vector3 ambientColor,
        bool enableParticles,
        in Vector3 particleCameraRight,
        in Vector3 particleCameraUp,
        float sceneTimeSeconds,
        SparkSpriteSortMode spriteSortMode = SparkSpriteSortMode.SparkSpriteSortMode_SortOrderOnly,
        bool ssaoEnabled = false)
        => World.SubmitStandardLit(
            context,
            viewProjection,
            cameraPositionWorld,
            lightDirectionWorld,
            lightColor,
            lightIntensity,
            ambientColor,
            enableParticles,
            particleCameraRight,
            particleCameraUp,
            sceneTimeSeconds,
            spriteSortMode,
            ssaoEnabled);

    public void FillStandardLitSceneParams(
        IEngineContext context,
        in Matrix4 viewProjection,
        in Vector3 cameraPositionWorld,
        in LitSceneSubmitOptions options,
        IntPtr outParams,
        uint outParamsByteSize)
    {
        unsafe
        {
            var vp = viewProjection.ToNative();
            var cam = cameraPositionWorld.ToNative();
            var nativeOpts = options.ToNative();
            Native.spark_scene_fill_standard_lit_from_world_options(
                InteropPtr.World(World.Handle),
                InteropPtr.Context(context.Handle),
                &vp,
                &cam,
                &nativeOpts,
                outParams.ToPointer(),
                outParamsByteSize);
        }
    }

    public void FillStandardLitSceneParams(
        IEngineContext context,
        in Matrix4 viewProjection,
        in Vector3 cameraPositionWorld,
        in Vector3 lightDirectionWorld,
        in Vector3 lightColor,
        float lightIntensity,
        in Vector3 ambientColor,
        bool enableParticles,
        in Vector3 particleCameraRight,
        in Vector3 particleCameraUp,
        float sceneTimeSeconds,
        IntPtr outParams,
        uint outParamsByteSize,
        SparkSpriteSortMode spriteSortMode = SparkSpriteSortMode.SparkSpriteSortMode_SortOrderOnly)
        => World.FillStandardLitFromWorld(
            context,
            viewProjection,
            cameraPositionWorld,
            lightDirectionWorld,
            lightColor,
            lightIntensity,
            ambientColor,
            enableParticles,
            particleCameraRight,
            particleCameraUp,
            sceneTimeSeconds,
            outParams,
            outParamsByteSize,
            spriteSortMode);

    /// <summary>Standard lit submit using a 2D camera (orthographic gameplay / UI scenes).</summary>
    public void SubmitStandardLit(
        IEngineContext context,
        in Camera2D camera,
        in LitSceneSubmitOptions options,
        bool ssaoEnabled = false)
    {
        context.GetFramebufferSize(out int fbW, out int fbH);
        if (fbW <= 0)
        {
            fbW = 1;
        }

        if (fbH <= 0)
        {
            fbH = 1;
        }

        Matrix4 viewProj = camera.ViewProjection(fbW, fbH);
        if (options.EnableParticles)
        {
            camera.BillboardBasisWorld(out Vector3 camRight, out Vector3 camUp);
            var patched = options;
            patched.ParticleCameraRight = camRight;
            patched.ParticleCameraUp = camUp;
            SubmitStandardLit(context, viewProj, camera.Position, patched, ssaoEnabled);
            return;
        }

        SubmitStandardLit(context, viewProj, camera.Position, options, ssaoEnabled);
    }

    public void SubmitStandardLit(
        IEngineContext context,
        in Camera2D camera,
        in Vector3 lightDirectionWorld,
        in Vector3 lightColor,
        float lightIntensity,
        in Vector3 ambientColor,
        float sceneTimeSeconds,
        bool enableParticles = false,
        SparkSpriteSortMode spriteSortMode = SparkSpriteSortMode.SparkSpriteSortMode_SortOrderThenWorldY)
    {
        var options = LitSceneSubmitOptions.Default3D
            .WithDirectionalLight(lightDirectionWorld, lightColor, lightIntensity)
            .WithAmbient(ambientColor)
            .WithSceneTime(sceneTimeSeconds)
            .WithSpriteSortMode(spriteSortMode)
            .WithParticles(
                enableParticles,
                new Vector3 { X = 1f, Y = 0f, Z = 0f },
                new Vector3 { X = 0f, Y = 1f, Z = 0f });
        SubmitStandardLit(context, camera, options);
    }

    /// <summary>Resolves main ECS camera and submits (C++ <c>SubmitStandardLitSceneFromWorldWithCamera</c>).</summary>
    public bool SubmitStandardLitWithCamera(
        IEngineContext context,
        in LitSceneSubmitOptions options,
        bool ssaoEnabled = false)
    {
        unsafe
        {
            var nativeOpts = options.ToNative();
            return Native.spark_scene_submit_standard_lit_with_camera(
                       InteropPtr.World(World.Handle),
                       InteropPtr.Context(context.Handle),
                       &nativeOpts,
                       ssaoEnabled ? 1 : 0)
                   != 0;
        }
    }
}