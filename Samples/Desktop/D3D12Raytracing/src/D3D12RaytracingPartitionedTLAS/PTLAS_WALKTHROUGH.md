# Learning PTLAS from this sample

PTLAS is useful when a large scene contains spatially coherent groups and only some groups change each frame. This sample places static scenery and dominoes in a grid of partitions, then compares sparse PTLAS updates with a regular TLAS using the same scene and animation. A regular DXR raytracing pipeline traces either result.

## Code reading order

All C++ entry points below are in [D3D12RaytracingPartitionedTLAS.cpp](src/D3D12RaytracingPartitionedTLAS.cpp). Window and device setup are in [Win32Application.cpp](src/Win32Application.cpp) and [DeviceResources.cpp](src/DeviceResources.cpp).

| Read | What to look for |
| --- | --- |
| `OnInit`, `CreateDeviceDependentResources` | Capability checks, scene creation, both acceleration structures, animation resources, and pipelines. |
| `CreateSceneAndTopLevels` | Native instance records, stable instance indices, spatial partition assignment, and a histogram used to determine partition capacity. |
| `CreatePartitionedTLAS` | Prebuild sizing, result/scratch allocation, and the initial write of every instance. |
| `CreateAnimationResources` | GPU-local source records, a bounded output array, and an operation template with zero writes. |
| `OnRender`, `RenderFrame` | The complete frame: generate changes, update the AS, trace, composite, show the UI, and present. |
| `GenerateInstanceUpdates` | Reset the write count, run physics, mark active partitions, produce instance writes, and transition their buffers for the build. |
| `Animate` in [animation.hlsl](shaders/animation.hlsl), then `AppendPtlasWrite` in [PtlasWrite.hlsli](shaders/PtlasWrite.hlsli) | Decide which records changed, reserve output slots, preserve immutable instance fields, and replace transform/partition data. |
| `UpdateAccelerationStructure` | The in-place PTLAS update beside the regular TLAS comparison path. |

The collision solver supplies moving transforms; procedural meshes and image compositing provide the visible scene. The PTLAS reading path above is independent of those rendering and simulation details.

## Follow one update

At initialization the sample computes a capacity description, queries its memory requirements, and writes every instance. During animation, a changed domino produces a complete `WRITE_INSTANCE` record. A partition reassignment also produces a write, even if its transform is unchanged. The update uses the existing PTLAS as both source and destination; unchanged instance records persist.

There are **two counts**. `IndirectPartitionedTlasOpCount` points to a constant value of one because this sample issues one operation type: `WRITE_INSTANCE`. That operation's `ArgCount` is the number of instance records appended this frame. The CPU resets an operation template, and the compute shader increments `ArgCount`. A statistics readback happens later, after the frame fence, and never controls the build.

The compact output slot is unrelated to an instance's identity. Each copied record preserves its stable `InstanceIndex` and application `InstanceID`. One thread processes each domino and appends at most one record, so a buffer sized for all dominoes bounds the worst case. [PtlasShaderBindings.h](shaders/PtlasShaderBindings.h) checks the raw shader offsets against the native SDK structures.

