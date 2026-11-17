// Companion blittable — mirrors SparkLitSceneSubmitOptions in SparkInteropTypes.h (until ClangSharp regen).
#nullable enable
using System.Runtime.InteropServices;

namespace Spark.Bindings;

[StructLayout(LayoutKind.Sequential)]
public struct SparkLitSceneSubmitOptions
{
    public SparkVector3 lightDirectionWorld;
    public SparkVector3 lightColor;
    public float lightIntensity;
    public SparkVector3 ambientColor;
    public int enableParticles;
    public SparkVector3 particleCameraRight;
    public SparkVector3 particleCameraUp;
    public float sceneTimeSeconds;
    public SparkSpriteSortMode spriteSortMode;
    public int useSceneFrustumCulling;
}