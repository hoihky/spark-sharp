// Companion — typed partition kind on scene spatial policy.
#nullable enable

namespace Spark.Bindings.Components;

public sealed partial class SceneSpatialPolicyComponent
{
    public ScenePartitionKind Partition
    {
        get => (ScenePartitionKind)(uint)PartitionKind;
        set => PartitionKind = (int)value;
    }
}