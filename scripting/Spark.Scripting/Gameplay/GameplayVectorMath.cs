using Spark.Bindings;

namespace Spark.Scripting.Gameplay;

/// <summary>Reusable vector/quaternion helpers for script games (mirrors common C++ demo math).</summary>
public sealed class GameplayVectorMath
{
    public static GameplayVectorMath Default { get; } = new();

    public SparkQuaternion Identity => new() { w = 1 };
    public float Pi => MathF.PI;
    public float HalfPi => MathF.PI * 0.5f;
    public float Epsilon => 1.0e-6f;

    public Vector3 Cross(in Vector3 a, in Vector3 b) =>
        new()
        {
            X = a.Y * b.Z - a.Z * b.Y,
            Y = a.Z * b.X - a.X * b.Z,
            Z = a.X * b.Y - a.Y * b.X,
        };

    public float Dot(in Vector3 a, in Vector3 b) => a.X * b.X + a.Y * b.Y + a.Z * b.Z;

    public Vector3 Normalized(in Vector3 v)
    {
        float len2 = Dot(v, v);
        if (len2 < Epsilon)
        {
            return v;
        }

        float inv = 1.0f / MathF.Sqrt(len2);
        return new Vector3 { X = v.X * inv, Y = v.Y * inv, Z = v.Z * inv };
    }

    public float LengthSquared(in Vector3 v) => Dot(v, v);

    public SparkQuaternion FromAxisAngleY(float radians)
    {
        float half = radians * 0.5f;
        return new SparkQuaternion
        {
            x = 0,
            y = MathF.Sin(half),
            z = 0,
            w = MathF.Cos(half),
        };
    }

    public Matrix4 LookAt(in Vector3 eye, in Vector3 target, in Vector3 up)
    {
        Vector3 f = Normalized(new Vector3
        {
            X = target.X - eye.X,
            Y = target.Y - eye.Y,
            Z = target.Z - eye.Z,
        });
        Vector3 s = Cross(f, up);
        if (LengthSquared(s) < Epsilon)
        {
            s = Cross(f, new Vector3 { X = 1, Y = 0, Z = 0 });
            if (LengthSquared(s) < Epsilon)
            {
                s = Cross(f, new Vector3 { X = 0, Y = 0, Z = 1 });
            }
        }

        s = Normalized(s);
        Vector3 u = Cross(s, f);
        return new Matrix4
        {
            M00 = s.X,
            M01 = u.X,
            M02 = -f.X,
            M03 = 0,
            M10 = s.Y,
            M11 = u.Y,
            M12 = -f.Y,
            M13 = 0,
            M20 = s.Z,
            M21 = u.Z,
            M22 = -f.Z,
            M23 = 0,
            M30 = -Dot(s, eye),
            M31 = -Dot(u, eye),
            M32 = Dot(f, eye),
            M33 = 1,
        };
    }

    public float WrapAnglePi(float a)
    {
        while (a > Pi)
        {
            a -= 2.0f * Pi;
        }

        while (a < -Pi)
        {
            a += 2.0f * Pi;
        }

        return a;
    }

    public SparkQuaternion Normalized(in SparkQuaternion q)
    {
        float len2 = q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
        if (len2 < Epsilon)
        {
            return Identity;
        }

        float inv = 1.0f / MathF.Sqrt(len2);
        return new SparkQuaternion { x = q.x * inv, y = q.y * inv, z = q.z * inv, w = q.w * inv };
    }

    public SparkQuaternion Multiply(in SparkQuaternion a, in SparkQuaternion b) =>
        new()
        {
            x = a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
            y = a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,
            z = a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w,
            w = a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z,
        };

    public Vector3 RotateVector(in SparkQuaternion q, in Vector3 v)
    {
        SparkQuaternion qv = new() { x = v.X, y = v.Y, z = v.Z, w = 0 };
        SparkQuaternion qConj = new() { x = -q.x, y = -q.y, z = -q.z, w = q.w };
        SparkQuaternion r = Multiply(Multiply(q, qv), qConj);
        return new Vector3 { X = r.x, Y = r.y, Z = r.z };
    }

    public SparkQuaternion FromShortestArc(in Vector3 fromUnit, in Vector3 toUnit)
    {
        float dot = Dot(fromUnit, toUnit);
        if (dot > 1.0f - 1.0e-6f)
        {
            return Identity;
        }

        if (dot < -1.0f + 1.0e-6f)
        {
            Vector3 axis = Cross(new Vector3 { X = 1, Y = 0, Z = 0 }, fromUnit);
            if (LengthSquared(axis) < Epsilon)
            {
                axis = Cross(new Vector3 { X = 0, Y = 1, Z = 0 }, fromUnit);
            }

            axis = Normalized(axis);
            return new SparkQuaternion { x = axis.X, y = axis.Y, z = axis.Z, w = 0 };
        }

        Vector3 cross = Cross(fromUnit, toUnit);
        return Normalized(new SparkQuaternion
        {
            x = cross.X,
            y = cross.Y,
            z = cross.Z,
            w = 1.0f + dot,
        });
    }
}
