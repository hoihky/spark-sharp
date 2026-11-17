// Companion — mirrors C++ ScenePartitionKind / spark/scene/spatial trees.
#nullable enable

namespace Spark.Bindings;

/// <summary>Frustum partition for <c>SceneSpatialPolicyComponent</c> (native OctTree for Octree).</summary>
public enum ScenePartitionKind : uint
{
    None = 0,
    BruteForce = 1,
    AxisAlignedBinarySpacePartition = 2,
    Octree = 3,
    QuadTree = 4,
    BoundingVolumeHierarchy = 5,
}