Compute writes use UAV access. The operation and argument buffers transition to `NON_PIXEL_SHADER_RESOURCE` for `ExecuteIndirectRTASOperations`; this API does not use the ordinary `INDIRECT_ARGUMENT` state for those inputs. The resulting AS has a UAV barrier before raytracing. Two CPU frame contexts protect allocators, constants, and readbacks, while a single direct queue orders updates to the shared GPU resources. See the [partitioned TLAS specification](https://microsoft.github.io/DirectX-Specs/d3d/Raytracing2.html) for the API contract.

### Initial build and sparse update

These excerpts show the contract used by `CreatePartitionedTLAS` and `UpdateAccelerationStructure`. The linked implementations include sizing, allocation, upload, and binding calls.

```cpp
// Initial build: every instance receives a stable InstanceIndex.
D3D12_RTAS_PARTITIONED_TLAS_OPERATION operation{};
operation.Type = D3D12_RTAS_PARTITIONED_TLAS_OPERATION_TYPE_WRITE_INSTANCE;
operation.ArgCount = m_totalInstanceCount;
operation.ArgData.StartAddress = m_ptlasInstances->GetGPUVirtualAddress();
operation.ArgData.StrideInBytes =
    sizeof(D3D12_RTAS_PARTITIONED_TLAS_OPERATION_WRITE_INSTANCE_ARGS);
// Upload this operation and a separate UINT containing 1 (the operation count).
// The initial build has SourceAccelerationStructureData == 0.
```

The update replaces that initial upload with a compact GPU-written array. Reset its instance-record count (`ArgCount`) to zero by copying the operation template; the outer operation count stays one. Unused array entries are ignored. The compute shader copies the full native instance record into each reserved slot and overwrites its transform and partition index. It preserves the original `InstanceIndex`, regardless of the append order.

```cpp
// After compute and the input-buffer transitions, retain unchanged instances.
D3D12_RTAS_PARTITIONED_TLAS_OPERATION_DATA data{};
data.SourceAccelerationStructureData = m_ptlas->GetGPUVirtualAddress();
data.DestAccelerationStructureData = m_ptlas->GetGPUVirtualAddress();
data.ScratchAccelerationStructureData = m_ptlasScratch->GetGPUVirtualAddress();
data.IndirectPartitionedTlasOpCount = m_ptlasOperationCount->GetGPUVirtualAddress();
data.IndirectPartitionedTlasOps = m_gpuPtlasOperation->GetGPUVirtualAddress();
D3D12_RTAS_OPERATION_DESC build{};
build.Inputs.Type = D3D12_RTAS_OPERATION_TYPE_PARTITIONED_TLAS;
build.Inputs.pPartitionedTLASInputsDesc = &m_ptlasInputs;
build.pPartitionedTlasOperationData = &data;
m_raytracingCommandList->ExecuteIndirectRTASOperations(
    1, &build, D3D12_EXECUTE_INDIRECT_RTAS_OPERATIONS_FLAG_NONE);
auto ready = CD3DX12_RESOURCE_BARRIER::UAV(m_ptlas.Get());
m_commandList->ResourceBarrier(1, &ready);
```

The capacity description remains fixed. If you extend the sample to allow arbitrary partition destinations, recompute the destination capacity bounds; the current histogram only covers the original-partition/global-partition movement policy. Never interpret a small current write count as permission to allocate a smaller persistent PTLAS.

## Partition choices to experiment with

| Mode | Behavior in this sample | What to compare |
| --- | --- | --- |
| 1 | Changed dominoes stay in their original partitions. | Local updates when few partitions contain motion. |
| 2 | Active dominoes enter the global partition, then return when quiet. | The cost of global updates versus rebuilding their original partitions. |
| 3 | Nearby partitions use mode 1; distant partitions use mode 2. | The camera-distance tradeoff using the near/far distance setting (F2). |

The global partition is a separate API partition identified by `0xffffffff`, not an extra element in the ordinary partition grid. Its capacity covers all dynamic dominoes here. Ordinary partition capacity includes both static objects and the dominoes that can return to that partition. These are conservative bounds for this scene's movement policy, not universal capacity settings.

### One domino and a quiet scene

Run `--full --self-topple-count 0 --paused --mode 1 --no-vsync`. After the initial synchronization, the write count should settle to zero. Right-click a domino, or use F2 and **Topple selected domino**; both actions resume simulation. Expect a small stream of writes that grows as collisions propagate. Press F to pause again: after any outstanding partition changes, writes return to zero while the built PTLAS remains traceable. P switches to regular TLAS; T toggles its refit/rebuild path. The first frame after changing AS paths forces synchronization, so exclude that frame from comparisons.

### Motion across many partitions

Run each of `--full --self-topple-count 2000 --mode 1`, `--mode 2`, and `--mode 3` with the same `--frames 120 --no-vsync --csv PATH`. Each starts with the same seeds and camera. Discard startup frames. Mode 1 can rebuild many spatial partitions containing static scenery; mode 2 concentrates active dominoes in the global partition. Expect different AS update costs even when the write counts are similar. Global membership changes also require writes, including when a domino becomes quiet. Compare trace time and combined GPU time: a cheaper update does not guarantee a cheaper frame.

### Near and far partitions

Select mode 3, open F2, and change **Near/far distance**. A small distance moves more active dominoes into the global partition; a large distance keeps more in their spatial partitions. Move the camera to see that the choice uses partition centers. Then try **Mark all dynamic in active partitions**: it can add writes for unmoving dominoes to keep the dynamic members of a partition together. Its effect depends on the scene's activity. For timing conclusions, restart each run with the same camera and motion frame instead of comparing different points in a collision chain. Read the write count as instance records, not the number of rebuilt partitions.

### Watch partitions become quiet

Run `--full --self-topple-count 2000 --mode 1 --frames 6000 --no-vsync --csv settling.csv`. Keep the camera and settings fixed. Watch **AS** time and the instance-write count: collisions spread first, then individual partitions stop receiving writes, and finally the scene becomes still. The 120-frame experiment above covers early motion; this longer run demonstrates settling.

The static scenery becomes more saturated when a domino associated with its spatial partition changes, and returns to its muted color ten rendered frames after the last modification. Hue identifies the partition. This history tracks simulation activity, including dominoes temporarily in the global partition; it is not a driver-reported count of rebuilt partitions. In mode 1, changed dominoes stay in their spatial partitions, making this the clearest starting point for the demonstration.

Expect the AS update time to decline as fewer partitions receive changed instances. Once motion and partition reassignments stop, the write count reaches zero and the scenery loses its activity highlight. The sample still submits the update when there are no instance writes. Its GPU cost depends on the implementation. Repeat with mode 3 to observe settling when distant dominoes use the global partition; returning a quiet domino to its spatial partition requires one final write. Exact settling frames and GPU times depend on the scene settings and hardware.

Compare medians over an early window after initialization and a later window with zero writes. Use Release without the debug layer, omit state/image capture, and keep camera, resolution, build hints, and simulation settings fixed. Read **AS** time separately from animation and raytracing; a quiet PTLAS still needs to be traced each frame. Do not assume update cost is proportional to the number of written instances: their distribution across partitions also matters.

This sample demonstrates fixed-capacity sparse instance writes. It does not demonstrate partition translations, deletion/streaming, explicit instance AABBs, or PTLAS resizing. `UPDATE_INSTANCE` is also unused because this animation changes transforms. Those operations have separate contracts and should not be inferred from this write path.

## Device loss and regeneration

`DeviceResources::HandleDeviceLost` notifies the sample before and after recreating the device. `OnDeviceLost` releases the PTLAS and every buffer containing GPU virtual addresses, plus descriptors, state objects, UI resources, and frame fence tags. `OnDeviceRestored` regenerates the CPU scene, builds BLAS again, and performs a complete initial PTLAS build on the new device. GPU addresses and AS contents cannot be reused across devices.

The simulation restarts after restoration; camera and settings survive. Scene regeneration reuses those sample resource lifecycle methods after a GPU wait, within the same window and device. Both paths resume sparse PTLAS updates only after full initialization. [test_device_recovery.py](tools/test_device_recovery.py) verifies the restart against fresh simulation states.
