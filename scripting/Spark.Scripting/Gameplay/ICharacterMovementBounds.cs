using Spark.Bindings;

namespace Spark.Scripting.Gameplay;

/// <summary>Optional room / level bounds for character and camera clamping.</summary>
public interface ICharacterMovementBounds
{
    Vector3 ClampHorizontal(Vector3 position, float margin);

    Vector3 ConstrainCameraAlongPivot(Vector3 pivot, Vector3 idealCamera, float margin);
}
