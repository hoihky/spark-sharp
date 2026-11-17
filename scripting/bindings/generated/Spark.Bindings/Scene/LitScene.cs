// Companion — mirrors C++ LitScene (static helpers + Builder).
#nullable enable

namespace Spark.Bindings;

/// <summary>Lit ECS → <c>SceneRenderParams</c> (native <c>LitSceneSubmitter</c> pipeline).</summary>
public static class LitScene
{
    /// <summary>Fluent submit (C++ <c>LitScene::Builder</c>).</summary>
    public sealed class Builder
    {
        private readonly GameWorld _world;
        private readonly IEngineContext _context;
        private LitSceneSubmitOptions _options = LitSceneSubmitOptions.Default3D;
        private Matrix4 _viewProjection;
        private Vector3 _cameraPositionWorld;
        private bool _hasView;
        private bool _ssaoEnabled;

        public Builder(GameWorld world, IEngineContext context)
        {
            _world = world;
            _context = context;
        }

        public Builder WithView(in Matrix4 viewProjection, in Vector3 cameraPositionWorld)
        {
            _viewProjection = viewProjection;
            _cameraPositionWorld = cameraPositionWorld;
            _hasView = true;
            return this;
        }

        public bool WithCameraFromWorld()
        {
            _context.GetFramebufferSize(out int fbW, out int fbH);
            if (fbW <= 0)
            {
                fbW = 1;
            }

            if (fbH <= 0)
            {
                fbH = 1;
            }

            if (!TryResolveCamera(_world, fbW, fbH, out _viewProjection, out _cameraPositionWorld, out var right, out var up))
            {
                _hasView = false;
                return false;
            }

            if (_options.EnableParticles)
            {
                _options.ParticleCameraRight = right;
                _options.ParticleCameraUp = up;
            }

            _hasView = true;
            return true;
        }

        public Builder WithOptions(in LitSceneSubmitOptions options)
        {
            _options = options;
            return this;
        }

        public Builder WithSsao(bool enabled)
        {
            _ssaoEnabled = enabled;
            return this;
        }

        public bool Fill(IntPtr outParams, uint outParamsByteSize)
        {
            if (!_hasView)
            {
                return false;
            }

            _world.Rendering.FillStandardLitSceneParams(
                _context,
                _viewProjection,
                _cameraPositionWorld,
                _options,
                outParams,
                outParamsByteSize);
            return true;
        }

        public bool Submit()
        {
            if (!_hasView)
            {
                return false;
            }

            _world.Rendering.SubmitStandardLit(_context, _viewProjection, _cameraPositionWorld, _options, _ssaoEnabled);
            return true;
        }
    }

    public static void Submit(
        GameWorld world,
        IEngineContext context,
        in Matrix4 viewProjection,
        in Vector3 cameraPositionWorld,
        in LitSceneSubmitOptions options,
        bool ssaoEnabled = false)
        => world.Rendering.SubmitStandardLit(context, viewProjection, cameraPositionWorld, options, ssaoEnabled);

    public static void Submit(
        GameWorld world,
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
        SparkSpriteSortMode spriteSortMode = SparkSpriteSortMode.SparkSpriteSortMode_SortOrderOnly)
    {
        var options = LitSceneSubmitOptions.Default3D
            .WithDirectionalLight(lightDirectionWorld, lightColor, lightIntensity)
            .WithAmbient(ambientColor)
            .WithParticles(enableParticles, particleCameraRight, particleCameraUp)
            .WithSceneTime(sceneTimeSeconds)
            .WithSpriteSortMode(spriteSortMode);
        world.Rendering.SubmitStandardLit(
            context,
            viewProjection,
            cameraPositionWorld,
            options);
    }

    public static bool TryResolveCamera(
        GameWorld world,
        int framebufferWidth,
        int framebufferHeight,
        out Matrix4 viewProjection,
        out Vector3 cameraPositionWorld,
        out Vector3 particleCameraRight,
        out Vector3 particleCameraUp)
    {
        unsafe
        {
            SparkMatrix4 vp;
            SparkVector3 cam;
            SparkVector3 right;
            SparkVector3 up;
            int ok = Native.spark_scene_try_resolve_lit_camera(
                InteropPtr.World(world.Handle),
                framebufferWidth,
                framebufferHeight,
                &vp,
                &cam,
                &right,
                &up);
            if (ok == 0)
            {
                viewProjection = default;
                cameraPositionWorld = default;
                particleCameraRight = default;
                particleCameraUp = default;
                return false;
            }

            viewProjection = Matrix4.FromNative(vp);
            cameraPositionWorld = Vector3.FromNative(cam);
            particleCameraRight = Vector3.FromNative(right);
            particleCameraUp = Vector3.FromNative(up);
            return true;
        }
    }

    public static bool SubmitWithCamera(
        GameWorld world,
        IEngineContext context,
        in LitSceneSubmitOptions options,
        bool ssaoEnabled = false)
        => world.Rendering.SubmitStandardLitWithCamera(context, options, ssaoEnabled);
}