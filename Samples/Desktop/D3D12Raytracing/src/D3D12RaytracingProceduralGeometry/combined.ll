; ModuleID = 'merged_lib'
target triple = "dxil-ms-dx"

%g_sceneCB = type { %struct.SceneConstantBuffer }
%struct.SceneConstantBuffer = type { %class.matrix.float.4.4, <4 x float>, <4 x float>, <4 x float>, <4 x float> }
%class.matrix.float.4.4 = type { [4 x <4 x float>] }
%"class.RWTexture2D<vector<float, 4> >" = type { <4 x float> }
%struct.RaytracingAccelerationStructure = type { i32 }
%"class.StructuredBuffer<AABBPrimitiveAttributes>" = type { %struct.AABBPrimitiveAttributes }
%struct.AABBPrimitiveAttributes = type { %class.matrix.float.4.4, <3 x float>, float }
%struct.ByteAddressBuffer = type { i32 }
%Constants = type { i32, i32, i32, i32, i32, i32, i32, i32 }
%struct.RWByteAddressBuffer = type { i32 }
%AccelerationStructureList = type { <2 x i32> }
%"class.StructuredBuffer<Vertex>" = type { %struct.Vertex }
%struct.Vertex = type { <3 x float>, <3 x float> }
%dx.types.CBufRet.f32 = type { float, float, float, float }
%dx.types.Handle = type { i8* }
%struct.ShadowPayload = type { i32 }
%struct.BuiltInTriangleIntersectionAttributes = type { <2 x float> }
%dx.types.ResRet.i32 = type { i32, i32, i32, i32, i32 }
%class.matrix.float.3.4 = type { [3 x <4 x float>] }
%struct.HitData = type { <4 x float> }
%dx.types.CBufRet.i32 = type { i32, i32, i32, i32 }
%struct.ProceduralPrimitiveAttributes = type { <3 x float> }
%dx.types.ResRet.f32 = type { float, float, float, float, i32 }
%struct.BVHMetadata = type { %struct.RaytracingInstanceDesc, [3 x <4 x float>], i32 }
%struct.RaytracingInstanceDesc = type { [3 x <4 x float>], i32, i32, <2 x i32> }
%struct.BoundingBox = type { <3 x float>, <3 x float> }
%struct.RWByteAddressBufferPointer = type { %struct.RWByteAddressBuffer, i32 }
%struct.AABB = type { <3 x float>, <3 x float> }
%struct.Primitive = type { i32, <4 x i32>, <4 x i32>, i32 }
%struct.Triangle = type { <3 x float>, <3 x float>, <3 x float> }
%struct.RayData = type { <3 x float>, <3 x float>, <3 x float>, <3 x float>, <3 x float>, <3 x i32> }
%struct.PrimitiveMetaData = type { i32, i32 }
%g_cubeCB = type { %struct.CubeConstantBuffer }
%struct.CubeConstantBuffer = type { <4 x float> }

@g_sceneCB = external constant %g_sceneCB
@"\01?RenderTarget@@3V?$RWTexture2D@V?$vector@M$03@@@@A" = external global %"class.RWTexture2D<vector<float, 4> >"
@"\01?Scene@@3URaytracingAccelerationStructure@@A" = external global %struct.RaytracingAccelerationStructure
@"\01?g_AABBPrimitiveAttributes@@3V?$StructuredBuffer@UAABBPrimitiveAttributes@@@@A" = external global %"class.StructuredBuffer<AABBPrimitiveAttributes>"
@"\01?stack@@3PAIA" = addrspace(3) global [2048 x i32] zeroinitializer
@"\01?HitGroupShaderTable@@3UByteAddressBuffer@@A" = external global %struct.ByteAddressBuffer
@"\01?MissShaderTable@@3UByteAddressBuffer@@A" = external global %struct.ByteAddressBuffer
@Constants = external constant %Constants
@"\01?DescriptorHeapBufferTable@@3PAURWByteAddressBuffer@@A" = external global [0 x %struct.RWByteAddressBuffer]
@AccelerationStructureList = external constant %AccelerationStructureList
@"\01?RayGenShaderTable@@3UByteAddressBuffer@@A" = external global %struct.ByteAddressBuffer
@"1boxVertices.3.hca" = internal constant [8 x float] [float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00]
@"\01?Indices@@3UByteAddressBuffer@@A" = external global %struct.ByteAddressBuffer
@"\01?Vertices@@3V?$StructuredBuffer@UVertex@@@@A" = external global %"class.StructuredBuffer<Vertex>"

; Function Attrs: alwaysinline
declare void @"\01?Fallback_SetPendingHitKind@@YAXI@Z"(i32) #0

; Function Attrs: alwaysinline
declare void @"\01?Fallback_SetPendingCustomVals@@YAXIIII@Z"(i32, i32, i32, i32) #0

; Function Attrs: nounwind readonly
declare %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32, %dx.types.Handle, i32) #1

; Function Attrs: nounwind
declare void @dx.op.traceRay.struct.ShadowPayload(i32, %dx.types.Handle, i32, i32, i32, i32, i32, float, float, float, float, float, float, float, float, %struct.ShadowPayload*) #2

; Function Attrs: nounwind readnone
declare float @dx.op.tertiary.f32(i32, float, float, float) #3

; Function Attrs: nounwind readonly
declare %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RaytracingAccelerationStructure(i32, %struct.RaytracingAccelerationStructure) #1

; Function Attrs: nounwind readonly
declare %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.ByteAddressBuffer(i32, %struct.ByteAddressBuffer) #1

; Function Attrs: nounwind readnone
declare float @dx.op.worldRayOrigin.f32(i32, i8) #3

; Function Attrs: nounwind
declare void @dx.op.textureStore.f32(i32, %dx.types.Handle, i32, i32, i32, float, float, float, float, i8) #2

; Function Attrs: alwaysinline
declare i32 @"\01?Fallback_AnyHitResult@@YAHXZ"() #0

; Function Attrs: alwaysinline
declare void @"\01?Fallback_SetObjectRayOrigin@@YAXV?$vector@M$02@@@Z"(<3 x float>) #0

; Function Attrs: nounwind readonly
declare float @dx.op.rayTCurrent.f32(i32) #1

; Function Attrs: nounwind readnone
declare float @dx.op.rayTMin.f32(i32) #3

; Function Attrs: nounwind readonly
declare %dx.types.Handle @"dx.op.createHandleFromResourceStructForLib.class.RWTexture2D<vector<float, 4> >"(i32, %"class.RWTexture2D<vector<float, 4> >") #1

; Function Attrs: nounwind readonly
declare %dx.types.Handle @"dx.op.createHandleFromResourceStructForLib.class.StructuredBuffer<AABBPrimitiveAttributes>"(i32, %"class.StructuredBuffer<AABBPrimitiveAttributes>") #1

; Function Attrs: alwaysinline
declare void @"\01?Fallback_SetPendingTriVals@@YAXIIIIMI@Z"(i32, i32, i32, i32, float, i32) #0

; Function Attrs: alwaysinline
declare void @"\01?Fallback_SetAnyHitResult@@YAXH@Z"(i32) #0

; Function Attrs: alwaysinline
declare void @"\01?Fallback_SetInstanceIndex@@YAXI@Z"(i32) #0

; Function Attrs: alwaysinline
declare void @"\01?Fallback_Scheduler@@YAXHII@Z"(i32, i32, i32) #0

; Function Attrs: nounwind readonly
declare %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.Constants(i32, %Constants) #1

; Function Attrs: alwaysinline
declare void @"\01?Fallback_CommitHit@@YAXXZ"() #0

; Function Attrs: alwaysinline
declare void @"\01?Fallback_SetPendingAttr@@YAXUBuiltInTriangleIntersectionAttributes@@@Z"(%struct.BuiltInTriangleIntersectionAttributes*) #0

; Function Attrs: nounwind readonly
declare %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32, %dx.types.Handle, i32, i32) #1

; Function Attrs: nounwind readonly
declare %dx.types.Handle @"dx.op.createHandleFromResourceStructForLib.class.StructuredBuffer<Vertex>"(i32, %"class.StructuredBuffer<Vertex>") #1

; Function Attrs: alwaysinline
declare void @"\01?Fallback_SetWorldToObject@@YAXV?$matrix@M$02$03@@@Z"(%class.matrix.float.3.4) #0

; Function Attrs: nounwind
declare void @dx.op.traceRay.struct.HitData(i32, %dx.types.Handle, i32, i32, i32, i32, i32, float, float, float, float, float, float, float, float, %struct.HitData*) #2

; Function Attrs: nounwind readnone
declare float @dx.op.objectToWorld.f32(i32, i32, i8) #3

; Function Attrs: nounwind readnone
declare float @dx.op.objectRayDirection.f32(i32, i8) #3

; Function Attrs: alwaysinline
declare void @"\01?Fallback_TraceRayEnd@@YAXH@Z"(i32) #0

; Function Attrs: nounwind readnone
declare float @dx.op.objectRayOrigin.f32(i32, i8) #3

; Function Attrs: alwaysinline
declare void @"\01?Fallback_SetPendingRayTCurrent@@YAXM@Z"(float) #0

; Function Attrs: nounwind readnone
declare i32 @dx.op.dispatchRaysDimensions.i32(i32, i8) #3

; Function Attrs: alwaysinline
declare i32 @"\01?Fallback_ShaderRecordOffset@@YAIXZ"() #0

; Function Attrs: nounwind readonly
declare %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32, %dx.types.Handle, i32) #1

; Function Attrs: alwaysinline
declare i32 @"\01?Fallback_TraceRayBegin@@YAIIV?$vector@M$02@@M0MI@Z"(i32, <3 x float>, float, <3 x float>, float, i32) #0

; Function Attrs: alwaysinline
declare i32 @"\01?Fallback_GroupIndex@@YAIXZ"() #0

; Function Attrs: nounwind
declare i1 @dx.op.reportHit.struct.ProceduralPrimitiveAttributes(i32, float, i32, %struct.ProceduralPrimitiveAttributes*) #2

; Function Attrs: alwaysinline
declare void @"\01?Fallback_SetAnyHitStateId@@YAXH@Z"(i32) #0

; Function Attrs: nounwind readonly
declare %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32, %struct.RWByteAddressBuffer) #1

; Function Attrs: nounwind readonly
declare %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.AccelerationStructureList(i32, %AccelerationStructureList) #1

; Function Attrs: alwaysinline
declare i32 @"\01?Fallback_AnyHitStateId@@YAHXZ"() #0

; Function Attrs: nounwind readnone
declare float @dx.op.worldRayDirection.f32(i32, i8) #3

; Function Attrs: nounwind readnone
declare float @dx.op.dot3.f32(i32, float, float, float, float, float, float) #3

; Function Attrs: nounwind readonly
declare %dx.types.ResRet.f32 @dx.op.bufferLoad.f32(i32, %dx.types.Handle, i32, i32) #1

; Function Attrs: alwaysinline
declare float @"\01?Fallback_RayTCurrent@@YAMXZ"() #0

; Function Attrs: alwaysinline
declare void @"\01?Fallback_SetObjectRayDirection@@YAXV?$vector@M$02@@@Z"(<3 x float>) #0

; Function Attrs: alwaysinline
declare void @"\01?Fallback_SetShaderRecordOffset@@YAXI@Z"(i32) #0

; Function Attrs: nounwind readnone
declare i32 @dx.op.rayFlags.i32(i32) #3

; Function Attrs: alwaysinline
declare void @"\01?Fallback_SetObjectToWorld@@YAXV?$matrix@M$02$03@@@Z"(%class.matrix.float.3.4) #0

; Function Attrs: alwaysinline
declare i32 @"\01?Fallback_InstanceIndex@@YAIXZ"() #0

; Function Attrs: alwaysinline
declare void @"\01?Fallback_CallIndirect@@YAXH@Z"(i32) #0

; Function Attrs: nounwind readnone
declare float @dx.op.binary.f32(i32, float, float) #3

; Function Attrs: nounwind readnone
declare i32 @dx.op.primitiveID.i32(i32) #3

; Function Attrs: nounwind readnone
declare i32 @dx.op.dispatchRaysIndex.i32(i32, i8) #3

; Function Attrs: nounwind
declare void @dx.op.bufferStore.i32(i32, %dx.types.Handle, i32, i32, i32, i32, i32, i32, i8) #2

; Function Attrs: nounwind readonly
declare %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.g_sceneCB(i32, %g_sceneCB) #1

; Function Attrs: nounwind readnone
declare float @dx.op.unary.f32(i32, float) #3

; Function Attrs: alwaysinline nounwind readnone
define <2 x i32> @"\01?PointerAdd@@YA?AV?$vector@I$01@@V1@I@Z"(<2 x i32>, i32) #4 {
entry:
  %2 = extractelement <2 x i32> %0, i32 0
  %add = add i32 %2, %1
  %3 = insertelement <2 x i32> %0, i32 %add, i32 0
  ret <2 x i32> %3
}

; Function Attrs: alwaysinline nounwind readnone
define void @"\01?LogTraceRayStart@@YAXXZ"() #4 {
entry:
  ret void
}

; Function Attrs: alwaysinline nounwind
define void @"\01?RecordClosestBox@@YAXIAIA_NM0MAIAM@Z"(i32, i32* noalias nocapture dereferenceable(4), float, i32* noalias nocapture dereferenceable(4), float, float* noalias nocapture dereferenceable(4)) #5 {
entry:
  ret void
}

; Function Attrs: alwaysinline nounwind
define void @"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z"(%struct.BVHMetadata* noalias nocapture sret, %struct.RWByteAddressBuffer* nocapture readonly, i32) #5 {
entry:
  %3 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %1, align 4
  %4 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %3)
  %5 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %4, i32 %2, i32 undef)
  %6 = extractvalue %dx.types.ResRet.i32 %5, 0
  %7 = extractvalue %dx.types.ResRet.i32 %5, 1
  %8 = extractvalue %dx.types.ResRet.i32 %5, 2
  %9 = extractvalue %dx.types.ResRet.i32 %5, 3
  %add = add i32 %2, 16
  %10 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %4, i32 %add, i32 undef)
  %11 = extractvalue %dx.types.ResRet.i32 %10, 0
  %12 = extractvalue %dx.types.ResRet.i32 %10, 1
  %13 = extractvalue %dx.types.ResRet.i32 %10, 2
  %14 = extractvalue %dx.types.ResRet.i32 %10, 3
  %add.1 = add i32 %2, 32
  %15 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %4, i32 %add.1, i32 undef)
  %16 = extractvalue %dx.types.ResRet.i32 %15, 0
  %17 = extractvalue %dx.types.ResRet.i32 %15, 1
  %18 = extractvalue %dx.types.ResRet.i32 %15, 2
  %19 = extractvalue %dx.types.ResRet.i32 %15, 3
  %add.2 = add i32 %2, 48
  %20 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %4, i32 %add.2, i32 undef)
  %21 = extractvalue %dx.types.ResRet.i32 %20, 0
  %22 = extractvalue %dx.types.ResRet.i32 %20, 1
  %23 = extractvalue %dx.types.ResRet.i32 %20, 2
  %24 = extractvalue %dx.types.ResRet.i32 %20, 3
  %add.3 = add i32 %2, 64
  %25 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %4, i32 %add.3, i32 undef)
  %26 = extractvalue %dx.types.ResRet.i32 %25, 0
  %27 = extractvalue %dx.types.ResRet.i32 %25, 1
  %28 = extractvalue %dx.types.ResRet.i32 %25, 2
  %29 = extractvalue %dx.types.ResRet.i32 %25, 3
  %add.4 = add i32 %2, 80
  %30 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %4, i32 %add.4, i32 undef)
  %31 = extractvalue %dx.types.ResRet.i32 %30, 0
  %32 = extractvalue %dx.types.ResRet.i32 %30, 1
  %33 = extractvalue %dx.types.ResRet.i32 %30, 2
  %34 = extractvalue %dx.types.ResRet.i32 %30, 3
  %add.5 = add i32 %2, 96
  %35 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %4, i32 %add.5, i32 undef)
  %36 = extractvalue %dx.types.ResRet.i32 %35, 0
  %37 = extractvalue %dx.types.ResRet.i32 %35, 1
  %38 = extractvalue %dx.types.ResRet.i32 %35, 2
  %39 = extractvalue %dx.types.ResRet.i32 %35, 3
  %add.6 = add i32 %2, 112
  %.upto0134 = insertelement <2 x i32> undef, i32 %23, i32 0
  %40 = insertelement <2 x i32> %.upto0134, i32 %24, i32 1
  %41 = getelementptr inbounds %struct.BVHMetadata, %struct.BVHMetadata* %0, i32 0, i32 0, i32 0, i32 0
  %42 = bitcast i32 %6 to float
  %insert76 = insertelement <4 x float> undef, float %42, i64 0
  %43 = bitcast i32 %7 to float
  %insert78 = insertelement <4 x float> %insert76, float %43, i64 1
  %44 = bitcast i32 %8 to float
  %insert80 = insertelement <4 x float> %insert78, float %44, i64 2
  %45 = bitcast i32 %9 to float
  %insert82 = insertelement <4 x float> %insert80, float %45, i64 3
  store <4 x float> %insert82, <4 x float>* %41, align 4
  %46 = getelementptr inbounds %struct.BVHMetadata, %struct.BVHMetadata* %0, i32 0, i32 0, i32 0, i32 1
  %47 = bitcast i32 %11 to float
  %insert84 = insertelement <4 x float> undef, float %47, i64 0
  %48 = bitcast i32 %12 to float
  %insert86 = insertelement <4 x float> %insert84, float %48, i64 1
  %49 = bitcast i32 %13 to float
  %insert88 = insertelement <4 x float> %insert86, float %49, i64 2
  %50 = bitcast i32 %14 to float
  %insert90 = insertelement <4 x float> %insert88, float %50, i64 3
  store <4 x float> %insert90, <4 x float>* %46, align 4
  %51 = getelementptr inbounds %struct.BVHMetadata, %struct.BVHMetadata* %0, i32 0, i32 0, i32 0, i32 2
  %52 = bitcast i32 %16 to float
  %insert92 = insertelement <4 x float> undef, float %52, i64 0
  %53 = bitcast i32 %17 to float
  %insert94 = insertelement <4 x float> %insert92, float %53, i64 1
  %54 = bitcast i32 %18 to float
  %insert96 = insertelement <4 x float> %insert94, float %54, i64 2
  %55 = bitcast i32 %19 to float
  %insert98 = insertelement <4 x float> %insert96, float %55, i64 3
  store <4 x float> %insert98, <4 x float>* %51, align 4
  %56 = getelementptr inbounds %struct.BVHMetadata, %struct.BVHMetadata* %0, i32 0, i32 0, i32 1
  store i32 %21, i32* %56, align 4
  %57 = getelementptr inbounds %struct.BVHMetadata, %struct.BVHMetadata* %0, i32 0, i32 0, i32 2
  store i32 %22, i32* %57, align 4
  %58 = getelementptr inbounds %struct.BVHMetadata, %struct.BVHMetadata* %0, i32 0, i32 0, i32 3
  store <2 x i32> %40, <2 x i32>* %58, align 4
  %.i0116 = bitcast i32 %26 to float
  %.i1117 = bitcast i32 %27 to float
  %.i2118 = bitcast i32 %28 to float
  %.i3119 = bitcast i32 %29 to float
  %.upto0136 = insertelement <4 x float> undef, float %.i0116, i32 0
  %.upto1137 = insertelement <4 x float> %.upto0136, float %.i1117, i32 1
  %.upto2138 = insertelement <4 x float> %.upto1137, float %.i2118, i32 2
  %59 = insertelement <4 x float> %.upto2138, float %.i3119, i32 3
  %arrayidx8 = getelementptr inbounds %struct.BVHMetadata, %struct.BVHMetadata* %0, i32 0, i32 1, i32 0
  store <4 x float> %59, <4 x float>* %arrayidx8, align 4, !tbaa !220
  %.i0120 = bitcast i32 %31 to float
  %.i1121 = bitcast i32 %32 to float
  %.i2122 = bitcast i32 %33 to float
  %.i3123 = bitcast i32 %34 to float
  %.upto0139 = insertelement <4 x float> undef, float %.i0120, i32 0
  %.upto1140 = insertelement <4 x float> %.upto0139, float %.i1121, i32 1
  %.upto2141 = insertelement <4 x float> %.upto1140, float %.i2122, i32 2
  %60 = insertelement <4 x float> %.upto2141, float %.i3123, i32 3
  %arrayidx12 = getelementptr inbounds %struct.BVHMetadata, %struct.BVHMetadata* %0, i32 0, i32 1, i32 1
  store <4 x float> %60, <4 x float>* %arrayidx12, align 4, !tbaa !220
  %.i0124 = bitcast i32 %36 to float
  %.i1125 = bitcast i32 %37 to float
  %.i2126 = bitcast i32 %38 to float
  %.i3127 = bitcast i32 %39 to float
  %.upto0142 = insertelement <4 x float> undef, float %.i0124, i32 0
  %.upto1143 = insertelement <4 x float> %.upto0142, float %.i1125, i32 1
  %.upto2144 = insertelement <4 x float> %.upto1143, float %.i2126, i32 2
  %61 = insertelement <4 x float> %.upto2144, float %.i3127, i32 3
  %arrayidx16 = getelementptr inbounds %struct.BVHMetadata, %struct.BVHMetadata* %0, i32 0, i32 1, i32 2
  store <4 x float> %61, <4 x float>* %arrayidx16, align 4, !tbaa !220
  %62 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %1, align 4
  %63 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %62)
  %64 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %63, i32 %add.6, i32 undef)
  %65 = extractvalue %dx.types.ResRet.i32 %64, 0
  %InstanceIndex = getelementptr inbounds %struct.BVHMetadata, %struct.BVHMetadata* %0, i32 0, i32 2
  store i32 %65, i32* %InstanceIndex, align 4, !tbaa !223
  ret void
}

; Function Attrs: alwaysinline nounwind readnone
define i1 @"\01?IsOpaque@@YA_N_NII@Z"(i1 zeroext, i32, i32) #4 {
entry:
  %and = and i32 %1, 4
  %tobool2 = icmp eq i32 %and, 0
  br i1 %tobool2, label %if.else, label %if.end.6

if.else:                                          ; preds = %entry
  %frombool1 = zext i1 %0 to i32
  %and3 = and i32 %1, 8
  %tobool4 = icmp eq i32 %and3, 0
  %frombool1. = select i1 %tobool4, i32 %frombool1, i32 0
  br label %if.end.6

if.end.6:                                         ; preds = %if.else, %entry
  %opaque.0 = phi i32 [ 1, %entry ], [ %frombool1., %if.else ]
  %and7 = and i32 %2, 1
  %tobool8 = icmp eq i32 %and7, 0
  br i1 %tobool8, label %if.else.10, label %if.end.15

if.else.10:                                       ; preds = %if.end.6
  %and11 = and i32 %2, 2
  %tobool12 = icmp eq i32 %and11, 0
  %opaque.0. = select i1 %tobool12, i32 %opaque.0, i32 0
  br label %if.end.15

if.end.15:                                        ; preds = %if.else.10, %if.end.6
  %opaque.1 = phi i32 [ 1, %if.end.6 ], [ %opaque.0., %if.else.10 ]
  %tobool16 = icmp ne i32 %opaque.1, 0
  ret i1 %tobool16
}

; Function Attrs: alwaysinline nounwind
define void @"\01?GetBoxFromBuffer@@YA?AUBoundingBox@@URWByteAddressBuffer@@II@Z"(%struct.BoundingBox* noalias nocapture sret, %struct.RWByteAddressBuffer* nocapture readonly, i32, i32) #5 {
entry:
  %mul.i = shl i32 %3, 5
  %add.i = add i32 %mul.i, %2
  %4 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %1, align 4
  %5 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %4)
  %6 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %5, i32 %add.i, i32 undef)
  %7 = extractvalue %dx.types.ResRet.i32 %6, 0
  %8 = extractvalue %dx.types.ResRet.i32 %6, 1
  %9 = extractvalue %dx.types.ResRet.i32 %6, 2
  %add = add i32 %add.i, 16
  %10 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %5, i32 %add, i32 undef)
  %11 = extractvalue %dx.types.ResRet.i32 %10, 0
  %12 = extractvalue %dx.types.ResRet.i32 %10, 1
  %13 = extractvalue %dx.types.ResRet.i32 %10, 2
  %14 = bitcast i32 %7 to float
  %15 = insertelement <3 x float> undef, float %14, i32 0
  %16 = bitcast i32 %8 to float
  %17 = insertelement <3 x float> %15, float %16, i32 1
  %18 = bitcast i32 %9 to float
  %19 = insertelement <3 x float> %17, float %18, i32 2
  %20 = bitcast i32 %11 to float
  %21 = insertelement <3 x float> undef, float %20, i32 0
  %22 = bitcast i32 %12 to float
  %23 = insertelement <3 x float> %21, float %22, i32 1
  %24 = bitcast i32 %13 to float
  %25 = insertelement <3 x float> %23, float %24, i32 2
  %26 = getelementptr inbounds %struct.BoundingBox, %struct.BoundingBox* %0, i32 0, i32 0
  store <3 x float> %19, <3 x float>* %26, align 4
  %27 = getelementptr inbounds %struct.BoundingBox, %struct.BoundingBox* %0, i32 0, i32 1
  store <3 x float> %25, <3 x float>* %27, align 4
  ret void
}

; Function Attrs: alwaysinline nounwind readnone
define float @"\01?Determinant@@YAMV?$matrix@M$02$03@@@Z"(%class.matrix.float.3.4) #4 {
entry:
  %.fca.0.0.extract = extractvalue %class.matrix.float.3.4 %0, 0, 0
  %1 = extractelement <4 x float> %.fca.0.0.extract, i32 2
  %2 = extractelement <4 x float> %.fca.0.0.extract, i32 1
  %3 = extractelement <4 x float> %.fca.0.0.extract, i32 0
  %.fca.0.1.extract = extractvalue %class.matrix.float.3.4 %0, 0, 1
  %4 = extractelement <4 x float> %.fca.0.1.extract, i32 2
  %5 = extractelement <4 x float> %.fca.0.1.extract, i32 1
  %6 = extractelement <4 x float> %.fca.0.1.extract, i32 0
  %.fca.0.2.extract = extractvalue %class.matrix.float.3.4 %0, 0, 2
  %7 = extractelement <4 x float> %.fca.0.2.extract, i32 2
  %8 = extractelement <4 x float> %.fca.0.2.extract, i32 1
  %9 = extractelement <4 x float> %.fca.0.2.extract, i32 0
  %mul = fmul fast float %3, %5
  %mul1 = fmul fast float %mul, %7
  %mul2 = fmul fast float %3, %8
  %mul3 = fmul fast float %mul2, %4
  %sub = fsub fast float %mul1, %mul3
  %mul4 = fmul fast float %6, %2
  %mul5 = fmul fast float %mul4, %7
  %sub6 = fsub fast float %sub, %mul5
  %mul7 = fmul fast float %6, %8
  %mul8 = fmul fast float %mul7, %1
  %add = fadd fast float %sub6, %mul8
  %mul9 = fmul fast float %9, %2
  %mul10 = fmul fast float %mul9, %4
  %add11 = fadd fast float %add, %mul10
  %mul12 = fmul fast float %9, %5
  %mul13 = fmul fast float %mul12, %1
  %sub14 = fsub fast float %add11, %mul13
  ret float %sub14
}

; Function Attrs: alwaysinline nounwind readnone
define float @"\01?ComputeCullFaceDir@@YAMII@Z"(i32, i32) #4 {
entry:
  %and = and i32 %1, 32
  %tobool = icmp eq i32 %and, 0
  br i1 %tobool, label %if.else, label %if.end.4

if.else:                                          ; preds = %entry
  %and1 = and i32 %1, 16
  %tobool2 = icmp eq i32 %and1, 0
  %. = select i1 %tobool2, float 0.000000e+00, float -1.000000e+00
  br label %if.end.4

if.end.4:                                         ; preds = %if.else, %entry
  %cullFaceDir.0 = phi float [ 1.000000e+00, %entry ], [ %., %if.else ]
  %and5 = and i32 %0, 1
  %tobool6 = icmp eq i32 %and5, 0
  %cullFaceDir.0. = select i1 %tobool6, float %cullFaceDir.0, float 0.000000e+00
  ret float %cullFaceDir.0.
}

; Function Attrs: alwaysinline nounwind
define void @"\01?MyRaygenShader@@YAXXZ"() #5 {
entry:
  %0 = load %struct.RaytracingAccelerationStructure, %struct.RaytracingAccelerationStructure* @"\01?Scene@@3URaytracingAccelerationStructure@@A", align 4
  %1 = load %"class.RWTexture2D<vector<float, 4> >", %"class.RWTexture2D<vector<float, 4> >"* @"\01?RenderTarget@@3V?$RWTexture2D@V?$vector@M$03@@@@A", align 4
  %2 = load %g_sceneCB, %g_sceneCB* @g_sceneCB, align 4
  %g_sceneCB = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.g_sceneCB(i32 160, %g_sceneCB %2)
  %3 = alloca %struct.HitData, align 8
  %DispatchRaysIndex = call i32 @dx.op.dispatchRaysIndex.i32(i32 145, i8 0)
  %DispatchRaysIndex5 = call i32 @dx.op.dispatchRaysIndex.i32(i32 145, i8 1)
  %conv.i.i0 = uitofp i32 %DispatchRaysIndex to float
  %conv.i.i1 = uitofp i32 %DispatchRaysIndex5 to float
  %add.i.i0 = fadd fast float %conv.i.i0, 5.000000e-01
  %add.i.i1 = fadd fast float %conv.i.i1, 5.000000e-01
  %DispatchRaysDimensions = call i32 @dx.op.dispatchRaysDimensions.i32(i32 146, i8 0)
  %DispatchRaysDimensions4 = call i32 @dx.op.dispatchRaysDimensions.i32(i32 146, i8 1)
  %conv1.i.i0 = uitofp i32 %DispatchRaysDimensions to float
  %conv1.i.i1 = uitofp i32 %DispatchRaysDimensions4 to float
  %div.i.i0 = fdiv fast float %add.i.i0, %conv1.i.i0
  %div.i.i1 = fdiv fast float %add.i.i1, %conv1.i.i1
  %mul.i.i0 = fmul fast float %div.i.i0, 2.000000e+00
  %mul.i.i1 = fmul fast float %div.i.i1, 2.000000e+00
  %sub.i.i0 = fadd fast float %mul.i.i0, -1.000000e+00
  %sub.i.i1 = fadd fast float %mul.i.i1, -1.000000e+00
  %sub2.i = fsub fast float -0.000000e+00, %sub.i.i1
  %4 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %g_sceneCB, i32 0)
  %5 = extractvalue %dx.types.CBufRet.f32 %4, 0
  %6 = extractvalue %dx.types.CBufRet.f32 %4, 1
  %7 = extractvalue %dx.types.CBufRet.f32 %4, 2
  %8 = extractvalue %dx.types.CBufRet.f32 %4, 3
  %9 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %g_sceneCB, i32 1)
  %10 = extractvalue %dx.types.CBufRet.f32 %9, 0
  %11 = extractvalue %dx.types.CBufRet.f32 %9, 1
  %12 = extractvalue %dx.types.CBufRet.f32 %9, 2
  %13 = extractvalue %dx.types.CBufRet.f32 %9, 3
  %14 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %g_sceneCB, i32 2)
  %15 = extractvalue %dx.types.CBufRet.f32 %14, 0
  %16 = extractvalue %dx.types.CBufRet.f32 %14, 1
  %17 = extractvalue %dx.types.CBufRet.f32 %14, 2
  %18 = extractvalue %dx.types.CBufRet.f32 %14, 3
  %19 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %g_sceneCB, i32 3)
  %20 = extractvalue %dx.types.CBufRet.f32 %19, 0
  %21 = extractvalue %dx.types.CBufRet.f32 %19, 1
  %22 = extractvalue %dx.types.CBufRet.f32 %19, 2
  %23 = extractvalue %dx.types.CBufRet.f32 %19, 3
  %24 = fmul fast float %5, %sub.i.i0
  %FMad18 = call float @dx.op.tertiary.f32(i32 46, float %sub2.i, float %10, float %24)
  %FMad17 = call float @dx.op.tertiary.f32(i32 46, float 0.000000e+00, float %15, float %FMad18)
  %FMad16 = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %20, float %FMad17)
  %25 = fmul fast float %6, %sub.i.i0
  %FMad15 = call float @dx.op.tertiary.f32(i32 46, float %sub2.i, float %11, float %25)
  %FMad14 = call float @dx.op.tertiary.f32(i32 46, float 0.000000e+00, float %16, float %FMad15)
  %FMad13 = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %21, float %FMad14)
  %26 = fmul fast float %7, %sub.i.i0
  %FMad12 = call float @dx.op.tertiary.f32(i32 46, float %sub2.i, float %12, float %26)
  %FMad11 = call float @dx.op.tertiary.f32(i32 46, float 0.000000e+00, float %17, float %FMad12)
  %FMad10 = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %22, float %FMad11)
  %27 = fmul fast float %8, %sub.i.i0
  %FMad9 = call float @dx.op.tertiary.f32(i32 46, float %sub2.i, float %13, float %27)
  %FMad8 = call float @dx.op.tertiary.f32(i32 46, float 0.000000e+00, float %18, float %FMad9)
  %FMad = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %23, float %FMad8)
  %div4.i.i0 = fdiv fast float %FMad16, %FMad
  %div4.i.i1 = fdiv fast float %FMad13, %FMad
  %div4.i.i2 = fdiv fast float %FMad10, %FMad
  %28 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %g_sceneCB, i32 4)
  %29 = extractvalue %dx.types.CBufRet.f32 %28, 0
  %30 = extractvalue %dx.types.CBufRet.f32 %28, 1
  %31 = extractvalue %dx.types.CBufRet.f32 %28, 2
  %sub5.i.i0 = fsub fast float %div4.i.i0, %29
  %sub5.i.i1 = fsub fast float %div4.i.i1, %30
  %sub5.i.i2 = fsub fast float %div4.i.i2, %31
  %32 = fmul fast float %sub5.i.i0, %sub5.i.i0
  %33 = fmul fast float %sub5.i.i1, %sub5.i.i1
  %34 = fadd fast float %32, %33
  %35 = fmul fast float %sub5.i.i2, %sub5.i.i2
  %36 = fadd fast float %34, %35
  %Sqrt = call float @dx.op.unary.f32(i32 24, float %36)
  %.i0 = fdiv fast float %sub5.i.i0, %Sqrt
  %.i1 = fdiv fast float %sub5.i.i1, %Sqrt
  %.i2 = fdiv fast float %sub5.i.i2, %Sqrt
  %37 = getelementptr inbounds %struct.HitData, %struct.HitData* %3, i32 0, i32 0
  store <4 x float> zeroinitializer, <4 x float>* %37, align 8
  %38 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RaytracingAccelerationStructure(i32 160, %struct.RaytracingAccelerationStructure %0)
  call void @dx.op.traceRay.struct.HitData(i32 157, %dx.types.Handle %38, i32 16, i32 -1, i32 0, i32 1, i32 0, float %29, float %30, float %31, float 0x3F50624DE0000000, float %.i0, float %.i1, float %.i2, float 1.000000e+04, %struct.HitData* nonnull %3)
  %39 = load <4 x float>, <4 x float>* %37, align 8
  %40 = call %dx.types.Handle @"dx.op.createHandleFromResourceStructForLib.class.RWTexture2D<vector<float, 4> >"(i32 160, %"class.RWTexture2D<vector<float, 4> >" %1)
  %41 = extractelement <4 x float> %39, i64 0
  %42 = extractelement <4 x float> %39, i64 1
  %43 = extractelement <4 x float> %39, i64 2
  %44 = extractelement <4 x float> %39, i64 3
  call void @dx.op.textureStore.f32(i32 67, %dx.types.Handle %40, i32 %DispatchRaysIndex, i32 %DispatchRaysIndex5, i32 undef, float %41, float %42, float %43, float %44, i8 15)
  ret void
}

; Function Attrs: alwaysinline nounwind
define i32 @"\01?Fallback_ReportHit@@YAHMI@Z"(float, i32) #5 {
entry:
  %RayTMin = call float @dx.op.rayTMin.f32(i32 153)
  %cmp = fcmp fast ogt float %RayTMin, %0
  %call1 = call float @"\01?Fallback_RayTCurrent@@YAMXZ"() #2
  %cmp2 = fcmp fast ole float %call1, %0
  %2 = or i1 %cmp, %cmp2
  br i1 %2, label %return, label %if.end

if.end:                                           ; preds = %entry
  call void @"\01?Fallback_SetPendingRayTCurrent@@YAXM@Z"(float %0) #2
  call void @"\01?Fallback_SetPendingHitKind@@YAXI@Z"(i32 %1) #2
  %call5 = call i32 @"\01?Fallback_AnyHitStateId@@YAHXZ"() #2
  %cmp6 = icmp sgt i32 %call5, 0
  br i1 %cmp6, label %if.then.9, label %if.end.11

if.then.9:                                        ; preds = %if.end
  call void @"\01?Fallback_SetAnyHitResult@@YAXH@Z"(i32 1) #2
  call void @"\01?Fallback_CallIndirect@@YAXH@Z"(i32 %call5) #2
  %call.i = call i32 @"\01?Fallback_AnyHitResult@@YAHXZ"() #2
  br label %if.end.11

if.end.11:                                        ; preds = %if.then.9, %if.end
  %ret.0 = phi i32 [ %call.i, %if.then.9 ], [ 1, %if.end ]
  %cmp12 = icmp eq i32 %ret.0, 0
  br i1 %cmp12, label %return, label %if.then.15

if.then.15:                                       ; preds = %if.end.11
  call void @"\01?Fallback_CommitHit@@YAXXZ"() #2
  %RayFlags = call i32 @dx.op.rayFlags.i32(i32 144)
  %and = and i32 %RayFlags, 4
  %tobool17 = icmp eq i32 %and, 0
  %ret.0. = select i1 %tobool17, i32 %ret.0, i32 -1
  ret i32 %ret.0.

return:                                           ; preds = %if.end.11, %entry
  ret i32 0
}

; Function Attrs: alwaysinline nounwind
define void @"\01?LoadRaytracingInstanceDesc@@YA?AURaytracingInstanceDesc@@UByteAddressBuffer@@I@Z"(%struct.RaytracingInstanceDesc* noalias nocapture sret, %struct.ByteAddressBuffer* nocapture readonly, i32) #5 {
entry:
  %3 = load %struct.ByteAddressBuffer, %struct.ByteAddressBuffer* %1, align 4
  %4 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.ByteAddressBuffer(i32 160, %struct.ByteAddressBuffer %3)
  %5 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %4, i32 %2, i32 undef)
  %6 = extractvalue %dx.types.ResRet.i32 %5, 0
  %7 = extractvalue %dx.types.ResRet.i32 %5, 1
  %8 = extractvalue %dx.types.ResRet.i32 %5, 2
  %9 = extractvalue %dx.types.ResRet.i32 %5, 3
  %add.1 = add i32 %2, 16
  %10 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %4, i32 %add.1, i32 undef)
  %11 = extractvalue %dx.types.ResRet.i32 %10, 0
  %12 = extractvalue %dx.types.ResRet.i32 %10, 1
  %13 = extractvalue %dx.types.ResRet.i32 %10, 2
  %14 = extractvalue %dx.types.ResRet.i32 %10, 3
  %add.2 = add i32 %2, 32
  %15 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %4, i32 %add.2, i32 undef)
  %16 = extractvalue %dx.types.ResRet.i32 %15, 0
  %17 = extractvalue %dx.types.ResRet.i32 %15, 1
  %18 = extractvalue %dx.types.ResRet.i32 %15, 2
  %19 = extractvalue %dx.types.ResRet.i32 %15, 3
  %add.3 = add i32 %2, 48
  %20 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %4, i32 %add.3, i32 undef)
  %21 = extractvalue %dx.types.ResRet.i32 %20, 0
  %22 = extractvalue %dx.types.ResRet.i32 %20, 1
  %23 = extractvalue %dx.types.ResRet.i32 %20, 2
  %24 = extractvalue %dx.types.ResRet.i32 %20, 3
  %.i0 = bitcast i32 %6 to float
  %.i1 = bitcast i32 %7 to float
  %.i2 = bitcast i32 %8 to float
  %.i3 = bitcast i32 %9 to float
  %.upto0 = insertelement <4 x float> undef, float %.i0, i32 0
  %.upto1 = insertelement <4 x float> %.upto0, float %.i1, i32 1
  %.upto2 = insertelement <4 x float> %.upto1, float %.i2, i32 2
  %25 = insertelement <4 x float> %.upto2, float %.i3, i32 3
  %arrayidx.i = getelementptr inbounds %struct.RaytracingInstanceDesc, %struct.RaytracingInstanceDesc* %0, i32 0, i32 0, i32 0
  store <4 x float> %25, <4 x float>* %arrayidx.i, align 4, !tbaa !220, !alias.scope !225
  %.i040 = bitcast i32 %11 to float
  %.i141 = bitcast i32 %12 to float
  %.i242 = bitcast i32 %13 to float
  %.i343 = bitcast i32 %14 to float
  %.upto048 = insertelement <4 x float> undef, float %.i040, i32 0
  %.upto149 = insertelement <4 x float> %.upto048, float %.i141, i32 1
  %.upto250 = insertelement <4 x float> %.upto149, float %.i242, i32 2
  %26 = insertelement <4 x float> %.upto250, float %.i343, i32 3
  %arrayidx3.i = getelementptr inbounds %struct.RaytracingInstanceDesc, %struct.RaytracingInstanceDesc* %0, i32 0, i32 0, i32 1
  store <4 x float> %26, <4 x float>* %arrayidx3.i, align 4, !tbaa !220, !alias.scope !225
  %.i044 = bitcast i32 %16 to float
  %.i145 = bitcast i32 %17 to float
  %.i246 = bitcast i32 %18 to float
  %.i347 = bitcast i32 %19 to float
  %.upto051 = insertelement <4 x float> undef, float %.i044, i32 0
  %.upto152 = insertelement <4 x float> %.upto051, float %.i145, i32 1
  %.upto253 = insertelement <4 x float> %.upto152, float %.i246, i32 2
  %27 = insertelement <4 x float> %.upto253, float %.i347, i32 3
  %arrayidx6.i = getelementptr inbounds %struct.RaytracingInstanceDesc, %struct.RaytracingInstanceDesc* %0, i32 0, i32 0, i32 2
  store <4 x float> %27, <4 x float>* %arrayidx6.i, align 4, !tbaa !220, !alias.scope !225
  %InstanceIDAndMask.i = getelementptr inbounds %struct.RaytracingInstanceDesc, %struct.RaytracingInstanceDesc* %0, i32 0, i32 1
  store i32 %21, i32* %InstanceIDAndMask.i, align 4, !tbaa !223, !alias.scope !225
  %InstanceContributionToHitGroupIndexAndFlags.i = getelementptr inbounds %struct.RaytracingInstanceDesc, %struct.RaytracingInstanceDesc* %0, i32 0, i32 2
  store i32 %22, i32* %InstanceContributionToHitGroupIndexAndFlags.i, align 4, !tbaa !223, !alias.scope !225
  %.upto054 = insertelement <2 x i32> undef, i32 %23, i32 0
  %28 = insertelement <2 x i32> %.upto054, i32 %24, i32 1
  %AccelerationStructure.i = getelementptr inbounds %struct.RaytracingInstanceDesc, %struct.RaytracingInstanceDesc* %0, i32 0, i32 3
  store <2 x i32> %28, <2 x i32>* %AccelerationStructure.i, align 4, !tbaa !220, !alias.scope !225
  ret void
}

; Function Attrs: alwaysinline nounwind
define void @"\01?GetBoxDataFromTriangle@@YA?AUBoundingBox@@V?$vector@M$02@@00HAIAV?$vector@I$01@@@Z"(%struct.BoundingBox* noalias nocapture sret, <3 x float>, <3 x float>, <3 x float>, i32, <2 x i32>* noalias nocapture dereferenceable(8)) #5 {
entry:
  %6 = extractelement <3 x float> %1, i64 0
  %7 = extractelement <3 x float> %2, i64 0
  %FMin = call float @dx.op.binary.f32(i32 36, float %6, float %7)
  %8 = extractelement <3 x float> %1, i64 1
  %9 = extractelement <3 x float> %2, i64 1
  %FMin13 = call float @dx.op.binary.f32(i32 36, float %8, float %9)
  %10 = extractelement <3 x float> %1, i64 2
  %11 = extractelement <3 x float> %2, i64 2
  %FMin14 = call float @dx.op.binary.f32(i32 36, float %10, float %11)
  %12 = extractelement <3 x float> %3, i64 0
  %FMin15 = call float @dx.op.binary.f32(i32 36, float %FMin, float %12)
  %13 = extractelement <3 x float> %3, i64 1
  %FMin16 = call float @dx.op.binary.f32(i32 36, float %FMin13, float %13)
  %14 = extractelement <3 x float> %3, i64 2
  %FMin17 = call float @dx.op.binary.f32(i32 36, float %FMin14, float %14)
  %FMax = call float @dx.op.binary.f32(i32 35, float %6, float %7)
  %FMax8 = call float @dx.op.binary.f32(i32 35, float %8, float %9)
  %FMax9 = call float @dx.op.binary.f32(i32 35, float %10, float %11)
  %FMax10 = call float @dx.op.binary.f32(i32 35, float %FMax, float %12)
  %FMax11 = call float @dx.op.binary.f32(i32 35, float %FMax8, float %13)
  %FMax12 = call float @dx.op.binary.f32(i32 35, float %FMax9, float %14)
  %sub.i0 = fadd fast float %FMax10, 0xBF50624DE0000000
  %sub.i1 = fadd fast float %FMax11, 0xBF50624DE0000000
  %sub.i2 = fadd fast float %FMax12, 0xBF50624DE0000000
  %FMin18 = call float @dx.op.binary.f32(i32 36, float %FMin15, float %sub.i0)
  %FMin19 = call float @dx.op.binary.f32(i32 36, float %FMin16, float %sub.i1)
  %FMin20 = call float @dx.op.binary.f32(i32 36, float %FMin17, float %sub.i2)
  %add.i.i0 = fadd fast float %FMin18, %FMax10
  %add.i.i1 = fadd fast float %FMin19, %FMax11
  %add.i.i2 = fadd fast float %FMin20, %FMax12
  %mul.i.i0 = fmul fast float %add.i.i0, 5.000000e-01
  %mul.i.i1 = fmul fast float %add.i.i1, 5.000000e-01
  %mul.i.i2 = fmul fast float %add.i.i2, 5.000000e-01
  %mul.i.upto0 = insertelement <3 x float> undef, float %mul.i.i0, i32 0
  %mul.i.upto1 = insertelement <3 x float> %mul.i.upto0, float %mul.i.i1, i32 1
  %mul.i = insertelement <3 x float> %mul.i.upto1, float %mul.i.i2, i32 2
  %sub.i.i0 = fsub fast float %FMax10, %mul.i.i0
  %sub.i.i1 = fsub fast float %FMax11, %mul.i.i1
  %sub.i.i2 = fsub fast float %FMax12, %mul.i.i2
  %sub.i.upto0 = insertelement <3 x float> undef, float %sub.i.i0, i32 0
  %sub.i.upto1 = insertelement <3 x float> %sub.i.upto0, float %sub.i.i1, i32 1
  %sub.i = insertelement <3 x float> %sub.i.upto1, float %sub.i.i2, i32 2
  %or = or i32 %4, -2147483648
  %15 = insertelement <2 x i32> undef, i32 %or, i32 0
  %16 = insertelement <2 x i32> %15, i32 1, i32 1
  %17 = getelementptr inbounds %struct.BoundingBox, %struct.BoundingBox* %0, i32 0, i32 0
  store <3 x float> %mul.i, <3 x float>* %17, align 4
  %18 = getelementptr inbounds %struct.BoundingBox, %struct.BoundingBox* %0, i32 0, i32 1
  store <3 x float> %sub.i, <3 x float>* %18, align 4
  store <2 x i32> %16, <2 x i32>* %5, align 4
  ret void
}

; Function Attrs: alwaysinline nounwind
define void @"\01?BVHReadBoundingBox@@YA?AUBoundingBox@@URWByteAddressBufferPointer@@HAIAV?$vector@I$01@@@Z"(%struct.BoundingBox* noalias nocapture sret, %struct.RWByteAddressBufferPointer* nocapture readonly, i32, <2 x i32>* noalias nocapture dereferenceable(8)) #5 {
entry:
  %offsetInBytes.i = getelementptr inbounds %struct.RWByteAddressBufferPointer, %struct.RWByteAddressBufferPointer* %1, i32 0, i32 1
  %4 = load i32, i32* %offsetInBytes.i, align 4, !tbaa !223
  %mul.i = shl i32 %2, 5
  %add.i = or i32 %mul.i, 16
  %add.i.5 = add i32 %add.i, %4
  %5 = getelementptr inbounds %struct.RWByteAddressBufferPointer, %struct.RWByteAddressBufferPointer* %1, i32 0, i32 0
  %6 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %5, align 4
  %7 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %6)
  %8 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %7, i32 %add.i.5, i32 undef)
  %9 = extractvalue %dx.types.ResRet.i32 %8, 0
  %10 = extractvalue %dx.types.ResRet.i32 %8, 1
  %11 = extractvalue %dx.types.ResRet.i32 %8, 2
  %12 = extractvalue %dx.types.ResRet.i32 %8, 3
  %add = add i32 %add.i.5, 16
  %13 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %7, i32 %add, i32 undef)
  %14 = extractvalue %dx.types.ResRet.i32 %13, 0
  %15 = extractvalue %dx.types.ResRet.i32 %13, 1
  %16 = extractvalue %dx.types.ResRet.i32 %13, 2
  %17 = extractvalue %dx.types.ResRet.i32 %13, 3
  %18 = bitcast i32 %9 to float
  %19 = insertelement <3 x float> undef, float %18, i32 0
  %20 = bitcast i32 %10 to float
  %21 = insertelement <3 x float> %19, float %20, i32 1
  %22 = bitcast i32 %11 to float
  %23 = insertelement <3 x float> %21, float %22, i32 2
  %24 = bitcast i32 %14 to float
  %25 = insertelement <3 x float> undef, float %24, i32 0
  %26 = bitcast i32 %15 to float
  %27 = insertelement <3 x float> %25, float %26, i32 1
  %28 = bitcast i32 %16 to float
  %29 = insertelement <3 x float> %27, float %28, i32 2
  %30 = insertelement <2 x i32> undef, i32 %12, i64 0
  %31 = insertelement <2 x i32> %30, i32 %17, i64 1
  store <2 x i32> %31, <2 x i32>* %3, align 4, !tbaa !220
  %32 = getelementptr inbounds %struct.BoundingBox, %struct.BoundingBox* %0, i32 0, i32 0
  store <3 x float> %23, <3 x float>* %32, align 4
  %33 = getelementptr inbounds %struct.BoundingBox, %struct.BoundingBox* %0, i32 0, i32 1
  store <3 x float> %29, <3 x float>* %33, align 4
  ret void
}

; Function Attrs: alwaysinline nounwind readnone
define i1 @"\01?Cull@@YA_N_NI@Z"(i1 zeroext, i32) #4 {
entry:
  %and = and i32 %1, 64
  %tobool1 = icmp ne i32 %and, 0
  %2 = and i1 %tobool1, %0
  %lnot = xor i1 %0, true
  %and3 = and i32 %1, 128
  %tobool4 = icmp ne i32 %and3, 0
  %3 = and i1 %tobool4, %lnot
  %4 = or i1 %2, %3
  ret i1 %4
}

; Function Attrs: alwaysinline nounwind
define i1 @"\01?IntersectCustomPrimitiveFrontToBack@@YA_NV?$vector@M$02@@0MMAIAMUProceduralPrimitiveAttributes@@@Z"(<3 x float>, <3 x float>, float, float, float* noalias nocapture dereferenceable(4), %struct.ProceduralPrimitiveAttributes* noalias nocapture) #5 {
entry:
  %6 = load %"class.StructuredBuffer<AABBPrimitiveAttributes>", %"class.StructuredBuffer<AABBPrimitiveAttributes>"* @"\01?g_AABBPrimitiveAttributes@@3V?$StructuredBuffer@UAABBPrimitiveAttributes@@@@A", align 4
  %PrimitiveID = call i32 @dx.op.primitiveID.i32(i32 108)
  %7 = call %dx.types.Handle @"dx.op.createHandleFromResourceStructForLib.class.StructuredBuffer<AABBPrimitiveAttributes>"(i32 160, %"class.StructuredBuffer<AABBPrimitiveAttributes>" %6)
  %8 = call %dx.types.ResRet.f32 @dx.op.bufferLoad.f32(i32 68, %dx.types.Handle %7, i32 %PrimitiveID, i32 0)
  %9 = extractvalue %dx.types.ResRet.f32 %8, 0
  %10 = extractvalue %dx.types.ResRet.f32 %8, 1
  %11 = extractvalue %dx.types.ResRet.f32 %8, 2
  %12 = call %dx.types.ResRet.f32 @dx.op.bufferLoad.f32(i32 68, %dx.types.Handle %7, i32 %PrimitiveID, i32 16)
  %13 = extractvalue %dx.types.ResRet.f32 %12, 0
  %14 = extractvalue %dx.types.ResRet.f32 %12, 1
  %15 = extractvalue %dx.types.ResRet.f32 %12, 2
  %16 = call %dx.types.ResRet.f32 @dx.op.bufferLoad.f32(i32 68, %dx.types.Handle %7, i32 %PrimitiveID, i32 32)
  %17 = extractvalue %dx.types.ResRet.f32 %16, 0
  %18 = extractvalue %dx.types.ResRet.f32 %16, 1
  %19 = extractvalue %dx.types.ResRet.f32 %16, 2
  %20 = call %dx.types.ResRet.f32 @dx.op.bufferLoad.f32(i32 68, %dx.types.Handle %7, i32 %PrimitiveID, i32 48)
  %21 = extractvalue %dx.types.ResRet.f32 %20, 0
  %22 = extractvalue %dx.types.ResRet.f32 %20, 1
  %23 = extractvalue %dx.types.ResRet.f32 %20, 2
  %24 = extractelement <3 x float> %0, i64 0
  %25 = extractelement <3 x float> %0, i64 1
  %26 = extractelement <3 x float> %0, i64 2
  %27 = fmul fast float %9, %24
  %FMad279 = call float @dx.op.tertiary.f32(i32 46, float %25, float %13, float %27)
  %FMad278 = call float @dx.op.tertiary.f32(i32 46, float %26, float %17, float %FMad279)
  %FMad277 = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %21, float %FMad278)
  %28 = fmul fast float %10, %24
  %FMad276 = call float @dx.op.tertiary.f32(i32 46, float %25, float %14, float %28)
  %FMad275 = call float @dx.op.tertiary.f32(i32 46, float %26, float %18, float %FMad276)
  %FMad274 = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %22, float %FMad275)
  %29 = fmul fast float %11, %24
  %FMad273 = call float @dx.op.tertiary.f32(i32 46, float %25, float %15, float %29)
  %FMad272 = call float @dx.op.tertiary.f32(i32 46, float %26, float %19, float %FMad273)
  %FMad271 = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %23, float %FMad272)
  %30 = extractelement <3 x float> %1, i64 0
  %31 = extractelement <3 x float> %1, i64 1
  %32 = extractelement <3 x float> %1, i64 2
  %33 = fmul fast float %9, %30
  %FMad251 = call float @dx.op.tertiary.f32(i32 46, float %31, float %13, float %33)
  %FMad250 = call float @dx.op.tertiary.f32(i32 46, float %32, float %17, float %FMad251)
  %FMad249 = call float @dx.op.tertiary.f32(i32 46, float 0.000000e+00, float %21, float %FMad250)
  %34 = fmul fast float %10, %30
  %FMad248 = call float @dx.op.tertiary.f32(i32 46, float %31, float %14, float %34)
  %FMad247 = call float @dx.op.tertiary.f32(i32 46, float %32, float %18, float %FMad248)
  %FMad246 = call float @dx.op.tertiary.f32(i32 46, float 0.000000e+00, float %22, float %FMad247)
  %35 = fmul fast float %11, %30
  %FMad245 = call float @dx.op.tertiary.f32(i32 46, float %31, float %15, float %35)
  %FMad244 = call float @dx.op.tertiary.f32(i32 46, float %32, float %19, float %FMad245)
  %FMad243 = call float @dx.op.tertiary.f32(i32 46, float 0.000000e+00, float %23, float %FMad244)
  %rem = urem i32 %PrimitiveID, 3
  switch i32 %rem, label %return [
    i32 0, label %sw.bb
    i32 2, label %sw.bb.9
    i32 1, label %sw.bb.11
  ]

sw.bb:                                            ; preds = %entry
  %sub.i = fsub fast float -1.000000e+00, %FMad277
  %div.i = fdiv fast float %sub.i, %FMad249
  %sub2.i = fsub fast float 1.000000e+00, %FMad277
  %div3.i = fdiv fast float %sub2.i, %FMad249
  %cmp.i = fcmp fast olt float %FMad249, 0.000000e+00
  %tmin.i.0 = select i1 %cmp.i, float %div3.i, float %div.i
  %tmax.i.0 = select i1 %cmp.i, float %div.i, float %div3.i
  %sub6.i = fsub fast float -1.000000e+00, %FMad274
  %div7.i = fdiv fast float %sub6.i, %FMad246
  %sub9.i = fsub fast float 1.000000e+00, %FMad274
  %div10.i = fdiv fast float %sub9.i, %FMad246
  %cmp11.i = fcmp fast olt float %FMad246, 0.000000e+00
  %tymin.i.0 = select i1 %cmp11.i, float %div10.i, float %div7.i
  %tymax.i.0 = select i1 %cmp11.i, float %div7.i, float %div10.i
  %sub17.i = fsub fast float -1.000000e+00, %FMad271
  %div18.i = fdiv fast float %sub17.i, %FMad243
  %sub20.i = fsub fast float 1.000000e+00, %FMad271
  %div21.i = fdiv fast float %sub20.i, %FMad243
  %cmp22.i = fcmp fast olt float %FMad243, 0.000000e+00
  %tzmin.i.0 = select i1 %cmp22.i, float %div21.i, float %div18.i
  %tzmax.i.0 = select i1 %cmp22.i, float %div18.i, float %div21.i
  %FMax163 = call float @dx.op.binary.f32(i32 35, float %tmin.i.0, float %tymin.i.0)
  %FMax = call float @dx.op.binary.f32(i32 35, float %FMax163, float %tzmin.i.0)
  %FMin162 = call float @dx.op.binary.f32(i32 36, float %tmax.i.0, float %tymax.i.0)
  %FMin = call float @dx.op.binary.f32(i32 36, float %FMin162, float %tzmax.i.0)
  %mul.i.i0 = fmul fast float %FMax, %FMad249
  %mul.i.i1 = fmul fast float %FMax, %FMad246
  %mul.i.i2 = fmul fast float %FMax, %FMad243
  %add.i.i0 = fadd fast float %mul.i.i0, %FMad277
  %add.i.i1 = fadd fast float %mul.i.i1, %FMad274
  %add.i.i2 = fadd fast float %mul.i.i2, %FMad271
  %36 = fmul fast float %add.i.i0, %add.i.i0
  %37 = fmul fast float %add.i.i1, %add.i.i1
  %38 = fadd fast float %36, %37
  %39 = fmul fast float %add.i.i2, %add.i.i2
  %40 = fadd fast float %38, %39
  %Sqrt169 = call float @dx.op.unary.f32(i32 24, float %40)
  %.i0 = fdiv fast float %add.i.i0, %Sqrt169
  %.i1 = fdiv fast float %add.i.i1, %Sqrt169
  %.i2 = fdiv fast float %add.i.i2, %Sqrt169
  %FAbs155 = call float @dx.op.unary.f32(i32 6, float %.i0)
  %FAbs154 = call float @dx.op.unary.f32(i32 6, float %.i1)
  %cmp34.i = fcmp fast ogt float %FAbs155, %FAbs154
  br i1 %cmp34.i, label %if.then.37.i, label %if.else.46.i

if.then.37.i:                                     ; preds = %sw.bb
  %FAbs = call float @dx.op.unary.f32(i32 6, float %.i2)
  %cmp40.i = fcmp fast ogt float %FAbs155, %FAbs
  br i1 %cmp40.i, label %"\01?intersectBox@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@@Z.exit", label %if.else.i

if.else.i:                                        ; preds = %if.then.37.i
  br label %"\01?intersectBox@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@@Z.exit"

if.else.46.i:                                     ; preds = %sw.bb
  %FAbs152 = call float @dx.op.unary.f32(i32 6, float %.i2)
  %cmp49.i = fcmp fast ogt float %FAbs154, %FAbs152
  br i1 %cmp49.i, label %"\01?intersectBox@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@@Z.exit", label %if.else.54.i

if.else.54.i:                                     ; preds = %if.else.46.i
  br label %"\01?intersectBox@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@@Z.exit"

"\01?intersectBox@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@@Z.exit": ; preds = %if.else.54.i, %if.else.46.i, %if.else.i, %if.then.37.i
  %.0111.0.i0 = phi float [ 0.000000e+00, %if.else.i ], [ 0.000000e+00, %if.else.54.i ], [ %.i0, %if.then.37.i ], [ 0.000000e+00, %if.else.46.i ]
  %.0111.0.i1 = phi float [ 0.000000e+00, %if.else.i ], [ 0.000000e+00, %if.else.54.i ], [ 0.000000e+00, %if.then.37.i ], [ %.i1, %if.else.46.i ]
  %.0111.0.i2 = phi float [ %.i2, %if.else.i ], [ %.i2, %if.else.54.i ], [ 0.000000e+00, %if.then.37.i ], [ 0.000000e+00, %if.else.46.i ]
  %ObjectToWorld221 = call float @dx.op.objectToWorld.f32(i32 151, i32 0, i8 0)
  %ObjectToWorld222 = call float @dx.op.objectToWorld.f32(i32 151, i32 0, i8 1)
  %ObjectToWorld223 = call float @dx.op.objectToWorld.f32(i32 151, i32 0, i8 2)
  %ObjectToWorld225 = call float @dx.op.objectToWorld.f32(i32 151, i32 1, i8 0)
  %ObjectToWorld226 = call float @dx.op.objectToWorld.f32(i32 151, i32 1, i8 1)
  %ObjectToWorld227 = call float @dx.op.objectToWorld.f32(i32 151, i32 1, i8 2)
  %ObjectToWorld229 = call float @dx.op.objectToWorld.f32(i32 151, i32 2, i8 0)
  %ObjectToWorld230 = call float @dx.op.objectToWorld.f32(i32 151, i32 2, i8 1)
  %ObjectToWorld231 = call float @dx.op.objectToWorld.f32(i32 151, i32 2, i8 2)
  %41 = fmul fast float %.0111.0.i0, %.0111.0.i0
  %42 = fmul fast float %.0111.0.i1, %.0111.0.i1
  %43 = fadd fast float %42, %41
  %44 = fmul fast float %.0111.0.i2, %.0111.0.i2
  %45 = fadd fast float %43, %44
  %Sqrt168 = call float @dx.op.unary.f32(i32 24, float %45)
  %.i0296 = fdiv fast float %.0111.0.i0, %Sqrt168
  %.i1297 = fdiv fast float %.0111.0.i1, %Sqrt168
  %.i2298 = fdiv fast float %.0111.0.i2, %Sqrt168
  %46 = fmul fast float %.i0296, %ObjectToWorld221
  %FMad267 = call float @dx.op.tertiary.f32(i32 46, float %.i1297, float %ObjectToWorld225, float %46)
  %FMad266 = call float @dx.op.tertiary.f32(i32 46, float %.i2298, float %ObjectToWorld229, float %FMad267)
  %47 = fmul fast float %.i0296, %ObjectToWorld222
  %FMad265 = call float @dx.op.tertiary.f32(i32 46, float %.i1297, float %ObjectToWorld226, float %47)
  %FMad264 = call float @dx.op.tertiary.f32(i32 46, float %.i2298, float %ObjectToWorld230, float %FMad265)
  %48 = fmul fast float %.i0296, %ObjectToWorld223
  %FMad263 = call float @dx.op.tertiary.f32(i32 46, float %.i1297, float %ObjectToWorld227, float %48)
  %FMad262 = call float @dx.op.tertiary.f32(i32 46, float %.i2298, float %ObjectToWorld231, float %FMad263)
  %.upto0319 = insertelement <3 x float> undef, float %FMad266, i32 0
  %.upto1320 = insertelement <3 x float> %.upto0319, float %FMad264, i32 1
  %49 = insertelement <3 x float> %.upto1320, float %FMad262, i32 2
  %cmp63.i = fcmp fast ogt float %FMin, %FMax
  store float %FMax, float* %4, align 4, !tbaa !228
  %50 = getelementptr inbounds %struct.ProceduralPrimitiveAttributes, %struct.ProceduralPrimitiveAttributes* %5, i32 0, i32 0
  store <3 x float> %49, <3 x float>* %50, align 4
  br label %return

sw.bb.9:                                          ; preds = %entry
  %51 = load float, float* %4, align 4, !tbaa !228
  %52 = getelementptr inbounds %struct.ProceduralPrimitiveAttributes, %struct.ProceduralPrimitiveAttributes* %5, i32 0, i32 0
  %53 = load <3 x float>, <3 x float>* %52, align 4
  %.i0302 = extractelement <3 x float> %53, i32 0
  %.i1303 = extractelement <3 x float> %53, i32 1
  %.i2304 = extractelement <3 x float> %53, i32 2
  %54 = call float @dx.op.dot3.f32(i32 55, float %FMad249, float %FMad246, float %FMad243, float %FMad249, float %FMad246, float %FMad243)
  %55 = call float @dx.op.dot3.f32(i32 55, float %FMad249, float %FMad246, float %FMad243, float %FMad277, float %FMad274, float %FMad271)
  %mul.i.19 = fmul fast float %55, 2.000000e+00
  %56 = call float @dx.op.dot3.f32(i32 55, float %FMad277, float %FMad274, float %FMad271, float %FMad277, float %FMad274, float %FMad271)
  %sub4.i = fadd fast float %56, -1.000000e+00
  %mul.i.i = fmul fast float %mul.i.19, %mul.i.19
  %mul1.i.i = fmul fast float %54, 4.000000e+00
  %mul2.i.i = fmul fast float %mul1.i.i, %sub4.i
  %sub.i.i = fsub fast float %mul.i.i, %mul2.i.i
  %cmp.i.i = fcmp fast olt float %sub.i.i, 0.000000e+00
  br i1 %cmp.i.i, label %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i", label %if.else.i.i

if.else.i.i:                                      ; preds = %sw.bb.9
  %cmp4.i.i = fcmp fast oeq float %sub.i.i, 0.000000e+00
  br i1 %cmp4.i.i, label %if.then.7.i.i, label %if.else.9.i.i

if.then.7.i.i:                                    ; preds = %if.else.i.i
  %mul8.i.i = fsub fast float -0.000000e+00, %55
  %div.i.i = fdiv fast float %mul8.i.i, %54
  br label %if.end.19.i.i

if.else.9.i.i:                                    ; preds = %if.else.i.i
  %cmp10.i.i = fcmp fast ogt float %mul.i.19, 0.000000e+00
  br i1 %cmp10.i.i, label %cond.true.i.i, label %cond.false.i.i

cond.true.i.i:                                    ; preds = %if.else.9.i.i
  %Sqrt149 = call float @dx.op.unary.f32(i32 24, float %sub.i.i)
  %add.i.i = fadd fast float %Sqrt149, %mul.i.19
  br label %cond.end.i.i

cond.false.i.i:                                   ; preds = %if.else.9.i.i
  %Sqrt150 = call float @dx.op.unary.f32(i32 24, float %sub.i.i)
  %sub15.i.i = fsub fast float %mul.i.19, %Sqrt150
  br label %cond.end.i.i

cond.end.i.i:                                     ; preds = %cond.false.i.i, %cond.true.i.i
  %add.sink.i.i = phi float [ %add.i.i, %cond.true.i.i ], [ %sub15.i.i, %cond.false.i.i ]
  %mul13.i.i = fmul fast float %add.sink.i.i, -5.000000e-01
  %div17.i.i = fdiv fast float %mul13.i.i, %54
  %div18.i.i = fdiv fast float %sub4.i, %mul13.i.i
  br label %if.end.19.i.i

if.end.19.i.i:                                    ; preds = %cond.end.i.i, %if.then.7.i.i
  %.0124 = phi float [ %div.i.i, %if.then.7.i.i ], [ %div18.i.i, %cond.end.i.i ]
  %.0121 = phi float [ %div.i.i, %if.then.7.i.i ], [ %div17.i.i, %cond.end.i.i ]
  %cmp20.i.i = fcmp fast ogt float %.0121, %.0124
  %.1125 = select i1 %cmp20.i.i, float %.0121, float %.0124
  %.1122 = select i1 %cmp20.i.i, float %.0124, float %.0121
  br label %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i"

"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i":      ; preds = %if.end.19.i.i, %sw.bb.9
  %retval.i.i.0 = phi i1 [ false, %sw.bb.9 ], [ true, %if.end.19.i.i ]
  %.2126 = phi float [ undef, %sw.bb.9 ], [ %.1125, %if.end.19.i.i ]
  %.2123 = phi float [ undef, %sw.bb.9 ], [ %.1122, %if.end.19.i.i ]
  br i1 %retval.i.i.0, label %if.end.i.22, label %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit"

if.end.i.22:                                      ; preds = %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i"
  %cmp.i.21 = fcmp fast ogt float %.2123, %.2126
  %t0.i.0 = select i1 %cmp.i.21, float %.2126, float %.2123
  %t1.i.0 = select i1 %cmp.i.21, float %.2123, float %.2126
  %cmp9.i = fcmp fast olt float %t0.i.0, 0.000000e+00
  br i1 %cmp9.i, label %if.then.12.i, label %if.end.18.i

if.then.12.i:                                     ; preds = %if.end.i.22
  %cmp13.i = fcmp fast olt float %t1.i.0, 0.000000e+00
  br i1 %cmp13.i, label %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit", label %if.end.18.i

if.end.18.i:                                      ; preds = %if.then.12.i, %if.end.i.22
  %t0.i.1 = phi float [ %t1.i.0, %if.then.12.i ], [ %t0.i.0, %if.end.i.22 ]
  %mul19.i.i0 = fmul fast float %t0.i.1, %FMad249
  %mul19.i.i1 = fmul fast float %t0.i.1, %FMad246
  %mul19.i.i2 = fmul fast float %t0.i.1, %FMad243
  %add.i.25.i0 = fadd fast float %mul19.i.i0, %FMad277
  %add.i.25.i1 = fadd fast float %mul19.i.i1, %FMad274
  %add.i.25.i2 = fadd fast float %mul19.i.i2, %FMad271
  %ObjectToWorld209 = call float @dx.op.objectToWorld.f32(i32 151, i32 0, i8 0)
  %ObjectToWorld210 = call float @dx.op.objectToWorld.f32(i32 151, i32 0, i8 1)
  %ObjectToWorld211 = call float @dx.op.objectToWorld.f32(i32 151, i32 0, i8 2)
  %ObjectToWorld213 = call float @dx.op.objectToWorld.f32(i32 151, i32 1, i8 0)
  %ObjectToWorld214 = call float @dx.op.objectToWorld.f32(i32 151, i32 1, i8 1)
  %ObjectToWorld215 = call float @dx.op.objectToWorld.f32(i32 151, i32 1, i8 2)
  %ObjectToWorld217 = call float @dx.op.objectToWorld.f32(i32 151, i32 2, i8 0)
  %ObjectToWorld218 = call float @dx.op.objectToWorld.f32(i32 151, i32 2, i8 1)
  %ObjectToWorld219 = call float @dx.op.objectToWorld.f32(i32 151, i32 2, i8 2)
  %57 = fmul fast float %add.i.25.i0, %add.i.25.i0
  %58 = fmul fast float %add.i.25.i1, %add.i.25.i1
  %59 = fadd fast float %57, %58
  %60 = fmul fast float %add.i.25.i2, %add.i.25.i2
  %61 = fadd fast float %59, %60
  %Sqrt167 = call float @dx.op.unary.f32(i32 24, float %61)
  %.i0299 = fdiv fast float %add.i.25.i0, %Sqrt167
  %.i1300 = fdiv fast float %add.i.25.i1, %Sqrt167
  %.i2301 = fdiv fast float %add.i.25.i2, %Sqrt167
  %62 = fmul fast float %.i0299, %ObjectToWorld209
  %FMad259 = call float @dx.op.tertiary.f32(i32 46, float %.i1300, float %ObjectToWorld213, float %62)
  %FMad258 = call float @dx.op.tertiary.f32(i32 46, float %.i2301, float %ObjectToWorld217, float %FMad259)
  %63 = fmul fast float %.i0299, %ObjectToWorld210
  %FMad257 = call float @dx.op.tertiary.f32(i32 46, float %.i1300, float %ObjectToWorld214, float %63)
  %FMad256 = call float @dx.op.tertiary.f32(i32 46, float %.i2301, float %ObjectToWorld218, float %FMad257)
  %64 = fmul fast float %.i0299, %ObjectToWorld211
  %FMad255 = call float @dx.op.tertiary.f32(i32 46, float %.i1300, float %ObjectToWorld215, float %64)
  %FMad254 = call float @dx.op.tertiary.f32(i32 46, float %.i2301, float %ObjectToWorld219, float %FMad255)
  br label %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit"

"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit": ; preds = %if.end.18.i, %if.then.12.i, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i"
  %retval.i.0 = phi i1 [ true, %if.end.18.i ], [ false, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i" ], [ false, %if.then.12.i ]
  %.0110.0.i0 = phi float [ %FMad258, %if.end.18.i ], [ %.i0302, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i" ], [ %.i0302, %if.then.12.i ]
  %.0110.0.i1 = phi float [ %FMad256, %if.end.18.i ], [ %.i1303, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i" ], [ %.i1303, %if.then.12.i ]
  %.0110.0.i2 = phi float [ %FMad254, %if.end.18.i ], [ %.i2304, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i" ], [ %.i2304, %if.then.12.i ]
  %.0113 = phi float [ %t0.i.1, %if.end.18.i ], [ %51, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i" ], [ %51, %if.then.12.i ]
  %.0110.0.upto0 = insertelement <3 x float> undef, float %.0110.0.i0, i32 0
  %.0110.0.upto1 = insertelement <3 x float> %.0110.0.upto0, float %.0110.0.i1, i32 1
  %.0110.0 = insertelement <3 x float> %.0110.0.upto1, float %.0110.0.i2, i32 2
  store float %.0113, float* %4, align 4, !tbaa !228
  store <3 x float> %.0110.0, <3 x float>* %52, align 4
  br label %return

sw.bb.11:                                         ; preds = %entry
  %65 = load float, float* %4, align 4, !tbaa !228
  %66 = getelementptr inbounds %struct.ProceduralPrimitiveAttributes, %struct.ProceduralPrimitiveAttributes* %5, i32 0, i32 0
  %67 = load <3 x float>, <3 x float>* %66, align 4
  %.i0308 = extractelement <3 x float> %67, i32 0
  %.i1309 = extractelement <3 x float> %67, i32 1
  %.i2310 = extractelement <3 x float> %67, i32 2
  %sub.i.i.35.i0 = fadd fast float %FMad277, 0x3FD3333340000000
  %sub.i.i.35.i1 = fadd fast float %FMad274, 0x3FD3333340000000
  %sub.i.i.35.i2 = fadd fast float %FMad271, 0x3FD3333340000000
  %68 = call float @dx.op.dot3.f32(i32 55, float %FMad249, float %FMad246, float %FMad243, float %FMad249, float %FMad246, float %FMad243)
  %69 = call float @dx.op.dot3.f32(i32 55, float %FMad249, float %FMad246, float %FMad243, float %sub.i.i.35.i0, float %sub.i.i.35.i1, float %sub.i.i.35.i2)
  %mul.i.i.36 = fmul fast float %69, 2.000000e+00
  %70 = call float @dx.op.dot3.f32(i32 55, float %sub.i.i.35.i0, float %sub.i.i.35.i1, float %sub.i.i.35.i2, float %sub.i.i.35.i0, float %sub.i.i.35.i1, float %sub.i.i.35.i2)
  %sub4.i.i = fadd fast float %70, 0xBFD70A3D80000000
  %mul.i.i.i = fmul fast float %mul.i.i.36, %mul.i.i.36
  %mul1.i.i.i = fmul fast float %68, 4.000000e+00
  %mul2.i.i.i = fmul fast float %mul1.i.i.i, %sub4.i.i
  %sub.i.i.i = fsub fast float %mul.i.i.i, %mul2.i.i.i
  %cmp.i.i.i = fcmp fast olt float %sub.i.i.i, 0.000000e+00
  br i1 %cmp.i.i.i, label %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.i", label %if.else.i.i.i

if.else.i.i.i:                                    ; preds = %sw.bb.11
  %cmp4.i.i.i = fcmp fast oeq float %sub.i.i.i, 0.000000e+00
  br i1 %cmp4.i.i.i, label %if.then.7.i.i.i, label %if.else.9.i.i.i

if.then.7.i.i.i:                                  ; preds = %if.else.i.i.i
  %mul8.i.i.i = fsub fast float -0.000000e+00, %69
  %div.i.i.i = fdiv fast float %mul8.i.i.i, %68
  br label %if.end.19.i.i.i

if.else.9.i.i.i:                                  ; preds = %if.else.i.i.i
  %cmp10.i.i.i = fcmp fast ogt float %mul.i.i.36, 0.000000e+00
  br i1 %cmp10.i.i.i, label %cond.true.i.i.i, label %cond.false.i.i.i

cond.true.i.i.i:                                  ; preds = %if.else.9.i.i.i
  %Sqrt = call float @dx.op.unary.f32(i32 24, float %sub.i.i.i)
  %add.i.i.i = fadd fast float %Sqrt, %mul.i.i.36
  br label %cond.end.i.i.i

cond.false.i.i.i:                                 ; preds = %if.else.9.i.i.i
  %Sqrt148 = call float @dx.op.unary.f32(i32 24, float %sub.i.i.i)
  %sub15.i.i.i = fsub fast float %mul.i.i.36, %Sqrt148
  br label %cond.end.i.i.i

cond.end.i.i.i:                                   ; preds = %cond.false.i.i.i, %cond.true.i.i.i
  %add.sink.i.i.i = phi float [ %add.i.i.i, %cond.true.i.i.i ], [ %sub15.i.i.i, %cond.false.i.i.i ]
  %mul13.i.i.i = fmul fast float %add.sink.i.i.i, -5.000000e-01
  %div17.i.i.i = fdiv fast float %mul13.i.i.i, %68
  %div18.i.i.i = fdiv fast float %sub4.i.i, %mul13.i.i.i
  br label %if.end.19.i.i.i

if.end.19.i.i.i:                                  ; preds = %cond.end.i.i.i, %if.then.7.i.i.i
  %.0133 = phi float [ %div.i.i.i, %if.then.7.i.i.i ], [ %div18.i.i.i, %cond.end.i.i.i ]
  %.0130 = phi float [ %div.i.i.i, %if.then.7.i.i.i ], [ %div17.i.i.i, %cond.end.i.i.i ]
  %cmp20.i.i.i = fcmp fast ogt float %.0130, %.0133
  %.1134 = select i1 %cmp20.i.i.i, float %.0130, float %.0133
  %.1131 = select i1 %cmp20.i.i.i, float %.0133, float %.0130
  br label %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.i"

"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.i":    ; preds = %if.end.19.i.i.i, %sw.bb.11
  %retval.i.i.i.0 = phi i1 [ false, %sw.bb.11 ], [ true, %if.end.19.i.i.i ]
  %.2135 = phi float [ undef, %sw.bb.11 ], [ %.1134, %if.end.19.i.i.i ]
  %.2132 = phi float [ undef, %sw.bb.11 ], [ %.1131, %if.end.19.i.i.i ]
  br i1 %retval.i.i.i.0, label %if.end.i.i, label %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit.i"

if.end.i.i:                                       ; preds = %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.i"
  %cmp.i.i.38 = fcmp fast ogt float %.2132, %.2135
  %t0.i.i.0 = select i1 %cmp.i.i.38, float %.2135, float %.2132
  %t1.i.i.0 = select i1 %cmp.i.i.38, float %.2132, float %.2135
  %cmp9.i.i = fcmp fast olt float %t0.i.i.0, 0.000000e+00
  br i1 %cmp9.i.i, label %if.then.12.i.i, label %if.end.18.i.i

if.then.12.i.i:                                   ; preds = %if.end.i.i
  %cmp13.i.i = fcmp fast olt float %t1.i.i.0, 0.000000e+00
  br i1 %cmp13.i.i, label %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit.i", label %if.end.18.i.i

if.end.18.i.i:                                    ; preds = %if.then.12.i.i, %if.end.i.i
  %t0.i.i.1 = phi float [ %t1.i.i.0, %if.then.12.i.i ], [ %t0.i.i.0, %if.end.i.i ]
  %mul19.i.i.i0 = fmul fast float %t0.i.i.1, %FMad249
  %mul19.i.i.i1 = fmul fast float %t0.i.i.1, %FMad246
  %mul19.i.i.i2 = fmul fast float %t0.i.i.1, %FMad243
  %ObjectToWorld197 = call float @dx.op.objectToWorld.f32(i32 151, i32 0, i8 0)
  %ObjectToWorld198 = call float @dx.op.objectToWorld.f32(i32 151, i32 0, i8 1)
  %ObjectToWorld199 = call float @dx.op.objectToWorld.f32(i32 151, i32 0, i8 2)
  %ObjectToWorld201 = call float @dx.op.objectToWorld.f32(i32 151, i32 1, i8 0)
  %ObjectToWorld202 = call float @dx.op.objectToWorld.f32(i32 151, i32 1, i8 1)
  %ObjectToWorld203 = call float @dx.op.objectToWorld.f32(i32 151, i32 1, i8 2)
  %ObjectToWorld205 = call float @dx.op.objectToWorld.f32(i32 151, i32 2, i8 0)
  %ObjectToWorld206 = call float @dx.op.objectToWorld.f32(i32 151, i32 2, i8 1)
  %ObjectToWorld207 = call float @dx.op.objectToWorld.f32(i32 151, i32 2, i8 2)
  %sub21.i.i.i0 = fadd fast float %sub.i.i.35.i0, %mul19.i.i.i0
  %sub21.i.i.i1 = fadd fast float %sub.i.i.35.i1, %mul19.i.i.i1
  %sub21.i.i.i2 = fadd fast float %sub.i.i.35.i2, %mul19.i.i.i2
  %71 = fmul fast float %sub21.i.i.i0, %sub21.i.i.i0
  %72 = fmul fast float %sub21.i.i.i1, %sub21.i.i.i1
  %73 = fadd fast float %71, %72
  %74 = fmul fast float %sub21.i.i.i2, %sub21.i.i.i2
  %75 = fadd fast float %73, %74
  %Sqrt164 = call float @dx.op.unary.f32(i32 24, float %75)
  %.i0305 = fdiv fast float %sub21.i.i.i0, %Sqrt164
  %.i1306 = fdiv fast float %sub21.i.i.i1, %Sqrt164
  %.i2307 = fdiv fast float %sub21.i.i.i2, %Sqrt164
  %76 = fmul fast float %.i0305, %ObjectToWorld197
  %FMad239 = call float @dx.op.tertiary.f32(i32 46, float %.i1306, float %ObjectToWorld201, float %76)
  %FMad238 = call float @dx.op.tertiary.f32(i32 46, float %.i2307, float %ObjectToWorld205, float %FMad239)
  %77 = fmul fast float %.i0305, %ObjectToWorld198
  %FMad237 = call float @dx.op.tertiary.f32(i32 46, float %.i1306, float %ObjectToWorld202, float %77)
  %FMad236 = call float @dx.op.tertiary.f32(i32 46, float %.i2307, float %ObjectToWorld206, float %FMad237)
  %78 = fmul fast float %.i0305, %ObjectToWorld199
  %FMad235 = call float @dx.op.tertiary.f32(i32 46, float %.i1306, float %ObjectToWorld203, float %78)
  %FMad234 = call float @dx.op.tertiary.f32(i32 46, float %.i2307, float %ObjectToWorld207, float %FMad235)
  br label %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit.i"

"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit.i": ; preds = %if.end.18.i.i, %if.then.12.i.i, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.i"
  %retval.i.i.28.0 = phi i1 [ true, %if.end.18.i.i ], [ false, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.i" ], [ false, %if.then.12.i.i ]
  %.0107.0.i0 = phi float [ %FMad238, %if.end.18.i.i ], [ undef, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.i" ], [ undef, %if.then.12.i.i ]
  %.0107.0.i1 = phi float [ %FMad236, %if.end.18.i.i ], [ undef, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.i" ], [ undef, %if.then.12.i.i ]
  %.0107.0.i2 = phi float [ %FMad234, %if.end.18.i.i ], [ undef, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.i" ], [ undef, %if.then.12.i.i ]
  %.0127 = phi float [ %t0.i.i.1, %if.end.18.i.i ], [ undef, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.i" ], [ undef, %if.then.12.i.i ]
  %cmp.i.41 = fcmp fast olt float %.0127, %65
  %or.cond = and i1 %retval.i.i.28.0, %cmp.i.41
  %.0109.0.i0 = select i1 %or.cond, float %.0107.0.i0, float %.i0308
  %.0109.0.i1 = select i1 %or.cond, float %.0107.0.i1, float %.i1309
  %.0109.0.i2 = select i1 %or.cond, float %.0107.0.i2, float %.i2310
  %.0114 = select i1 %or.cond, float %.0127, float %65
  %sub.i.52.i.i2 = fadd fast float %FMad271, 0xBFD99999A0000000
  %79 = call float @dx.op.dot3.f32(i32 55, float %FMad249, float %FMad246, float %FMad243, float %FMad277, float %FMad274, float %sub.i.52.i.i2)
  %mul.i.53.i = fmul fast float %79, 2.000000e+00
  %80 = call float @dx.op.dot3.f32(i32 55, float %FMad277, float %FMad274, float %sub.i.52.i.i2, float %FMad277, float %FMad274, float %sub.i.52.i.i2)
  %sub4.i.54.i = fadd fast float %80, 0xBFB70A3D80000000
  %mul.i.i.55.i = fmul fast float %mul.i.53.i, %mul.i.53.i
  %mul2.i.i.57.i = fmul fast float %mul1.i.i.i, %sub4.i.54.i
  %sub.i.i.58.i = fsub fast float %mul.i.i.55.i, %mul2.i.i.57.i
  %cmp.i.i.59.i = fcmp fast olt float %sub.i.i.58.i, 0.000000e+00
  br i1 %cmp.i.i.59.i, label %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.81.i", label %if.else.i.i.62.i

if.else.i.i.62.i:                                 ; preds = %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit.i"
  %cmp4.i.i.61.i = fcmp fast oeq float %sub.i.i.58.i, 0.000000e+00
  br i1 %cmp4.i.i.61.i, label %if.then.7.i.i.65.i, label %if.else.9.i.i.67.i

if.then.7.i.i.65.i:                               ; preds = %if.else.i.i.62.i
  %mul8.i.i.63.i = fsub fast float -0.000000e+00, %79
  %div.i.i.64.i = fdiv fast float %mul8.i.i.63.i, %68
  br label %if.end.19.i.i.78.i

if.else.9.i.i.67.i:                               ; preds = %if.else.i.i.62.i
  %cmp10.i.i.66.i = fcmp fast ogt float %mul.i.53.i, 0.000000e+00
  br i1 %cmp10.i.i.66.i, label %cond.true.i.i.69.i, label %cond.false.i.i.71.i

cond.true.i.i.69.i:                               ; preds = %if.else.9.i.i.67.i
  %Sqrt144 = call float @dx.op.unary.f32(i32 24, float %sub.i.i.58.i)
  %add.i.i.68.i = fadd fast float %Sqrt144, %mul.i.53.i
  br label %cond.end.i.i.76.i

cond.false.i.i.71.i:                              ; preds = %if.else.9.i.i.67.i
  %Sqrt147 = call float @dx.op.unary.f32(i32 24, float %sub.i.i.58.i)
  %sub15.i.i.70.i = fsub fast float %mul.i.53.i, %Sqrt147
  br label %cond.end.i.i.76.i

cond.end.i.i.76.i:                                ; preds = %cond.false.i.i.71.i, %cond.true.i.i.69.i
  %add.sink.i.i.72.i = phi float [ %add.i.i.68.i, %cond.true.i.i.69.i ], [ %sub15.i.i.70.i, %cond.false.i.i.71.i ]
  %mul13.i.i.73.i = fmul fast float %add.sink.i.i.72.i, -5.000000e-01
  %div17.i.i.74.i = fdiv fast float %mul13.i.i.73.i, %68
  %div18.i.i.75.i = fdiv fast float %sub4.i.54.i, %mul13.i.i.73.i
  br label %if.end.19.i.i.78.i

if.end.19.i.i.78.i:                               ; preds = %cond.end.i.i.76.i, %if.then.7.i.i.65.i
  %.0139 = phi float [ %div.i.i.64.i, %if.then.7.i.i.65.i ], [ %div17.i.i.74.i, %cond.end.i.i.76.i ]
  %.0136 = phi float [ %div.i.i.64.i, %if.then.7.i.i.65.i ], [ %div18.i.i.75.i, %cond.end.i.i.76.i ]
  %cmp20.i.i.77.i = fcmp fast ogt float %.0139, %.0136
  %.1140 = select i1 %cmp20.i.i.77.i, float %.0136, float %.0139
  %.1137 = select i1 %cmp20.i.i.77.i, float %.0139, float %.0136
  br label %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.81.i"

"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.81.i": ; preds = %if.end.19.i.i.78.i, %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit.i"
  %.2141 = phi float [ undef, %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit.i" ], [ %.1140, %if.end.19.i.i.78.i ]
  %.2138 = phi float [ undef, %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit.i" ], [ %.1137, %if.end.19.i.i.78.i ]
  %retval.i.i.32.i.0 = phi i1 [ false, %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit.i" ], [ true, %if.end.19.i.i.78.i ]
  br i1 %retval.i.i.32.i.0, label %if.end.i.84.i, label %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit98.i"

if.end.i.84.i:                                    ; preds = %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.81.i"
  %cmp.i.83.i = fcmp fast ogt float %.2141, %.2138
  %t0.i.43.i.0 = select i1 %cmp.i.83.i, float %.2138, float %.2141
  %t1.i.44.i.0 = select i1 %cmp.i.83.i, float %.2141, float %.2138
  %cmp9.i.86.i = fcmp fast olt float %t0.i.43.i.0, 0.000000e+00
  br i1 %cmp9.i.86.i, label %if.then.12.i.89.i, label %if.end.18.i.97.i

if.then.12.i.89.i:                                ; preds = %if.end.i.84.i
  %cmp13.i.88.i = fcmp fast olt float %t1.i.44.i.0, 0.000000e+00
  br i1 %cmp13.i.88.i, label %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit98.i", label %if.end.18.i.97.i

if.end.18.i.97.i:                                 ; preds = %if.then.12.i.89.i, %if.end.i.84.i
  %t0.i.43.i.1 = phi float [ %t1.i.44.i.0, %if.then.12.i.89.i ], [ %t0.i.43.i.0, %if.end.i.84.i ]
  %mul19.i.93.i.i0 = fmul fast float %t0.i.43.i.1, %FMad249
  %mul19.i.93.i.i1 = fmul fast float %t0.i.43.i.1, %FMad246
  %mul19.i.93.i.i2 = fmul fast float %t0.i.43.i.1, %FMad243
  %ObjectToWorld185 = call float @dx.op.objectToWorld.f32(i32 151, i32 0, i8 0)
  %ObjectToWorld186 = call float @dx.op.objectToWorld.f32(i32 151, i32 0, i8 1)
  %ObjectToWorld187 = call float @dx.op.objectToWorld.f32(i32 151, i32 0, i8 2)
  %ObjectToWorld189 = call float @dx.op.objectToWorld.f32(i32 151, i32 1, i8 0)
  %ObjectToWorld190 = call float @dx.op.objectToWorld.f32(i32 151, i32 1, i8 1)
  %ObjectToWorld191 = call float @dx.op.objectToWorld.f32(i32 151, i32 1, i8 2)
  %ObjectToWorld193 = call float @dx.op.objectToWorld.f32(i32 151, i32 2, i8 0)
  %ObjectToWorld194 = call float @dx.op.objectToWorld.f32(i32 151, i32 2, i8 1)
  %ObjectToWorld195 = call float @dx.op.objectToWorld.f32(i32 151, i32 2, i8 2)
  %sub21.i.95.i.i0 = fadd fast float %FMad277, %mul19.i.93.i.i0
  %sub21.i.95.i.i1 = fadd fast float %FMad274, %mul19.i.93.i.i1
  %sub21.i.95.i.i2 = fadd fast float %sub.i.52.i.i2, %mul19.i.93.i.i2
  %81 = fmul fast float %sub21.i.95.i.i0, %sub21.i.95.i.i0
  %82 = fmul fast float %sub21.i.95.i.i1, %sub21.i.95.i.i1
  %83 = fadd fast float %81, %82
  %84 = fmul fast float %sub21.i.95.i.i2, %sub21.i.95.i.i2
  %85 = fadd fast float %83, %84
  %Sqrt165 = call float @dx.op.unary.f32(i32 24, float %85)
  %.i0311 = fdiv fast float %sub21.i.95.i.i0, %Sqrt165
  %.i1312 = fdiv fast float %sub21.i.95.i.i1, %Sqrt165
  %.i2313 = fdiv fast float %sub21.i.95.i.i2, %Sqrt165
  %86 = fmul fast float %.i0311, %ObjectToWorld185
  %FMad295 = call float @dx.op.tertiary.f32(i32 46, float %.i1312, float %ObjectToWorld189, float %86)
  %FMad294 = call float @dx.op.tertiary.f32(i32 46, float %.i2313, float %ObjectToWorld193, float %FMad295)
  %87 = fmul fast float %.i0311, %ObjectToWorld186
  %FMad293 = call float @dx.op.tertiary.f32(i32 46, float %.i1312, float %ObjectToWorld190, float %87)
  %FMad292 = call float @dx.op.tertiary.f32(i32 46, float %.i2313, float %ObjectToWorld194, float %FMad293)
  %88 = fmul fast float %.i0311, %ObjectToWorld187
  %FMad291 = call float @dx.op.tertiary.f32(i32 46, float %.i1312, float %ObjectToWorld191, float %88)
  %FMad290 = call float @dx.op.tertiary.f32(i32 46, float %.i2313, float %ObjectToWorld195, float %FMad291)
  br label %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit98.i"

"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit98.i": ; preds = %if.end.18.i.97.i, %if.then.12.i.89.i, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.81.i"
  %retval.i.38.i.0 = phi i1 [ true, %if.end.18.i.97.i ], [ false, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.81.i" ], [ false, %if.then.12.i.89.i ]
  %.0.0.i0 = phi float [ %FMad294, %if.end.18.i.97.i ], [ %.0107.0.i0, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.81.i" ], [ %.0107.0.i0, %if.then.12.i.89.i ]
  %.0.0.i1 = phi float [ %FMad292, %if.end.18.i.97.i ], [ %.0107.0.i1, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.81.i" ], [ %.0107.0.i1, %if.then.12.i.89.i ]
  %.0.0.i2 = phi float [ %FMad290, %if.end.18.i.97.i ], [ %.0107.0.i2, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.81.i" ], [ %.0107.0.i2, %if.then.12.i.89.i ]
  %.0128 = phi float [ %t0.i.43.i.1, %if.end.18.i.97.i ], [ %.0127, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.81.i" ], [ %.0127, %if.then.12.i.89.i ]
  %cmp13.i.45 = fcmp fast olt float %.0128, %.0114
  %or.cond142 = and i1 %retval.i.38.i.0, %cmp13.i.45
  %.0109.1.i0 = select i1 %or.cond142, float %.0.0.i0, float %.0109.0.i0
  %.0109.1.i1 = select i1 %or.cond142, float %.0.0.i1, float %.0109.0.i1
  %.0109.1.i2 = select i1 %or.cond142, float %.0.0.i2, float %.0109.0.i2
  %.1 = select i1 %or.cond142, float %.0128, float %.0114
  %sub.i.121.i.i0 = fadd fast float %FMad277, 0xBFD3333340000000
  %sub.i.121.i.i1 = fadd fast float %FMad274, 0xBFD3333340000000
  %89 = call float @dx.op.dot3.f32(i32 55, float %FMad249, float %FMad246, float %FMad243, float %sub.i.121.i.i0, float %sub.i.121.i.i1, float %FMad271)
  %mul.i.122.i = fmul fast float %89, 2.000000e+00
  %90 = call float @dx.op.dot3.f32(i32 55, float %sub.i.121.i.i0, float %sub.i.121.i.i1, float %FMad271, float %sub.i.121.i.i0, float %sub.i.121.i.i1, float %FMad271)
  %sub4.i.123.i = fadd fast float %90, 0xBF970A3DC0000000
  %mul.i.i.124.i = fmul fast float %mul.i.122.i, %mul.i.122.i
  %mul2.i.i.126.i = fmul fast float %mul1.i.i.i, %sub4.i.123.i
  %sub.i.i.127.i = fsub fast float %mul.i.i.124.i, %mul2.i.i.126.i
  %cmp.i.i.128.i = fcmp fast olt float %sub.i.i.127.i, 0.000000e+00
  br i1 %cmp.i.i.128.i, label %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.150.i", label %if.else.i.i.131.i

if.else.i.i.131.i:                                ; preds = %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit98.i"
  %cmp4.i.i.130.i = fcmp fast oeq float %sub.i.i.127.i, 0.000000e+00
  br i1 %cmp4.i.i.130.i, label %if.then.7.i.i.134.i, label %if.else.9.i.i.136.i

if.then.7.i.i.134.i:                              ; preds = %if.else.i.i.131.i
  %mul8.i.i.132.i = fsub fast float -0.000000e+00, %89
  %div.i.i.133.i = fdiv fast float %mul8.i.i.132.i, %68
  br label %if.end.19.i.i.147.i

if.else.9.i.i.136.i:                              ; preds = %if.else.i.i.131.i
  %cmp10.i.i.135.i = fcmp fast ogt float %mul.i.122.i, 0.000000e+00
  br i1 %cmp10.i.i.135.i, label %cond.true.i.i.138.i, label %cond.false.i.i.140.i

cond.true.i.i.138.i:                              ; preds = %if.else.9.i.i.136.i
  %Sqrt145 = call float @dx.op.unary.f32(i32 24, float %sub.i.i.127.i)
  %add.i.i.137.i = fadd fast float %Sqrt145, %mul.i.122.i
  br label %cond.end.i.i.145.i

cond.false.i.i.140.i:                             ; preds = %if.else.9.i.i.136.i
  %Sqrt146 = call float @dx.op.unary.f32(i32 24, float %sub.i.i.127.i)
  %sub15.i.i.139.i = fsub fast float %mul.i.122.i, %Sqrt146
  br label %cond.end.i.i.145.i

cond.end.i.i.145.i:                               ; preds = %cond.false.i.i.140.i, %cond.true.i.i.138.i
  %add.sink.i.i.141.i = phi float [ %add.i.i.137.i, %cond.true.i.i.138.i ], [ %sub15.i.i.139.i, %cond.false.i.i.140.i ]
  %mul13.i.i.142.i = fmul fast float %add.sink.i.i.141.i, -5.000000e-01
  %div17.i.i.143.i = fdiv fast float %mul13.i.i.142.i, %68
  %div18.i.i.144.i = fdiv fast float %sub4.i.123.i, %mul13.i.i.142.i
  br label %if.end.19.i.i.147.i

if.end.19.i.i.147.i:                              ; preds = %cond.end.i.i.145.i, %if.then.7.i.i.134.i
  %.0118 = phi float [ %div.i.i.133.i, %if.then.7.i.i.134.i ], [ %div17.i.i.143.i, %cond.end.i.i.145.i ]
  %.0115 = phi float [ %div.i.i.133.i, %if.then.7.i.i.134.i ], [ %div18.i.i.144.i, %cond.end.i.i.145.i ]
  %cmp20.i.i.146.i = fcmp fast ogt float %.0118, %.0115
  %.1119 = select i1 %cmp20.i.i.146.i, float %.0115, float %.0118
  %.1116 = select i1 %cmp20.i.i.146.i, float %.0118, float %.0115
  br label %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.150.i"

"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.150.i": ; preds = %if.end.19.i.i.147.i, %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit98.i"
  %.2120 = phi float [ undef, %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit98.i" ], [ %.1119, %if.end.19.i.i.147.i ]
  %.2117 = phi float [ undef, %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit98.i" ], [ %.1116, %if.end.19.i.i.147.i ]
  %retval.i.i.101.i.0 = phi i1 [ false, %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit98.i" ], [ true, %if.end.19.i.i.147.i ]
  br i1 %retval.i.i.101.i.0, label %if.end.i.153.i, label %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit167.i"

if.end.i.153.i:                                   ; preds = %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.150.i"
  %cmp.i.152.i = fcmp fast ogt float %.2120, %.2117
  %t1.i.113.i.0 = select i1 %cmp.i.152.i, float %.2120, float %.2117
  %t0.i.112.i.0 = select i1 %cmp.i.152.i, float %.2117, float %.2120
  %cmp9.i.155.i = fcmp fast olt float %t0.i.112.i.0, 0.000000e+00
  br i1 %cmp9.i.155.i, label %if.then.12.i.158.i, label %if.end.18.i.166.i

if.then.12.i.158.i:                               ; preds = %if.end.i.153.i
  %cmp13.i.157.i = fcmp fast olt float %t1.i.113.i.0, 0.000000e+00
  br i1 %cmp13.i.157.i, label %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit167.i", label %if.end.18.i.166.i

if.end.18.i.166.i:                                ; preds = %if.then.12.i.158.i, %if.end.i.153.i
  %t0.i.112.i.1 = phi float [ %t1.i.113.i.0, %if.then.12.i.158.i ], [ %t0.i.112.i.0, %if.end.i.153.i ]
  %mul19.i.162.i.i0 = fmul fast float %t0.i.112.i.1, %FMad249
  %mul19.i.162.i.i1 = fmul fast float %t0.i.112.i.1, %FMad246
  %mul19.i.162.i.i2 = fmul fast float %t0.i.112.i.1, %FMad243
  %ObjectToWorld = call float @dx.op.objectToWorld.f32(i32 151, i32 0, i8 0)
  %ObjectToWorld174 = call float @dx.op.objectToWorld.f32(i32 151, i32 0, i8 1)
  %ObjectToWorld175 = call float @dx.op.objectToWorld.f32(i32 151, i32 0, i8 2)
  %ObjectToWorld177 = call float @dx.op.objectToWorld.f32(i32 151, i32 1, i8 0)
  %ObjectToWorld178 = call float @dx.op.objectToWorld.f32(i32 151, i32 1, i8 1)
  %ObjectToWorld179 = call float @dx.op.objectToWorld.f32(i32 151, i32 1, i8 2)
  %ObjectToWorld181 = call float @dx.op.objectToWorld.f32(i32 151, i32 2, i8 0)
  %ObjectToWorld182 = call float @dx.op.objectToWorld.f32(i32 151, i32 2, i8 1)
  %ObjectToWorld183 = call float @dx.op.objectToWorld.f32(i32 151, i32 2, i8 2)
  %sub21.i.164.i.i0 = fadd fast float %sub.i.121.i.i0, %mul19.i.162.i.i0
  %sub21.i.164.i.i1 = fadd fast float %sub.i.121.i.i1, %mul19.i.162.i.i1
  %sub21.i.164.i.i2 = fadd fast float %FMad271, %mul19.i.162.i.i2
  %91 = fmul fast float %sub21.i.164.i.i0, %sub21.i.164.i.i0
  %92 = fmul fast float %sub21.i.164.i.i1, %sub21.i.164.i.i1
  %93 = fadd fast float %91, %92
  %94 = fmul fast float %sub21.i.164.i.i2, %sub21.i.164.i.i2
  %95 = fadd fast float %93, %94
  %Sqrt166 = call float @dx.op.unary.f32(i32 24, float %95)
  %.i0314 = fdiv fast float %sub21.i.164.i.i0, %Sqrt166
  %.i1315 = fdiv fast float %sub21.i.164.i.i1, %Sqrt166
  %.i2316 = fdiv fast float %sub21.i.164.i.i2, %Sqrt166
  %96 = fmul fast float %.i0314, %ObjectToWorld
  %FMad287 = call float @dx.op.tertiary.f32(i32 46, float %.i1315, float %ObjectToWorld177, float %96)
  %FMad286 = call float @dx.op.tertiary.f32(i32 46, float %.i2316, float %ObjectToWorld181, float %FMad287)
  %97 = fmul fast float %.i0314, %ObjectToWorld174
  %FMad285 = call float @dx.op.tertiary.f32(i32 46, float %.i1315, float %ObjectToWorld178, float %97)
  %FMad284 = call float @dx.op.tertiary.f32(i32 46, float %.i2316, float %ObjectToWorld182, float %FMad285)
  %98 = fmul fast float %.i0314, %ObjectToWorld175
  %FMad283 = call float @dx.op.tertiary.f32(i32 46, float %.i1315, float %ObjectToWorld179, float %98)
  %FMad282 = call float @dx.op.tertiary.f32(i32 46, float %.i2316, float %ObjectToWorld183, float %FMad283)
  br label %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit167.i"

"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit167.i": ; preds = %if.end.18.i.166.i, %if.then.12.i.158.i, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.150.i"
  %.0108.0.i0 = phi float [ %FMad286, %if.end.18.i.166.i ], [ %.0.0.i0, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.150.i" ], [ %.0.0.i0, %if.then.12.i.158.i ]
  %.0108.0.i1 = phi float [ %FMad284, %if.end.18.i.166.i ], [ %.0.0.i1, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.150.i" ], [ %.0.0.i1, %if.then.12.i.158.i ]
  %.0108.0.i2 = phi float [ %FMad282, %if.end.18.i.166.i ], [ %.0.0.i2, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.150.i" ], [ %.0.0.i2, %if.then.12.i.158.i ]
  %.0129 = phi float [ %t0.i.112.i.1, %if.end.18.i.166.i ], [ %.0128, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.150.i" ], [ %.0128, %if.then.12.i.158.i ]
  %retval.i.107.i.0 = phi i1 [ true, %if.end.18.i.166.i ], [ false, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.150.i" ], [ false, %if.then.12.i.158.i ]
  %cmp23.i = fcmp fast olt float %.0129, %.1
  %or.cond143 = and i1 %retval.i.107.i.0, %cmp23.i
  %99 = or i1 %or.cond142, %or.cond143
  %.0109.2.i0 = select i1 %or.cond143, float %.0108.0.i0, float %.0109.1.i0
  %.0109.2.i1 = select i1 %or.cond143, float %.0108.0.i1, float %.0109.1.i1
  %.0109.2.i2 = select i1 %or.cond143, float %.0108.0.i2, float %.0109.1.i2
  %.0109.2.upto0 = insertelement <3 x float> undef, float %.0109.2.i0, i32 0
  %.0109.2.upto1 = insertelement <3 x float> %.0109.2.upto0, float %.0109.2.i1, i32 1
  %.0109.2 = insertelement <3 x float> %.0109.2.upto1, float %.0109.2.i2, i32 2
  %.2 = select i1 %or.cond143, float %.0129, float %.1
  %tobool29.i = or i1 %or.cond, %99
  store float %.2, float* %4, align 4, !tbaa !228
  store <3 x float> %.0109.2, <3 x float>* %66, align 4
  br label %return

return:                                           ; preds = %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit167.i", %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit", %"\01?intersectBox@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@@Z.exit", %entry
  %retval.0 = phi i1 [ %tobool29.i, %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit167.i" ], [ %retval.i.0, %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit" ], [ %cmp63.i, %"\01?intersectBox@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@@Z.exit" ], [ false, %entry ]
  ret i1 %retval.0
}

; Function Attrs: alwaysinline nounwind
define void @"\01?RawDataToRaytracingInstanceDesc@@YA?AURaytracingInstanceDesc@@V?$vector@I$03@@000@Z"(%struct.RaytracingInstanceDesc* noalias nocapture sret, <4 x i32>, <4 x i32>, <4 x i32>, <4 x i32>) #5 {
entry:
  %c.i0 = extractelement <4 x i32> %3, i32 0
  %c.i1 = extractelement <4 x i32> %3, i32 1
  %c.i2 = extractelement <4 x i32> %3, i32 2
  %c.i3 = extractelement <4 x i32> %3, i32 3
  %b.i0 = extractelement <4 x i32> %2, i32 0
  %b.i1 = extractelement <4 x i32> %2, i32 1
  %b.i2 = extractelement <4 x i32> %2, i32 2
  %b.i3 = extractelement <4 x i32> %2, i32 3
  %a.i0 = extractelement <4 x i32> %1, i32 0
  %a.i1 = extractelement <4 x i32> %1, i32 1
  %a.i2 = extractelement <4 x i32> %1, i32 2
  %a.i3 = extractelement <4 x i32> %1, i32 3
  %.i0 = bitcast i32 %a.i0 to float
  %.i1 = bitcast i32 %a.i1 to float
  %.i2 = bitcast i32 %a.i2 to float
  %.i3 = bitcast i32 %a.i3 to float
  %.upto0 = insertelement <4 x float> undef, float %.i0, i32 0
  %.upto1 = insertelement <4 x float> %.upto0, float %.i1, i32 1
  %.upto2 = insertelement <4 x float> %.upto1, float %.i2, i32 2
  %5 = insertelement <4 x float> %.upto2, float %.i3, i32 3
  %arrayidx = getelementptr inbounds %struct.RaytracingInstanceDesc, %struct.RaytracingInstanceDesc* %0, i32 0, i32 0, i32 0
  store <4 x float> %5, <4 x float>* %arrayidx, align 4, !tbaa !220
  %.i07 = bitcast i32 %b.i0 to float
  %.i18 = bitcast i32 %b.i1 to float
  %.i29 = bitcast i32 %b.i2 to float
  %.i310 = bitcast i32 %b.i3 to float
  %.upto015 = insertelement <4 x float> undef, float %.i07, i32 0
  %.upto116 = insertelement <4 x float> %.upto015, float %.i18, i32 1
  %.upto217 = insertelement <4 x float> %.upto116, float %.i29, i32 2
  %6 = insertelement <4 x float> %.upto217, float %.i310, i32 3
  %arrayidx3 = getelementptr inbounds %struct.RaytracingInstanceDesc, %struct.RaytracingInstanceDesc* %0, i32 0, i32 0, i32 1
  store <4 x float> %6, <4 x float>* %arrayidx3, align 4, !tbaa !220
  %.i011 = bitcast i32 %c.i0 to float
  %.i112 = bitcast i32 %c.i1 to float
  %.i213 = bitcast i32 %c.i2 to float
  %.i314 = bitcast i32 %c.i3 to float
  %.upto018 = insertelement <4 x float> undef, float %.i011, i32 0
  %.upto119 = insertelement <4 x float> %.upto018, float %.i112, i32 1
  %.upto220 = insertelement <4 x float> %.upto119, float %.i213, i32 2
  %7 = insertelement <4 x float> %.upto220, float %.i314, i32 3
  %arrayidx6 = getelementptr inbounds %struct.RaytracingInstanceDesc, %struct.RaytracingInstanceDesc* %0, i32 0, i32 0, i32 2
  store <4 x float> %7, <4 x float>* %arrayidx6, align 4, !tbaa !220
  %8 = extractelement <4 x i32> %4, i32 0
  %InstanceIDAndMask = getelementptr inbounds %struct.RaytracingInstanceDesc, %struct.RaytracingInstanceDesc* %0, i32 0, i32 1
  store i32 %8, i32* %InstanceIDAndMask, align 4, !tbaa !223
  %9 = extractelement <4 x i32> %4, i32 1
  %InstanceContributionToHitGroupIndexAndFlags = getelementptr inbounds %struct.RaytracingInstanceDesc, %struct.RaytracingInstanceDesc* %0, i32 0, i32 2
  store i32 %9, i32* %InstanceContributionToHitGroupIndexAndFlags, align 4, !tbaa !223
  %.upto03 = insertelement <2 x i32> undef, i32 %12, i32 0
  %10 = insertelement <2 x i32> %.upto03, i32 %11, i32 1
  %11 = extractelement <4 x i32> %4, i32 3
  %12 = extractelement <4 x i32> %4, i32 2
  %AccelerationStructure = getelementptr inbounds %struct.RaytracingInstanceDesc, %struct.RaytracingInstanceDesc* %0, i32 0, i32 3
  store <2 x i32> %10, <2 x i32>* %AccelerationStructure, align 4, !tbaa !220
  ret void
}

; Function Attrs: alwaysinline nounwind readnone
define <2 x i32> @"\01?CreateFlag@@YA?AV?$vector@I$01@@II@Z"(i32, i32) #4 {
entry:
  %and = and i32 %0, 16777215
  %2 = insertelement <2 x i32> undef, i32 %and, i32 0
  %3 = insertelement <2 x i32> %2, i32 %1, i32 1
  ret <2 x i32> %3
}

; Function Attrs: alwaysinline nounwind
define void @"\01?MyClosestHitShader_ShadowAABB@@YAXUShadowPayload@@UProceduralPrimitiveAttributes@@@Z"(%struct.ShadowPayload* noalias nocapture, %struct.ProceduralPrimitiveAttributes* nocapture readnone) #5 {
entry:
  %2 = getelementptr inbounds %struct.ShadowPayload, %struct.ShadowPayload* %0, i32 0, i32 0
  store i32 1, i32* %2, align 4
  ret void
}

; Function Attrs: alwaysinline nounwind
define void @"\01?StackPush@@YAXAIAHIII@Z"(i32* noalias nocapture dereferenceable(4), i32, i32, i32) #5 {
entry:
  %4 = load i32, i32* %0, align 4, !tbaa !223
  %mul = shl i32 %4, 6
  %add = add i32 %mul, %3
  %arrayidx = getelementptr [2048 x i32], [2048 x i32] addrspace(3)* @"\01?stack@@3PAIA", i32 0, i32 %add
  store i32 %1, i32 addrspace(3)* %arrayidx, align 4, !tbaa !223
  %inc = add nsw i32 %4, 1
  store i32 %inc, i32* %0, align 4, !tbaa !223
  ret void
}

; Function Attrs: alwaysinline nounwind
define void @"\01?dump@@YAXUBoundingBox@@V?$vector@I$01@@@Z"(%struct.BoundingBox* nocapture readonly, <2 x i32>) #5 {
entry:
  ret void
}

; Function Attrs: alwaysinline nounwind
define i1 @"\01?intersectSpheres@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@@Z"(<3 x float>, <3 x float>, float* noalias nocapture dereferenceable(4), %struct.ProceduralPrimitiveAttributes* noalias nocapture) #5 {
entry:
  %direction.i0 = extractelement <3 x float> %1, i32 0
  %direction.i1 = extractelement <3 x float> %1, i32 1
  %direction.i2 = extractelement <3 x float> %1, i32 2
  %origin.i0 = extractelement <3 x float> %0, i32 0
  %origin.i1 = extractelement <3 x float> %0, i32 1
  %origin.i2 = extractelement <3 x float> %0, i32 2
  %4 = getelementptr inbounds %struct.ProceduralPrimitiveAttributes, %struct.ProceduralPrimitiveAttributes* %3, i32 0, i32 0
  %5 = load <3 x float>, <3 x float>* %4, align 4
  %.i0282 = extractelement <3 x float> %5, i32 0
  %.i1283 = extractelement <3 x float> %5, i32 1
  %.i2284 = extractelement <3 x float> %5, i32 2
  %sub.i.i0 = fadd fast float %origin.i0, 0x3FD3333340000000
  %sub.i.i1 = fadd fast float %origin.i1, 0x3FD3333340000000
  %sub.i.i2 = fadd fast float %origin.i2, 0x3FD3333340000000
  %6 = extractelement <3 x float> %1, i64 0
  %7 = extractelement <3 x float> %1, i64 1
  %8 = extractelement <3 x float> %1, i64 2
  %9 = call float @dx.op.dot3.f32(i32 55, float %6, float %7, float %8, float %6, float %7, float %8)
  %10 = call float @dx.op.dot3.f32(i32 55, float %6, float %7, float %8, float %sub.i.i0, float %sub.i.i1, float %sub.i.i2)
  %mul.i = fmul fast float %10, 2.000000e+00
  %11 = call float @dx.op.dot3.f32(i32 55, float %sub.i.i0, float %sub.i.i1, float %sub.i.i2, float %sub.i.i0, float %sub.i.i1, float %sub.i.i2)
  %sub4.i = fadd fast float %11, 0xBFD70A3D80000000
  %mul.i.i = fmul fast float %mul.i, %mul.i
  %mul1.i.i = fmul fast float %9, 4.000000e+00
  %mul2.i.i = fmul fast float %mul1.i.i, %sub4.i
  %sub.i.i = fsub fast float %mul.i.i, %mul2.i.i
  %cmp.i.i = fcmp fast olt float %sub.i.i, 0.000000e+00
  br i1 %cmp.i.i, label %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i", label %if.else.i.i

if.else.i.i:                                      ; preds = %entry
  %cmp4.i.i = fcmp fast oeq float %sub.i.i, 0.000000e+00
  br i1 %cmp4.i.i, label %if.then.7.i.i, label %if.else.9.i.i

if.then.7.i.i:                                    ; preds = %if.else.i.i
  %mul8.i.i = fsub fast float -0.000000e+00, %10
  %div.i.i = fdiv fast float %mul8.i.i, %9
  br label %if.end.19.i.i

if.else.9.i.i:                                    ; preds = %if.else.i.i
  %cmp10.i.i = fcmp fast ogt float %mul.i, 0.000000e+00
  br i1 %cmp10.i.i, label %cond.true.i.i, label %cond.false.i.i

cond.true.i.i:                                    ; preds = %if.else.9.i.i
  %Sqrt215 = call float @dx.op.unary.f32(i32 24, float %sub.i.i)
  %add.i.i = fadd fast float %Sqrt215, %mul.i
  br label %cond.end.i.i

cond.false.i.i:                                   ; preds = %if.else.9.i.i
  %Sqrt216 = call float @dx.op.unary.f32(i32 24, float %sub.i.i)
  %sub15.i.i = fsub fast float %mul.i, %Sqrt216
  br label %cond.end.i.i

cond.end.i.i:                                     ; preds = %cond.false.i.i, %cond.true.i.i
  %add.sink.i.i = phi float [ %add.i.i, %cond.true.i.i ], [ %sub15.i.i, %cond.false.i.i ]
  %mul13.i.i = fmul fast float %add.sink.i.i, -5.000000e-01
  %div17.i.i = fdiv fast float %mul13.i.i, %9
  %div18.i.i = fdiv fast float %sub4.i, %mul13.i.i
  br label %if.end.19.i.i

if.end.19.i.i:                                    ; preds = %cond.end.i.i, %if.then.7.i.i
  %.0201 = phi float [ %div.i.i, %if.then.7.i.i ], [ %div18.i.i, %cond.end.i.i ]
  %.0198 = phi float [ %div.i.i, %if.then.7.i.i ], [ %div17.i.i, %cond.end.i.i ]
  %cmp20.i.i = fcmp fast ogt float %.0198, %.0201
  %.1202 = select i1 %cmp20.i.i, float %.0198, float %.0201
  %.1199 = select i1 %cmp20.i.i, float %.0201, float %.0198
  br label %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i"

"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i":      ; preds = %if.end.19.i.i, %entry
  %retval.i.i.0 = phi i1 [ false, %entry ], [ true, %if.end.19.i.i ]
  %.2203 = phi float [ undef, %entry ], [ %.1202, %if.end.19.i.i ]
  %.2200 = phi float [ undef, %entry ], [ %.1199, %if.end.19.i.i ]
  br i1 %retval.i.i.0, label %if.end.i, label %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit"

if.end.i:                                         ; preds = %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i"
  %cmp.i = fcmp fast ogt float %.2200, %.2203
  %t0.i.0 = select i1 %cmp.i, float %.2203, float %.2200
  %t1.i.0 = select i1 %cmp.i, float %.2200, float %.2203
  %cmp9.i = fcmp fast olt float %t0.i.0, 0.000000e+00
  br i1 %cmp9.i, label %if.then.12.i, label %if.end.18.i

if.then.12.i:                                     ; preds = %if.end.i
  %cmp13.i = fcmp fast olt float %t1.i.0, 0.000000e+00
  br i1 %cmp13.i, label %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit", label %if.end.18.i

if.end.18.i:                                      ; preds = %if.then.12.i, %if.end.i
  %t0.i.1 = phi float [ %t1.i.0, %if.then.12.i ], [ %t0.i.0, %if.end.i ]
  %mul19.i.i0 = fmul fast float %t0.i.1, %direction.i0
  %mul19.i.i1 = fmul fast float %t0.i.1, %direction.i1
  %mul19.i.i2 = fmul fast float %t0.i.1, %direction.i2
  %ObjectToWorld247 = call float @dx.op.objectToWorld.f32(i32 151, i32 0, i8 0)
  %ObjectToWorld248 = call float @dx.op.objectToWorld.f32(i32 151, i32 0, i8 1)
  %ObjectToWorld249 = call float @dx.op.objectToWorld.f32(i32 151, i32 0, i8 2)
  %ObjectToWorld251 = call float @dx.op.objectToWorld.f32(i32 151, i32 1, i8 0)
  %ObjectToWorld252 = call float @dx.op.objectToWorld.f32(i32 151, i32 1, i8 1)
  %ObjectToWorld253 = call float @dx.op.objectToWorld.f32(i32 151, i32 1, i8 2)
  %ObjectToWorld255 = call float @dx.op.objectToWorld.f32(i32 151, i32 2, i8 0)
  %ObjectToWorld256 = call float @dx.op.objectToWorld.f32(i32 151, i32 2, i8 1)
  %ObjectToWorld257 = call float @dx.op.objectToWorld.f32(i32 151, i32 2, i8 2)
  %sub21.i.i0 = fadd fast float %sub.i.i0, %mul19.i.i0
  %sub21.i.i1 = fadd fast float %sub.i.i1, %mul19.i.i1
  %sub21.i.i2 = fadd fast float %sub.i.i2, %mul19.i.i2
  %12 = fmul fast float %sub21.i.i0, %sub21.i.i0
  %13 = fmul fast float %sub21.i.i1, %sub21.i.i1
  %14 = fadd fast float %12, %13
  %15 = fmul fast float %sub21.i.i2, %sub21.i.i2
  %16 = fadd fast float %14, %15
  %Sqrt223 = call float @dx.op.unary.f32(i32 24, float %16)
  %.i0 = fdiv fast float %sub21.i.i0, %Sqrt223
  %.i1 = fdiv fast float %sub21.i.i1, %Sqrt223
  %.i2 = fdiv fast float %sub21.i.i2, %Sqrt223
  %17 = fmul fast float %.i0, %ObjectToWorld247
  %FMad281 = call float @dx.op.tertiary.f32(i32 46, float %.i1, float %ObjectToWorld251, float %17)
  %FMad280 = call float @dx.op.tertiary.f32(i32 46, float %.i2, float %ObjectToWorld255, float %FMad281)
  %18 = fmul fast float %.i0, %ObjectToWorld248
  %FMad279 = call float @dx.op.tertiary.f32(i32 46, float %.i1, float %ObjectToWorld252, float %18)
  %FMad278 = call float @dx.op.tertiary.f32(i32 46, float %.i2, float %ObjectToWorld256, float %FMad279)
  %19 = fmul fast float %.i0, %ObjectToWorld249
  %FMad277 = call float @dx.op.tertiary.f32(i32 46, float %.i1, float %ObjectToWorld253, float %19)
  %FMad276 = call float @dx.op.tertiary.f32(i32 46, float %.i2, float %ObjectToWorld257, float %FMad277)
  br label %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit"

"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit": ; preds = %if.end.18.i, %if.then.12.i, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i"
  %retval.i.0 = phi i1 [ true, %if.end.18.i ], [ false, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i" ], [ false, %if.then.12.i ]
  %.0190.0.i0 = phi float [ %FMad280, %if.end.18.i ], [ undef, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i" ], [ undef, %if.then.12.i ]
  %.0190.0.i1 = phi float [ %FMad278, %if.end.18.i ], [ undef, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i" ], [ undef, %if.then.12.i ]
  %.0190.0.i2 = phi float [ %FMad276, %if.end.18.i ], [ undef, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i" ], [ undef, %if.then.12.i ]
  %.0191 = phi float [ %t0.i.1, %if.end.18.i ], [ undef, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i" ], [ undef, %if.then.12.i ]
  %20 = load float, float* %2, align 4
  %cmp = fcmp fast olt float %.0191, %20
  %or.cond = and i1 %retval.i.0, %cmp
  br i1 %or.cond, label %if.then.7, label %if.end.8

if.then.7:                                        ; preds = %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit"
  store float %.0191, float* %2, align 4, !tbaa !228
  br label %if.end.8

if.end.8:                                         ; preds = %if.then.7, %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit"
  %21 = phi float [ %.0191, %if.then.7 ], [ %20, %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit" ]
  %hitFound.0 = phi i32 [ 1, %if.then.7 ], [ 0, %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit" ]
  %.0.0.i0 = phi float [ %.0190.0.i0, %if.then.7 ], [ %.i0282, %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit" ]
  %.0.0.i1 = phi float [ %.0190.0.i1, %if.then.7 ], [ %.i1283, %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit" ]
  %.0.0.i2 = phi float [ %.0190.0.i2, %if.then.7 ], [ %.i2284, %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit" ]
  %sub.i.52.i2 = fadd fast float %origin.i2, 0xBFD99999A0000000
  %22 = call float @dx.op.dot3.f32(i32 55, float %6, float %7, float %8, float %origin.i0, float %origin.i1, float %sub.i.52.i2)
  %mul.i.53 = fmul fast float %22, 2.000000e+00
  %23 = call float @dx.op.dot3.f32(i32 55, float %origin.i0, float %origin.i1, float %sub.i.52.i2, float %origin.i0, float %origin.i1, float %sub.i.52.i2)
  %sub4.i.54 = fadd fast float %23, 0xBFB70A3D80000000
  %mul.i.i.55 = fmul fast float %mul.i.53, %mul.i.53
  %mul2.i.i.57 = fmul fast float %mul1.i.i, %sub4.i.54
  %sub.i.i.58 = fsub fast float %mul.i.i.55, %mul2.i.i.57
  %cmp.i.i.59 = fcmp fast olt float %sub.i.i.58, 0.000000e+00
  br i1 %cmp.i.i.59, label %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.81", label %if.else.i.i.62

if.else.i.i.62:                                   ; preds = %if.end.8
  %cmp4.i.i.61 = fcmp fast oeq float %sub.i.i.58, 0.000000e+00
  br i1 %cmp4.i.i.61, label %if.then.7.i.i.65, label %if.else.9.i.i.67

if.then.7.i.i.65:                                 ; preds = %if.else.i.i.62
  %mul8.i.i.63 = fsub fast float -0.000000e+00, %22
  %div.i.i.64 = fdiv fast float %mul8.i.i.63, %9
  br label %if.end.19.i.i.78

if.else.9.i.i.67:                                 ; preds = %if.else.i.i.62
  %cmp10.i.i.66 = fcmp fast ogt float %mul.i.53, 0.000000e+00
  br i1 %cmp10.i.i.66, label %cond.true.i.i.69, label %cond.false.i.i.71

cond.true.i.i.69:                                 ; preds = %if.else.9.i.i.67
  %Sqrt213 = call float @dx.op.unary.f32(i32 24, float %sub.i.i.58)
  %add.i.i.68 = fadd fast float %Sqrt213, %mul.i.53
  br label %cond.end.i.i.76

cond.false.i.i.71:                                ; preds = %if.else.9.i.i.67
  %Sqrt214 = call float @dx.op.unary.f32(i32 24, float %sub.i.i.58)
  %sub15.i.i.70 = fsub fast float %mul.i.53, %Sqrt214
  br label %cond.end.i.i.76

cond.end.i.i.76:                                  ; preds = %cond.false.i.i.71, %cond.true.i.i.69
  %add.sink.i.i.72 = phi float [ %add.i.i.68, %cond.true.i.i.69 ], [ %sub15.i.i.70, %cond.false.i.i.71 ]
  %mul13.i.i.73 = fmul fast float %add.sink.i.i.72, -5.000000e-01
  %div17.i.i.74 = fdiv fast float %mul13.i.i.73, %9
  %div18.i.i.75 = fdiv fast float %sub4.i.54, %mul13.i.i.73
  br label %if.end.19.i.i.78

if.end.19.i.i.78:                                 ; preds = %cond.end.i.i.76, %if.then.7.i.i.65
  %.0207 = phi float [ %div.i.i.64, %if.then.7.i.i.65 ], [ %div18.i.i.75, %cond.end.i.i.76 ]
  %.0204 = phi float [ %div.i.i.64, %if.then.7.i.i.65 ], [ %div17.i.i.74, %cond.end.i.i.76 ]
  %cmp20.i.i.77 = fcmp fast ogt float %.0204, %.0207
  %.1208 = select i1 %cmp20.i.i.77, float %.0204, float %.0207
  %.1205 = select i1 %cmp20.i.i.77, float %.0207, float %.0204
  br label %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.81"

"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.81":   ; preds = %if.end.19.i.i.78, %if.end.8
  %retval.i.i.32.0 = phi i1 [ false, %if.end.8 ], [ true, %if.end.19.i.i.78 ]
  %.2209 = phi float [ undef, %if.end.8 ], [ %.1208, %if.end.19.i.i.78 ]
  %.2206 = phi float [ undef, %if.end.8 ], [ %.1205, %if.end.19.i.i.78 ]
  br i1 %retval.i.i.32.0, label %if.end.i.84, label %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit98"

if.end.i.84:                                      ; preds = %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.81"
  %cmp.i.83 = fcmp fast ogt float %.2206, %.2209
  %t0.i.43.0 = select i1 %cmp.i.83, float %.2209, float %.2206
  %t1.i.44.0 = select i1 %cmp.i.83, float %.2206, float %.2209
  %cmp9.i.86 = fcmp fast olt float %t0.i.43.0, 0.000000e+00
  br i1 %cmp9.i.86, label %if.then.12.i.89, label %if.end.18.i.97

if.then.12.i.89:                                  ; preds = %if.end.i.84
  %cmp13.i.88 = fcmp fast olt float %t1.i.44.0, 0.000000e+00
  br i1 %cmp13.i.88, label %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit98", label %if.end.18.i.97

if.end.18.i.97:                                   ; preds = %if.then.12.i.89, %if.end.i.84
  %t0.i.43.1 = phi float [ %t1.i.44.0, %if.then.12.i.89 ], [ %t0.i.43.0, %if.end.i.84 ]
  %mul19.i.93.i0 = fmul fast float %t0.i.43.1, %direction.i0
  %mul19.i.93.i1 = fmul fast float %t0.i.43.1, %direction.i1
  %mul19.i.93.i2 = fmul fast float %t0.i.43.1, %direction.i2
  %ObjectToWorld235 = call float @dx.op.objectToWorld.f32(i32 151, i32 0, i8 0)
  %ObjectToWorld236 = call float @dx.op.objectToWorld.f32(i32 151, i32 0, i8 1)
  %ObjectToWorld237 = call float @dx.op.objectToWorld.f32(i32 151, i32 0, i8 2)
  %ObjectToWorld239 = call float @dx.op.objectToWorld.f32(i32 151, i32 1, i8 0)
  %ObjectToWorld240 = call float @dx.op.objectToWorld.f32(i32 151, i32 1, i8 1)
  %ObjectToWorld241 = call float @dx.op.objectToWorld.f32(i32 151, i32 1, i8 2)
  %ObjectToWorld243 = call float @dx.op.objectToWorld.f32(i32 151, i32 2, i8 0)
  %ObjectToWorld244 = call float @dx.op.objectToWorld.f32(i32 151, i32 2, i8 1)
  %ObjectToWorld245 = call float @dx.op.objectToWorld.f32(i32 151, i32 2, i8 2)
  %sub21.i.95.i0 = fadd fast float %origin.i0, %mul19.i.93.i0
  %sub21.i.95.i1 = fadd fast float %origin.i1, %mul19.i.93.i1
  %sub21.i.95.i2 = fadd fast float %sub.i.52.i2, %mul19.i.93.i2
  %24 = fmul fast float %sub21.i.95.i0, %sub21.i.95.i0
  %25 = fmul fast float %sub21.i.95.i1, %sub21.i.95.i1
  %26 = fadd fast float %24, %25
  %27 = fmul fast float %sub21.i.95.i2, %sub21.i.95.i2
  %28 = fadd fast float %26, %27
  %Sqrt222 = call float @dx.op.unary.f32(i32 24, float %28)
  %.i0285 = fdiv fast float %sub21.i.95.i0, %Sqrt222
  %.i1286 = fdiv fast float %sub21.i.95.i1, %Sqrt222
  %.i2287 = fdiv fast float %sub21.i.95.i2, %Sqrt222
  %29 = fmul fast float %.i0285, %ObjectToWorld235
  %FMad265 = call float @dx.op.tertiary.f32(i32 46, float %.i1286, float %ObjectToWorld239, float %29)
  %FMad264 = call float @dx.op.tertiary.f32(i32 46, float %.i2287, float %ObjectToWorld243, float %FMad265)
  %30 = fmul fast float %.i0285, %ObjectToWorld236
  %FMad263 = call float @dx.op.tertiary.f32(i32 46, float %.i1286, float %ObjectToWorld240, float %30)
  %FMad262 = call float @dx.op.tertiary.f32(i32 46, float %.i2287, float %ObjectToWorld244, float %FMad263)
  %31 = fmul fast float %.i0285, %ObjectToWorld237
  %FMad261 = call float @dx.op.tertiary.f32(i32 46, float %.i1286, float %ObjectToWorld241, float %31)
  %FMad260 = call float @dx.op.tertiary.f32(i32 46, float %.i2287, float %ObjectToWorld245, float %FMad261)
  br label %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit98"

"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit98": ; preds = %if.end.18.i.97, %if.then.12.i.89, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.81"
  %retval.i.38.0 = phi i1 [ true, %if.end.18.i.97 ], [ false, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.81" ], [ false, %if.then.12.i.89 ]
  %.0189.0.i0 = phi float [ %FMad264, %if.end.18.i.97 ], [ %.0190.0.i0, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.81" ], [ %.0190.0.i0, %if.then.12.i.89 ]
  %.0189.0.i1 = phi float [ %FMad262, %if.end.18.i.97 ], [ %.0190.0.i1, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.81" ], [ %.0190.0.i1, %if.then.12.i.89 ]
  %.0189.0.i2 = phi float [ %FMad260, %if.end.18.i.97 ], [ %.0190.0.i2, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.81" ], [ %.0190.0.i2, %if.then.12.i.89 ]
  %.0192 = phi float [ %t0.i.43.1, %if.end.18.i.97 ], [ %.0191, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.81" ], [ %.0191, %if.then.12.i.89 ]
  %cmp13 = fcmp fast olt float %.0192, %21
  %or.cond210 = and i1 %retval.i.38.0, %cmp13
  br i1 %or.cond210, label %if.then.16, label %if.end.18

if.then.16:                                       ; preds = %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit98"
  store float %.0192, float* %2, align 4, !tbaa !228
  br label %if.end.18

if.end.18:                                        ; preds = %if.then.16, %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit98"
  %32 = phi float [ %.0192, %if.then.16 ], [ %21, %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit98" ]
  %hitFound.1 = phi i32 [ 1, %if.then.16 ], [ %hitFound.0, %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit98" ]
  %.0.1.i0 = phi float [ %.0189.0.i0, %if.then.16 ], [ %.0.0.i0, %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit98" ]
  %.0.1.i1 = phi float [ %.0189.0.i1, %if.then.16 ], [ %.0.0.i1, %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit98" ]
  %.0.1.i2 = phi float [ %.0189.0.i2, %if.then.16 ], [ %.0.0.i2, %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit98" ]
  %sub.i.121.i0 = fadd fast float %origin.i0, 0xBFD3333340000000
  %sub.i.121.i1 = fadd fast float %origin.i1, 0xBFD3333340000000
  %33 = call float @dx.op.dot3.f32(i32 55, float %6, float %7, float %8, float %sub.i.121.i0, float %sub.i.121.i1, float %origin.i2)
  %mul.i.122 = fmul fast float %33, 2.000000e+00
  %34 = call float @dx.op.dot3.f32(i32 55, float %sub.i.121.i0, float %sub.i.121.i1, float %origin.i2, float %sub.i.121.i0, float %sub.i.121.i1, float %origin.i2)
  %sub4.i.123 = fadd fast float %34, 0xBF970A3DC0000000
  %mul.i.i.124 = fmul fast float %mul.i.122, %mul.i.122
  %mul2.i.i.126 = fmul fast float %mul1.i.i, %sub4.i.123
  %sub.i.i.127 = fsub fast float %mul.i.i.124, %mul2.i.i.126
  %cmp.i.i.128 = fcmp fast olt float %sub.i.i.127, 0.000000e+00
  br i1 %cmp.i.i.128, label %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.150", label %if.else.i.i.131

if.else.i.i.131:                                  ; preds = %if.end.18
  %cmp4.i.i.130 = fcmp fast oeq float %sub.i.i.127, 0.000000e+00
  br i1 %cmp4.i.i.130, label %if.then.7.i.i.134, label %if.else.9.i.i.136

if.then.7.i.i.134:                                ; preds = %if.else.i.i.131
  %mul8.i.i.132 = fsub fast float -0.000000e+00, %33
  %div.i.i.133 = fdiv fast float %mul8.i.i.132, %9
  br label %if.end.19.i.i.147

if.else.9.i.i.136:                                ; preds = %if.else.i.i.131
  %cmp10.i.i.135 = fcmp fast ogt float %mul.i.122, 0.000000e+00
  br i1 %cmp10.i.i.135, label %cond.true.i.i.138, label %cond.false.i.i.140

cond.true.i.i.138:                                ; preds = %if.else.9.i.i.136
  %Sqrt = call float @dx.op.unary.f32(i32 24, float %sub.i.i.127)
  %add.i.i.137 = fadd fast float %Sqrt, %mul.i.122
  br label %cond.end.i.i.145

cond.false.i.i.140:                               ; preds = %if.else.9.i.i.136
  %Sqrt212 = call float @dx.op.unary.f32(i32 24, float %sub.i.i.127)
  %sub15.i.i.139 = fsub fast float %mul.i.122, %Sqrt212
  br label %cond.end.i.i.145

cond.end.i.i.145:                                 ; preds = %cond.false.i.i.140, %cond.true.i.i.138
  %add.sink.i.i.141 = phi float [ %add.i.i.137, %cond.true.i.i.138 ], [ %sub15.i.i.139, %cond.false.i.i.140 ]
  %mul13.i.i.142 = fmul fast float %add.sink.i.i.141, -5.000000e-01
  %div17.i.i.143 = fdiv fast float %mul13.i.i.142, %9
  %div18.i.i.144 = fdiv fast float %sub4.i.123, %mul13.i.i.142
  br label %if.end.19.i.i.147

if.end.19.i.i.147:                                ; preds = %cond.end.i.i.145, %if.then.7.i.i.134
  %.0195 = phi float [ %div.i.i.133, %if.then.7.i.i.134 ], [ %div17.i.i.143, %cond.end.i.i.145 ]
  %.0194 = phi float [ %div.i.i.133, %if.then.7.i.i.134 ], [ %div18.i.i.144, %cond.end.i.i.145 ]
  %cmp20.i.i.146 = fcmp fast ogt float %.0195, %.0194
  %.1196 = select i1 %cmp20.i.i.146, float %.0194, float %.0195
  %.1 = select i1 %cmp20.i.i.146, float %.0195, float %.0194
  br label %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.150"

"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.150":  ; preds = %if.end.19.i.i.147, %if.end.18
  %.2197 = phi float [ undef, %if.end.18 ], [ %.1196, %if.end.19.i.i.147 ]
  %.2 = phi float [ undef, %if.end.18 ], [ %.1, %if.end.19.i.i.147 ]
  %retval.i.i.101.0 = phi i1 [ false, %if.end.18 ], [ true, %if.end.19.i.i.147 ]
  br i1 %retval.i.i.101.0, label %if.end.i.153, label %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit167"

if.end.i.153:                                     ; preds = %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.150"
  %cmp.i.152 = fcmp fast ogt float %.2197, %.2
  %t1.i.113.0 = select i1 %cmp.i.152, float %.2197, float %.2
  %t0.i.112.0 = select i1 %cmp.i.152, float %.2, float %.2197
  %cmp9.i.155 = fcmp fast olt float %t0.i.112.0, 0.000000e+00
  br i1 %cmp9.i.155, label %if.then.12.i.158, label %if.end.18.i.166

if.then.12.i.158:                                 ; preds = %if.end.i.153
  %cmp13.i.157 = fcmp fast olt float %t1.i.113.0, 0.000000e+00
  br i1 %cmp13.i.157, label %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit167", label %if.end.18.i.166

if.end.18.i.166:                                  ; preds = %if.then.12.i.158, %if.end.i.153
  %t0.i.112.1 = phi float [ %t1.i.113.0, %if.then.12.i.158 ], [ %t0.i.112.0, %if.end.i.153 ]
  %mul19.i.162.i0 = fmul fast float %t0.i.112.1, %direction.i0
  %mul19.i.162.i1 = fmul fast float %t0.i.112.1, %direction.i1
  %mul19.i.162.i2 = fmul fast float %t0.i.112.1, %direction.i2
  %ObjectToWorld = call float @dx.op.objectToWorld.f32(i32 151, i32 0, i8 0)
  %ObjectToWorld224 = call float @dx.op.objectToWorld.f32(i32 151, i32 0, i8 1)
  %ObjectToWorld225 = call float @dx.op.objectToWorld.f32(i32 151, i32 0, i8 2)
  %ObjectToWorld227 = call float @dx.op.objectToWorld.f32(i32 151, i32 1, i8 0)
  %ObjectToWorld228 = call float @dx.op.objectToWorld.f32(i32 151, i32 1, i8 1)
  %ObjectToWorld229 = call float @dx.op.objectToWorld.f32(i32 151, i32 1, i8 2)
  %ObjectToWorld231 = call float @dx.op.objectToWorld.f32(i32 151, i32 2, i8 0)
  %ObjectToWorld232 = call float @dx.op.objectToWorld.f32(i32 151, i32 2, i8 1)
  %ObjectToWorld233 = call float @dx.op.objectToWorld.f32(i32 151, i32 2, i8 2)
  %sub21.i.164.i0 = fadd fast float %sub.i.121.i0, %mul19.i.162.i0
  %sub21.i.164.i1 = fadd fast float %sub.i.121.i1, %mul19.i.162.i1
  %sub21.i.164.i2 = fadd fast float %origin.i2, %mul19.i.162.i2
  %35 = fmul fast float %sub21.i.164.i0, %sub21.i.164.i0
  %36 = fmul fast float %sub21.i.164.i1, %sub21.i.164.i1
  %37 = fadd fast float %35, %36
  %38 = fmul fast float %sub21.i.164.i2, %sub21.i.164.i2
  %39 = fadd fast float %37, %38
  %Sqrt221 = call float @dx.op.unary.f32(i32 24, float %39)
  %.i0288 = fdiv fast float %sub21.i.164.i0, %Sqrt221
  %.i1289 = fdiv fast float %sub21.i.164.i1, %Sqrt221
  %.i2290 = fdiv fast float %sub21.i.164.i2, %Sqrt221
  %40 = fmul fast float %.i0288, %ObjectToWorld
  %FMad273 = call float @dx.op.tertiary.f32(i32 46, float %.i1289, float %ObjectToWorld227, float %40)
  %FMad272 = call float @dx.op.tertiary.f32(i32 46, float %.i2290, float %ObjectToWorld231, float %FMad273)
  %41 = fmul fast float %.i0288, %ObjectToWorld224
  %FMad271 = call float @dx.op.tertiary.f32(i32 46, float %.i1289, float %ObjectToWorld228, float %41)
  %FMad270 = call float @dx.op.tertiary.f32(i32 46, float %.i2290, float %ObjectToWorld232, float %FMad271)
  %42 = fmul fast float %.i0288, %ObjectToWorld225
  %FMad269 = call float @dx.op.tertiary.f32(i32 46, float %.i1289, float %ObjectToWorld229, float %42)
  %FMad268 = call float @dx.op.tertiary.f32(i32 46, float %.i2290, float %ObjectToWorld233, float %FMad269)
  br label %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit167"

"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit167": ; preds = %if.end.18.i.166, %if.then.12.i.158, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.150"
  %retval.i.107.0 = phi i1 [ true, %if.end.18.i.166 ], [ false, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.150" ], [ false, %if.then.12.i.158 ]
  %.0188.0.i0 = phi float [ %FMad272, %if.end.18.i.166 ], [ %.0189.0.i0, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.150" ], [ %.0189.0.i0, %if.then.12.i.158 ]
  %.0188.0.i1 = phi float [ %FMad270, %if.end.18.i.166 ], [ %.0189.0.i1, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.150" ], [ %.0189.0.i1, %if.then.12.i.158 ]
  %.0188.0.i2 = phi float [ %FMad268, %if.end.18.i.166 ], [ %.0189.0.i2, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.150" ], [ %.0189.0.i2, %if.then.12.i.158 ]
  %.0193 = phi float [ %t0.i.112.1, %if.end.18.i.166 ], [ %.0192, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.150" ], [ %.0192, %if.then.12.i.158 ]
  %cmp23 = fcmp fast olt float %.0193, %32
  %or.cond211 = and i1 %retval.i.107.0, %cmp23
  br i1 %or.cond211, label %if.then.26, label %if.end.28

if.then.26:                                       ; preds = %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit167"
  store float %.0193, float* %2, align 4, !tbaa !228
  br label %if.end.28

if.end.28:                                        ; preds = %if.then.26, %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit167"
  %hitFound.2 = phi i32 [ 1, %if.then.26 ], [ %hitFound.1, %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit167" ]
  %.0.2.i0 = phi float [ %.0188.0.i0, %if.then.26 ], [ %.0.1.i0, %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit167" ]
  %.0.2.i1 = phi float [ %.0188.0.i1, %if.then.26 ], [ %.0.1.i1, %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit167" ]
  %.0.2.i2 = phi float [ %.0188.0.i2, %if.then.26 ], [ %.0.1.i2, %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit167" ]
  %.0.2.upto0 = insertelement <3 x float> undef, float %.0.2.i0, i32 0
  %.0.2.upto1 = insertelement <3 x float> %.0.2.upto0, float %.0.2.i1, i32 1
  %.0.2 = insertelement <3 x float> %.0.2.upto1, float %.0.2.i2, i32 2
  %tobool29 = icmp ne i32 %hitFound.2, 0
  store <3 x float> %.0.2, <3 x float>* %4, align 4
  ret i1 %tobool29
}

; Function Attrs: alwaysinline nounwind
define i32 @"\01?StackPop@@YAIAIAHAIAII@Z"(i32* noalias nocapture dereferenceable(4), i32* noalias nocapture readnone dereferenceable(4), i32) #5 {
entry:
  %3 = load i32, i32* %0, align 4, !tbaa !223
  %dec = add nsw i32 %3, -1
  store i32 %dec, i32* %0, align 4, !tbaa !223
  %mul = shl i32 %dec, 6
  %add = add i32 %mul, %2
  %arrayidx = getelementptr [2048 x i32], [2048 x i32] addrspace(3)* @"\01?stack@@3PAIA", i32 0, i32 %add
  %4 = load i32, i32 addrspace(3)* %arrayidx, align 4, !tbaa !223
  ret i32 %4
}

; Function Attrs: alwaysinline nounwind readnone
define %class.matrix.float.3.4 @"\01?InverseAffineTransform@@YA?AV?$matrix@M$02$03@@V1@@Z"(%class.matrix.float.3.4) #4 {
entry:
  %.fca.0.0.extract = extractvalue %class.matrix.float.3.4 %0, 0, 0
  %1 = extractelement <4 x float> %.fca.0.0.extract, i32 3
  %2 = extractelement <4 x float> %.fca.0.0.extract, i32 2
  %3 = extractelement <4 x float> %.fca.0.0.extract, i32 1
  %4 = extractelement <4 x float> %.fca.0.0.extract, i32 0
  %.fca.0.1.extract = extractvalue %class.matrix.float.3.4 %0, 0, 1
  %5 = extractelement <4 x float> %.fca.0.1.extract, i32 3
  %6 = extractelement <4 x float> %.fca.0.1.extract, i32 2
  %7 = extractelement <4 x float> %.fca.0.1.extract, i32 1
  %8 = extractelement <4 x float> %.fca.0.1.extract, i32 0
  %.fca.0.2.extract = extractvalue %class.matrix.float.3.4 %0, 0, 2
  %9 = extractelement <4 x float> %.fca.0.2.extract, i32 3
  %10 = extractelement <4 x float> %.fca.0.2.extract, i32 2
  %11 = extractelement <4 x float> %.fca.0.2.extract, i32 1
  %12 = extractelement <4 x float> %.fca.0.2.extract, i32 0
  %mul.i = fmul fast float %4, %7
  %mul1.i = fmul fast float %mul.i, %10
  %mul2.i = fmul fast float %4, %11
  %mul3.i = fmul fast float %mul2.i, %6
  %sub.i = fsub fast float %mul1.i, %mul3.i
  %mul4.i = fmul fast float %8, %3
  %mul5.i = fmul fast float %mul4.i, %10
  %sub6.i = fsub fast float %sub.i, %mul5.i
  %mul7.i = fmul fast float %8, %11
  %mul8.i = fmul fast float %mul7.i, %2
  %add.i = fadd fast float %sub6.i, %mul8.i
  %mul9.i = fmul fast float %12, %3
  %mul10.i = fmul fast float %mul9.i, %6
  %add11.i = fadd fast float %add.i, %mul10.i
  %mul12.i = fmul fast float %12, %7
  %mul13.i = fmul fast float %mul12.i, %2
  %sub14.i = fsub fast float %add11.i, %mul13.i
  %13 = fdiv fast float 1.000000e+00, %sub14.i
  %mul3 = fmul fast float %7, %10
  %14 = fmul fast float %6, %11
  %add = fsub fast float %mul3, %14
  %mul13 = fmul fast float %add, %13
  %mul17 = fmul fast float %6, %12
  %15 = fmul fast float %8, %10
  %add22 = fsub fast float %mul17, %15
  %mul28 = fmul fast float %add22, %13
  %sub40 = fsub fast float %mul7.i, %mul12.i
  %mul43 = fmul fast float %sub40, %13
  %mul47 = fmul fast float %11, %2
  %16 = fmul fast float %10, %3
  %add57 = fsub fast float %mul47, %16
  %mul58 = fmul fast float %add57, %13
  %mul62 = fmul fast float %10, %4
  %17 = fmul fast float %12, %2
  %add72 = fsub fast float %mul62, %17
  %mul73 = fmul fast float %add72, %13
  %sub80 = fsub fast float %mul9.i, %mul2.i
  %mul88 = fmul fast float %sub80, %13
  %mul96 = fmul fast float %3, %6
  %18 = fmul fast float %2, %7
  %add102 = fsub fast float %mul96, %18
  %mul103 = fmul fast float %add102, %13
  %mul111 = fmul fast float %2, %8
  %19 = fmul fast float %4, %6
  %add117 = fsub fast float %mul111, %19
  %mul118 = fmul fast float %add117, %13
  %sub121 = fsub fast float %mul.i, %mul4.i
  %mul133 = fmul fast float %sub121, %13
  %mul134 = fmul fast float %10, %5
  %mul135 = fmul fast float %6, %9
  %sub136 = fsub fast float %mul134, %mul135
  %mul137 = fmul fast float %sub136, %3
  %mul138 = fmul fast float %2, %9
  %mul139 = fmul fast float %10, %1
  %sub140 = fsub fast float %mul138, %mul139
  %mul141 = fmul fast float %sub140, %7
  %add142 = fadd fast float %mul137, %mul141
  %mul143 = fmul fast float %6, %1
  %mul144 = fmul fast float %2, %5
  %sub145 = fsub fast float %mul143, %mul144
  %mul146 = fmul fast float %sub145, %11
  %add147 = fadd fast float %add142, %mul146
  %mul148 = fmul fast float %add147, %13
  %mul149 = fmul fast float %12, %5
  %mul150 = fmul fast float %8, %9
  %sub151 = fsub fast float %mul149, %mul150
  %mul152 = fmul fast float %sub151, %2
  %mul153 = fmul fast float %4, %9
  %mul154 = fmul fast float %12, %1
  %sub155 = fsub fast float %mul153, %mul154
  %mul156 = fmul fast float %sub155, %6
  %add157 = fadd fast float %mul152, %mul156
  %mul158 = fmul fast float %8, %1
  %mul159 = fmul fast float %4, %5
  %sub160 = fsub fast float %mul158, %mul159
  %mul161 = fmul fast float %sub160, %10
  %add162 = fadd fast float %add157, %mul161
  %mul163 = fmul fast float %add162, %13
  %sub166 = fsub fast float %mul12.i, %mul7.i
  %mul167 = fmul fast float %sub166, %1
  %sub170 = fsub fast float %mul2.i, %mul9.i
  %mul171 = fmul fast float %sub170, %5
  %add172 = fadd fast float %mul167, %mul171
  %sub175 = fsub fast float %mul4.i, %mul.i
  %mul176 = fmul fast float %sub175, %9
  %add177 = fadd fast float %add172, %mul176
  %mul178 = fmul fast float %add177, %13
  %.0.vec.extract.upto0 = insertelement <4 x float> undef, float %mul13, i32 0
  %.0.vec.extract.upto1 = insertelement <4 x float> %.0.vec.extract.upto0, float %mul58, i32 1
  %.0.vec.extract.upto2 = insertelement <4 x float> %.0.vec.extract.upto1, float %mul103, i32 2
  %.0.vec.extract = insertelement <4 x float> %.0.vec.extract.upto2, float %mul148, i32 3
  %.fca.0.0.insert = insertvalue %class.matrix.float.3.4 undef, <4 x float> %.0.vec.extract, 0, 0
  %.16.vec.extract.upto0 = insertelement <4 x float> undef, float %mul28, i32 0
  %.16.vec.extract.upto1 = insertelement <4 x float> %.16.vec.extract.upto0, float %mul73, i32 1
  %.16.vec.extract.upto2 = insertelement <4 x float> %.16.vec.extract.upto1, float %mul118, i32 2
  %.16.vec.extract = insertelement <4 x float> %.16.vec.extract.upto2, float %mul163, i32 3
  %.fca.0.1.insert = insertvalue %class.matrix.float.3.4 %.fca.0.0.insert, <4 x float> %.16.vec.extract, 0, 1
  %.32.vec.extract.upto0 = insertelement <4 x float> undef, float %mul43, i32 0
  %.32.vec.extract.upto1 = insertelement <4 x float> %.32.vec.extract.upto0, float %mul88, i32 1
  %.32.vec.extract.upto2 = insertelement <4 x float> %.32.vec.extract.upto1, float %mul133, i32 2
  %.32.vec.extract = insertelement <4 x float> %.32.vec.extract.upto2, float %mul178, i32 3
  %.fca.0.2.insert = insertvalue %class.matrix.float.3.4 %.fca.0.1.insert, <4 x float> %.32.vec.extract, 0, 2
  ret %class.matrix.float.3.4 %.fca.0.2.insert
}

; Function Attrs: alwaysinline nounwind
define void @"\01?CreateRWByteAddressBufferPointer@@YA?AURWByteAddressBufferPointer@@URWByteAddressBuffer@@I@Z"(%struct.RWByteAddressBufferPointer* noalias nocapture sret, %struct.RWByteAddressBuffer* nocapture readonly, i32) #5 {
entry:
  %buffer1 = getelementptr inbounds %struct.RWByteAddressBufferPointer, %struct.RWByteAddressBufferPointer* %0, i32 0, i32 0
  %3 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %1, align 4
  store %struct.RWByteAddressBuffer %3, %struct.RWByteAddressBuffer* %buffer1, align 4
  %offsetInBytes2 = getelementptr inbounds %struct.RWByteAddressBufferPointer, %struct.RWByteAddressBufferPointer* %0, i32 0, i32 1
  store i32 %2, i32* %offsetInBytes2, align 4, !tbaa !223
  ret void
}

; Function Attrs: alwaysinline nounwind readonly
define <3 x float> @"\01?HitAttribute@@YA?AV?$vector@M$02@@Y02V1@V?$vector@M$01@@@Z"([3 x <3 x float>]* nocapture readonly, <2 x float>) #6 {
entry:
  %arrayidx = getelementptr inbounds [3 x <3 x float>], [3 x <3 x float>]* %0, i32 0, i32 0
  %2 = load <3 x float>, <3 x float>* %arrayidx, align 4, !tbaa !220
  %.i013 = extractelement <3 x float> %2, i32 0
  %.i114 = extractelement <3 x float> %2, i32 1
  %.i215 = extractelement <3 x float> %2, i32 2
  %3 = extractelement <2 x float> %1, i32 0
  %arrayidx1 = getelementptr inbounds [3 x <3 x float>], [3 x <3 x float>]* %0, i32 0, i32 1
  %4 = load <3 x float>, <3 x float>* %arrayidx1, align 4, !tbaa !220
  %.i010 = extractelement <3 x float> %4, i32 0
  %.i111 = extractelement <3 x float> %4, i32 1
  %.i212 = extractelement <3 x float> %4, i32 2
  %sub.i0 = fsub fast float %.i010, %.i013
  %sub.i1 = fsub fast float %.i111, %.i114
  %sub.i2 = fsub fast float %.i212, %.i215
  %mul.i0 = fmul fast float %sub.i0, %3
  %mul.i1 = fmul fast float %sub.i1, %3
  %mul.i2 = fmul fast float %sub.i2, %3
  %add.i0 = fadd fast float %mul.i0, %.i013
  %add.i1 = fadd fast float %mul.i1, %.i114
  %add.i2 = fadd fast float %mul.i2, %.i215
  %5 = extractelement <2 x float> %1, i32 1
  %arrayidx5 = getelementptr inbounds [3 x <3 x float>], [3 x <3 x float>]* %0, i32 0, i32 2
  %6 = load <3 x float>, <3 x float>* %arrayidx5, align 4, !tbaa !220
  %.i017 = extractelement <3 x float> %6, i32 0
  %.i119 = extractelement <3 x float> %6, i32 1
  %.i221 = extractelement <3 x float> %6, i32 2
  %sub7.i0 = fsub fast float %.i017, %.i013
  %sub7.i1 = fsub fast float %.i119, %.i114
  %sub7.i2 = fsub fast float %.i221, %.i215
  %mul8.i0 = fmul fast float %sub7.i0, %5
  %mul8.i1 = fmul fast float %sub7.i1, %5
  %mul8.i2 = fmul fast float %sub7.i2, %5
  %add9.i0 = fadd fast float %add.i0, %mul8.i0
  %add9.i1 = fadd fast float %add.i1, %mul8.i1
  %add9.i2 = fadd fast float %add.i2, %mul8.i2
  %add9.upto0 = insertelement <3 x float> undef, float %add9.i0, i32 0
  %add9.upto1 = insertelement <3 x float> %add9.upto0, float %add9.i1, i32 1
  %add9 = insertelement <3 x float> %add9.upto1, float %add9.i2, i32 2
  ret <3 x float> %add9
}

; Function Attrs: alwaysinline nounwind
define void @"\01?Fallback_TraceRay@@YAXIIIIIMMMMMMMMI@Z"(i32, i32, i32, i32, i32, float, float, float, float, float, float, float, float, i32) #7 {
entry:
  %14 = load %struct.ByteAddressBuffer, %struct.ByteAddressBuffer* @"\01?MissShaderTable@@3UByteAddressBuffer@@A", align 4
  %15 = load %struct.ByteAddressBuffer, %struct.ByteAddressBuffer* @"\01?HitGroupShaderTable@@3UByteAddressBuffer@@A", align 4
  %16 = load %AccelerationStructureList, %AccelerationStructureList* @AccelerationStructureList, align 4
  %17 = load %Constants, %Constants* @Constants, align 4
  %AccelerationStructureList = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.AccelerationStructureList(i32 160, %AccelerationStructureList %16)
  %Constants337 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.Constants(i32 160, %Constants %17)
  %18 = alloca [3 x float], align 4
  %19 = alloca [3 x float], align 4
  %20 = alloca [3 x float], align 4
  %21 = alloca [3 x float], align 4
  %22 = alloca [3 x float], align 4
  %23 = alloca [3 x float], align 4
  %nodesToProcess.i = alloca [2 x i32], align 4
  %attr.i = alloca %struct.BuiltInTriangleIntersectionAttributes, align 4
  %24 = insertelement <3 x float> undef, float %9, i64 0
  %25 = insertelement <3 x float> %24, float %10, i64 1
  %26 = insertelement <3 x float> %25, float %11, i64 2
  %27 = insertelement <3 x float> undef, float %5, i64 0
  %28 = insertelement <3 x float> %27, float %6, i64 1
  %29 = insertelement <3 x float> %28, float %7, i64 2
  %call = call i32 @"\01?Fallback_TraceRayBegin@@YAIIV?$vector@M$02@@M0MI@Z"(i32 %0, <3 x float> %29, float %8, <3 x float> %26, float %12, i32 %13) #2
  %call.i = call i32 @"\01?Fallback_GroupIndex@@YAIXZ"() #2
  %WorldRayDirection298 = call float @dx.op.worldRayDirection.f32(i32 148, i8 0)
  %WorldRayDirection299 = call float @dx.op.worldRayDirection.f32(i32 148, i8 1)
  %WorldRayDirection300 = call float @dx.op.worldRayDirection.f32(i32 148, i8 2)
  %WorldRayOrigin295 = call float @dx.op.worldRayOrigin.f32(i32 147, i8 0)
  %WorldRayOrigin296 = call float @dx.op.worldRayOrigin.f32(i32 147, i8 1)
  %WorldRayOrigin297 = call float @dx.op.worldRayOrigin.f32(i32 147, i8 2)
  %30 = getelementptr inbounds [3 x float], [3 x float]* %23, i32 0, i32 0
  store float %WorldRayDirection298, float* %30, align 4
  %31 = getelementptr inbounds [3 x float], [3 x float]* %23, i32 0, i32 1
  store float %WorldRayDirection299, float* %31, align 4
  %32 = getelementptr inbounds [3 x float], [3 x float]* %23, i32 0, i32 2
  store float %WorldRayDirection300, float* %32, align 4
  %.i0 = fdiv fast float 1.000000e+00, %WorldRayDirection298
  %.i1 = fdiv fast float 1.000000e+00, %WorldRayDirection299
  %.i2 = fdiv fast float 1.000000e+00, %WorldRayDirection300
  %mul.i.i.i0 = fmul fast float %.i0, %WorldRayOrigin295
  %mul.i.i.i1 = fmul fast float %.i1, %WorldRayOrigin296
  %mul.i.i.i2 = fmul fast float %.i2, %WorldRayOrigin297
  %FAbs282 = call float @dx.op.unary.f32(i32 6, float %WorldRayDirection298)
  %FAbs283 = call float @dx.op.unary.f32(i32 6, float %WorldRayDirection299)
  %FAbs284 = call float @dx.op.unary.f32(i32 6, float %WorldRayDirection300)
  %cmp.i.i.i = fcmp fast ogt float %FAbs282, %FAbs283
  %cmp1.i.i.i = fcmp fast ogt float %FAbs282, %FAbs284
  %33 = and i1 %cmp.i.i.i, %cmp1.i.i.i
  br i1 %33, label %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i", label %if.else.i.i.i

if.else.i.i.i:                                    ; preds = %entry
  %cmp4.i.i.i = fcmp fast ogt float %FAbs283, %FAbs284
  br i1 %cmp4.i.i.i, label %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i", label %if.else.8.i.i.i

if.else.8.i.i.i:                                  ; preds = %if.else.i.i.i
  br label %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i"

"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i": ; preds = %if.else.8.i.i.i, %if.else.i.i.i, %entry
  %retval.i.i.i.0 = phi i32 [ 2, %if.else.8.i.i.i ], [ 0, %entry ], [ 1, %if.else.i.i.i ]
  %add.i.i = add nuw nsw i32 %retval.i.i.i.0, 1
  %rem.i.i = urem i32 %add.i.i, 3
  %add4.i.i = add nuw nsw i32 %retval.i.i.i.0, 2
  %rem5.i.i = urem i32 %add4.i.i, 3
  %34 = getelementptr [3 x float], [3 x float]* %23, i32 0, i32 %retval.i.i.i.0
  %35 = load float, float* %34, align 4, !tbaa !228, !noalias !230
  %cmp.i.i = fcmp fast olt float %35, 0.000000e+00
  %currentRayData.i.5.0.i0 = select i1 %cmp.i.i, i32 %rem5.i.i, i32 %rem.i.i
  %currentRayData.i.5.0.i1 = select i1 %cmp.i.i, i32 %rem.i.i, i32 %rem5.i.i
  %36 = getelementptr [3 x float], [3 x float]* %23, i32 0, i32 %currentRayData.i.5.0.i0
  %37 = load float, float* %36, align 4, !tbaa !228, !noalias !230
  %38 = getelementptr [3 x float], [3 x float]* %23, i32 0, i32 %currentRayData.i.5.0.i1
  %39 = load float, float* %38, align 4, !tbaa !228, !noalias !230
  %40 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %AccelerationStructureList, i32 0)
  %41 = extractvalue %dx.types.CBufRet.i32 %40, 0
  %42 = extractvalue %dx.types.CBufRet.i32 %40, 1
  %arrayidx.i = getelementptr inbounds [2 x i32], [2 x i32]* %nodesToProcess.i, i32 0, i32 0
  store i32 0, i32* %arrayidx.i, align 4, !tbaa !223
  %arrayidx.i.i.i = getelementptr inbounds [0 x %struct.RWByteAddressBuffer], [0 x %struct.RWByteAddressBuffer]* @"\01?DescriptorHeapBufferTable@@3PAURWByteAddressBuffer@@A", i32 0, i32 %42, !dx.nonuniform !233
  %43 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %arrayidx.i.i.i, align 4, !noalias !115
  %add.i = add i32 %41, 4
  %44 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %43)
  %45 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %44, i32 %add.i, i32 undef)
  %46 = extractvalue %dx.types.ResRet.i32 %45, 0
  %add5.i = add i32 %46, %41
  %add.i.i.i = add i32 %41, 16
  %47 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %44, i32 %add.i.i.i, i32 undef)
  %48 = extractvalue %dx.types.ResRet.i32 %47, 0
  %49 = extractvalue %dx.types.ResRet.i32 %47, 1
  %50 = extractvalue %dx.types.ResRet.i32 %47, 2
  %add.i.214.i = add i32 %41, 32
  %51 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %44, i32 %add.i.214.i, i32 undef)
  %52 = extractvalue %dx.types.ResRet.i32 %51, 0
  %53 = extractvalue %dx.types.ResRet.i32 %51, 1
  %54 = extractvalue %dx.types.ResRet.i32 %51, 2
  %55 = bitcast i32 %48 to float
  %56 = bitcast i32 %49 to float
  %57 = bitcast i32 %50 to float
  %58 = bitcast i32 %52 to float
  %59 = bitcast i32 %53 to float
  %60 = bitcast i32 %54 to float
  %call6.i = call float @"\01?Fallback_RayTCurrent@@YAMXZ"() #2
  %61 = fsub fast float %55, %WorldRayOrigin295
  %62 = fmul fast float %61, %.i0
  %63 = fsub fast float %56, %WorldRayOrigin296
  %64 = fmul fast float %63, %.i1
  %65 = fsub fast float %57, %WorldRayOrigin297
  %66 = fmul fast float %65, %.i2
  %FAbs279 = call float @dx.op.unary.f32(i32 6, float %.i0)
  %FAbs280 = call float @dx.op.unary.f32(i32 6, float %.i1)
  %FAbs281 = call float @dx.op.unary.f32(i32 6, float %.i2)
  %mul1.i.i.i0 = fmul fast float %58, %FAbs279
  %mul1.i.i.i1 = fmul fast float %FAbs280, %59
  %mul1.i.i.i2 = fmul fast float %FAbs281, %60
  %add.i.216.i.i0 = fadd fast float %mul1.i.i.i0, %62
  %add.i.216.i.i1 = fadd fast float %mul1.i.i.i1, %64
  %add.i.216.i.i2 = fadd fast float %mul1.i.i.i2, %66
  %sub4.i.i.i0 = fsub fast float %62, %mul1.i.i.i0
  %sub4.i.i.i1 = fsub fast float %64, %mul1.i.i.i1
  %sub4.i.i.i2 = fsub fast float %66, %mul1.i.i.i2
  %FMax319 = call float @dx.op.binary.f32(i32 35, float %sub4.i.i.i0, float %sub4.i.i.i1)
  %FMax318 = call float @dx.op.binary.f32(i32 35, float %FMax319, float %sub4.i.i.i2)
  %FMin317 = call float @dx.op.binary.f32(i32 36, float %add.i.216.i.i0, float %add.i.216.i.i1)
  %FMin316 = call float @dx.op.binary.f32(i32 36, float %FMin317, float %add.i.216.i.i2)
  %FMax315 = call float @dx.op.binary.f32(i32 35, float %FMax318, float 0.000000e+00)
  %FMin314 = call float @dx.op.binary.f32(i32 36, float %FMin316, float %call6.i)
  %cmp.i.217.i = fcmp fast olt float %FMax315, %FMin314
  br i1 %cmp.i.217.i, label %if.then.i, label %if.end.i

if.then.i:                                        ; preds = %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i"
  %div14.i.i = fdiv float %39, %35
  %div.i.i = fdiv float %37, %35
  %arrayidx.i.i = getelementptr [2048 x i32], [2048 x i32] addrspace(3)* @"\01?stack@@3PAIA", i32 0, i32 %call.i
  store i32 0, i32 addrspace(3)* %arrayidx.i.i, align 4, !tbaa !223, !noalias !234
  store i32 1, i32* %arrayidx.i, align 4, !tbaa !223
  call void @"\01?Fallback_SetInstanceIndex@@YAXI@Z"(i32 -1) #2
  %67 = getelementptr inbounds [3 x float], [3 x float]* %22, i32 0, i32 0
  %68 = getelementptr inbounds [3 x float], [3 x float]* %22, i32 0, i32 1
  %69 = getelementptr inbounds [3 x float], [3 x float]* %22, i32 0, i32 2
  %arrayidx42.i = getelementptr inbounds [2 x i32], [2 x i32]* %nodesToProcess.i, i32 0, i32 1
  %70 = getelementptr inbounds [3 x float], [3 x float]* %21, i32 0, i32 0
  %71 = getelementptr inbounds [3 x float], [3 x float]* %21, i32 0, i32 1
  %72 = getelementptr inbounds [3 x float], [3 x float]* %21, i32 0, i32 2
  %73 = getelementptr inbounds [3 x float], [3 x float]* %19, i32 0, i32 0
  %74 = getelementptr inbounds [3 x float], [3 x float]* %19, i32 0, i32 1
  %75 = getelementptr inbounds [3 x float], [3 x float]* %19, i32 0, i32 2
  %76 = getelementptr inbounds [3 x float], [3 x float]* %20, i32 0, i32 0
  %77 = getelementptr inbounds [3 x float], [3 x float]* %20, i32 0, i32 1
  %78 = getelementptr inbounds [3 x float], [3 x float]* %20, i32 0, i32 2
  %barycentrics.i = getelementptr inbounds %struct.BuiltInTriangleIntersectionAttributes, %struct.BuiltInTriangleIntersectionAttributes* %attr.i, i32 0, i32 0
  %79 = getelementptr inbounds [3 x float], [3 x float]* %18, i32 0, i32 0
  %80 = getelementptr inbounds [3 x float], [3 x float]* %18, i32 0, i32 1
  %81 = getelementptr inbounds [3 x float], [3 x float]* %18, i32 0, i32 2
  %cmp4.i.i.522.i = fcmp fast ogt float %FAbs283, %FAbs284
  %brmerge = or i1 %33, %cmp4.i.i.522.i
  %82 = zext i1 %33 to i32
  %.mux = xor i32 %82, 1
  br label %while.body.i

if.end.i:                                         ; preds = %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i"
  call void @"\01?Fallback_SetInstanceIndex@@YAXI@Z"(i32 -1) #2
  br label %"\01?Traverse@@YA_NIII@Z.exit"

while.body.i:                                     ; preds = %do.end.i, %if.then.i
  %resultTriId.i.0221 = phi i32 [ %resultTriId.i.3, %do.end.i ], [ undef, %if.then.i ]
  %resultBary.i.0220.i0 = phi float [ %resultBary.i.3.i0, %do.end.i ], [ undef, %if.then.i ]
  %resultBary.i.0220.i1 = phi float [ %resultBary.i.3.i1, %do.end.i ], [ undef, %if.then.i ]
  %stackPointer.i.1218 = phi i32 [ %stackPointer.i.3, %do.end.i ], [ 1, %if.then.i ]
  %instanceId.i.0217 = phi i32 [ %instanceId.i.2, %do.end.i ], [ 0, %if.then.i ]
  %instanceOffset.i.0216 = phi i32 [ %instanceOffset.i.2, %do.end.i ], [ 0, %if.then.i ]
  %instanceFlags.i.0215 = phi i32 [ %instanceFlags.i.2, %do.end.i ], [ 0, %if.then.i ]
  %instanceIndex.i.0214 = phi i32 [ %instanceIndex.i.2, %do.end.i ], [ 0, %if.then.i ]
  %currentGpuVA.i.0213.i0 = phi i32 [ %299, %do.end.i ], [ %41, %if.then.i ]
  %currentGpuVA.i.0213.i1 = phi i32 [ %300, %do.end.i ], [ %42, %if.then.i ]
  %currentBVHIndex.i.0212 = phi i32 [ %dec196.i, %do.end.i ], [ 0, %if.then.i ]
  %flagContainer.i.0211 = phi i32 [ %flagContainer.i.3, %do.end.i ], [ 0, %if.then.i ]
  %currentRayData.i.5.1210.i0 = phi i32 [ %tmp199.i.5.0.i0, %do.end.i ], [ %currentRayData.i.5.0.i0, %if.then.i ]
  %currentRayData.i.5.1210.i1 = phi i32 [ %tmp199.i.5.0.i1, %do.end.i ], [ %currentRayData.i.5.0.i1, %if.then.i ]
  %currentRayData.i.5.1210.i2 = phi i32 [ %.mux., %do.end.i ], [ %retval.i.i.i.0, %if.then.i ]
  %currentRayData.i.4.0209.i0 = phi float [ %div.i.539.i, %do.end.i ], [ %div.i.i, %if.then.i ]
  %currentRayData.i.4.0209.i1 = phi float [ %div14.i.542.i, %do.end.i ], [ %div14.i.i, %if.then.i ]
  %.pn = phi float [ %293, %do.end.i ], [ %35, %if.then.i ]
  %.0200204.i0 = phi float [ %.4.i0, %do.end.i ], [ undef, %if.then.i ]
  %.0200204.i1 = phi float [ %.4.i1, %do.end.i ], [ undef, %if.then.i ]
  %currentRayData.i.4.0209.i2 = fdiv float 1.000000e+00, %.pn
  %arrayidx13.i.phi.trans.insert = getelementptr inbounds [2 x i32], [2 x i32]* %nodesToProcess.i, i32 0, i32 %currentBVHIndex.i.0212
  %.pre = load i32, i32* %arrayidx13.i.phi.trans.insert, align 4, !tbaa !223
  br label %do.body.i

do.body.i:                                        ; preds = %do.cond.i, %while.body.i
  %83 = phi i32 [ %.pre, %while.body.i ], [ %291, %do.cond.i ]
  %.1.i0 = phi float [ %.0200204.i0, %while.body.i ], [ %.4.i0, %do.cond.i ]
  %.1.i1 = phi float [ %.0200204.i1, %while.body.i ], [ %.4.i1, %do.cond.i ]
  %currentRayData.i.0.1.i0 = phi float [ %WorldRayOrigin295, %while.body.i ], [ %currentRayData.i.0.2.i0, %do.cond.i ]
  %currentRayData.i.0.1.i1 = phi float [ %WorldRayOrigin296, %while.body.i ], [ %currentRayData.i.0.2.i1, %do.cond.i ]
  %currentRayData.i.0.1.i2 = phi float [ %WorldRayOrigin297, %while.body.i ], [ %currentRayData.i.0.2.i2, %do.cond.i ]
  %currentRayData.i.2.1.i0 = phi float [ %.i0, %while.body.i ], [ %currentRayData.i.2.2.i0, %do.cond.i ]
  %currentRayData.i.2.1.i1 = phi float [ %.i1, %while.body.i ], [ %currentRayData.i.2.2.i1, %do.cond.i ]
  %currentRayData.i.2.1.i2 = phi float [ %.i2, %while.body.i ], [ %currentRayData.i.2.2.i2, %do.cond.i ]
  %currentRayData.i.3.1.i0 = phi float [ %mul.i.i.i0, %while.body.i ], [ %currentRayData.i.3.2.i0, %do.cond.i ]
  %currentRayData.i.3.1.i1 = phi float [ %mul.i.i.i1, %while.body.i ], [ %currentRayData.i.3.2.i1, %do.cond.i ]
  %currentRayData.i.3.1.i2 = phi float [ %mul.i.i.i2, %while.body.i ], [ %currentRayData.i.3.2.i2, %do.cond.i ]
  %currentRayData.i.4.1.i0 = phi float [ %currentRayData.i.4.0209.i0, %while.body.i ], [ %currentRayData.i.4.2.i0, %do.cond.i ]
  %currentRayData.i.4.1.i1 = phi float [ %currentRayData.i.4.0209.i1, %while.body.i ], [ %currentRayData.i.4.2.i1, %do.cond.i ]
  %currentRayData.i.4.1.i2 = phi float [ %currentRayData.i.4.0209.i2, %while.body.i ], [ %currentRayData.i.4.2.i2, %do.cond.i ]
  %currentRayData.i.5.2.i0 = phi i32 [ %currentRayData.i.5.1210.i0, %while.body.i ], [ %currentRayData.i.5.3.i0, %do.cond.i ]
  %currentRayData.i.5.2.i1 = phi i32 [ %currentRayData.i.5.1210.i1, %while.body.i ], [ %currentRayData.i.5.3.i1, %do.cond.i ]
  %currentRayData.i.5.2.i2 = phi i32 [ %currentRayData.i.5.1210.i2, %while.body.i ], [ %currentRayData.i.5.3.i2, %do.cond.i ]
  %flagContainer.i.1 = phi i32 [ %flagContainer.i.0211, %while.body.i ], [ %flagContainer.i.3, %do.cond.i ]
  %currentBVHIndex.i.1 = phi i32 [ %currentBVHIndex.i.0212, %while.body.i ], [ %currentBVHIndex.i.2, %do.cond.i ]
  %currentGpuVA.i.1.i0 = phi i32 [ %currentGpuVA.i.0213.i0, %while.body.i ], [ %currentGpuVA.i.2.i0, %do.cond.i ]
  %currentGpuVA.i.1.i1 = phi i32 [ %currentGpuVA.i.0213.i1, %while.body.i ], [ %currentGpuVA.i.2.i1, %do.cond.i ]
  %instanceIndex.i.1 = phi i32 [ %instanceIndex.i.0214, %while.body.i ], [ %instanceIndex.i.2, %do.cond.i ]
  %instanceFlags.i.1 = phi i32 [ %instanceFlags.i.0215, %while.body.i ], [ %instanceFlags.i.2, %do.cond.i ]
  %instanceOffset.i.1 = phi i32 [ %instanceOffset.i.0216, %while.body.i ], [ %instanceOffset.i.2, %do.cond.i ]
  %instanceId.i.1 = phi i32 [ %instanceId.i.0217, %while.body.i ], [ %instanceId.i.2, %do.cond.i ]
  %stackPointer.i.2 = phi i32 [ %stackPointer.i.1218, %while.body.i ], [ %stackPointer.i.3, %do.cond.i ]
  %resultBary.i.1.i0 = phi float [ %resultBary.i.0220.i0, %while.body.i ], [ %resultBary.i.3.i0, %do.cond.i ]
  %resultBary.i.1.i1 = phi float [ %resultBary.i.0220.i1, %while.body.i ], [ %resultBary.i.3.i1, %do.cond.i ]
  %resultTriId.i.1 = phi i32 [ %resultTriId.i.0221, %while.body.i ], [ %resultTriId.i.3, %do.cond.i ]
  %dec.i.i = add nsw i32 %stackPointer.i.2, -1
  %mul.i.222.i = shl i32 %dec.i.i, 6
  %add.i.223.i = add i32 %mul.i.222.i, %call.i
  %arrayidx.i.224.i = getelementptr [2048 x i32], [2048 x i32] addrspace(3)* @"\01?stack@@3PAIA", i32 0, i32 %add.i.223.i
  %84 = load i32, i32 addrspace(3)* %arrayidx.i.224.i, align 4, !tbaa !223, !noalias !237
  %arrayidx13.i = getelementptr inbounds [2 x i32], [2 x i32]* %nodesToProcess.i, i32 0, i32 %currentBVHIndex.i.1
  %dec.i = add i32 %83, -1
  store i32 %dec.i, i32* %arrayidx13.i, align 4, !tbaa !223
  %arrayidx.i.i.230.i = getelementptr inbounds [0 x %struct.RWByteAddressBuffer], [0 x %struct.RWByteAddressBuffer]* @"\01?DescriptorHeapBufferTable@@3PAURWByteAddressBuffer@@A", i32 0, i32 %currentGpuVA.i.1.i1, !dx.nonuniform !233
  %85 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %arrayidx.i.i.230.i, align 4, !noalias !240
  %add.i.i.242.i = add i32 %currentGpuVA.i.1.i0, 16
  %mul.i.i.243.i = shl i32 %84, 5
  %add.i.5.i.244.i = add i32 %add.i.i.242.i, %mul.i.i.243.i
  %86 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %85)
  %87 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %86, i32 %add.i.5.i.244.i, i32 undef)
  %88 = extractvalue %dx.types.ResRet.i32 %87, 3
  %add.i.247.i = add i32 %add.i.5.i.244.i, 16
  %89 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %86, i32 %add.i.247.i, i32 undef)
  %90 = extractvalue %dx.types.ResRet.i32 %89, 3
  %tobool.i.i = icmp slt i32 %88, 0
  br i1 %tobool.i.i, label %if.then.17.i, label %if.else.136.i

if.then.17.i:                                     ; preds = %do.body.i
  %cmp18.i = icmp eq i32 %currentBVHIndex.i.1, 0
  br i1 %cmp18.i, label %if.then.21.i, label %if.else.i

if.then.21.i:                                     ; preds = %if.then.17.i
  %and.i.255.i = and i32 %88, 1073741823
  %mul.i = mul i32 %and.i.255.i, 116
  %add23.i = add i32 %add5.i, %mul.i
  %91 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %43)
  %92 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %91, i32 %add23.i, i32 undef)
  %93 = extractvalue %dx.types.ResRet.i32 %92, 0
  %94 = extractvalue %dx.types.ResRet.i32 %92, 1
  %95 = extractvalue %dx.types.ResRet.i32 %92, 2
  %96 = extractvalue %dx.types.ResRet.i32 %92, 3
  %add.i.260.i = add i32 %add23.i, 16
  %97 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %91, i32 %add.i.260.i, i32 undef)
  %98 = extractvalue %dx.types.ResRet.i32 %97, 0
  %99 = extractvalue %dx.types.ResRet.i32 %97, 1
  %100 = extractvalue %dx.types.ResRet.i32 %97, 2
  %101 = extractvalue %dx.types.ResRet.i32 %97, 3
  %add.i.260.i.1 = add i32 %add23.i, 32
  %102 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %91, i32 %add.i.260.i.1, i32 undef)
  %103 = extractvalue %dx.types.ResRet.i32 %102, 0
  %104 = extractvalue %dx.types.ResRet.i32 %102, 1
  %105 = extractvalue %dx.types.ResRet.i32 %102, 2
  %106 = extractvalue %dx.types.ResRet.i32 %102, 3
  %add.i.260.i.2 = add i32 %add23.i, 48
  %107 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %91, i32 %add.i.260.i.2, i32 undef)
  %108 = extractvalue %dx.types.ResRet.i32 %107, 0
  %109 = extractvalue %dx.types.ResRet.i32 %107, 1
  %110 = extractvalue %dx.types.ResRet.i32 %107, 2
  %111 = extractvalue %dx.types.ResRet.i32 %107, 3
  %add.i.260.i.3 = add i32 %add23.i, 64
  %112 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %91, i32 %add.i.260.i.3, i32 undef)
  %add.i.260.i.4 = add i32 %add23.i, 80
  %113 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %91, i32 %add.i.260.i.4, i32 undef)
  %add.i.260.i.5 = add i32 %add23.i, 96
  %114 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %91, i32 %add.i.260.i.5, i32 undef)
  %add.i.260.i.6 = add i32 %add23.i, 112
  %115 = bitcast i32 %93 to float
  %116 = bitcast i32 %94 to float
  %117 = bitcast i32 %95 to float
  %118 = bitcast i32 %96 to float
  %119 = bitcast i32 %98 to float
  %120 = bitcast i32 %99 to float
  %121 = bitcast i32 %100 to float
  %122 = bitcast i32 %101 to float
  %123 = bitcast i32 %103 to float
  %124 = bitcast i32 %104 to float
  %125 = bitcast i32 %105 to float
  %126 = bitcast i32 %106 to float
  %127 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %43)
  %128 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %127, i32 %add.i.260.i.6, i32 undef)
  %129 = extractvalue %dx.types.ResRet.i32 %128, 0
  %and.i.263.i = and i32 %109, 16777215
  %and.i.264.i = and i32 %108, 16777215
  %shr.i.i = lshr i32 %108, 24
  %and.i = and i32 %shr.i.i, %1
  %tobool29.i = icmp eq i32 %and.i, 0
  br i1 %tobool29.i, label %do.cond.i, label %if.then.31.i

if.then.31.i:                                     ; preds = %if.then.21.i
  %130 = extractvalue %dx.types.ResRet.i32 %114, 3
  %131 = bitcast i32 %130 to float
  %132 = extractvalue %dx.types.ResRet.i32 %114, 2
  %133 = bitcast i32 %132 to float
  %134 = extractvalue %dx.types.ResRet.i32 %114, 1
  %135 = bitcast i32 %134 to float
  %136 = extractvalue %dx.types.ResRet.i32 %114, 0
  %137 = bitcast i32 %136 to float
  %138 = extractvalue %dx.types.ResRet.i32 %113, 3
  %139 = bitcast i32 %138 to float
  %140 = extractvalue %dx.types.ResRet.i32 %113, 2
  %141 = bitcast i32 %140 to float
  %142 = extractvalue %dx.types.ResRet.i32 %113, 1
  %143 = bitcast i32 %142 to float
  %144 = extractvalue %dx.types.ResRet.i32 %113, 0
  %145 = bitcast i32 %144 to float
  %146 = extractvalue %dx.types.ResRet.i32 %112, 3
  %147 = bitcast i32 %146 to float
  %148 = extractvalue %dx.types.ResRet.i32 %112, 2
  %149 = bitcast i32 %148 to float
  %150 = extractvalue %dx.types.ResRet.i32 %112, 1
  %151 = bitcast i32 %150 to float
  %152 = extractvalue %dx.types.ResRet.i32 %112, 0
  %153 = bitcast i32 %152 to float
  store i32 0, i32 addrspace(3)* %arrayidx.i.224.i, align 4, !tbaa !223, !noalias !245
  %shr.i.275.i = lshr i32 %109, 24
  %154 = fmul fast float %WorldRayDirection298, %115
  %FMad327 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayDirection299, float %116, float %154)
  %FMad326 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayDirection300, float %117, float %FMad327)
  %FMad325 = call float @dx.op.tertiary.f32(i32 46, float 0.000000e+00, float %118, float %FMad326)
  %155 = fmul fast float %WorldRayDirection298, %119
  %FMad324 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayDirection299, float %120, float %155)
  %FMad323 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayDirection300, float %121, float %FMad324)
  %FMad322 = call float @dx.op.tertiary.f32(i32 46, float 0.000000e+00, float %122, float %FMad323)
  %156 = fmul fast float %WorldRayDirection298, %123
  %FMad321 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayDirection299, float %124, float %156)
  %FMad320 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayDirection300, float %125, float %FMad321)
  %FMad = call float @dx.op.tertiary.f32(i32 46, float 0.000000e+00, float %126, float %FMad320)
  %157 = fmul fast float %WorldRayOrigin295, %115
  %FMad336 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayOrigin296, float %116, float %157)
  %FMad335 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayOrigin297, float %117, float %FMad336)
  %FMad334 = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %118, float %FMad335)
  %158 = insertelement <3 x float> undef, float %FMad334, i64 0
  %159 = fmul fast float %WorldRayOrigin295, %119
  %FMad333 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayOrigin296, float %120, float %159)
  %FMad332 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayOrigin297, float %121, float %FMad333)
  %FMad331 = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %122, float %FMad332)
  %160 = insertelement <3 x float> %158, float %FMad331, i64 1
  %161 = fmul fast float %WorldRayOrigin295, %123
  %FMad330 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayOrigin296, float %124, float %161)
  %FMad329 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayOrigin297, float %125, float %FMad330)
  %FMad328 = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %126, float %FMad329)
  %162 = insertelement <3 x float> %160, float %FMad328, i64 2
  store float %FMad325, float* %67, align 4
  store float %FMad322, float* %68, align 4
  store float %FMad, float* %69, align 4
  %163 = insertelement <3 x float> undef, float %FMad325, i64 0
  %164 = insertelement <3 x float> %163, float %FMad322, i64 1
  %165 = insertelement <3 x float> %164, float %FMad, i64 2
  %.i0364 = fdiv fast float 1.000000e+00, %FMad325
  %.i1365 = fdiv fast float 1.000000e+00, %FMad322
  %.i2366 = fdiv fast float 1.000000e+00, %FMad
  %mul.i.292.i.i0 = fmul fast float %.i0364, %FMad334
  %mul.i.292.i.i1 = fmul fast float %.i1365, %FMad331
  %mul.i.292.i.i2 = fmul fast float %.i2366, %FMad328
  %FAbs = call float @dx.op.unary.f32(i32 6, float %FMad325)
  %FAbs259 = call float @dx.op.unary.f32(i32 6, float %FMad322)
  %FAbs260 = call float @dx.op.unary.f32(i32 6, float %FMad)
  %cmp.i.i.294.i = fcmp fast ogt float %FAbs, %FAbs259
  %cmp1.i.i.295.i = fcmp fast ogt float %FAbs, %FAbs260
  %166 = and i1 %cmp.i.i.294.i, %cmp1.i.i.295.i
  br i1 %166, label %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.308.i", label %if.else.i.i.298.i

if.else.i.i.298.i:                                ; preds = %if.then.31.i
  %cmp4.i.i.297.i = fcmp fast ogt float %FAbs259, %FAbs260
  br i1 %cmp4.i.i.297.i, label %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.308.i", label %if.else.8.i.i.300.i

if.else.8.i.i.300.i:                              ; preds = %if.else.i.i.298.i
  br label %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.308.i"

"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.308.i": ; preds = %if.else.8.i.i.300.i, %if.else.i.i.298.i, %if.then.31.i
  %retval.i.i.283.i.0 = phi i32 [ 2, %if.else.8.i.i.300.i ], [ 0, %if.then.31.i ], [ 1, %if.else.i.i.298.i ]
  %add.i.301.i = add nuw nsw i32 %retval.i.i.283.i.0, 1
  %rem.i.302.i = urem i32 %add.i.301.i, 3
  %add4.i.303.i = add nuw nsw i32 %retval.i.i.283.i.0, 2
  %rem5.i.304.i = urem i32 %add4.i.303.i, 3
  %167 = getelementptr [3 x float], [3 x float]* %22, i32 0, i32 %retval.i.i.283.i.0
  %168 = load float, float* %167, align 4, !tbaa !228, !noalias !248
  %cmp.i.307.i = fcmp fast olt float %168, 0.000000e+00
  %tmp.i.5.0.i0 = select i1 %cmp.i.307.i, i32 %rem5.i.304.i, i32 %rem.i.302.i
  %tmp.i.5.0.i1 = select i1 %cmp.i.307.i, i32 %rem.i.302.i, i32 %rem5.i.304.i
  %169 = getelementptr [3 x float], [3 x float]* %22, i32 0, i32 %tmp.i.5.0.i0
  %170 = load float, float* %169, align 4, !tbaa !228, !noalias !248
  %div.i.314.i = fdiv float %170, %168
  %171 = getelementptr [3 x float], [3 x float]* %22, i32 0, i32 %tmp.i.5.0.i1
  %172 = load float, float* %171, align 4, !tbaa !228, !noalias !248
  %div14.i.317.i = fdiv float %172, %168
  %div16.i.319.i = fdiv float 1.000000e+00, %168
  call void @"\01?Fallback_SetObjectRayOrigin@@YAXV?$vector@M$02@@@Z"(<3 x float> %162) #2
  call void @"\01?Fallback_SetObjectRayDirection@@YAXV?$vector@M$02@@@Z"(<3 x float> %165) #2
  %.0.vec.extract5.upto0 = insertelement <4 x float> undef, float %115, i32 0
  %.0.vec.extract5.upto1 = insertelement <4 x float> %.0.vec.extract5.upto0, float %116, i32 1
  %.0.vec.extract5.upto2 = insertelement <4 x float> %.0.vec.extract5.upto1, float %117, i32 2
  %.0.vec.extract5 = insertelement <4 x float> %.0.vec.extract5.upto2, float %118, i32 3
  %.fca.0.0.insert1 = insertvalue %class.matrix.float.3.4 undef, <4 x float> %.0.vec.extract5, 0, 0
  %.16.vec.extract7.upto0 = insertelement <4 x float> undef, float %119, i32 0
  %.16.vec.extract7.upto1 = insertelement <4 x float> %.16.vec.extract7.upto0, float %120, i32 1
  %.16.vec.extract7.upto2 = insertelement <4 x float> %.16.vec.extract7.upto1, float %121, i32 2
  %.16.vec.extract7 = insertelement <4 x float> %.16.vec.extract7.upto2, float %122, i32 3
  %.fca.0.1.insert2 = insertvalue %class.matrix.float.3.4 %.fca.0.0.insert1, <4 x float> %.16.vec.extract7, 0, 1
  %.32.vec.extract9.upto0 = insertelement <4 x float> undef, float %123, i32 0
  %.32.vec.extract9.upto1 = insertelement <4 x float> %.32.vec.extract9.upto0, float %124, i32 1
  %.32.vec.extract9.upto2 = insertelement <4 x float> %.32.vec.extract9.upto1, float %125, i32 2
  %.32.vec.extract9 = insertelement <4 x float> %.32.vec.extract9.upto2, float %126, i32 3
  %.fca.0.2.insert3 = insertvalue %class.matrix.float.3.4 %.fca.0.1.insert2, <4 x float> %.32.vec.extract9, 0, 2
  call void @"\01?Fallback_SetWorldToObject@@YAXV?$matrix@M$02$03@@@Z"(%class.matrix.float.3.4 %.fca.0.2.insert3) #2
  %.0.vec.extract.upto0 = insertelement <4 x float> undef, float %153, i32 0
  %.0.vec.extract.upto1 = insertelement <4 x float> %.0.vec.extract.upto0, float %151, i32 1
  %.0.vec.extract.upto2 = insertelement <4 x float> %.0.vec.extract.upto1, float %149, i32 2
  %.0.vec.extract = insertelement <4 x float> %.0.vec.extract.upto2, float %147, i32 3
  %.fca.0.0.insert = insertvalue %class.matrix.float.3.4 undef, <4 x float> %.0.vec.extract, 0, 0
  %.16.vec.extract.upto0 = insertelement <4 x float> undef, float %145, i32 0
  %.16.vec.extract.upto1 = insertelement <4 x float> %.16.vec.extract.upto0, float %143, i32 1
  %.16.vec.extract.upto2 = insertelement <4 x float> %.16.vec.extract.upto1, float %141, i32 2
  %.16.vec.extract = insertelement <4 x float> %.16.vec.extract.upto2, float %139, i32 3
  %.fca.0.1.insert = insertvalue %class.matrix.float.3.4 %.fca.0.0.insert, <4 x float> %.16.vec.extract, 0, 1
  %.32.vec.extract.upto0 = insertelement <4 x float> undef, float %137, i32 0
  %.32.vec.extract.upto1 = insertelement <4 x float> %.32.vec.extract.upto0, float %135, i32 1
  %.32.vec.extract.upto2 = insertelement <4 x float> %.32.vec.extract.upto1, float %133, i32 2
  %.32.vec.extract = insertelement <4 x float> %.32.vec.extract.upto2, float %131, i32 3
  %.fca.0.2.insert = insertvalue %class.matrix.float.3.4 %.fca.0.1.insert, <4 x float> %.32.vec.extract, 0, 2
  call void @"\01?Fallback_SetObjectToWorld@@YAXV?$matrix@M$02$03@@@Z"(%class.matrix.float.3.4 %.fca.0.2.insert) #2
  store i32 1, i32* %arrayidx42.i, align 4, !tbaa !223
  br label %do.cond.i

if.else.i:                                        ; preds = %if.then.17.i
  %RayFlags257 = call i32 @dx.op.rayFlags.i32(i32 144)
  %173 = and i32 %instanceFlags.i.1, 12
  %174 = icmp eq i32 %173, 8
  %. = select i1 %174, i32 0, i32 1
  %and7.i.i = and i32 %RayFlags257, 1
  %tobool8.i.i = icmp eq i32 %and7.i.i, 0
  br i1 %tobool8.i.i, label %if.else.10.i.i, label %"\01?IsOpaque@@YA_N_NII@Z.exit.i"

if.else.10.i.i:                                   ; preds = %if.else.i
  %and11.i.i = and i32 %RayFlags257, 2
  %tobool12.i.i = icmp eq i32 %and11.i.i, 0
  %.. = select i1 %tobool12.i.i, i32 %., i32 0
  br label %"\01?IsOpaque@@YA_N_NII@Z.exit.i"

"\01?IsOpaque@@YA_N_NII@Z.exit.i":                ; preds = %if.else.10.i.i, %if.else.i
  %opaque.i.i.1 = phi i32 [ 1, %if.else.i ], [ %.., %if.else.10.i.i ]
  %tobool16.i.i = icmp ne i32 %opaque.i.i.1, 0
  %and.i.328.i = and i32 %RayFlags257, 64
  %tobool1.i.i = icmp ne i32 %and.i.328.i, 0
  %175 = and i1 %tobool16.i.i, %tobool1.i.i
  %lnot.i.i = xor i1 %tobool16.i.i, true
  %and3.i.330.i = and i32 %RayFlags257, 128
  %tobool4.i.331.i = icmp ne i32 %and3.i.330.i, 0
  %176 = and i1 %tobool4.i.331.i, %lnot.i.i
  %177 = or i1 %175, %176
  %call56.i = call float @"\01?Fallback_RayTCurrent@@YAMXZ"() #2
  %and.i.333.i = and i32 %88, 1073741824
  %tobool.i.334.i = icmp ne i32 %and.i.333.i, 0
  %178 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %arrayidx.i.i.230.i, align 4, !noalias !251
  %lnot.i = xor i1 %177, true
  %179 = and i1 %tobool.i.334.i, %lnot.i
  br i1 %179, label %if.then.68.i, label %if.else.83.i

if.then.68.i:                                     ; preds = %"\01?IsOpaque@@YA_N_NII@Z.exit.i"
  %add.i.i.348.i = add i32 %currentGpuVA.i.1.i0, 8
  %180 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %178)
  %181 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %180, i32 %add.i.i.348.i, i32 undef)
  %182 = extractvalue %dx.types.ResRet.i32 %181, 0
  %mul.i.i.349.i = shl i32 %88, 3
  %add2.i.i.i = add i32 %mul.i.i.349.i, %currentGpuVA.i.1.i0
  %add.i.3.i.i = add i32 %add2.i.i.i, %182
  %183 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %180, i32 %add.i.3.i.i, i32 undef)
  %184 = extractvalue %dx.types.ResRet.i32 %183, 0
  %185 = extractvalue %dx.types.ResRet.i32 %183, 1
  %186 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %Constants337, i32 0)
  %187 = extractvalue %dx.types.CBufRet.i32 %186, 2
  %mul74.i = mul i32 %184, %3
  %add75.i = add i32 %instanceOffset.i.1, %2
  %add76.i = add i32 %add75.i, %mul74.i
  %mul77.i = mul i32 %add76.i, %187
  call void @"\01?Fallback_SetPendingCustomVals@@YAXIIII@Z"(i32 %mul77.i, i32 %185, i32 %instanceIndex.i.1, i32 %instanceId.i.1) #2
  %add.i.351.i = add i32 %mul77.i, 4
  %188 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.ByteAddressBuffer(i32 160, %struct.ByteAddressBuffer %15)
  %189 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %188, i32 %add.i.351.i, i32 undef)
  %190 = extractvalue %dx.types.ResRet.i32 %189, 0
  %191 = extractvalue %dx.types.ResRet.i32 %189, 1
  call void @"\01?Fallback_SetAnyHitStateId@@YAXH@Z"(i32 %190) #2
  call void @"\01?Fallback_SetAnyHitResult@@YAXH@Z"(i32 1) #2
  call void @"\01?Fallback_CallIndirect@@YAXH@Z"(i32 %191) #2
  %call80.i = call i32 @"\01?Fallback_AnyHitResult@@YAHXZ"() #2
  %cmp81.i = icmp eq i32 %call80.i, -1
  br i1 %cmp81.i, label %if.then.i.355.i, label %if.else.i.357.i

if.then.i.355.i:                                  ; preds = %if.then.68.i
  %or.i.i = or i32 %flagContainer.i.1, 1
  br label %if.end.129.i

if.else.i.357.i:                                  ; preds = %if.then.68.i
  %and.i.356.i = and i32 %flagContainer.i.1, -2
  br label %if.end.129.i

if.else.83.i:                                     ; preds = %"\01?IsOpaque@@YA_N_NII@Z.exit.i"
  %and.i.362.i = and i32 %88, 16777215
  %add.i.i.i.i = add i32 %currentGpuVA.i.1.i0, 4
  %192 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %85)
  %193 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %192, i32 %add.i.i.i.i, i32 undef)
  %194 = extractvalue %dx.types.ResRet.i32 %193, 0
  %mul.i.i.364.i = mul nuw nsw i32 %and.i.362.i, 40
  %add2.i.i.i.i = add i32 %mul.i.i.364.i, %currentGpuVA.i.1.i0
  %add.i.i.365.i = add i32 %add2.i.i.i.i, %194
  %add1.i.i.i = add i32 %add.i.i.365.i, 4
  %195 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %192, i32 %add1.i.i.i, i32 undef)
  %196 = extractvalue %dx.types.ResRet.i32 %195, 0
  %197 = extractvalue %dx.types.ResRet.i32 %195, 1
  %198 = extractvalue %dx.types.ResRet.i32 %195, 2
  %199 = extractvalue %dx.types.ResRet.i32 %195, 3
  %.i0367 = bitcast i32 %196 to float
  %.i1368 = bitcast i32 %197 to float
  %.i2369 = bitcast i32 %198 to float
  %.i3370 = bitcast i32 %199 to float
  %add5.i.i.i = add i32 %add.i.i.365.i, 20
  %200 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %192, i32 %add5.i.i.i, i32 undef)
  %201 = extractvalue %dx.types.ResRet.i32 %200, 0
  %202 = extractvalue %dx.types.ResRet.i32 %200, 1
  %203 = extractvalue %dx.types.ResRet.i32 %200, 2
  %204 = extractvalue %dx.types.ResRet.i32 %200, 3
  %.i0371 = bitcast i32 %201 to float
  %.i1372 = bitcast i32 %202 to float
  %.i2373 = bitcast i32 %203 to float
  %.i3374 = bitcast i32 %204 to float
  %add9.i.i.i = add i32 %add.i.i.365.i, 36
  %205 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %192, i32 %add9.i.i.i, i32 undef)
  %206 = extractvalue %dx.types.ResRet.i32 %205, 0
  %207 = bitcast i32 %206 to float
  %and.i.i.i = and i32 %instanceFlags.i.1, 1
  %lnot.i.i.i = icmp eq i32 %and.i.i.i, 0
  %and1.i.i.i = shl i32 %instanceFlags.i.1, 3
  %208 = and i32 %and1.i.i.i, 16
  %209 = add nuw nsw i32 %208, 16
  %210 = xor i32 %208, 16
  %211 = add nuw nsw i32 %210, 16
  %and8.i.i.i = and i32 %RayFlags257, %209
  %tobool9.i.i.i = icmp ne i32 %and8.i.i.i, 0
  %212 = and i1 %lnot.i.i.i, %tobool9.i.i.i
  %and13.i.i.i = and i32 %RayFlags257, %211
  %tobool14.i.i.i = icmp ne i32 %and13.i.i.i, 0
  %213 = and i1 %lnot.i.i.i, %tobool14.i.i.i
  %sub.i.i.i.i0 = fsub fast float %.i0367, %currentRayData.i.0.1.i0
  %sub.i.i.i.i1 = fsub fast float %.i1368, %currentRayData.i.0.1.i1
  %sub.i.i.i.i2 = fsub fast float %.i2369, %currentRayData.i.0.1.i2
  store float %sub.i.i.i.i0, float* %70, align 4
  store float %sub.i.i.i.i1, float* %71, align 4
  store float %sub.i.i.i.i2, float* %72, align 4
  %214 = getelementptr [3 x float], [3 x float]* %21, i32 0, i32 %currentRayData.i.5.2.i0
  %215 = load float, float* %214, align 4, !tbaa !228, !noalias !256
  %216 = getelementptr [3 x float], [3 x float]* %21, i32 0, i32 %currentRayData.i.5.2.i1
  %217 = load float, float* %216, align 4, !tbaa !228, !noalias !256
  %218 = getelementptr [3 x float], [3 x float]* %21, i32 0, i32 %currentRayData.i.5.2.i2
  %219 = load float, float* %218, align 4, !tbaa !228, !noalias !256
  %sub17.i.i.i.i0 = fsub fast float %.i3370, %currentRayData.i.0.1.i0
  %sub17.i.i.i.i1 = fsub fast float %.i0371, %currentRayData.i.0.1.i1
  %sub17.i.i.i.i2 = fsub fast float %.i1372, %currentRayData.i.0.1.i2
  store float %sub17.i.i.i.i0, float* %73, align 4
  store float %sub17.i.i.i.i1, float* %74, align 4
  store float %sub17.i.i.i.i2, float* %75, align 4
  %220 = getelementptr [3 x float], [3 x float]* %19, i32 0, i32 %currentRayData.i.5.2.i0
  %221 = load float, float* %220, align 4, !tbaa !228, !noalias !256
  %222 = getelementptr [3 x float], [3 x float]* %19, i32 0, i32 %currentRayData.i.5.2.i1
  %223 = load float, float* %222, align 4, !tbaa !228, !noalias !256
  %224 = getelementptr [3 x float], [3 x float]* %19, i32 0, i32 %currentRayData.i.5.2.i2
  %225 = load float, float* %224, align 4, !tbaa !228, !noalias !256
  %sub19.i.i.i.i0 = fsub fast float %.i2373, %currentRayData.i.0.1.i0
  %sub19.i.i.i.i1 = fsub fast float %.i3374, %currentRayData.i.0.1.i1
  %sub19.i.i.i.i2 = fsub fast float %207, %currentRayData.i.0.1.i2
  store float %sub19.i.i.i.i0, float* %76, align 4
  store float %sub19.i.i.i.i1, float* %77, align 4
  store float %sub19.i.i.i.i2, float* %78, align 4
  %226 = getelementptr [3 x float], [3 x float]* %20, i32 0, i32 %currentRayData.i.5.2.i0
  %227 = load float, float* %226, align 4, !tbaa !228, !noalias !256
  %228 = getelementptr [3 x float], [3 x float]* %20, i32 0, i32 %currentRayData.i.5.2.i1
  %229 = load float, float* %228, align 4, !tbaa !228, !noalias !256
  %230 = getelementptr [3 x float], [3 x float]* %20, i32 0, i32 %currentRayData.i.5.2.i2
  %231 = load float, float* %230, align 4, !tbaa !228, !noalias !256
  %mul.i.5.i.i.i0 = fmul float %currentRayData.i.4.1.i0, %219
  %mul.i.5.i.i.i1 = fmul float %currentRayData.i.4.1.i1, %219
  %sub21.i.i.i.i0 = fsub float %215, %mul.i.5.i.i.i0
  %sub21.i.i.i.i1 = fsub float %217, %mul.i.5.i.i.i1
  %mul24.i.i.i.i0 = fmul float %currentRayData.i.4.1.i0, %225
  %mul24.i.i.i.i1 = fmul float %currentRayData.i.4.1.i1, %225
  %sub25.i.i.i.i0 = fsub float %221, %mul24.i.i.i.i0
  %sub25.i.i.i.i1 = fsub float %223, %mul24.i.i.i.i1
  %mul28.i.i.i.i0 = fmul float %currentRayData.i.4.1.i0, %231
  %mul28.i.i.i.i1 = fmul float %currentRayData.i.4.1.i1, %231
  %sub29.i.i.i.i0 = fsub float %227, %mul28.i.i.i.i0
  %sub29.i.i.i.i1 = fsub float %229, %mul28.i.i.i.i1
  %mul30.i.i.i = fmul float %sub25.i.i.i.i1, %sub29.i.i.i.i0
  %mul31.i.i.i = fmul float %sub25.i.i.i.i0, %sub29.i.i.i.i1
  %sub32.i.i.i = fsub float %mul30.i.i.i, %mul31.i.i.i
  %mul33.i.i.i = fmul float %sub21.i.i.i.i0, %sub29.i.i.i.i1
  %mul34.i.i.i = fmul float %sub21.i.i.i.i1, %sub29.i.i.i.i0
  %sub35.i.i.i = fsub float %mul33.i.i.i, %mul34.i.i.i
  %mul36.i.i.i = fmul float %sub21.i.i.i.i1, %sub25.i.i.i.i0
  %mul37.i.i.i = fmul float %sub21.i.i.i.i0, %sub25.i.i.i.i1
  %sub38.i.i.i = fsub float %mul36.i.i.i, %mul37.i.i.i
  %add.i.6.i.i = fadd fast float %sub35.i.i.i, %sub38.i.i.i
  %add39.i.i.i = fadd fast float %add.i.6.i.i, %sub32.i.i.i
  br i1 %213, label %if.then.i.i.368.i, label %if.else.i.i.369.i

if.then.i.i.368.i:                                ; preds = %if.else.83.i
  %cmp.i.i.367.i = fcmp fast ogt float %sub32.i.i.i, 0.000000e+00
  %cmp42.i.i.i = fcmp fast ogt float %sub35.i.i.i, 0.000000e+00
  %232 = or i1 %cmp.i.i.367.i, %cmp42.i.i.i
  %cmp44.i.i.i = fcmp fast ogt float %sub38.i.i.i, 0.000000e+00
  %233 = or i1 %cmp44.i.i.i, %232
  %cmp77.i.i.i.old.old = fcmp fast oeq float %add39.i.i.i, 0.000000e+00
  %or.cond224 = or i1 %233, %cmp77.i.i.i.old.old
  br i1 %or.cond224, label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i", label %if.end.81.i.i.i

if.else.i.i.369.i:                                ; preds = %if.else.83.i
  br i1 %212, label %if.then.49.i.i.i, label %if.else.59.i.i.i

if.then.49.i.i.i:                                 ; preds = %if.else.i.i.369.i
  %cmp50.i.i.i = fcmp fast olt float %sub32.i.i.i, 0.000000e+00
  %cmp52.i.i.i = fcmp fast olt float %sub35.i.i.i, 0.000000e+00
  %234 = or i1 %cmp50.i.i.i, %cmp52.i.i.i
  %cmp54.i.i.i = fcmp fast olt float %sub38.i.i.i, 0.000000e+00
  %235 = or i1 %cmp54.i.i.i, %234
  %cmp77.i.i.i.old = fcmp fast oeq float %add39.i.i.i, 0.000000e+00
  %or.cond223 = or i1 %235, %cmp77.i.i.i.old
  br i1 %or.cond223, label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i", label %if.end.81.i.i.i

if.else.59.i.i.i:                                 ; preds = %if.else.i.i.369.i
  %cmp60.i.i.i = fcmp fast olt float %sub32.i.i.i, 0.000000e+00
  %cmp62.i.i.i = fcmp fast olt float %sub35.i.i.i, 0.000000e+00
  %236 = or i1 %cmp60.i.i.i, %cmp62.i.i.i
  %cmp64.i.i.i = fcmp fast olt float %sub38.i.i.i, 0.000000e+00
  %237 = or i1 %cmp64.i.i.i, %236
  %cmp66.i.i.i = fcmp fast ogt float %sub32.i.i.i, 0.000000e+00
  %cmp68.i.i.i = fcmp fast ogt float %sub35.i.i.i, 0.000000e+00
  %238 = or i1 %cmp66.i.i.i, %cmp68.i.i.i
  %cmp70.i.i.i = fcmp fast ogt float %sub38.i.i.i, 0.000000e+00
  %239 = or i1 %cmp70.i.i.i, %238
  %240 = and i1 %237, %239
  %cmp77.i.i.i = fcmp fast oeq float %add39.i.i.i, 0.000000e+00
  %or.cond222 = or i1 %cmp77.i.i.i, %240
  br i1 %or.cond222, label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i", label %if.end.81.i.i.i

if.end.81.i.i.i:                                  ; preds = %if.else.59.i.i.i, %if.then.49.i.i.i, %if.then.i.i.368.i
  %mul85.i.i.i = fmul fast float %sub32.i.i.i, %219
  %mul86.i.i.i = fmul fast float %sub35.i.i.i, %225
  %mul88.i.i.i = fmul fast float %sub38.i.i.i, %231
  %tmp = fadd fast float %mul86.i.i.i, %mul88.i.i.i
  %tmp432 = fadd fast float %tmp, %mul85.i.i.i
  %tmp433 = fmul fast float %tmp432, %currentRayData.i.4.1.i2
  br i1 %213, label %if.then.91.i.i.i, label %if.else.100.i.i.i

if.then.91.i.i.i:                                 ; preds = %if.end.81.i.i.i
  %cmp92.i.i.i = fcmp fast ogt float %tmp433, 0.000000e+00
  %mul94.i.i.i = fmul fast float %add39.i.i.i, %call56.i
  %cmp95.i.i.i = fcmp fast olt float %tmp433, %mul94.i.i.i
  %241 = or i1 %cmp92.i.i.i, %cmp95.i.i.i
  br i1 %241, label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i", label %if.end.133.i.i.i

if.else.100.i.i.i:                                ; preds = %if.end.81.i.i.i
  br i1 %212, label %if.then.102.i.i.i, label %if.else.111.i.i.i

if.then.102.i.i.i:                                ; preds = %if.else.100.i.i.i
  %cmp103.i.i.i = fcmp fast olt float %tmp433, 0.000000e+00
  %mul105.i.i.i = fmul fast float %add39.i.i.i, %call56.i
  %cmp106.i.i.i = fcmp fast ogt float %tmp433, %mul105.i.i.i
  %242 = or i1 %cmp103.i.i.i, %cmp106.i.i.i
  br i1 %242, label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i", label %if.end.133.i.i.i

if.else.111.i.i.i:                                ; preds = %if.else.100.i.i.i
  %cmp112.i.i.i = fcmp fast ogt float %add39.i.i.i, 0.000000e+00
  %cond116.i.i.i = select i1 %cmp112.i.i.i, i32 1, i32 -1
  %243 = bitcast float %tmp433 to i32
  %xor121.i.i.i = xor i32 %243, %cond116.i.i.i
  %conv122.i.i.i = uitofp i32 %xor121.i.i.i to float
  %244 = bitcast float %add39.i.i.i to i32
  %xor124.i.i.i = xor i32 %cond116.i.i.i, %244
  %conv125.i.i.i = uitofp i32 %xor124.i.i.i to float
  %mul126.i.i.i = fmul fast float %conv125.i.i.i, %call56.i
  %cmp127.i.i.i = fcmp fast ogt float %conv122.i.i.i, %mul126.i.i.i
  br i1 %cmp127.i.i.i, label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i", label %if.end.133.i.i.i

if.end.133.i.i.i:                                 ; preds = %if.else.111.i.i.i, %if.then.102.i.i.i, %if.then.91.i.i.i
  %245 = fdiv fast float 1.000000e+00, %add39.i.i.i
  %mul135.i.i.i = fmul fast float %245, %sub35.i.i.i
  %mul136.i.i.i = fmul fast float %245, %sub38.i.i.i
  %mul137.i.i.i = fmul fast float %245, %tmp433
  br label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i"

"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i": ; preds = %if.end.133.i.i.i, %if.else.111.i.i.i, %if.then.102.i.i.i, %if.then.91.i.i.i, %if.else.59.i.i.i, %if.then.49.i.i.i, %if.then.i.i.368.i
  %.2.i0 = phi float [ %.1.i0, %if.then.i.i.368.i ], [ %.1.i0, %if.then.91.i.i.i ], [ %mul135.i.i.i, %if.end.133.i.i.i ], [ %.1.i0, %if.then.102.i.i.i ], [ %.1.i0, %if.else.111.i.i.i ], [ %.1.i0, %if.then.49.i.i.i ], [ %.1.i0, %if.else.59.i.i.i ]
  %.2.i1 = phi float [ %.1.i1, %if.then.i.i.368.i ], [ %.1.i1, %if.then.91.i.i.i ], [ %mul136.i.i.i, %if.end.133.i.i.i ], [ %.1.i1, %if.then.102.i.i.i ], [ %.1.i1, %if.else.111.i.i.i ], [ %.1.i1, %if.then.49.i.i.i ], [ %.1.i1, %if.else.59.i.i.i ]
  %.0199 = phi float [ %call56.i, %if.then.i.i.368.i ], [ %call56.i, %if.then.91.i.i.i ], [ %mul137.i.i.i, %if.end.133.i.i.i ], [ %call56.i, %if.then.102.i.i.i ], [ %call56.i, %if.else.111.i.i.i ], [ %call56.i, %if.then.49.i.i.i ], [ %call56.i, %if.else.59.i.i.i ]
  %cmp.i.370.i = fcmp fast olt float %.0199, %call56.i
  %RayTMin = call float @dx.op.rayTMin.f32(i32 153)
  %cmp1.i.i = fcmp fast ogt float %.0199, %RayTMin
  %246 = and i1 %cmp.i.370.i, %cmp1.i.i
  %.10 = select i1 %246, i1 true, i1 false
  %and.i.362.i.resultTriId.i.1 = select i1 %246, i32 %and.i.362.i, i32 %resultTriId.i.1
  %.0199.call56.i = select i1 %246, float %.0199, float %call56.i
  %.2.i0.resultBary.i.1.i0 = select i1 %246, float %.2.i0, float %resultBary.i.1.i0
  %.2.i1.resultBary.i.1.i1 = select i1 %246, float %.2.i1, float %resultBary.i.1.i1
  %247 = and i1 %.10, %lnot.i
  br i1 %247, label %if.then.90.i, label %if.end.129.i

if.then.90.i:                                     ; preds = %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i"
  %.0194.upto0 = insertelement <2 x float> undef, float %.2.i0.resultBary.i.1.i0, i32 0
  %.0194 = insertelement <2 x float> %.0194.upto0, float %.2.i1.resultBary.i.1.i1, i32 1
  %add.i.i.380.i = add i32 %currentGpuVA.i.1.i0, 8
  %248 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %178)
  %249 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %248, i32 %add.i.i.380.i, i32 undef)
  %250 = extractvalue %dx.types.ResRet.i32 %249, 0
  %mul.i.i.383.i = shl i32 %and.i.362.i.resultTriId.i.1, 3
  %add2.i.i.382.i = add i32 %mul.i.i.383.i, %currentGpuVA.i.1.i0
  %add.i.3.i.384.i = add i32 %add2.i.i.382.i, %250
  %251 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %248, i32 %add.i.3.i.384.i, i32 undef)
  %252 = extractvalue %dx.types.ResRet.i32 %251, 0
  %253 = extractvalue %dx.types.ResRet.i32 %251, 1
  %254 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %Constants337, i32 0)
  %255 = extractvalue %dx.types.CBufRet.i32 %254, 2
  %mul95.i = mul i32 %252, %3
  %add96.i = add i32 %instanceOffset.i.1, %2
  %add97.i = add i32 %add96.i, %mul95.i
  %mul98.i = mul i32 %add97.i, %255
  store <2 x float> %.0194, <2 x float>* %barycentrics.i, align 4, !tbaa !220
  call void @"\01?Fallback_SetPendingAttr@@YAXUBuiltInTriangleIntersectionAttributes@@@Z"(%struct.BuiltInTriangleIntersectionAttributes* nonnull %attr.i) #2
  call void @"\01?Fallback_SetPendingTriVals@@YAXIIIIMI@Z"(i32 %mul98.i, i32 %253, i32 %instanceIndex.i.1, i32 %instanceId.i.1, float %.0199.call56.i, i32 254) #2
  br i1 %tobool16.i.i, label %if.then.105.i, label %if.else.109.i

if.then.105.i:                                    ; preds = %if.then.90.i
  call void @"\01?Fallback_CommitHit@@YAXXZ"() #2
  %and107.i = and i32 %RayFlags257, 4
  %tobool108.i = icmp eq i32 %and107.i, 0
  br i1 %tobool108.i, label %if.else.i.396.i, label %if.then.i.393.i

if.then.i.393.i:                                  ; preds = %if.then.105.i
  %or.i.392.i = or i32 %flagContainer.i.1, 1
  br label %if.end.129.i

if.else.i.396.i:                                  ; preds = %if.then.105.i
  %and.i.395.i = and i32 %flagContainer.i.1, -2
  br label %if.end.129.i

if.else.109.i:                                    ; preds = %if.then.90.i
  %add.i.399.i = add i32 %mul98.i, 4
  %256 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.ByteAddressBuffer(i32 160, %struct.ByteAddressBuffer %15)
  %257 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %256, i32 %add.i.399.i, i32 undef)
  %258 = extractvalue %dx.types.ResRet.i32 %257, 0
  %tobool113.i = icmp eq i32 %258, 0
  br i1 %tobool113.i, label %if.end.116.i, label %if.then.114.i

if.then.114.i:                                    ; preds = %if.else.109.i
  call void @"\01?Fallback_SetAnyHitResult@@YAXH@Z"(i32 1) #2
  call void @"\01?Fallback_CallIndirect@@YAXH@Z"(i32 %258) #2
  %call.i.i = call i32 @"\01?Fallback_AnyHitResult@@YAHXZ"() #2
  br label %if.end.116.i

if.end.116.i:                                     ; preds = %if.then.114.i, %if.else.109.i
  %ret.i.0 = phi i32 [ %call.i.i, %if.then.114.i ], [ 1, %if.else.109.i ]
  %cmp117.i = icmp eq i32 %ret.i.0, 0
  br i1 %cmp117.i, label %if.end.121.i, label %if.then.120.i

if.then.120.i:                                    ; preds = %if.end.116.i
  call void @"\01?Fallback_CommitHit@@YAXXZ"() #2
  br label %if.end.121.i

if.end.121.i:                                     ; preds = %if.then.120.i, %if.end.116.i
  %cmp122.i = icmp eq i32 %ret.i.0, -1
  %and125.i = and i32 %RayFlags257, 4
  %tobool126.i = icmp ne i32 %and125.i, 0
  %259 = or i1 %cmp122.i, %tobool126.i
  br i1 %259, label %if.then.i.405.i, label %if.else.i.408.i

if.then.i.405.i:                                  ; preds = %if.end.121.i
  %or.i.404.i = or i32 %flagContainer.i.1, 1
  br label %if.end.129.i

if.else.i.408.i:                                  ; preds = %if.end.121.i
  %and.i.407.i = and i32 %flagContainer.i.1, -2
  br label %if.end.129.i

if.end.129.i:                                     ; preds = %if.else.i.408.i, %if.then.i.405.i, %if.else.i.396.i, %if.then.i.393.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i", %if.else.i.357.i, %if.then.i.355.i
  %.3.i0 = phi float [ %.2.i0, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i" ], [ %.1.i0, %if.else.i.357.i ], [ %.1.i0, %if.then.i.355.i ], [ %.2.i0, %if.else.i.396.i ], [ %.2.i0, %if.then.i.393.i ], [ %.2.i0, %if.else.i.408.i ], [ %.2.i0, %if.then.i.405.i ]
  %.3.i1 = phi float [ %.2.i1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i" ], [ %.1.i1, %if.else.i.357.i ], [ %.1.i1, %if.then.i.355.i ], [ %.2.i1, %if.else.i.396.i ], [ %.2.i1, %if.then.i.393.i ], [ %.2.i1, %if.else.i.408.i ], [ %.2.i1, %if.then.i.405.i ]
  %flagContainer.i.2 = phi i32 [ %flagContainer.i.1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i" ], [ %and.i.356.i, %if.else.i.357.i ], [ %or.i.i, %if.then.i.355.i ], [ %and.i.395.i, %if.else.i.396.i ], [ %or.i.392.i, %if.then.i.393.i ], [ %and.i.407.i, %if.else.i.408.i ], [ %or.i.404.i, %if.then.i.405.i ]
  %resultBary.i.2.i0 = phi float [ %.2.i0.resultBary.i.1.i0, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i" ], [ %resultBary.i.1.i0, %if.else.i.357.i ], [ %resultBary.i.1.i0, %if.then.i.355.i ], [ %.2.i0.resultBary.i.1.i0, %if.else.i.396.i ], [ %.2.i0.resultBary.i.1.i0, %if.then.i.393.i ], [ %.2.i0.resultBary.i.1.i0, %if.else.i.408.i ], [ %.2.i0.resultBary.i.1.i0, %if.then.i.405.i ]
  %resultBary.i.2.i1 = phi float [ %.2.i1.resultBary.i.1.i1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i" ], [ %resultBary.i.1.i1, %if.else.i.357.i ], [ %resultBary.i.1.i1, %if.then.i.355.i ], [ %.2.i1.resultBary.i.1.i1, %if.else.i.396.i ], [ %.2.i1.resultBary.i.1.i1, %if.then.i.393.i ], [ %.2.i1.resultBary.i.1.i1, %if.else.i.408.i ], [ %.2.i1.resultBary.i.1.i1, %if.then.i.405.i ]
  %resultTriId.i.2 = phi i32 [ %and.i.362.i.resultTriId.i.1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i" ], [ %resultTriId.i.1, %if.else.i.357.i ], [ %resultTriId.i.1, %if.then.i.355.i ], [ %and.i.362.i.resultTriId.i.1, %if.else.i.396.i ], [ %and.i.362.i.resultTriId.i.1, %if.then.i.393.i ], [ %and.i.362.i.resultTriId.i.1, %if.else.i.408.i ], [ %and.i.362.i.resultTriId.i.1, %if.then.i.405.i ]
  %and.i.411.i = and i32 %flagContainer.i.2, 1
  %tobool.i.412.i = icmp eq i32 %and.i.411.i, 0
  br i1 %tobool.i.412.i, label %do.cond.i, label %if.then.131.i

if.then.131.i:                                    ; preds = %if.end.129.i
  store i32 0, i32* %arrayidx42.i, align 4, !tbaa !223
  store i32 0, i32* %arrayidx.i, align 4, !tbaa !223
  br label %do.cond.i

if.else.136.i:                                    ; preds = %do.body.i
  %and.i.414.i = and i32 %88, 16777215
  %RayTCurrent = call float @dx.op.rayTCurrent.f32(i32 154)
  %mul.i.i.426.i = shl nuw nsw i32 %and.i.414.i, 5
  %add.i.5.i.427.i = add i32 %add.i.i.242.i, %mul.i.i.426.i
  %260 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %85)
  %261 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %260, i32 %add.i.5.i.427.i, i32 undef)
  %262 = extractvalue %dx.types.ResRet.i32 %261, 0
  %263 = extractvalue %dx.types.ResRet.i32 %261, 1
  %264 = extractvalue %dx.types.ResRet.i32 %261, 2
  %add.i.430.i = add i32 %add.i.5.i.427.i, 16
  %265 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %260, i32 %add.i.430.i, i32 undef)
  %266 = extractvalue %dx.types.ResRet.i32 %265, 0
  %267 = extractvalue %dx.types.ResRet.i32 %265, 1
  %268 = extractvalue %dx.types.ResRet.i32 %265, 2
  %269 = bitcast i32 %262 to float
  %270 = bitcast i32 %263 to float
  %271 = bitcast i32 %264 to float
  %272 = bitcast i32 %266 to float
  %273 = bitcast i32 %267 to float
  %274 = bitcast i32 %268 to float
  %mul.i.i.447.i = shl i32 %90, 5
  %add.i.5.i.448.i = add i32 %add.i.i.242.i, %mul.i.i.447.i
  %275 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %260, i32 %add.i.5.i.448.i, i32 undef)
  %276 = extractvalue %dx.types.ResRet.i32 %275, 0
  %277 = extractvalue %dx.types.ResRet.i32 %275, 1
  %278 = extractvalue %dx.types.ResRet.i32 %275, 2
  %add.i.451.i = add i32 %add.i.5.i.448.i, 16
  %279 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %260, i32 %add.i.451.i, i32 undef)
  %280 = extractvalue %dx.types.ResRet.i32 %279, 0
  %281 = extractvalue %dx.types.ResRet.i32 %279, 1
  %282 = extractvalue %dx.types.ResRet.i32 %279, 2
  %283 = bitcast i32 %276 to float
  %284 = bitcast i32 %277 to float
  %285 = bitcast i32 %278 to float
  %286 = bitcast i32 %280 to float
  %287 = bitcast i32 %281 to float
  %288 = bitcast i32 %282 to float
  %mul.i.468.i.i0 = fmul fast float %269, %currentRayData.i.2.1.i0
  %mul.i.468.i.i1 = fmul fast float %270, %currentRayData.i.2.1.i1
  %mul.i.468.i.i2 = fmul fast float %271, %currentRayData.i.2.1.i2
  %sub.i.469.i.i0 = fsub fast float %mul.i.468.i.i0, %currentRayData.i.3.1.i0
  %sub.i.469.i.i1 = fsub fast float %mul.i.468.i.i1, %currentRayData.i.3.1.i1
  %sub.i.469.i.i2 = fsub fast float %mul.i.468.i.i2, %currentRayData.i.3.1.i2
  %FAbs273 = call float @dx.op.unary.f32(i32 6, float %currentRayData.i.2.1.i0)
  %FAbs274 = call float @dx.op.unary.f32(i32 6, float %currentRayData.i.2.1.i1)
  %FAbs275 = call float @dx.op.unary.f32(i32 6, float %currentRayData.i.2.1.i2)
  %mul1.i.470.i.i0 = fmul fast float %FAbs273, %272
  %mul1.i.470.i.i1 = fmul fast float %FAbs274, %273
  %mul1.i.470.i.i2 = fmul fast float %FAbs275, %274
  %add.i.471.i.i0 = fadd fast float %mul1.i.470.i.i0, %sub.i.469.i.i0
  %add.i.471.i.i1 = fadd fast float %mul1.i.470.i.i1, %sub.i.469.i.i1
  %add.i.471.i.i2 = fadd fast float %mul1.i.470.i.i2, %sub.i.469.i.i2
  %sub4.i.473.i.i0 = fsub fast float %sub.i.469.i.i0, %mul1.i.470.i.i0
  %sub4.i.473.i.i1 = fsub fast float %sub.i.469.i.i1, %mul1.i.470.i.i1
  %sub4.i.473.i.i2 = fsub fast float %sub.i.469.i.i2, %mul1.i.470.i.i2
  %FMax313 = call float @dx.op.binary.f32(i32 35, float %sub4.i.473.i.i0, float %sub4.i.473.i.i1)
  %FMax312 = call float @dx.op.binary.f32(i32 35, float %FMax313, float %sub4.i.473.i.i2)
  %FMin311 = call float @dx.op.binary.f32(i32 36, float %add.i.471.i.i0, float %add.i.471.i.i1)
  %FMin310 = call float @dx.op.binary.f32(i32 36, float %FMin311, float %add.i.471.i.i2)
  %FMax308 = call float @dx.op.binary.f32(i32 35, float %FMax312, float 0.000000e+00)
  %FMin307 = call float @dx.op.binary.f32(i32 36, float %FMin310, float %RayTCurrent)
  %cmp.i.474.i = fcmp fast olt float %FMax308, %FMin307
  %mul.i.485.i.i0 = fmul fast float %283, %currentRayData.i.2.1.i0
  %mul.i.485.i.i1 = fmul fast float %284, %currentRayData.i.2.1.i1
  %mul.i.485.i.i2 = fmul fast float %285, %currentRayData.i.2.1.i2
  %sub.i.486.i.i0 = fsub fast float %mul.i.485.i.i0, %currentRayData.i.3.1.i0
  %sub.i.486.i.i1 = fsub fast float %mul.i.485.i.i1, %currentRayData.i.3.1.i1
  %sub.i.486.i.i2 = fsub fast float %mul.i.485.i.i2, %currentRayData.i.3.1.i2
  %mul1.i.487.i.i0 = fmul fast float %FAbs273, %286
  %mul1.i.487.i.i1 = fmul fast float %FAbs274, %287
  %mul1.i.487.i.i2 = fmul fast float %FAbs275, %288
  %add.i.488.i.i0 = fadd fast float %mul1.i.487.i.i0, %sub.i.486.i.i0
  %add.i.488.i.i1 = fadd fast float %mul1.i.487.i.i1, %sub.i.486.i.i1
  %add.i.488.i.i2 = fadd fast float %mul1.i.487.i.i2, %sub.i.486.i.i2
  %sub4.i.490.i.i0 = fsub fast float %sub.i.486.i.i0, %mul1.i.487.i.i0
  %sub4.i.490.i.i1 = fsub fast float %sub.i.486.i.i1, %mul1.i.487.i.i1
  %sub4.i.490.i.i2 = fsub fast float %sub.i.486.i.i2, %mul1.i.487.i.i2
  %FMax306 = call float @dx.op.binary.f32(i32 35, float %sub4.i.490.i.i0, float %sub4.i.490.i.i1)
  %FMax305 = call float @dx.op.binary.f32(i32 35, float %FMax306, float %sub4.i.490.i.i2)
  %FMin304 = call float @dx.op.binary.f32(i32 36, float %add.i.488.i.i0, float %add.i.488.i.i1)
  %FMin303 = call float @dx.op.binary.f32(i32 36, float %FMin304, float %add.i.488.i.i2)
  %FMax = call float @dx.op.binary.f32(i32 35, float %FMax305, float 0.000000e+00)
  %FMin301 = call float @dx.op.binary.f32(i32 36, float %FMin303, float %RayTCurrent)
  %cmp.i.491.i = fcmp fast olt float %FMax, %FMin301
  %289 = and i1 %cmp.i.474.i, %cmp.i.491.i
  br i1 %289, label %if.then.171.i, label %if.else.180.i

if.then.171.i:                                    ; preds = %if.else.136.i
  %cmp173.i = fcmp fast olt float %FMax, %FMax308
  %cond.i.i = select i1 %cmp173.i, i32 %and.i.414.i, i32 %90
  %cond5.i.i = select i1 %cmp173.i, i32 %90, i32 %and.i.414.i
  %mul8.i.i = shl i32 %stackPointer.i.2, 6
  %add9.i.i = add i32 %mul8.i.i, %call.i
  store i32 %cond.i.i, i32 addrspace(3)* %arrayidx.i.224.i, align 4, !tbaa !223, !noalias !264
  %arrayidx10.i.i = getelementptr [2048 x i32], [2048 x i32] addrspace(3)* @"\01?stack@@3PAIA", i32 0, i32 %add9.i.i
  store i32 %cond5.i.i, i32 addrspace(3)* %arrayidx10.i.i, align 4, !tbaa !223, !noalias !264
  %add11.i.i = add nsw i32 %stackPointer.i.2, 1
  %add179.i = add i32 %83, 1
  store i32 %add179.i, i32* %arrayidx13.i, align 4, !tbaa !223
  br label %do.cond.i

if.else.180.i:                                    ; preds = %if.else.136.i
  %290 = or i1 %cmp.i.474.i, %cmp.i.491.i
  br i1 %290, label %if.then.184.i, label %do.cond.i

if.then.184.i:                                    ; preds = %if.else.180.i
  %cond.i = select i1 %cmp.i.491.i, i32 %90, i32 %and.i.414.i
  store i32 %cond.i, i32 addrspace(3)* %arrayidx.i.224.i, align 4, !tbaa !223, !noalias !267
  store i32 %83, i32* %arrayidx13.i, align 4, !tbaa !223
  br label %do.cond.i

do.cond.i:                                        ; preds = %if.then.184.i, %if.else.180.i, %if.then.171.i, %if.then.131.i, %if.end.129.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.308.i", %if.then.21.i
  %.4.i0 = phi float [ %.1.i0, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.308.i" ], [ %.1.i0, %if.then.21.i ], [ %.3.i0, %if.then.131.i ], [ %.3.i0, %if.end.129.i ], [ %.1.i0, %if.then.171.i ], [ %.1.i0, %if.then.184.i ], [ %.1.i0, %if.else.180.i ]
  %.4.i1 = phi float [ %.1.i1, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.308.i" ], [ %.1.i1, %if.then.21.i ], [ %.3.i1, %if.then.131.i ], [ %.3.i1, %if.end.129.i ], [ %.1.i1, %if.then.171.i ], [ %.1.i1, %if.then.184.i ], [ %.1.i1, %if.else.180.i ]
  %currentRayData.i.0.2.i0 = phi float [ %FMad334, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.308.i" ], [ %currentRayData.i.0.1.i0, %if.then.21.i ], [ %currentRayData.i.0.1.i0, %if.then.131.i ], [ %currentRayData.i.0.1.i0, %if.end.129.i ], [ %currentRayData.i.0.1.i0, %if.then.171.i ], [ %currentRayData.i.0.1.i0, %if.then.184.i ], [ %currentRayData.i.0.1.i0, %if.else.180.i ]
  %currentRayData.i.0.2.i1 = phi float [ %FMad331, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.308.i" ], [ %currentRayData.i.0.1.i1, %if.then.21.i ], [ %currentRayData.i.0.1.i1, %if.then.131.i ], [ %currentRayData.i.0.1.i1, %if.end.129.i ], [ %currentRayData.i.0.1.i1, %if.then.171.i ], [ %currentRayData.i.0.1.i1, %if.then.184.i ], [ %currentRayData.i.0.1.i1, %if.else.180.i ]
  %currentRayData.i.0.2.i2 = phi float [ %FMad328, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.308.i" ], [ %currentRayData.i.0.1.i2, %if.then.21.i ], [ %currentRayData.i.0.1.i2, %if.then.131.i ], [ %currentRayData.i.0.1.i2, %if.end.129.i ], [ %currentRayData.i.0.1.i2, %if.then.171.i ], [ %currentRayData.i.0.1.i2, %if.then.184.i ], [ %currentRayData.i.0.1.i2, %if.else.180.i ]
  %currentRayData.i.2.2.i0 = phi float [ %.i0364, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.308.i" ], [ %currentRayData.i.2.1.i0, %if.then.21.i ], [ %currentRayData.i.2.1.i0, %if.then.131.i ], [ %currentRayData.i.2.1.i0, %if.end.129.i ], [ %currentRayData.i.2.1.i0, %if.then.171.i ], [ %currentRayData.i.2.1.i0, %if.then.184.i ], [ %currentRayData.i.2.1.i0, %if.else.180.i ]
  %currentRayData.i.2.2.i1 = phi float [ %.i1365, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.308.i" ], [ %currentRayData.i.2.1.i1, %if.then.21.i ], [ %currentRayData.i.2.1.i1, %if.then.131.i ], [ %currentRayData.i.2.1.i1, %if.end.129.i ], [ %currentRayData.i.2.1.i1, %if.then.171.i ], [ %currentRayData.i.2.1.i1, %if.then.184.i ], [ %currentRayData.i.2.1.i1, %if.else.180.i ]
  %currentRayData.i.2.2.i2 = phi float [ %.i2366, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.308.i" ], [ %currentRayData.i.2.1.i2, %if.then.21.i ], [ %currentRayData.i.2.1.i2, %if.then.131.i ], [ %currentRayData.i.2.1.i2, %if.end.129.i ], [ %currentRayData.i.2.1.i2, %if.then.171.i ], [ %currentRayData.i.2.1.i2, %if.then.184.i ], [ %currentRayData.i.2.1.i2, %if.else.180.i ]
  %currentRayData.i.3.2.i0 = phi float [ %mul.i.292.i.i0, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.308.i" ], [ %currentRayData.i.3.1.i0, %if.then.21.i ], [ %currentRayData.i.3.1.i0, %if.then.131.i ], [ %currentRayData.i.3.1.i0, %if.end.129.i ], [ %currentRayData.i.3.1.i0, %if.then.171.i ], [ %currentRayData.i.3.1.i0, %if.then.184.i ], [ %currentRayData.i.3.1.i0, %if.else.180.i ]
  %currentRayData.i.3.2.i1 = phi float [ %mul.i.292.i.i1, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.308.i" ], [ %currentRayData.i.3.1.i1, %if.then.21.i ], [ %currentRayData.i.3.1.i1, %if.then.131.i ], [ %currentRayData.i.3.1.i1, %if.end.129.i ], [ %currentRayData.i.3.1.i1, %if.then.171.i ], [ %currentRayData.i.3.1.i1, %if.then.184.i ], [ %currentRayData.i.3.1.i1, %if.else.180.i ]
  %currentRayData.i.3.2.i2 = phi float [ %mul.i.292.i.i2, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.308.i" ], [ %currentRayData.i.3.1.i2, %if.then.21.i ], [ %currentRayData.i.3.1.i2, %if.then.131.i ], [ %currentRayData.i.3.1.i2, %if.end.129.i ], [ %currentRayData.i.3.1.i2, %if.then.171.i ], [ %currentRayData.i.3.1.i2, %if.then.184.i ], [ %currentRayData.i.3.1.i2, %if.else.180.i ]
  %currentRayData.i.4.2.i0 = phi float [ %div.i.314.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.308.i" ], [ %currentRayData.i.4.1.i0, %if.then.21.i ], [ %currentRayData.i.4.1.i0, %if.then.131.i ], [ %currentRayData.i.4.1.i0, %if.end.129.i ], [ %currentRayData.i.4.1.i0, %if.then.171.i ], [ %currentRayData.i.4.1.i0, %if.then.184.i ], [ %currentRayData.i.4.1.i0, %if.else.180.i ]
  %currentRayData.i.4.2.i1 = phi float [ %div14.i.317.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.308.i" ], [ %currentRayData.i.4.1.i1, %if.then.21.i ], [ %currentRayData.i.4.1.i1, %if.then.131.i ], [ %currentRayData.i.4.1.i1, %if.end.129.i ], [ %currentRayData.i.4.1.i1, %if.then.171.i ], [ %currentRayData.i.4.1.i1, %if.then.184.i ], [ %currentRayData.i.4.1.i1, %if.else.180.i ]
  %currentRayData.i.4.2.i2 = phi float [ %div16.i.319.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.308.i" ], [ %currentRayData.i.4.1.i2, %if.then.21.i ], [ %currentRayData.i.4.1.i2, %if.then.131.i ], [ %currentRayData.i.4.1.i2, %if.end.129.i ], [ %currentRayData.i.4.1.i2, %if.then.171.i ], [ %currentRayData.i.4.1.i2, %if.then.184.i ], [ %currentRayData.i.4.1.i2, %if.else.180.i ]
  %currentRayData.i.5.3.i0 = phi i32 [ %tmp.i.5.0.i0, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.308.i" ], [ %currentRayData.i.5.2.i0, %if.then.21.i ], [ %currentRayData.i.5.2.i0, %if.then.131.i ], [ %currentRayData.i.5.2.i0, %if.end.129.i ], [ %currentRayData.i.5.2.i0, %if.then.171.i ], [ %currentRayData.i.5.2.i0, %if.then.184.i ], [ %currentRayData.i.5.2.i0, %if.else.180.i ]
  %currentRayData.i.5.3.i1 = phi i32 [ %tmp.i.5.0.i1, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.308.i" ], [ %currentRayData.i.5.2.i1, %if.then.21.i ], [ %currentRayData.i.5.2.i1, %if.then.131.i ], [ %currentRayData.i.5.2.i1, %if.end.129.i ], [ %currentRayData.i.5.2.i1, %if.then.171.i ], [ %currentRayData.i.5.2.i1, %if.then.184.i ], [ %currentRayData.i.5.2.i1, %if.else.180.i ]
  %currentRayData.i.5.3.i2 = phi i32 [ %retval.i.i.283.i.0, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.308.i" ], [ %currentRayData.i.5.2.i2, %if.then.21.i ], [ %currentRayData.i.5.2.i2, %if.then.131.i ], [ %currentRayData.i.5.2.i2, %if.end.129.i ], [ %currentRayData.i.5.2.i2, %if.then.171.i ], [ %currentRayData.i.5.2.i2, %if.then.184.i ], [ %currentRayData.i.5.2.i2, %if.else.180.i ]
  %flagContainer.i.3 = phi i32 [ %flagContainer.i.1, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.308.i" ], [ %flagContainer.i.1, %if.then.21.i ], [ %flagContainer.i.2, %if.then.131.i ], [ %flagContainer.i.2, %if.end.129.i ], [ %flagContainer.i.1, %if.then.171.i ], [ %flagContainer.i.1, %if.then.184.i ], [ %flagContainer.i.1, %if.else.180.i ]
  %currentBVHIndex.i.2 = phi i32 [ 1, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.308.i" ], [ 0, %if.then.21.i ], [ %currentBVHIndex.i.1, %if.then.131.i ], [ %currentBVHIndex.i.1, %if.end.129.i ], [ %currentBVHIndex.i.1, %if.then.171.i ], [ %currentBVHIndex.i.1, %if.then.184.i ], [ %currentBVHIndex.i.1, %if.else.180.i ]
  %currentGpuVA.i.2.i0 = phi i32 [ %110, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.308.i" ], [ %currentGpuVA.i.1.i0, %if.then.21.i ], [ %currentGpuVA.i.1.i0, %if.then.131.i ], [ %currentGpuVA.i.1.i0, %if.end.129.i ], [ %currentGpuVA.i.1.i0, %if.then.171.i ], [ %currentGpuVA.i.1.i0, %if.then.184.i ], [ %currentGpuVA.i.1.i0, %if.else.180.i ]
  %currentGpuVA.i.2.i1 = phi i32 [ %111, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.308.i" ], [ %currentGpuVA.i.1.i1, %if.then.21.i ], [ %currentGpuVA.i.1.i1, %if.then.131.i ], [ %currentGpuVA.i.1.i1, %if.end.129.i ], [ %currentGpuVA.i.1.i1, %if.then.171.i ], [ %currentGpuVA.i.1.i1, %if.then.184.i ], [ %currentGpuVA.i.1.i1, %if.else.180.i ]
  %instanceIndex.i.2 = phi i32 [ %129, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.308.i" ], [ %129, %if.then.21.i ], [ %instanceIndex.i.1, %if.then.131.i ], [ %instanceIndex.i.1, %if.end.129.i ], [ %instanceIndex.i.1, %if.then.171.i ], [ %instanceIndex.i.1, %if.then.184.i ], [ %instanceIndex.i.1, %if.else.180.i ]
  %instanceFlags.i.2 = phi i32 [ %shr.i.275.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.308.i" ], [ %instanceFlags.i.1, %if.then.21.i ], [ %instanceFlags.i.1, %if.then.131.i ], [ %instanceFlags.i.1, %if.end.129.i ], [ %instanceFlags.i.1, %if.then.171.i ], [ %instanceFlags.i.1, %if.then.184.i ], [ %instanceFlags.i.1, %if.else.180.i ]
  %instanceOffset.i.2 = phi i32 [ %and.i.263.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.308.i" ], [ %and.i.263.i, %if.then.21.i ], [ %instanceOffset.i.1, %if.then.131.i ], [ %instanceOffset.i.1, %if.end.129.i ], [ %instanceOffset.i.1, %if.then.171.i ], [ %instanceOffset.i.1, %if.then.184.i ], [ %instanceOffset.i.1, %if.else.180.i ]
  %instanceId.i.2 = phi i32 [ %and.i.264.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.308.i" ], [ %and.i.264.i, %if.then.21.i ], [ %instanceId.i.1, %if.then.131.i ], [ %instanceId.i.1, %if.end.129.i ], [ %instanceId.i.1, %if.then.171.i ], [ %instanceId.i.1, %if.then.184.i ], [ %instanceId.i.1, %if.else.180.i ]
  %stackPointer.i.3 = phi i32 [ %stackPointer.i.2, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.308.i" ], [ %dec.i.i, %if.then.21.i ], [ %dec.i.i, %if.then.131.i ], [ %dec.i.i, %if.end.129.i ], [ %add11.i.i, %if.then.171.i ], [ %stackPointer.i.2, %if.then.184.i ], [ %dec.i.i, %if.else.180.i ]
  %resultBary.i.3.i0 = phi float [ %resultBary.i.1.i0, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.308.i" ], [ %resultBary.i.1.i0, %if.then.21.i ], [ %resultBary.i.2.i0, %if.then.131.i ], [ %resultBary.i.2.i0, %if.end.129.i ], [ %resultBary.i.1.i0, %if.then.171.i ], [ %resultBary.i.1.i0, %if.then.184.i ], [ %resultBary.i.1.i0, %if.else.180.i ]
  %resultBary.i.3.i1 = phi float [ %resultBary.i.1.i1, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.308.i" ], [ %resultBary.i.1.i1, %if.then.21.i ], [ %resultBary.i.2.i1, %if.then.131.i ], [ %resultBary.i.2.i1, %if.end.129.i ], [ %resultBary.i.1.i1, %if.then.171.i ], [ %resultBary.i.1.i1, %if.then.184.i ], [ %resultBary.i.1.i1, %if.else.180.i ]
  %resultTriId.i.3 = phi i32 [ %resultTriId.i.1, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.308.i" ], [ %resultTriId.i.1, %if.then.21.i ], [ %resultTriId.i.2, %if.then.131.i ], [ %resultTriId.i.2, %if.end.129.i ], [ %resultTriId.i.1, %if.then.171.i ], [ %resultTriId.i.1, %if.then.184.i ], [ %resultTriId.i.1, %if.else.180.i ]
  %arrayidx192.i = getelementptr inbounds [2 x i32], [2 x i32]* %nodesToProcess.i, i32 0, i32 %currentBVHIndex.i.2
  %291 = load i32, i32* %arrayidx192.i, align 4, !tbaa !223
  %cmp193.i = icmp eq i32 %291, 0
  br i1 %cmp193.i, label %do.end.i, label %do.body.i

do.end.i:                                         ; preds = %do.cond.i
  %dec196.i = add i32 %currentBVHIndex.i.2, -1
  store float %WorldRayDirection298, float* %79, align 4
  store float %WorldRayDirection299, float* %80, align 4
  store float %WorldRayDirection300, float* %81, align 4
  %.mux. = select i1 %brmerge, i32 %.mux, i32 2
  %add.i.526.i = add nuw nsw i32 %.mux., 1
  %rem.i.527.i = urem i32 %add.i.526.i, 3
  %add4.i.528.i = add nuw nsw i32 %.mux., 2
  %rem5.i.529.i = urem i32 %add4.i.528.i, 3
  %292 = getelementptr [3 x float], [3 x float]* %18, i32 0, i32 %.mux.
  %293 = load float, float* %292, align 4, !tbaa !228, !noalias !270
  %cmp.i.532.i = fcmp fast olt float %293, 0.000000e+00
  %tmp199.i.5.0.i0 = select i1 %cmp.i.532.i, i32 %rem5.i.529.i, i32 %rem.i.527.i
  %tmp199.i.5.0.i1 = select i1 %cmp.i.532.i, i32 %rem.i.527.i, i32 %rem5.i.529.i
  %294 = getelementptr [3 x float], [3 x float]* %18, i32 0, i32 %tmp199.i.5.0.i0
  %295 = load float, float* %294, align 4, !tbaa !228, !noalias !270
  %div.i.539.i = fdiv float %295, %293
  %296 = getelementptr [3 x float], [3 x float]* %18, i32 0, i32 %tmp199.i.5.0.i1
  %297 = load float, float* %296, align 4, !tbaa !228, !noalias !270
  %div14.i.542.i = fdiv float %297, %293
  %298 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %AccelerationStructureList, i32 0)
  %299 = extractvalue %dx.types.CBufRet.i32 %298, 0
  %300 = extractvalue %dx.types.CBufRet.i32 %298, 1
  %301 = load i32, i32* %arrayidx.i, align 4, !tbaa !223
  %cmp.i = icmp eq i32 %301, 0
  br i1 %cmp.i, label %"\01?Traverse@@YA_NIII@Z.exit", label %while.body.i

"\01?Traverse@@YA_NIII@Z.exit":                   ; preds = %do.end.i, %if.end.i
  %call201.i = call i32 @"\01?Fallback_InstanceIndex@@YAIXZ"() #2
  %cmp202.i = icmp eq i32 %call201.i, -1
  br i1 %cmp202.i, label %if.else.7, label %if.then

if.then:                                          ; preds = %"\01?Traverse@@YA_NIII@Z.exit"
  %RayFlags258 = call i32 @dx.op.rayFlags.i32(i32 144)
  %and = and i32 %RayFlags258, 8
  %tobool3 = icmp eq i32 %and, 0
  br i1 %tobool3, label %if.else, label %if.end.9

if.else:                                          ; preds = %if.then
  %call5 = call i32 @"\01?Fallback_ShaderRecordOffset@@YAIXZ"() #2
  %302 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.ByteAddressBuffer(i32 160, %struct.ByteAddressBuffer %15)
  %303 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %302, i32 %call5, i32 undef)
  %304 = extractvalue %dx.types.ResRet.i32 %303, 0
  br label %if.end.9

if.else.7:                                        ; preds = %"\01?Traverse@@YA_NIII@Z.exit"
  %305 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %Constants337, i32 0)
  %306 = extractvalue %dx.types.CBufRet.i32 %305, 3
  %mul = mul i32 %306, %4
  call void @"\01?Fallback_SetShaderRecordOffset@@YAXI@Z"(i32 %mul) #2
  %307 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.ByteAddressBuffer(i32 160, %struct.ByteAddressBuffer %14)
  %308 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %307, i32 %mul, i32 undef)
  %309 = extractvalue %dx.types.ResRet.i32 %308, 0
  br label %if.end.9

if.end.9:                                         ; preds = %if.else.7, %if.else, %if.then
  %stateID.0 = phi i32 [ %304, %if.else ], [ %309, %if.else.7 ], [ 0, %if.then ]
  %cmp = icmp eq i32 %stateID.0, 0
  br i1 %cmp, label %if.end.13, label %if.then.12

if.then.12:                                       ; preds = %if.end.9
  call void @"\01?Fallback_CallIndirect@@YAXH@Z"(i32 %stateID.0) #2
  br label %if.end.13

if.end.13:                                        ; preds = %if.then.12, %if.end.9
  call void @"\01?Fallback_TraceRayEnd@@YAXH@Z"(i32 %call) #2
  ret void
}

; Function Attrs: alwaysinline nounwind readnone
define i32 @"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z"(<3 x float>) #4 {
entry:
  %1 = extractelement <3 x float> %0, i32 0
  %2 = extractelement <3 x float> %0, i32 1
  %cmp = fcmp fast ogt float %1, %2
  %3 = extractelement <3 x float> %0, i32 2
  %cmp1 = fcmp fast ogt float %1, %3
  %4 = and i1 %cmp, %cmp1
  br i1 %4, label %return, label %if.else

if.else:                                          ; preds = %entry
  %cmp4 = fcmp fast ogt float %2, %3
  br i1 %cmp4, label %return, label %if.else.8

if.else.8:                                        ; preds = %if.else
  br label %return

return:                                           ; preds = %if.else.8, %if.else, %entry
  %retval.0 = phi i32 [ 2, %if.else.8 ], [ 0, %entry ], [ 1, %if.else ]
  ret i32 %retval.0
}

; Function Attrs: alwaysinline nounwind
define void @"\01?Fallback_IgnoreHit@@YAXXZ"() #5 {
entry:
  call void @"\01?Fallback_SetAnyHitResult@@YAXH@Z"(i32 0) #2
  ret void
}

; Function Attrs: alwaysinline nounwind
define void @"\01?BVHReadTriangle@@YAXURWByteAddressBufferPointer@@AIAV?$vector@M$02@@11I@Z"(%struct.RWByteAddressBufferPointer* nocapture readonly, <3 x float>* noalias nocapture dereferenceable(12), <3 x float>* noalias nocapture dereferenceable(12), <3 x float>* noalias nocapture dereferenceable(12), i32) #5 {
entry:
  %offsetInBytes.i = getelementptr inbounds %struct.RWByteAddressBufferPointer, %struct.RWByteAddressBufferPointer* %0, i32 0, i32 1
  %5 = load i32, i32* %offsetInBytes.i, align 4, !tbaa !223
  %add.i = add i32 %5, 4
  %6 = getelementptr inbounds %struct.RWByteAddressBufferPointer, %struct.RWByteAddressBufferPointer* %0, i32 0, i32 0
  %7 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %6, align 4
  %8 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %7)
  %9 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %8, i32 %add.i, i32 undef)
  %10 = extractvalue %dx.types.ResRet.i32 %9, 0
  %mul = mul i32 %4, 40
  %add2.i = add i32 %5, %mul
  %add = add i32 %add2.i, %10
  %add1 = add i32 %add, 4
  %11 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %8, i32 %add1, i32 undef)
  %12 = extractvalue %dx.types.ResRet.i32 %11, 0
  %13 = extractvalue %dx.types.ResRet.i32 %11, 1
  %14 = extractvalue %dx.types.ResRet.i32 %11, 2
  %15 = extractvalue %dx.types.ResRet.i32 %11, 3
  %.i0 = bitcast i32 %12 to float
  %.i1 = bitcast i32 %13 to float
  %.i2 = bitcast i32 %14 to float
  %.i3 = bitcast i32 %15 to float
  %add5 = add i32 %add, 20
  %16 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %8, i32 %add5, i32 undef)
  %17 = extractvalue %dx.types.ResRet.i32 %16, 0
  %18 = extractvalue %dx.types.ResRet.i32 %16, 1
  %19 = extractvalue %dx.types.ResRet.i32 %16, 2
  %20 = extractvalue %dx.types.ResRet.i32 %16, 3
  %.i015 = bitcast i32 %17 to float
  %.i116 = bitcast i32 %18 to float
  %.i217 = bitcast i32 %19 to float
  %.i318 = bitcast i32 %20 to float
  %add9 = add i32 %add, 36
  %21 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %8, i32 %add9, i32 undef)
  %22 = extractvalue %dx.types.ResRet.i32 %21, 0
  %23 = bitcast i32 %22 to float
  %.upto019 = insertelement <3 x float> undef, float %.i0, i32 0
  %.upto120 = insertelement <3 x float> %.upto019, float %.i1, i32 1
  %24 = insertelement <3 x float> %.upto120, float %.i2, i32 2
  store <3 x float> %24, <3 x float>* %1, align 4, !tbaa !220
  %25 = insertelement <3 x float> undef, float %.i3, i64 0
  %26 = insertelement <3 x float> %25, float %.i015, i64 1
  %27 = insertelement <3 x float> %26, float %.i116, i64 2
  store <3 x float> %27, <3 x float>* %2, align 4, !tbaa !220
  %28 = insertelement <3 x float> undef, float %.i217, i64 0
  %29 = insertelement <3 x float> %28, float %.i318, i64 1
  %30 = insertelement <3 x float> %29, float %23, i64 2
  store <3 x float> %30, <3 x float>* %3, align 4, !tbaa !220
  ret void
}

; Function Attrs: alwaysinline nounwind readnone
define void @"\01?Log@@YAXV?$vector@I$03@@@Z"(<4 x i32>) #4 {
entry:
  ret void
}

; Function Attrs: alwaysinline nounwind
define void @"\01?UpdateObjectSpaceProperties@@YAXV?$vector@M$02@@0V?$matrix@M$02$03@@1@Z"(<3 x float>, <3 x float>, %class.matrix.float.3.4, %class.matrix.float.3.4) #5 {
entry:
  %.fca.0.0.extract14 = extractvalue %class.matrix.float.3.4 %3, 0, 0
  %4 = extractelement <4 x float> %.fca.0.0.extract14, i32 3
  %5 = extractelement <4 x float> %.fca.0.0.extract14, i32 2
  %6 = extractelement <4 x float> %.fca.0.0.extract14, i32 1
  %7 = extractelement <4 x float> %.fca.0.0.extract14, i32 0
  %.fca.0.1.extract15 = extractvalue %class.matrix.float.3.4 %3, 0, 1
  %8 = extractelement <4 x float> %.fca.0.1.extract15, i32 3
  %9 = extractelement <4 x float> %.fca.0.1.extract15, i32 2
  %10 = extractelement <4 x float> %.fca.0.1.extract15, i32 1
  %11 = extractelement <4 x float> %.fca.0.1.extract15, i32 0
  %.fca.0.2.extract16 = extractvalue %class.matrix.float.3.4 %3, 0, 2
  %12 = extractelement <4 x float> %.fca.0.2.extract16, i32 3
  %13 = extractelement <4 x float> %.fca.0.2.extract16, i32 2
  %14 = extractelement <4 x float> %.fca.0.2.extract16, i32 1
  %15 = extractelement <4 x float> %.fca.0.2.extract16, i32 0
  %.fca.0.0.extract = extractvalue %class.matrix.float.3.4 %2, 0, 0
  %16 = extractelement <4 x float> %.fca.0.0.extract, i32 3
  %17 = extractelement <4 x float> %.fca.0.0.extract, i32 2
  %18 = extractelement <4 x float> %.fca.0.0.extract, i32 1
  %19 = extractelement <4 x float> %.fca.0.0.extract, i32 0
  %.fca.0.1.extract = extractvalue %class.matrix.float.3.4 %2, 0, 1
  %20 = extractelement <4 x float> %.fca.0.1.extract, i32 3
  %21 = extractelement <4 x float> %.fca.0.1.extract, i32 2
  %22 = extractelement <4 x float> %.fca.0.1.extract, i32 1
  %23 = extractelement <4 x float> %.fca.0.1.extract, i32 0
  %.fca.0.2.extract = extractvalue %class.matrix.float.3.4 %2, 0, 2
  %24 = extractelement <4 x float> %.fca.0.2.extract, i32 3
  %25 = extractelement <4 x float> %.fca.0.2.extract, i32 2
  %26 = extractelement <4 x float> %.fca.0.2.extract, i32 1
  %27 = extractelement <4 x float> %.fca.0.2.extract, i32 0
  call void @"\01?Fallback_SetObjectRayOrigin@@YAXV?$vector@M$02@@@Z"(<3 x float> %0) #2
  call void @"\01?Fallback_SetObjectRayDirection@@YAXV?$vector@M$02@@@Z"(<3 x float> %1) #2
  %.0.vec.extract.upto0 = insertelement <4 x float> undef, float %19, i32 0
  %.0.vec.extract.upto1 = insertelement <4 x float> %.0.vec.extract.upto0, float %18, i32 1
  %.0.vec.extract.upto2 = insertelement <4 x float> %.0.vec.extract.upto1, float %17, i32 2
  %.0.vec.extract = insertelement <4 x float> %.0.vec.extract.upto2, float %16, i32 3
  %.fca.0.0.insert = insertvalue %class.matrix.float.3.4 undef, <4 x float> %.0.vec.extract, 0, 0
  %.16.vec.extract.upto0 = insertelement <4 x float> undef, float %23, i32 0
  %.16.vec.extract.upto1 = insertelement <4 x float> %.16.vec.extract.upto0, float %22, i32 1
  %.16.vec.extract.upto2 = insertelement <4 x float> %.16.vec.extract.upto1, float %21, i32 2
  %.16.vec.extract = insertelement <4 x float> %.16.vec.extract.upto2, float %20, i32 3
  %.fca.0.1.insert = insertvalue %class.matrix.float.3.4 %.fca.0.0.insert, <4 x float> %.16.vec.extract, 0, 1
  %.32.vec.extract.upto0 = insertelement <4 x float> undef, float %27, i32 0
  %.32.vec.extract.upto1 = insertelement <4 x float> %.32.vec.extract.upto0, float %26, i32 1
  %.32.vec.extract.upto2 = insertelement <4 x float> %.32.vec.extract.upto1, float %25, i32 2
  %.32.vec.extract = insertelement <4 x float> %.32.vec.extract.upto2, float %24, i32 3
  %.fca.0.2.insert = insertvalue %class.matrix.float.3.4 %.fca.0.1.insert, <4 x float> %.32.vec.extract, 0, 2
  call void @"\01?Fallback_SetWorldToObject@@YAXV?$matrix@M$02$03@@@Z"(%class.matrix.float.3.4 %.fca.0.2.insert) #2
  %.0.vec.extract5.upto0 = insertelement <4 x float> undef, float %7, i32 0
  %.0.vec.extract5.upto1 = insertelement <4 x float> %.0.vec.extract5.upto0, float %6, i32 1
  %.0.vec.extract5.upto2 = insertelement <4 x float> %.0.vec.extract5.upto1, float %5, i32 2
  %.0.vec.extract5 = insertelement <4 x float> %.0.vec.extract5.upto2, float %4, i32 3
  %.fca.0.0.insert1 = insertvalue %class.matrix.float.3.4 undef, <4 x float> %.0.vec.extract5, 0, 0
  %.16.vec.extract7.upto0 = insertelement <4 x float> undef, float %11, i32 0
  %.16.vec.extract7.upto1 = insertelement <4 x float> %.16.vec.extract7.upto0, float %10, i32 1
  %.16.vec.extract7.upto2 = insertelement <4 x float> %.16.vec.extract7.upto1, float %9, i32 2
  %.16.vec.extract7 = insertelement <4 x float> %.16.vec.extract7.upto2, float %8, i32 3
  %.fca.0.1.insert2 = insertvalue %class.matrix.float.3.4 %.fca.0.0.insert1, <4 x float> %.16.vec.extract7, 0, 1
  %.32.vec.extract9.upto0 = insertelement <4 x float> undef, float %15, i32 0
  %.32.vec.extract9.upto1 = insertelement <4 x float> %.32.vec.extract9.upto0, float %14, i32 1
  %.32.vec.extract9.upto2 = insertelement <4 x float> %.32.vec.extract9.upto1, float %13, i32 2
  %.32.vec.extract9 = insertelement <4 x float> %.32.vec.extract9.upto2, float %12, i32 3
  %.fca.0.2.insert3 = insertvalue %class.matrix.float.3.4 %.fca.0.1.insert2, <4 x float> %.32.vec.extract9, 0, 2
  call void @"\01?Fallback_SetObjectToWorld@@YAXV?$matrix@M$02$03@@@Z"(%class.matrix.float.3.4 %.fca.0.2.insert3) #2
  ret void
}

; Function Attrs: alwaysinline nounwind
define void @"\01?CompressBox@@YAXUBoundingBox@@V?$vector@I$01@@AIAV?$vector@I$03@@2@Z"(%struct.BoundingBox* nocapture readonly, <2 x i32>, <4 x i32>* noalias nocapture dereferenceable(16), <4 x i32>* noalias nocapture dereferenceable(16)) #5 {
entry:
  %center = getelementptr inbounds %struct.BoundingBox, %struct.BoundingBox* %0, i32 0, i32 0
  %4 = load <3 x float>, <3 x float>* %center, align 4
  %5 = extractelement <3 x float> %4, i32 0
  %6 = bitcast float %5 to i32
  %7 = insertelement <4 x i32> undef, i32 %6, i32 0
  %8 = extractelement <3 x float> %4, i32 1
  %9 = bitcast float %8 to i32
  %10 = insertelement <4 x i32> %7, i32 %9, i32 1
  %11 = extractelement <3 x float> %4, i32 2
  %12 = bitcast float %11 to i32
  %13 = insertelement <4 x i32> %10, i32 %12, i32 2
  %14 = extractelement <2 x i32> %1, i32 0
  %15 = insertelement <4 x i32> %13, i32 %14, i32 3
  %halfDim = getelementptr inbounds %struct.BoundingBox, %struct.BoundingBox* %0, i32 0, i32 1
  %16 = load <3 x float>, <3 x float>* %halfDim, align 4
  %17 = extractelement <3 x float> %16, i32 0
  %18 = bitcast float %17 to i32
  %19 = insertelement <4 x i32> undef, i32 %18, i32 0
  %20 = extractelement <3 x float> %16, i32 1
  %21 = bitcast float %20 to i32
  %22 = insertelement <4 x i32> %19, i32 %21, i32 1
  %23 = extractelement <3 x float> %16, i32 2
  %24 = bitcast float %23 to i32
  %25 = insertelement <4 x i32> %22, i32 %24, i32 2
  %26 = extractelement <2 x i32> %1, i32 1
  %27 = insertelement <4 x i32> %25, i32 %26, i32 3
  store <4 x i32> %27, <4 x i32>* %3, align 4
  store <4 x i32> %15, <4 x i32>* %2, align 4
  ret void
}

; Function Attrs: alwaysinline nounwind readnone
define void @"\01?LogNoData@@YAXI@Z"(i32) #4 {
entry:
  ret void
}

; Function Attrs: alwaysinline nounwind
define void @"\01?CreateRWByteAddressBufferPointerFromGpuVA@@YA?AURWByteAddressBufferPointer@@V?$vector@I$01@@@Z"(%struct.RWByteAddressBufferPointer* noalias nocapture sret, <2 x i32>) #5 {
entry:
  %2 = extractelement <2 x i32> %1, i32 0
  %offsetInBytes2.i = getelementptr inbounds %struct.RWByteAddressBufferPointer, %struct.RWByteAddressBufferPointer* %0, i32 0, i32 1
  store i32 %2, i32* %offsetInBytes2.i, align 4, !tbaa !223, !alias.scope !273
  ret void
}

; Function Attrs: alwaysinline nounwind
define void @"\01?AABBtoBoundingBox@@YA?AUBoundingBox@@UAABB@@@Z"(%struct.BoundingBox* noalias nocapture sret, %struct.AABB* nocapture readonly) #5 {
entry:
  %min = getelementptr inbounds %struct.AABB, %struct.AABB* %1, i32 0, i32 0
  %2 = load <3 x float>, <3 x float>* %min, align 4, !tbaa !220
  %.i03 = extractelement <3 x float> %2, i32 0
  %.i14 = extractelement <3 x float> %2, i32 1
  %.i25 = extractelement <3 x float> %2, i32 2
  %max = getelementptr inbounds %struct.AABB, %struct.AABB* %1, i32 0, i32 1
  %3 = load <3 x float>, <3 x float>* %max, align 4, !tbaa !220
  %.i0 = extractelement <3 x float> %3, i32 0
  %add.i0 = fadd fast float %.i0, %.i03
  %.i1 = extractelement <3 x float> %3, i32 1
  %add.i1 = fadd fast float %.i1, %.i14
  %.i2 = extractelement <3 x float> %3, i32 2
  %add.i2 = fadd fast float %.i2, %.i25
  %mul.i0 = fmul fast float %add.i0, 5.000000e-01
  %mul.i1 = fmul fast float %add.i1, 5.000000e-01
  %mul.i2 = fmul fast float %add.i2, 5.000000e-01
  %mul.upto0 = insertelement <3 x float> undef, float %mul.i0, i32 0
  %mul.upto1 = insertelement <3 x float> %mul.upto0, float %mul.i1, i32 1
  %mul = insertelement <3 x float> %mul.upto1, float %mul.i2, i32 2
  %sub.i0 = fsub fast float %.i0, %mul.i0
  %sub.i1 = fsub fast float %.i1, %mul.i1
  %sub.i2 = fsub fast float %.i2, %mul.i2
  %sub.upto0 = insertelement <3 x float> undef, float %sub.i0, i32 0
  %sub.upto1 = insertelement <3 x float> %sub.upto0, float %sub.i1, i32 1
  %sub = insertelement <3 x float> %sub.upto1, float %sub.i2, i32 2
  %4 = getelementptr inbounds %struct.BoundingBox, %struct.BoundingBox* %0, i32 0, i32 0
  store <3 x float> %mul, <3 x float>* %4, align 4
  %5 = getelementptr inbounds %struct.BoundingBox, %struct.BoundingBox* %0, i32 0, i32 1
  store <3 x float> %sub, <3 x float>* %5, align 4
  ret void
}

; Function Attrs: alwaysinline nounwind
define i1 @"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z"(<3 x float>, <3 x float>, float* noalias nocapture dereferenceable(4), %struct.ProceduralPrimitiveAttributes* noalias nocapture, <3 x float>, float) #5 {
entry:
  %direction.i0 = extractelement <3 x float> %1, i32 0
  %direction.i1 = extractelement <3 x float> %1, i32 1
  %direction.i2 = extractelement <3 x float> %1, i32 2
  %center.i0 = extractelement <3 x float> %4, i32 0
  %origin.i0 = extractelement <3 x float> %0, i32 0
  %center.i1 = extractelement <3 x float> %4, i32 1
  %origin.i1 = extractelement <3 x float> %0, i32 1
  %center.i2 = extractelement <3 x float> %4, i32 2
  %origin.i2 = extractelement <3 x float> %0, i32 2
  %6 = getelementptr inbounds %struct.ProceduralPrimitiveAttributes, %struct.ProceduralPrimitiveAttributes* %3, i32 0, i32 0
  %7 = load <3 x float>, <3 x float>* %6, align 4
  %.i048 = extractelement <3 x float> %7, i32 0
  %.i149 = extractelement <3 x float> %7, i32 1
  %.i250 = extractelement <3 x float> %7, i32 2
  %Log = call float @dx.op.unary.f32(i32 23, float %5)
  %8 = fmul fast float %Log, 2.000000e+00
  %Exp = call float @dx.op.unary.f32(i32 21, float %8)
  %sub.i0 = fsub fast float %origin.i0, %center.i0
  %sub.i1 = fsub fast float %origin.i1, %center.i1
  %sub.i2 = fsub fast float %origin.i2, %center.i2
  %9 = extractelement <3 x float> %1, i64 0
  %10 = extractelement <3 x float> %1, i64 1
  %11 = extractelement <3 x float> %1, i64 2
  %12 = call float @dx.op.dot3.f32(i32 55, float %9, float %10, float %11, float %9, float %10, float %11)
  %13 = call float @dx.op.dot3.f32(i32 55, float %9, float %10, float %11, float %sub.i0, float %sub.i1, float %sub.i2)
  %mul = fmul fast float %13, 2.000000e+00
  %14 = call float @dx.op.dot3.f32(i32 55, float %sub.i0, float %sub.i1, float %sub.i2, float %sub.i0, float %sub.i1, float %sub.i2)
  %sub4 = fsub fast float %14, %Exp
  %mul.i = fmul fast float %mul, %mul
  %mul1.i = fmul fast float %12, 4.000000e+00
  %mul2.i = fmul fast float %mul1.i, %sub4
  %sub.i = fsub fast float %mul.i, %mul2.i
  %cmp.i = fcmp fast olt float %sub.i, 0.000000e+00
  br i1 %cmp.i, label %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit", label %if.else.i

if.else.i:                                        ; preds = %entry
  %cmp4.i = fcmp fast oeq float %sub.i, 0.000000e+00
  br i1 %cmp4.i, label %if.then.7.i, label %if.else.9.i

if.then.7.i:                                      ; preds = %if.else.i
  %mul8.i = fsub fast float -0.000000e+00, %13
  %div.i = fdiv fast float %mul8.i, %12
  br label %if.end.19.i

if.else.9.i:                                      ; preds = %if.else.i
  %cmp10.i = fcmp fast ogt float %mul, 0.000000e+00
  br i1 %cmp10.i, label %cond.true.i, label %cond.false.i

cond.true.i:                                      ; preds = %if.else.9.i
  %Sqrt = call float @dx.op.unary.f32(i32 24, float %sub.i)
  %add.i = fadd fast float %Sqrt, %mul
  br label %cond.end.i

cond.false.i:                                     ; preds = %if.else.9.i
  %Sqrt28 = call float @dx.op.unary.f32(i32 24, float %sub.i)
  %sub15.i = fsub fast float %mul, %Sqrt28
  br label %cond.end.i

cond.end.i:                                       ; preds = %cond.false.i, %cond.true.i
  %add.sink.i = phi float [ %add.i, %cond.true.i ], [ %sub15.i, %cond.false.i ]
  %mul13.i = fmul fast float %add.sink.i, -5.000000e-01
  %div17.i = fdiv fast float %mul13.i, %12
  %div18.i = fdiv fast float %sub4, %mul13.i
  br label %if.end.19.i

if.end.19.i:                                      ; preds = %cond.end.i, %if.then.7.i
  %.025 = phi float [ %div.i, %if.then.7.i ], [ %div18.i, %cond.end.i ]
  %.024 = phi float [ %div.i, %if.then.7.i ], [ %div17.i, %cond.end.i ]
  %cmp20.i = fcmp fast ogt float %.024, %.025
  %.126 = select i1 %cmp20.i, float %.024, float %.025
  %.1 = select i1 %cmp20.i, float %.025, float %.024
  br label %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit"

"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit":        ; preds = %if.end.19.i, %entry
  %.227 = phi float [ undef, %entry ], [ %.126, %if.end.19.i ]
  %retval.i.0 = phi i1 [ false, %entry ], [ true, %if.end.19.i ]
  %.2 = phi float [ undef, %entry ], [ %.1, %if.end.19.i ]
  br i1 %retval.i.0, label %if.end, label %return

if.end:                                           ; preds = %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit"
  %cmp = fcmp fast ogt float %.2, %.227
  %t0.0 = select i1 %cmp, float %.227, float %.2
  %t1.0 = select i1 %cmp, float %.2, float %.227
  %cmp9 = fcmp fast olt float %t0.0, 0.000000e+00
  br i1 %cmp9, label %if.then.12, label %if.end.18

if.then.12:                                       ; preds = %if.end
  %cmp13 = fcmp fast olt float %t1.0, 0.000000e+00
  br i1 %cmp13, label %return, label %if.end.18

if.end.18:                                        ; preds = %if.then.12, %if.end
  %t0.1 = phi float [ %t1.0, %if.then.12 ], [ %t0.0, %if.end ]
  store float %t0.1, float* %2, align 4, !tbaa !228
  %mul19.i0 = fmul fast float %t0.1, %direction.i0
  %mul19.i1 = fmul fast float %t0.1, %direction.i1
  %mul19.i2 = fmul fast float %t0.1, %direction.i2
  %ObjectToWorld = call float @dx.op.objectToWorld.f32(i32 151, i32 0, i8 0)
  %ObjectToWorld30 = call float @dx.op.objectToWorld.f32(i32 151, i32 0, i8 1)
  %ObjectToWorld31 = call float @dx.op.objectToWorld.f32(i32 151, i32 0, i8 2)
  %ObjectToWorld33 = call float @dx.op.objectToWorld.f32(i32 151, i32 1, i8 0)
  %ObjectToWorld34 = call float @dx.op.objectToWorld.f32(i32 151, i32 1, i8 1)
  %ObjectToWorld35 = call float @dx.op.objectToWorld.f32(i32 151, i32 1, i8 2)
  %ObjectToWorld37 = call float @dx.op.objectToWorld.f32(i32 151, i32 2, i8 0)
  %ObjectToWorld38 = call float @dx.op.objectToWorld.f32(i32 151, i32 2, i8 1)
  %ObjectToWorld39 = call float @dx.op.objectToWorld.f32(i32 151, i32 2, i8 2)
  %sub21.i0 = fadd fast float %sub.i0, %mul19.i0
  %sub21.i1 = fadd fast float %sub.i1, %mul19.i1
  %sub21.i2 = fadd fast float %sub.i2, %mul19.i2
  %15 = fmul fast float %sub21.i0, %sub21.i0
  %16 = fmul fast float %sub21.i1, %sub21.i1
  %17 = fadd fast float %15, %16
  %18 = fmul fast float %sub21.i2, %sub21.i2
  %19 = fadd fast float %17, %18
  %Sqrt29 = call float @dx.op.unary.f32(i32 24, float %19)
  %.i0 = fdiv fast float %sub21.i0, %Sqrt29
  %.i1 = fdiv fast float %sub21.i1, %Sqrt29
  %.i2 = fdiv fast float %sub21.i2, %Sqrt29
  %20 = fmul fast float %.i0, %ObjectToWorld
  %FMad47 = call float @dx.op.tertiary.f32(i32 46, float %.i1, float %ObjectToWorld33, float %20)
  %FMad46 = call float @dx.op.tertiary.f32(i32 46, float %.i2, float %ObjectToWorld37, float %FMad47)
  %21 = fmul fast float %.i0, %ObjectToWorld30
  %FMad45 = call float @dx.op.tertiary.f32(i32 46, float %.i1, float %ObjectToWorld34, float %21)
  %FMad44 = call float @dx.op.tertiary.f32(i32 46, float %.i2, float %ObjectToWorld38, float %FMad45)
  %22 = fmul fast float %.i0, %ObjectToWorld31
  %FMad43 = call float @dx.op.tertiary.f32(i32 46, float %.i1, float %ObjectToWorld35, float %22)
  %FMad42 = call float @dx.op.tertiary.f32(i32 46, float %.i2, float %ObjectToWorld39, float %FMad43)
  br label %return

return:                                           ; preds = %if.end.18, %if.then.12, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit"
  %retval.0 = phi i1 [ true, %if.end.18 ], [ false, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit" ], [ false, %if.then.12 ]
  %.0.0.i0 = phi float [ %FMad46, %if.end.18 ], [ %.i048, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit" ], [ %.i048, %if.then.12 ]
  %.0.0.i1 = phi float [ %FMad44, %if.end.18 ], [ %.i149, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit" ], [ %.i149, %if.then.12 ]
  %.0.0.i2 = phi float [ %FMad42, %if.end.18 ], [ %.i250, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit" ], [ %.i250, %if.then.12 ]
  %.0.0.upto0 = insertelement <3 x float> undef, float %.0.0.i0, i32 0
  %.0.0.upto1 = insertelement <3 x float> %.0.0.upto0, float %.0.0.i1, i32 1
  %.0.0 = insertelement <3 x float> %.0.0.upto1, float %.0.0.i2, i32 2
  store <3 x float> %.0.0, <3 x float>* %6, align 4
  ret i1 %retval.0
}

; Function Attrs: alwaysinline nounwind readonly
define <3 x float> @"\01?GetMinCorner@@YA?AV?$vector@M$02@@UBoundingBox@@@Z"(%struct.BoundingBox* nocapture readonly) #6 {
entry:
  %center = getelementptr inbounds %struct.BoundingBox, %struct.BoundingBox* %0, i32 0, i32 0
  %1 = load <3 x float>, <3 x float>* %center, align 4, !tbaa !220
  %.i01 = extractelement <3 x float> %1, i32 0
  %.i12 = extractelement <3 x float> %1, i32 1
  %.i23 = extractelement <3 x float> %1, i32 2
  %halfDim = getelementptr inbounds %struct.BoundingBox, %struct.BoundingBox* %0, i32 0, i32 1
  %2 = load <3 x float>, <3 x float>* %halfDim, align 4, !tbaa !220
  %.i0 = extractelement <3 x float> %2, i32 0
  %sub.i0 = fsub fast float %.i01, %.i0
  %.i1 = extractelement <3 x float> %2, i32 1
  %sub.i1 = fsub fast float %.i12, %.i1
  %.i2 = extractelement <3 x float> %2, i32 2
  %sub.i2 = fsub fast float %.i23, %.i2
  %sub.upto0 = insertelement <3 x float> undef, float %sub.i0, i32 0
  %sub.upto1 = insertelement <3 x float> %sub.upto0, float %sub.i1, i32 1
  %sub = insertelement <3 x float> %sub.upto1, float %sub.i2, i32 2
  ret <3 x float> %sub
}

; Function Attrs: alwaysinline nounwind
define void @"\01?LoadRaytracingInstanceDesc@@YA?AURaytracingInstanceDesc@@URWByteAddressBuffer@@I@Z"(%struct.RaytracingInstanceDesc* noalias nocapture sret, %struct.RWByteAddressBuffer* nocapture readonly, i32) #5 {
entry:
  %3 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %1, align 4
  %4 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %3)
  %5 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %4, i32 %2, i32 undef)
  %6 = extractvalue %dx.types.ResRet.i32 %5, 0
  %7 = extractvalue %dx.types.ResRet.i32 %5, 1
  %8 = extractvalue %dx.types.ResRet.i32 %5, 2
  %9 = extractvalue %dx.types.ResRet.i32 %5, 3
  %add.1 = add i32 %2, 16
  %10 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %4, i32 %add.1, i32 undef)
  %11 = extractvalue %dx.types.ResRet.i32 %10, 0
  %12 = extractvalue %dx.types.ResRet.i32 %10, 1
  %13 = extractvalue %dx.types.ResRet.i32 %10, 2
  %14 = extractvalue %dx.types.ResRet.i32 %10, 3
  %add.2 = add i32 %2, 32
  %15 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %4, i32 %add.2, i32 undef)
  %16 = extractvalue %dx.types.ResRet.i32 %15, 0
  %17 = extractvalue %dx.types.ResRet.i32 %15, 1
  %18 = extractvalue %dx.types.ResRet.i32 %15, 2
  %19 = extractvalue %dx.types.ResRet.i32 %15, 3
  %add.3 = add i32 %2, 48
  %20 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %4, i32 %add.3, i32 undef)
  %21 = extractvalue %dx.types.ResRet.i32 %20, 0
  %22 = extractvalue %dx.types.ResRet.i32 %20, 1
  %23 = extractvalue %dx.types.ResRet.i32 %20, 2
  %24 = extractvalue %dx.types.ResRet.i32 %20, 3
  %.i0 = bitcast i32 %6 to float
  %.i1 = bitcast i32 %7 to float
  %.i2 = bitcast i32 %8 to float
  %.i3 = bitcast i32 %9 to float
  %.upto0 = insertelement <4 x float> undef, float %.i0, i32 0
  %.upto1 = insertelement <4 x float> %.upto0, float %.i1, i32 1
  %.upto2 = insertelement <4 x float> %.upto1, float %.i2, i32 2
  %25 = insertelement <4 x float> %.upto2, float %.i3, i32 3
  %arrayidx.i = getelementptr inbounds %struct.RaytracingInstanceDesc, %struct.RaytracingInstanceDesc* %0, i32 0, i32 0, i32 0
  store <4 x float> %25, <4 x float>* %arrayidx.i, align 4, !tbaa !220, !alias.scope !276
  %.i040 = bitcast i32 %11 to float
  %.i141 = bitcast i32 %12 to float
  %.i242 = bitcast i32 %13 to float
  %.i343 = bitcast i32 %14 to float
  %.upto048 = insertelement <4 x float> undef, float %.i040, i32 0
  %.upto149 = insertelement <4 x float> %.upto048, float %.i141, i32 1
  %.upto250 = insertelement <4 x float> %.upto149, float %.i242, i32 2
  %26 = insertelement <4 x float> %.upto250, float %.i343, i32 3
  %arrayidx3.i = getelementptr inbounds %struct.RaytracingInstanceDesc, %struct.RaytracingInstanceDesc* %0, i32 0, i32 0, i32 1
  store <4 x float> %26, <4 x float>* %arrayidx3.i, align 4, !tbaa !220, !alias.scope !276
  %.i044 = bitcast i32 %16 to float
  %.i145 = bitcast i32 %17 to float
  %.i246 = bitcast i32 %18 to float
  %.i347 = bitcast i32 %19 to float
  %.upto051 = insertelement <4 x float> undef, float %.i044, i32 0
  %.upto152 = insertelement <4 x float> %.upto051, float %.i145, i32 1
  %.upto253 = insertelement <4 x float> %.upto152, float %.i246, i32 2
  %27 = insertelement <4 x float> %.upto253, float %.i347, i32 3
  %arrayidx6.i = getelementptr inbounds %struct.RaytracingInstanceDesc, %struct.RaytracingInstanceDesc* %0, i32 0, i32 0, i32 2
  store <4 x float> %27, <4 x float>* %arrayidx6.i, align 4, !tbaa !220, !alias.scope !276
  %InstanceIDAndMask.i = getelementptr inbounds %struct.RaytracingInstanceDesc, %struct.RaytracingInstanceDesc* %0, i32 0, i32 1
  store i32 %21, i32* %InstanceIDAndMask.i, align 4, !tbaa !223, !alias.scope !276
  %InstanceContributionToHitGroupIndexAndFlags.i = getelementptr inbounds %struct.RaytracingInstanceDesc, %struct.RaytracingInstanceDesc* %0, i32 0, i32 2
  store i32 %22, i32* %InstanceContributionToHitGroupIndexAndFlags.i, align 4, !tbaa !223, !alias.scope !276
  %.upto054 = insertelement <2 x i32> undef, i32 %23, i32 0
  %28 = insertelement <2 x i32> %.upto054, i32 %24, i32 1
  %AccelerationStructure.i = getelementptr inbounds %struct.RaytracingInstanceDesc, %struct.RaytracingInstanceDesc* %0, i32 0, i32 3
  store <2 x i32> %28, <2 x i32>* %AccelerationStructure.i, align 4, !tbaa !220, !alias.scope !276
  ret void
}

; Function Attrs: alwaysinline nounwind readonly
define <3 x float> @"\01?GetMaxCorner@@YA?AV?$vector@M$02@@UBoundingBox@@@Z"(%struct.BoundingBox* nocapture readonly) #6 {
entry:
  %center = getelementptr inbounds %struct.BoundingBox, %struct.BoundingBox* %0, i32 0, i32 0
  %1 = load <3 x float>, <3 x float>* %center, align 4, !tbaa !220
  %.i01 = extractelement <3 x float> %1, i32 0
  %.i12 = extractelement <3 x float> %1, i32 1
  %.i23 = extractelement <3 x float> %1, i32 2
  %halfDim = getelementptr inbounds %struct.BoundingBox, %struct.BoundingBox* %0, i32 0, i32 1
  %2 = load <3 x float>, <3 x float>* %halfDim, align 4, !tbaa !220
  %.i0 = extractelement <3 x float> %2, i32 0
  %add.i0 = fadd fast float %.i0, %.i01
  %.i1 = extractelement <3 x float> %2, i32 1
  %add.i1 = fadd fast float %.i1, %.i12
  %.i2 = extractelement <3 x float> %2, i32 2
  %add.i2 = fadd fast float %.i2, %.i23
  %add.upto0 = insertelement <3 x float> undef, float %add.i0, i32 0
  %add.upto1 = insertelement <3 x float> %add.upto0, float %add.i1, i32 1
  %add = insertelement <3 x float> %add.upto1, float %add.i2, i32 2
  ret <3 x float> %add
}

; Function Attrs: alwaysinline nounwind
define void @"\01?MyMissShader_Shadow@@YAXUShadowPayload@@@Z"(%struct.ShadowPayload* noalias nocapture) #5 {
entry:
  %1 = getelementptr inbounds %struct.ShadowPayload, %struct.ShadowPayload* %0, i32 0, i32 0
  store i32 0, i32* %1, align 4
  ret void
}

; Function Attrs: alwaysinline nounwind readnone
define i1 @"\01?IsProceduralGeometry@@YA_NV?$vector@I$01@@@Z"(<2 x i32>) #4 {
entry:
  %1 = extractelement <2 x i32> %0, i32 0
  %and = and i32 %1, 1073741824
  %tobool = icmp ne i32 %and, 0
  ret i1 %tobool
}

; Function Attrs: alwaysinline nounwind
define void @"\01?IntersectionShader_Box@@YAXXZ"() #5 {
entry:
  %0 = load %"class.StructuredBuffer<AABBPrimitiveAttributes>", %"class.StructuredBuffer<AABBPrimitiveAttributes>"* @"\01?g_AABBPrimitiveAttributes@@3V?$StructuredBuffer@UAABBPrimitiveAttributes@@@@A", align 4
  %attr = alloca %struct.ProceduralPrimitiveAttributes, align 4
  %RayTCurrent = call float @dx.op.rayTCurrent.f32(i32 154)
  %ObjectRayDirection = call float @dx.op.objectRayDirection.f32(i32 150, i8 0)
  %ObjectRayDirection131 = call float @dx.op.objectRayDirection.f32(i32 150, i8 1)
  %ObjectRayDirection132 = call float @dx.op.objectRayDirection.f32(i32 150, i8 2)
  %ObjectRayOrigin = call float @dx.op.objectRayOrigin.f32(i32 149, i8 0)
  %ObjectRayOrigin129 = call float @dx.op.objectRayOrigin.f32(i32 149, i8 1)
  %ObjectRayOrigin130 = call float @dx.op.objectRayOrigin.f32(i32 149, i8 2)
  %PrimitiveID137 = call i32 @dx.op.primitiveID.i32(i32 108)
  %1 = call %dx.types.Handle @"dx.op.createHandleFromResourceStructForLib.class.StructuredBuffer<AABBPrimitiveAttributes>"(i32 160, %"class.StructuredBuffer<AABBPrimitiveAttributes>" %0)
  %2 = call %dx.types.ResRet.f32 @dx.op.bufferLoad.f32(i32 68, %dx.types.Handle %1, i32 %PrimitiveID137, i32 0)
  %3 = extractvalue %dx.types.ResRet.f32 %2, 0
  %4 = extractvalue %dx.types.ResRet.f32 %2, 1
  %5 = extractvalue %dx.types.ResRet.f32 %2, 2
  %6 = call %dx.types.ResRet.f32 @dx.op.bufferLoad.f32(i32 68, %dx.types.Handle %1, i32 %PrimitiveID137, i32 16)
  %7 = extractvalue %dx.types.ResRet.f32 %6, 0
  %8 = extractvalue %dx.types.ResRet.f32 %6, 1
  %9 = extractvalue %dx.types.ResRet.f32 %6, 2
  %10 = call %dx.types.ResRet.f32 @dx.op.bufferLoad.f32(i32 68, %dx.types.Handle %1, i32 %PrimitiveID137, i32 32)
  %11 = extractvalue %dx.types.ResRet.f32 %10, 0
  %12 = extractvalue %dx.types.ResRet.f32 %10, 1
  %13 = extractvalue %dx.types.ResRet.f32 %10, 2
  %14 = call %dx.types.ResRet.f32 @dx.op.bufferLoad.f32(i32 68, %dx.types.Handle %1, i32 %PrimitiveID137, i32 48)
  %15 = extractvalue %dx.types.ResRet.f32 %14, 0
  %16 = extractvalue %dx.types.ResRet.f32 %14, 1
  %17 = extractvalue %dx.types.ResRet.f32 %14, 2
  %18 = fmul fast float %3, %ObjectRayOrigin
  %FMad223 = call float @dx.op.tertiary.f32(i32 46, float %ObjectRayOrigin129, float %7, float %18)
  %FMad222 = call float @dx.op.tertiary.f32(i32 46, float %ObjectRayOrigin130, float %11, float %FMad223)
  %FMad221 = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %15, float %FMad222)
  %19 = fmul fast float %4, %ObjectRayOrigin
  %FMad220 = call float @dx.op.tertiary.f32(i32 46, float %ObjectRayOrigin129, float %8, float %19)
  %FMad219 = call float @dx.op.tertiary.f32(i32 46, float %ObjectRayOrigin130, float %12, float %FMad220)
  %FMad218 = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %16, float %FMad219)
  %20 = fmul fast float %5, %ObjectRayOrigin
  %FMad217 = call float @dx.op.tertiary.f32(i32 46, float %ObjectRayOrigin129, float %9, float %20)
  %FMad216 = call float @dx.op.tertiary.f32(i32 46, float %ObjectRayOrigin130, float %13, float %FMad217)
  %FMad215 = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %17, float %FMad216)
  %21 = fmul fast float %3, %ObjectRayDirection
  %FMad259 = call float @dx.op.tertiary.f32(i32 46, float %ObjectRayDirection131, float %7, float %21)
  %FMad258 = call float @dx.op.tertiary.f32(i32 46, float %ObjectRayDirection132, float %11, float %FMad259)
  %FMad257 = call float @dx.op.tertiary.f32(i32 46, float 0.000000e+00, float %15, float %FMad258)
  %22 = fmul fast float %4, %ObjectRayDirection
  %FMad256 = call float @dx.op.tertiary.f32(i32 46, float %ObjectRayDirection131, float %8, float %22)
  %FMad255 = call float @dx.op.tertiary.f32(i32 46, float %ObjectRayDirection132, float %12, float %FMad256)
  %FMad254 = call float @dx.op.tertiary.f32(i32 46, float 0.000000e+00, float %16, float %FMad255)
  %23 = fmul fast float %5, %ObjectRayDirection
  %FMad253 = call float @dx.op.tertiary.f32(i32 46, float %ObjectRayDirection131, float %9, float %23)
  %FMad252 = call float @dx.op.tertiary.f32(i32 46, float %ObjectRayDirection132, float %13, float %FMad253)
  %FMad251 = call float @dx.op.tertiary.f32(i32 46, float 0.000000e+00, float %17, float %FMad252)
  %rem.i = urem i32 %PrimitiveID137, 3
  switch i32 %rem.i, label %"\01?IntersectCustomPrimitiveFrontToBack@@YA_NV?$vector@M$02@@0MMAIAMUProceduralPrimitiveAttributes@@@Z.exit" [
    i32 0, label %sw.bb.i
    i32 2, label %sw.bb.9.i
    i32 1, label %sw.bb.11.i
  ]

sw.bb.i:                                          ; preds = %entry
  %sub.i.i = fsub fast float -1.000000e+00, %FMad221
  %div.i.i = fdiv fast float %sub.i.i, %FMad257
  %sub2.i.i = fsub fast float 1.000000e+00, %FMad221
  %div3.i.i = fdiv fast float %sub2.i.i, %FMad257
  %cmp.i.i = fcmp fast olt float %FMad257, 0.000000e+00
  %tmin.i.i.0 = select i1 %cmp.i.i, float %div3.i.i, float %div.i.i
  %tmax.i.i.0 = select i1 %cmp.i.i, float %div.i.i, float %div3.i.i
  %sub6.i.i = fsub fast float -1.000000e+00, %FMad218
  %div7.i.i = fdiv fast float %sub6.i.i, %FMad254
  %sub9.i.i = fsub fast float 1.000000e+00, %FMad218
  %div10.i.i = fdiv fast float %sub9.i.i, %FMad254
  %cmp11.i.i = fcmp fast olt float %FMad254, 0.000000e+00
  %tymin.i.i.0 = select i1 %cmp11.i.i, float %div10.i.i, float %div7.i.i
  %tymax.i.i.0 = select i1 %cmp11.i.i, float %div7.i.i, float %div10.i.i
  %sub17.i.i = fsub fast float -1.000000e+00, %FMad215
  %div18.i.i = fdiv fast float %sub17.i.i, %FMad251
  %sub20.i.i = fsub fast float 1.000000e+00, %FMad215
  %div21.i.i = fdiv fast float %sub20.i.i, %FMad251
  %cmp22.i.i = fcmp fast olt float %FMad251, 0.000000e+00
  %tzmin.i.i.0 = select i1 %cmp22.i.i, float %div21.i.i, float %div18.i.i
  %tzmax.i.i.0 = select i1 %cmp22.i.i, float %div18.i.i, float %div21.i.i
  %FMax116 = call float @dx.op.binary.f32(i32 35, float %tmin.i.i.0, float %tymin.i.i.0)
  %FMax = call float @dx.op.binary.f32(i32 35, float %FMax116, float %tzmin.i.i.0)
  %FMin115 = call float @dx.op.binary.f32(i32 36, float %tmax.i.i.0, float %tymax.i.i.0)
  %FMin = call float @dx.op.binary.f32(i32 36, float %FMin115, float %tzmax.i.i.0)
  %mul.i.i.i0 = fmul fast float %FMax, %FMad257
  %mul.i.i.i1 = fmul fast float %FMax, %FMad254
  %mul.i.i.i2 = fmul fast float %FMax, %FMad251
  %add.i.i.i0 = fadd fast float %mul.i.i.i0, %FMad221
  %add.i.i.i1 = fadd fast float %mul.i.i.i1, %FMad218
  %add.i.i.i2 = fadd fast float %mul.i.i.i2, %FMad215
  %24 = fmul fast float %add.i.i.i0, %add.i.i.i0
  %25 = fmul fast float %add.i.i.i1, %add.i.i.i1
  %26 = fadd fast float %24, %25
  %27 = fmul fast float %add.i.i.i2, %add.i.i.i2
  %28 = fadd fast float %26, %27
  %Sqrt124 = call float @dx.op.unary.f32(i32 24, float %28)
  %.i0 = fdiv fast float %add.i.i.i0, %Sqrt124
  %.i1 = fdiv fast float %add.i.i.i1, %Sqrt124
  %.i2 = fdiv fast float %add.i.i.i2, %Sqrt124
  %FAbs107 = call float @dx.op.unary.f32(i32 6, float %.i0)
  %FAbs106 = call float @dx.op.unary.f32(i32 6, float %.i1)
  %cmp34.i.i = fcmp fast ogt float %FAbs107, %FAbs106
  br i1 %cmp34.i.i, label %if.then.37.i.i, label %if.else.46.i.i

if.then.37.i.i:                                   ; preds = %sw.bb.i
  %FAbs = call float @dx.op.unary.f32(i32 6, float %.i2)
  %cmp40.i.i = fcmp fast ogt float %FAbs107, %FAbs
  br i1 %cmp40.i.i, label %"\01?intersectBox@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@@Z.exit.i", label %if.else.i.i

if.else.i.i:                                      ; preds = %if.then.37.i.i
  br label %"\01?intersectBox@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@@Z.exit.i"

if.else.46.i.i:                                   ; preds = %sw.bb.i
  %FAbs104 = call float @dx.op.unary.f32(i32 6, float %.i2)
  %cmp49.i.i = fcmp fast ogt float %FAbs106, %FAbs104
  br i1 %cmp49.i.i, label %"\01?intersectBox@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@@Z.exit.i", label %if.else.54.i.i

if.else.54.i.i:                                   ; preds = %if.else.46.i.i
  br label %"\01?intersectBox@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@@Z.exit.i"

"\01?intersectBox@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@@Z.exit.i": ; preds = %if.else.54.i.i, %if.else.46.i.i, %if.else.i.i, %if.then.37.i.i
  %.068.0.i0 = phi float [ 0.000000e+00, %if.else.i.i ], [ 0.000000e+00, %if.else.54.i.i ], [ %.i0, %if.then.37.i.i ], [ 0.000000e+00, %if.else.46.i.i ]
  %.068.0.i1 = phi float [ 0.000000e+00, %if.else.i.i ], [ 0.000000e+00, %if.else.54.i.i ], [ 0.000000e+00, %if.then.37.i.i ], [ %.i1, %if.else.46.i.i ]
  %.068.0.i2 = phi float [ %.i2, %if.else.i.i ], [ %.i2, %if.else.54.i.i ], [ 0.000000e+00, %if.then.37.i.i ], [ 0.000000e+00, %if.else.46.i.i ]
  %ObjectToWorld185 = call float @dx.op.objectToWorld.f32(i32 151, i32 0, i8 0)
  %ObjectToWorld186 = call float @dx.op.objectToWorld.f32(i32 151, i32 0, i8 1)
  %ObjectToWorld187 = call float @dx.op.objectToWorld.f32(i32 151, i32 0, i8 2)
  %ObjectToWorld189 = call float @dx.op.objectToWorld.f32(i32 151, i32 1, i8 0)
  %ObjectToWorld190 = call float @dx.op.objectToWorld.f32(i32 151, i32 1, i8 1)
  %ObjectToWorld191 = call float @dx.op.objectToWorld.f32(i32 151, i32 1, i8 2)
  %ObjectToWorld193 = call float @dx.op.objectToWorld.f32(i32 151, i32 2, i8 0)
  %ObjectToWorld194 = call float @dx.op.objectToWorld.f32(i32 151, i32 2, i8 1)
  %ObjectToWorld195 = call float @dx.op.objectToWorld.f32(i32 151, i32 2, i8 2)
  %29 = fmul fast float %.068.0.i0, %.068.0.i0
  %30 = fmul fast float %.068.0.i1, %.068.0.i1
  %31 = fadd fast float %30, %29
  %32 = fmul fast float %.068.0.i2, %.068.0.i2
  %33 = fadd fast float %31, %32
  %Sqrt123 = call float @dx.op.unary.f32(i32 24, float %33)
  %.i0260 = fdiv fast float %.068.0.i0, %Sqrt123
  %.i1261 = fdiv fast float %.068.0.i1, %Sqrt123
  %.i2262 = fdiv fast float %.068.0.i2, %Sqrt123
  %34 = fmul fast float %.i0260, %ObjectToWorld185
  %FMad211 = call float @dx.op.tertiary.f32(i32 46, float %.i1261, float %ObjectToWorld189, float %34)
  %FMad210 = call float @dx.op.tertiary.f32(i32 46, float %.i2262, float %ObjectToWorld193, float %FMad211)
  %35 = fmul fast float %.i0260, %ObjectToWorld186
  %FMad209 = call float @dx.op.tertiary.f32(i32 46, float %.i1261, float %ObjectToWorld190, float %35)
  %FMad208 = call float @dx.op.tertiary.f32(i32 46, float %.i2262, float %ObjectToWorld194, float %FMad209)
  %36 = fmul fast float %.i0260, %ObjectToWorld187
  %FMad207 = call float @dx.op.tertiary.f32(i32 46, float %.i1261, float %ObjectToWorld191, float %36)
  %FMad206 = call float @dx.op.tertiary.f32(i32 46, float %.i2262, float %ObjectToWorld195, float %FMad207)
  %cmp63.i.i = fcmp fast ogt float %FMin, %FMax
  br label %"\01?IntersectCustomPrimitiveFrontToBack@@YA_NV?$vector@M$02@@0MMAIAMUProceduralPrimitiveAttributes@@@Z.exit"

sw.bb.9.i:                                        ; preds = %entry
  %37 = call float @dx.op.dot3.f32(i32 55, float %FMad257, float %FMad254, float %FMad251, float %FMad257, float %FMad254, float %FMad251)
  %38 = call float @dx.op.dot3.f32(i32 55, float %FMad257, float %FMad254, float %FMad251, float %FMad221, float %FMad218, float %FMad215)
  %mul.i.19.i = fmul fast float %38, 2.000000e+00
  %39 = call float @dx.op.dot3.f32(i32 55, float %FMad221, float %FMad218, float %FMad215, float %FMad221, float %FMad218, float %FMad215)
  %sub4.i.i = fadd fast float %39, -1.000000e+00
  %mul.i.i.i = fmul fast float %mul.i.19.i, %mul.i.19.i
  %mul1.i.i.i = fmul fast float %37, 4.000000e+00
  %mul2.i.i.i = fmul fast float %mul1.i.i.i, %sub4.i.i
  %sub.i.i.i = fsub fast float %mul.i.i.i, %mul2.i.i.i
  %cmp.i.i.i = fcmp fast olt float %sub.i.i.i, 0.000000e+00
  br i1 %cmp.i.i.i, label %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.i", label %if.else.i.i.i

if.else.i.i.i:                                    ; preds = %sw.bb.9.i
  %cmp4.i.i.i = fcmp fast oeq float %sub.i.i.i, 0.000000e+00
  br i1 %cmp4.i.i.i, label %if.then.7.i.i.i, label %if.else.9.i.i.i

if.then.7.i.i.i:                                  ; preds = %if.else.i.i.i
  %mul8.i.i.i = fsub fast float -0.000000e+00, %38
  %div.i.i.i = fdiv fast float %mul8.i.i.i, %37
  br label %if.end.19.i.i.i

if.else.9.i.i.i:                                  ; preds = %if.else.i.i.i
  %cmp10.i.i.i = fcmp fast ogt float %mul.i.19.i, 0.000000e+00
  br i1 %cmp10.i.i.i, label %cond.true.i.i.i, label %cond.false.i.i.i

cond.true.i.i.i:                                  ; preds = %if.else.9.i.i.i
  %Sqrt = call float @dx.op.unary.f32(i32 24, float %sub.i.i.i)
  %add.i.i.i = fadd fast float %Sqrt, %mul.i.19.i
  br label %cond.end.i.i.i

cond.false.i.i.i:                                 ; preds = %if.else.9.i.i.i
  %Sqrt108 = call float @dx.op.unary.f32(i32 24, float %sub.i.i.i)
  %sub15.i.i.i = fsub fast float %mul.i.19.i, %Sqrt108
  br label %cond.end.i.i.i

cond.end.i.i.i:                                   ; preds = %cond.false.i.i.i, %cond.true.i.i.i
  %add.sink.i.i.i = phi float [ %add.i.i.i, %cond.true.i.i.i ], [ %sub15.i.i.i, %cond.false.i.i.i ]
  %mul13.i.i.i = fmul fast float %add.sink.i.i.i, -5.000000e-01
  %div17.i.i.i = fdiv fast float %mul13.i.i.i, %37
  %div18.i.i.i = fdiv fast float %sub4.i.i, %mul13.i.i.i
  br label %if.end.19.i.i.i

if.end.19.i.i.i:                                  ; preds = %cond.end.i.i.i, %if.then.7.i.i.i
  %.083 = phi float [ %div.i.i.i, %if.then.7.i.i.i ], [ %div18.i.i.i, %cond.end.i.i.i ]
  %.080 = phi float [ %div.i.i.i, %if.then.7.i.i.i ], [ %div17.i.i.i, %cond.end.i.i.i ]
  %cmp20.i.i.i = fcmp fast ogt float %.080, %.083
  %.184 = select i1 %cmp20.i.i.i, float %.080, float %.083
  %.181 = select i1 %cmp20.i.i.i, float %.083, float %.080
  br label %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.i"

"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.i":    ; preds = %if.end.19.i.i.i, %sw.bb.9.i
  %retval.i.i.i.0 = phi i1 [ false, %sw.bb.9.i ], [ true, %if.end.19.i.i.i ]
  %.285 = phi float [ undef, %sw.bb.9.i ], [ %.184, %if.end.19.i.i.i ]
  %.282 = phi float [ undef, %sw.bb.9.i ], [ %.181, %if.end.19.i.i.i ]
  br i1 %retval.i.i.i.0, label %if.end.i.22.i, label %"\01?IntersectCustomPrimitiveFrontToBack@@YA_NV?$vector@M$02@@0MMAIAMUProceduralPrimitiveAttributes@@@Z.exit"

if.end.i.22.i:                                    ; preds = %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.i"
  %cmp.i.21.i = fcmp fast ogt float %.282, %.285
  %t0.i.i.0 = select i1 %cmp.i.21.i, float %.285, float %.282
  %t1.i.i.0 = select i1 %cmp.i.21.i, float %.282, float %.285
  %cmp9.i.i = fcmp fast olt float %t0.i.i.0, 0.000000e+00
  br i1 %cmp9.i.i, label %if.then.12.i.i, label %if.end.18.i.i

if.then.12.i.i:                                   ; preds = %if.end.i.22.i
  %cmp13.i.i = fcmp fast olt float %t1.i.i.0, 0.000000e+00
  br i1 %cmp13.i.i, label %"\01?IntersectCustomPrimitiveFrontToBack@@YA_NV?$vector@M$02@@0MMAIAMUProceduralPrimitiveAttributes@@@Z.exit", label %if.end.18.i.i

if.end.18.i.i:                                    ; preds = %if.then.12.i.i, %if.end.i.22.i
  %t0.i.i.1 = phi float [ %t1.i.i.0, %if.then.12.i.i ], [ %t0.i.i.0, %if.end.i.22.i ]
  %mul19.i.i.i0 = fmul fast float %t0.i.i.1, %FMad257
  %mul19.i.i.i1 = fmul fast float %t0.i.i.1, %FMad254
  %mul19.i.i.i2 = fmul fast float %t0.i.i.1, %FMad251
  %add.i.25.i.i0 = fadd fast float %mul19.i.i.i0, %FMad221
  %add.i.25.i.i1 = fadd fast float %mul19.i.i.i1, %FMad218
  %add.i.25.i.i2 = fadd fast float %mul19.i.i.i2, %FMad215
  %ObjectToWorld173 = call float @dx.op.objectToWorld.f32(i32 151, i32 0, i8 0)
  %ObjectToWorld174 = call float @dx.op.objectToWorld.f32(i32 151, i32 0, i8 1)
  %ObjectToWorld175 = call float @dx.op.objectToWorld.f32(i32 151, i32 0, i8 2)
  %ObjectToWorld177 = call float @dx.op.objectToWorld.f32(i32 151, i32 1, i8 0)
  %ObjectToWorld178 = call float @dx.op.objectToWorld.f32(i32 151, i32 1, i8 1)
  %ObjectToWorld179 = call float @dx.op.objectToWorld.f32(i32 151, i32 1, i8 2)
  %ObjectToWorld181 = call float @dx.op.objectToWorld.f32(i32 151, i32 2, i8 0)
  %ObjectToWorld182 = call float @dx.op.objectToWorld.f32(i32 151, i32 2, i8 1)
  %ObjectToWorld183 = call float @dx.op.objectToWorld.f32(i32 151, i32 2, i8 2)
  %40 = fmul fast float %add.i.25.i.i0, %add.i.25.i.i0
  %41 = fmul fast float %add.i.25.i.i1, %add.i.25.i.i1
  %42 = fadd fast float %40, %41
  %43 = fmul fast float %add.i.25.i.i2, %add.i.25.i.i2
  %44 = fadd fast float %42, %43
  %Sqrt125 = call float @dx.op.unary.f32(i32 24, float %44)
  %.i0263 = fdiv fast float %add.i.25.i.i0, %Sqrt125
  %.i1264 = fdiv fast float %add.i.25.i.i1, %Sqrt125
  %.i2265 = fdiv fast float %add.i.25.i.i2, %Sqrt125
  %45 = fmul fast float %.i0263, %ObjectToWorld173
  %FMad231 = call float @dx.op.tertiary.f32(i32 46, float %.i1264, float %ObjectToWorld177, float %45)
  %FMad230 = call float @dx.op.tertiary.f32(i32 46, float %.i2265, float %ObjectToWorld181, float %FMad231)
  %46 = fmul fast float %.i0263, %ObjectToWorld174
  %FMad229 = call float @dx.op.tertiary.f32(i32 46, float %.i1264, float %ObjectToWorld178, float %46)
  %FMad228 = call float @dx.op.tertiary.f32(i32 46, float %.i2265, float %ObjectToWorld182, float %FMad229)
  %47 = fmul fast float %.i0263, %ObjectToWorld175
  %FMad227 = call float @dx.op.tertiary.f32(i32 46, float %.i1264, float %ObjectToWorld179, float %47)
  %FMad226 = call float @dx.op.tertiary.f32(i32 46, float %.i2265, float %ObjectToWorld183, float %FMad227)
  br label %"\01?IntersectCustomPrimitiveFrontToBack@@YA_NV?$vector@M$02@@0MMAIAMUProceduralPrimitiveAttributes@@@Z.exit"

sw.bb.11.i:                                       ; preds = %entry
  %sub.i.i.35.i.i0 = fadd fast float %FMad221, 0x3FD3333340000000
  %sub.i.i.35.i.i1 = fadd fast float %FMad218, 0x3FD3333340000000
  %sub.i.i.35.i.i2 = fadd fast float %FMad215, 0x3FD3333340000000
  %48 = call float @dx.op.dot3.f32(i32 55, float %FMad257, float %FMad254, float %FMad251, float %FMad257, float %FMad254, float %FMad251)
  %49 = call float @dx.op.dot3.f32(i32 55, float %FMad257, float %FMad254, float %FMad251, float %sub.i.i.35.i.i0, float %sub.i.i.35.i.i1, float %sub.i.i.35.i.i2)
  %mul.i.i.36.i = fmul fast float %49, 2.000000e+00
  %50 = call float @dx.op.dot3.f32(i32 55, float %sub.i.i.35.i.i0, float %sub.i.i.35.i.i1, float %sub.i.i.35.i.i2, float %sub.i.i.35.i.i0, float %sub.i.i.35.i.i1, float %sub.i.i.35.i.i2)
  %sub4.i.i.i = fadd fast float %50, 0xBFD70A3D80000000
  %mul.i.i.i.i = fmul fast float %mul.i.i.36.i, %mul.i.i.36.i
  %mul1.i.i.i.i = fmul fast float %48, 4.000000e+00
  %mul2.i.i.i.i = fmul fast float %mul1.i.i.i.i, %sub4.i.i.i
  %sub.i.i.i.i = fsub fast float %mul.i.i.i.i, %mul2.i.i.i.i
  %cmp.i.i.i.i = fcmp fast olt float %sub.i.i.i.i, 0.000000e+00
  br i1 %cmp.i.i.i.i, label %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.i.i", label %if.else.i.i.i.i

if.else.i.i.i.i:                                  ; preds = %sw.bb.11.i
  %cmp4.i.i.i.i = fcmp fast oeq float %sub.i.i.i.i, 0.000000e+00
  br i1 %cmp4.i.i.i.i, label %if.then.7.i.i.i.i, label %if.else.9.i.i.i.i

if.then.7.i.i.i.i:                                ; preds = %if.else.i.i.i.i
  %mul8.i.i.i.i = fsub fast float -0.000000e+00, %49
  %div.i.i.i.i = fdiv fast float %mul8.i.i.i.i, %48
  br label %if.end.19.i.i.i.i

if.else.9.i.i.i.i:                                ; preds = %if.else.i.i.i.i
  %cmp10.i.i.i.i = fcmp fast ogt float %mul.i.i.36.i, 0.000000e+00
  br i1 %cmp10.i.i.i.i, label %cond.true.i.i.i.i, label %cond.false.i.i.i.i

cond.true.i.i.i.i:                                ; preds = %if.else.9.i.i.i.i
  %Sqrt109 = call float @dx.op.unary.f32(i32 24, float %sub.i.i.i.i)
  %add.i.i.i.i = fadd fast float %Sqrt109, %mul.i.i.36.i
  br label %cond.end.i.i.i.i

cond.false.i.i.i.i:                               ; preds = %if.else.9.i.i.i.i
  %Sqrt114 = call float @dx.op.unary.f32(i32 24, float %sub.i.i.i.i)
  %sub15.i.i.i.i = fsub fast float %mul.i.i.36.i, %Sqrt114
  br label %cond.end.i.i.i.i

cond.end.i.i.i.i:                                 ; preds = %cond.false.i.i.i.i, %cond.true.i.i.i.i
  %add.sink.i.i.i.i = phi float [ %add.i.i.i.i, %cond.true.i.i.i.i ], [ %sub15.i.i.i.i, %cond.false.i.i.i.i ]
  %mul13.i.i.i.i = fmul fast float %add.sink.i.i.i.i, -5.000000e-01
  %div17.i.i.i.i = fdiv fast float %mul13.i.i.i.i, %48
  %div18.i.i.i.i = fdiv fast float %sub4.i.i.i, %mul13.i.i.i.i
  br label %if.end.19.i.i.i.i

if.end.19.i.i.i.i:                                ; preds = %cond.end.i.i.i.i, %if.then.7.i.i.i.i
  %.098 = phi float [ %div.i.i.i.i, %if.then.7.i.i.i.i ], [ %div18.i.i.i.i, %cond.end.i.i.i.i ]
  %.095 = phi float [ %div.i.i.i.i, %if.then.7.i.i.i.i ], [ %div17.i.i.i.i, %cond.end.i.i.i.i ]
  %cmp20.i.i.i.i = fcmp fast ogt float %.095, %.098
  %.199 = select i1 %cmp20.i.i.i.i, float %.095, float %.098
  %.196 = select i1 %cmp20.i.i.i.i, float %.098, float %.095
  br label %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.i.i"

"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.i.i":  ; preds = %if.end.19.i.i.i.i, %sw.bb.11.i
  %retval.i.i.i.i.0 = phi i1 [ false, %sw.bb.11.i ], [ true, %if.end.19.i.i.i.i ]
  %.2100 = phi float [ undef, %sw.bb.11.i ], [ %.199, %if.end.19.i.i.i.i ]
  %.297 = phi float [ undef, %sw.bb.11.i ], [ %.196, %if.end.19.i.i.i.i ]
  br i1 %retval.i.i.i.i.0, label %if.end.i.i.i, label %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit.i.i"

if.end.i.i.i:                                     ; preds = %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.i.i"
  %cmp.i.i.38.i = fcmp fast ogt float %.297, %.2100
  %t0.i.i.i.0 = select i1 %cmp.i.i.38.i, float %.2100, float %.297
  %t1.i.i.i.0 = select i1 %cmp.i.i.38.i, float %.297, float %.2100
  %cmp9.i.i.i = fcmp fast olt float %t0.i.i.i.0, 0.000000e+00
  br i1 %cmp9.i.i.i, label %if.then.12.i.i.i, label %if.end.18.i.i.i

if.then.12.i.i.i:                                 ; preds = %if.end.i.i.i
  %cmp13.i.i.i = fcmp fast olt float %t1.i.i.i.0, 0.000000e+00
  br i1 %cmp13.i.i.i, label %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit.i.i", label %if.end.18.i.i.i

if.end.18.i.i.i:                                  ; preds = %if.then.12.i.i.i, %if.end.i.i.i
  %t0.i.i.i.1 = phi float [ %t1.i.i.i.0, %if.then.12.i.i.i ], [ %t0.i.i.i.0, %if.end.i.i.i ]
  %mul19.i.i.i.i0 = fmul fast float %t0.i.i.i.1, %FMad257
  %mul19.i.i.i.i1 = fmul fast float %t0.i.i.i.1, %FMad254
  %mul19.i.i.i.i2 = fmul fast float %t0.i.i.i.1, %FMad251
  %ObjectToWorld161 = call float @dx.op.objectToWorld.f32(i32 151, i32 0, i8 0)
  %ObjectToWorld162 = call float @dx.op.objectToWorld.f32(i32 151, i32 0, i8 1)
  %ObjectToWorld163 = call float @dx.op.objectToWorld.f32(i32 151, i32 0, i8 2)
  %ObjectToWorld165 = call float @dx.op.objectToWorld.f32(i32 151, i32 1, i8 0)
  %ObjectToWorld166 = call float @dx.op.objectToWorld.f32(i32 151, i32 1, i8 1)
  %ObjectToWorld167 = call float @dx.op.objectToWorld.f32(i32 151, i32 1, i8 2)
  %ObjectToWorld169 = call float @dx.op.objectToWorld.f32(i32 151, i32 2, i8 0)
  %ObjectToWorld170 = call float @dx.op.objectToWorld.f32(i32 151, i32 2, i8 1)
  %ObjectToWorld171 = call float @dx.op.objectToWorld.f32(i32 151, i32 2, i8 2)
  %sub21.i.i.i.i0 = fadd fast float %sub.i.i.35.i.i0, %mul19.i.i.i.i0
  %sub21.i.i.i.i1 = fadd fast float %sub.i.i.35.i.i1, %mul19.i.i.i.i1
  %sub21.i.i.i.i2 = fadd fast float %sub.i.i.35.i.i2, %mul19.i.i.i.i2
  %51 = fmul fast float %sub21.i.i.i.i0, %sub21.i.i.i.i0
  %52 = fmul fast float %sub21.i.i.i.i1, %sub21.i.i.i.i1
  %53 = fadd fast float %51, %52
  %54 = fmul fast float %sub21.i.i.i.i2, %sub21.i.i.i.i2
  %55 = fadd fast float %53, %54
  %Sqrt126 = call float @dx.op.unary.f32(i32 24, float %55)
  %.i0266 = fdiv fast float %sub21.i.i.i.i0, %Sqrt126
  %.i1267 = fdiv fast float %sub21.i.i.i.i1, %Sqrt126
  %.i2268 = fdiv fast float %sub21.i.i.i.i2, %Sqrt126
  %56 = fmul fast float %.i0266, %ObjectToWorld161
  %FMad239 = call float @dx.op.tertiary.f32(i32 46, float %.i1267, float %ObjectToWorld165, float %56)
  %FMad238 = call float @dx.op.tertiary.f32(i32 46, float %.i2268, float %ObjectToWorld169, float %FMad239)
  %57 = fmul fast float %.i0266, %ObjectToWorld162
  %FMad237 = call float @dx.op.tertiary.f32(i32 46, float %.i1267, float %ObjectToWorld166, float %57)
  %FMad236 = call float @dx.op.tertiary.f32(i32 46, float %.i2268, float %ObjectToWorld170, float %FMad237)
  %58 = fmul fast float %.i0266, %ObjectToWorld163
  %FMad235 = call float @dx.op.tertiary.f32(i32 46, float %.i1267, float %ObjectToWorld167, float %58)
  %FMad234 = call float @dx.op.tertiary.f32(i32 46, float %.i2268, float %ObjectToWorld171, float %FMad235)
  br label %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit.i.i"

"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit.i.i": ; preds = %if.end.18.i.i.i, %if.then.12.i.i.i, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.i.i"
  %retval.i.i.28.i.0 = phi i1 [ true, %if.end.18.i.i.i ], [ false, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.i.i" ], [ false, %if.then.12.i.i.i ]
  %.065.0.i0 = phi float [ %FMad238, %if.end.18.i.i.i ], [ undef, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.i.i" ], [ undef, %if.then.12.i.i.i ]
  %.065.0.i1 = phi float [ %FMad236, %if.end.18.i.i.i ], [ undef, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.i.i" ], [ undef, %if.then.12.i.i.i ]
  %.065.0.i2 = phi float [ %FMad234, %if.end.18.i.i.i ], [ undef, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.i.i" ], [ undef, %if.then.12.i.i.i ]
  %.086 = phi float [ %t0.i.i.i.1, %if.end.18.i.i.i ], [ undef, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.i.i" ], [ undef, %if.then.12.i.i.i ]
  %cmp.i.41.i = fcmp fast olt float %.086, %RayTCurrent
  %or.cond = and i1 %retval.i.i.28.i.0, %cmp.i.41.i
  %.073 = select i1 %or.cond, float %.086, float %RayTCurrent
  %sub.i.52.i.i.i2 = fadd fast float %FMad215, 0xBFD99999A0000000
  %59 = call float @dx.op.dot3.f32(i32 55, float %FMad257, float %FMad254, float %FMad251, float %FMad221, float %FMad218, float %sub.i.52.i.i.i2)
  %mul.i.53.i.i = fmul fast float %59, 2.000000e+00
  %60 = call float @dx.op.dot3.f32(i32 55, float %FMad221, float %FMad218, float %sub.i.52.i.i.i2, float %FMad221, float %FMad218, float %sub.i.52.i.i.i2)
  %sub4.i.54.i.i = fadd fast float %60, 0xBFB70A3D80000000
  %mul.i.i.55.i.i = fmul fast float %mul.i.53.i.i, %mul.i.53.i.i
  %mul2.i.i.57.i.i = fmul fast float %mul1.i.i.i.i, %sub4.i.54.i.i
  %sub.i.i.58.i.i = fsub fast float %mul.i.i.55.i.i, %mul2.i.i.57.i.i
  %cmp.i.i.59.i.i = fcmp fast olt float %sub.i.i.58.i.i, 0.000000e+00
  br i1 %cmp.i.i.59.i.i, label %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.81.i.i", label %if.else.i.i.62.i.i

if.else.i.i.62.i.i:                               ; preds = %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit.i.i"
  %cmp4.i.i.61.i.i = fcmp fast oeq float %sub.i.i.58.i.i, 0.000000e+00
  br i1 %cmp4.i.i.61.i.i, label %if.then.7.i.i.65.i.i, label %if.else.9.i.i.67.i.i

if.then.7.i.i.65.i.i:                             ; preds = %if.else.i.i.62.i.i
  %mul8.i.i.63.i.i = fsub fast float -0.000000e+00, %59
  %div.i.i.64.i.i = fdiv fast float %mul8.i.i.63.i.i, %48
  br label %if.end.19.i.i.78.i.i

if.else.9.i.i.67.i.i:                             ; preds = %if.else.i.i.62.i.i
  %cmp10.i.i.66.i.i = fcmp fast ogt float %mul.i.53.i.i, 0.000000e+00
  br i1 %cmp10.i.i.66.i.i, label %cond.true.i.i.69.i.i, label %cond.false.i.i.71.i.i

cond.true.i.i.69.i.i:                             ; preds = %if.else.9.i.i.67.i.i
  %Sqrt110 = call float @dx.op.unary.f32(i32 24, float %sub.i.i.58.i.i)
  %add.i.i.68.i.i = fadd fast float %Sqrt110, %mul.i.53.i.i
  br label %cond.end.i.i.76.i.i

cond.false.i.i.71.i.i:                            ; preds = %if.else.9.i.i.67.i.i
  %Sqrt113 = call float @dx.op.unary.f32(i32 24, float %sub.i.i.58.i.i)
  %sub15.i.i.70.i.i = fsub fast float %mul.i.53.i.i, %Sqrt113
  br label %cond.end.i.i.76.i.i

cond.end.i.i.76.i.i:                              ; preds = %cond.false.i.i.71.i.i, %cond.true.i.i.69.i.i
  %add.sink.i.i.72.i.i = phi float [ %add.i.i.68.i.i, %cond.true.i.i.69.i.i ], [ %sub15.i.i.70.i.i, %cond.false.i.i.71.i.i ]
  %mul13.i.i.73.i.i = fmul fast float %add.sink.i.i.72.i.i, -5.000000e-01
  %div17.i.i.74.i.i = fdiv fast float %mul13.i.i.73.i.i, %48
  %div18.i.i.75.i.i = fdiv fast float %sub4.i.54.i.i, %mul13.i.i.73.i.i
  br label %if.end.19.i.i.78.i.i

if.end.19.i.i.78.i.i:                             ; preds = %cond.end.i.i.76.i.i, %if.then.7.i.i.65.i.i
  %.092 = phi float [ %div.i.i.64.i.i, %if.then.7.i.i.65.i.i ], [ %div17.i.i.74.i.i, %cond.end.i.i.76.i.i ]
  %.089 = phi float [ %div.i.i.64.i.i, %if.then.7.i.i.65.i.i ], [ %div18.i.i.75.i.i, %cond.end.i.i.76.i.i ]
  %cmp20.i.i.77.i.i = fcmp fast ogt float %.092, %.089
  %.193 = select i1 %cmp20.i.i.77.i.i, float %.089, float %.092
  %.190 = select i1 %cmp20.i.i.77.i.i, float %.092, float %.089
  br label %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.81.i.i"

"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.81.i.i": ; preds = %if.end.19.i.i.78.i.i, %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit.i.i"
  %.294 = phi float [ undef, %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit.i.i" ], [ %.193, %if.end.19.i.i.78.i.i ]
  %.291 = phi float [ undef, %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit.i.i" ], [ %.190, %if.end.19.i.i.78.i.i ]
  %retval.i.i.32.i.i.0 = phi i1 [ false, %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit.i.i" ], [ true, %if.end.19.i.i.78.i.i ]
  br i1 %retval.i.i.32.i.i.0, label %if.end.i.84.i.i, label %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit98.i.i"

if.end.i.84.i.i:                                  ; preds = %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.81.i.i"
  %cmp.i.83.i.i = fcmp fast ogt float %.294, %.291
  %t0.i.43.i.i.0 = select i1 %cmp.i.83.i.i, float %.291, float %.294
  %t1.i.44.i.i.0 = select i1 %cmp.i.83.i.i, float %.294, float %.291
  %cmp9.i.86.i.i = fcmp fast olt float %t0.i.43.i.i.0, 0.000000e+00
  br i1 %cmp9.i.86.i.i, label %if.then.12.i.89.i.i, label %if.end.18.i.97.i.i

if.then.12.i.89.i.i:                              ; preds = %if.end.i.84.i.i
  %cmp13.i.88.i.i = fcmp fast olt float %t1.i.44.i.i.0, 0.000000e+00
  br i1 %cmp13.i.88.i.i, label %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit98.i.i", label %if.end.18.i.97.i.i

if.end.18.i.97.i.i:                               ; preds = %if.then.12.i.89.i.i, %if.end.i.84.i.i
  %t0.i.43.i.i.1 = phi float [ %t1.i.44.i.i.0, %if.then.12.i.89.i.i ], [ %t0.i.43.i.i.0, %if.end.i.84.i.i ]
  %mul19.i.93.i.i.i0 = fmul fast float %t0.i.43.i.i.1, %FMad257
  %mul19.i.93.i.i.i1 = fmul fast float %t0.i.43.i.i.1, %FMad254
  %mul19.i.93.i.i.i2 = fmul fast float %t0.i.43.i.i.1, %FMad251
  %ObjectToWorld149 = call float @dx.op.objectToWorld.f32(i32 151, i32 0, i8 0)
  %ObjectToWorld150 = call float @dx.op.objectToWorld.f32(i32 151, i32 0, i8 1)
  %ObjectToWorld151 = call float @dx.op.objectToWorld.f32(i32 151, i32 0, i8 2)
  %ObjectToWorld153 = call float @dx.op.objectToWorld.f32(i32 151, i32 1, i8 0)
  %ObjectToWorld154 = call float @dx.op.objectToWorld.f32(i32 151, i32 1, i8 1)
  %ObjectToWorld155 = call float @dx.op.objectToWorld.f32(i32 151, i32 1, i8 2)
  %ObjectToWorld157 = call float @dx.op.objectToWorld.f32(i32 151, i32 2, i8 0)
  %ObjectToWorld158 = call float @dx.op.objectToWorld.f32(i32 151, i32 2, i8 1)
  %ObjectToWorld159 = call float @dx.op.objectToWorld.f32(i32 151, i32 2, i8 2)
  %sub21.i.95.i.i.i0 = fadd fast float %FMad221, %mul19.i.93.i.i.i0
  %sub21.i.95.i.i.i1 = fadd fast float %FMad218, %mul19.i.93.i.i.i1
  %sub21.i.95.i.i.i2 = fadd fast float %sub.i.52.i.i.i2, %mul19.i.93.i.i.i2
  %61 = fmul fast float %sub21.i.95.i.i.i0, %sub21.i.95.i.i.i0
  %62 = fmul fast float %sub21.i.95.i.i.i1, %sub21.i.95.i.i.i1
  %63 = fadd fast float %61, %62
  %64 = fmul fast float %sub21.i.95.i.i.i2, %sub21.i.95.i.i.i2
  %65 = fadd fast float %63, %64
  %Sqrt127 = call float @dx.op.unary.f32(i32 24, float %65)
  %.i0269 = fdiv fast float %sub21.i.95.i.i.i0, %Sqrt127
  %.i1270 = fdiv fast float %sub21.i.95.i.i.i1, %Sqrt127
  %.i2271 = fdiv fast float %sub21.i.95.i.i.i2, %Sqrt127
  %66 = fmul fast float %.i0269, %ObjectToWorld149
  %FMad203 = call float @dx.op.tertiary.f32(i32 46, float %.i1270, float %ObjectToWorld153, float %66)
  %FMad202 = call float @dx.op.tertiary.f32(i32 46, float %.i2271, float %ObjectToWorld157, float %FMad203)
  %67 = fmul fast float %.i0269, %ObjectToWorld150
  %FMad201 = call float @dx.op.tertiary.f32(i32 46, float %.i1270, float %ObjectToWorld154, float %67)
  %FMad200 = call float @dx.op.tertiary.f32(i32 46, float %.i2271, float %ObjectToWorld158, float %FMad201)
  %68 = fmul fast float %.i0269, %ObjectToWorld151
  %FMad199 = call float @dx.op.tertiary.f32(i32 46, float %.i1270, float %ObjectToWorld155, float %68)
  %FMad198 = call float @dx.op.tertiary.f32(i32 46, float %.i2271, float %ObjectToWorld159, float %FMad199)
  br label %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit98.i.i"

"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit98.i.i": ; preds = %if.end.18.i.97.i.i, %if.then.12.i.89.i.i, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.81.i.i"
  %retval.i.38.i.i.0 = phi i1 [ true, %if.end.18.i.97.i.i ], [ false, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.81.i.i" ], [ false, %if.then.12.i.89.i.i ]
  %.064.0.i0 = phi float [ %FMad202, %if.end.18.i.97.i.i ], [ %.065.0.i0, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.81.i.i" ], [ %.065.0.i0, %if.then.12.i.89.i.i ]
  %.064.0.i1 = phi float [ %FMad200, %if.end.18.i.97.i.i ], [ %.065.0.i1, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.81.i.i" ], [ %.065.0.i1, %if.then.12.i.89.i.i ]
  %.064.0.i2 = phi float [ %FMad198, %if.end.18.i.97.i.i ], [ %.065.0.i2, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.81.i.i" ], [ %.065.0.i2, %if.then.12.i.89.i.i ]
  %.087 = phi float [ %t0.i.43.i.i.1, %if.end.18.i.97.i.i ], [ %.086, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.81.i.i" ], [ %.086, %if.then.12.i.89.i.i ]
  %cmp13.i.45.i = fcmp fast olt float %.087, %.073
  %or.cond101 = and i1 %retval.i.38.i.i.0, %cmp13.i.45.i
  %.066.1.i0 = select i1 %or.cond101, float %.064.0.i0, float %.065.0.i0
  %.066.1.i1 = select i1 %or.cond101, float %.064.0.i1, float %.065.0.i1
  %.066.1.i2 = select i1 %or.cond101, float %.064.0.i2, float %.065.0.i2
  %.1 = select i1 %or.cond101, float %.087, float %.073
  %sub.i.121.i.i.i0 = fadd fast float %FMad221, 0xBFD3333340000000
  %sub.i.121.i.i.i1 = fadd fast float %FMad218, 0xBFD3333340000000
  %69 = call float @dx.op.dot3.f32(i32 55, float %FMad257, float %FMad254, float %FMad251, float %sub.i.121.i.i.i0, float %sub.i.121.i.i.i1, float %FMad215)
  %mul.i.122.i.i = fmul fast float %69, 2.000000e+00
  %70 = call float @dx.op.dot3.f32(i32 55, float %sub.i.121.i.i.i0, float %sub.i.121.i.i.i1, float %FMad215, float %sub.i.121.i.i.i0, float %sub.i.121.i.i.i1, float %FMad215)
  %sub4.i.123.i.i = fadd fast float %70, 0xBF970A3DC0000000
  %mul.i.i.124.i.i = fmul fast float %mul.i.122.i.i, %mul.i.122.i.i
  %mul2.i.i.126.i.i = fmul fast float %mul1.i.i.i.i, %sub4.i.123.i.i
  %sub.i.i.127.i.i = fsub fast float %mul.i.i.124.i.i, %mul2.i.i.126.i.i
  %cmp.i.i.128.i.i = fcmp fast olt float %sub.i.i.127.i.i, 0.000000e+00
  br i1 %cmp.i.i.128.i.i, label %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.150.i.i", label %if.else.i.i.131.i.i

if.else.i.i.131.i.i:                              ; preds = %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit98.i.i"
  %cmp4.i.i.130.i.i = fcmp fast oeq float %sub.i.i.127.i.i, 0.000000e+00
  br i1 %cmp4.i.i.130.i.i, label %if.then.7.i.i.134.i.i, label %if.else.9.i.i.136.i.i

if.then.7.i.i.134.i.i:                            ; preds = %if.else.i.i.131.i.i
  %mul8.i.i.132.i.i = fsub fast float -0.000000e+00, %69
  %div.i.i.133.i.i = fdiv fast float %mul8.i.i.132.i.i, %48
  br label %if.end.19.i.i.147.i.i

if.else.9.i.i.136.i.i:                            ; preds = %if.else.i.i.131.i.i
  %cmp10.i.i.135.i.i = fcmp fast ogt float %mul.i.122.i.i, 0.000000e+00
  br i1 %cmp10.i.i.135.i.i, label %cond.true.i.i.138.i.i, label %cond.false.i.i.140.i.i

cond.true.i.i.138.i.i:                            ; preds = %if.else.9.i.i.136.i.i
  %Sqrt111 = call float @dx.op.unary.f32(i32 24, float %sub.i.i.127.i.i)
  %add.i.i.137.i.i = fadd fast float %Sqrt111, %mul.i.122.i.i
  br label %cond.end.i.i.145.i.i

cond.false.i.i.140.i.i:                           ; preds = %if.else.9.i.i.136.i.i
  %Sqrt112 = call float @dx.op.unary.f32(i32 24, float %sub.i.i.127.i.i)
  %sub15.i.i.139.i.i = fsub fast float %mul.i.122.i.i, %Sqrt112
  br label %cond.end.i.i.145.i.i

cond.end.i.i.145.i.i:                             ; preds = %cond.false.i.i.140.i.i, %cond.true.i.i.138.i.i
  %add.sink.i.i.141.i.i = phi float [ %add.i.i.137.i.i, %cond.true.i.i.138.i.i ], [ %sub15.i.i.139.i.i, %cond.false.i.i.140.i.i ]
  %mul13.i.i.142.i.i = fmul fast float %add.sink.i.i.141.i.i, -5.000000e-01
  %div17.i.i.143.i.i = fdiv fast float %mul13.i.i.142.i.i, %48
  %div18.i.i.144.i.i = fdiv fast float %sub4.i.123.i.i, %mul13.i.i.142.i.i
  br label %if.end.19.i.i.147.i.i

if.end.19.i.i.147.i.i:                            ; preds = %cond.end.i.i.145.i.i, %if.then.7.i.i.134.i.i
  %.077 = phi float [ %div.i.i.133.i.i, %if.then.7.i.i.134.i.i ], [ %div17.i.i.143.i.i, %cond.end.i.i.145.i.i ]
  %.074 = phi float [ %div.i.i.133.i.i, %if.then.7.i.i.134.i.i ], [ %div18.i.i.144.i.i, %cond.end.i.i.145.i.i ]
  %cmp20.i.i.146.i.i = fcmp fast ogt float %.077, %.074
  %.178 = select i1 %cmp20.i.i.146.i.i, float %.074, float %.077
  %.175 = select i1 %cmp20.i.i.146.i.i, float %.077, float %.074
  br label %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.150.i.i"

"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.150.i.i": ; preds = %if.end.19.i.i.147.i.i, %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit98.i.i"
  %.279 = phi float [ undef, %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit98.i.i" ], [ %.178, %if.end.19.i.i.147.i.i ]
  %.276 = phi float [ undef, %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit98.i.i" ], [ %.175, %if.end.19.i.i.147.i.i ]
  %retval.i.i.101.i.i.0 = phi i1 [ false, %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit98.i.i" ], [ true, %if.end.19.i.i.147.i.i ]
  br i1 %retval.i.i.101.i.i.0, label %if.end.i.153.i.i, label %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit167.i.i"

if.end.i.153.i.i:                                 ; preds = %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.150.i.i"
  %cmp.i.152.i.i = fcmp fast ogt float %.279, %.276
  %t1.i.113.i.i.0 = select i1 %cmp.i.152.i.i, float %.279, float %.276
  %t0.i.112.i.i.0 = select i1 %cmp.i.152.i.i, float %.276, float %.279
  %cmp9.i.155.i.i = fcmp fast olt float %t0.i.112.i.i.0, 0.000000e+00
  br i1 %cmp9.i.155.i.i, label %if.then.12.i.158.i.i, label %if.end.18.i.166.i.i

if.then.12.i.158.i.i:                             ; preds = %if.end.i.153.i.i
  %cmp13.i.157.i.i = fcmp fast olt float %t1.i.113.i.i.0, 0.000000e+00
  br i1 %cmp13.i.157.i.i, label %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit167.i.i", label %if.end.18.i.166.i.i

if.end.18.i.166.i.i:                              ; preds = %if.then.12.i.158.i.i, %if.end.i.153.i.i
  %t0.i.112.i.i.1 = phi float [ %t1.i.113.i.i.0, %if.then.12.i.158.i.i ], [ %t0.i.112.i.i.0, %if.end.i.153.i.i ]
  %mul19.i.162.i.i.i0 = fmul fast float %t0.i.112.i.i.1, %FMad257
  %mul19.i.162.i.i.i1 = fmul fast float %t0.i.112.i.i.1, %FMad254
  %mul19.i.162.i.i.i2 = fmul fast float %t0.i.112.i.i.1, %FMad251
  %ObjectToWorld = call float @dx.op.objectToWorld.f32(i32 151, i32 0, i8 0)
  %ObjectToWorld138 = call float @dx.op.objectToWorld.f32(i32 151, i32 0, i8 1)
  %ObjectToWorld139 = call float @dx.op.objectToWorld.f32(i32 151, i32 0, i8 2)
  %ObjectToWorld141 = call float @dx.op.objectToWorld.f32(i32 151, i32 1, i8 0)
  %ObjectToWorld142 = call float @dx.op.objectToWorld.f32(i32 151, i32 1, i8 1)
  %ObjectToWorld143 = call float @dx.op.objectToWorld.f32(i32 151, i32 1, i8 2)
  %ObjectToWorld145 = call float @dx.op.objectToWorld.f32(i32 151, i32 2, i8 0)
  %ObjectToWorld146 = call float @dx.op.objectToWorld.f32(i32 151, i32 2, i8 1)
  %ObjectToWorld147 = call float @dx.op.objectToWorld.f32(i32 151, i32 2, i8 2)
  %sub21.i.164.i.i.i0 = fadd fast float %sub.i.121.i.i.i0, %mul19.i.162.i.i.i0
  %sub21.i.164.i.i.i1 = fadd fast float %sub.i.121.i.i.i1, %mul19.i.162.i.i.i1
  %sub21.i.164.i.i.i2 = fadd fast float %FMad215, %mul19.i.162.i.i.i2
  %71 = fmul fast float %sub21.i.164.i.i.i0, %sub21.i.164.i.i.i0
  %72 = fmul fast float %sub21.i.164.i.i.i1, %sub21.i.164.i.i.i1
  %73 = fadd fast float %71, %72
  %74 = fmul fast float %sub21.i.164.i.i.i2, %sub21.i.164.i.i.i2
  %75 = fadd fast float %73, %74
  %Sqrt128 = call float @dx.op.unary.f32(i32 24, float %75)
  %.i0272 = fdiv fast float %sub21.i.164.i.i.i0, %Sqrt128
  %.i1273 = fdiv fast float %sub21.i.164.i.i.i1, %Sqrt128
  %.i2274 = fdiv fast float %sub21.i.164.i.i.i2, %Sqrt128
  %76 = fmul fast float %.i0272, %ObjectToWorld
  %FMad247 = call float @dx.op.tertiary.f32(i32 46, float %.i1273, float %ObjectToWorld141, float %76)
  %FMad246 = call float @dx.op.tertiary.f32(i32 46, float %.i2274, float %ObjectToWorld145, float %FMad247)
  %77 = fmul fast float %.i0272, %ObjectToWorld138
  %FMad245 = call float @dx.op.tertiary.f32(i32 46, float %.i1273, float %ObjectToWorld142, float %77)
  %FMad244 = call float @dx.op.tertiary.f32(i32 46, float %.i2274, float %ObjectToWorld146, float %FMad245)
  %78 = fmul fast float %.i0272, %ObjectToWorld139
  %FMad243 = call float @dx.op.tertiary.f32(i32 46, float %.i1273, float %ObjectToWorld143, float %78)
  %FMad242 = call float @dx.op.tertiary.f32(i32 46, float %.i2274, float %ObjectToWorld147, float %FMad243)
  br label %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit167.i.i"

"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit167.i.i": ; preds = %if.end.18.i.166.i.i, %if.then.12.i.158.i.i, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.150.i.i"
  %.0.0.i0 = phi float [ %FMad246, %if.end.18.i.166.i.i ], [ %.064.0.i0, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.150.i.i" ], [ %.064.0.i0, %if.then.12.i.158.i.i ]
  %.0.0.i1 = phi float [ %FMad244, %if.end.18.i.166.i.i ], [ %.064.0.i1, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.150.i.i" ], [ %.064.0.i1, %if.then.12.i.158.i.i ]
  %.0.0.i2 = phi float [ %FMad242, %if.end.18.i.166.i.i ], [ %.064.0.i2, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.150.i.i" ], [ %.064.0.i2, %if.then.12.i.158.i.i ]
  %.088 = phi float [ %t0.i.112.i.i.1, %if.end.18.i.166.i.i ], [ %.087, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.150.i.i" ], [ %.087, %if.then.12.i.158.i.i ]
  %retval.i.107.i.i.0 = phi i1 [ true, %if.end.18.i.166.i.i ], [ false, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.150.i.i" ], [ false, %if.then.12.i.158.i.i ]
  %cmp23.i.i = fcmp fast olt float %.088, %.1
  %or.cond102 = and i1 %retval.i.107.i.i.0, %cmp23.i.i
  %79 = or i1 %or.cond101, %or.cond102
  %.066.2.i0 = select i1 %or.cond102, float %.0.0.i0, float %.066.1.i0
  %.066.2.i1 = select i1 %or.cond102, float %.0.0.i1, float %.066.1.i1
  %.066.2.i2 = select i1 %or.cond102, float %.0.0.i2, float %.066.1.i2
  %.2 = select i1 %or.cond102, float %.088, float %.1
  %tobool29.i.i = or i1 %or.cond, %79
  br label %"\01?IntersectCustomPrimitiveFrontToBack@@YA_NV?$vector@M$02@@0MMAIAMUProceduralPrimitiveAttributes@@@Z.exit"

"\01?IntersectCustomPrimitiveFrontToBack@@YA_NV?$vector@M$02@@0MMAIAMUProceduralPrimitiveAttributes@@@Z.exit": ; preds = %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit167.i.i", %if.end.18.i.i, %if.then.12.i.i, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.i", %"\01?intersectBox@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@@Z.exit.i", %entry
  %retval.i.0 = phi i1 [ %tobool29.i.i, %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit167.i.i" ], [ %cmp63.i.i, %"\01?intersectBox@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@@Z.exit.i" ], [ true, %if.end.18.i.i ], [ false, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.i" ], [ false, %if.then.12.i.i ], [ false, %entry ]
  %.069.0.i0 = phi float [ %.066.2.i0, %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit167.i.i" ], [ %FMad210, %"\01?intersectBox@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@@Z.exit.i" ], [ %FMad230, %if.end.18.i.i ], [ undef, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.i" ], [ undef, %if.then.12.i.i ], [ undef, %entry ]
  %.069.0.i1 = phi float [ %.066.2.i1, %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit167.i.i" ], [ %FMad208, %"\01?intersectBox@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@@Z.exit.i" ], [ %FMad228, %if.end.18.i.i ], [ undef, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.i" ], [ undef, %if.then.12.i.i ], [ undef, %entry ]
  %.069.0.i2 = phi float [ %.066.2.i2, %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit167.i.i" ], [ %FMad206, %"\01?intersectBox@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@@Z.exit.i" ], [ %FMad226, %if.end.18.i.i ], [ undef, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.i" ], [ undef, %if.then.12.i.i ], [ undef, %entry ]
  %.071 = phi float [ %.2, %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit167.i.i" ], [ %FMax, %"\01?intersectBox@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@@Z.exit.i" ], [ %t0.i.i.1, %if.end.18.i.i ], [ %RayTCurrent, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.i" ], [ %RayTCurrent, %if.then.12.i.i ], [ %RayTCurrent, %entry ]
  %.069.0.upto0 = insertelement <3 x float> undef, float %.069.0.i0, i32 0
  %.069.0.upto1 = insertelement <3 x float> %.069.0.upto0, float %.069.0.i1, i32 1
  %.069.0 = insertelement <3 x float> %.069.0.upto1, float %.069.0.i2, i32 2
  %80 = getelementptr inbounds %struct.ProceduralPrimitiveAttributes, %struct.ProceduralPrimitiveAttributes* %attr, i32 0, i32 0
  store <3 x float> %.069.0, <3 x float>* %80, align 4
  br i1 %retval.i.0, label %if.then, label %if.end

if.then:                                          ; preds = %"\01?IntersectCustomPrimitiveFrontToBack@@YA_NV?$vector@M$02@@0MMAIAMUProceduralPrimitiveAttributes@@@Z.exit"
  %81 = call i1 @dx.op.reportHit.struct.ProceduralPrimitiveAttributes(i32 158, float %.071, i32 0, %struct.ProceduralPrimitiveAttributes* nonnull %attr)
  br label %if.end

if.end:                                           ; preds = %if.then, %"\01?IntersectCustomPrimitiveFrontToBack@@YA_NV?$vector@M$02@@0MMAIAMUProceduralPrimitiveAttributes@@@Z.exit"
  ret void
}

; Function Attrs: alwaysinline nounwind readonly
define i32 @"\01?GetInstanceContributionToHitGroupIndex@@YAIURaytracingInstanceDesc@@@Z"(%struct.RaytracingInstanceDesc* nocapture readonly) #6 {
entry:
  %InstanceContributionToHitGroupIndexAndFlags = getelementptr inbounds %struct.RaytracingInstanceDesc, %struct.RaytracingInstanceDesc* %0, i32 0, i32 2
  %1 = load i32, i32* %InstanceContributionToHitGroupIndexAndFlags, align 4, !tbaa !223
  %and = and i32 %1, 16777215
  ret i32 %and
}

; Function Attrs: alwaysinline nounwind readnone
define void @"\01?LogTraceRayEnd@@YAXXZ"() #4 {
entry:
  ret void
}

; Function Attrs: alwaysinline nounwind
define void @"\01?GetProceduralPrimitiveAABB@@YA?AUAABB@@UPrimitive@@@Z"(%struct.AABB* noalias nocapture sret, %struct.Primitive* nocapture readonly) #5 {
entry:
  %data1 = getelementptr inbounds %struct.Primitive, %struct.Primitive* %1, i32 0, i32 2
  %2 = load <4 x i32>, <4 x i32>* %data1, align 4
  %data0 = getelementptr inbounds %struct.Primitive, %struct.Primitive* %1, i32 0, i32 1
  %3 = load <4 x i32>, <4 x i32>* %data0, align 4, !tbaa !220
  %4 = extractelement <4 x i32> %3, i32 2
  %5 = extractelement <4 x i32> %3, i32 1
  %6 = extractelement <4 x i32> %3, i32 0
  %.i03 = bitcast i32 %6 to float
  %.i14 = bitcast i32 %5 to float
  %.i25 = bitcast i32 %4 to float
  %.upto0 = insertelement <3 x float> undef, float %.i03, i32 0
  %.upto1 = insertelement <3 x float> %.upto0, float %.i14, i32 1
  %7 = insertelement <3 x float> %.upto1, float %.i25, i32 2
  %min.i = getelementptr inbounds %struct.AABB, %struct.AABB* %0, i32 0, i32 0
  store <3 x float> %7, <3 x float>* %min.i, align 4, !tbaa !220, !alias.scope !279
  %8 = extractelement <4 x i32> %3, i32 3
  %9 = extractelement <4 x i32> %2, i32 1
  %10 = extractelement <4 x i32> %2, i32 0
  %.i06 = bitcast i32 %8 to float
  %.i17 = bitcast i32 %10 to float
  %.i28 = bitcast i32 %9 to float
  %.upto011 = insertelement <3 x float> undef, float %.i06, i32 0
  %.upto112 = insertelement <3 x float> %.upto011, float %.i17, i32 1
  %11 = insertelement <3 x float> %.upto112, float %.i28, i32 2
  %max.i = getelementptr inbounds %struct.AABB, %struct.AABB* %0, i32 0, i32 1
  store <3 x float> %11, <3 x float>* %max.i, align 4, !tbaa !220, !alias.scope !279
  ret void
}

; Function Attrs: alwaysinline nounwind
define void @"\01?MyIntersectionShader_AABB@@YAXXZ"() #5 {
entry:
  %0 = load %"class.StructuredBuffer<AABBPrimitiveAttributes>", %"class.StructuredBuffer<AABBPrimitiveAttributes>"* @"\01?g_AABBPrimitiveAttributes@@3V?$StructuredBuffer@UAABBPrimitiveAttributes@@@@A", align 4
  %attr = alloca %struct.ProceduralPrimitiveAttributes, align 4
  %RayTCurrent = call float @dx.op.rayTCurrent.f32(i32 154)
  %ObjectRayDirection = call float @dx.op.objectRayDirection.f32(i32 150, i8 0)
  %ObjectRayDirection131 = call float @dx.op.objectRayDirection.f32(i32 150, i8 1)
  %ObjectRayDirection132 = call float @dx.op.objectRayDirection.f32(i32 150, i8 2)
  %ObjectRayOrigin = call float @dx.op.objectRayOrigin.f32(i32 149, i8 0)
  %ObjectRayOrigin129 = call float @dx.op.objectRayOrigin.f32(i32 149, i8 1)
  %ObjectRayOrigin130 = call float @dx.op.objectRayOrigin.f32(i32 149, i8 2)
  %PrimitiveID137 = call i32 @dx.op.primitiveID.i32(i32 108)
  %1 = call %dx.types.Handle @"dx.op.createHandleFromResourceStructForLib.class.StructuredBuffer<AABBPrimitiveAttributes>"(i32 160, %"class.StructuredBuffer<AABBPrimitiveAttributes>" %0)
  %2 = call %dx.types.ResRet.f32 @dx.op.bufferLoad.f32(i32 68, %dx.types.Handle %1, i32 %PrimitiveID137, i32 0)
  %3 = extractvalue %dx.types.ResRet.f32 %2, 0
  %4 = extractvalue %dx.types.ResRet.f32 %2, 1
  %5 = extractvalue %dx.types.ResRet.f32 %2, 2
  %6 = call %dx.types.ResRet.f32 @dx.op.bufferLoad.f32(i32 68, %dx.types.Handle %1, i32 %PrimitiveID137, i32 16)
  %7 = extractvalue %dx.types.ResRet.f32 %6, 0
  %8 = extractvalue %dx.types.ResRet.f32 %6, 1
  %9 = extractvalue %dx.types.ResRet.f32 %6, 2
  %10 = call %dx.types.ResRet.f32 @dx.op.bufferLoad.f32(i32 68, %dx.types.Handle %1, i32 %PrimitiveID137, i32 32)
  %11 = extractvalue %dx.types.ResRet.f32 %10, 0
  %12 = extractvalue %dx.types.ResRet.f32 %10, 1
  %13 = extractvalue %dx.types.ResRet.f32 %10, 2
  %14 = call %dx.types.ResRet.f32 @dx.op.bufferLoad.f32(i32 68, %dx.types.Handle %1, i32 %PrimitiveID137, i32 48)
  %15 = extractvalue %dx.types.ResRet.f32 %14, 0
  %16 = extractvalue %dx.types.ResRet.f32 %14, 1
  %17 = extractvalue %dx.types.ResRet.f32 %14, 2
  %18 = fmul fast float %3, %ObjectRayOrigin
  %FMad259 = call float @dx.op.tertiary.f32(i32 46, float %ObjectRayOrigin129, float %7, float %18)
  %FMad258 = call float @dx.op.tertiary.f32(i32 46, float %ObjectRayOrigin130, float %11, float %FMad259)
  %FMad257 = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %15, float %FMad258)
  %19 = fmul fast float %4, %ObjectRayOrigin
  %FMad256 = call float @dx.op.tertiary.f32(i32 46, float %ObjectRayOrigin129, float %8, float %19)
  %FMad255 = call float @dx.op.tertiary.f32(i32 46, float %ObjectRayOrigin130, float %12, float %FMad256)
  %FMad254 = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %16, float %FMad255)
  %20 = fmul fast float %5, %ObjectRayOrigin
  %FMad253 = call float @dx.op.tertiary.f32(i32 46, float %ObjectRayOrigin129, float %9, float %20)
  %FMad252 = call float @dx.op.tertiary.f32(i32 46, float %ObjectRayOrigin130, float %13, float %FMad253)
  %FMad251 = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %17, float %FMad252)
  %21 = fmul fast float %3, %ObjectRayDirection
  %FMad247 = call float @dx.op.tertiary.f32(i32 46, float %ObjectRayDirection131, float %7, float %21)
  %FMad246 = call float @dx.op.tertiary.f32(i32 46, float %ObjectRayDirection132, float %11, float %FMad247)
  %FMad245 = call float @dx.op.tertiary.f32(i32 46, float 0.000000e+00, float %15, float %FMad246)
  %22 = fmul fast float %4, %ObjectRayDirection
  %FMad244 = call float @dx.op.tertiary.f32(i32 46, float %ObjectRayDirection131, float %8, float %22)
  %FMad243 = call float @dx.op.tertiary.f32(i32 46, float %ObjectRayDirection132, float %12, float %FMad244)
  %FMad242 = call float @dx.op.tertiary.f32(i32 46, float 0.000000e+00, float %16, float %FMad243)
  %23 = fmul fast float %5, %ObjectRayDirection
  %FMad241 = call float @dx.op.tertiary.f32(i32 46, float %ObjectRayDirection131, float %9, float %23)
  %FMad240 = call float @dx.op.tertiary.f32(i32 46, float %ObjectRayDirection132, float %13, float %FMad241)
  %FMad239 = call float @dx.op.tertiary.f32(i32 46, float 0.000000e+00, float %17, float %FMad240)
  %rem.i = urem i32 %PrimitiveID137, 3
  switch i32 %rem.i, label %"\01?IntersectCustomPrimitiveFrontToBack@@YA_NV?$vector@M$02@@0MMAIAMUProceduralPrimitiveAttributes@@@Z.exit" [
    i32 0, label %sw.bb.i
    i32 2, label %sw.bb.9.i
    i32 1, label %sw.bb.11.i
  ]

sw.bb.i:                                          ; preds = %entry
  %sub.i.i = fsub fast float -1.000000e+00, %FMad257
  %div.i.i = fdiv fast float %sub.i.i, %FMad245
  %sub2.i.i = fsub fast float 1.000000e+00, %FMad257
  %div3.i.i = fdiv fast float %sub2.i.i, %FMad245
  %cmp.i.i = fcmp fast olt float %FMad245, 0.000000e+00
  %tmin.i.i.0 = select i1 %cmp.i.i, float %div3.i.i, float %div.i.i
  %tmax.i.i.0 = select i1 %cmp.i.i, float %div.i.i, float %div3.i.i
  %sub6.i.i = fsub fast float -1.000000e+00, %FMad254
  %div7.i.i = fdiv fast float %sub6.i.i, %FMad242
  %sub9.i.i = fsub fast float 1.000000e+00, %FMad254
  %div10.i.i = fdiv fast float %sub9.i.i, %FMad242
  %cmp11.i.i = fcmp fast olt float %FMad242, 0.000000e+00
  %tymin.i.i.0 = select i1 %cmp11.i.i, float %div10.i.i, float %div7.i.i
  %tymax.i.i.0 = select i1 %cmp11.i.i, float %div7.i.i, float %div10.i.i
  %sub17.i.i = fsub fast float -1.000000e+00, %FMad251
  %div18.i.i = fdiv fast float %sub17.i.i, %FMad239
  %sub20.i.i = fsub fast float 1.000000e+00, %FMad251
  %div21.i.i = fdiv fast float %sub20.i.i, %FMad239
  %cmp22.i.i = fcmp fast olt float %FMad239, 0.000000e+00
  %tzmin.i.i.0 = select i1 %cmp22.i.i, float %div21.i.i, float %div18.i.i
  %tzmax.i.i.0 = select i1 %cmp22.i.i, float %div18.i.i, float %div21.i.i
  %FMax116 = call float @dx.op.binary.f32(i32 35, float %tmin.i.i.0, float %tymin.i.i.0)
  %FMax = call float @dx.op.binary.f32(i32 35, float %FMax116, float %tzmin.i.i.0)
  %FMin115 = call float @dx.op.binary.f32(i32 36, float %tmax.i.i.0, float %tymax.i.i.0)
  %FMin = call float @dx.op.binary.f32(i32 36, float %FMin115, float %tzmax.i.i.0)
  %mul.i.i.i0 = fmul fast float %FMax, %FMad245
  %mul.i.i.i1 = fmul fast float %FMax, %FMad242
  %mul.i.i.i2 = fmul fast float %FMax, %FMad239
  %add.i.i.i0 = fadd fast float %mul.i.i.i0, %FMad257
  %add.i.i.i1 = fadd fast float %mul.i.i.i1, %FMad254
  %add.i.i.i2 = fadd fast float %mul.i.i.i2, %FMad251
  %24 = fmul fast float %add.i.i.i0, %add.i.i.i0
  %25 = fmul fast float %add.i.i.i1, %add.i.i.i1
  %26 = fadd fast float %24, %25
  %27 = fmul fast float %add.i.i.i2, %add.i.i.i2
  %28 = fadd fast float %26, %27
  %Sqrt124 = call float @dx.op.unary.f32(i32 24, float %28)
  %.i0 = fdiv fast float %add.i.i.i0, %Sqrt124
  %.i1 = fdiv fast float %add.i.i.i1, %Sqrt124
  %.i2 = fdiv fast float %add.i.i.i2, %Sqrt124
  %FAbs107 = call float @dx.op.unary.f32(i32 6, float %.i0)
  %FAbs106 = call float @dx.op.unary.f32(i32 6, float %.i1)
  %cmp34.i.i = fcmp fast ogt float %FAbs107, %FAbs106
  br i1 %cmp34.i.i, label %if.then.37.i.i, label %if.else.46.i.i

if.then.37.i.i:                                   ; preds = %sw.bb.i
  %FAbs = call float @dx.op.unary.f32(i32 6, float %.i2)
  %cmp40.i.i = fcmp fast ogt float %FAbs107, %FAbs
  br i1 %cmp40.i.i, label %"\01?intersectBox@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@@Z.exit.i", label %if.else.i.i

if.else.i.i:                                      ; preds = %if.then.37.i.i
  br label %"\01?intersectBox@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@@Z.exit.i"

if.else.46.i.i:                                   ; preds = %sw.bb.i
  %FAbs104 = call float @dx.op.unary.f32(i32 6, float %.i2)
  %cmp49.i.i = fcmp fast ogt float %FAbs106, %FAbs104
  br i1 %cmp49.i.i, label %"\01?intersectBox@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@@Z.exit.i", label %if.else.54.i.i

if.else.54.i.i:                                   ; preds = %if.else.46.i.i
  br label %"\01?intersectBox@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@@Z.exit.i"

"\01?intersectBox@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@@Z.exit.i": ; preds = %if.else.54.i.i, %if.else.46.i.i, %if.else.i.i, %if.then.37.i.i
  %.068.0.i0 = phi float [ 0.000000e+00, %if.else.i.i ], [ 0.000000e+00, %if.else.54.i.i ], [ %.i0, %if.then.37.i.i ], [ 0.000000e+00, %if.else.46.i.i ]
  %.068.0.i1 = phi float [ 0.000000e+00, %if.else.i.i ], [ 0.000000e+00, %if.else.54.i.i ], [ 0.000000e+00, %if.then.37.i.i ], [ %.i1, %if.else.46.i.i ]
  %.068.0.i2 = phi float [ %.i2, %if.else.i.i ], [ %.i2, %if.else.54.i.i ], [ 0.000000e+00, %if.then.37.i.i ], [ 0.000000e+00, %if.else.46.i.i ]
  %ObjectToWorld185 = call float @dx.op.objectToWorld.f32(i32 151, i32 0, i8 0)
  %ObjectToWorld186 = call float @dx.op.objectToWorld.f32(i32 151, i32 0, i8 1)
  %ObjectToWorld187 = call float @dx.op.objectToWorld.f32(i32 151, i32 0, i8 2)
  %ObjectToWorld189 = call float @dx.op.objectToWorld.f32(i32 151, i32 1, i8 0)
  %ObjectToWorld190 = call float @dx.op.objectToWorld.f32(i32 151, i32 1, i8 1)
  %ObjectToWorld191 = call float @dx.op.objectToWorld.f32(i32 151, i32 1, i8 2)
  %ObjectToWorld193 = call float @dx.op.objectToWorld.f32(i32 151, i32 2, i8 0)
  %ObjectToWorld194 = call float @dx.op.objectToWorld.f32(i32 151, i32 2, i8 1)
  %ObjectToWorld195 = call float @dx.op.objectToWorld.f32(i32 151, i32 2, i8 2)
  %29 = fmul fast float %.068.0.i0, %.068.0.i0
  %30 = fmul fast float %.068.0.i1, %.068.0.i1
  %31 = fadd fast float %30, %29
  %32 = fmul fast float %.068.0.i2, %.068.0.i2
  %33 = fadd fast float %31, %32
  %Sqrt123 = call float @dx.op.unary.f32(i32 24, float %33)
  %.i0260 = fdiv fast float %.068.0.i0, %Sqrt123
  %.i1261 = fdiv fast float %.068.0.i1, %Sqrt123
  %.i2262 = fdiv fast float %.068.0.i2, %Sqrt123
  %34 = fmul fast float %.i0260, %ObjectToWorld185
  %FMad203 = call float @dx.op.tertiary.f32(i32 46, float %.i1261, float %ObjectToWorld189, float %34)
  %FMad202 = call float @dx.op.tertiary.f32(i32 46, float %.i2262, float %ObjectToWorld193, float %FMad203)
  %35 = fmul fast float %.i0260, %ObjectToWorld186
  %FMad201 = call float @dx.op.tertiary.f32(i32 46, float %.i1261, float %ObjectToWorld190, float %35)
  %FMad200 = call float @dx.op.tertiary.f32(i32 46, float %.i2262, float %ObjectToWorld194, float %FMad201)
  %36 = fmul fast float %.i0260, %ObjectToWorld187
  %FMad199 = call float @dx.op.tertiary.f32(i32 46, float %.i1261, float %ObjectToWorld191, float %36)
  %FMad198 = call float @dx.op.tertiary.f32(i32 46, float %.i2262, float %ObjectToWorld195, float %FMad199)
  %cmp63.i.i = fcmp fast ogt float %FMin, %FMax
  br label %"\01?IntersectCustomPrimitiveFrontToBack@@YA_NV?$vector@M$02@@0MMAIAMUProceduralPrimitiveAttributes@@@Z.exit"

sw.bb.9.i:                                        ; preds = %entry
  %37 = call float @dx.op.dot3.f32(i32 55, float %FMad245, float %FMad242, float %FMad239, float %FMad245, float %FMad242, float %FMad239)
  %38 = call float @dx.op.dot3.f32(i32 55, float %FMad245, float %FMad242, float %FMad239, float %FMad257, float %FMad254, float %FMad251)
  %mul.i.19.i = fmul fast float %38, 2.000000e+00
  %39 = call float @dx.op.dot3.f32(i32 55, float %FMad257, float %FMad254, float %FMad251, float %FMad257, float %FMad254, float %FMad251)
  %sub4.i.i = fadd fast float %39, -1.000000e+00
  %mul.i.i.i = fmul fast float %mul.i.19.i, %mul.i.19.i
  %mul1.i.i.i = fmul fast float %37, 4.000000e+00
  %mul2.i.i.i = fmul fast float %mul1.i.i.i, %sub4.i.i
  %sub.i.i.i = fsub fast float %mul.i.i.i, %mul2.i.i.i
  %cmp.i.i.i = fcmp fast olt float %sub.i.i.i, 0.000000e+00
  br i1 %cmp.i.i.i, label %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.i", label %if.else.i.i.i

if.else.i.i.i:                                    ; preds = %sw.bb.9.i
  %cmp4.i.i.i = fcmp fast oeq float %sub.i.i.i, 0.000000e+00
  br i1 %cmp4.i.i.i, label %if.then.7.i.i.i, label %if.else.9.i.i.i

if.then.7.i.i.i:                                  ; preds = %if.else.i.i.i
  %mul8.i.i.i = fsub fast float -0.000000e+00, %38
  %div.i.i.i = fdiv fast float %mul8.i.i.i, %37
  br label %if.end.19.i.i.i

if.else.9.i.i.i:                                  ; preds = %if.else.i.i.i
  %cmp10.i.i.i = fcmp fast ogt float %mul.i.19.i, 0.000000e+00
  br i1 %cmp10.i.i.i, label %cond.true.i.i.i, label %cond.false.i.i.i

cond.true.i.i.i:                                  ; preds = %if.else.9.i.i.i
  %Sqrt = call float @dx.op.unary.f32(i32 24, float %sub.i.i.i)
  %add.i.i.i = fadd fast float %Sqrt, %mul.i.19.i
  br label %cond.end.i.i.i

cond.false.i.i.i:                                 ; preds = %if.else.9.i.i.i
  %Sqrt108 = call float @dx.op.unary.f32(i32 24, float %sub.i.i.i)
  %sub15.i.i.i = fsub fast float %mul.i.19.i, %Sqrt108
  br label %cond.end.i.i.i

cond.end.i.i.i:                                   ; preds = %cond.false.i.i.i, %cond.true.i.i.i
  %add.sink.i.i.i = phi float [ %add.i.i.i, %cond.true.i.i.i ], [ %sub15.i.i.i, %cond.false.i.i.i ]
  %mul13.i.i.i = fmul fast float %add.sink.i.i.i, -5.000000e-01
  %div17.i.i.i = fdiv fast float %mul13.i.i.i, %37
  %div18.i.i.i = fdiv fast float %sub4.i.i, %mul13.i.i.i
  br label %if.end.19.i.i.i

if.end.19.i.i.i:                                  ; preds = %cond.end.i.i.i, %if.then.7.i.i.i
  %.083 = phi float [ %div.i.i.i, %if.then.7.i.i.i ], [ %div18.i.i.i, %cond.end.i.i.i ]
  %.080 = phi float [ %div.i.i.i, %if.then.7.i.i.i ], [ %div17.i.i.i, %cond.end.i.i.i ]
  %cmp20.i.i.i = fcmp fast ogt float %.080, %.083
  %.184 = select i1 %cmp20.i.i.i, float %.080, float %.083
  %.181 = select i1 %cmp20.i.i.i, float %.083, float %.080
  br label %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.i"

"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.i":    ; preds = %if.end.19.i.i.i, %sw.bb.9.i
  %retval.i.i.i.0 = phi i1 [ false, %sw.bb.9.i ], [ true, %if.end.19.i.i.i ]
  %.285 = phi float [ undef, %sw.bb.9.i ], [ %.184, %if.end.19.i.i.i ]
  %.282 = phi float [ undef, %sw.bb.9.i ], [ %.181, %if.end.19.i.i.i ]
  br i1 %retval.i.i.i.0, label %if.end.i.22.i, label %"\01?IntersectCustomPrimitiveFrontToBack@@YA_NV?$vector@M$02@@0MMAIAMUProceduralPrimitiveAttributes@@@Z.exit"

if.end.i.22.i:                                    ; preds = %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.i"
  %cmp.i.21.i = fcmp fast ogt float %.282, %.285
  %t0.i.i.0 = select i1 %cmp.i.21.i, float %.285, float %.282
  %t1.i.i.0 = select i1 %cmp.i.21.i, float %.282, float %.285
  %cmp9.i.i = fcmp fast olt float %t0.i.i.0, 0.000000e+00
  br i1 %cmp9.i.i, label %if.then.12.i.i, label %if.end.18.i.i

if.then.12.i.i:                                   ; preds = %if.end.i.22.i
  %cmp13.i.i = fcmp fast olt float %t1.i.i.0, 0.000000e+00
  br i1 %cmp13.i.i, label %"\01?IntersectCustomPrimitiveFrontToBack@@YA_NV?$vector@M$02@@0MMAIAMUProceduralPrimitiveAttributes@@@Z.exit", label %if.end.18.i.i

if.end.18.i.i:                                    ; preds = %if.then.12.i.i, %if.end.i.22.i
  %t0.i.i.1 = phi float [ %t1.i.i.0, %if.then.12.i.i ], [ %t0.i.i.0, %if.end.i.22.i ]
  %mul19.i.i.i0 = fmul fast float %t0.i.i.1, %FMad245
  %mul19.i.i.i1 = fmul fast float %t0.i.i.1, %FMad242
  %mul19.i.i.i2 = fmul fast float %t0.i.i.1, %FMad239
  %add.i.25.i.i0 = fadd fast float %mul19.i.i.i0, %FMad257
  %add.i.25.i.i1 = fadd fast float %mul19.i.i.i1, %FMad254
  %add.i.25.i.i2 = fadd fast float %mul19.i.i.i2, %FMad251
  %ObjectToWorld173 = call float @dx.op.objectToWorld.f32(i32 151, i32 0, i8 0)
  %ObjectToWorld174 = call float @dx.op.objectToWorld.f32(i32 151, i32 0, i8 1)
  %ObjectToWorld175 = call float @dx.op.objectToWorld.f32(i32 151, i32 0, i8 2)
  %ObjectToWorld177 = call float @dx.op.objectToWorld.f32(i32 151, i32 1, i8 0)
  %ObjectToWorld178 = call float @dx.op.objectToWorld.f32(i32 151, i32 1, i8 1)
  %ObjectToWorld179 = call float @dx.op.objectToWorld.f32(i32 151, i32 1, i8 2)
  %ObjectToWorld181 = call float @dx.op.objectToWorld.f32(i32 151, i32 2, i8 0)
  %ObjectToWorld182 = call float @dx.op.objectToWorld.f32(i32 151, i32 2, i8 1)
  %ObjectToWorld183 = call float @dx.op.objectToWorld.f32(i32 151, i32 2, i8 2)
  %40 = fmul fast float %add.i.25.i.i0, %add.i.25.i.i0
  %41 = fmul fast float %add.i.25.i.i1, %add.i.25.i.i1
  %42 = fadd fast float %40, %41
  %43 = fmul fast float %add.i.25.i.i2, %add.i.25.i.i2
  %44 = fadd fast float %42, %43
  %Sqrt125 = call float @dx.op.unary.f32(i32 24, float %44)
  %.i0263 = fdiv fast float %add.i.25.i.i0, %Sqrt125
  %.i1264 = fdiv fast float %add.i.25.i.i1, %Sqrt125
  %.i2265 = fdiv fast float %add.i.25.i.i2, %Sqrt125
  %45 = fmul fast float %.i0263, %ObjectToWorld173
  %FMad227 = call float @dx.op.tertiary.f32(i32 46, float %.i1264, float %ObjectToWorld177, float %45)
  %FMad226 = call float @dx.op.tertiary.f32(i32 46, float %.i2265, float %ObjectToWorld181, float %FMad227)
  %46 = fmul fast float %.i0263, %ObjectToWorld174
  %FMad225 = call float @dx.op.tertiary.f32(i32 46, float %.i1264, float %ObjectToWorld178, float %46)
  %FMad224 = call float @dx.op.tertiary.f32(i32 46, float %.i2265, float %ObjectToWorld182, float %FMad225)
  %47 = fmul fast float %.i0263, %ObjectToWorld175
  %FMad223 = call float @dx.op.tertiary.f32(i32 46, float %.i1264, float %ObjectToWorld179, float %47)
  %FMad222 = call float @dx.op.tertiary.f32(i32 46, float %.i2265, float %ObjectToWorld183, float %FMad223)
  br label %"\01?IntersectCustomPrimitiveFrontToBack@@YA_NV?$vector@M$02@@0MMAIAMUProceduralPrimitiveAttributes@@@Z.exit"

sw.bb.11.i:                                       ; preds = %entry
  %sub.i.i.35.i.i0 = fadd fast float %FMad257, 0x3FD3333340000000
  %sub.i.i.35.i.i1 = fadd fast float %FMad254, 0x3FD3333340000000
  %sub.i.i.35.i.i2 = fadd fast float %FMad251, 0x3FD3333340000000
  %48 = call float @dx.op.dot3.f32(i32 55, float %FMad245, float %FMad242, float %FMad239, float %FMad245, float %FMad242, float %FMad239)
  %49 = call float @dx.op.dot3.f32(i32 55, float %FMad245, float %FMad242, float %FMad239, float %sub.i.i.35.i.i0, float %sub.i.i.35.i.i1, float %sub.i.i.35.i.i2)
  %mul.i.i.36.i = fmul fast float %49, 2.000000e+00
  %50 = call float @dx.op.dot3.f32(i32 55, float %sub.i.i.35.i.i0, float %sub.i.i.35.i.i1, float %sub.i.i.35.i.i2, float %sub.i.i.35.i.i0, float %sub.i.i.35.i.i1, float %sub.i.i.35.i.i2)
  %sub4.i.i.i = fadd fast float %50, 0xBFD70A3D80000000
  %mul.i.i.i.i = fmul fast float %mul.i.i.36.i, %mul.i.i.36.i
  %mul1.i.i.i.i = fmul fast float %48, 4.000000e+00
  %mul2.i.i.i.i = fmul fast float %mul1.i.i.i.i, %sub4.i.i.i
  %sub.i.i.i.i = fsub fast float %mul.i.i.i.i, %mul2.i.i.i.i
  %cmp.i.i.i.i = fcmp fast olt float %sub.i.i.i.i, 0.000000e+00
  br i1 %cmp.i.i.i.i, label %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.i.i", label %if.else.i.i.i.i

if.else.i.i.i.i:                                  ; preds = %sw.bb.11.i
  %cmp4.i.i.i.i = fcmp fast oeq float %sub.i.i.i.i, 0.000000e+00
  br i1 %cmp4.i.i.i.i, label %if.then.7.i.i.i.i, label %if.else.9.i.i.i.i

if.then.7.i.i.i.i:                                ; preds = %if.else.i.i.i.i
  %mul8.i.i.i.i = fsub fast float -0.000000e+00, %49
  %div.i.i.i.i = fdiv fast float %mul8.i.i.i.i, %48
  br label %if.end.19.i.i.i.i

if.else.9.i.i.i.i:                                ; preds = %if.else.i.i.i.i
  %cmp10.i.i.i.i = fcmp fast ogt float %mul.i.i.36.i, 0.000000e+00
  br i1 %cmp10.i.i.i.i, label %cond.true.i.i.i.i, label %cond.false.i.i.i.i

cond.true.i.i.i.i:                                ; preds = %if.else.9.i.i.i.i
  %Sqrt109 = call float @dx.op.unary.f32(i32 24, float %sub.i.i.i.i)
  %add.i.i.i.i = fadd fast float %Sqrt109, %mul.i.i.36.i
  br label %cond.end.i.i.i.i

cond.false.i.i.i.i:                               ; preds = %if.else.9.i.i.i.i
  %Sqrt114 = call float @dx.op.unary.f32(i32 24, float %sub.i.i.i.i)
  %sub15.i.i.i.i = fsub fast float %mul.i.i.36.i, %Sqrt114
  br label %cond.end.i.i.i.i

cond.end.i.i.i.i:                                 ; preds = %cond.false.i.i.i.i, %cond.true.i.i.i.i
  %add.sink.i.i.i.i = phi float [ %add.i.i.i.i, %cond.true.i.i.i.i ], [ %sub15.i.i.i.i, %cond.false.i.i.i.i ]
  %mul13.i.i.i.i = fmul fast float %add.sink.i.i.i.i, -5.000000e-01
  %div17.i.i.i.i = fdiv fast float %mul13.i.i.i.i, %48
  %div18.i.i.i.i = fdiv fast float %sub4.i.i.i, %mul13.i.i.i.i
  br label %if.end.19.i.i.i.i

if.end.19.i.i.i.i:                                ; preds = %cond.end.i.i.i.i, %if.then.7.i.i.i.i
  %.098 = phi float [ %div.i.i.i.i, %if.then.7.i.i.i.i ], [ %div18.i.i.i.i, %cond.end.i.i.i.i ]
  %.095 = phi float [ %div.i.i.i.i, %if.then.7.i.i.i.i ], [ %div17.i.i.i.i, %cond.end.i.i.i.i ]
  %cmp20.i.i.i.i = fcmp fast ogt float %.095, %.098
  %.199 = select i1 %cmp20.i.i.i.i, float %.095, float %.098
  %.196 = select i1 %cmp20.i.i.i.i, float %.098, float %.095
  br label %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.i.i"

"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.i.i":  ; preds = %if.end.19.i.i.i.i, %sw.bb.11.i
  %retval.i.i.i.i.0 = phi i1 [ false, %sw.bb.11.i ], [ true, %if.end.19.i.i.i.i ]
  %.2100 = phi float [ undef, %sw.bb.11.i ], [ %.199, %if.end.19.i.i.i.i ]
  %.297 = phi float [ undef, %sw.bb.11.i ], [ %.196, %if.end.19.i.i.i.i ]
  br i1 %retval.i.i.i.i.0, label %if.end.i.i.i, label %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit.i.i"

if.end.i.i.i:                                     ; preds = %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.i.i"
  %cmp.i.i.38.i = fcmp fast ogt float %.297, %.2100
  %t0.i.i.i.0 = select i1 %cmp.i.i.38.i, float %.2100, float %.297
  %t1.i.i.i.0 = select i1 %cmp.i.i.38.i, float %.297, float %.2100
  %cmp9.i.i.i = fcmp fast olt float %t0.i.i.i.0, 0.000000e+00
  br i1 %cmp9.i.i.i, label %if.then.12.i.i.i, label %if.end.18.i.i.i

if.then.12.i.i.i:                                 ; preds = %if.end.i.i.i
  %cmp13.i.i.i = fcmp fast olt float %t1.i.i.i.0, 0.000000e+00
  br i1 %cmp13.i.i.i, label %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit.i.i", label %if.end.18.i.i.i

if.end.18.i.i.i:                                  ; preds = %if.then.12.i.i.i, %if.end.i.i.i
  %t0.i.i.i.1 = phi float [ %t1.i.i.i.0, %if.then.12.i.i.i ], [ %t0.i.i.i.0, %if.end.i.i.i ]
  %mul19.i.i.i.i0 = fmul fast float %t0.i.i.i.1, %FMad245
  %mul19.i.i.i.i1 = fmul fast float %t0.i.i.i.1, %FMad242
  %mul19.i.i.i.i2 = fmul fast float %t0.i.i.i.1, %FMad239
  %ObjectToWorld161 = call float @dx.op.objectToWorld.f32(i32 151, i32 0, i8 0)
  %ObjectToWorld162 = call float @dx.op.objectToWorld.f32(i32 151, i32 0, i8 1)
  %ObjectToWorld163 = call float @dx.op.objectToWorld.f32(i32 151, i32 0, i8 2)
  %ObjectToWorld165 = call float @dx.op.objectToWorld.f32(i32 151, i32 1, i8 0)
  %ObjectToWorld166 = call float @dx.op.objectToWorld.f32(i32 151, i32 1, i8 1)
  %ObjectToWorld167 = call float @dx.op.objectToWorld.f32(i32 151, i32 1, i8 2)
  %ObjectToWorld169 = call float @dx.op.objectToWorld.f32(i32 151, i32 2, i8 0)
  %ObjectToWorld170 = call float @dx.op.objectToWorld.f32(i32 151, i32 2, i8 1)
  %ObjectToWorld171 = call float @dx.op.objectToWorld.f32(i32 151, i32 2, i8 2)
  %sub21.i.i.i.i0 = fadd fast float %sub.i.i.35.i.i0, %mul19.i.i.i.i0
  %sub21.i.i.i.i1 = fadd fast float %sub.i.i.35.i.i1, %mul19.i.i.i.i1
  %sub21.i.i.i.i2 = fadd fast float %sub.i.i.35.i.i2, %mul19.i.i.i.i2
  %51 = fmul fast float %sub21.i.i.i.i0, %sub21.i.i.i.i0
  %52 = fmul fast float %sub21.i.i.i.i1, %sub21.i.i.i.i1
  %53 = fadd fast float %51, %52
  %54 = fmul fast float %sub21.i.i.i.i2, %sub21.i.i.i.i2
  %55 = fadd fast float %53, %54
  %Sqrt126 = call float @dx.op.unary.f32(i32 24, float %55)
  %.i0266 = fdiv fast float %sub21.i.i.i.i0, %Sqrt126
  %.i1267 = fdiv fast float %sub21.i.i.i.i1, %Sqrt126
  %.i2268 = fdiv fast float %sub21.i.i.i.i2, %Sqrt126
  %56 = fmul fast float %.i0266, %ObjectToWorld161
  %FMad235 = call float @dx.op.tertiary.f32(i32 46, float %.i1267, float %ObjectToWorld165, float %56)
  %FMad234 = call float @dx.op.tertiary.f32(i32 46, float %.i2268, float %ObjectToWorld169, float %FMad235)
  %57 = fmul fast float %.i0266, %ObjectToWorld162
  %FMad233 = call float @dx.op.tertiary.f32(i32 46, float %.i1267, float %ObjectToWorld166, float %57)
  %FMad232 = call float @dx.op.tertiary.f32(i32 46, float %.i2268, float %ObjectToWorld170, float %FMad233)
  %58 = fmul fast float %.i0266, %ObjectToWorld163
  %FMad231 = call float @dx.op.tertiary.f32(i32 46, float %.i1267, float %ObjectToWorld167, float %58)
  %FMad230 = call float @dx.op.tertiary.f32(i32 46, float %.i2268, float %ObjectToWorld171, float %FMad231)
  br label %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit.i.i"

"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit.i.i": ; preds = %if.end.18.i.i.i, %if.then.12.i.i.i, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.i.i"
  %retval.i.i.28.i.0 = phi i1 [ true, %if.end.18.i.i.i ], [ false, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.i.i" ], [ false, %if.then.12.i.i.i ]
  %.065.0.i0 = phi float [ %FMad234, %if.end.18.i.i.i ], [ undef, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.i.i" ], [ undef, %if.then.12.i.i.i ]
  %.065.0.i1 = phi float [ %FMad232, %if.end.18.i.i.i ], [ undef, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.i.i" ], [ undef, %if.then.12.i.i.i ]
  %.065.0.i2 = phi float [ %FMad230, %if.end.18.i.i.i ], [ undef, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.i.i" ], [ undef, %if.then.12.i.i.i ]
  %.086 = phi float [ %t0.i.i.i.1, %if.end.18.i.i.i ], [ undef, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.i.i" ], [ undef, %if.then.12.i.i.i ]
  %cmp.i.41.i = fcmp fast olt float %.086, %RayTCurrent
  %or.cond = and i1 %retval.i.i.28.i.0, %cmp.i.41.i
  %.073 = select i1 %or.cond, float %.086, float %RayTCurrent
  %sub.i.52.i.i.i2 = fadd fast float %FMad251, 0xBFD99999A0000000
  %59 = call float @dx.op.dot3.f32(i32 55, float %FMad245, float %FMad242, float %FMad239, float %FMad257, float %FMad254, float %sub.i.52.i.i.i2)
  %mul.i.53.i.i = fmul fast float %59, 2.000000e+00
  %60 = call float @dx.op.dot3.f32(i32 55, float %FMad257, float %FMad254, float %sub.i.52.i.i.i2, float %FMad257, float %FMad254, float %sub.i.52.i.i.i2)
  %sub4.i.54.i.i = fadd fast float %60, 0xBFB70A3D80000000
  %mul.i.i.55.i.i = fmul fast float %mul.i.53.i.i, %mul.i.53.i.i
  %mul2.i.i.57.i.i = fmul fast float %mul1.i.i.i.i, %sub4.i.54.i.i
  %sub.i.i.58.i.i = fsub fast float %mul.i.i.55.i.i, %mul2.i.i.57.i.i
  %cmp.i.i.59.i.i = fcmp fast olt float %sub.i.i.58.i.i, 0.000000e+00
  br i1 %cmp.i.i.59.i.i, label %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.81.i.i", label %if.else.i.i.62.i.i

if.else.i.i.62.i.i:                               ; preds = %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit.i.i"
  %cmp4.i.i.61.i.i = fcmp fast oeq float %sub.i.i.58.i.i, 0.000000e+00
  br i1 %cmp4.i.i.61.i.i, label %if.then.7.i.i.65.i.i, label %if.else.9.i.i.67.i.i

if.then.7.i.i.65.i.i:                             ; preds = %if.else.i.i.62.i.i
  %mul8.i.i.63.i.i = fsub fast float -0.000000e+00, %59
  %div.i.i.64.i.i = fdiv fast float %mul8.i.i.63.i.i, %48
  br label %if.end.19.i.i.78.i.i

if.else.9.i.i.67.i.i:                             ; preds = %if.else.i.i.62.i.i
  %cmp10.i.i.66.i.i = fcmp fast ogt float %mul.i.53.i.i, 0.000000e+00
  br i1 %cmp10.i.i.66.i.i, label %cond.true.i.i.69.i.i, label %cond.false.i.i.71.i.i

cond.true.i.i.69.i.i:                             ; preds = %if.else.9.i.i.67.i.i
  %Sqrt110 = call float @dx.op.unary.f32(i32 24, float %sub.i.i.58.i.i)
  %add.i.i.68.i.i = fadd fast float %Sqrt110, %mul.i.53.i.i
  br label %cond.end.i.i.76.i.i

cond.false.i.i.71.i.i:                            ; preds = %if.else.9.i.i.67.i.i
  %Sqrt113 = call float @dx.op.unary.f32(i32 24, float %sub.i.i.58.i.i)
  %sub15.i.i.70.i.i = fsub fast float %mul.i.53.i.i, %Sqrt113
  br label %cond.end.i.i.76.i.i

cond.end.i.i.76.i.i:                              ; preds = %cond.false.i.i.71.i.i, %cond.true.i.i.69.i.i
  %add.sink.i.i.72.i.i = phi float [ %add.i.i.68.i.i, %cond.true.i.i.69.i.i ], [ %sub15.i.i.70.i.i, %cond.false.i.i.71.i.i ]
  %mul13.i.i.73.i.i = fmul fast float %add.sink.i.i.72.i.i, -5.000000e-01
  %div17.i.i.74.i.i = fdiv fast float %mul13.i.i.73.i.i, %48
  %div18.i.i.75.i.i = fdiv fast float %sub4.i.54.i.i, %mul13.i.i.73.i.i
  br label %if.end.19.i.i.78.i.i

if.end.19.i.i.78.i.i:                             ; preds = %cond.end.i.i.76.i.i, %if.then.7.i.i.65.i.i
  %.092 = phi float [ %div.i.i.64.i.i, %if.then.7.i.i.65.i.i ], [ %div17.i.i.74.i.i, %cond.end.i.i.76.i.i ]
  %.089 = phi float [ %div.i.i.64.i.i, %if.then.7.i.i.65.i.i ], [ %div18.i.i.75.i.i, %cond.end.i.i.76.i.i ]
  %cmp20.i.i.77.i.i = fcmp fast ogt float %.092, %.089
  %.193 = select i1 %cmp20.i.i.77.i.i, float %.089, float %.092
  %.190 = select i1 %cmp20.i.i.77.i.i, float %.092, float %.089
  br label %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.81.i.i"

"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.81.i.i": ; preds = %if.end.19.i.i.78.i.i, %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit.i.i"
  %.294 = phi float [ undef, %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit.i.i" ], [ %.193, %if.end.19.i.i.78.i.i ]
  %.291 = phi float [ undef, %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit.i.i" ], [ %.190, %if.end.19.i.i.78.i.i ]
  %retval.i.i.32.i.i.0 = phi i1 [ false, %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit.i.i" ], [ true, %if.end.19.i.i.78.i.i ]
  br i1 %retval.i.i.32.i.i.0, label %if.end.i.84.i.i, label %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit98.i.i"

if.end.i.84.i.i:                                  ; preds = %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.81.i.i"
  %cmp.i.83.i.i = fcmp fast ogt float %.294, %.291
  %t0.i.43.i.i.0 = select i1 %cmp.i.83.i.i, float %.291, float %.294
  %t1.i.44.i.i.0 = select i1 %cmp.i.83.i.i, float %.294, float %.291
  %cmp9.i.86.i.i = fcmp fast olt float %t0.i.43.i.i.0, 0.000000e+00
  br i1 %cmp9.i.86.i.i, label %if.then.12.i.89.i.i, label %if.end.18.i.97.i.i

if.then.12.i.89.i.i:                              ; preds = %if.end.i.84.i.i
  %cmp13.i.88.i.i = fcmp fast olt float %t1.i.44.i.i.0, 0.000000e+00
  br i1 %cmp13.i.88.i.i, label %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit98.i.i", label %if.end.18.i.97.i.i

if.end.18.i.97.i.i:                               ; preds = %if.then.12.i.89.i.i, %if.end.i.84.i.i
  %t0.i.43.i.i.1 = phi float [ %t1.i.44.i.i.0, %if.then.12.i.89.i.i ], [ %t0.i.43.i.i.0, %if.end.i.84.i.i ]
  %mul19.i.93.i.i.i0 = fmul fast float %t0.i.43.i.i.1, %FMad245
  %mul19.i.93.i.i.i1 = fmul fast float %t0.i.43.i.i.1, %FMad242
  %mul19.i.93.i.i.i2 = fmul fast float %t0.i.43.i.i.1, %FMad239
  %ObjectToWorld149 = call float @dx.op.objectToWorld.f32(i32 151, i32 0, i8 0)
  %ObjectToWorld150 = call float @dx.op.objectToWorld.f32(i32 151, i32 0, i8 1)
  %ObjectToWorld151 = call float @dx.op.objectToWorld.f32(i32 151, i32 0, i8 2)
  %ObjectToWorld153 = call float @dx.op.objectToWorld.f32(i32 151, i32 1, i8 0)
  %ObjectToWorld154 = call float @dx.op.objectToWorld.f32(i32 151, i32 1, i8 1)
  %ObjectToWorld155 = call float @dx.op.objectToWorld.f32(i32 151, i32 1, i8 2)
  %ObjectToWorld157 = call float @dx.op.objectToWorld.f32(i32 151, i32 2, i8 0)
  %ObjectToWorld158 = call float @dx.op.objectToWorld.f32(i32 151, i32 2, i8 1)
  %ObjectToWorld159 = call float @dx.op.objectToWorld.f32(i32 151, i32 2, i8 2)
  %sub21.i.95.i.i.i0 = fadd fast float %FMad257, %mul19.i.93.i.i.i0
  %sub21.i.95.i.i.i1 = fadd fast float %FMad254, %mul19.i.93.i.i.i1
  %sub21.i.95.i.i.i2 = fadd fast float %sub.i.52.i.i.i2, %mul19.i.93.i.i.i2
  %61 = fmul fast float %sub21.i.95.i.i.i0, %sub21.i.95.i.i.i0
  %62 = fmul fast float %sub21.i.95.i.i.i1, %sub21.i.95.i.i.i1
  %63 = fadd fast float %61, %62
  %64 = fmul fast float %sub21.i.95.i.i.i2, %sub21.i.95.i.i.i2
  %65 = fadd fast float %63, %64
  %Sqrt127 = call float @dx.op.unary.f32(i32 24, float %65)
  %.i0269 = fdiv fast float %sub21.i.95.i.i.i0, %Sqrt127
  %.i1270 = fdiv fast float %sub21.i.95.i.i.i1, %Sqrt127
  %.i2271 = fdiv fast float %sub21.i.95.i.i.i2, %Sqrt127
  %66 = fmul fast float %.i0269, %ObjectToWorld149
  %FMad219 = call float @dx.op.tertiary.f32(i32 46, float %.i1270, float %ObjectToWorld153, float %66)
  %FMad218 = call float @dx.op.tertiary.f32(i32 46, float %.i2271, float %ObjectToWorld157, float %FMad219)
  %67 = fmul fast float %.i0269, %ObjectToWorld150
  %FMad217 = call float @dx.op.tertiary.f32(i32 46, float %.i1270, float %ObjectToWorld154, float %67)
  %FMad216 = call float @dx.op.tertiary.f32(i32 46, float %.i2271, float %ObjectToWorld158, float %FMad217)
  %68 = fmul fast float %.i0269, %ObjectToWorld151
  %FMad215 = call float @dx.op.tertiary.f32(i32 46, float %.i1270, float %ObjectToWorld155, float %68)
  %FMad214 = call float @dx.op.tertiary.f32(i32 46, float %.i2271, float %ObjectToWorld159, float %FMad215)
  br label %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit98.i.i"

"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit98.i.i": ; preds = %if.end.18.i.97.i.i, %if.then.12.i.89.i.i, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.81.i.i"
  %retval.i.38.i.i.0 = phi i1 [ true, %if.end.18.i.97.i.i ], [ false, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.81.i.i" ], [ false, %if.then.12.i.89.i.i ]
  %.064.0.i0 = phi float [ %FMad218, %if.end.18.i.97.i.i ], [ %.065.0.i0, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.81.i.i" ], [ %.065.0.i0, %if.then.12.i.89.i.i ]
  %.064.0.i1 = phi float [ %FMad216, %if.end.18.i.97.i.i ], [ %.065.0.i1, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.81.i.i" ], [ %.065.0.i1, %if.then.12.i.89.i.i ]
  %.064.0.i2 = phi float [ %FMad214, %if.end.18.i.97.i.i ], [ %.065.0.i2, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.81.i.i" ], [ %.065.0.i2, %if.then.12.i.89.i.i ]
  %.087 = phi float [ %t0.i.43.i.i.1, %if.end.18.i.97.i.i ], [ %.086, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.81.i.i" ], [ %.086, %if.then.12.i.89.i.i ]
  %cmp13.i.45.i = fcmp fast olt float %.087, %.073
  %or.cond101 = and i1 %retval.i.38.i.i.0, %cmp13.i.45.i
  %.066.1.i0 = select i1 %or.cond101, float %.064.0.i0, float %.065.0.i0
  %.066.1.i1 = select i1 %or.cond101, float %.064.0.i1, float %.065.0.i1
  %.066.1.i2 = select i1 %or.cond101, float %.064.0.i2, float %.065.0.i2
  %.1 = select i1 %or.cond101, float %.087, float %.073
  %sub.i.121.i.i.i0 = fadd fast float %FMad257, 0xBFD3333340000000
  %sub.i.121.i.i.i1 = fadd fast float %FMad254, 0xBFD3333340000000
  %69 = call float @dx.op.dot3.f32(i32 55, float %FMad245, float %FMad242, float %FMad239, float %sub.i.121.i.i.i0, float %sub.i.121.i.i.i1, float %FMad251)
  %mul.i.122.i.i = fmul fast float %69, 2.000000e+00
  %70 = call float @dx.op.dot3.f32(i32 55, float %sub.i.121.i.i.i0, float %sub.i.121.i.i.i1, float %FMad251, float %sub.i.121.i.i.i0, float %sub.i.121.i.i.i1, float %FMad251)
  %sub4.i.123.i.i = fadd fast float %70, 0xBF970A3DC0000000
  %mul.i.i.124.i.i = fmul fast float %mul.i.122.i.i, %mul.i.122.i.i
  %mul2.i.i.126.i.i = fmul fast float %mul1.i.i.i.i, %sub4.i.123.i.i
  %sub.i.i.127.i.i = fsub fast float %mul.i.i.124.i.i, %mul2.i.i.126.i.i
  %cmp.i.i.128.i.i = fcmp fast olt float %sub.i.i.127.i.i, 0.000000e+00
  br i1 %cmp.i.i.128.i.i, label %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.150.i.i", label %if.else.i.i.131.i.i

if.else.i.i.131.i.i:                              ; preds = %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit98.i.i"
  %cmp4.i.i.130.i.i = fcmp fast oeq float %sub.i.i.127.i.i, 0.000000e+00
  br i1 %cmp4.i.i.130.i.i, label %if.then.7.i.i.134.i.i, label %if.else.9.i.i.136.i.i

if.then.7.i.i.134.i.i:                            ; preds = %if.else.i.i.131.i.i
  %mul8.i.i.132.i.i = fsub fast float -0.000000e+00, %69
  %div.i.i.133.i.i = fdiv fast float %mul8.i.i.132.i.i, %48
  br label %if.end.19.i.i.147.i.i

if.else.9.i.i.136.i.i:                            ; preds = %if.else.i.i.131.i.i
  %cmp10.i.i.135.i.i = fcmp fast ogt float %mul.i.122.i.i, 0.000000e+00
  br i1 %cmp10.i.i.135.i.i, label %cond.true.i.i.138.i.i, label %cond.false.i.i.140.i.i

cond.true.i.i.138.i.i:                            ; preds = %if.else.9.i.i.136.i.i
  %Sqrt111 = call float @dx.op.unary.f32(i32 24, float %sub.i.i.127.i.i)
  %add.i.i.137.i.i = fadd fast float %Sqrt111, %mul.i.122.i.i
  br label %cond.end.i.i.145.i.i

cond.false.i.i.140.i.i:                           ; preds = %if.else.9.i.i.136.i.i
  %Sqrt112 = call float @dx.op.unary.f32(i32 24, float %sub.i.i.127.i.i)
  %sub15.i.i.139.i.i = fsub fast float %mul.i.122.i.i, %Sqrt112
  br label %cond.end.i.i.145.i.i

cond.end.i.i.145.i.i:                             ; preds = %cond.false.i.i.140.i.i, %cond.true.i.i.138.i.i
  %add.sink.i.i.141.i.i = phi float [ %add.i.i.137.i.i, %cond.true.i.i.138.i.i ], [ %sub15.i.i.139.i.i, %cond.false.i.i.140.i.i ]
  %mul13.i.i.142.i.i = fmul fast float %add.sink.i.i.141.i.i, -5.000000e-01
  %div17.i.i.143.i.i = fdiv fast float %mul13.i.i.142.i.i, %48
  %div18.i.i.144.i.i = fdiv fast float %sub4.i.123.i.i, %mul13.i.i.142.i.i
  br label %if.end.19.i.i.147.i.i

if.end.19.i.i.147.i.i:                            ; preds = %cond.end.i.i.145.i.i, %if.then.7.i.i.134.i.i
  %.077 = phi float [ %div.i.i.133.i.i, %if.then.7.i.i.134.i.i ], [ %div17.i.i.143.i.i, %cond.end.i.i.145.i.i ]
  %.074 = phi float [ %div.i.i.133.i.i, %if.then.7.i.i.134.i.i ], [ %div18.i.i.144.i.i, %cond.end.i.i.145.i.i ]
  %cmp20.i.i.146.i.i = fcmp fast ogt float %.077, %.074
  %.178 = select i1 %cmp20.i.i.146.i.i, float %.074, float %.077
  %.175 = select i1 %cmp20.i.i.146.i.i, float %.077, float %.074
  br label %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.150.i.i"

"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.150.i.i": ; preds = %if.end.19.i.i.147.i.i, %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit98.i.i"
  %.279 = phi float [ undef, %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit98.i.i" ], [ %.178, %if.end.19.i.i.147.i.i ]
  %.276 = phi float [ undef, %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit98.i.i" ], [ %.175, %if.end.19.i.i.147.i.i ]
  %retval.i.i.101.i.i.0 = phi i1 [ false, %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit98.i.i" ], [ true, %if.end.19.i.i.147.i.i ]
  br i1 %retval.i.i.101.i.i.0, label %if.end.i.153.i.i, label %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit167.i.i"

if.end.i.153.i.i:                                 ; preds = %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.150.i.i"
  %cmp.i.152.i.i = fcmp fast ogt float %.279, %.276
  %t1.i.113.i.i.0 = select i1 %cmp.i.152.i.i, float %.279, float %.276
  %t0.i.112.i.i.0 = select i1 %cmp.i.152.i.i, float %.276, float %.279
  %cmp9.i.155.i.i = fcmp fast olt float %t0.i.112.i.i.0, 0.000000e+00
  br i1 %cmp9.i.155.i.i, label %if.then.12.i.158.i.i, label %if.end.18.i.166.i.i

if.then.12.i.158.i.i:                             ; preds = %if.end.i.153.i.i
  %cmp13.i.157.i.i = fcmp fast olt float %t1.i.113.i.i.0, 0.000000e+00
  br i1 %cmp13.i.157.i.i, label %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit167.i.i", label %if.end.18.i.166.i.i

if.end.18.i.166.i.i:                              ; preds = %if.then.12.i.158.i.i, %if.end.i.153.i.i
  %t0.i.112.i.i.1 = phi float [ %t1.i.113.i.i.0, %if.then.12.i.158.i.i ], [ %t0.i.112.i.i.0, %if.end.i.153.i.i ]
  %mul19.i.162.i.i.i0 = fmul fast float %t0.i.112.i.i.1, %FMad245
  %mul19.i.162.i.i.i1 = fmul fast float %t0.i.112.i.i.1, %FMad242
  %mul19.i.162.i.i.i2 = fmul fast float %t0.i.112.i.i.1, %FMad239
  %ObjectToWorld = call float @dx.op.objectToWorld.f32(i32 151, i32 0, i8 0)
  %ObjectToWorld138 = call float @dx.op.objectToWorld.f32(i32 151, i32 0, i8 1)
  %ObjectToWorld139 = call float @dx.op.objectToWorld.f32(i32 151, i32 0, i8 2)
  %ObjectToWorld141 = call float @dx.op.objectToWorld.f32(i32 151, i32 1, i8 0)
  %ObjectToWorld142 = call float @dx.op.objectToWorld.f32(i32 151, i32 1, i8 1)
  %ObjectToWorld143 = call float @dx.op.objectToWorld.f32(i32 151, i32 1, i8 2)
  %ObjectToWorld145 = call float @dx.op.objectToWorld.f32(i32 151, i32 2, i8 0)
  %ObjectToWorld146 = call float @dx.op.objectToWorld.f32(i32 151, i32 2, i8 1)
  %ObjectToWorld147 = call float @dx.op.objectToWorld.f32(i32 151, i32 2, i8 2)
  %sub21.i.164.i.i.i0 = fadd fast float %sub.i.121.i.i.i0, %mul19.i.162.i.i.i0
  %sub21.i.164.i.i.i1 = fadd fast float %sub.i.121.i.i.i1, %mul19.i.162.i.i.i1
  %sub21.i.164.i.i.i2 = fadd fast float %FMad251, %mul19.i.162.i.i.i2
  %71 = fmul fast float %sub21.i.164.i.i.i0, %sub21.i.164.i.i.i0
  %72 = fmul fast float %sub21.i.164.i.i.i1, %sub21.i.164.i.i.i1
  %73 = fadd fast float %71, %72
  %74 = fmul fast float %sub21.i.164.i.i.i2, %sub21.i.164.i.i.i2
  %75 = fadd fast float %73, %74
  %Sqrt128 = call float @dx.op.unary.f32(i32 24, float %75)
  %.i0272 = fdiv fast float %sub21.i.164.i.i.i0, %Sqrt128
  %.i1273 = fdiv fast float %sub21.i.164.i.i.i1, %Sqrt128
  %.i2274 = fdiv fast float %sub21.i.164.i.i.i2, %Sqrt128
  %76 = fmul fast float %.i0272, %ObjectToWorld
  %FMad211 = call float @dx.op.tertiary.f32(i32 46, float %.i1273, float %ObjectToWorld141, float %76)
  %FMad210 = call float @dx.op.tertiary.f32(i32 46, float %.i2274, float %ObjectToWorld145, float %FMad211)
  %77 = fmul fast float %.i0272, %ObjectToWorld138
  %FMad209 = call float @dx.op.tertiary.f32(i32 46, float %.i1273, float %ObjectToWorld142, float %77)
  %FMad208 = call float @dx.op.tertiary.f32(i32 46, float %.i2274, float %ObjectToWorld146, float %FMad209)
  %78 = fmul fast float %.i0272, %ObjectToWorld139
  %FMad207 = call float @dx.op.tertiary.f32(i32 46, float %.i1273, float %ObjectToWorld143, float %78)
  %FMad206 = call float @dx.op.tertiary.f32(i32 46, float %.i2274, float %ObjectToWorld147, float %FMad207)
  br label %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit167.i.i"

"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit167.i.i": ; preds = %if.end.18.i.166.i.i, %if.then.12.i.158.i.i, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.150.i.i"
  %.0.0.i0 = phi float [ %FMad210, %if.end.18.i.166.i.i ], [ %.064.0.i0, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.150.i.i" ], [ %.064.0.i0, %if.then.12.i.158.i.i ]
  %.0.0.i1 = phi float [ %FMad208, %if.end.18.i.166.i.i ], [ %.064.0.i1, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.150.i.i" ], [ %.064.0.i1, %if.then.12.i.158.i.i ]
  %.0.0.i2 = phi float [ %FMad206, %if.end.18.i.166.i.i ], [ %.064.0.i2, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.150.i.i" ], [ %.064.0.i2, %if.then.12.i.158.i.i ]
  %.088 = phi float [ %t0.i.112.i.i.1, %if.end.18.i.166.i.i ], [ %.087, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.150.i.i" ], [ %.087, %if.then.12.i.158.i.i ]
  %retval.i.107.i.i.0 = phi i1 [ true, %if.end.18.i.166.i.i ], [ false, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.150.i.i" ], [ false, %if.then.12.i.158.i.i ]
  %cmp23.i.i = fcmp fast olt float %.088, %.1
  %or.cond102 = and i1 %retval.i.107.i.i.0, %cmp23.i.i
  %79 = or i1 %or.cond101, %or.cond102
  %.066.2.i0 = select i1 %or.cond102, float %.0.0.i0, float %.066.1.i0
  %.066.2.i1 = select i1 %or.cond102, float %.0.0.i1, float %.066.1.i1
  %.066.2.i2 = select i1 %or.cond102, float %.0.0.i2, float %.066.1.i2
  %.2 = select i1 %or.cond102, float %.088, float %.1
  %tobool29.i.i = or i1 %or.cond, %79
  br label %"\01?IntersectCustomPrimitiveFrontToBack@@YA_NV?$vector@M$02@@0MMAIAMUProceduralPrimitiveAttributes@@@Z.exit"

"\01?IntersectCustomPrimitiveFrontToBack@@YA_NV?$vector@M$02@@0MMAIAMUProceduralPrimitiveAttributes@@@Z.exit": ; preds = %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit167.i.i", %if.end.18.i.i, %if.then.12.i.i, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.i", %"\01?intersectBox@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@@Z.exit.i", %entry
  %retval.i.0 = phi i1 [ %tobool29.i.i, %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit167.i.i" ], [ %cmp63.i.i, %"\01?intersectBox@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@@Z.exit.i" ], [ true, %if.end.18.i.i ], [ false, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.i" ], [ false, %if.then.12.i.i ], [ false, %entry ]
  %.069.0.i0 = phi float [ %.066.2.i0, %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit167.i.i" ], [ %FMad202, %"\01?intersectBox@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@@Z.exit.i" ], [ %FMad226, %if.end.18.i.i ], [ undef, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.i" ], [ undef, %if.then.12.i.i ], [ undef, %entry ]
  %.069.0.i1 = phi float [ %.066.2.i1, %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit167.i.i" ], [ %FMad200, %"\01?intersectBox@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@@Z.exit.i" ], [ %FMad224, %if.end.18.i.i ], [ undef, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.i" ], [ undef, %if.then.12.i.i ], [ undef, %entry ]
  %.069.0.i2 = phi float [ %.066.2.i2, %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit167.i.i" ], [ %FMad198, %"\01?intersectBox@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@@Z.exit.i" ], [ %FMad222, %if.end.18.i.i ], [ undef, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.i" ], [ undef, %if.then.12.i.i ], [ undef, %entry ]
  %.071 = phi float [ %.2, %"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z.exit167.i.i" ], [ %FMax, %"\01?intersectBox@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@@Z.exit.i" ], [ %t0.i.i.1, %if.end.18.i.i ], [ %RayTCurrent, %"\01?solveQuadratic@@YA_NMMMAIAM0@Z.exit.i.i" ], [ %RayTCurrent, %if.then.12.i.i ], [ %RayTCurrent, %entry ]
  %.069.0.upto0 = insertelement <3 x float> undef, float %.069.0.i0, i32 0
  %.069.0.upto1 = insertelement <3 x float> %.069.0.upto0, float %.069.0.i1, i32 1
  %.069.0 = insertelement <3 x float> %.069.0.upto1, float %.069.0.i2, i32 2
  %80 = getelementptr inbounds %struct.ProceduralPrimitiveAttributes, %struct.ProceduralPrimitiveAttributes* %attr, i32 0, i32 0
  store <3 x float> %.069.0, <3 x float>* %80, align 4
  br i1 %retval.i.0, label %if.then, label %if.end

if.then:                                          ; preds = %"\01?IntersectCustomPrimitiveFrontToBack@@YA_NV?$vector@M$02@@0MMAIAMUProceduralPrimitiveAttributes@@@Z.exit"
  %81 = call i1 @dx.op.reportHit.struct.ProceduralPrimitiveAttributes(i32 158, float %.071, i32 0, %struct.ProceduralPrimitiveAttributes* nonnull %attr)
  br label %if.end

if.end:                                           ; preds = %if.then, %"\01?IntersectCustomPrimitiveFrontToBack@@YA_NV?$vector@M$02@@0MMAIAMUProceduralPrimitiveAttributes@@@Z.exit"
  ret void
}

; Function Attrs: alwaysinline nounwind
define void @"\01?RawDataToTriangle@@YA?AUTriangle@@V?$vector@I$03@@0I@Z"(%struct.Triangle* noalias nocapture sret, <4 x i32>, <4 x i32>, i32) #5 {
entry:
  %4 = extractelement <4 x i32> %1, i32 2
  %5 = extractelement <4 x i32> %1, i32 1
  %6 = extractelement <4 x i32> %1, i32 0
  %.i0 = bitcast i32 %6 to float
  %.i1 = bitcast i32 %5 to float
  %.i2 = bitcast i32 %4 to float
  %.upto0 = insertelement <3 x float> undef, float %.i0, i32 0
  %.upto1 = insertelement <3 x float> %.upto0, float %.i1, i32 1
  %7 = insertelement <3 x float> %.upto1, float %.i2, i32 2
  %v0 = getelementptr inbounds %struct.Triangle, %struct.Triangle* %0, i32 0, i32 0
  store <3 x float> %7, <3 x float>* %v0, align 4, !tbaa !220
  %8 = extractelement <4 x i32> %1, i32 3
  %9 = extractelement <4 x i32> %2, i32 1
  %10 = extractelement <4 x i32> %2, i32 0
  %.i03 = bitcast i32 %8 to float
  %.i14 = bitcast i32 %10 to float
  %.i25 = bitcast i32 %9 to float
  %.upto011 = insertelement <3 x float> undef, float %.i03, i32 0
  %.upto112 = insertelement <3 x float> %.upto011, float %.i14, i32 1
  %11 = insertelement <3 x float> %.upto112, float %.i25, i32 2
  %v1 = getelementptr inbounds %struct.Triangle, %struct.Triangle* %0, i32 0, i32 1
  store <3 x float> %11, <3 x float>* %v1, align 4, !tbaa !220
  %12 = extractelement <4 x i32> %2, i32 3
  %13 = extractelement <4 x i32> %2, i32 2
  %.i06 = bitcast i32 %13 to float
  %.i17 = bitcast i32 %12 to float
  %.i28 = bitcast i32 %3 to float
  %.upto015 = insertelement <3 x float> undef, float %.i06, i32 0
  %.upto116 = insertelement <3 x float> %.upto015, float %.i17, i32 1
  %14 = insertelement <3 x float> %.upto116, float %.i28, i32 2
  %v2 = getelementptr inbounds %struct.Triangle, %struct.Triangle* %0, i32 0, i32 2
  store <3 x float> %14, <3 x float>* %v2, align 4, !tbaa !220
  ret void
}

; Function Attrs: alwaysinline nounwind readnone
define i1 @"\01?IsLeaf@@YA_NV?$vector@I$01@@@Z"(<2 x i32>) #4 {
entry:
  %1 = extractelement <2 x i32> %0, i32 0
  %tobool = icmp slt i32 %1, 0
  ret i1 %tobool
}

; Function Attrs: alwaysinline nounwind
define void @"\01?GetTriangle@@YA?AUTriangle@@UPrimitive@@@Z"(%struct.Triangle* noalias nocapture sret, %struct.Primitive* nocapture readonly) #5 {
entry:
  %data2 = getelementptr inbounds %struct.Primitive, %struct.Primitive* %1, i32 0, i32 3
  %2 = load i32, i32* %data2, align 4, !tbaa !223
  %data1 = getelementptr inbounds %struct.Primitive, %struct.Primitive* %1, i32 0, i32 2
  %3 = load <4 x i32>, <4 x i32>* %data1, align 4, !tbaa !220
  %data0 = getelementptr inbounds %struct.Primitive, %struct.Primitive* %1, i32 0, i32 1
  %4 = load <4 x i32>, <4 x i32>* %data0, align 4, !tbaa !220
  %5 = extractelement <4 x i32> %4, i32 2
  %6 = extractelement <4 x i32> %4, i32 1
  %7 = extractelement <4 x i32> %4, i32 0
  %.i01 = bitcast i32 %7 to float
  %.i12 = bitcast i32 %6 to float
  %.i23 = bitcast i32 %5 to float
  %.upto0 = insertelement <3 x float> undef, float %.i01, i32 0
  %.upto1 = insertelement <3 x float> %.upto0, float %.i12, i32 1
  %8 = insertelement <3 x float> %.upto1, float %.i23, i32 2
  %v0.i = getelementptr inbounds %struct.Triangle, %struct.Triangle* %0, i32 0, i32 0
  store <3 x float> %8, <3 x float>* %v0.i, align 4, !tbaa !220, !alias.scope !282
  %9 = extractelement <4 x i32> %4, i32 3
  %10 = extractelement <4 x i32> %3, i32 1
  %11 = extractelement <4 x i32> %3, i32 0
  %.i06 = bitcast i32 %9 to float
  %.i17 = bitcast i32 %11 to float
  %.i28 = bitcast i32 %10 to float
  %.upto015 = insertelement <3 x float> undef, float %.i06, i32 0
  %.upto116 = insertelement <3 x float> %.upto015, float %.i17, i32 1
  %12 = insertelement <3 x float> %.upto116, float %.i28, i32 2
  %v1.i = getelementptr inbounds %struct.Triangle, %struct.Triangle* %0, i32 0, i32 1
  store <3 x float> %12, <3 x float>* %v1.i, align 4, !tbaa !220, !alias.scope !282
  %13 = extractelement <4 x i32> %3, i32 3
  %14 = extractelement <4 x i32> %3, i32 2
  %.i010 = bitcast i32 %14 to float
  %.i111 = bitcast i32 %13 to float
  %.i212 = bitcast i32 %2 to float
  %.upto019 = insertelement <3 x float> undef, float %.i010, i32 0
  %.upto120 = insertelement <3 x float> %.upto019, float %.i111, i32 1
  %15 = insertelement <3 x float> %.upto120, float %.i212, i32 2
  %v2.i = getelementptr inbounds %struct.Triangle, %struct.Triangle* %0, i32 0, i32 2
  store <3 x float> %15, <3 x float>* %v2.i, align 4, !tbaa !220, !alias.scope !282
  ret void
}

; Function Attrs: alwaysinline nounwind
define void @"\01?GetRayData@@YA?AURayData@@V?$vector@M$02@@0@Z"(%struct.RayData* noalias nocapture sret, <3 x float>, <3 x float>) #5 {
entry:
  %rayOrigin.i0 = extractelement <3 x float> %1, i32 0
  %rayOrigin.i1 = extractelement <3 x float> %1, i32 1
  %rayOrigin.i2 = extractelement <3 x float> %1, i32 2
  %3 = alloca [3 x float], align 4
  %4 = extractelement <3 x float> %2, i64 0
  %5 = getelementptr inbounds [3 x float], [3 x float]* %3, i32 0, i32 0
  store float %4, float* %5, align 4
  %6 = extractelement <3 x float> %2, i64 1
  %7 = getelementptr inbounds [3 x float], [3 x float]* %3, i32 0, i32 1
  store float %6, float* %7, align 4
  %8 = extractelement <3 x float> %2, i64 2
  %9 = getelementptr inbounds [3 x float], [3 x float]* %3, i32 0, i32 2
  store float %8, float* %9, align 4
  %.i0 = fdiv fast float 1.000000e+00, %4
  %.i1 = fdiv fast float 1.000000e+00, %6
  %.i2 = fdiv fast float 1.000000e+00, %8
  %.upto0 = insertelement <3 x float> undef, float %.i0, i32 0
  %.upto1 = insertelement <3 x float> %.upto0, float %.i1, i32 1
  %10 = insertelement <3 x float> %.upto1, float %.i2, i32 2
  %mul.i0 = fmul fast float %.i0, %rayOrigin.i0
  %mul.i1 = fmul fast float %.i1, %rayOrigin.i1
  %mul.i2 = fmul fast float %.i2, %rayOrigin.i2
  %mul.upto0 = insertelement <3 x float> undef, float %mul.i0, i32 0
  %mul.upto1 = insertelement <3 x float> %mul.upto0, float %mul.i1, i32 1
  %mul = insertelement <3 x float> %mul.upto1, float %mul.i2, i32 2
  %FAbs = call float @dx.op.unary.f32(i32 6, float %4)
  %FAbs17 = call float @dx.op.unary.f32(i32 6, float %6)
  %FAbs18 = call float @dx.op.unary.f32(i32 6, float %8)
  %cmp.i = fcmp fast ogt float %FAbs, %FAbs17
  %cmp1.i = fcmp fast ogt float %FAbs, %FAbs18
  %11 = and i1 %cmp.i, %cmp1.i
  br i1 %11, label %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit", label %if.else.i

if.else.i:                                        ; preds = %entry
  %cmp4.i = fcmp fast ogt float %FAbs17, %FAbs18
  br i1 %cmp4.i, label %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit", label %if.else.8.i

if.else.8.i:                                      ; preds = %if.else.i
  br label %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit"

"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit": ; preds = %if.else.8.i, %if.else.i, %entry
  %retval.i.0 = phi i32 [ 2, %if.else.8.i ], [ 0, %entry ], [ 1, %if.else.i ]
  %add = add nuw nsw i32 %retval.i.0, 1
  %rem = urem i32 %add, 3
  %add4 = add nuw nsw i32 %retval.i.0, 2
  %rem5 = urem i32 %add4, 3
  %12 = getelementptr [3 x float], [3 x float]* %3, i32 0, i32 %retval.i.0
  %13 = load float, float* %12, align 4, !tbaa !228
  %cmp = fcmp fast olt float %13, 0.000000e+00
  %.5.0.i0 = select i1 %cmp, i32 %rem5, i32 %rem
  %.5.0.i1 = select i1 %cmp, i32 %rem, i32 %rem5
  %.5.0.upto0 = insertelement <3 x i32> undef, i32 %.5.0.i0, i32 0
  %.5.0.upto1 = insertelement <3 x i32> %.5.0.upto0, i32 %.5.0.i1, i32 1
  %.5.0 = insertelement <3 x i32> %.5.0.upto1, i32 %retval.i.0, i32 2
  %14 = getelementptr [3 x float], [3 x float]* %3, i32 0, i32 %.5.0.i0
  %15 = load float, float* %14, align 4, !tbaa !228
  %div = fdiv fast float %15, %13
  %16 = getelementptr [3 x float], [3 x float]* %3, i32 0, i32 %.5.0.i1
  %17 = load float, float* %16, align 4, !tbaa !228
  %div14 = fdiv fast float %17, %13
  %div16 = fdiv fast float 1.000000e+00, %13
  %18 = insertelement <3 x float> undef, float %div, i64 0
  %19 = insertelement <3 x float> %18, float %div14, i64 1
  %20 = insertelement <3 x float> %19, float %div16, i64 2
  %21 = getelementptr inbounds %struct.RayData, %struct.RayData* %0, i32 0, i32 0
  store <3 x float> %1, <3 x float>* %21, align 4
  %22 = getelementptr inbounds %struct.RayData, %struct.RayData* %0, i32 0, i32 1
  store <3 x float> %2, <3 x float>* %22, align 4
  %23 = getelementptr inbounds %struct.RayData, %struct.RayData* %0, i32 0, i32 2
  store <3 x float> %10, <3 x float>* %23, align 4
  %24 = getelementptr inbounds %struct.RayData, %struct.RayData* %0, i32 0, i32 3
  store <3 x float> %mul, <3 x float>* %24, align 4
  %25 = getelementptr inbounds %struct.RayData, %struct.RayData* %0, i32 0, i32 4
  store <3 x float> %20, <3 x float>* %25, align 4
  %26 = getelementptr inbounds %struct.RayData, %struct.RayData* %0, i32 0, i32 5
  store <3 x i32> %.5.0, <3 x i32>* %26, align 4
  ret void
}

; Function Attrs: alwaysinline nounwind readnone
define void @"\01?LogInt2@@YAXV?$vector@H$01@@@Z"(<2 x i32>) #4 {
entry:
  ret void
}

; Function Attrs: alwaysinline nounwind readnone
define i32 @"\01?GetBoxAddress@@YAIII@Z"(i32, i32) #4 {
entry:
  %mul = shl i32 %1, 5
  %add = add i32 %mul, %0
  ret i32 %add
}

; Function Attrs: alwaysinline nounwind
define i32 @"\01?InvokeAnyHit@@YAHH@Z"(i32) #5 {
entry:
  call void @"\01?Fallback_SetAnyHitResult@@YAXH@Z"(i32 1) #2
  call void @"\01?Fallback_CallIndirect@@YAXH@Z"(i32 %0) #2
  %call = call i32 @"\01?Fallback_AnyHitResult@@YAHXZ"() #2
  ret i32 %call
}

; Function Attrs: alwaysinline nounwind
define void @"\01?CreateProceduralGeometryPrimitive@@YA?AUPrimitive@@UAABB@@@Z"(%struct.Primitive* noalias nocapture sret, %struct.AABB* nocapture readonly) #5 {
entry:
  %min.i = getelementptr inbounds %struct.AABB, %struct.AABB* %1, i32 0, i32 0
  %2 = load <3 x float>, <3 x float>* %min.i, align 4, !noalias !285
  %3 = extractelement <3 x float> %2, i32 2
  %4 = extractelement <3 x float> %2, i32 1
  %5 = extractelement <3 x float> %2, i32 0
  %max.i = getelementptr inbounds %struct.AABB, %struct.AABB* %1, i32 0, i32 1
  %6 = load <3 x float>, <3 x float>* %max.i, align 4, !noalias !285
  %7 = extractelement <3 x float> %6, i32 0
  %.i01 = bitcast float %5 to i32
  %.i12 = bitcast float %4 to i32
  %.i23 = bitcast float %3 to i32
  %.i3 = bitcast float %7 to i32
  %.upto08 = insertelement <4 x i32> undef, i32 %.i01, i32 0
  %.upto19 = insertelement <4 x i32> %.upto08, i32 %.i12, i32 1
  %.upto2 = insertelement <4 x i32> %.upto19, i32 %.i23, i32 2
  %8 = insertelement <4 x i32> %.upto2, i32 %.i3, i32 3
  %9 = extractelement <3 x float> %6, i32 2
  %10 = extractelement <3 x float> %6, i32 1
  %.i06 = bitcast float %10 to i32
  %.i17 = bitcast float %9 to i32
  %11 = insertelement <4 x i32> <i32 undef, i32 undef, i32 0, i32 0>, i32 %.i06, i32 0
  %12 = insertelement <4 x i32> %11, i32 %.i17, i32 1
  %13 = getelementptr inbounds %struct.Primitive, %struct.Primitive* %0, i32 0, i32 0
  store i32 2, i32* %13, align 4
  %14 = getelementptr inbounds %struct.Primitive, %struct.Primitive* %0, i32 0, i32 1
  store <4 x i32> %8, <4 x i32>* %14, align 4
  %15 = getelementptr inbounds %struct.Primitive, %struct.Primitive* %0, i32 0, i32 2
  store <4 x i32> %12, <4 x i32>* %15, align 4
  %16 = getelementptr inbounds %struct.Primitive, %struct.Primitive* %0, i32 0, i32 3
  store i32 0, i32* %16, align 4
  ret void
}

; Function Attrs: alwaysinline nounwind
define i1 @"\01?TestLeafNodeIntersections@@YA_NURWByteAddressBufferPointer@@V?$vector@I$01@@IV?$vector@M$02@@2V?$vector@H$02@@2AIAV?$vector@M$01@@AIAMAIAI@Z"(%struct.RWByteAddressBufferPointer* nocapture readonly, <2 x i32>, i32, <3 x float>, <3 x float>, <3 x i32>, <3 x float>, <2 x float>* noalias nocapture dereferenceable(8), float* noalias nocapture dereferenceable(4), i32* noalias nocapture dereferenceable(4)) #5 {
entry:
  %shear.i0 = extractelement <3 x float> %6, i32 0
  %shear.i1 = extractelement <3 x float> %6, i32 1
  %rayOrigin.i0 = extractelement <3 x float> %3, i32 0
  %rayOrigin.i1 = extractelement <3 x float> %3, i32 1
  %rayOrigin.i2 = extractelement <3 x float> %3, i32 2
  %10 = load i32, i32* %9, align 4
  %11 = load <2 x float>, <2 x float>* %7, align 4
  %.i020 = extractelement <2 x float> %11, i32 0
  %.i121 = extractelement <2 x float> %11, i32 1
  %12 = alloca [3 x float], align 4
  %13 = alloca [3 x float], align 4
  %14 = alloca [3 x float], align 4
  %15 = extractelement <2 x i32> %1, i32 0
  %and = and i32 %15, 16777215
  %offsetInBytes.i.i = getelementptr inbounds %struct.RWByteAddressBufferPointer, %struct.RWByteAddressBufferPointer* %0, i32 0, i32 1
  %16 = load i32, i32* %offsetInBytes.i.i, align 4, !tbaa !223, !noalias !289
  %add.i.i = add i32 %16, 4
  %17 = getelementptr inbounds %struct.RWByteAddressBufferPointer, %struct.RWByteAddressBufferPointer* %0, i32 0, i32 0
  %18 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %17, align 4, !noalias !289
  %19 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %18)
  %20 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %19, i32 %add.i.i, i32 undef)
  %21 = extractvalue %dx.types.ResRet.i32 %20, 0
  %mul.i = mul nuw nsw i32 %and, 40
  %add2.i.i = add i32 %16, %mul.i
  %add.i = add i32 %add2.i.i, %21
  %add1.i = add i32 %add.i, 4
  %22 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %19, i32 %add1.i, i32 undef)
  %23 = extractvalue %dx.types.ResRet.i32 %22, 0
  %24 = extractvalue %dx.types.ResRet.i32 %22, 1
  %25 = extractvalue %dx.types.ResRet.i32 %22, 2
  %26 = extractvalue %dx.types.ResRet.i32 %22, 3
  %.i0 = bitcast i32 %23 to float
  %.i1 = bitcast i32 %24 to float
  %.i2 = bitcast i32 %25 to float
  %.i3 = bitcast i32 %26 to float
  %add5.i = add i32 %add.i, 20
  %27 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %19, i32 %add5.i, i32 undef)
  %28 = extractvalue %dx.types.ResRet.i32 %27, 0
  %29 = extractvalue %dx.types.ResRet.i32 %27, 1
  %30 = extractvalue %dx.types.ResRet.i32 %27, 2
  %31 = extractvalue %dx.types.ResRet.i32 %27, 3
  %.i016 = bitcast i32 %28 to float
  %.i117 = bitcast i32 %29 to float
  %.i218 = bitcast i32 %30 to float
  %.i319 = bitcast i32 %31 to float
  %add9.i = add i32 %add.i, 36
  %32 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %19, i32 %add9.i, i32 undef)
  %33 = extractvalue %dx.types.ResRet.i32 %32, 0
  %34 = bitcast i32 %33 to float
  %35 = load float, float* %8, align 4, !tbaa !228
  %and.i = and i32 %2, 1
  %lnot.i = icmp eq i32 %and.i, 0
  %and1.i = shl i32 %2, 3
  %36 = and i32 %and1.i, 16
  %37 = add nuw nsw i32 %36, 16
  %38 = xor i32 %36, 16
  %39 = add nuw nsw i32 %38, 16
  %RayFlags15 = call i32 @dx.op.rayFlags.i32(i32 144)
  %and8.i = and i32 %RayFlags15, %37
  %tobool9.i = icmp ne i32 %and8.i, 0
  %40 = and i1 %lnot.i, %tobool9.i
  %and13.i = and i32 %RayFlags15, %39
  %tobool14.i = icmp ne i32 %and13.i, 0
  %41 = and i1 %lnot.i, %tobool14.i
  %sub.i.i0 = fsub fast float %.i0, %rayOrigin.i0
  %sub.i.i1 = fsub fast float %.i1, %rayOrigin.i1
  %sub.i.i2 = fsub fast float %.i2, %rayOrigin.i2
  %42 = getelementptr inbounds [3 x float], [3 x float]* %14, i32 0, i32 0
  store float %sub.i.i0, float* %42, align 4
  %43 = getelementptr inbounds [3 x float], [3 x float]* %14, i32 0, i32 1
  store float %sub.i.i1, float* %43, align 4
  %44 = getelementptr inbounds [3 x float], [3 x float]* %14, i32 0, i32 2
  store float %sub.i.i2, float* %44, align 4
  %45 = extractelement <3 x i32> %5, i32 0
  %46 = getelementptr [3 x float], [3 x float]* %14, i32 0, i32 %45
  %47 = load float, float* %46, align 4, !tbaa !228, !noalias !294
  %48 = extractelement <3 x i32> %5, i32 1
  %49 = getelementptr [3 x float], [3 x float]* %14, i32 0, i32 %48
  %50 = load float, float* %49, align 4, !tbaa !228, !noalias !294
  %51 = extractelement <3 x i32> %5, i32 2
  %52 = getelementptr [3 x float], [3 x float]* %14, i32 0, i32 %51
  %53 = load float, float* %52, align 4, !tbaa !228, !noalias !294
  %sub17.i.i0 = fsub fast float %.i3, %rayOrigin.i0
  %sub17.i.i1 = fsub fast float %.i016, %rayOrigin.i1
  %sub17.i.i2 = fsub fast float %.i117, %rayOrigin.i2
  %54 = getelementptr inbounds [3 x float], [3 x float]* %12, i32 0, i32 0
  store float %sub17.i.i0, float* %54, align 4
  %55 = getelementptr inbounds [3 x float], [3 x float]* %12, i32 0, i32 1
  store float %sub17.i.i1, float* %55, align 4
  %56 = getelementptr inbounds [3 x float], [3 x float]* %12, i32 0, i32 2
  store float %sub17.i.i2, float* %56, align 4
  %57 = getelementptr [3 x float], [3 x float]* %12, i32 0, i32 %45
  %58 = load float, float* %57, align 4, !tbaa !228, !noalias !294
  %59 = getelementptr [3 x float], [3 x float]* %12, i32 0, i32 %48
  %60 = load float, float* %59, align 4, !tbaa !228, !noalias !294
  %61 = getelementptr [3 x float], [3 x float]* %12, i32 0, i32 %51
  %62 = load float, float* %61, align 4, !tbaa !228, !noalias !294
  %sub19.i.i0 = fsub fast float %.i218, %rayOrigin.i0
  %sub19.i.i1 = fsub fast float %.i319, %rayOrigin.i1
  %sub19.i.i2 = fsub fast float %34, %rayOrigin.i2
  %63 = getelementptr inbounds [3 x float], [3 x float]* %13, i32 0, i32 0
  store float %sub19.i.i0, float* %63, align 4
  %64 = getelementptr inbounds [3 x float], [3 x float]* %13, i32 0, i32 1
  store float %sub19.i.i1, float* %64, align 4
  %65 = getelementptr inbounds [3 x float], [3 x float]* %13, i32 0, i32 2
  store float %sub19.i.i2, float* %65, align 4
  %66 = getelementptr [3 x float], [3 x float]* %13, i32 0, i32 %45
  %67 = load float, float* %66, align 4, !tbaa !228, !noalias !294
  %68 = getelementptr [3 x float], [3 x float]* %13, i32 0, i32 %48
  %69 = load float, float* %68, align 4, !tbaa !228, !noalias !294
  %70 = getelementptr [3 x float], [3 x float]* %13, i32 0, i32 %51
  %71 = load float, float* %70, align 4, !tbaa !228, !noalias !294
  %mul.i.5.i0 = fmul float %shear.i0, %53
  %mul.i.5.i1 = fmul float %shear.i1, %53
  %sub21.i.i0 = fsub float %47, %mul.i.5.i0
  %sub21.i.i1 = fsub float %50, %mul.i.5.i1
  %mul24.i.i0 = fmul float %shear.i0, %62
  %mul24.i.i1 = fmul float %shear.i1, %62
  %sub25.i.i0 = fsub float %58, %mul24.i.i0
  %sub25.i.i1 = fsub float %60, %mul24.i.i1
  %mul28.i.i0 = fmul float %shear.i0, %71
  %mul28.i.i1 = fmul float %shear.i1, %71
  %sub29.i.i0 = fsub float %67, %mul28.i.i0
  %sub29.i.i1 = fsub float %69, %mul28.i.i1
  %mul30.i = fmul float %sub25.i.i1, %sub29.i.i0
  %mul31.i = fmul float %sub25.i.i0, %sub29.i.i1
  %sub32.i = fsub float %mul30.i, %mul31.i
  %mul33.i = fmul float %sub21.i.i0, %sub29.i.i1
  %mul34.i = fmul float %sub21.i.i1, %sub29.i.i0
  %sub35.i = fsub float %mul33.i, %mul34.i
  %mul36.i = fmul float %sub21.i.i1, %sub25.i.i0
  %mul37.i = fmul float %sub21.i.i0, %sub25.i.i1
  %sub38.i = fsub float %mul36.i, %mul37.i
  %add.i.6 = fadd fast float %sub35.i, %sub38.i
  %add39.i = fadd fast float %add.i.6, %sub32.i
  br i1 %41, label %if.then.i, label %if.else.i

if.then.i:                                        ; preds = %entry
  %cmp.i = fcmp fast ogt float %sub32.i, 0.000000e+00
  %cmp42.i = fcmp fast ogt float %sub35.i, 0.000000e+00
  %72 = or i1 %cmp.i, %cmp42.i
  %cmp44.i = fcmp fast ogt float %sub38.i, 0.000000e+00
  %73 = or i1 %cmp44.i, %72
  %cmp77.i.old.old = fcmp fast oeq float %add39.i, 0.000000e+00
  %or.cond11 = or i1 %73, %cmp77.i.old.old
  br i1 %or.cond11, label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit", label %if.end.81.i

if.else.i:                                        ; preds = %entry
  br i1 %40, label %if.then.49.i, label %if.else.59.i

if.then.49.i:                                     ; preds = %if.else.i
  %cmp50.i = fcmp fast olt float %sub32.i, 0.000000e+00
  %cmp52.i = fcmp fast olt float %sub35.i, 0.000000e+00
  %74 = or i1 %cmp50.i, %cmp52.i
  %cmp54.i = fcmp fast olt float %sub38.i, 0.000000e+00
  %75 = or i1 %cmp54.i, %74
  %cmp77.i.old = fcmp fast oeq float %add39.i, 0.000000e+00
  %or.cond10 = or i1 %75, %cmp77.i.old
  br i1 %or.cond10, label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit", label %if.end.81.i

if.else.59.i:                                     ; preds = %if.else.i
  %cmp60.i = fcmp fast olt float %sub32.i, 0.000000e+00
  %cmp62.i = fcmp fast olt float %sub35.i, 0.000000e+00
  %76 = or i1 %cmp60.i, %cmp62.i
  %cmp64.i = fcmp fast olt float %sub38.i, 0.000000e+00
  %77 = or i1 %cmp64.i, %76
  %cmp66.i = fcmp fast ogt float %sub32.i, 0.000000e+00
  %cmp68.i = fcmp fast ogt float %sub35.i, 0.000000e+00
  %78 = or i1 %cmp66.i, %cmp68.i
  %cmp70.i = fcmp fast ogt float %sub38.i, 0.000000e+00
  %79 = or i1 %cmp70.i, %78
  %80 = and i1 %77, %79
  %cmp77.i = fcmp fast oeq float %add39.i, 0.000000e+00
  %or.cond = or i1 %cmp77.i, %80
  br i1 %or.cond, label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit", label %if.end.81.i

if.end.81.i:                                      ; preds = %if.else.59.i, %if.then.49.i, %if.then.i
  %81 = extractelement <3 x float> %6, i32 2
  %mul82.i = fmul fast float %53, %81
  %mul83.i = fmul fast float %62, %81
  %mul84.i = fmul fast float %71, %81
  %mul85.i = fmul fast float %mul82.i, %sub32.i
  %mul86.i = fmul fast float %mul83.i, %sub35.i
  %mul88.i = fmul fast float %mul84.i, %sub38.i
  %add87.i = fadd fast float %mul86.i, %mul88.i
  %add89.i = fadd fast float %add87.i, %mul85.i
  br i1 %41, label %if.then.91.i, label %if.else.100.i

if.then.91.i:                                     ; preds = %if.end.81.i
  %cmp92.i = fcmp fast ogt float %add89.i, 0.000000e+00
  %mul94.i = fmul fast float %add39.i, %35
  %cmp95.i = fcmp fast olt float %add89.i, %mul94.i
  %82 = or i1 %cmp92.i, %cmp95.i
  br i1 %82, label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit", label %if.end.133.i

if.else.100.i:                                    ; preds = %if.end.81.i
  br i1 %40, label %if.then.102.i, label %if.else.111.i

if.then.102.i:                                    ; preds = %if.else.100.i
  %cmp103.i = fcmp fast olt float %add89.i, 0.000000e+00
  %mul105.i = fmul fast float %add39.i, %35
  %cmp106.i = fcmp fast ogt float %add89.i, %mul105.i
  %83 = or i1 %cmp103.i, %cmp106.i
  br i1 %83, label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit", label %if.end.133.i

if.else.111.i:                                    ; preds = %if.else.100.i
  %cmp112.i = fcmp fast ogt float %add39.i, 0.000000e+00
  %cond116.i = select i1 %cmp112.i, i32 1, i32 -1
  %84 = bitcast float %add89.i to i32
  %xor121.i = xor i32 %84, %cond116.i
  %conv122.i = uitofp i32 %xor121.i to float
  %85 = bitcast float %add39.i to i32
  %xor124.i = xor i32 %cond116.i, %85
  %conv125.i = uitofp i32 %xor124.i to float
  %mul126.i = fmul fast float %conv125.i, %35
  %cmp127.i = fcmp fast ogt float %conv122.i, %mul126.i
  br i1 %cmp127.i, label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit", label %if.end.133.i

if.end.133.i:                                     ; preds = %if.else.111.i, %if.then.102.i, %if.then.91.i
  %86 = fdiv fast float 1.000000e+00, %add39.i
  %mul135.i = fmul fast float %86, %sub35.i
  %mul136.i = fmul fast float %86, %sub38.i
  %mul137.i = fmul fast float %86, %add89.i
  br label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit"

"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit": ; preds = %if.end.133.i, %if.else.111.i, %if.then.102.i, %if.then.91.i, %if.else.59.i, %if.then.49.i, %if.then.i
  %.09.i0 = phi float [ undef, %if.then.i ], [ undef, %if.then.91.i ], [ %mul135.i, %if.end.133.i ], [ undef, %if.then.102.i ], [ undef, %if.else.111.i ], [ undef, %if.then.49.i ], [ undef, %if.else.59.i ]
  %.09.i1 = phi float [ undef, %if.then.i ], [ undef, %if.then.91.i ], [ %mul136.i, %if.end.133.i ], [ undef, %if.then.102.i ], [ undef, %if.else.111.i ], [ undef, %if.then.49.i ], [ undef, %if.else.59.i ]
  %.08 = phi float [ %35, %if.then.i ], [ %35, %if.then.91.i ], [ %mul137.i, %if.end.133.i ], [ %35, %if.then.102.i ], [ %35, %if.else.111.i ], [ %35, %if.then.49.i ], [ %35, %if.else.59.i ]
  %cmp = fcmp fast olt float %.08, %35
  %RayTMin = call float @dx.op.rayTMin.f32(i32 153)
  %cmp1 = fcmp fast ogt float %.08, %RayTMin
  %87 = and i1 %cmp, %cmp1
  br i1 %87, label %if.then, label %if.end

if.then:                                          ; preds = %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit"
  store float %.08, float* %8, align 4, !tbaa !228
  br label %if.end

if.end:                                           ; preds = %if.then, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit"
  %bIsIntersect.0 = phi i1 [ true, %if.then ], [ false, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit" ]
  %.07.i0 = phi float [ %.09.i0, %if.then ], [ %.i020, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit" ]
  %.07.i1 = phi float [ %.09.i1, %if.then ], [ %.i121, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit" ]
  %.0 = phi i32 [ %and, %if.then ], [ %10, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit" ]
  %.07.upto0 = insertelement <2 x float> undef, float %.07.i0, i32 0
  %.07 = insertelement <2 x float> %.07.upto0, float %.07.i1, i32 1
  store <2 x float> %.07, <2 x float>* %7, align 4
  store i32 %.0, i32* %9, align 4
  ret i1 %bIsIntersect.0
}

; Function Attrs: alwaysinline nounwind
define i1 @"\01?intersectBox@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@@Z"(<3 x float>, <3 x float>, float* noalias nocapture dereferenceable(4), %struct.ProceduralPrimitiveAttributes* noalias nocapture) #5 {
entry:
  %origin.i0 = extractelement <3 x float> %0, i32 0
  %origin.i1 = extractelement <3 x float> %0, i32 1
  %origin.i2 = extractelement <3 x float> %0, i32 2
  %direction.i0 = extractelement <3 x float> %1, i32 0
  %direction.i1 = extractelement <3 x float> %1, i32 1
  %direction.i2 = extractelement <3 x float> %1, i32 2
  %sub = fsub fast float -1.000000e+00, %origin.i0
  %div = fdiv fast float %sub, %direction.i0
  %sub2 = fsub fast float 1.000000e+00, %origin.i0
  %div3 = fdiv fast float %sub2, %direction.i0
  %cmp = fcmp fast olt float %direction.i0, 0.000000e+00
  %tmin.0 = select i1 %cmp, float %div3, float %div
  %tmax.0 = select i1 %cmp, float %div, float %div3
  %sub6 = fsub fast float -1.000000e+00, %origin.i1
  %div7 = fdiv fast float %sub6, %direction.i1
  %sub9 = fsub fast float 1.000000e+00, %origin.i1
  %div10 = fdiv fast float %sub9, %direction.i1
  %cmp11 = fcmp fast olt float %direction.i1, 0.000000e+00
  %tymin.0 = select i1 %cmp11, float %div10, float %div7
  %tymax.0 = select i1 %cmp11, float %div7, float %div10
  %sub17 = fsub fast float -1.000000e+00, %origin.i2
  %div18 = fdiv fast float %sub17, %direction.i2
  %sub20 = fsub fast float 1.000000e+00, %origin.i2
  %div21 = fdiv fast float %sub20, %direction.i2
  %cmp22 = fcmp fast olt float %direction.i2, 0.000000e+00
  %tzmin.0 = select i1 %cmp22, float %div21, float %div18
  %tzmax.0 = select i1 %cmp22, float %div18, float %div21
  %FMax = call float @dx.op.binary.f32(i32 35, float %tmin.0, float %tymin.0)
  %FMax108 = call float @dx.op.binary.f32(i32 35, float %FMax, float %tzmin.0)
  %FMin = call float @dx.op.binary.f32(i32 36, float %tmax.0, float %tymax.0)
  %FMin107 = call float @dx.op.binary.f32(i32 36, float %FMin, float %tzmax.0)
  %mul.i0 = fmul fast float %FMax108, %direction.i0
  %mul.i1 = fmul fast float %FMax108, %direction.i1
  %mul.i2 = fmul fast float %FMax108, %direction.i2
  %add.i0 = fadd fast float %mul.i0, %origin.i0
  %add.i1 = fadd fast float %mul.i1, %origin.i1
  %add.i2 = fadd fast float %mul.i2, %origin.i2
  %4 = fmul fast float %add.i0, %add.i0
  %5 = fmul fast float %add.i1, %add.i1
  %6 = fadd fast float %4, %5
  %7 = fmul fast float %add.i2, %add.i2
  %8 = fadd fast float %6, %7
  %Sqrt = call float @dx.op.unary.f32(i32 24, float %8)
  %.i0 = fdiv fast float %add.i0, %Sqrt
  %.i1 = fdiv fast float %add.i1, %Sqrt
  %.i2 = fdiv fast float %add.i2, %Sqrt
  %FAbs = call float @dx.op.unary.f32(i32 6, float %.i0)
  %FAbs102 = call float @dx.op.unary.f32(i32 6, float %.i1)
  %cmp34 = fcmp fast ogt float %FAbs, %FAbs102
  br i1 %cmp34, label %if.then.37, label %if.else.46

if.then.37:                                       ; preds = %entry
  %FAbs104 = call float @dx.op.unary.f32(i32 6, float %.i2)
  %cmp40 = fcmp fast ogt float %FAbs, %FAbs104
  br i1 %cmp40, label %if.then.43, label %if.else

if.then.43:                                       ; preds = %if.then.37
  %9 = insertelement <3 x float> undef, float %.i0, i64 0
  %10 = insertelement <3 x float> %9, float 0.000000e+00, i64 1
  %11 = insertelement <3 x float> %10, float 0.000000e+00, i64 2
  %normal = getelementptr inbounds %struct.ProceduralPrimitiveAttributes, %struct.ProceduralPrimitiveAttributes* %3, i32 0, i32 0
  store <3 x float> %11, <3 x float>* %normal, align 4, !tbaa !220
  br label %if.end.57

if.else:                                          ; preds = %if.then.37
  %12 = insertelement <3 x float> <float 0.000000e+00, float 0.000000e+00, float undef>, float %.i2, i64 2
  %normal44 = getelementptr inbounds %struct.ProceduralPrimitiveAttributes, %struct.ProceduralPrimitiveAttributes* %3, i32 0, i32 0
  store <3 x float> %12, <3 x float>* %normal44, align 4, !tbaa !220
  br label %if.end.57

if.else.46:                                       ; preds = %entry
  %FAbs106 = call float @dx.op.unary.f32(i32 6, float %.i2)
  %cmp49 = fcmp fast ogt float %FAbs102, %FAbs106
  br i1 %cmp49, label %if.then.52, label %if.else.54

if.then.52:                                       ; preds = %if.else.46
  %13 = insertelement <3 x float> <float 0.000000e+00, float undef, float undef>, float %.i1, i64 1
  %14 = insertelement <3 x float> %13, float 0.000000e+00, i64 2
  %normal53 = getelementptr inbounds %struct.ProceduralPrimitiveAttributes, %struct.ProceduralPrimitiveAttributes* %3, i32 0, i32 0
  store <3 x float> %14, <3 x float>* %normal53, align 4, !tbaa !220
  br label %if.end.57

if.else.54:                                       ; preds = %if.else.46
  %15 = insertelement <3 x float> <float 0.000000e+00, float 0.000000e+00, float undef>, float %.i2, i64 2
  %normal55 = getelementptr inbounds %struct.ProceduralPrimitiveAttributes, %struct.ProceduralPrimitiveAttributes* %3, i32 0, i32 0
  store <3 x float> %15, <3 x float>* %normal55, align 4, !tbaa !220
  br label %if.end.57

if.end.57:                                        ; preds = %if.else.54, %if.then.52, %if.else, %if.then.43
  %.i01 = phi float [ 0.000000e+00, %if.then.52 ], [ 0.000000e+00, %if.else.54 ], [ %.i0, %if.then.43 ], [ 0.000000e+00, %if.else ]
  %.i12 = phi float [ %.i1, %if.then.52 ], [ 0.000000e+00, %if.else.54 ], [ 0.000000e+00, %if.then.43 ], [ 0.000000e+00, %if.else ]
  %.i23 = phi float [ 0.000000e+00, %if.then.52 ], [ %.i2, %if.else.54 ], [ 0.000000e+00, %if.then.43 ], [ %.i2, %if.else ]
  %ObjectToWorld = call float @dx.op.objectToWorld.f32(i32 151, i32 0, i8 0)
  %ObjectToWorld110 = call float @dx.op.objectToWorld.f32(i32 151, i32 0, i8 1)
  %ObjectToWorld111 = call float @dx.op.objectToWorld.f32(i32 151, i32 0, i8 2)
  %ObjectToWorld113 = call float @dx.op.objectToWorld.f32(i32 151, i32 1, i8 0)
  %ObjectToWorld114 = call float @dx.op.objectToWorld.f32(i32 151, i32 1, i8 1)
  %ObjectToWorld115 = call float @dx.op.objectToWorld.f32(i32 151, i32 1, i8 2)
  %ObjectToWorld117 = call float @dx.op.objectToWorld.f32(i32 151, i32 2, i8 0)
  %ObjectToWorld118 = call float @dx.op.objectToWorld.f32(i32 151, i32 2, i8 1)
  %ObjectToWorld119 = call float @dx.op.objectToWorld.f32(i32 151, i32 2, i8 2)
  %normal59 = getelementptr inbounds %struct.ProceduralPrimitiveAttributes, %struct.ProceduralPrimitiveAttributes* %3, i32 0, i32 0
  %16 = fmul fast float %.i01, %.i01
  %17 = fmul fast float %.i12, %.i12
  %18 = fadd fast float %16, %17
  %19 = fmul fast float %.i23, %.i23
  %20 = fadd fast float %18, %19
  %Sqrt109 = call float @dx.op.unary.f32(i32 24, float %20)
  %.i0129 = fdiv fast float %.i01, %Sqrt109
  %.i1131 = fdiv fast float %.i12, %Sqrt109
  %.i2133 = fdiv fast float %.i23, %Sqrt109
  %21 = fmul fast float %.i0129, %ObjectToWorld
  %FMad127 = call float @dx.op.tertiary.f32(i32 46, float %.i1131, float %ObjectToWorld113, float %21)
  %FMad126 = call float @dx.op.tertiary.f32(i32 46, float %.i2133, float %ObjectToWorld117, float %FMad127)
  %22 = fmul fast float %.i0129, %ObjectToWorld110
  %FMad125 = call float @dx.op.tertiary.f32(i32 46, float %.i1131, float %ObjectToWorld114, float %22)
  %FMad124 = call float @dx.op.tertiary.f32(i32 46, float %.i2133, float %ObjectToWorld118, float %FMad125)
  %23 = fmul fast float %.i0129, %ObjectToWorld111
  %FMad123 = call float @dx.op.tertiary.f32(i32 46, float %.i1131, float %ObjectToWorld115, float %23)
  %FMad122 = call float @dx.op.tertiary.f32(i32 46, float %.i2133, float %ObjectToWorld119, float %FMad123)
  %.upto0136 = insertelement <3 x float> undef, float %FMad126, i32 0
  %.upto1137 = insertelement <3 x float> %.upto0136, float %FMad124, i32 1
  %24 = insertelement <3 x float> %.upto1137, float %FMad122, i32 2
  store <3 x float> %24, <3 x float>* %normal59, align 4, !tbaa !220
  %cmp63 = fcmp fast ogt float %FMin107, %FMax108
  store float %FMax108, float* %2, align 4
  ret i1 %cmp63
}

; Function Attrs: alwaysinline nounwind
define void @"\01?GetAnyHitAndIntersectionStateId@@YAXUByteAddressBuffer@@IAIAI1@Z"(%struct.ByteAddressBuffer* nocapture readonly, i32, i32* noalias nocapture dereferenceable(4), i32* noalias nocapture dereferenceable(4)) #5 {
entry:
  %add = add i32 %1, 4
  %4 = load %struct.ByteAddressBuffer, %struct.ByteAddressBuffer* %0, align 4
  %5 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.ByteAddressBuffer(i32 160, %struct.ByteAddressBuffer %4)
  %6 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %5, i32 %add, i32 undef)
  %7 = extractvalue %dx.types.ResRet.i32 %6, 0
  %8 = extractvalue %dx.types.ResRet.i32 %6, 1
  store i32 %7, i32* %2, align 4, !tbaa !223
  store i32 %8, i32* %3, align 4, !tbaa !223
  ret void
}

; Function Attrs: alwaysinline nounwind
define void @"\01?WriteOnlyFlagToBuffer@@YAXURWByteAddressBuffer@@IIV?$vector@I$01@@@Z"(%struct.RWByteAddressBuffer* nocapture readonly, i32, i32, <2 x i32>) #5 {
entry:
  %mul.i = shl i32 %2, 5
  %add.i = add i32 %mul.i, %1
  %4 = extractelement <2 x i32> %3, i32 0
  %add = add i32 %add.i, 12
  %5 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %0, align 4
  %6 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %5)
  call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %6, i32 %add, i32 undef, i32 %4, i32 undef, i32 undef, i32 undef, i8 1)
  %7 = extractelement <2 x i32> %3, i32 1
  %add1 = add i32 %add.i, 28
  %8 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %0, align 4
  %9 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %8)
  call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %9, i32 %add1, i32 undef, i32 %7, i32 undef, i32 undef, i32 undef, i8 1)
  ret void
}

; Function Attrs: alwaysinline nounwind
define void @"\01?CreateTrianglePrimitive@@YA?AUPrimitive@@UTriangle@@@Z"(%struct.Primitive* noalias nocapture sret, %struct.Triangle* nocapture readonly) #5 {
entry:
  %PrimitiveType.i = getelementptr inbounds %struct.Primitive, %struct.Primitive* %0, i32 0, i32 0
  %data0.i = getelementptr inbounds %struct.Primitive, %struct.Primitive* %0, i32 0, i32 1
  %data1.i = getelementptr inbounds %struct.Primitive, %struct.Primitive* %0, i32 0, i32 2
  %data2.i = getelementptr inbounds %struct.Primitive, %struct.Primitive* %0, i32 0, i32 3
  store i32 1, i32* %PrimitiveType.i, align 4, !tbaa !223
  %v0.i = getelementptr inbounds %struct.Triangle, %struct.Triangle* %1, i32 0, i32 0
  %2 = load <3 x float>, <3 x float>* %v0.i, align 4, !noalias !298
  %3 = extractelement <3 x float> %2, i32 2
  %4 = extractelement <3 x float> %2, i32 1
  %5 = extractelement <3 x float> %2, i32 0
  %v1.i = getelementptr inbounds %struct.Triangle, %struct.Triangle* %1, i32 0, i32 1
  %6 = load <3 x float>, <3 x float>* %v1.i, align 4, !noalias !298
  %7 = extractelement <3 x float> %6, i32 0
  %.i01 = bitcast float %5 to i32
  %.i12 = bitcast float %4 to i32
  %.i23 = bitcast float %3 to i32
  %.i3 = bitcast float %7 to i32
  %.upto012 = insertelement <4 x i32> undef, i32 %.i01, i32 0
  %.upto113 = insertelement <4 x i32> %.upto012, i32 %.i12, i32 1
  %.upto2 = insertelement <4 x i32> %.upto113, i32 %.i23, i32 2
  %8 = insertelement <4 x i32> %.upto2, i32 %.i3, i32 3
  %9 = extractelement <3 x float> %6, i32 2
  %10 = extractelement <3 x float> %6, i32 1
  %v2.i = getelementptr inbounds %struct.Triangle, %struct.Triangle* %1, i32 0, i32 2
  %11 = load <3 x float>, <3 x float>* %v2.i, align 4, !noalias !298
  %12 = extractelement <3 x float> %11, i32 1
  %13 = extractelement <3 x float> %11, i32 0
  %.i08 = bitcast float %10 to i32
  %.i19 = bitcast float %9 to i32
  %.i210 = bitcast float %13 to i32
  %.i311 = bitcast float %12 to i32
  %.upto018 = insertelement <4 x i32> undef, i32 %.i08, i32 0
  %.upto119 = insertelement <4 x i32> %.upto018, i32 %.i19, i32 1
  %.upto220 = insertelement <4 x i32> %.upto119, i32 %.i210, i32 2
  %14 = insertelement <4 x i32> %.upto220, i32 %.i311, i32 3
  %15 = extractelement <3 x float> %11, i32 2
  store <4 x i32> %8, <4 x i32>* %data0.i, align 4, !tbaa !220
  store <4 x i32> %14, <4 x i32>* %data1.i, align 4, !tbaa !220
  %16 = bitcast i32* %data2.i to float*
  store float %15, float* %16, align 4, !tbaa !223
  ret void
}

; Function Attrs: alwaysinline nounwind readonly
define %class.matrix.float.3.4 @"\01?CreateMatrix@@YA?AV?$matrix@M$02$03@@Y02V?$vector@M$03@@@Z"([3 x <4 x float>]* nocapture readonly) #6 {
entry:
  %arrayidx = getelementptr inbounds [3 x <4 x float>], [3 x <4 x float>]* %0, i32 0, i32 0
  %1 = load <4 x float>, <4 x float>* %arrayidx, align 4, !tbaa !220
  %arrayidx1 = getelementptr inbounds [3 x <4 x float>], [3 x <4 x float>]* %0, i32 0, i32 1
  %2 = load <4 x float>, <4 x float>* %arrayidx1, align 4, !tbaa !220
  %arrayidx2 = getelementptr inbounds [3 x <4 x float>], [3 x <4 x float>]* %0, i32 0, i32 2
  %3 = load <4 x float>, <4 x float>* %arrayidx2, align 4, !tbaa !220
  %.0.vec.extract.upto0 = insertelement <4 x float> undef, float %7, i32 0
  %.0.vec.extract.upto1 = insertelement <4 x float> %.0.vec.extract.upto0, float %6, i32 1
  %.0.vec.extract.upto2 = insertelement <4 x float> %.0.vec.extract.upto1, float %5, i32 2
  %.0.vec.extract = insertelement <4 x float> %.0.vec.extract.upto2, float %4, i32 3
  %4 = extractelement <4 x float> %1, i64 3
  %5 = extractelement <4 x float> %1, i64 2
  %6 = extractelement <4 x float> %1, i64 1
  %7 = extractelement <4 x float> %1, i64 0
  %.fca.0.0.insert = insertvalue %class.matrix.float.3.4 undef, <4 x float> %.0.vec.extract, 0, 0
  %.16.vec.extract.upto0 = insertelement <4 x float> undef, float %11, i32 0
  %.16.vec.extract.upto1 = insertelement <4 x float> %.16.vec.extract.upto0, float %10, i32 1
  %.16.vec.extract.upto2 = insertelement <4 x float> %.16.vec.extract.upto1, float %9, i32 2
  %.16.vec.extract = insertelement <4 x float> %.16.vec.extract.upto2, float %8, i32 3
  %8 = extractelement <4 x float> %2, i64 3
  %9 = extractelement <4 x float> %2, i64 2
  %10 = extractelement <4 x float> %2, i64 1
  %11 = extractelement <4 x float> %2, i64 0
  %.fca.0.1.insert = insertvalue %class.matrix.float.3.4 %.fca.0.0.insert, <4 x float> %.16.vec.extract, 0, 1
  %.32.vec.extract.upto0 = insertelement <4 x float> undef, float %15, i32 0
  %.32.vec.extract.upto1 = insertelement <4 x float> %.32.vec.extract.upto0, float %14, i32 1
  %.32.vec.extract.upto2 = insertelement <4 x float> %.32.vec.extract.upto1, float %13, i32 2
  %.32.vec.extract = insertelement <4 x float> %.32.vec.extract.upto2, float %12, i32 3
  %12 = extractelement <4 x float> %3, i64 3
  %13 = extractelement <4 x float> %3, i64 2
  %14 = extractelement <4 x float> %3, i64 1
  %15 = extractelement <4 x float> %3, i64 0
  %.fca.0.2.insert = insertvalue %class.matrix.float.3.4 %.fca.0.1.insert, <4 x float> %.32.vec.extract, 0, 2
  ret %class.matrix.float.3.4 %.fca.0.2.insert
}

; Function Attrs: alwaysinline nounwind readnone
define i32 @"\01?GetLeftNodeIndex@@YAIV?$vector@I$01@@@Z"(<2 x i32>) #4 {
entry:
  %1 = extractelement <2 x i32> %0, i32 0
  %and = and i32 %1, 16777215
  ret i32 %and
}

; Function Attrs: alwaysinline nounwind readonly
define i32 @"\01?GetAnyHitStateId@@YAIUByteAddressBuffer@@I@Z"(%struct.ByteAddressBuffer* nocapture readonly, i32) #6 {
entry:
  %add = add i32 %1, 4
  %2 = load %struct.ByteAddressBuffer, %struct.ByteAddressBuffer* %0, align 4
  %3 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.ByteAddressBuffer(i32 160, %struct.ByteAddressBuffer %2)
  %4 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %3, i32 %add, i32 undef)
  %5 = extractvalue %dx.types.ResRet.i32 %4, 0
  ret i32 %5
}

; Function Attrs: alwaysinline nounwind readnone
define i32 @"\01?GetRightNodeIndex@@YAIV?$vector@I$01@@@Z"(<2 x i32>) #4 {
entry:
  %1 = extractelement <2 x i32> %0, i32 1
  ret i32 %1
}

; Function Attrs: alwaysinline nounwind readonly
define i32 @"\01?GetInstanceID@@YAIURaytracingInstanceDesc@@@Z"(%struct.RaytracingInstanceDesc* nocapture readonly) #6 {
entry:
  %InstanceIDAndMask = getelementptr inbounds %struct.RaytracingInstanceDesc, %struct.RaytracingInstanceDesc* %0, i32 0, i32 1
  %1 = load i32, i32* %InstanceIDAndMask, align 4, !tbaa !223
  %and = and i32 %1, 16777215
  ret i32 %and
}

; Function Attrs: alwaysinline nounwind
define void @"\01?StackPush2@@YAXAIAH_NIIII@Z"(i32* noalias nocapture dereferenceable(4), i1 zeroext, i32, i32, i32, i32) #5 {
entry:
  %. = select i1 %1, i32 %2, i32 %3
  %cond5 = select i1 %1, i32 %3, i32 %2
  %6 = load i32, i32* %0, align 4, !tbaa !223
  %mul = shl i32 %6, 6
  %add6 = add i32 %mul, %5
  %mul8 = add i32 %5, 64
  %add9 = add i32 %mul8, %mul
  %arrayidx = getelementptr [2048 x i32], [2048 x i32] addrspace(3)* @"\01?stack@@3PAIA", i32 0, i32 %add6
  store i32 %., i32 addrspace(3)* %arrayidx, align 4, !tbaa !223
  %arrayidx10 = getelementptr [2048 x i32], [2048 x i32] addrspace(3)* @"\01?stack@@3PAIA", i32 0, i32 %add9
  store i32 %cond5, i32 addrspace(3)* %arrayidx10, align 4, !tbaa !223
  %add11 = add nsw i32 %6, 2
  store i32 %add11, i32* %0, align 4, !tbaa !223
  ret void
}

; Function Attrs: alwaysinline nounwind readonly
define i32 @"\01?GetInstanceMask@@YAIURaytracingInstanceDesc@@@Z"(%struct.RaytracingInstanceDesc* nocapture readonly) #6 {
entry:
  %InstanceIDAndMask = getelementptr inbounds %struct.RaytracingInstanceDesc, %struct.RaytracingInstanceDesc* %0, i32 0, i32 1
  %1 = load i32, i32* %InstanceIDAndMask, align 4, !tbaa !223
  %shr = lshr i32 %1, 24
  ret i32 %shr
}

; Function Attrs: alwaysinline nounwind
define void @"\01?TriangleToRawData@@YAXUTriangle@@AIAV?$vector@I$03@@1AIAI@Z"(%struct.Triangle* nocapture readonly, <4 x i32>* noalias nocapture dereferenceable(16), <4 x i32>* noalias nocapture dereferenceable(16), i32* noalias nocapture dereferenceable(4)) #5 {
entry:
  %v0 = getelementptr inbounds %struct.Triangle, %struct.Triangle* %0, i32 0, i32 0
  %4 = load <3 x float>, <3 x float>* %v0, align 4
  %5 = extractelement <3 x float> %4, i32 2
  %6 = extractelement <3 x float> %4, i32 1
  %7 = extractelement <3 x float> %4, i32 0
  %v1 = getelementptr inbounds %struct.Triangle, %struct.Triangle* %0, i32 0, i32 1
  %8 = load <3 x float>, <3 x float>* %v1, align 4
  %9 = extractelement <3 x float> %8, i32 0
  %.i05 = bitcast float %7 to i32
  %.i16 = bitcast float %6 to i32
  %.i27 = bitcast float %5 to i32
  %.i3 = bitcast float %9 to i32
  %.upto016 = insertelement <4 x i32> undef, i32 %.i05, i32 0
  %.upto117 = insertelement <4 x i32> %.upto016, i32 %.i16, i32 1
  %.upto2 = insertelement <4 x i32> %.upto117, i32 %.i27, i32 2
  %10 = insertelement <4 x i32> %.upto2, i32 %.i3, i32 3
  store <4 x i32> %10, <4 x i32>* %1, align 4, !tbaa !220
  %11 = extractelement <3 x float> %8, i32 2
  %12 = extractelement <3 x float> %8, i32 1
  %v2 = getelementptr inbounds %struct.Triangle, %struct.Triangle* %0, i32 0, i32 2
  %13 = load <3 x float>, <3 x float>* %v2, align 4
  %14 = extractelement <3 x float> %13, i32 1
  %15 = extractelement <3 x float> %13, i32 0
  %.i012 = bitcast float %12 to i32
  %.i113 = bitcast float %11 to i32
  %.i214 = bitcast float %15 to i32
  %.i315 = bitcast float %14 to i32
  %.upto022 = insertelement <4 x i32> undef, i32 %.i012, i32 0
  %.upto123 = insertelement <4 x i32> %.upto022, i32 %.i113, i32 1
  %.upto224 = insertelement <4 x i32> %.upto123, i32 %.i214, i32 2
  %16 = insertelement <4 x i32> %.upto224, i32 %.i315, i32 3
  store <4 x i32> %16, <4 x i32>* %2, align 4, !tbaa !220
  %17 = extractelement <3 x float> %13, i32 2
  %18 = bitcast i32* %3 to float*
  store float %17, float* %18, align 4, !tbaa !223
  ret void
}

; Function Attrs: alwaysinline nounwind
define void @"\01?RawDataToAABB@@YA?AUAABB@@V?$vector@H$03@@0@Z"(%struct.AABB* noalias nocapture sret, <4 x i32>, <4 x i32>) #5 {
entry:
  %3 = extractelement <4 x i32> %1, i32 0
  %4 = bitcast i32 %3 to float
  %5 = extractelement <4 x i32> %1, i32 1
  %6 = bitcast i32 %5 to float
  %7 = extractelement <4 x i32> %1, i32 2
  %8 = bitcast i32 %7 to float
  %9 = extractelement <4 x i32> %2, i32 0
  %10 = bitcast i32 %9 to float
  %11 = extractelement <4 x i32> %2, i32 1
  %12 = bitcast i32 %11 to float
  %13 = extractelement <4 x i32> %2, i32 2
  %14 = bitcast i32 %13 to float
  %sub.i.i0 = fsub fast float %4, %10
  %sub.i.i1 = fsub fast float %6, %12
  %sub.i.i2 = fsub fast float %8, %14
  %sub.i.upto0 = insertelement <3 x float> undef, float %sub.i.i0, i32 0
  %sub.i.upto1 = insertelement <3 x float> %sub.i.upto0, float %sub.i.i1, i32 1
  %sub.i = insertelement <3 x float> %sub.i.upto1, float %sub.i.i2, i32 2
  %min.i = getelementptr inbounds %struct.AABB, %struct.AABB* %0, i32 0, i32 0
  store <3 x float> %sub.i, <3 x float>* %min.i, align 4, !tbaa !220, !alias.scope !303
  %add.i.i0 = fadd fast float %10, %4
  %add.i.i1 = fadd fast float %12, %6
  %add.i.i2 = fadd fast float %14, %8
  %add.i.upto0 = insertelement <3 x float> undef, float %add.i.i0, i32 0
  %add.i.upto1 = insertelement <3 x float> %add.i.upto0, float %add.i.i1, i32 1
  %add.i = insertelement <3 x float> %add.i.upto1, float %add.i.i2, i32 2
  %max.i = getelementptr inbounds %struct.AABB, %struct.AABB* %0, i32 0, i32 1
  store <3 x float> %add.i, <3 x float>* %max.i, align 4, !tbaa !220, !alias.scope !303
  ret void
}

; Function Attrs: alwaysinline nounwind
define void @"\01?MyClosestHitShader_AABB@@YAXUHitData@@UProceduralPrimitiveAttributes@@@Z"(%struct.HitData* noalias nocapture, %struct.ProceduralPrimitiveAttributes* nocapture readonly) #5 {
entry:
  %2 = load %"class.StructuredBuffer<AABBPrimitiveAttributes>", %"class.StructuredBuffer<AABBPrimitiveAttributes>"* @"\01?g_AABBPrimitiveAttributes@@3V?$StructuredBuffer@UAABBPrimitiveAttributes@@@@A", align 4
  %3 = load %g_sceneCB, %g_sceneCB* @g_sceneCB, align 4
  %g_sceneCB14 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.g_sceneCB(i32 160, %g_sceneCB %3)
  %WorldRayOrigin = call float @dx.op.worldRayOrigin.f32(i32 147, i8 0)
  %WorldRayOrigin12 = call float @dx.op.worldRayOrigin.f32(i32 147, i8 1)
  %WorldRayOrigin13 = call float @dx.op.worldRayOrigin.f32(i32 147, i8 2)
  %RayTCurrent = call float @dx.op.rayTCurrent.f32(i32 154)
  %WorldRayDirection = call float @dx.op.worldRayDirection.f32(i32 148, i8 0)
  %WorldRayDirection10 = call float @dx.op.worldRayDirection.f32(i32 148, i8 1)
  %WorldRayDirection11 = call float @dx.op.worldRayDirection.f32(i32 148, i8 2)
  %mul.i.i0 = fmul fast float %WorldRayDirection, %RayTCurrent
  %mul.i.i1 = fmul fast float %WorldRayDirection10, %RayTCurrent
  %mul.i.i2 = fmul fast float %WorldRayDirection11, %RayTCurrent
  %normal = getelementptr inbounds %struct.ProceduralPrimitiveAttributes, %struct.ProceduralPrimitiveAttributes* %1, i32 0, i32 0
  %4 = load <3 x float>, <3 x float>* %normal, align 4, !tbaa !220
  %PrimitiveID = call i32 @dx.op.primitiveID.i32(i32 108)
  %5 = call %dx.types.Handle @"dx.op.createHandleFromResourceStructForLib.class.StructuredBuffer<AABBPrimitiveAttributes>"(i32 160, %"class.StructuredBuffer<AABBPrimitiveAttributes>" %2)
  %6 = call %dx.types.ResRet.f32 @dx.op.bufferLoad.f32(i32 68, %dx.types.Handle %5, i32 %PrimitiveID, i32 64)
  %7 = extractvalue %dx.types.ResRet.f32 %6, 0
  %8 = extractvalue %dx.types.ResRet.f32 %6, 1
  %9 = extractvalue %dx.types.ResRet.f32 %6, 2
  %10 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %g_sceneCB14, i32 5)
  %11 = extractvalue %dx.types.CBufRet.f32 %10, 0
  %12 = extractvalue %dx.types.CBufRet.f32 %10, 1
  %13 = extractvalue %dx.types.CBufRet.f32 %10, 2
  %WorldRayOrigin.neg = fsub fast float -0.000000e+00, %WorldRayOrigin
  %add.i.i0.neg = fsub fast float %WorldRayOrigin.neg, %mul.i.i0
  %sub.i.i0 = fadd fast float %add.i.i0.neg, %11
  %WorldRayOrigin12.neg = fsub fast float -0.000000e+00, %WorldRayOrigin12
  %add.i.i1.neg = fsub fast float %WorldRayOrigin12.neg, %mul.i.i1
  %sub.i.i1 = fadd fast float %add.i.i1.neg, %12
  %WorldRayOrigin13.neg = fsub fast float -0.000000e+00, %WorldRayOrigin13
  %add.i.i2.neg = fsub fast float %WorldRayOrigin13.neg, %mul.i.i2
  %sub.i.i2 = fadd fast float %add.i.i2.neg, %13
  %14 = fmul fast float %sub.i.i0, %sub.i.i0
  %15 = fmul fast float %sub.i.i1, %sub.i.i1
  %16 = fadd fast float %14, %15
  %17 = fmul fast float %sub.i.i2, %sub.i.i2
  %18 = fadd fast float %16, %17
  %Sqrt = call float @dx.op.unary.f32(i32 24, float %18)
  %.i0 = fdiv fast float %sub.i.i0, %Sqrt
  %.i1 = fdiv fast float %sub.i.i1, %Sqrt
  %.i2 = fdiv fast float %sub.i.i2, %Sqrt
  %19 = extractelement <3 x float> %4, i64 0
  %20 = extractelement <3 x float> %4, i64 1
  %21 = extractelement <3 x float> %4, i64 2
  %22 = call float @dx.op.dot3.f32(i32 55, float %.i0, float %.i1, float %.i2, float %19, float %20, float %21)
  %FMax = call float @dx.op.binary.f32(i32 35, float 0.000000e+00, float %22)
  %23 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %g_sceneCB14, i32 7)
  %24 = extractvalue %dx.types.CBufRet.f32 %23, 0
  %25 = extractvalue %dx.types.CBufRet.f32 %23, 1
  %26 = extractvalue %dx.types.CBufRet.f32 %23, 2
  %27 = extractvalue %dx.types.CBufRet.f32 %23, 3
  %mul.i.9.i3 = fmul fast float %27, %FMax
  %mul.i.9.i0 = fmul fast float %FMax, %7
  %mul5.i0 = fmul fast float %mul.i.9.i0, %24
  %mul.i.9.i1 = fmul fast float %FMax, %8
  %mul5.i1 = fmul fast float %mul.i.9.i1, %25
  %mul.i.9.i2 = fmul fast float %FMax, %9
  %mul5.i2 = fmul fast float %mul.i.9.i2, %26
  %28 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %g_sceneCB14, i32 6)
  %29 = extractvalue %dx.types.CBufRet.f32 %28, 0
  %30 = extractvalue %dx.types.CBufRet.f32 %28, 1
  %31 = extractvalue %dx.types.CBufRet.f32 %28, 2
  %32 = extractvalue %dx.types.CBufRet.f32 %28, 3
  %add.i0 = fadd fast float %mul5.i0, %29
  %add.i1 = fadd fast float %mul5.i1, %30
  %add.i2 = fadd fast float %mul5.i2, %31
  %add.i3 = fadd fast float %32, %mul.i.9.i3
  %add.upto0 = insertelement <4 x float> undef, float %add.i0, i32 0
  %add.upto1 = insertelement <4 x float> %add.upto0, float %add.i1, i32 1
  %add.upto2 = insertelement <4 x float> %add.upto1, float %add.i2, i32 2
  %add = insertelement <4 x float> %add.upto2, float %add.i3, i32 3
  %33 = getelementptr inbounds %struct.HitData, %struct.HitData* %0, i32 0, i32 0
  store <4 x float> %add, <4 x float>* %33, align 4
  ret void
}

; Function Attrs: alwaysinline
define void @main() #8 {
entry:
  %0 = load %struct.ByteAddressBuffer, %struct.ByteAddressBuffer* @"\01?RayGenShaderTable@@3UByteAddressBuffer@@A", align 4
  %1 = load %Constants, %Constants* @Constants, align 4
  %Constants = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.Constants(i32 160, %Constants %1)
  %2 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.ByteAddressBuffer(i32 160, %struct.ByteAddressBuffer %0)
  %3 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %2, i32 0, i32 undef)
  %4 = extractvalue %dx.types.ResRet.i32 %3, 0
  %5 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %Constants, i32 0)
  %6 = extractvalue %dx.types.CBufRet.i32 %5, 1
  %7 = extractvalue %dx.types.CBufRet.i32 %5, 0
  call void @"\01?Fallback_Scheduler@@YAXHII@Z"(i32 %4, i32 %7, i32 %6) #2
  ret void
}

; Function Attrs: alwaysinline nounwind
define void @"\01?BoundingBoxToAABB@@YA?AUAABB@@UBoundingBox@@@Z"(%struct.AABB* noalias nocapture sret, %struct.BoundingBox* nocapture readonly) #5 {
entry:
  %center = getelementptr inbounds %struct.BoundingBox, %struct.BoundingBox* %1, i32 0, i32 0
  %2 = load <3 x float>, <3 x float>* %center, align 4, !tbaa !220
  %.i03 = extractelement <3 x float> %2, i32 0
  %.i14 = extractelement <3 x float> %2, i32 1
  %.i25 = extractelement <3 x float> %2, i32 2
  %halfDim = getelementptr inbounds %struct.BoundingBox, %struct.BoundingBox* %1, i32 0, i32 1
  %3 = load <3 x float>, <3 x float>* %halfDim, align 4, !tbaa !220
  %.i0 = extractelement <3 x float> %3, i32 0
  %sub.i0 = fsub fast float %.i03, %.i0
  %.i1 = extractelement <3 x float> %3, i32 1
  %sub.i1 = fsub fast float %.i14, %.i1
  %.i2 = extractelement <3 x float> %3, i32 2
  %sub.i2 = fsub fast float %.i25, %.i2
  %sub.upto0 = insertelement <3 x float> undef, float %sub.i0, i32 0
  %sub.upto1 = insertelement <3 x float> %sub.upto0, float %sub.i1, i32 1
  %sub = insertelement <3 x float> %sub.upto1, float %sub.i2, i32 2
  %min = getelementptr inbounds %struct.AABB, %struct.AABB* %0, i32 0, i32 0
  store <3 x float> %sub, <3 x float>* %min, align 4, !tbaa !220
  %add.i0 = fadd fast float %.i0, %.i03
  %add.i1 = fadd fast float %.i1, %.i14
  %add.i2 = fadd fast float %.i2, %.i25
  %add.upto0 = insertelement <3 x float> undef, float %add.i0, i32 0
  %add.upto1 = insertelement <3 x float> %add.upto0, float %add.i1, i32 1
  %add = insertelement <3 x float> %add.upto1, float %add.i2, i32 2
  %max = getelementptr inbounds %struct.AABB, %struct.AABB* %0, i32 0, i32 1
  store <3 x float> %add, <3 x float>* %max, align 4, !tbaa !220
  ret void
}

; Function Attrs: alwaysinline nounwind readnone
define void @"\01?LogFloat@@YAXM@Z"(float) #4 {
entry:
  ret void
}

; Function Attrs: alwaysinline nounwind
define void @"\01?BVHReadPrimitiveMetaData@@YA?AUPrimitiveMetaData@@URWByteAddressBufferPointer@@H@Z"(%struct.PrimitiveMetaData* noalias nocapture sret, %struct.RWByteAddressBufferPointer* nocapture readonly, i32) #5 {
entry:
  %offsetInBytes.i = getelementptr inbounds %struct.RWByteAddressBufferPointer, %struct.RWByteAddressBufferPointer* %1, i32 0, i32 1
  %3 = load i32, i32* %offsetInBytes.i, align 4, !tbaa !223
  %add.i = add i32 %3, 8
  %4 = getelementptr inbounds %struct.RWByteAddressBufferPointer, %struct.RWByteAddressBufferPointer* %1, i32 0, i32 0
  %5 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %4, align 4
  %6 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %5)
  %7 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %6, i32 %add.i, i32 undef)
  %8 = extractvalue %dx.types.ResRet.i32 %7, 0
  %mul.i = shl i32 %2, 3
  %add2.i = add i32 %3, %mul.i
  %add.i.3 = add i32 %add2.i, %8
  %9 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %6, i32 %add.i.3, i32 undef)
  %10 = extractvalue %dx.types.ResRet.i32 %9, 0
  %11 = extractvalue %dx.types.ResRet.i32 %9, 1
  %GeometryContributionToHitGroupIndex = getelementptr inbounds %struct.PrimitiveMetaData, %struct.PrimitiveMetaData* %0, i32 0, i32 0
  store i32 %10, i32* %GeometryContributionToHitGroupIndex, align 4, !tbaa !223
  %PrimitiveIndex = getelementptr inbounds %struct.PrimitiveMetaData, %struct.PrimitiveMetaData* %0, i32 0, i32 1
  store i32 %11, i32* %PrimitiveIndex, align 4, !tbaa !223
  ret void
}

; Function Attrs: alwaysinline nounwind
define void @"\01?GetBoxFromChildBoxes@@YA?AUBoundingBox@@U1@H0HAIAV?$vector@I$01@@@Z"(%struct.BoundingBox* noalias nocapture sret, %struct.BoundingBox* nocapture readonly, i32, %struct.BoundingBox* nocapture readonly, i32, <2 x i32>* noalias nocapture dereferenceable(8)) #5 {
entry:
  %center.i = getelementptr inbounds %struct.BoundingBox, %struct.BoundingBox* %3, i32 0, i32 0
  %6 = load <3 x float>, <3 x float>* %center.i, align 4, !tbaa !220
  %.i023 = extractelement <3 x float> %6, i32 0
  %.i124 = extractelement <3 x float> %6, i32 1
  %.i225 = extractelement <3 x float> %6, i32 2
  %halfDim.i = getelementptr inbounds %struct.BoundingBox, %struct.BoundingBox* %3, i32 0, i32 1
  %7 = load <3 x float>, <3 x float>* %halfDim.i, align 4, !tbaa !220
  %.i0 = extractelement <3 x float> %7, i32 0
  %sub.i.i0 = fsub fast float %.i023, %.i0
  %.i1 = extractelement <3 x float> %7, i32 1
  %sub.i.i1 = fsub fast float %.i124, %.i1
  %.i2 = extractelement <3 x float> %7, i32 2
  %sub.i.i2 = fsub fast float %.i225, %.i2
  %center.i.16 = getelementptr inbounds %struct.BoundingBox, %struct.BoundingBox* %1, i32 0, i32 0
  %8 = load <3 x float>, <3 x float>* %center.i.16, align 4, !tbaa !220
  %.i027 = extractelement <3 x float> %8, i32 0
  %.i129 = extractelement <3 x float> %8, i32 1
  %.i231 = extractelement <3 x float> %8, i32 2
  %halfDim.i.17 = getelementptr inbounds %struct.BoundingBox, %struct.BoundingBox* %1, i32 0, i32 1
  %9 = load <3 x float>, <3 x float>* %halfDim.i.17, align 4, !tbaa !220
  %.i026 = extractelement <3 x float> %9, i32 0
  %sub.i.18.i0 = fsub fast float %.i027, %.i026
  %.i128 = extractelement <3 x float> %9, i32 1
  %sub.i.18.i1 = fsub fast float %.i129, %.i128
  %.i230 = extractelement <3 x float> %9, i32 2
  %sub.i.18.i2 = fsub fast float %.i231, %.i230
  %FMin = call float @dx.op.binary.f32(i32 36, float %sub.i.18.i0, float %sub.i.i0)
  %FMin21 = call float @dx.op.binary.f32(i32 36, float %sub.i.18.i1, float %sub.i.i1)
  %FMin22 = call float @dx.op.binary.f32(i32 36, float %sub.i.18.i2, float %sub.i.i2)
  %add.i.15.i0 = fadd fast float %.i0, %.i023
  %add.i.15.i1 = fadd fast float %.i1, %.i124
  %add.i.15.i2 = fadd fast float %.i2, %.i225
  %add.i.12.i0 = fadd fast float %.i026, %.i027
  %add.i.12.i1 = fadd fast float %.i128, %.i129
  %add.i.12.i2 = fadd fast float %.i230, %.i231
  %FMax = call float @dx.op.binary.f32(i32 35, float %add.i.12.i0, float %add.i.15.i0)
  %FMax19 = call float @dx.op.binary.f32(i32 35, float %add.i.12.i1, float %add.i.15.i1)
  %FMax20 = call float @dx.op.binary.f32(i32 35, float %add.i.12.i2, float %add.i.15.i2)
  %add.i.i0 = fadd fast float %FMax, %FMin
  %add.i.i1 = fadd fast float %FMax19, %FMin21
  %add.i.i2 = fadd fast float %FMax20, %FMin22
  %mul.i.i0 = fmul fast float %add.i.i0, 5.000000e-01
  %mul.i.i1 = fmul fast float %add.i.i1, 5.000000e-01
  %mul.i.i2 = fmul fast float %add.i.i2, 5.000000e-01
  %mul.i.upto0 = insertelement <3 x float> undef, float %mul.i.i0, i32 0
  %mul.i.upto1 = insertelement <3 x float> %mul.i.upto0, float %mul.i.i1, i32 1
  %mul.i = insertelement <3 x float> %mul.i.upto1, float %mul.i.i2, i32 2
  %sub.i.8.i0 = fsub fast float %FMax, %mul.i.i0
  %sub.i.8.i1 = fsub fast float %FMax19, %mul.i.i1
  %sub.i.8.i2 = fsub fast float %FMax20, %mul.i.i2
  %sub.i.8.upto0 = insertelement <3 x float> undef, float %sub.i.8.i0, i32 0
  %sub.i.8.upto1 = insertelement <3 x float> %sub.i.8.upto0, float %sub.i.8.i1, i32 1
  %sub.i.8 = insertelement <3 x float> %sub.i.8.upto1, float %sub.i.8.i2, i32 2
  %and.i = and i32 %2, 16777215
  %10 = insertelement <2 x i32> undef, i32 %and.i, i32 0
  %11 = insertelement <2 x i32> %10, i32 %4, i32 1
  store <2 x i32> %11, <2 x i32>* %5, align 4, !tbaa !220
  %12 = getelementptr inbounds %struct.BoundingBox, %struct.BoundingBox* %0, i32 0, i32 0
  store <3 x float> %mul.i, <3 x float>* %12, align 4
  %13 = getelementptr inbounds %struct.BoundingBox, %struct.BoundingBox* %0, i32 0, i32 1
  store <3 x float> %sub.i.8, <3 x float>* %13, align 4
  ret void
}

; Function Attrs: alwaysinline nounwind readnone
define i1 @"\01?GetBoolFlag@@YA_NII@Z"(i32, i32) #4 {
entry:
  %and = and i32 %1, %0
  %tobool = icmp ne i32 %and, 0
  ret i1 %tobool
}

; Function Attrs: alwaysinline nounwind readonly
define i32 @"\01?GetInstanceFlags@@YAIURaytracingInstanceDesc@@@Z"(%struct.RaytracingInstanceDesc* nocapture readonly) #6 {
entry:
  %InstanceContributionToHitGroupIndexAndFlags = getelementptr inbounds %struct.RaytracingInstanceDesc, %struct.RaytracingInstanceDesc* %0, i32 0, i32 2
  %1 = load i32, i32* %InstanceContributionToHitGroupIndexAndFlags, align 4, !tbaa !223
  %shr = lshr i32 %1, 24
  ret i32 %shr
}

; Function Attrs: alwaysinline nounwind readonly
define i32 @"\01?GetOffsetToVertices@@YAHURWByteAddressBufferPointer@@@Z"(%struct.RWByteAddressBufferPointer* nocapture readonly) #6 {
entry:
  %offsetInBytes = getelementptr inbounds %struct.RWByteAddressBufferPointer, %struct.RWByteAddressBufferPointer* %0, i32 0, i32 1
  %1 = load i32, i32* %offsetInBytes, align 4, !tbaa !223
  %add = add i32 %1, 4
  %2 = getelementptr inbounds %struct.RWByteAddressBufferPointer, %struct.RWByteAddressBufferPointer* %0, i32 0, i32 0
  %3 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %2, align 4
  %4 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %3)
  %5 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %4, i32 %add, i32 undef)
  %6 = extractvalue %dx.types.ResRet.i32 %5, 0
  %add2 = add i32 %6, %1
  ret i32 %add2
}

; Function Attrs: alwaysinline nounwind
define void @"\01?AABBToRawData@@YAXUAABB@@AIAV?$vector@I$03@@AIAV?$vector@I$01@@@Z"(%struct.AABB* nocapture readonly, <4 x i32>* noalias nocapture dereferenceable(16), <2 x i32>* noalias nocapture dereferenceable(8)) #5 {
entry:
  %min = getelementptr inbounds %struct.AABB, %struct.AABB* %0, i32 0, i32 0
  %3 = load <3 x float>, <3 x float>* %min, align 4
  %4 = extractelement <3 x float> %3, i32 2
  %5 = extractelement <3 x float> %3, i32 1
  %6 = extractelement <3 x float> %3, i32 0
  %max = getelementptr inbounds %struct.AABB, %struct.AABB* %0, i32 0, i32 1
  %7 = load <3 x float>, <3 x float>* %max, align 4
  %8 = extractelement <3 x float> %7, i32 0
  %.i03 = bitcast float %6 to i32
  %.i14 = bitcast float %5 to i32
  %.i25 = bitcast float %4 to i32
  %.i3 = bitcast float %8 to i32
  %.upto010 = insertelement <4 x i32> undef, i32 %.i03, i32 0
  %.upto111 = insertelement <4 x i32> %.upto010, i32 %.i14, i32 1
  %.upto2 = insertelement <4 x i32> %.upto111, i32 %.i25, i32 2
  %9 = insertelement <4 x i32> %.upto2, i32 %.i3, i32 3
  store <4 x i32> %9, <4 x i32>* %1, align 4, !tbaa !220
  %10 = extractelement <3 x float> %7, i32 2
  %11 = extractelement <3 x float> %7, i32 1
  %.i08 = bitcast float %11 to i32
  %.i19 = bitcast float %10 to i32
  %.upto014 = insertelement <2 x i32> undef, i32 %.i08, i32 0
  %12 = insertelement <2 x i32> %.upto014, i32 %.i19, i32 1
  store <2 x i32> %12, <2 x i32>* %2, align 4, !tbaa !220
  ret void
}

; Function Attrs: alwaysinline nounwind readnone
define void @"\01?LogInt@@YAXH@Z"(i32) #4 {
entry:
  ret void
}

; Function Attrs: alwaysinline nounwind readnone
define void @"\01?LogInt3@@YAXV?$vector@H$02@@@Z"(<3 x i32>) #4 {
entry:
  ret void
}

; Function Attrs: alwaysinline nounwind
define void @"\01?SetBoolFlag@@YAXAIAII_N@Z"(i32* noalias nocapture dereferenceable(4), i32, i1 zeroext) #5 {
entry:
  br i1 %2, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  %3 = load i32, i32* %0, align 4, !tbaa !223
  %or = or i32 %3, %1
  br label %if.end

if.else:                                          ; preds = %entry
  %neg = xor i32 %1, -1
  %4 = load i32, i32* %0, align 4, !tbaa !223
  %and = and i32 %4, %neg
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %storemerge = phi i32 [ %and, %if.else ], [ %or, %if.then ]
  store i32 %storemerge, i32* %0, align 4, !tbaa !223
  ret void
}

; Function Attrs: alwaysinline nounwind
define i1 @"\01?Traverse@@YA_NIII@Z"(i32, i32, i32) #5 {
entry:
  %3 = load %struct.ByteAddressBuffer, %struct.ByteAddressBuffer* @"\01?HitGroupShaderTable@@3UByteAddressBuffer@@A", align 4
  %4 = load %AccelerationStructureList, %AccelerationStructureList* @AccelerationStructureList, align 4
  %5 = load %Constants, %Constants* @Constants, align 4
  %AccelerationStructureList = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.AccelerationStructureList(i32 160, %AccelerationStructureList %4)
  %Constants = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.Constants(i32 160, %Constants %5)
  %6 = alloca [3 x float], align 4
  %7 = alloca [3 x float], align 4
  %8 = alloca [3 x float], align 4
  %9 = alloca [3 x float], align 4
  %10 = alloca [3 x float], align 4
  %11 = alloca [3 x float], align 4
  %nodesToProcess = alloca [2 x i32], align 4
  %attr = alloca %struct.BuiltInTriangleIntersectionAttributes, align 4
  %call = call i32 @"\01?Fallback_GroupIndex@@YAIXZ"() #2
  %WorldRayDirection = call float @dx.op.worldRayDirection.f32(i32 148, i8 0)
  %WorldRayDirection823 = call float @dx.op.worldRayDirection.f32(i32 148, i8 1)
  %WorldRayDirection824 = call float @dx.op.worldRayDirection.f32(i32 148, i8 2)
  %WorldRayOrigin = call float @dx.op.worldRayOrigin.f32(i32 147, i8 0)
  %WorldRayOrigin815 = call float @dx.op.worldRayOrigin.f32(i32 147, i8 1)
  %WorldRayOrigin816 = call float @dx.op.worldRayOrigin.f32(i32 147, i8 2)
  %12 = getelementptr inbounds [3 x float], [3 x float]* %11, i32 0, i32 0
  store float %WorldRayDirection, float* %12, align 4
  %13 = getelementptr inbounds [3 x float], [3 x float]* %11, i32 0, i32 1
  store float %WorldRayDirection823, float* %13, align 4
  %14 = getelementptr inbounds [3 x float], [3 x float]* %11, i32 0, i32 2
  store float %WorldRayDirection824, float* %14, align 4
  %.i0 = fdiv fast float 1.000000e+00, %WorldRayDirection
  %.i1 = fdiv fast float 1.000000e+00, %WorldRayDirection823
  %.i2 = fdiv fast float 1.000000e+00, %WorldRayDirection824
  %mul.i.i0 = fmul fast float %.i0, %WorldRayOrigin
  %mul.i.i1 = fmul fast float %.i1, %WorldRayOrigin815
  %mul.i.i2 = fmul fast float %.i2, %WorldRayOrigin816
  %FAbs812 = call float @dx.op.unary.f32(i32 6, float %WorldRayDirection)
  %FAbs813 = call float @dx.op.unary.f32(i32 6, float %WorldRayDirection823)
  %FAbs814 = call float @dx.op.unary.f32(i32 6, float %WorldRayDirection824)
  %cmp.i.i = fcmp fast ogt float %FAbs812, %FAbs813
  %cmp1.i.i = fcmp fast ogt float %FAbs812, %FAbs814
  %15 = and i1 %cmp.i.i, %cmp1.i.i
  br i1 %15, label %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i", label %if.else.i.i

if.else.i.i:                                      ; preds = %entry
  %cmp4.i.i = fcmp fast ogt float %FAbs813, %FAbs814
  br i1 %cmp4.i.i, label %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i", label %if.else.8.i.i

if.else.8.i.i:                                    ; preds = %if.else.i.i
  br label %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i"

"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i": ; preds = %if.else.8.i.i, %if.else.i.i, %entry
  %retval.i.i.0 = phi i32 [ 2, %if.else.8.i.i ], [ 0, %entry ], [ 1, %if.else.i.i ]
  %add.i = add nuw nsw i32 %retval.i.i.0, 1
  %rem.i = urem i32 %add.i, 3
  %add4.i = add nuw nsw i32 %retval.i.i.0, 2
  %rem5.i = urem i32 %add4.i, 3
  %16 = getelementptr [3 x float], [3 x float]* %11, i32 0, i32 %retval.i.i.0
  %17 = load float, float* %16, align 4, !tbaa !228, !noalias !306
  %cmp.i = fcmp fast olt float %17, 0.000000e+00
  %currentRayData.5.0.i0 = select i1 %cmp.i, i32 %rem5.i, i32 %rem.i
  %currentRayData.5.0.i1 = select i1 %cmp.i, i32 %rem.i, i32 %rem5.i
  %18 = getelementptr [3 x float], [3 x float]* %11, i32 0, i32 %currentRayData.5.0.i0
  %19 = load float, float* %18, align 4, !tbaa !228, !noalias !306
  %20 = getelementptr [3 x float], [3 x float]* %11, i32 0, i32 %currentRayData.5.0.i1
  %21 = load float, float* %20, align 4, !tbaa !228, !noalias !306
  %22 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %AccelerationStructureList, i32 0)
  %23 = extractvalue %dx.types.CBufRet.i32 %22, 0
  %24 = extractvalue %dx.types.CBufRet.i32 %22, 1
  %arrayidx = getelementptr inbounds [2 x i32], [2 x i32]* %nodesToProcess, i32 0, i32 0
  store i32 0, i32* %arrayidx, align 4, !tbaa !223
  %arrayidx.i.i = getelementptr inbounds [0 x %struct.RWByteAddressBuffer], [0 x %struct.RWByteAddressBuffer]* @"\01?DescriptorHeapBufferTable@@3PAURWByteAddressBuffer@@A", i32 0, i32 %24, !dx.nonuniform !233
  %25 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %arrayidx.i.i, align 4, !noalias !115
  %add = add i32 %23, 4
  %26 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %25)
  %27 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %26, i32 %add, i32 undef)
  %28 = extractvalue %dx.types.ResRet.i32 %27, 0
  %add5 = add i32 %28, %23
  %add.i.i = add i32 %23, 16
  %29 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %26, i32 %add.i.i, i32 undef)
  %30 = extractvalue %dx.types.ResRet.i32 %29, 0
  %31 = extractvalue %dx.types.ResRet.i32 %29, 1
  %32 = extractvalue %dx.types.ResRet.i32 %29, 2
  %add.i.214 = add i32 %23, 32
  %33 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %26, i32 %add.i.214, i32 undef)
  %34 = extractvalue %dx.types.ResRet.i32 %33, 0
  %35 = extractvalue %dx.types.ResRet.i32 %33, 1
  %36 = extractvalue %dx.types.ResRet.i32 %33, 2
  %37 = bitcast i32 %30 to float
  %38 = bitcast i32 %31 to float
  %39 = bitcast i32 %32 to float
  %40 = bitcast i32 %34 to float
  %41 = bitcast i32 %35 to float
  %42 = bitcast i32 %36 to float
  %call6 = call float @"\01?Fallback_RayTCurrent@@YAMXZ"() #2
  %43 = fsub fast float %37, %WorldRayOrigin
  %44 = fmul fast float %43, %.i0
  %45 = fsub fast float %38, %WorldRayOrigin815
  %46 = fmul fast float %45, %.i1
  %47 = fsub fast float %39, %WorldRayOrigin816
  %48 = fmul fast float %47, %.i2
  %FAbs809 = call float @dx.op.unary.f32(i32 6, float %.i0)
  %FAbs810 = call float @dx.op.unary.f32(i32 6, float %.i1)
  %FAbs811 = call float @dx.op.unary.f32(i32 6, float %.i2)
  %mul1.i.i0 = fmul fast float %40, %FAbs809
  %mul1.i.i1 = fmul fast float %FAbs810, %41
  %mul1.i.i2 = fmul fast float %FAbs811, %42
  %add.i.216.i0 = fadd fast float %mul1.i.i0, %44
  %add.i.216.i1 = fadd fast float %mul1.i.i1, %46
  %add.i.216.i2 = fadd fast float %mul1.i.i2, %48
  %sub4.i.i0 = fsub fast float %44, %mul1.i.i0
  %sub4.i.i1 = fsub fast float %46, %mul1.i.i1
  %sub4.i.i2 = fsub fast float %48, %mul1.i.i2
  %FMax848 = call float @dx.op.binary.f32(i32 35, float %sub4.i.i0, float %sub4.i.i1)
  %FMax847 = call float @dx.op.binary.f32(i32 35, float %FMax848, float %sub4.i.i2)
  %FMin846 = call float @dx.op.binary.f32(i32 36, float %add.i.216.i0, float %add.i.216.i1)
  %FMin845 = call float @dx.op.binary.f32(i32 36, float %FMin846, float %add.i.216.i2)
  %FMax844 = call float @dx.op.binary.f32(i32 35, float %FMax847, float 0.000000e+00)
  %FMin843 = call float @dx.op.binary.f32(i32 36, float %FMin845, float %call6)
  %cmp.i.217 = fcmp fast olt float %FMax844, %FMin843
  br i1 %cmp.i.217, label %if.then, label %if.end

if.then:                                          ; preds = %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i"
  %div14.i = fdiv float %21, %17
  %div.i = fdiv float %19, %17
  %arrayidx.i = getelementptr [2048 x i32], [2048 x i32] addrspace(3)* @"\01?stack@@3PAIA", i32 0, i32 %call
  store i32 0, i32 addrspace(3)* %arrayidx.i, align 4, !tbaa !223, !noalias !309
  store i32 1, i32* %arrayidx, align 4, !tbaa !223
  call void @"\01?Fallback_SetInstanceIndex@@YAXI@Z"(i32 -1) #2
  %49 = getelementptr inbounds [3 x float], [3 x float]* %10, i32 0, i32 0
  %50 = getelementptr inbounds [3 x float], [3 x float]* %10, i32 0, i32 1
  %51 = getelementptr inbounds [3 x float], [3 x float]* %10, i32 0, i32 2
  %arrayidx42 = getelementptr inbounds [2 x i32], [2 x i32]* %nodesToProcess, i32 0, i32 1
  %52 = getelementptr inbounds [3 x float], [3 x float]* %9, i32 0, i32 0
  %53 = getelementptr inbounds [3 x float], [3 x float]* %9, i32 0, i32 1
  %54 = getelementptr inbounds [3 x float], [3 x float]* %9, i32 0, i32 2
  %55 = getelementptr inbounds [3 x float], [3 x float]* %7, i32 0, i32 0
  %56 = getelementptr inbounds [3 x float], [3 x float]* %7, i32 0, i32 1
  %57 = getelementptr inbounds [3 x float], [3 x float]* %7, i32 0, i32 2
  %58 = getelementptr inbounds [3 x float], [3 x float]* %8, i32 0, i32 0
  %59 = getelementptr inbounds [3 x float], [3 x float]* %8, i32 0, i32 1
  %60 = getelementptr inbounds [3 x float], [3 x float]* %8, i32 0, i32 2
  %barycentrics = getelementptr inbounds %struct.BuiltInTriangleIntersectionAttributes, %struct.BuiltInTriangleIntersectionAttributes* %attr, i32 0, i32 0
  %61 = getelementptr inbounds [3 x float], [3 x float]* %6, i32 0, i32 0
  %62 = getelementptr inbounds [3 x float], [3 x float]* %6, i32 0, i32 1
  %63 = getelementptr inbounds [3 x float], [3 x float]* %6, i32 0, i32 2
  %cmp4.i.i.522 = fcmp fast ogt float %FAbs813, %FAbs814
  %brmerge = or i1 %15, %cmp4.i.i.522
  %64 = zext i1 %15 to i32
  %.mux = xor i32 %64, 1
  br label %while.body

if.end:                                           ; preds = %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i"
  call void @"\01?Fallback_SetInstanceIndex@@YAXI@Z"(i32 -1) #2
  br label %while.end

while.body:                                       ; preds = %do.end, %if.then
  %resultTriId.0754 = phi i32 [ %resultTriId.3, %do.end ], [ undef, %if.then ]
  %resultBary.0753.i0 = phi float [ %resultBary.3.i0, %do.end ], [ undef, %if.then ]
  %resultBary.0753.i1 = phi float [ %resultBary.3.i1, %do.end ], [ undef, %if.then ]
  %stackPointer.1751 = phi i32 [ %stackPointer.3, %do.end ], [ 1, %if.then ]
  %instanceId.0750 = phi i32 [ %instanceId.2, %do.end ], [ 0, %if.then ]
  %instanceOffset.0749 = phi i32 [ %instanceOffset.2, %do.end ], [ 0, %if.then ]
  %instanceFlags.0748 = phi i32 [ %instanceFlags.2, %do.end ], [ 0, %if.then ]
  %instanceIndex.0747 = phi i32 [ %instanceIndex.2, %do.end ], [ 0, %if.then ]
  %currentGpuVA.0746.i0 = phi i32 [ %281, %do.end ], [ %23, %if.then ]
  %currentGpuVA.0746.i1 = phi i32 [ %282, %do.end ], [ %24, %if.then ]
  %currentBVHIndex.0745 = phi i32 [ %dec196, %do.end ], [ 0, %if.then ]
  %flagContainer.0744 = phi i32 [ %flagContainer.3, %do.end ], [ 0, %if.then ]
  %currentRayData.5.1743.i0 = phi i32 [ %tmp199.5.0.i0, %do.end ], [ %currentRayData.5.0.i0, %if.then ]
  %currentRayData.5.1743.i1 = phi i32 [ %tmp199.5.0.i1, %do.end ], [ %currentRayData.5.0.i1, %if.then ]
  %currentRayData.5.1743.i2 = phi i32 [ %.mux., %do.end ], [ %retval.i.i.0, %if.then ]
  %currentRayData.4.0742.i0 = phi float [ %div.i.539, %do.end ], [ %div.i, %if.then ]
  %currentRayData.4.0742.i1 = phi float [ %div14.i.542, %do.end ], [ %div14.i, %if.then ]
  %.pn = phi float [ %275, %do.end ], [ %17, %if.then ]
  %.0733737.i0 = phi float [ %.4.i0, %do.end ], [ undef, %if.then ]
  %.0733737.i1 = phi float [ %.4.i1, %do.end ], [ undef, %if.then ]
  %currentRayData.4.0742.i2 = fdiv float 1.000000e+00, %.pn
  %arrayidx13.phi.trans.insert = getelementptr inbounds [2 x i32], [2 x i32]* %nodesToProcess, i32 0, i32 %currentBVHIndex.0745
  %.pre = load i32, i32* %arrayidx13.phi.trans.insert, align 4, !tbaa !223
  br label %do.body

do.body:                                          ; preds = %do.cond, %while.body
  %65 = phi i32 [ %.pre, %while.body ], [ %273, %do.cond ]
  %.1.i0 = phi float [ %.0733737.i0, %while.body ], [ %.4.i0, %do.cond ]
  %.1.i1 = phi float [ %.0733737.i1, %while.body ], [ %.4.i1, %do.cond ]
  %currentRayData.0.1.i0 = phi float [ %WorldRayOrigin, %while.body ], [ %currentRayData.0.2.i0, %do.cond ]
  %currentRayData.0.1.i1 = phi float [ %WorldRayOrigin815, %while.body ], [ %currentRayData.0.2.i1, %do.cond ]
  %currentRayData.0.1.i2 = phi float [ %WorldRayOrigin816, %while.body ], [ %currentRayData.0.2.i2, %do.cond ]
  %currentRayData.2.1.i0 = phi float [ %.i0, %while.body ], [ %currentRayData.2.2.i0, %do.cond ]
  %currentRayData.2.1.i1 = phi float [ %.i1, %while.body ], [ %currentRayData.2.2.i1, %do.cond ]
  %currentRayData.2.1.i2 = phi float [ %.i2, %while.body ], [ %currentRayData.2.2.i2, %do.cond ]
  %currentRayData.3.1.i0 = phi float [ %mul.i.i0, %while.body ], [ %currentRayData.3.2.i0, %do.cond ]
  %currentRayData.3.1.i1 = phi float [ %mul.i.i1, %while.body ], [ %currentRayData.3.2.i1, %do.cond ]
  %currentRayData.3.1.i2 = phi float [ %mul.i.i2, %while.body ], [ %currentRayData.3.2.i2, %do.cond ]
  %currentRayData.4.1.i0 = phi float [ %currentRayData.4.0742.i0, %while.body ], [ %currentRayData.4.2.i0, %do.cond ]
  %currentRayData.4.1.i1 = phi float [ %currentRayData.4.0742.i1, %while.body ], [ %currentRayData.4.2.i1, %do.cond ]
  %currentRayData.4.1.i2 = phi float [ %currentRayData.4.0742.i2, %while.body ], [ %currentRayData.4.2.i2, %do.cond ]
  %currentRayData.5.2.i0 = phi i32 [ %currentRayData.5.1743.i0, %while.body ], [ %currentRayData.5.3.i0, %do.cond ]
  %currentRayData.5.2.i1 = phi i32 [ %currentRayData.5.1743.i1, %while.body ], [ %currentRayData.5.3.i1, %do.cond ]
  %currentRayData.5.2.i2 = phi i32 [ %currentRayData.5.1743.i2, %while.body ], [ %currentRayData.5.3.i2, %do.cond ]
  %flagContainer.1 = phi i32 [ %flagContainer.0744, %while.body ], [ %flagContainer.3, %do.cond ]
  %currentBVHIndex.1 = phi i32 [ %currentBVHIndex.0745, %while.body ], [ %currentBVHIndex.2, %do.cond ]
  %currentGpuVA.1.i0 = phi i32 [ %currentGpuVA.0746.i0, %while.body ], [ %currentGpuVA.2.i0, %do.cond ]
  %currentGpuVA.1.i1 = phi i32 [ %currentGpuVA.0746.i1, %while.body ], [ %currentGpuVA.2.i1, %do.cond ]
  %instanceIndex.1 = phi i32 [ %instanceIndex.0747, %while.body ], [ %instanceIndex.2, %do.cond ]
  %instanceFlags.1 = phi i32 [ %instanceFlags.0748, %while.body ], [ %instanceFlags.2, %do.cond ]
  %instanceOffset.1 = phi i32 [ %instanceOffset.0749, %while.body ], [ %instanceOffset.2, %do.cond ]
  %instanceId.1 = phi i32 [ %instanceId.0750, %while.body ], [ %instanceId.2, %do.cond ]
  %stackPointer.2 = phi i32 [ %stackPointer.1751, %while.body ], [ %stackPointer.3, %do.cond ]
  %resultBary.1.i0 = phi float [ %resultBary.0753.i0, %while.body ], [ %resultBary.3.i0, %do.cond ]
  %resultBary.1.i1 = phi float [ %resultBary.0753.i1, %while.body ], [ %resultBary.3.i1, %do.cond ]
  %resultTriId.1 = phi i32 [ %resultTriId.0754, %while.body ], [ %resultTriId.3, %do.cond ]
  %dec.i = add nsw i32 %stackPointer.2, -1
  %mul.i.222 = shl i32 %dec.i, 6
  %add.i.223 = add i32 %mul.i.222, %call
  %arrayidx.i.224 = getelementptr [2048 x i32], [2048 x i32] addrspace(3)* @"\01?stack@@3PAIA", i32 0, i32 %add.i.223
  %66 = load i32, i32 addrspace(3)* %arrayidx.i.224, align 4, !tbaa !223, !noalias !312
  %arrayidx13 = getelementptr inbounds [2 x i32], [2 x i32]* %nodesToProcess, i32 0, i32 %currentBVHIndex.1
  %dec = add i32 %65, -1
  store i32 %dec, i32* %arrayidx13, align 4, !tbaa !223
  %arrayidx.i.i.230 = getelementptr inbounds [0 x %struct.RWByteAddressBuffer], [0 x %struct.RWByteAddressBuffer]* @"\01?DescriptorHeapBufferTable@@3PAURWByteAddressBuffer@@A", i32 0, i32 %currentGpuVA.1.i1, !dx.nonuniform !233
  %67 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %arrayidx.i.i.230, align 4, !noalias !315
  %add.i.i.242 = add i32 %currentGpuVA.1.i0, 16
  %mul.i.i.243 = shl i32 %66, 5
  %add.i.5.i.244 = add i32 %add.i.i.242, %mul.i.i.243
  %68 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %67)
  %69 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %68, i32 %add.i.5.i.244, i32 undef)
  %70 = extractvalue %dx.types.ResRet.i32 %69, 3
  %add.i.247 = add i32 %add.i.5.i.244, 16
  %71 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %68, i32 %add.i.247, i32 undef)
  %72 = extractvalue %dx.types.ResRet.i32 %71, 3
  %tobool.i = icmp slt i32 %70, 0
  br i1 %tobool.i, label %if.then.17, label %if.else.136

if.then.17:                                       ; preds = %do.body
  %cmp18 = icmp eq i32 %currentBVHIndex.1, 0
  br i1 %cmp18, label %if.then.21, label %if.else

if.then.21:                                       ; preds = %if.then.17
  %and.i.255 = and i32 %70, 1073741823
  %mul = mul i32 %and.i.255, 116
  %add23 = add i32 %add5, %mul
  %73 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %25)
  %74 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %73, i32 %add23, i32 undef)
  %75 = extractvalue %dx.types.ResRet.i32 %74, 0
  %76 = extractvalue %dx.types.ResRet.i32 %74, 1
  %77 = extractvalue %dx.types.ResRet.i32 %74, 2
  %78 = extractvalue %dx.types.ResRet.i32 %74, 3
  %add.i.260 = add i32 %add23, 16
  %79 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %73, i32 %add.i.260, i32 undef)
  %80 = extractvalue %dx.types.ResRet.i32 %79, 0
  %81 = extractvalue %dx.types.ResRet.i32 %79, 1
  %82 = extractvalue %dx.types.ResRet.i32 %79, 2
  %83 = extractvalue %dx.types.ResRet.i32 %79, 3
  %add.i.260.1 = add i32 %add23, 32
  %84 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %73, i32 %add.i.260.1, i32 undef)
  %85 = extractvalue %dx.types.ResRet.i32 %84, 0
  %86 = extractvalue %dx.types.ResRet.i32 %84, 1
  %87 = extractvalue %dx.types.ResRet.i32 %84, 2
  %88 = extractvalue %dx.types.ResRet.i32 %84, 3
  %add.i.260.2 = add i32 %add23, 48
  %89 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %73, i32 %add.i.260.2, i32 undef)
  %90 = extractvalue %dx.types.ResRet.i32 %89, 0
  %91 = extractvalue %dx.types.ResRet.i32 %89, 1
  %92 = extractvalue %dx.types.ResRet.i32 %89, 2
  %93 = extractvalue %dx.types.ResRet.i32 %89, 3
  %add.i.260.3 = add i32 %add23, 64
  %94 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %73, i32 %add.i.260.3, i32 undef)
  %add.i.260.4 = add i32 %add23, 80
  %95 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %73, i32 %add.i.260.4, i32 undef)
  %add.i.260.5 = add i32 %add23, 96
  %96 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %73, i32 %add.i.260.5, i32 undef)
  %add.i.260.6 = add i32 %add23, 112
  %97 = bitcast i32 %75 to float
  %98 = bitcast i32 %76 to float
  %99 = bitcast i32 %77 to float
  %100 = bitcast i32 %78 to float
  %101 = bitcast i32 %80 to float
  %102 = bitcast i32 %81 to float
  %103 = bitcast i32 %82 to float
  %104 = bitcast i32 %83 to float
  %105 = bitcast i32 %85 to float
  %106 = bitcast i32 %86 to float
  %107 = bitcast i32 %87 to float
  %108 = bitcast i32 %88 to float
  %109 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %25)
  %110 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %109, i32 %add.i.260.6, i32 undef)
  %111 = extractvalue %dx.types.ResRet.i32 %110, 0
  %and.i.263 = and i32 %91, 16777215
  %and.i.264 = and i32 %90, 16777215
  %shr.i = lshr i32 %90, 24
  %and = and i32 %shr.i, %0
  %tobool29 = icmp eq i32 %and, 0
  br i1 %tobool29, label %do.cond, label %if.then.31

if.then.31:                                       ; preds = %if.then.21
  %112 = extractvalue %dx.types.ResRet.i32 %96, 3
  %113 = bitcast i32 %112 to float
  %114 = extractvalue %dx.types.ResRet.i32 %96, 2
  %115 = bitcast i32 %114 to float
  %116 = extractvalue %dx.types.ResRet.i32 %96, 1
  %117 = bitcast i32 %116 to float
  %118 = extractvalue %dx.types.ResRet.i32 %96, 0
  %119 = bitcast i32 %118 to float
  %120 = extractvalue %dx.types.ResRet.i32 %95, 3
  %121 = bitcast i32 %120 to float
  %122 = extractvalue %dx.types.ResRet.i32 %95, 2
  %123 = bitcast i32 %122 to float
  %124 = extractvalue %dx.types.ResRet.i32 %95, 1
  %125 = bitcast i32 %124 to float
  %126 = extractvalue %dx.types.ResRet.i32 %95, 0
  %127 = bitcast i32 %126 to float
  %128 = extractvalue %dx.types.ResRet.i32 %94, 3
  %129 = bitcast i32 %128 to float
  %130 = extractvalue %dx.types.ResRet.i32 %94, 2
  %131 = bitcast i32 %130 to float
  %132 = extractvalue %dx.types.ResRet.i32 %94, 1
  %133 = bitcast i32 %132 to float
  %134 = extractvalue %dx.types.ResRet.i32 %94, 0
  %135 = bitcast i32 %134 to float
  store i32 0, i32 addrspace(3)* %arrayidx.i.224, align 4, !tbaa !223, !noalias !320
  %shr.i.275 = lshr i32 %91, 24
  %136 = fmul fast float %WorldRayDirection, %97
  %FMad857 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayDirection823, float %98, float %136)
  %FMad856 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayDirection824, float %99, float %FMad857)
  %FMad855 = call float @dx.op.tertiary.f32(i32 46, float 0.000000e+00, float %100, float %FMad856)
  %137 = fmul fast float %WorldRayDirection, %101
  %FMad854 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayDirection823, float %102, float %137)
  %FMad853 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayDirection824, float %103, float %FMad854)
  %FMad852 = call float @dx.op.tertiary.f32(i32 46, float 0.000000e+00, float %104, float %FMad853)
  %138 = fmul fast float %WorldRayDirection, %105
  %FMad851 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayDirection823, float %106, float %138)
  %FMad850 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayDirection824, float %107, float %FMad851)
  %FMad = call float @dx.op.tertiary.f32(i32 46, float 0.000000e+00, float %108, float %FMad850)
  %139 = fmul fast float %WorldRayOrigin, %97
  %FMad866 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayOrigin815, float %98, float %139)
  %FMad865 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayOrigin816, float %99, float %FMad866)
  %FMad864 = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %100, float %FMad865)
  %140 = insertelement <3 x float> undef, float %FMad864, i64 0
  %141 = fmul fast float %WorldRayOrigin, %101
  %FMad863 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayOrigin815, float %102, float %141)
  %FMad862 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayOrigin816, float %103, float %FMad863)
  %FMad861 = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %104, float %FMad862)
  %142 = insertelement <3 x float> %140, float %FMad861, i64 1
  %143 = fmul fast float %WorldRayOrigin, %105
  %FMad860 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayOrigin815, float %106, float %143)
  %FMad859 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayOrigin816, float %107, float %FMad860)
  %FMad858 = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %108, float %FMad859)
  %144 = insertelement <3 x float> %142, float %FMad858, i64 2
  store float %FMad855, float* %49, align 4
  store float %FMad852, float* %50, align 4
  store float %FMad, float* %51, align 4
  %145 = insertelement <3 x float> undef, float %FMad855, i64 0
  %146 = insertelement <3 x float> %145, float %FMad852, i64 1
  %147 = insertelement <3 x float> %146, float %FMad, i64 2
  %.i0893 = fdiv fast float 1.000000e+00, %FMad855
  %.i1894 = fdiv fast float 1.000000e+00, %FMad852
  %.i2895 = fdiv fast float 1.000000e+00, %FMad
  %mul.i.292.i0 = fmul fast float %.i0893, %FMad864
  %mul.i.292.i1 = fmul fast float %.i1894, %FMad861
  %mul.i.292.i2 = fmul fast float %.i2895, %FMad858
  %FAbs803 = call float @dx.op.unary.f32(i32 6, float %FMad855)
  %FAbs804 = call float @dx.op.unary.f32(i32 6, float %FMad852)
  %FAbs805 = call float @dx.op.unary.f32(i32 6, float %FMad)
  %cmp.i.i.294 = fcmp fast ogt float %FAbs803, %FAbs804
  %cmp1.i.i.295 = fcmp fast ogt float %FAbs803, %FAbs805
  %148 = and i1 %cmp.i.i.294, %cmp1.i.i.295
  br i1 %148, label %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.308", label %if.else.i.i.298

if.else.i.i.298:                                  ; preds = %if.then.31
  %cmp4.i.i.297 = fcmp fast ogt float %FAbs804, %FAbs805
  br i1 %cmp4.i.i.297, label %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.308", label %if.else.8.i.i.300

if.else.8.i.i.300:                                ; preds = %if.else.i.i.298
  br label %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.308"

"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.308": ; preds = %if.else.8.i.i.300, %if.else.i.i.298, %if.then.31
  %retval.i.i.283.0 = phi i32 [ 2, %if.else.8.i.i.300 ], [ 0, %if.then.31 ], [ 1, %if.else.i.i.298 ]
  %add.i.301 = add nuw nsw i32 %retval.i.i.283.0, 1
  %rem.i.302 = urem i32 %add.i.301, 3
  %add4.i.303 = add nuw nsw i32 %retval.i.i.283.0, 2
  %rem5.i.304 = urem i32 %add4.i.303, 3
  %149 = getelementptr [3 x float], [3 x float]* %10, i32 0, i32 %retval.i.i.283.0
  %150 = load float, float* %149, align 4, !tbaa !228, !noalias !323
  %cmp.i.307 = fcmp fast olt float %150, 0.000000e+00
  %tmp.5.0.i0 = select i1 %cmp.i.307, i32 %rem5.i.304, i32 %rem.i.302
  %tmp.5.0.i1 = select i1 %cmp.i.307, i32 %rem.i.302, i32 %rem5.i.304
  %151 = getelementptr [3 x float], [3 x float]* %10, i32 0, i32 %tmp.5.0.i0
  %152 = load float, float* %151, align 4, !tbaa !228, !noalias !323
  %div.i.314 = fdiv float %152, %150
  %153 = getelementptr [3 x float], [3 x float]* %10, i32 0, i32 %tmp.5.0.i1
  %154 = load float, float* %153, align 4, !tbaa !228, !noalias !323
  %div14.i.317 = fdiv float %154, %150
  %div16.i.319 = fdiv float 1.000000e+00, %150
  call void @"\01?Fallback_SetObjectRayOrigin@@YAXV?$vector@M$02@@@Z"(<3 x float> %144) #2
  call void @"\01?Fallback_SetObjectRayDirection@@YAXV?$vector@M$02@@@Z"(<3 x float> %147) #2
  %.0.vec.extract5.upto0 = insertelement <4 x float> undef, float %97, i32 0
  %.0.vec.extract5.upto1 = insertelement <4 x float> %.0.vec.extract5.upto0, float %98, i32 1
  %.0.vec.extract5.upto2 = insertelement <4 x float> %.0.vec.extract5.upto1, float %99, i32 2
  %.0.vec.extract5 = insertelement <4 x float> %.0.vec.extract5.upto2, float %100, i32 3
  %.fca.0.0.insert1 = insertvalue %class.matrix.float.3.4 undef, <4 x float> %.0.vec.extract5, 0, 0
  %.16.vec.extract7.upto0 = insertelement <4 x float> undef, float %101, i32 0
  %.16.vec.extract7.upto1 = insertelement <4 x float> %.16.vec.extract7.upto0, float %102, i32 1
  %.16.vec.extract7.upto2 = insertelement <4 x float> %.16.vec.extract7.upto1, float %103, i32 2
  %.16.vec.extract7 = insertelement <4 x float> %.16.vec.extract7.upto2, float %104, i32 3
  %.fca.0.1.insert2 = insertvalue %class.matrix.float.3.4 %.fca.0.0.insert1, <4 x float> %.16.vec.extract7, 0, 1
  %.32.vec.extract9.upto0 = insertelement <4 x float> undef, float %105, i32 0
  %.32.vec.extract9.upto1 = insertelement <4 x float> %.32.vec.extract9.upto0, float %106, i32 1
  %.32.vec.extract9.upto2 = insertelement <4 x float> %.32.vec.extract9.upto1, float %107, i32 2
  %.32.vec.extract9 = insertelement <4 x float> %.32.vec.extract9.upto2, float %108, i32 3
  %.fca.0.2.insert3 = insertvalue %class.matrix.float.3.4 %.fca.0.1.insert2, <4 x float> %.32.vec.extract9, 0, 2
  call void @"\01?Fallback_SetWorldToObject@@YAXV?$matrix@M$02$03@@@Z"(%class.matrix.float.3.4 %.fca.0.2.insert3) #2
  %.0.vec.extract.upto0 = insertelement <4 x float> undef, float %135, i32 0
  %.0.vec.extract.upto1 = insertelement <4 x float> %.0.vec.extract.upto0, float %133, i32 1
  %.0.vec.extract.upto2 = insertelement <4 x float> %.0.vec.extract.upto1, float %131, i32 2
  %.0.vec.extract = insertelement <4 x float> %.0.vec.extract.upto2, float %129, i32 3
  %.fca.0.0.insert = insertvalue %class.matrix.float.3.4 undef, <4 x float> %.0.vec.extract, 0, 0
  %.16.vec.extract.upto0 = insertelement <4 x float> undef, float %127, i32 0
  %.16.vec.extract.upto1 = insertelement <4 x float> %.16.vec.extract.upto0, float %125, i32 1
  %.16.vec.extract.upto2 = insertelement <4 x float> %.16.vec.extract.upto1, float %123, i32 2
  %.16.vec.extract = insertelement <4 x float> %.16.vec.extract.upto2, float %121, i32 3
  %.fca.0.1.insert = insertvalue %class.matrix.float.3.4 %.fca.0.0.insert, <4 x float> %.16.vec.extract, 0, 1
  %.32.vec.extract.upto0 = insertelement <4 x float> undef, float %119, i32 0
  %.32.vec.extract.upto1 = insertelement <4 x float> %.32.vec.extract.upto0, float %117, i32 1
  %.32.vec.extract.upto2 = insertelement <4 x float> %.32.vec.extract.upto1, float %115, i32 2
  %.32.vec.extract = insertelement <4 x float> %.32.vec.extract.upto2, float %113, i32 3
  %.fca.0.2.insert = insertvalue %class.matrix.float.3.4 %.fca.0.1.insert, <4 x float> %.32.vec.extract, 0, 2
  call void @"\01?Fallback_SetObjectToWorld@@YAXV?$matrix@M$02$03@@@Z"(%class.matrix.float.3.4 %.fca.0.2.insert) #2
  store i32 1, i32* %arrayidx42, align 4, !tbaa !223
  br label %do.cond

if.else:                                          ; preds = %if.then.17
  %RayFlags785 = call i32 @dx.op.rayFlags.i32(i32 144)
  %155 = and i32 %instanceFlags.1, 12
  %156 = icmp eq i32 %155, 8
  %. = select i1 %156, i32 0, i32 1
  %and7.i = and i32 %RayFlags785, 1
  %tobool8.i = icmp eq i32 %and7.i, 0
  br i1 %tobool8.i, label %if.else.10.i, label %"\01?IsOpaque@@YA_N_NII@Z.exit"

if.else.10.i:                                     ; preds = %if.else
  %and11.i = and i32 %RayFlags785, 2
  %tobool12.i = icmp eq i32 %and11.i, 0
  %.. = select i1 %tobool12.i, i32 %., i32 0
  br label %"\01?IsOpaque@@YA_N_NII@Z.exit"

"\01?IsOpaque@@YA_N_NII@Z.exit":                  ; preds = %if.else.10.i, %if.else
  %opaque.i.1 = phi i32 [ 1, %if.else ], [ %.., %if.else.10.i ]
  %tobool16.i = icmp ne i32 %opaque.i.1, 0
  %and.i.328 = and i32 %RayFlags785, 64
  %tobool1.i = icmp ne i32 %and.i.328, 0
  %157 = and i1 %tobool16.i, %tobool1.i
  %lnot.i = xor i1 %tobool16.i, true
  %and3.i.330 = and i32 %RayFlags785, 128
  %tobool4.i.331 = icmp ne i32 %and3.i.330, 0
  %158 = and i1 %tobool4.i.331, %lnot.i
  %159 = or i1 %157, %158
  %call56 = call float @"\01?Fallback_RayTCurrent@@YAMXZ"() #2
  %and.i.333 = and i32 %70, 1073741824
  %tobool.i.334 = icmp ne i32 %and.i.333, 0
  %160 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %arrayidx.i.i.230, align 4, !noalias !326
  %lnot = xor i1 %159, true
  %161 = and i1 %tobool.i.334, %lnot
  br i1 %161, label %if.then.68, label %if.else.83

if.then.68:                                       ; preds = %"\01?IsOpaque@@YA_N_NII@Z.exit"
  %add.i.i.348 = add i32 %currentGpuVA.1.i0, 8
  %162 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %160)
  %163 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %162, i32 %add.i.i.348, i32 undef)
  %164 = extractvalue %dx.types.ResRet.i32 %163, 0
  %mul.i.i.349 = shl i32 %70, 3
  %add2.i.i = add i32 %mul.i.i.349, %currentGpuVA.1.i0
  %add.i.3.i = add i32 %add2.i.i, %164
  %165 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %162, i32 %add.i.3.i, i32 undef)
  %166 = extractvalue %dx.types.ResRet.i32 %165, 0
  %167 = extractvalue %dx.types.ResRet.i32 %165, 1
  %168 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %Constants, i32 0)
  %169 = extractvalue %dx.types.CBufRet.i32 %168, 2
  %mul74 = mul i32 %166, %2
  %add75 = add i32 %instanceOffset.1, %1
  %add76 = add i32 %add75, %mul74
  %mul77 = mul i32 %add76, %169
  call void @"\01?Fallback_SetPendingCustomVals@@YAXIIII@Z"(i32 %mul77, i32 %167, i32 %instanceIndex.1, i32 %instanceId.1) #2
  %add.i.351 = add i32 %mul77, 4
  %170 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.ByteAddressBuffer(i32 160, %struct.ByteAddressBuffer %3)
  %171 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %170, i32 %add.i.351, i32 undef)
  %172 = extractvalue %dx.types.ResRet.i32 %171, 0
  %173 = extractvalue %dx.types.ResRet.i32 %171, 1
  call void @"\01?Fallback_SetAnyHitStateId@@YAXH@Z"(i32 %172) #2
  call void @"\01?Fallback_SetAnyHitResult@@YAXH@Z"(i32 1) #2
  call void @"\01?Fallback_CallIndirect@@YAXH@Z"(i32 %173) #2
  %call80 = call i32 @"\01?Fallback_AnyHitResult@@YAHXZ"() #2
  %cmp81 = icmp eq i32 %call80, -1
  br i1 %cmp81, label %if.then.i.355, label %if.else.i.357

if.then.i.355:                                    ; preds = %if.then.68
  %or.i = or i32 %flagContainer.1, 1
  br label %if.end.129

if.else.i.357:                                    ; preds = %if.then.68
  %and.i.356 = and i32 %flagContainer.1, -2
  br label %if.end.129

if.else.83:                                       ; preds = %"\01?IsOpaque@@YA_N_NII@Z.exit"
  %and.i.362 = and i32 %70, 16777215
  %add.i.i.i = add i32 %currentGpuVA.1.i0, 4
  %174 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %67)
  %175 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %174, i32 %add.i.i.i, i32 undef)
  %176 = extractvalue %dx.types.ResRet.i32 %175, 0
  %mul.i.i.364 = mul nuw nsw i32 %and.i.362, 40
  %add2.i.i.i = add i32 %mul.i.i.364, %currentGpuVA.1.i0
  %add.i.i.365 = add i32 %add2.i.i.i, %176
  %add1.i.i = add i32 %add.i.i.365, 4
  %177 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %174, i32 %add1.i.i, i32 undef)
  %178 = extractvalue %dx.types.ResRet.i32 %177, 0
  %179 = extractvalue %dx.types.ResRet.i32 %177, 1
  %180 = extractvalue %dx.types.ResRet.i32 %177, 2
  %181 = extractvalue %dx.types.ResRet.i32 %177, 3
  %.i0896 = bitcast i32 %178 to float
  %.i1897 = bitcast i32 %179 to float
  %.i2898 = bitcast i32 %180 to float
  %.i3899 = bitcast i32 %181 to float
  %add5.i.i = add i32 %add.i.i.365, 20
  %182 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %174, i32 %add5.i.i, i32 undef)
  %183 = extractvalue %dx.types.ResRet.i32 %182, 0
  %184 = extractvalue %dx.types.ResRet.i32 %182, 1
  %185 = extractvalue %dx.types.ResRet.i32 %182, 2
  %186 = extractvalue %dx.types.ResRet.i32 %182, 3
  %.i0900 = bitcast i32 %183 to float
  %.i1901 = bitcast i32 %184 to float
  %.i2902 = bitcast i32 %185 to float
  %.i3903 = bitcast i32 %186 to float
  %add9.i.i = add i32 %add.i.i.365, 36
  %187 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %174, i32 %add9.i.i, i32 undef)
  %188 = extractvalue %dx.types.ResRet.i32 %187, 0
  %189 = bitcast i32 %188 to float
  %and.i.i = and i32 %instanceFlags.1, 1
  %lnot.i.i = icmp eq i32 %and.i.i, 0
  %and1.i.i = shl i32 %instanceFlags.1, 3
  %190 = and i32 %and1.i.i, 16
  %191 = add nuw nsw i32 %190, 16
  %192 = xor i32 %190, 16
  %193 = add nuw nsw i32 %192, 16
  %and8.i.i = and i32 %RayFlags785, %191
  %tobool9.i.i = icmp ne i32 %and8.i.i, 0
  %194 = and i1 %lnot.i.i, %tobool9.i.i
  %and13.i.i = and i32 %RayFlags785, %193
  %tobool14.i.i = icmp ne i32 %and13.i.i, 0
  %195 = and i1 %lnot.i.i, %tobool14.i.i
  %sub.i.i.i0 = fsub fast float %.i0896, %currentRayData.0.1.i0
  %sub.i.i.i1 = fsub fast float %.i1897, %currentRayData.0.1.i1
  %sub.i.i.i2 = fsub fast float %.i2898, %currentRayData.0.1.i2
  store float %sub.i.i.i0, float* %52, align 4
  store float %sub.i.i.i1, float* %53, align 4
  store float %sub.i.i.i2, float* %54, align 4
  %196 = getelementptr [3 x float], [3 x float]* %9, i32 0, i32 %currentRayData.5.2.i0
  %197 = load float, float* %196, align 4, !tbaa !228, !noalias !331
  %198 = getelementptr [3 x float], [3 x float]* %9, i32 0, i32 %currentRayData.5.2.i1
  %199 = load float, float* %198, align 4, !tbaa !228, !noalias !331
  %200 = getelementptr [3 x float], [3 x float]* %9, i32 0, i32 %currentRayData.5.2.i2
  %201 = load float, float* %200, align 4, !tbaa !228, !noalias !331
  %sub17.i.i.i0 = fsub fast float %.i3899, %currentRayData.0.1.i0
  %sub17.i.i.i1 = fsub fast float %.i0900, %currentRayData.0.1.i1
  %sub17.i.i.i2 = fsub fast float %.i1901, %currentRayData.0.1.i2
  store float %sub17.i.i.i0, float* %55, align 4
  store float %sub17.i.i.i1, float* %56, align 4
  store float %sub17.i.i.i2, float* %57, align 4
  %202 = getelementptr [3 x float], [3 x float]* %7, i32 0, i32 %currentRayData.5.2.i0
  %203 = load float, float* %202, align 4, !tbaa !228, !noalias !331
  %204 = getelementptr [3 x float], [3 x float]* %7, i32 0, i32 %currentRayData.5.2.i1
  %205 = load float, float* %204, align 4, !tbaa !228, !noalias !331
  %206 = getelementptr [3 x float], [3 x float]* %7, i32 0, i32 %currentRayData.5.2.i2
  %207 = load float, float* %206, align 4, !tbaa !228, !noalias !331
  %sub19.i.i.i0 = fsub fast float %.i2902, %currentRayData.0.1.i0
  %sub19.i.i.i1 = fsub fast float %.i3903, %currentRayData.0.1.i1
  %sub19.i.i.i2 = fsub fast float %189, %currentRayData.0.1.i2
  store float %sub19.i.i.i0, float* %58, align 4
  store float %sub19.i.i.i1, float* %59, align 4
  store float %sub19.i.i.i2, float* %60, align 4
  %208 = getelementptr [3 x float], [3 x float]* %8, i32 0, i32 %currentRayData.5.2.i0
  %209 = load float, float* %208, align 4, !tbaa !228, !noalias !331
  %210 = getelementptr [3 x float], [3 x float]* %8, i32 0, i32 %currentRayData.5.2.i1
  %211 = load float, float* %210, align 4, !tbaa !228, !noalias !331
  %212 = getelementptr [3 x float], [3 x float]* %8, i32 0, i32 %currentRayData.5.2.i2
  %213 = load float, float* %212, align 4, !tbaa !228, !noalias !331
  %mul.i.5.i.i0 = fmul float %currentRayData.4.1.i0, %201
  %mul.i.5.i.i1 = fmul float %currentRayData.4.1.i1, %201
  %sub21.i.i.i0 = fsub float %197, %mul.i.5.i.i0
  %sub21.i.i.i1 = fsub float %199, %mul.i.5.i.i1
  %mul24.i.i.i0 = fmul float %currentRayData.4.1.i0, %207
  %mul24.i.i.i1 = fmul float %currentRayData.4.1.i1, %207
  %sub25.i.i.i0 = fsub float %203, %mul24.i.i.i0
  %sub25.i.i.i1 = fsub float %205, %mul24.i.i.i1
  %mul28.i.i.i0 = fmul float %currentRayData.4.1.i0, %213
  %mul28.i.i.i1 = fmul float %currentRayData.4.1.i1, %213
  %sub29.i.i.i0 = fsub float %209, %mul28.i.i.i0
  %sub29.i.i.i1 = fsub float %211, %mul28.i.i.i1
  %mul30.i.i = fmul float %sub25.i.i.i1, %sub29.i.i.i0
  %mul31.i.i = fmul float %sub25.i.i.i0, %sub29.i.i.i1
  %sub32.i.i = fsub float %mul30.i.i, %mul31.i.i
  %mul33.i.i = fmul float %sub21.i.i.i0, %sub29.i.i.i1
  %mul34.i.i = fmul float %sub21.i.i.i1, %sub29.i.i.i0
  %sub35.i.i = fsub float %mul33.i.i, %mul34.i.i
  %mul36.i.i = fmul float %sub21.i.i.i1, %sub25.i.i.i0
  %mul37.i.i = fmul float %sub21.i.i.i0, %sub25.i.i.i1
  %sub38.i.i = fsub float %mul36.i.i, %mul37.i.i
  %add.i.6.i = fadd fast float %sub35.i.i, %sub38.i.i
  %add39.i.i = fadd fast float %add.i.6.i, %sub32.i.i
  br i1 %195, label %if.then.i.i.368, label %if.else.i.i.369

if.then.i.i.368:                                  ; preds = %if.else.83
  %cmp.i.i.367 = fcmp fast ogt float %sub32.i.i, 0.000000e+00
  %cmp42.i.i = fcmp fast ogt float %sub35.i.i, 0.000000e+00
  %214 = or i1 %cmp.i.i.367, %cmp42.i.i
  %cmp44.i.i = fcmp fast ogt float %sub38.i.i, 0.000000e+00
  %215 = or i1 %cmp44.i.i, %214
  %cmp77.i.i.old.old = fcmp fast oeq float %add39.i.i, 0.000000e+00
  %or.cond757 = or i1 %215, %cmp77.i.i.old.old
  br i1 %or.cond757, label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i", label %if.end.81.i.i

if.else.i.i.369:                                  ; preds = %if.else.83
  br i1 %194, label %if.then.49.i.i, label %if.else.59.i.i

if.then.49.i.i:                                   ; preds = %if.else.i.i.369
  %cmp50.i.i = fcmp fast olt float %sub32.i.i, 0.000000e+00
  %cmp52.i.i = fcmp fast olt float %sub35.i.i, 0.000000e+00
  %216 = or i1 %cmp50.i.i, %cmp52.i.i
  %cmp54.i.i = fcmp fast olt float %sub38.i.i, 0.000000e+00
  %217 = or i1 %cmp54.i.i, %216
  %cmp77.i.i.old = fcmp fast oeq float %add39.i.i, 0.000000e+00
  %or.cond756 = or i1 %217, %cmp77.i.i.old
  br i1 %or.cond756, label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i", label %if.end.81.i.i

if.else.59.i.i:                                   ; preds = %if.else.i.i.369
  %cmp60.i.i = fcmp fast olt float %sub32.i.i, 0.000000e+00
  %cmp62.i.i = fcmp fast olt float %sub35.i.i, 0.000000e+00
  %218 = or i1 %cmp60.i.i, %cmp62.i.i
  %cmp64.i.i = fcmp fast olt float %sub38.i.i, 0.000000e+00
  %219 = or i1 %cmp64.i.i, %218
  %cmp66.i.i = fcmp fast ogt float %sub32.i.i, 0.000000e+00
  %cmp68.i.i = fcmp fast ogt float %sub35.i.i, 0.000000e+00
  %220 = or i1 %cmp66.i.i, %cmp68.i.i
  %cmp70.i.i = fcmp fast ogt float %sub38.i.i, 0.000000e+00
  %221 = or i1 %cmp70.i.i, %220
  %222 = and i1 %219, %221
  %cmp77.i.i = fcmp fast oeq float %add39.i.i, 0.000000e+00
  %or.cond755 = or i1 %cmp77.i.i, %222
  br i1 %or.cond755, label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i", label %if.end.81.i.i

if.end.81.i.i:                                    ; preds = %if.else.59.i.i, %if.then.49.i.i, %if.then.i.i.368
  %mul85.i.i = fmul fast float %sub32.i.i, %201
  %mul86.i.i = fmul fast float %sub35.i.i, %207
  %mul88.i.i = fmul fast float %sub38.i.i, %213
  %tmp = fadd fast float %mul86.i.i, %mul88.i.i
  %tmp961 = fadd fast float %tmp, %mul85.i.i
  %tmp962 = fmul fast float %tmp961, %currentRayData.4.1.i2
  br i1 %195, label %if.then.91.i.i, label %if.else.100.i.i

if.then.91.i.i:                                   ; preds = %if.end.81.i.i
  %cmp92.i.i = fcmp fast ogt float %tmp962, 0.000000e+00
  %mul94.i.i = fmul fast float %add39.i.i, %call56
  %cmp95.i.i = fcmp fast olt float %tmp962, %mul94.i.i
  %223 = or i1 %cmp92.i.i, %cmp95.i.i
  br i1 %223, label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i", label %if.end.133.i.i

if.else.100.i.i:                                  ; preds = %if.end.81.i.i
  br i1 %194, label %if.then.102.i.i, label %if.else.111.i.i

if.then.102.i.i:                                  ; preds = %if.else.100.i.i
  %cmp103.i.i = fcmp fast olt float %tmp962, 0.000000e+00
  %mul105.i.i = fmul fast float %add39.i.i, %call56
  %cmp106.i.i = fcmp fast ogt float %tmp962, %mul105.i.i
  %224 = or i1 %cmp103.i.i, %cmp106.i.i
  br i1 %224, label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i", label %if.end.133.i.i

if.else.111.i.i:                                  ; preds = %if.else.100.i.i
  %cmp112.i.i = fcmp fast ogt float %add39.i.i, 0.000000e+00
  %cond116.i.i = select i1 %cmp112.i.i, i32 1, i32 -1
  %225 = bitcast float %tmp962 to i32
  %xor121.i.i = xor i32 %225, %cond116.i.i
  %conv122.i.i = uitofp i32 %xor121.i.i to float
  %226 = bitcast float %add39.i.i to i32
  %xor124.i.i = xor i32 %cond116.i.i, %226
  %conv125.i.i = uitofp i32 %xor124.i.i to float
  %mul126.i.i = fmul fast float %conv125.i.i, %call56
  %cmp127.i.i = fcmp fast ogt float %conv122.i.i, %mul126.i.i
  br i1 %cmp127.i.i, label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i", label %if.end.133.i.i

if.end.133.i.i:                                   ; preds = %if.else.111.i.i, %if.then.102.i.i, %if.then.91.i.i
  %227 = fdiv fast float 1.000000e+00, %add39.i.i
  %mul135.i.i = fmul fast float %227, %sub35.i.i
  %mul136.i.i = fmul fast float %227, %sub38.i.i
  %mul137.i.i = fmul fast float %227, %tmp962
  br label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i"

"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i": ; preds = %if.end.133.i.i, %if.else.111.i.i, %if.then.102.i.i, %if.then.91.i.i, %if.else.59.i.i, %if.then.49.i.i, %if.then.i.i.368
  %.2.i0 = phi float [ %.1.i0, %if.then.i.i.368 ], [ %.1.i0, %if.then.91.i.i ], [ %mul135.i.i, %if.end.133.i.i ], [ %.1.i0, %if.then.102.i.i ], [ %.1.i0, %if.else.111.i.i ], [ %.1.i0, %if.then.49.i.i ], [ %.1.i0, %if.else.59.i.i ]
  %.2.i1 = phi float [ %.1.i1, %if.then.i.i.368 ], [ %.1.i1, %if.then.91.i.i ], [ %mul136.i.i, %if.end.133.i.i ], [ %.1.i1, %if.then.102.i.i ], [ %.1.i1, %if.else.111.i.i ], [ %.1.i1, %if.then.49.i.i ], [ %.1.i1, %if.else.59.i.i ]
  %.0732 = phi float [ %call56, %if.then.i.i.368 ], [ %call56, %if.then.91.i.i ], [ %mul137.i.i, %if.end.133.i.i ], [ %call56, %if.then.102.i.i ], [ %call56, %if.else.111.i.i ], [ %call56, %if.then.49.i.i ], [ %call56, %if.else.59.i.i ]
  %cmp.i.370 = fcmp fast olt float %.0732, %call56
  %RayTMin = call float @dx.op.rayTMin.f32(i32 153)
  %cmp1.i = fcmp fast ogt float %.0732, %RayTMin
  %228 = and i1 %cmp.i.370, %cmp1.i
  %.10 = select i1 %228, i1 true, i1 false
  %and.i.362.resultTriId.1 = select i1 %228, i32 %and.i.362, i32 %resultTriId.1
  %.0732.call56 = select i1 %228, float %.0732, float %call56
  %.2.i0.resultBary.1.i0 = select i1 %228, float %.2.i0, float %resultBary.1.i0
  %.2.i1.resultBary.1.i1 = select i1 %228, float %.2.i1, float %resultBary.1.i1
  %229 = and i1 %.10, %lnot
  br i1 %229, label %if.then.90, label %if.end.129

if.then.90:                                       ; preds = %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i"
  %.0727.upto0 = insertelement <2 x float> undef, float %.2.i0.resultBary.1.i0, i32 0
  %.0727 = insertelement <2 x float> %.0727.upto0, float %.2.i1.resultBary.1.i1, i32 1
  %add.i.i.380 = add i32 %currentGpuVA.1.i0, 8
  %230 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %160)
  %231 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %230, i32 %add.i.i.380, i32 undef)
  %232 = extractvalue %dx.types.ResRet.i32 %231, 0
  %mul.i.i.383 = shl i32 %and.i.362.resultTriId.1, 3
  %add2.i.i.382 = add i32 %mul.i.i.383, %currentGpuVA.1.i0
  %add.i.3.i.384 = add i32 %add2.i.i.382, %232
  %233 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %230, i32 %add.i.3.i.384, i32 undef)
  %234 = extractvalue %dx.types.ResRet.i32 %233, 0
  %235 = extractvalue %dx.types.ResRet.i32 %233, 1
  %236 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %Constants, i32 0)
  %237 = extractvalue %dx.types.CBufRet.i32 %236, 2
  %mul95 = mul i32 %234, %2
  %add96 = add i32 %instanceOffset.1, %1
  %add97 = add i32 %add96, %mul95
  %mul98 = mul i32 %add97, %237
  store <2 x float> %.0727, <2 x float>* %barycentrics, align 4, !tbaa !220
  call void @"\01?Fallback_SetPendingAttr@@YAXUBuiltInTriangleIntersectionAttributes@@@Z"(%struct.BuiltInTriangleIntersectionAttributes* nonnull %attr) #2
  call void @"\01?Fallback_SetPendingTriVals@@YAXIIIIMI@Z"(i32 %mul98, i32 %235, i32 %instanceIndex.1, i32 %instanceId.1, float %.0732.call56, i32 254) #2
  br i1 %tobool16.i, label %if.then.105, label %if.else.109

if.then.105:                                      ; preds = %if.then.90
  call void @"\01?Fallback_CommitHit@@YAXXZ"() #2
  %and107 = and i32 %RayFlags785, 4
  %tobool108 = icmp eq i32 %and107, 0
  br i1 %tobool108, label %if.else.i.396, label %if.then.i.393

if.then.i.393:                                    ; preds = %if.then.105
  %or.i.392 = or i32 %flagContainer.1, 1
  br label %if.end.129

if.else.i.396:                                    ; preds = %if.then.105
  %and.i.395 = and i32 %flagContainer.1, -2
  br label %if.end.129

if.else.109:                                      ; preds = %if.then.90
  %add.i.399 = add i32 %mul98, 4
  %238 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.ByteAddressBuffer(i32 160, %struct.ByteAddressBuffer %3)
  %239 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %238, i32 %add.i.399, i32 undef)
  %240 = extractvalue %dx.types.ResRet.i32 %239, 0
  %tobool113 = icmp eq i32 %240, 0
  br i1 %tobool113, label %if.end.116, label %if.then.114

if.then.114:                                      ; preds = %if.else.109
  call void @"\01?Fallback_SetAnyHitResult@@YAXH@Z"(i32 1) #2
  call void @"\01?Fallback_CallIndirect@@YAXH@Z"(i32 %240) #2
  %call.i = call i32 @"\01?Fallback_AnyHitResult@@YAHXZ"() #2
  br label %if.end.116

if.end.116:                                       ; preds = %if.then.114, %if.else.109
  %ret.0 = phi i32 [ %call.i, %if.then.114 ], [ 1, %if.else.109 ]
  %cmp117 = icmp eq i32 %ret.0, 0
  br i1 %cmp117, label %if.end.121, label %if.then.120

if.then.120:                                      ; preds = %if.end.116
  call void @"\01?Fallback_CommitHit@@YAXXZ"() #2
  br label %if.end.121

if.end.121:                                       ; preds = %if.then.120, %if.end.116
  %cmp122 = icmp eq i32 %ret.0, -1
  %and125 = and i32 %RayFlags785, 4
  %tobool126 = icmp ne i32 %and125, 0
  %241 = or i1 %cmp122, %tobool126
  br i1 %241, label %if.then.i.405, label %if.else.i.408

if.then.i.405:                                    ; preds = %if.end.121
  %or.i.404 = or i32 %flagContainer.1, 1
  br label %if.end.129

if.else.i.408:                                    ; preds = %if.end.121
  %and.i.407 = and i32 %flagContainer.1, -2
  br label %if.end.129

if.end.129:                                       ; preds = %if.else.i.408, %if.then.i.405, %if.else.i.396, %if.then.i.393, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i", %if.else.i.357, %if.then.i.355
  %.3.i0 = phi float [ %.2.i0, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i" ], [ %.1.i0, %if.then.i.355 ], [ %.1.i0, %if.else.i.357 ], [ %.2.i0, %if.then.i.393 ], [ %.2.i0, %if.else.i.396 ], [ %.2.i0, %if.then.i.405 ], [ %.2.i0, %if.else.i.408 ]
  %.3.i1 = phi float [ %.2.i1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i" ], [ %.1.i1, %if.then.i.355 ], [ %.1.i1, %if.else.i.357 ], [ %.2.i1, %if.then.i.393 ], [ %.2.i1, %if.else.i.396 ], [ %.2.i1, %if.then.i.405 ], [ %.2.i1, %if.else.i.408 ]
  %flagContainer.2 = phi i32 [ %flagContainer.1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i" ], [ %or.i, %if.then.i.355 ], [ %and.i.356, %if.else.i.357 ], [ %or.i.392, %if.then.i.393 ], [ %and.i.395, %if.else.i.396 ], [ %or.i.404, %if.then.i.405 ], [ %and.i.407, %if.else.i.408 ]
  %resultBary.2.i0 = phi float [ %.2.i0.resultBary.1.i0, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i" ], [ %resultBary.1.i0, %if.then.i.355 ], [ %resultBary.1.i0, %if.else.i.357 ], [ %.2.i0.resultBary.1.i0, %if.then.i.393 ], [ %.2.i0.resultBary.1.i0, %if.else.i.396 ], [ %.2.i0.resultBary.1.i0, %if.then.i.405 ], [ %.2.i0.resultBary.1.i0, %if.else.i.408 ]
  %resultBary.2.i1 = phi float [ %.2.i1.resultBary.1.i1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i" ], [ %resultBary.1.i1, %if.then.i.355 ], [ %resultBary.1.i1, %if.else.i.357 ], [ %.2.i1.resultBary.1.i1, %if.then.i.393 ], [ %.2.i1.resultBary.1.i1, %if.else.i.396 ], [ %.2.i1.resultBary.1.i1, %if.then.i.405 ], [ %.2.i1.resultBary.1.i1, %if.else.i.408 ]
  %resultTriId.2 = phi i32 [ %and.i.362.resultTriId.1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i" ], [ %resultTriId.1, %if.then.i.355 ], [ %resultTriId.1, %if.else.i.357 ], [ %and.i.362.resultTriId.1, %if.then.i.393 ], [ %and.i.362.resultTriId.1, %if.else.i.396 ], [ %and.i.362.resultTriId.1, %if.then.i.405 ], [ %and.i.362.resultTriId.1, %if.else.i.408 ]
  %and.i.411 = and i32 %flagContainer.2, 1
  %tobool.i.412 = icmp eq i32 %and.i.411, 0
  br i1 %tobool.i.412, label %do.cond, label %if.then.131

if.then.131:                                      ; preds = %if.end.129
  store i32 0, i32* %arrayidx42, align 4, !tbaa !223
  store i32 0, i32* %arrayidx, align 4, !tbaa !223
  br label %do.cond

if.else.136:                                      ; preds = %do.body
  %and.i.414 = and i32 %70, 16777215
  %RayTCurrent = call float @dx.op.rayTCurrent.f32(i32 154)
  %mul.i.i.426 = shl nuw nsw i32 %and.i.414, 5
  %add.i.5.i.427 = add i32 %add.i.i.242, %mul.i.i.426
  %242 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %67)
  %243 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %242, i32 %add.i.5.i.427, i32 undef)
  %244 = extractvalue %dx.types.ResRet.i32 %243, 0
  %245 = extractvalue %dx.types.ResRet.i32 %243, 1
  %246 = extractvalue %dx.types.ResRet.i32 %243, 2
  %add.i.430 = add i32 %add.i.5.i.427, 16
  %247 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %242, i32 %add.i.430, i32 undef)
  %248 = extractvalue %dx.types.ResRet.i32 %247, 0
  %249 = extractvalue %dx.types.ResRet.i32 %247, 1
  %250 = extractvalue %dx.types.ResRet.i32 %247, 2
  %251 = bitcast i32 %244 to float
  %252 = bitcast i32 %245 to float
  %253 = bitcast i32 %246 to float
  %254 = bitcast i32 %248 to float
  %255 = bitcast i32 %249 to float
  %256 = bitcast i32 %250 to float
  %mul.i.i.447 = shl i32 %72, 5
  %add.i.5.i.448 = add i32 %add.i.i.242, %mul.i.i.447
  %257 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %242, i32 %add.i.5.i.448, i32 undef)
  %258 = extractvalue %dx.types.ResRet.i32 %257, 0
  %259 = extractvalue %dx.types.ResRet.i32 %257, 1
  %260 = extractvalue %dx.types.ResRet.i32 %257, 2
  %add.i.451 = add i32 %add.i.5.i.448, 16
  %261 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %242, i32 %add.i.451, i32 undef)
  %262 = extractvalue %dx.types.ResRet.i32 %261, 0
  %263 = extractvalue %dx.types.ResRet.i32 %261, 1
  %264 = extractvalue %dx.types.ResRet.i32 %261, 2
  %265 = bitcast i32 %258 to float
  %266 = bitcast i32 %259 to float
  %267 = bitcast i32 %260 to float
  %268 = bitcast i32 %262 to float
  %269 = bitcast i32 %263 to float
  %270 = bitcast i32 %264 to float
  %mul.i.468.i0 = fmul fast float %251, %currentRayData.2.1.i0
  %mul.i.468.i1 = fmul fast float %252, %currentRayData.2.1.i1
  %mul.i.468.i2 = fmul fast float %253, %currentRayData.2.1.i2
  %sub.i.469.i0 = fsub fast float %mul.i.468.i0, %currentRayData.3.1.i0
  %sub.i.469.i1 = fsub fast float %mul.i.468.i1, %currentRayData.3.1.i1
  %sub.i.469.i2 = fsub fast float %mul.i.468.i2, %currentRayData.3.1.i2
  %FAbs800 = call float @dx.op.unary.f32(i32 6, float %currentRayData.2.1.i0)
  %FAbs801 = call float @dx.op.unary.f32(i32 6, float %currentRayData.2.1.i1)
  %FAbs802 = call float @dx.op.unary.f32(i32 6, float %currentRayData.2.1.i2)
  %mul1.i.470.i0 = fmul fast float %FAbs800, %254
  %mul1.i.470.i1 = fmul fast float %FAbs801, %255
  %mul1.i.470.i2 = fmul fast float %FAbs802, %256
  %add.i.471.i0 = fadd fast float %mul1.i.470.i0, %sub.i.469.i0
  %add.i.471.i1 = fadd fast float %mul1.i.470.i1, %sub.i.469.i1
  %add.i.471.i2 = fadd fast float %mul1.i.470.i2, %sub.i.469.i2
  %sub4.i.473.i0 = fsub fast float %sub.i.469.i0, %mul1.i.470.i0
  %sub4.i.473.i1 = fsub fast float %sub.i.469.i1, %mul1.i.470.i1
  %sub4.i.473.i2 = fsub fast float %sub.i.469.i2, %mul1.i.470.i2
  %FMax842 = call float @dx.op.binary.f32(i32 35, float %sub4.i.473.i0, float %sub4.i.473.i1)
  %FMax841 = call float @dx.op.binary.f32(i32 35, float %FMax842, float %sub4.i.473.i2)
  %FMin840 = call float @dx.op.binary.f32(i32 36, float %add.i.471.i0, float %add.i.471.i1)
  %FMin839 = call float @dx.op.binary.f32(i32 36, float %FMin840, float %add.i.471.i2)
  %FMax837 = call float @dx.op.binary.f32(i32 35, float %FMax841, float 0.000000e+00)
  %FMin836 = call float @dx.op.binary.f32(i32 36, float %FMin839, float %RayTCurrent)
  %cmp.i.474 = fcmp fast olt float %FMax837, %FMin836
  %mul.i.485.i0 = fmul fast float %265, %currentRayData.2.1.i0
  %mul.i.485.i1 = fmul fast float %266, %currentRayData.2.1.i1
  %mul.i.485.i2 = fmul fast float %267, %currentRayData.2.1.i2
  %sub.i.486.i0 = fsub fast float %mul.i.485.i0, %currentRayData.3.1.i0
  %sub.i.486.i1 = fsub fast float %mul.i.485.i1, %currentRayData.3.1.i1
  %sub.i.486.i2 = fsub fast float %mul.i.485.i2, %currentRayData.3.1.i2
  %mul1.i.487.i0 = fmul fast float %FAbs800, %268
  %mul1.i.487.i1 = fmul fast float %FAbs801, %269
  %mul1.i.487.i2 = fmul fast float %FAbs802, %270
  %add.i.488.i0 = fadd fast float %mul1.i.487.i0, %sub.i.486.i0
  %add.i.488.i1 = fadd fast float %mul1.i.487.i1, %sub.i.486.i1
  %add.i.488.i2 = fadd fast float %mul1.i.487.i2, %sub.i.486.i2
  %sub4.i.490.i0 = fsub fast float %sub.i.486.i0, %mul1.i.487.i0
  %sub4.i.490.i1 = fsub fast float %sub.i.486.i1, %mul1.i.487.i1
  %sub4.i.490.i2 = fsub fast float %sub.i.486.i2, %mul1.i.487.i2
  %FMax835 = call float @dx.op.binary.f32(i32 35, float %sub4.i.490.i0, float %sub4.i.490.i1)
  %FMax834 = call float @dx.op.binary.f32(i32 35, float %FMax835, float %sub4.i.490.i2)
  %FMin833 = call float @dx.op.binary.f32(i32 36, float %add.i.488.i0, float %add.i.488.i1)
  %FMin832 = call float @dx.op.binary.f32(i32 36, float %FMin833, float %add.i.488.i2)
  %FMax = call float @dx.op.binary.f32(i32 35, float %FMax834, float 0.000000e+00)
  %FMin = call float @dx.op.binary.f32(i32 36, float %FMin832, float %RayTCurrent)
  %cmp.i.491 = fcmp fast olt float %FMax, %FMin
  %271 = and i1 %cmp.i.474, %cmp.i.491
  br i1 %271, label %if.then.171, label %if.else.180

if.then.171:                                      ; preds = %if.else.136
  %cmp173 = fcmp fast olt float %FMax, %FMax837
  %cond.i = select i1 %cmp173, i32 %and.i.414, i32 %72
  %cond5.i = select i1 %cmp173, i32 %72, i32 %and.i.414
  %mul8.i = shl i32 %stackPointer.2, 6
  %add9.i = add i32 %mul8.i, %call
  store i32 %cond.i, i32 addrspace(3)* %arrayidx.i.224, align 4, !tbaa !223, !noalias !339
  %arrayidx10.i = getelementptr [2048 x i32], [2048 x i32] addrspace(3)* @"\01?stack@@3PAIA", i32 0, i32 %add9.i
  store i32 %cond5.i, i32 addrspace(3)* %arrayidx10.i, align 4, !tbaa !223, !noalias !339
  %add11.i = add nsw i32 %stackPointer.2, 1
  %add179 = add i32 %65, 1
  store i32 %add179, i32* %arrayidx13, align 4, !tbaa !223
  br label %do.cond

if.else.180:                                      ; preds = %if.else.136
  %272 = or i1 %cmp.i.474, %cmp.i.491
  br i1 %272, label %if.then.184, label %do.cond

if.then.184:                                      ; preds = %if.else.180
  %cond = select i1 %cmp.i.491, i32 %72, i32 %and.i.414
  store i32 %cond, i32 addrspace(3)* %arrayidx.i.224, align 4, !tbaa !223, !noalias !342
  store i32 %65, i32* %arrayidx13, align 4, !tbaa !223
  br label %do.cond

do.cond:                                          ; preds = %if.then.184, %if.else.180, %if.then.171, %if.then.131, %if.end.129, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.308", %if.then.21
  %.4.i0 = phi float [ %.1.i0, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.308" ], [ %.1.i0, %if.then.21 ], [ %.3.i0, %if.then.131 ], [ %.3.i0, %if.end.129 ], [ %.1.i0, %if.then.171 ], [ %.1.i0, %if.then.184 ], [ %.1.i0, %if.else.180 ]
  %.4.i1 = phi float [ %.1.i1, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.308" ], [ %.1.i1, %if.then.21 ], [ %.3.i1, %if.then.131 ], [ %.3.i1, %if.end.129 ], [ %.1.i1, %if.then.171 ], [ %.1.i1, %if.then.184 ], [ %.1.i1, %if.else.180 ]
  %currentRayData.0.2.i0 = phi float [ %FMad864, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.308" ], [ %currentRayData.0.1.i0, %if.then.21 ], [ %currentRayData.0.1.i0, %if.then.131 ], [ %currentRayData.0.1.i0, %if.end.129 ], [ %currentRayData.0.1.i0, %if.then.171 ], [ %currentRayData.0.1.i0, %if.then.184 ], [ %currentRayData.0.1.i0, %if.else.180 ]
  %currentRayData.0.2.i1 = phi float [ %FMad861, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.308" ], [ %currentRayData.0.1.i1, %if.then.21 ], [ %currentRayData.0.1.i1, %if.then.131 ], [ %currentRayData.0.1.i1, %if.end.129 ], [ %currentRayData.0.1.i1, %if.then.171 ], [ %currentRayData.0.1.i1, %if.then.184 ], [ %currentRayData.0.1.i1, %if.else.180 ]
  %currentRayData.0.2.i2 = phi float [ %FMad858, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.308" ], [ %currentRayData.0.1.i2, %if.then.21 ], [ %currentRayData.0.1.i2, %if.then.131 ], [ %currentRayData.0.1.i2, %if.end.129 ], [ %currentRayData.0.1.i2, %if.then.171 ], [ %currentRayData.0.1.i2, %if.then.184 ], [ %currentRayData.0.1.i2, %if.else.180 ]
  %currentRayData.2.2.i0 = phi float [ %.i0893, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.308" ], [ %currentRayData.2.1.i0, %if.then.21 ], [ %currentRayData.2.1.i0, %if.then.131 ], [ %currentRayData.2.1.i0, %if.end.129 ], [ %currentRayData.2.1.i0, %if.then.171 ], [ %currentRayData.2.1.i0, %if.then.184 ], [ %currentRayData.2.1.i0, %if.else.180 ]
  %currentRayData.2.2.i1 = phi float [ %.i1894, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.308" ], [ %currentRayData.2.1.i1, %if.then.21 ], [ %currentRayData.2.1.i1, %if.then.131 ], [ %currentRayData.2.1.i1, %if.end.129 ], [ %currentRayData.2.1.i1, %if.then.171 ], [ %currentRayData.2.1.i1, %if.then.184 ], [ %currentRayData.2.1.i1, %if.else.180 ]
  %currentRayData.2.2.i2 = phi float [ %.i2895, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.308" ], [ %currentRayData.2.1.i2, %if.then.21 ], [ %currentRayData.2.1.i2, %if.then.131 ], [ %currentRayData.2.1.i2, %if.end.129 ], [ %currentRayData.2.1.i2, %if.then.171 ], [ %currentRayData.2.1.i2, %if.then.184 ], [ %currentRayData.2.1.i2, %if.else.180 ]
  %currentRayData.3.2.i0 = phi float [ %mul.i.292.i0, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.308" ], [ %currentRayData.3.1.i0, %if.then.21 ], [ %currentRayData.3.1.i0, %if.then.131 ], [ %currentRayData.3.1.i0, %if.end.129 ], [ %currentRayData.3.1.i0, %if.then.171 ], [ %currentRayData.3.1.i0, %if.then.184 ], [ %currentRayData.3.1.i0, %if.else.180 ]
  %currentRayData.3.2.i1 = phi float [ %mul.i.292.i1, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.308" ], [ %currentRayData.3.1.i1, %if.then.21 ], [ %currentRayData.3.1.i1, %if.then.131 ], [ %currentRayData.3.1.i1, %if.end.129 ], [ %currentRayData.3.1.i1, %if.then.171 ], [ %currentRayData.3.1.i1, %if.then.184 ], [ %currentRayData.3.1.i1, %if.else.180 ]
  %currentRayData.3.2.i2 = phi float [ %mul.i.292.i2, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.308" ], [ %currentRayData.3.1.i2, %if.then.21 ], [ %currentRayData.3.1.i2, %if.then.131 ], [ %currentRayData.3.1.i2, %if.end.129 ], [ %currentRayData.3.1.i2, %if.then.171 ], [ %currentRayData.3.1.i2, %if.then.184 ], [ %currentRayData.3.1.i2, %if.else.180 ]
  %currentRayData.4.2.i0 = phi float [ %div.i.314, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.308" ], [ %currentRayData.4.1.i0, %if.then.21 ], [ %currentRayData.4.1.i0, %if.then.131 ], [ %currentRayData.4.1.i0, %if.end.129 ], [ %currentRayData.4.1.i0, %if.then.171 ], [ %currentRayData.4.1.i0, %if.then.184 ], [ %currentRayData.4.1.i0, %if.else.180 ]
  %currentRayData.4.2.i1 = phi float [ %div14.i.317, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.308" ], [ %currentRayData.4.1.i1, %if.then.21 ], [ %currentRayData.4.1.i1, %if.then.131 ], [ %currentRayData.4.1.i1, %if.end.129 ], [ %currentRayData.4.1.i1, %if.then.171 ], [ %currentRayData.4.1.i1, %if.then.184 ], [ %currentRayData.4.1.i1, %if.else.180 ]
  %currentRayData.4.2.i2 = phi float [ %div16.i.319, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.308" ], [ %currentRayData.4.1.i2, %if.then.21 ], [ %currentRayData.4.1.i2, %if.then.131 ], [ %currentRayData.4.1.i2, %if.end.129 ], [ %currentRayData.4.1.i2, %if.then.171 ], [ %currentRayData.4.1.i2, %if.then.184 ], [ %currentRayData.4.1.i2, %if.else.180 ]
  %currentRayData.5.3.i0 = phi i32 [ %tmp.5.0.i0, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.308" ], [ %currentRayData.5.2.i0, %if.then.21 ], [ %currentRayData.5.2.i0, %if.then.131 ], [ %currentRayData.5.2.i0, %if.end.129 ], [ %currentRayData.5.2.i0, %if.then.171 ], [ %currentRayData.5.2.i0, %if.then.184 ], [ %currentRayData.5.2.i0, %if.else.180 ]
  %currentRayData.5.3.i1 = phi i32 [ %tmp.5.0.i1, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.308" ], [ %currentRayData.5.2.i1, %if.then.21 ], [ %currentRayData.5.2.i1, %if.then.131 ], [ %currentRayData.5.2.i1, %if.end.129 ], [ %currentRayData.5.2.i1, %if.then.171 ], [ %currentRayData.5.2.i1, %if.then.184 ], [ %currentRayData.5.2.i1, %if.else.180 ]
  %currentRayData.5.3.i2 = phi i32 [ %retval.i.i.283.0, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.308" ], [ %currentRayData.5.2.i2, %if.then.21 ], [ %currentRayData.5.2.i2, %if.then.131 ], [ %currentRayData.5.2.i2, %if.end.129 ], [ %currentRayData.5.2.i2, %if.then.171 ], [ %currentRayData.5.2.i2, %if.then.184 ], [ %currentRayData.5.2.i2, %if.else.180 ]
  %flagContainer.3 = phi i32 [ %flagContainer.1, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.308" ], [ %flagContainer.1, %if.then.21 ], [ %flagContainer.2, %if.then.131 ], [ %flagContainer.2, %if.end.129 ], [ %flagContainer.1, %if.then.171 ], [ %flagContainer.1, %if.then.184 ], [ %flagContainer.1, %if.else.180 ]
  %currentBVHIndex.2 = phi i32 [ 1, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.308" ], [ 0, %if.then.21 ], [ %currentBVHIndex.1, %if.then.131 ], [ %currentBVHIndex.1, %if.end.129 ], [ %currentBVHIndex.1, %if.then.171 ], [ %currentBVHIndex.1, %if.then.184 ], [ %currentBVHIndex.1, %if.else.180 ]
  %currentGpuVA.2.i0 = phi i32 [ %92, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.308" ], [ %currentGpuVA.1.i0, %if.then.21 ], [ %currentGpuVA.1.i0, %if.then.131 ], [ %currentGpuVA.1.i0, %if.end.129 ], [ %currentGpuVA.1.i0, %if.then.171 ], [ %currentGpuVA.1.i0, %if.then.184 ], [ %currentGpuVA.1.i0, %if.else.180 ]
  %currentGpuVA.2.i1 = phi i32 [ %93, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.308" ], [ %currentGpuVA.1.i1, %if.then.21 ], [ %currentGpuVA.1.i1, %if.then.131 ], [ %currentGpuVA.1.i1, %if.end.129 ], [ %currentGpuVA.1.i1, %if.then.171 ], [ %currentGpuVA.1.i1, %if.then.184 ], [ %currentGpuVA.1.i1, %if.else.180 ]
  %instanceIndex.2 = phi i32 [ %111, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.308" ], [ %111, %if.then.21 ], [ %instanceIndex.1, %if.then.131 ], [ %instanceIndex.1, %if.end.129 ], [ %instanceIndex.1, %if.then.171 ], [ %instanceIndex.1, %if.then.184 ], [ %instanceIndex.1, %if.else.180 ]
  %instanceFlags.2 = phi i32 [ %shr.i.275, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.308" ], [ %instanceFlags.1, %if.then.21 ], [ %instanceFlags.1, %if.then.131 ], [ %instanceFlags.1, %if.end.129 ], [ %instanceFlags.1, %if.then.171 ], [ %instanceFlags.1, %if.then.184 ], [ %instanceFlags.1, %if.else.180 ]
  %instanceOffset.2 = phi i32 [ %and.i.263, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.308" ], [ %and.i.263, %if.then.21 ], [ %instanceOffset.1, %if.then.131 ], [ %instanceOffset.1, %if.end.129 ], [ %instanceOffset.1, %if.then.171 ], [ %instanceOffset.1, %if.then.184 ], [ %instanceOffset.1, %if.else.180 ]
  %instanceId.2 = phi i32 [ %and.i.264, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.308" ], [ %and.i.264, %if.then.21 ], [ %instanceId.1, %if.then.131 ], [ %instanceId.1, %if.end.129 ], [ %instanceId.1, %if.then.171 ], [ %instanceId.1, %if.then.184 ], [ %instanceId.1, %if.else.180 ]
  %stackPointer.3 = phi i32 [ %stackPointer.2, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.308" ], [ %dec.i, %if.then.21 ], [ %dec.i, %if.then.131 ], [ %dec.i, %if.end.129 ], [ %add11.i, %if.then.171 ], [ %stackPointer.2, %if.then.184 ], [ %dec.i, %if.else.180 ]
  %resultBary.3.i0 = phi float [ %resultBary.1.i0, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.308" ], [ %resultBary.1.i0, %if.then.21 ], [ %resultBary.2.i0, %if.then.131 ], [ %resultBary.2.i0, %if.end.129 ], [ %resultBary.1.i0, %if.then.171 ], [ %resultBary.1.i0, %if.then.184 ], [ %resultBary.1.i0, %if.else.180 ]
  %resultBary.3.i1 = phi float [ %resultBary.1.i1, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.308" ], [ %resultBary.1.i1, %if.then.21 ], [ %resultBary.2.i1, %if.then.131 ], [ %resultBary.2.i1, %if.end.129 ], [ %resultBary.1.i1, %if.then.171 ], [ %resultBary.1.i1, %if.then.184 ], [ %resultBary.1.i1, %if.else.180 ]
  %resultTriId.3 = phi i32 [ %resultTriId.1, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.308" ], [ %resultTriId.1, %if.then.21 ], [ %resultTriId.2, %if.then.131 ], [ %resultTriId.2, %if.end.129 ], [ %resultTriId.1, %if.then.171 ], [ %resultTriId.1, %if.then.184 ], [ %resultTriId.1, %if.else.180 ]
  %arrayidx192 = getelementptr inbounds [2 x i32], [2 x i32]* %nodesToProcess, i32 0, i32 %currentBVHIndex.2
  %273 = load i32, i32* %arrayidx192, align 4, !tbaa !223
  %cmp193 = icmp eq i32 %273, 0
  br i1 %cmp193, label %do.end, label %do.body

do.end:                                           ; preds = %do.cond
  %dec196 = add i32 %currentBVHIndex.2, -1
  store float %WorldRayDirection, float* %61, align 4
  store float %WorldRayDirection823, float* %62, align 4
  store float %WorldRayDirection824, float* %63, align 4
  %.mux. = select i1 %brmerge, i32 %.mux, i32 2
  %add.i.526 = add nuw nsw i32 %.mux., 1
  %rem.i.527 = urem i32 %add.i.526, 3
  %add4.i.528 = add nuw nsw i32 %.mux., 2
  %rem5.i.529 = urem i32 %add4.i.528, 3
  %274 = getelementptr [3 x float], [3 x float]* %6, i32 0, i32 %.mux.
  %275 = load float, float* %274, align 4, !tbaa !228, !noalias !345
  %cmp.i.532 = fcmp fast olt float %275, 0.000000e+00
  %tmp199.5.0.i0 = select i1 %cmp.i.532, i32 %rem5.i.529, i32 %rem.i.527
  %tmp199.5.0.i1 = select i1 %cmp.i.532, i32 %rem.i.527, i32 %rem5.i.529
  %276 = getelementptr [3 x float], [3 x float]* %6, i32 0, i32 %tmp199.5.0.i0
  %277 = load float, float* %276, align 4, !tbaa !228, !noalias !345
  %div.i.539 = fdiv float %277, %275
  %278 = getelementptr [3 x float], [3 x float]* %6, i32 0, i32 %tmp199.5.0.i1
  %279 = load float, float* %278, align 4, !tbaa !228, !noalias !345
  %div14.i.542 = fdiv float %279, %275
  %280 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %AccelerationStructureList, i32 0)
  %281 = extractvalue %dx.types.CBufRet.i32 %280, 0
  %282 = extractvalue %dx.types.CBufRet.i32 %280, 1
  %283 = load i32, i32* %arrayidx, align 4, !tbaa !223
  %cmp = icmp eq i32 %283, 0
  br i1 %cmp, label %while.end, label %while.body

while.end:                                        ; preds = %do.end, %if.end
  %call201 = call i32 @"\01?Fallback_InstanceIndex@@YAIXZ"() #2
  %cmp202 = icmp ne i32 %call201, -1
  ret i1 %cmp202
}

; Function Attrs: alwaysinline nounwind readnone
define <3 x float> @"\01?Swizzle@@YA?AV?$vector@M$02@@V1@V?$vector@H$02@@@Z"(<3 x float>, <3 x i32>) #4 {
entry:
  %2 = alloca [3 x float], align 4
  %3 = extractelement <3 x float> %0, i64 0
  %4 = getelementptr inbounds [3 x float], [3 x float]* %2, i32 0, i32 0
  store float %3, float* %4, align 4
  %5 = extractelement <3 x float> %0, i64 1
  %6 = getelementptr inbounds [3 x float], [3 x float]* %2, i32 0, i32 1
  store float %5, float* %6, align 4
  %7 = extractelement <3 x float> %0, i64 2
  %8 = getelementptr inbounds [3 x float], [3 x float]* %2, i32 0, i32 2
  store float %7, float* %8, align 4
  %9 = extractelement <3 x i32> %1, i32 0
  %10 = getelementptr [3 x float], [3 x float]* %2, i32 0, i32 %9
  %11 = load float, float* %10, align 4, !tbaa !228
  %12 = extractelement <3 x i32> %1, i32 1
  %13 = getelementptr [3 x float], [3 x float]* %2, i32 0, i32 %12
  %14 = load float, float* %13, align 4, !tbaa !228
  %15 = extractelement <3 x i32> %1, i32 2
  %16 = getelementptr [3 x float], [3 x float]* %2, i32 0, i32 %15
  %17 = load float, float* %16, align 4, !tbaa !228
  %18 = insertelement <3 x float> undef, float %11, i64 0
  %19 = insertelement <3 x float> %18, float %14, i64 1
  %20 = insertelement <3 x float> %19, float %17, i64 2
  ret <3 x float> %20
}

; Function Attrs: alwaysinline nounwind readonly
define <4 x float> @"\01?CalculateDiffuseLighting@@YA?AV?$vector@M$03@@V?$vector@M$02@@0@Z"(<3 x float>, <3 x float>) #6 {
entry:
  %hitPosition.i0 = extractelement <3 x float> %0, i32 0
  %hitPosition.i1 = extractelement <3 x float> %0, i32 1
  %hitPosition.i2 = extractelement <3 x float> %0, i32 2
  %2 = load %g_sceneCB, %g_sceneCB* @g_sceneCB, align 4
  %g_sceneCB = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.g_sceneCB(i32 160, %g_sceneCB %2)
  %3 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %g_sceneCB, i32 5)
  %4 = extractvalue %dx.types.CBufRet.f32 %3, 0
  %5 = extractvalue %dx.types.CBufRet.f32 %3, 1
  %6 = extractvalue %dx.types.CBufRet.f32 %3, 2
  %sub.i0 = fsub fast float %4, %hitPosition.i0
  %sub.i1 = fsub fast float %5, %hitPosition.i1
  %sub.i2 = fsub fast float %6, %hitPosition.i2
  %7 = fmul fast float %sub.i0, %sub.i0
  %8 = fmul fast float %sub.i1, %sub.i1
  %9 = fadd fast float %7, %8
  %10 = fmul fast float %sub.i2, %sub.i2
  %11 = fadd fast float %9, %10
  %Sqrt = call float @dx.op.unary.f32(i32 24, float %11)
  %.i0 = fdiv fast float %sub.i0, %Sqrt
  %.i1 = fdiv fast float %sub.i1, %Sqrt
  %.i2 = fdiv fast float %sub.i2, %Sqrt
  %12 = extractelement <3 x float> %1, i64 0
  %13 = extractelement <3 x float> %1, i64 1
  %14 = extractelement <3 x float> %1, i64 2
  %15 = call float @dx.op.dot3.f32(i32 55, float %.i0, float %.i1, float %.i2, float %12, float %13, float %14)
  %FMax = call float @dx.op.binary.f32(i32 35, float 0.000000e+00, float %15)
  %16 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %g_sceneCB, i32 7)
  %17 = extractvalue %dx.types.CBufRet.f32 %16, 0
  %18 = extractvalue %dx.types.CBufRet.f32 %16, 1
  %19 = extractvalue %dx.types.CBufRet.f32 %16, 2
  %20 = extractvalue %dx.types.CBufRet.f32 %16, 3
  %mul.i0 = fmul fast float %17, %FMax
  %mul.i1 = fmul fast float %18, %FMax
  %mul.i2 = fmul fast float %19, %FMax
  %mul.i3 = fmul fast float %20, %FMax
  %mul.upto0 = insertelement <4 x float> undef, float %mul.i0, i32 0
  %mul.upto1 = insertelement <4 x float> %mul.upto0, float %mul.i1, i32 1
  %mul.upto2 = insertelement <4 x float> %mul.upto1, float %mul.i2, i32 2
  %mul = insertelement <4 x float> %mul.upto2, float %mul.i3, i32 3
  ret <4 x float> %mul
}

; Function Attrs: alwaysinline nounwind
define void @"\01?NullPrimitive@@YA?AUPrimitive@@XZ"(%struct.Primitive* noalias nocapture sret) #5 {
entry:
  %PrimitiveType = getelementptr inbounds %struct.Primitive, %struct.Primitive* %0, i32 0, i32 0
  store i32 0, i32* %PrimitiveType, align 4, !tbaa !223
  %data0 = getelementptr inbounds %struct.Primitive, %struct.Primitive* %0, i32 0, i32 1
  store <4 x i32> zeroinitializer, <4 x i32>* %data0, align 4, !tbaa !220
  %data1 = getelementptr inbounds %struct.Primitive, %struct.Primitive* %0, i32 0, i32 2
  store <4 x i32> zeroinitializer, <4 x i32>* %data1, align 4, !tbaa !220
  %data2 = getelementptr inbounds %struct.Primitive, %struct.Primitive* %0, i32 0, i32 3
  store i32 0, i32* %data2, align 4, !tbaa !223
  ret void
}

; Function Attrs: alwaysinline nounwind
define void @"\01?swap@@YAXAIAM0@Z"(float* noalias nocapture dereferenceable(4), float* noalias nocapture dereferenceable(4)) #5 {
entry:
  %2 = load float, float* %0, align 4, !tbaa !228
  %3 = load float, float* %1, align 4, !tbaa !228
  store float %3, float* %0, align 4, !tbaa !228
  store float %2, float* %1, align 4, !tbaa !228
  ret void
}

; Function Attrs: alwaysinline nounwind readnone
define i32 @"\01?GetPrimitiveMetaDataAddress@@YAIII@Z"(i32, i32) #4 {
entry:
  %mul = shl i32 %1, 3
  %add = add i32 %mul, %0
  ret i32 %add
}

; Function Attrs: alwaysinline nounwind
define void @"\01?RawDataToAABB@@YA?AUAABB@@V?$vector@I$03@@V?$vector@I$01@@@Z"(%struct.AABB* noalias nocapture sret, <4 x i32>, <2 x i32>) #5 {
entry:
  %3 = extractelement <4 x i32> %1, i32 2
  %4 = extractelement <4 x i32> %1, i32 1
  %5 = extractelement <4 x i32> %1, i32 0
  %.i0 = bitcast i32 %5 to float
  %.i1 = bitcast i32 %4 to float
  %.i2 = bitcast i32 %3 to float
  %.upto0 = insertelement <3 x float> undef, float %.i0, i32 0
  %.upto1 = insertelement <3 x float> %.upto0, float %.i1, i32 1
  %6 = insertelement <3 x float> %.upto1, float %.i2, i32 2
  %min = getelementptr inbounds %struct.AABB, %struct.AABB* %0, i32 0, i32 0
  store <3 x float> %6, <3 x float>* %min, align 4, !tbaa !220
  %7 = extractelement <4 x i32> %1, i32 3
  %8 = extractelement <2 x i32> %2, i32 1
  %9 = extractelement <2 x i32> %2, i32 0
  %.i02 = bitcast i32 %7 to float
  %.i13 = bitcast i32 %9 to float
  %.i24 = bitcast i32 %8 to float
  %.upto07 = insertelement <3 x float> undef, float %.i02, i32 0
  %.upto18 = insertelement <3 x float> %.upto07, float %.i13, i32 1
  %10 = insertelement <3 x float> %.upto18, float %.i24, i32 2
  %max = getelementptr inbounds %struct.AABB, %struct.AABB* %0, i32 0, i32 1
  store <3 x float> %10, <3 x float>* %max, align 4, !tbaa !220
  ret void
}

; Function Attrs: alwaysinline nounwind
define void @"\01?StoreBVHMetadataToRawData@@YAXURWByteAddressBuffer@@IUBVHMetadata@@@Z"(%struct.RWByteAddressBuffer* nocapture readonly, i32, %struct.BVHMetadata* nocapture readonly) #5 {
entry:
  %arrayidx = getelementptr inbounds %struct.BVHMetadata, %struct.BVHMetadata* %2, i32 0, i32 0, i32 0, i32 0
  %3 = load <4 x float>, <4 x float>* %arrayidx, align 4, !tbaa !220
  %.i0 = extractelement <4 x float> %3, i32 0
  %.i1 = extractelement <4 x float> %3, i32 1
  %.i2 = extractelement <4 x float> %3, i32 2
  %.i3 = extractelement <4 x float> %3, i32 3
  %4 = bitcast float %.i0 to i32
  %5 = bitcast float %.i1 to i32
  %6 = bitcast float %.i2 to i32
  %7 = bitcast float %.i3 to i32
  %arrayidx4 = getelementptr inbounds %struct.BVHMetadata, %struct.BVHMetadata* %2, i32 0, i32 0, i32 0, i32 1
  %8 = load <4 x float>, <4 x float>* %arrayidx4, align 4, !tbaa !220
  %.i067 = extractelement <4 x float> %8, i32 0
  %.i169 = extractelement <4 x float> %8, i32 1
  %.i271 = extractelement <4 x float> %8, i32 2
  %.i373 = extractelement <4 x float> %8, i32 3
  %9 = bitcast float %.i067 to i32
  %10 = bitcast float %.i169 to i32
  %11 = bitcast float %.i271 to i32
  %12 = bitcast float %.i373 to i32
  %arrayidx9 = getelementptr inbounds %struct.BVHMetadata, %struct.BVHMetadata* %2, i32 0, i32 0, i32 0, i32 2
  %13 = load <4 x float>, <4 x float>* %arrayidx9, align 4, !tbaa !220
  %.i075 = extractelement <4 x float> %13, i32 0
  %.i177 = extractelement <4 x float> %13, i32 1
  %.i279 = extractelement <4 x float> %13, i32 2
  %.i381 = extractelement <4 x float> %13, i32 3
  %InstanceIDAndMask = getelementptr inbounds %struct.BVHMetadata, %struct.BVHMetadata* %2, i32 0, i32 0, i32 1
  %14 = load i32, i32* %InstanceIDAndMask, align 4, !tbaa !223
  %InstanceContributionToHitGroupIndexAndFlags = getelementptr inbounds %struct.BVHMetadata, %struct.BVHMetadata* %2, i32 0, i32 0, i32 2
  %15 = load i32, i32* %InstanceContributionToHitGroupIndexAndFlags, align 4, !tbaa !223
  %AccelerationStructure = getelementptr inbounds %struct.BVHMetadata, %struct.BVHMetadata* %2, i32 0, i32 0, i32 3
  %16 = load <2 x i32>, <2 x i32>* %AccelerationStructure, align 4, !tbaa !220
  %17 = extractelement <2 x i32> %16, i64 0
  %18 = extractelement <2 x i32> %16, i64 1
  %arrayidx18 = getelementptr inbounds %struct.BVHMetadata, %struct.BVHMetadata* %2, i32 0, i32 1, i32 0
  %19 = load <4 x float>, <4 x float>* %arrayidx18, align 4, !tbaa !220
  %.i083 = extractelement <4 x float> %19, i32 0
  %.i185 = extractelement <4 x float> %19, i32 1
  %.i287 = extractelement <4 x float> %19, i32 2
  %.i389 = extractelement <4 x float> %19, i32 3
  %arrayidx22 = getelementptr inbounds %struct.BVHMetadata, %struct.BVHMetadata* %2, i32 0, i32 1, i32 1
  %20 = load <4 x float>, <4 x float>* %arrayidx22, align 4, !tbaa !220
  %.i091 = extractelement <4 x float> %20, i32 0
  %.i193 = extractelement <4 x float> %20, i32 1
  %.i295 = extractelement <4 x float> %20, i32 2
  %.i397 = extractelement <4 x float> %20, i32 3
  %arrayidx26 = getelementptr inbounds %struct.BVHMetadata, %struct.BVHMetadata* %2, i32 0, i32 1, i32 2
  %21 = load <4 x float>, <4 x float>* %arrayidx26, align 4, !tbaa !220
  %.i099 = extractelement <4 x float> %21, i32 0
  %.i1101 = extractelement <4 x float> %21, i32 1
  %.i2103 = extractelement <4 x float> %21, i32 2
  %.i3105 = extractelement <4 x float> %21, i32 3
  %InstanceIndex = getelementptr inbounds %struct.BVHMetadata, %struct.BVHMetadata* %2, i32 0, i32 2
  %22 = load i32, i32* %InstanceIndex, align 4, !tbaa !223
  %23 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %0, align 4
  %24 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %23)
  call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %24, i32 %1, i32 undef, i32 %4, i32 %5, i32 %6, i32 %7, i8 15)
  %add = add i32 %1, 16
  %25 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %0, align 4
  %26 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %25)
  call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %26, i32 %add, i32 undef, i32 %9, i32 %10, i32 %11, i32 %12, i8 15)
  %add.1 = add i32 %1, 32
  %27 = bitcast float %.i075 to i32
  %28 = bitcast float %.i177 to i32
  %29 = bitcast float %.i279 to i32
  %30 = bitcast float %.i381 to i32
  %31 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %0, align 4
  %32 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %31)
  call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %32, i32 %add.1, i32 undef, i32 %27, i32 %28, i32 %29, i32 %30, i8 15)
  %add.2 = add i32 %1, 48
  %33 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %0, align 4
  %34 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %33)
  call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %34, i32 %add.2, i32 undef, i32 %14, i32 %15, i32 %17, i32 %18, i8 15)
  %add.3 = add i32 %1, 64
  %35 = bitcast float %.i083 to i32
  %36 = bitcast float %.i185 to i32
  %37 = bitcast float %.i287 to i32
  %38 = bitcast float %.i389 to i32
  %39 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %0, align 4
  %40 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %39)
  call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %40, i32 %add.3, i32 undef, i32 %35, i32 %36, i32 %37, i32 %38, i8 15)
  %add.4 = add i32 %1, 80
  %41 = bitcast float %.i091 to i32
  %42 = bitcast float %.i193 to i32
  %43 = bitcast float %.i295 to i32
  %44 = bitcast float %.i397 to i32
  %45 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %0, align 4
  %46 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %45)
  call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %46, i32 %add.4, i32 undef, i32 %41, i32 %42, i32 %43, i32 %44, i8 15)
  %add.5 = add i32 %1, 96
  %47 = bitcast float %.i099 to i32
  %48 = bitcast float %.i1101 to i32
  %49 = bitcast float %.i2103 to i32
  %50 = bitcast float %.i3105 to i32
  %51 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %0, align 4
  %52 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %51)
  call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %52, i32 %add.5, i32 undef, i32 %47, i32 %48, i32 %49, i32 %50, i8 15)
  %add.6 = add i32 %1, 112
  %53 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %0, align 4
  %54 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %53)
  call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %54, i32 %add.6, i32 undef, i32 %22, i32 undef, i32 undef, i32 undef, i8 1)
  ret void
}

; Function Attrs: alwaysinline nounwind readnone
define i32 @"\01?GetLeafIndexFromFlag@@YAHV?$vector@I$01@@@Z"(<2 x i32>) #4 {
entry:
  %1 = extractelement <2 x i32> %0, i32 0
  %and = and i32 %1, 1073741823
  ret i32 %and
}

; Function Attrs: alwaysinline nounwind readnone
define void @"\01?LogFloat3@@YAXV?$vector@M$02@@@Z"(<3 x float>) #4 {
entry:
  ret void
}

; Function Attrs: alwaysinline nounwind
define i1 @"\01?solveQuadratic@@YA_NMMMAIAM0@Z"(float, float, float, float* noalias nocapture dereferenceable(4), float* noalias nocapture dereferenceable(4)) #5 {
entry:
  %mul = fmul fast float %1, %1
  %mul1 = fmul fast float %0, 4.000000e+00
  %mul2 = fmul fast float %mul1, %2
  %sub = fsub fast float %mul, %mul2
  %cmp = fcmp fast olt float %sub, 0.000000e+00
  br i1 %cmp, label %return, label %if.else

if.else:                                          ; preds = %entry
  %cmp4 = fcmp fast oeq float %sub, 0.000000e+00
  br i1 %cmp4, label %if.then.7, label %if.else.9

if.then.7:                                        ; preds = %if.else
  %mul8 = fmul fast float %1, -5.000000e-01
  %div = fdiv fast float %mul8, %0
  br label %if.end.19

if.else.9:                                        ; preds = %if.else
  %cmp10 = fcmp fast ogt float %1, 0.000000e+00
  br i1 %cmp10, label %cond.true, label %cond.false

cond.true:                                        ; preds = %if.else.9
  %Sqrt = call float @dx.op.unary.f32(i32 24, float %sub)
  %add = fadd fast float %Sqrt, %1
  br label %cond.end

cond.false:                                       ; preds = %if.else.9
  %Sqrt28 = call float @dx.op.unary.f32(i32 24, float %sub)
  %sub15 = fsub fast float %1, %Sqrt28
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %add.sink = phi float [ %add, %cond.true ], [ %sub15, %cond.false ]
  %mul13 = fmul fast float %add.sink, -5.000000e-01
  %div17 = fdiv fast float %mul13, %0
  %div18 = fdiv fast float %2, %mul13
  br label %if.end.19

if.end.19:                                        ; preds = %cond.end, %if.then.7
  %.025 = phi float [ %div, %if.then.7 ], [ %div17, %cond.end ]
  %.0 = phi float [ %div, %if.then.7 ], [ %div18, %cond.end ]
  %cmp20 = fcmp fast ogt float %.025, %.0
  %.126 = select i1 %cmp20, float %.0, float %.025
  %.1 = select i1 %cmp20, float %.025, float %.0
  br label %return

return:                                           ; preds = %if.end.19, %entry
  %retval.0 = phi i1 [ false, %entry ], [ true, %if.end.19 ]
  %.227 = phi float [ undef, %entry ], [ %.126, %if.end.19 ]
  %.2 = phi float [ undef, %entry ], [ %.1, %if.end.19 ]
  store float %.227, float* %3, align 4
  store float %.2, float* %4, align 4
  ret i1 %retval.0
}

; Function Attrs: alwaysinline nounwind
define void @"\01?MyMissShader@@YAXUHitData@@@Z"(%struct.HitData* noalias nocapture) #5 {
entry:
  %1 = getelementptr inbounds %struct.HitData, %struct.HitData* %0, i32 0, i32 0
  store <4 x float> <float 0.000000e+00, float 0x3FC99999A0000000, float 0x3FD99999A0000000, float 1.000000e+00>, <4 x float>* %1, align 4
  ret void
}

; Function Attrs: alwaysinline nounwind
define void @"\01?PointerGetBuffer@@YA?AURWByteAddressBuffer@@V?$vector@I$01@@@Z"(%struct.RWByteAddressBuffer* noalias nocapture sret, <2 x i32>) #5 {
entry:
  ret void
}

; Function Attrs: alwaysinline nounwind
define void @"\01?RawDataToBoundingBox@@YA?AUBoundingBox@@V?$vector@H$03@@0AIAV?$vector@I$01@@@Z"(%struct.BoundingBox* noalias nocapture sret, <4 x i32>, <4 x i32>, <2 x i32>* noalias nocapture dereferenceable(8)) #5 {
entry:
  %4 = extractelement <4 x i32> %1, i32 0
  %5 = bitcast i32 %4 to float
  %6 = insertelement <3 x float> undef, float %5, i32 0
  %7 = extractelement <4 x i32> %1, i32 1
  %8 = bitcast i32 %7 to float
  %9 = insertelement <3 x float> %6, float %8, i32 1
  %10 = extractelement <4 x i32> %1, i32 2
  %11 = bitcast i32 %10 to float
  %12 = insertelement <3 x float> %9, float %11, i32 2
  %13 = extractelement <4 x i32> %2, i32 0
  %14 = bitcast i32 %13 to float
  %15 = insertelement <3 x float> undef, float %14, i32 0
  %16 = extractelement <4 x i32> %2, i32 1
  %17 = bitcast i32 %16 to float
  %18 = insertelement <3 x float> %15, float %17, i32 1
  %19 = extractelement <4 x i32> %2, i32 2
  %20 = bitcast i32 %19 to float
  %21 = insertelement <3 x float> %18, float %20, i32 2
  %.upto0 = insertelement <2 x i32> undef, i32 %24, i32 0
  %22 = insertelement <2 x i32> %.upto0, i32 %23, i32 1
  %23 = extractelement <4 x i32> %2, i32 3
  %24 = extractelement <4 x i32> %1, i32 3
  store <2 x i32> %22, <2 x i32>* %3, align 4, !tbaa !220
  %25 = getelementptr inbounds %struct.BoundingBox, %struct.BoundingBox* %0, i32 0, i32 0
  store <3 x float> %12, <3 x float>* %25, align 4
  %26 = getelementptr inbounds %struct.BoundingBox, %struct.BoundingBox* %0, i32 0, i32 1
  store <3 x float> %21, <3 x float>* %26, align 4
  ret void
}

; Function Attrs: alwaysinline nounwind
define void @"\01?TransformAABB@@YA?AUAABB@@U1@V?$matrix@M$02$03@@@Z"(%struct.AABB* noalias nocapture sret, %struct.AABB* nocapture readonly, %class.matrix.float.3.4) #5 {
entry:
  %.fca.0.0.extract = extractvalue %class.matrix.float.3.4 %2, 0, 0
  %3 = extractelement <4 x float> %.fca.0.0.extract, i32 3
  %4 = extractelement <4 x float> %.fca.0.0.extract, i32 2
  %5 = extractelement <4 x float> %.fca.0.0.extract, i32 1
  %6 = extractelement <4 x float> %.fca.0.0.extract, i32 0
  %.fca.0.1.extract = extractvalue %class.matrix.float.3.4 %2, 0, 1
  %7 = extractelement <4 x float> %.fca.0.1.extract, i32 3
  %8 = extractelement <4 x float> %.fca.0.1.extract, i32 2
  %9 = extractelement <4 x float> %.fca.0.1.extract, i32 1
  %10 = extractelement <4 x float> %.fca.0.1.extract, i32 0
  %.fca.0.2.extract = extractvalue %class.matrix.float.3.4 %2, 0, 2
  %11 = extractelement <4 x float> %.fca.0.2.extract, i32 3
  %12 = extractelement <4 x float> %.fca.0.2.extract, i32 2
  %13 = extractelement <4 x float> %.fca.0.2.extract, i32 1
  %14 = extractelement <4 x float> %.fca.0.2.extract, i32 0
  %boxVertices.0 = alloca [8 x float], align 4
  %boxVertices.1 = alloca [8 x float], align 4
  %boxVertices.2 = alloca [8 x float], align 4
  %min = getelementptr inbounds %struct.AABB, %struct.AABB* %1, i32 0, i32 0
  %15 = load <3 x float>, <3 x float>* %min, align 4
  %16 = extractelement <3 x float> %15, i64 0
  %17 = extractelement <3 x float> %15, i64 1
  %18 = extractelement <3 x float> %15, i64 2
  %19 = getelementptr [8 x float], [8 x float]* %boxVertices.0, i32 0, i32 0
  %20 = getelementptr [8 x float], [8 x float]* %boxVertices.1, i32 0, i32 0
  %21 = getelementptr [8 x float], [8 x float]* %boxVertices.2, i32 0, i32 0
  store float %16, float* %19, align 4
  store float %17, float* %20, align 4
  store float %18, float* %21, align 4
  %22 = extractelement <3 x float> %15, i32 1
  %23 = extractelement <3 x float> %15, i32 0
  %max = getelementptr inbounds %struct.AABB, %struct.AABB* %1, i32 0, i32 1
  %24 = load <3 x float>, <3 x float>* %max, align 4
  %25 = extractelement <3 x float> %24, i32 2
  %26 = getelementptr [8 x float], [8 x float]* %boxVertices.0, i32 0, i32 1
  %27 = getelementptr [8 x float], [8 x float]* %boxVertices.1, i32 0, i32 1
  %28 = getelementptr [8 x float], [8 x float]* %boxVertices.2, i32 0, i32 1
  store float %23, float* %26, align 4
  store float %22, float* %27, align 4
  store float %25, float* %28, align 4
  %29 = extractelement <3 x float> %24, i32 1
  %30 = getelementptr [8 x float], [8 x float]* %boxVertices.0, i32 0, i32 2
  %31 = getelementptr [8 x float], [8 x float]* %boxVertices.1, i32 0, i32 2
  %32 = getelementptr [8 x float], [8 x float]* %boxVertices.2, i32 0, i32 2
  store float %23, float* %30, align 4
  store float %29, float* %31, align 4
  store float %25, float* %32, align 4
  %33 = extractelement <3 x float> %15, i32 2
  %34 = getelementptr [8 x float], [8 x float]* %boxVertices.0, i32 0, i32 3
  %35 = getelementptr [8 x float], [8 x float]* %boxVertices.1, i32 0, i32 3
  %36 = getelementptr [8 x float], [8 x float]* %boxVertices.2, i32 0, i32 3
  store float %23, float* %34, align 4
  store float %29, float* %35, align 4
  store float %33, float* %36, align 4
  %37 = extractelement <3 x float> %24, i32 0
  %38 = getelementptr [8 x float], [8 x float]* %boxVertices.0, i32 0, i32 4
  %39 = getelementptr [8 x float], [8 x float]* %boxVertices.1, i32 0, i32 4
  %40 = getelementptr [8 x float], [8 x float]* %boxVertices.2, i32 0, i32 4
  store float %37, float* %38, align 4
  store float %22, float* %39, align 4
  store float %33, float* %40, align 4
  %41 = getelementptr [8 x float], [8 x float]* %boxVertices.0, i32 0, i32 6
  %42 = getelementptr [8 x float], [8 x float]* %boxVertices.1, i32 0, i32 6
  %43 = getelementptr [8 x float], [8 x float]* %boxVertices.2, i32 0, i32 6
  store float %37, float* %41, align 4
  store float %22, float* %42, align 4
  store float %25, float* %43, align 4
  %44 = getelementptr [8 x float], [8 x float]* %boxVertices.0, i32 0, i32 5
  %45 = getelementptr [8 x float], [8 x float]* %boxVertices.1, i32 0, i32 5
  %46 = getelementptr [8 x float], [8 x float]* %boxVertices.2, i32 0, i32 5
  store float %37, float* %44, align 4
  store float %29, float* %45, align 4
  store float %33, float* %46, align 4
  %47 = extractelement <3 x float> %24, i64 0
  %48 = extractelement <3 x float> %24, i64 1
  %49 = extractelement <3 x float> %24, i64 2
  %50 = getelementptr [8 x float], [8 x float]* %boxVertices.0, i32 0, i32 7
  %51 = getelementptr [8 x float], [8 x float]* %boxVertices.1, i32 0, i32 7
  %52 = getelementptr [8 x float], [8 x float]* %boxVertices.2, i32 0, i32 7
  store float %47, float* %50, align 4
  store float %48, float* %51, align 4
  store float %49, float* %52, align 4
  br label %for.body

for.body:                                         ; preds = %for.body.for.body_crit_edge, %entry
  %load44 = phi float [ 1.000000e+00, %entry ], [ %load44.pre, %for.body.for.body_crit_edge ]
  %load42 = phi float [ %18, %entry ], [ %load42.pre, %for.body.for.body_crit_edge ]
  %load40 = phi float [ %17, %entry ], [ %load40.pre, %for.body.for.body_crit_edge ]
  %load = phi float [ %16, %entry ], [ %load.pre, %for.body.for.body_crit_edge ]
  %.0.049.i0 = phi float [ 0x47EFFFFFE0000000, %entry ], [ %FMin, %for.body.for.body_crit_edge ]
  %.0.049.i1 = phi float [ 0x47EFFFFFE0000000, %entry ], [ %FMin52, %for.body.for.body_crit_edge ]
  %.0.049.i2 = phi float [ 0x47EFFFFFE0000000, %entry ], [ %FMin53, %for.body.for.body_crit_edge ]
  %.1.048.i0 = phi float [ 0xC7EFFFFFE0000000, %entry ], [ %FMax, %for.body.for.body_crit_edge ]
  %.1.048.i1 = phi float [ 0xC7EFFFFFE0000000, %entry ], [ %FMax50, %for.body.for.body_crit_edge ]
  %.1.048.i2 = phi float [ 0xC7EFFFFFE0000000, %entry ], [ %FMax51, %for.body.for.body_crit_edge ]
  %i.047 = phi i32 [ 0, %entry ], [ %inc, %for.body.for.body_crit_edge ]
  %53 = fmul fast float %load, %6
  %FMad62 = call float @dx.op.tertiary.f32(i32 46, float %load40, float %5, float %53)
  %FMad61 = call float @dx.op.tertiary.f32(i32 46, float %load42, float %4, float %FMad62)
  %FMad60 = call float @dx.op.tertiary.f32(i32 46, float %load44, float %3, float %FMad61)
  %54 = fmul fast float %load, %10
  %FMad59 = call float @dx.op.tertiary.f32(i32 46, float %load40, float %9, float %54)
  %FMad58 = call float @dx.op.tertiary.f32(i32 46, float %load42, float %8, float %FMad59)
  %FMad57 = call float @dx.op.tertiary.f32(i32 46, float %load44, float %7, float %FMad58)
  %55 = fmul fast float %load, %14
  %FMad56 = call float @dx.op.tertiary.f32(i32 46, float %load40, float %13, float %55)
  %FMad55 = call float @dx.op.tertiary.f32(i32 46, float %load42, float %12, float %FMad56)
  %FMad = call float @dx.op.tertiary.f32(i32 46, float %load44, float %11, float %FMad55)
  %FMin = call float @dx.op.binary.f32(i32 36, float %.0.049.i0, float %FMad60)
  %FMin52 = call float @dx.op.binary.f32(i32 36, float %.0.049.i1, float %FMad57)
  %FMin53 = call float @dx.op.binary.f32(i32 36, float %.0.049.i2, float %FMad)
  %FMax = call float @dx.op.binary.f32(i32 35, float %.1.048.i0, float %FMad60)
  %FMax50 = call float @dx.op.binary.f32(i32 35, float %.1.048.i1, float %FMad57)
  %FMax51 = call float @dx.op.binary.f32(i32 35, float %.1.048.i2, float %FMad)
  %inc = add nuw nsw i32 %i.047, 1
  %exitcond = icmp eq i32 %inc, 8
  br i1 %exitcond, label %for.end, label %for.body.for.body_crit_edge

for.body.for.body_crit_edge:                      ; preds = %for.body
  %.phi.trans.insert = getelementptr [8 x float], [8 x float]* %boxVertices.0, i32 0, i32 %inc
  %load.pre = load float, float* %.phi.trans.insert, align 4
  %.phi.trans.insert72 = getelementptr [8 x float], [8 x float]* %boxVertices.1, i32 0, i32 %inc
  %load40.pre = load float, float* %.phi.trans.insert72, align 4
  %.phi.trans.insert74 = getelementptr [8 x float], [8 x float]* %boxVertices.2, i32 0, i32 %inc
  %load42.pre = load float, float* %.phi.trans.insert74, align 4
  %.phi.trans.insert76 = getelementptr [8 x float], [8 x float]* @"1boxVertices.3.hca", i32 0, i32 %inc
  %load44.pre = load float, float* %.phi.trans.insert76, align 4
  br label %for.body

for.end:                                          ; preds = %for.body
  %56 = insertelement <3 x float> undef, float %FMax, i64 0
  %57 = insertelement <3 x float> %56, float %FMax50, i64 1
  %58 = insertelement <3 x float> %57, float %FMax51, i64 2
  %59 = insertelement <3 x float> undef, float %FMin, i64 0
  %60 = insertelement <3 x float> %59, float %FMin52, i64 1
  %61 = insertelement <3 x float> %60, float %FMin53, i64 2
  %62 = getelementptr inbounds %struct.AABB, %struct.AABB* %0, i32 0, i32 0
  store <3 x float> %61, <3 x float>* %62, align 4
  %63 = getelementptr inbounds %struct.AABB, %struct.AABB* %0, i32 0, i32 1
  store <3 x float> %58, <3 x float>* %63, align 4
  ret void
}

; Function Attrs: alwaysinline nounwind readonly
define <4 x i32> @"\01?Load4@@YA?AV?$vector@I$03@@V?$vector@I$01@@@Z"(<2 x i32>) #6 {
entry:
  %1 = extractelement <2 x i32> %0, i32 1
  %arrayidx.i = getelementptr inbounds [0 x %struct.RWByteAddressBuffer], [0 x %struct.RWByteAddressBuffer]* @"\01?DescriptorHeapBufferTable@@3PAURWByteAddressBuffer@@A", i32 0, i32 %1, !dx.nonuniform !233
  %2 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %arrayidx.i, align 4, !noalias !348
  %3 = extractelement <2 x i32> %0, i32 0
  %4 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %2)
  %5 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %4, i32 %3, i32 undef)
  %6 = extractvalue %dx.types.ResRet.i32 %5, 0
  %7 = insertelement <4 x i32> undef, i32 %6, i64 0
  %8 = extractvalue %dx.types.ResRet.i32 %5, 1
  %9 = insertelement <4 x i32> %7, i32 %8, i64 1
  %10 = extractvalue %dx.types.ResRet.i32 %5, 2
  %11 = insertelement <4 x i32> %9, i32 %10, i64 2
  %12 = extractvalue %dx.types.ResRet.i32 %5, 3
  %13 = insertelement <4 x i32> %11, i32 %12, i64 3
  ret <4 x i32> %13
}

; Function Attrs: alwaysinline nounwind readnone
define i32 @"\01?PointerGetBufferStartOffset@@YAIV?$vector@I$01@@@Z"(<2 x i32>) #4 {
entry:
  %1 = extractelement <2 x i32> %0, i32 0
  ret i32 %1
}

; Function Attrs: alwaysinline nounwind readonly
define <3 x i32> @"\01?Load3x16BitIndices@@YA?AV?$vector@I$02@@I@Z"(i32) #6 {
entry:
  %1 = load %struct.ByteAddressBuffer, %struct.ByteAddressBuffer* @"\01?Indices@@3UByteAddressBuffer@@A", align 4
  %and = and i32 %0, -4
  %2 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.ByteAddressBuffer(i32 160, %struct.ByteAddressBuffer %1)
  %3 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %2, i32 %and, i32 undef)
  %4 = extractvalue %dx.types.ResRet.i32 %3, 0
  %5 = extractvalue %dx.types.ResRet.i32 %3, 1
  %cmp = icmp eq i32 %and, %0
  %and2 = and i32 %4, 65535
  %shr = lshr i32 %4, 16
  %shr8 = lshr i32 %5, 16
  %.sink = select i1 %cmp, i32 %5, i32 %shr8
  %retval.0.i0 = select i1 %cmp, i32 %and2, i32 %shr
  %retval.0.i1.in = select i1 %cmp, i32 %shr, i32 %5
  %retval.0.i1 = and i32 %retval.0.i1.in, 65535
  %and4 = and i32 %.sink, 65535
  %retval.0.upto0 = insertelement <3 x i32> undef, i32 %retval.0.i0, i32 0
  %retval.0.upto1 = insertelement <3 x i32> %retval.0.upto0, i32 %retval.0.i1, i32 1
  %retval.0 = insertelement <3 x i32> %retval.0.upto1, i32 %and4, i32 2
  ret <3 x i32> %retval.0
}

; Function Attrs: alwaysinline nounwind
define void @"\01?Fallback_AcceptHitAndEndSearch@@YAXXZ"() #5 {
entry:
  call void @"\01?Fallback_SetAnyHitResult@@YAXH@Z"(i32 -1) #2
  ret void
}

; Function Attrs: alwaysinline nounwind
define void @"\01?swap@@YAXAIAH0@Z"(i32* noalias nocapture dereferenceable(4), i32* noalias nocapture dereferenceable(4)) #5 {
entry:
  %2 = load i32, i32* %0, align 4, !tbaa !223
  %3 = load i32, i32* %1, align 4, !tbaa !223
  store i32 %3, i32* %0, align 4, !tbaa !223
  store i32 %2, i32* %1, align 4, !tbaa !223
  ret void
}

; Function Attrs: alwaysinline nounwind readonly
define <3 x float> @"\01?HitWorldPosition@@YA?AV?$vector@M$02@@XZ"() #6 {
entry:
  %WorldRayOrigin = call float @dx.op.worldRayOrigin.f32(i32 147, i8 0)
  %WorldRayOrigin5 = call float @dx.op.worldRayOrigin.f32(i32 147, i8 1)
  %WorldRayOrigin6 = call float @dx.op.worldRayOrigin.f32(i32 147, i8 2)
  %RayTCurrent = call float @dx.op.rayTCurrent.f32(i32 154)
  %WorldRayDirection = call float @dx.op.worldRayDirection.f32(i32 148, i8 0)
  %WorldRayDirection3 = call float @dx.op.worldRayDirection.f32(i32 148, i8 1)
  %WorldRayDirection4 = call float @dx.op.worldRayDirection.f32(i32 148, i8 2)
  %mul.i0 = fmul fast float %WorldRayDirection, %RayTCurrent
  %mul.i1 = fmul fast float %WorldRayDirection3, %RayTCurrent
  %mul.i2 = fmul fast float %WorldRayDirection4, %RayTCurrent
  %add.i0 = fadd fast float %mul.i0, %WorldRayOrigin
  %add.i1 = fadd fast float %mul.i1, %WorldRayOrigin5
  %add.i2 = fadd fast float %mul.i2, %WorldRayOrigin6
  %add.upto0 = insertelement <3 x float> undef, float %add.i0, i32 0
  %add.upto1 = insertelement <3 x float> %add.upto0, float %add.i1, i32 1
  %add = insertelement <3 x float> %add.upto1, float %add.i2, i32 2
  ret <3 x float> %add
}

; Function Attrs: alwaysinline nounwind
define void @"\01?WriteBoxToBuffer@@YAXURWByteAddressBuffer@@IIUBoundingBox@@V?$vector@I$01@@@Z"(%struct.RWByteAddressBuffer* nocapture readonly, i32, i32, %struct.BoundingBox* nocapture readonly, <2 x i32>) #5 {
entry:
  %mul.i = shl i32 %2, 5
  %add.i = add i32 %mul.i, %1
  %center.i = getelementptr inbounds %struct.BoundingBox, %struct.BoundingBox* %3, i32 0, i32 0
  %5 = load <3 x float>, <3 x float>* %center.i, align 4, !noalias !351
  %6 = extractelement <3 x float> %5, i32 0
  %7 = bitcast float %6 to i32
  %8 = extractelement <3 x float> %5, i32 1
  %9 = bitcast float %8 to i32
  %10 = extractelement <3 x float> %5, i32 2
  %11 = bitcast float %10 to i32
  %12 = extractelement <2 x i32> %4, i32 0
  %halfDim.i = getelementptr inbounds %struct.BoundingBox, %struct.BoundingBox* %3, i32 0, i32 1
  %13 = load <3 x float>, <3 x float>* %halfDim.i, align 4, !noalias !351
  %14 = extractelement <3 x float> %13, i32 0
  %15 = bitcast float %14 to i32
  %16 = extractelement <3 x float> %13, i32 1
  %17 = bitcast float %16 to i32
  %18 = extractelement <3 x float> %13, i32 2
  %19 = bitcast float %18 to i32
  %20 = extractelement <2 x i32> %4, i32 1
  %21 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %0, align 4
  %22 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %21)
  call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %22, i32 %add.i, i32 undef, i32 %7, i32 %9, i32 %11, i32 %12, i8 15)
  %add = add i32 %add.i, 16
  %23 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %0, align 4
  %24 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %23)
  call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %24, i32 %add, i32 undef, i32 %15, i32 %17, i32 %19, i32 %20, i8 15)
  ret void
}

; Function Attrs: alwaysinline nounwind readonly
define i32 @"\01?GetOffsetToPrimitiveMetaData@@YAHURWByteAddressBufferPointer@@@Z"(%struct.RWByteAddressBufferPointer* nocapture readonly) #6 {
entry:
  %offsetInBytes = getelementptr inbounds %struct.RWByteAddressBufferPointer, %struct.RWByteAddressBufferPointer* %0, i32 0, i32 1
  %1 = load i32, i32* %offsetInBytes, align 4, !tbaa !223
  %add = add i32 %1, 8
  %2 = getelementptr inbounds %struct.RWByteAddressBufferPointer, %struct.RWByteAddressBufferPointer* %0, i32 0, i32 0
  %3 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %2, align 4
  %4 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %3)
  %5 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %4, i32 %add, i32 undef)
  %6 = extractvalue %dx.types.ResRet.i32 %5, 0
  %add2 = add i32 %6, %1
  ret i32 %add2
}

; Function Attrs: alwaysinline nounwind
define void @"\01?MyClosestHitShader_Triangle@@YAXUHitData@@UBuiltInTriangleIntersectionAttributes@@@Z"(%struct.HitData* noalias nocapture, %struct.BuiltInTriangleIntersectionAttributes* nocapture readonly) #5 {
entry:
  %LoadShaderTableHandle = load %struct.ByteAddressBuffer, %struct.ByteAddressBuffer* @"\01?HitGroupShaderTable@@3UByteAddressBuffer@@A"
  %ShaderTableHandle = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.ByteAddressBuffer(i32 160, %struct.ByteAddressBuffer %LoadShaderTableHandle)
  %shaderRecordOffset = call i32 @"\01?Fallback_ShaderRecordOffset@@YAIXZ"()
  %2 = load %"class.StructuredBuffer<Vertex>", %"class.StructuredBuffer<Vertex>"* @"\01?Vertices@@3V?$StructuredBuffer@UVertex@@@@A", align 4
  %3 = load %struct.ByteAddressBuffer, %struct.ByteAddressBuffer* @"\01?Indices@@3UByteAddressBuffer@@A", align 4
  %4 = load %struct.RaytracingAccelerationStructure, %struct.RaytracingAccelerationStructure* @"\01?Scene@@3URaytracingAccelerationStructure@@A", align 4
  %5 = load %g_sceneCB, %g_sceneCB* @g_sceneCB, align 4
  %g_sceneCB63 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.g_sceneCB(i32 160, %g_sceneCB %5)
  %6 = alloca %struct.ShadowPayload, align 8
  %WorldRayOrigin = call float @dx.op.worldRayOrigin.f32(i32 147, i8 0)
  %WorldRayOrigin58 = call float @dx.op.worldRayOrigin.f32(i32 147, i8 1)
  %WorldRayOrigin59 = call float @dx.op.worldRayOrigin.f32(i32 147, i8 2)
  %RayTCurrent = call float @dx.op.rayTCurrent.f32(i32 154)
  %WorldRayDirection = call float @dx.op.worldRayDirection.f32(i32 148, i8 0)
  %WorldRayDirection56 = call float @dx.op.worldRayDirection.f32(i32 148, i8 1)
  %WorldRayDirection57 = call float @dx.op.worldRayDirection.f32(i32 148, i8 2)
  %mul.i.i0 = fmul fast float %WorldRayDirection, %RayTCurrent
  %mul.i.i1 = fmul fast float %WorldRayDirection56, %RayTCurrent
  %mul.i.i2 = fmul fast float %WorldRayDirection57, %RayTCurrent
  %add.i.i0 = fadd fast float %mul.i.i0, %WorldRayOrigin
  %add.i.i1 = fadd fast float %mul.i.i1, %WorldRayOrigin58
  %add.i.i2 = fadd fast float %mul.i.i2, %WorldRayOrigin59
  %PrimitiveID = call i32 @dx.op.primitiveID.i32(i32 108)
  %mul2 = mul i32 %PrimitiveID, 6
  %and.i = and i32 %mul2, -4
  %7 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.ByteAddressBuffer(i32 160, %struct.ByteAddressBuffer %3)
  %8 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %7, i32 %and.i, i32 undef)
  %9 = extractvalue %dx.types.ResRet.i32 %8, 0
  %10 = extractvalue %dx.types.ResRet.i32 %8, 1
  %cmp.i = icmp eq i32 %and.i, %mul2
  %and2.i = and i32 %9, 65535
  %shr.i = lshr i32 %9, 16
  %shr8.i = lshr i32 %10, 16
  %.sink = select i1 %cmp.i, i32 %10, i32 %shr8.i
  %retval.i.0.i0 = select i1 %cmp.i, i32 %and2.i, i32 %shr.i
  %retval.i.0.i1.in = select i1 %cmp.i, i32 %shr.i, i32 %10
  %retval.i.0.i1 = and i32 %retval.i.0.i1.in, 65535
  %and4.i = and i32 %.sink, 65535
  %11 = call %dx.types.Handle @"dx.op.createHandleFromResourceStructForLib.class.StructuredBuffer<Vertex>"(i32 160, %"class.StructuredBuffer<Vertex>" %2)
  %12 = call %dx.types.ResRet.f32 @dx.op.bufferLoad.f32(i32 68, %dx.types.Handle %11, i32 %retval.i.0.i0, i32 12)
  %13 = extractvalue %dx.types.ResRet.f32 %12, 0
  %14 = extractvalue %dx.types.ResRet.f32 %12, 1
  %15 = extractvalue %dx.types.ResRet.f32 %12, 2
  %16 = call %dx.types.ResRet.f32 @dx.op.bufferLoad.f32(i32 68, %dx.types.Handle %11, i32 %retval.i.0.i1, i32 12)
  %17 = extractvalue %dx.types.ResRet.f32 %16, 0
  %18 = extractvalue %dx.types.ResRet.f32 %16, 1
  %19 = extractvalue %dx.types.ResRet.f32 %16, 2
  %20 = call %dx.types.ResRet.f32 @dx.op.bufferLoad.f32(i32 68, %dx.types.Handle %11, i32 %and4.i, i32 12)
  %21 = extractvalue %dx.types.ResRet.f32 %20, 0
  %22 = extractvalue %dx.types.ResRet.f32 %20, 1
  %23 = extractvalue %dx.types.ResRet.f32 %20, 2
  %barycentrics = getelementptr inbounds %struct.BuiltInTriangleIntersectionAttributes, %struct.BuiltInTriangleIntersectionAttributes* %1, i32 0, i32 0
  %24 = load <2 x float>, <2 x float>* %barycentrics, align 4, !tbaa !220
  %25 = extractelement <2 x float> %24, i32 0
  %sub.i.i0 = fsub fast float %17, %13
  %sub.i.i1 = fsub fast float %18, %14
  %sub.i.i2 = fsub fast float %19, %15
  %mul.i.19.i0 = fmul fast float %sub.i.i0, %25
  %mul.i.19.i1 = fmul fast float %sub.i.i1, %25
  %mul.i.19.i2 = fmul fast float %sub.i.i2, %25
  %add.i.20.i0 = fadd fast float %mul.i.19.i0, %13
  %add.i.20.i1 = fadd fast float %mul.i.19.i1, %14
  %add.i.20.i2 = fadd fast float %mul.i.19.i2, %15
  %26 = extractelement <2 x float> %24, i32 1
  %sub7.i.i0 = fsub fast float %21, %13
  %sub7.i.i1 = fsub fast float %22, %14
  %sub7.i.i2 = fsub fast float %23, %15
  %mul8.i.i0 = fmul fast float %sub7.i.i0, %26
  %mul8.i.i1 = fmul fast float %sub7.i.i1, %26
  %mul8.i.i2 = fmul fast float %sub7.i.i2, %26
  %add9.i.i0 = fadd fast float %add.i.20.i0, %mul8.i.i0
  %add9.i.i1 = fadd fast float %add.i.20.i1, %mul8.i.i1
  %add9.i.i2 = fadd fast float %add.i.20.i2, %mul8.i.i2
  %27 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %g_sceneCB63, i32 5)
  %28 = extractvalue %dx.types.CBufRet.f32 %27, 0
  %29 = extractvalue %dx.types.CBufRet.f32 %27, 1
  %30 = extractvalue %dx.types.CBufRet.f32 %27, 2
  %sub.i0 = fsub fast float %28, %add.i.i0
  %sub.i1 = fsub fast float %29, %add.i.i1
  %sub.i2 = fsub fast float %30, %add.i.i2
  %31 = fmul fast float %sub.i0, %sub.i0
  %32 = fmul fast float %sub.i1, %sub.i1
  %33 = fadd fast float %31, %32
  %34 = fmul fast float %sub.i2, %sub.i2
  %35 = fadd fast float %33, %34
  %Sqrt55 = call float @dx.op.unary.f32(i32 24, float %35)
  %.i0 = fdiv fast float %sub.i0, %Sqrt55
  %.i1 = fdiv fast float %sub.i1, %Sqrt55
  %.i2 = fdiv fast float %sub.i2, %Sqrt55
  %36 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RaytracingAccelerationStructure(i32 160, %struct.RaytracingAccelerationStructure %4)
  call void @dx.op.traceRay.struct.ShadowPayload(i32 157, %dx.types.Handle %36, i32 4, i32 -1, i32 1, i32 0, i32 1, float %add.i.i0, float %add.i.i1, float %add.i.i2, float 0x3F1A36E2E0000000, float %.i0, float %.i1, float %.i2, float 1.000000e+04, %struct.ShadowPayload* nonnull %6)
  %37 = getelementptr inbounds %struct.ShadowPayload, %struct.ShadowPayload* %6, i32 0, i32 0
  %38 = load i32, i32* %37, align 8
  %tobool = icmp ne i32 %38, 0
  %cond12 = select i1 %tobool, float 0x3FB99999A0000000, float 1.000000e+00
  %39 = add i32 12, %shaderRecordOffset
  %ShaderRecordBuffer = call %dx.types.ResRet.f32 @dx.op.bufferLoad.f32(i32 68, %dx.types.Handle %ShaderTableHandle, i32 %39, i32 undef)
  %40 = extractvalue %dx.types.ResRet.f32 %ShaderRecordBuffer, 0
  %41 = extractvalue %dx.types.ResRet.f32 %ShaderRecordBuffer, 1
  %42 = extractvalue %dx.types.ResRet.f32 %ShaderRecordBuffer, 2
  %43 = extractvalue %dx.types.ResRet.f32 %ShaderRecordBuffer, 3
  %44 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %g_sceneCB63, i32 5)
  %45 = extractvalue %dx.types.CBufRet.f32 %44, 0
  %46 = extractvalue %dx.types.CBufRet.f32 %44, 1
  %47 = extractvalue %dx.types.CBufRet.f32 %44, 2
  %sub.i.21.i0 = fsub fast float %45, %add.i.i0
  %sub.i.21.i1 = fsub fast float %46, %add.i.i1
  %sub.i.21.i2 = fsub fast float %47, %add.i.i2
  %48 = fmul fast float %sub.i.21.i0, %sub.i.21.i0
  %49 = fmul fast float %sub.i.21.i1, %sub.i.21.i1
  %50 = fadd fast float %48, %49
  %51 = fmul fast float %sub.i.21.i2, %sub.i.21.i2
  %52 = fadd fast float %50, %51
  %Sqrt = call float @dx.op.unary.f32(i32 24, float %52)
  %.i064 = fdiv fast float %sub.i.21.i0, %Sqrt
  %.i165 = fdiv fast float %sub.i.21.i1, %Sqrt
  %.i266 = fdiv fast float %sub.i.21.i2, %Sqrt
  %53 = call float @dx.op.dot3.f32(i32 55, float %.i064, float %.i165, float %.i266, float %add9.i.i0, float %add9.i.i1, float %add9.i.i2)
  %FMax = call float @dx.op.binary.f32(i32 35, float 0.000000e+00, float %53)
  %54 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %g_sceneCB63, i32 7)
  %55 = extractvalue %dx.types.CBufRet.f32 %54, 0
  %56 = extractvalue %dx.types.CBufRet.f32 %54, 1
  %57 = extractvalue %dx.types.CBufRet.f32 %54, 2
  %58 = extractvalue %dx.types.CBufRet.f32 %54, 3
  %mul.i.24.i0 = fmul fast float %cond12, %40
  %mul13.i0 = fmul fast float %mul.i.24.i0, %FMax
  %mul15.i0 = fmul fast float %mul13.i0, %55
  %mul.i.24.i1 = fmul fast float %cond12, %41
  %mul13.i1 = fmul fast float %mul.i.24.i1, %FMax
  %mul15.i1 = fmul fast float %mul13.i1, %56
  %mul.i.24.i2 = fmul fast float %cond12, %42
  %mul13.i2 = fmul fast float %mul.i.24.i2, %FMax
  %mul15.i2 = fmul fast float %mul13.i2, %57
  %mul.i.24.i3 = fmul fast float %cond12, %43
  %mul13.i3 = fmul fast float %mul.i.24.i3, %FMax
  %mul15.i3 = fmul fast float %mul13.i3, %58
  %59 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %g_sceneCB63, i32 6)
  %60 = extractvalue %dx.types.CBufRet.f32 %59, 0
  %61 = extractvalue %dx.types.CBufRet.f32 %59, 1
  %62 = extractvalue %dx.types.CBufRet.f32 %59, 2
  %63 = extractvalue %dx.types.CBufRet.f32 %59, 3
  %add.i0 = fadd fast float %60, %mul15.i0
  %add.i1 = fadd fast float %61, %mul15.i1
  %add.i2 = fadd fast float %62, %mul15.i2
  %add.i3 = fadd fast float %63, %mul15.i3
  %add.upto0 = insertelement <4 x float> undef, float %add.i0, i32 0
  %add.upto1 = insertelement <4 x float> %add.upto0, float %add.i1, i32 1
  %add.upto2 = insertelement <4 x float> %add.upto1, float %add.i2, i32 2
  %add = insertelement <4 x float> %add.upto2, float %add.i3, i32 3
  %64 = getelementptr inbounds %struct.HitData, %struct.HitData* %0, i32 0, i32 0
  store <4 x float> %add, <4 x float>* %64, align 4
  ret void
}

; Function Attrs: alwaysinline nounwind
define void @"\01?main@@YAXV?$vector@I$02@@I@Z"(<3 x i32>, i32) #5 {
entry:
  %2 = load %struct.ByteAddressBuffer, %struct.ByteAddressBuffer* @"\01?RayGenShaderTable@@3UByteAddressBuffer@@A", align 4
  %3 = load %Constants, %Constants* @Constants, align 4
  %Constants = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.Constants(i32 160, %Constants %3)
  %4 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.ByteAddressBuffer(i32 160, %struct.ByteAddressBuffer %2)
  %5 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %4, i32 0, i32 undef)
  %6 = extractvalue %dx.types.ResRet.i32 %5, 0
  %7 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %Constants, i32 0)
  %8 = extractvalue %dx.types.CBufRet.i32 %7, 1
  %9 = extractvalue %dx.types.CBufRet.i32 %7, 0
  call void @"\01?Fallback_Scheduler@@YAXHII@Z"(i32 %6, i32 %9, i32 %8) #2
  ret void
}

; Function Attrs: alwaysinline nounwind readonly
define i32 @"\01?GetOffsetToBoxes@@YAHURWByteAddressBufferPointer@@@Z"(%struct.RWByteAddressBufferPointer* nocapture readonly) #6 {
entry:
  %offsetInBytes = getelementptr inbounds %struct.RWByteAddressBufferPointer, %struct.RWByteAddressBufferPointer* %0, i32 0, i32 1
  %1 = load i32, i32* %offsetInBytes, align 4, !tbaa !223
  %add = add i32 %1, 16
  ret i32 %add
}

attributes #0 = { alwaysinline "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="0" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readonly }
attributes #2 = { nounwind }
attributes #3 = { nounwind readnone }
attributes #4 = { alwaysinline nounwind readnone "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="0" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { alwaysinline nounwind "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="0" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #6 = { alwaysinline nounwind readonly "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="0" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #7 = { alwaysinline nounwind "disable-tail-calls"="false" "exp-shader"="internal" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="0" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #8 = { alwaysinline }

!llvm.ident = !{!0, !0, !0}
!llvm.module.flags = !{!1, !2}
!dx.version = !{!3}
!dx.valver = !{!4}
!dx.shaderModel = !{!5}
!dx.resources = !{!6}
!dx.typeAnnotations = !{!27, !111}
!dx.entryPoints = !{!209}
!dx.func.props = !{!210, !211, !212, !213, !214, !215, !216, !217, !218}
!dx.func.signatures = !{!219}

!0 = !{!"clang version 3.7 (tags/RELEASE_370/final)"}
!1 = !{i32 2, !"Dwarf Version", i32 4}
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 1, i32 1}
!4 = !{i32 1, i32 2}
!5 = !{!"lib", i32 6, i32 1}
!6 = !{!7, !19, !23, null}
!7 = !{!8, !9, !11, !13, !14, !16, !17, !18}
!8 = !{i32 0, %struct.ByteAddressBuffer* @"\01?RayGenShaderTable@@3UByteAddressBuffer@@A", !"RayGenShaderTable", i32 214743647, i32 2, i32 1, i32 11, i32 0, null}
!9 = !{i32 1, %"class.StructuredBuffer<Vertex>"* @"\01?Vertices@@3V?$StructuredBuffer@UVertex@@@@A", !"Vertices", i32 0, i32 2, i32 1, i32 12, i32 0, !10}
!10 = !{i32 1, i32 24}
!11 = !{i32 2, %"class.StructuredBuffer<AABBPrimitiveAttributes>"* @"\01?g_AABBPrimitiveAttributes@@3V?$StructuredBuffer@UAABBPrimitiveAttributes@@@@A", !"g_AABBPrimitiveAttributes", i32 0, i32 3, i32 1, i32 12, i32 0, !12}
!12 = !{i32 1, i32 80}
!13 = !{i32 3, %struct.ByteAddressBuffer* @"\01?HitGroupShaderTable@@3UByteAddressBuffer@@A", !"\01?HitGroupShaderTable@@3UByteAddressBuffer@@A", i32 214743647, i32 0, i32 1, i32 11, i32 0, null}
!14 = !{i32 4, %struct.RaytracingAccelerationStructure* @"\01?Scene@@3URaytracingAccelerationStructure@@A", !"Scene", i32 0, i32 0, i32 1, i32 16, i32 0, !15}
!15 = !{i32 0, i32 4}
!16 = !{i32 5, %struct.ByteAddressBuffer* @"\01?HitGroupShaderTable@@3UByteAddressBuffer@@A", !"HitGroupShaderTable", i32 214743647, i32 0, i32 1, i32 11, i32 0, null}
!17 = !{i32 6, %struct.ByteAddressBuffer* @"\01?MissShaderTable@@3UByteAddressBuffer@@A", !"MissShaderTable", i32 214743647, i32 1, i32 1, i32 11, i32 0, null}
!18 = !{i32 7, %struct.ByteAddressBuffer* @"\01?Indices@@3UByteAddressBuffer@@A", !"Indices", i32 0, i32 1, i32 1, i32 11, i32 0, null}
!19 = !{!20, !21}
!20 = !{i32 0, [0 x %struct.RWByteAddressBuffer]* @"\01?DescriptorHeapBufferTable@@3PAURWByteAddressBuffer@@A", !"DescriptorHeapBufferTable", i32 214743648, i32 0, i32 -1, i32 11, i1 false, i1 false, i1 false, null}
!21 = !{i32 1, %"class.RWTexture2D<vector<float, 4> >"* @"\01?RenderTarget@@3V?$RWTexture2D@V?$vector@M$03@@@@A", !"RenderTarget", i32 0, i32 0, i32 1, i32 2, i1 false, i1 false, i1 false, !22}
!22 = !{i32 0, i32 9}
!23 = !{!24, !25, !26}
!24 = !{i32 0, %AccelerationStructureList* @AccelerationStructureList, !"AccelerationStructureList", i32 214743647, i32 1, i32 1, i32 8, null}
!25 = !{i32 1, %g_sceneCB* @g_sceneCB, !"g_sceneCB", i32 0, i32 0, i32 1, i32 128, null}
!26 = !{i32 2, %Constants* @Constants, !"Constants", i32 214743647, i32 0, i32 1, i32 32, null}
!27 = !{i32 0, %struct.BVHMetadata undef, !28, %struct.RaytracingInstanceDesc undef, !32, %struct.RWByteAddressBuffer undef, !37, %struct.BoundingBox undef, !39, %struct.ByteAddressBuffer undef, !37, %struct.RWByteAddressBufferPointer undef, !42, %struct.ProceduralPrimitiveAttributes undef, !45, %struct.ShadowPayload undef, !47, %struct.AABB undef, !49, %struct.Primitive undef, !52, %struct.Triangle undef, !57, %struct.RayData undef, !61, %struct.HitData undef, !68, %struct.PrimitiveMetaData undef, !70, %struct.BuiltInTriangleIntersectionAttributes undef, !73, %g_sceneCB undef, !75, %struct.SceneConstantBuffer undef, !77, %"class.RWTexture2D<vector<float, 4> >" undef, !84, %struct.RaytracingAccelerationStructure undef, !37, %"class.StructuredBuffer<AABBPrimitiveAttributes>" undef, !86, %struct.AABBPrimitiveAttributes undef, !88, %Constants undef, !92, %AccelerationStructureList undef, !101, %"class.StructuredBuffer<Vertex>" undef, !103, %struct.Vertex undef, !104, %g_cubeCB undef, !107, %struct.CubeConstantBuffer undef, !109}
!28 = !{i32 116, !29, !30, !31}
!29 = !{i32 6, !"instanceDesc", i32 3, i32 0}
!30 = !{i32 6, !"ObjectToWorld", i32 3, i32 64, i32 7, i32 9}
!31 = !{i32 6, !"InstanceIndex", i32 3, i32 112, i32 7, i32 5}
!32 = !{i32 64, !33, !34, !35, !36}
!33 = !{i32 6, !"Transform", i32 3, i32 0, i32 7, i32 9}
!34 = !{i32 6, !"InstanceIDAndMask", i32 3, i32 48, i32 7, i32 5}
!35 = !{i32 6, !"InstanceContributionToHitGroupIndexAndFlags", i32 3, i32 52, i32 7, i32 5}
!36 = !{i32 6, !"AccelerationStructure", i32 3, i32 56, i32 7, i32 5}
!37 = !{i32 4, !38}
!38 = !{i32 6, !"h", i32 3, i32 0, i32 7, i32 4}
!39 = !{i32 28, !40, !41}
!40 = !{i32 6, !"center", i32 3, i32 0, i32 7, i32 9}
!41 = !{i32 6, !"halfDim", i32 3, i32 16, i32 7, i32 9}
!42 = !{i32 8, !43, !44}
!43 = !{i32 6, !"buffer", i32 3, i32 0}
!44 = !{i32 6, !"offsetInBytes", i32 3, i32 4, i32 7, i32 5}
!45 = !{i32 12, !46}
!46 = !{i32 6, !"normal", i32 3, i32 0, i32 7, i32 9}
!47 = !{i32 4, !48}
!48 = !{i32 6, !"hit", i32 3, i32 0, i32 7, i32 1}
!49 = !{i32 28, !50, !51}
!50 = !{i32 6, !"min", i32 3, i32 0, i32 7, i32 9}
!51 = !{i32 6, !"max", i32 3, i32 16, i32 7, i32 9}
!52 = !{i32 52, !53, !54, !55, !56}
!53 = !{i32 6, !"PrimitiveType", i32 3, i32 0, i32 7, i32 5}
!54 = !{i32 6, !"data0", i32 3, i32 16, i32 7, i32 5}
!55 = !{i32 6, !"data1", i32 3, i32 32, i32 7, i32 5}
!56 = !{i32 6, !"data2", i32 3, i32 48, i32 7, i32 5}
!57 = !{i32 44, !58, !59, !60}
!58 = !{i32 6, !"v0", i32 3, i32 0, i32 7, i32 9}
!59 = !{i32 6, !"v1", i32 3, i32 16, i32 7, i32 9}
!60 = !{i32 6, !"v2", i32 3, i32 32, i32 7, i32 9}
!61 = !{i32 92, !62, !63, !64, !65, !66, !67}
!62 = !{i32 6, !"Origin", i32 3, i32 0, i32 7, i32 9}
!63 = !{i32 6, !"Direction", i32 3, i32 16, i32 7, i32 9}
!64 = !{i32 6, !"InverseDirection", i32 3, i32 32, i32 7, i32 9}
!65 = !{i32 6, !"OriginTimesRayInverseDirection", i32 3, i32 48, i32 7, i32 9}
!66 = !{i32 6, !"Shear", i32 3, i32 64, i32 7, i32 9}
!67 = !{i32 6, !"SwizzledIndices", i32 3, i32 80, i32 7, i32 4}
!68 = !{i32 16, !69}
!69 = !{i32 6, !"color", i32 3, i32 0, i32 7, i32 9}
!70 = !{i32 8, !71, !72}
!71 = !{i32 6, !"GeometryContributionToHitGroupIndex", i32 3, i32 0, i32 7, i32 5}
!72 = !{i32 6, !"PrimitiveIndex", i32 3, i32 4, i32 7, i32 5}
!73 = !{i32 8, !74}
!74 = !{i32 6, !"barycentrics", i32 3, i32 0, i32 7, i32 9}
!75 = !{i32 128, !76}
!76 = !{i32 6, !"g_sceneCB", i32 3, i32 0}
!77 = !{i32 128, !78, !80, !81, !82, !83}
!78 = !{i32 6, !"projectionToWorld", i32 2, !79, i32 3, i32 0, i32 7, i32 9}
!79 = !{i32 4, i32 4, i32 1}
!80 = !{i32 6, !"cameraPosition", i32 3, i32 64, i32 7, i32 9}
!81 = !{i32 6, !"lightPosition", i32 3, i32 80, i32 7, i32 9}
!82 = !{i32 6, !"lightAmbientColor", i32 3, i32 96, i32 7, i32 9}
!83 = !{i32 6, !"lightDiffuseColor", i32 3, i32 112, i32 7, i32 9}
!84 = !{i32 16, !85}
!85 = !{i32 6, !"h", i32 3, i32 0, i32 7, i32 9}
!86 = !{i32 80, !87}
!87 = !{i32 6, !"h", i32 3, i32 0}
!88 = !{i32 80, !89, !90, !91}
!89 = !{i32 6, !"bottomLevelASToLocalSpace", i32 2, !79, i32 3, i32 0, i32 7, i32 9}
!90 = !{i32 6, !"albedo", i32 3, i32 64, i32 7, i32 9}
!91 = !{i32 6, !"padding", i32 3, i32 76, i32 7, i32 9}
!92 = !{i32 32, !93, !94, !95, !96, !97, !98, !99, !100}
!93 = !{i32 6, !"RayDispatchDimensionsWidth", i32 3, i32 0, i32 7, i32 5}
!94 = !{i32 6, !"RayDispatchDimensionsHeight", i32 3, i32 4, i32 7, i32 5}
!95 = !{i32 6, !"HitGroupShaderRecordStride", i32 3, i32 8, i32 7, i32 5}
!96 = !{i32 6, !"MissShaderRecordStride", i32 3, i32 12, i32 7, i32 5}
!97 = !{i32 6, !"SamplerDescriptorHeapStartLo", i32 3, i32 16, i32 7, i32 5}
!98 = !{i32 6, !"SamplerDescriptorHeapStartHi", i32 3, i32 20, i32 7, i32 5}
!99 = !{i32 6, !"SrvCbvUavDescriptorHeapStartLo", i32 3, i32 24, i32 7, i32 5}
!100 = !{i32 6, !"SrvCbvUavDescriptorHeapStartHi", i32 3, i32 28, i32 7, i32 5}
!101 = !{i32 8, !102}
!102 = !{i32 6, !"TopLevelAccelerationStructureGpuVA", i32 3, i32 0, i32 7, i32 5}
!103 = !{i32 28, !87}
!104 = !{i32 28, !105, !106}
!105 = !{i32 6, !"position", i32 3, i32 0, i32 7, i32 9}
!106 = !{i32 6, !"normal", i32 3, i32 16, i32 7, i32 9}
!107 = !{i32 16, !108}
!108 = !{i32 6, !"g_cubeCB", i32 3, i32 0}
!109 = !{i32 16, !110}
!110 = !{i32 6, !"diffuseColor", i32 3, i32 0, i32 7, i32 9}
!111 = !{i32 1, <2 x i32> (<2 x i32>, i32)* @"\01?PointerAdd@@YA?AV?$vector@I$01@@V1@I@Z", !112, void ()* @"\01?LogTraceRayStart@@YAXXZ", !117, void (i32, i32*, float, i32*, float, float*)* @"\01?RecordClosestBox@@YAXIAIA_NM0MAIAM@Z", !119, void (%struct.BVHMetadata*, %struct.RWByteAddressBuffer*, i32)* @"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z", !125, i1 (i1, i32, i32)* @"\01?IsOpaque@@YA_N_NII@Z", !127, void (%struct.BoundingBox*, %struct.RWByteAddressBuffer*, i32, i32)* @"\01?GetBoxFromBuffer@@YA?AUBoundingBox@@URWByteAddressBuffer@@II@Z", !130, float (%class.matrix.float.3.4)* @"\01?Determinant@@YAMV?$matrix@M$02$03@@@Z", !131, float (i32, i32)* @"\01?ComputeCullFaceDir@@YAMII@Z", !136, void ()* @"\01?MyRaygenShader@@YAXXZ", !117, i32 (float, i32)* @"\01?Fallback_ReportHit@@YAHMI@Z", !137, void (%struct.RaytracingInstanceDesc*, %struct.ByteAddressBuffer*, i32)* @"\01?LoadRaytracingInstanceDesc@@YA?AURaytracingInstanceDesc@@UByteAddressBuffer@@I@Z", !125, void (%struct.BoundingBox*, <3 x float>, <3 x float>, <3 x float>, i32, <2 x i32>*)* @"\01?GetBoxDataFromTriangle@@YA?AUBoundingBox@@V?$vector@M$02@@00HAIAV?$vector@I$01@@@Z", !140, void (%struct.BoundingBox*, %struct.RWByteAddressBufferPointer*, i32, <2 x i32>*)* @"\01?BVHReadBoundingBox@@YA?AUBoundingBox@@URWByteAddressBufferPointer@@HAIAV?$vector@I$01@@@Z", !142, i1 (i1, i32)* @"\01?Cull@@YA_N_NI@Z", !143, i1 (<3 x float>, <3 x float>, float, float, float*, %struct.ProceduralPrimitiveAttributes*)* @"\01?IntersectCustomPrimitiveFrontToBack@@YA_NV?$vector@M$02@@0MMAIAMUProceduralPrimitiveAttributes@@@Z", !144, void (%struct.RaytracingInstanceDesc*, <4 x i32>, <4 x i32>, <4 x i32>, <4 x i32>)* @"\01?RawDataToRaytracingInstanceDesc@@YA?AURaytracingInstanceDesc@@V?$vector@I$03@@000@Z", !145, <2 x i32> (i32, i32)* @"\01?CreateFlag@@YA?AV?$vector@I$01@@II@Z", !112, void (%struct.ShadowPayload*, %struct.ProceduralPrimitiveAttributes*)* @"\01?MyClosestHitShader_ShadowAABB@@YAXUShadowPayload@@UProceduralPrimitiveAttributes@@@Z", !146, void (i32*, i32, i32, i32)* @"\01?StackPush@@YAXAIAHIII@Z", !151, void (%struct.BoundingBox*, <2 x i32>)* @"\01?dump@@YAXUBoundingBox@@V?$vector@I$01@@@Z", !153, i1 (<3 x float>, <3 x float>, float*, %struct.ProceduralPrimitiveAttributes*)* @"\01?intersectSpheres@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@@Z", !154, i32 (i32*, i32*, i32)* @"\01?StackPop@@YAIAIAHAIAII@Z", !156, %class.matrix.float.3.4 (%class.matrix.float.3.4)* @"\01?InverseAffineTransform@@YA?AV?$matrix@M$02$03@@V1@@Z", !157, void (%struct.RWByteAddressBufferPointer*, %struct.RWByteAddressBuffer*, i32)* @"\01?CreateRWByteAddressBufferPointer@@YA?AURWByteAddressBufferPointer@@URWByteAddressBuffer@@I@Z", !125, <3 x float> ([3 x <3 x float>]*, <2 x float>)* @"\01?HitAttribute@@YA?AV?$vector@M$02@@Y02V1@V?$vector@M$01@@@Z", !159, void (i32, i32, i32, i32, i32, float, float, float, float, float, float, float, float, i32)* @"\01?Fallback_TraceRay@@YAXIIIIIMMMMMMMMI@Z", !160, i32 (<3 x float>)* @"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z", !161, void ()* @"\01?Fallback_IgnoreHit@@YAXXZ", !117, void (%struct.RWByteAddressBufferPointer*, <3 x float>*, <3 x float>*, <3 x float>*, i32)* @"\01?BVHReadTriangle@@YAXURWByteAddressBufferPointer@@AIAV?$vector@M$02@@11I@Z", !162, void (<4 x i32>)* @"\01?Log@@YAXV?$vector@I$03@@@Z", !163, void (<3 x float>, <3 x float>, %class.matrix.float.3.4, %class.matrix.float.3.4)* @"\01?UpdateObjectSpaceProperties@@YAXV?$vector@M$02@@0V?$matrix@M$02$03@@1@Z", !164, void (%struct.BoundingBox*, <2 x i32>, <4 x i32>*, <4 x i32>*)* @"\01?CompressBox@@YAXUBoundingBox@@V?$vector@I$01@@AIAV?$vector@I$03@@2@Z", !165, void (i32)* @"\01?LogNoData@@YAXI@Z", !163, void (%struct.RWByteAddressBufferPointer*, <2 x i32>)* @"\01?CreateRWByteAddressBufferPointerFromGpuVA@@YA?AURWByteAddressBufferPointer@@V?$vector@I$01@@@Z", !166, void (%struct.BoundingBox*, %struct.AABB*)* @"\01?AABBtoBoundingBox@@YA?AUBoundingBox@@UAABB@@@Z", !167, i1 (<3 x float>, <3 x float>, float*, %struct.ProceduralPrimitiveAttributes*, <3 x float>, float)* @"\01?intersectSphere@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@0M@Z", !168, <3 x float> (%struct.BoundingBox*)* @"\01?GetMinCorner@@YA?AV?$vector@M$02@@UBoundingBox@@@Z", !169, void (%struct.RaytracingInstanceDesc*, %struct.RWByteAddressBuffer*, i32)* @"\01?LoadRaytracingInstanceDesc@@YA?AURaytracingInstanceDesc@@URWByteAddressBuffer@@I@Z", !125, <3 x float> (%struct.BoundingBox*)* @"\01?GetMaxCorner@@YA?AV?$vector@M$02@@UBoundingBox@@@Z", !169, void (%struct.ShadowPayload*)* @"\01?MyMissShader_Shadow@@YAXUShadowPayload@@@Z", !170, i1 (<2 x i32>)* @"\01?IsProceduralGeometry@@YA_NV?$vector@I$01@@@Z", !171, void ()* @"\01?IntersectionShader_Box@@YAXXZ", !117, i32 (%struct.RaytracingInstanceDesc*)* @"\01?GetInstanceContributionToHitGroupIndex@@YAIURaytracingInstanceDesc@@@Z", !172, void ()* @"\01?LogTraceRayEnd@@YAXXZ", !117, void (%struct.AABB*, %struct.Primitive*)* @"\01?GetProceduralPrimitiveAABB@@YA?AUAABB@@UPrimitive@@@Z", !167, void ()* @"\01?MyIntersectionShader_AABB@@YAXXZ", !117, void (%struct.Triangle*, <4 x i32>, <4 x i32>, i32)* @"\01?RawDataToTriangle@@YA?AUTriangle@@V?$vector@I$03@@0I@Z", !173, i1 (<2 x i32>)* @"\01?IsLeaf@@YA_NV?$vector@I$01@@@Z", !171, void (%struct.Triangle*, %struct.Primitive*)* @"\01?GetTriangle@@YA?AUTriangle@@UPrimitive@@@Z", !167, void (%struct.RayData*, <3 x float>, <3 x float>)* @"\01?GetRayData@@YA?AURayData@@V?$vector@M$02@@0@Z", !174, void (<2 x i32>)* @"\01?LogInt2@@YAXV?$vector@H$01@@@Z", !175, i32 (i32, i32)* @"\01?GetBoxAddress@@YAIII@Z", !112, i32 (i32)* @"\01?InvokeAnyHit@@YAHH@Z", !176, void (%struct.Primitive*, %struct.AABB*)* @"\01?CreateProceduralGeometryPrimitive@@YA?AUPrimitive@@UAABB@@@Z", !167, i1 (%struct.RWByteAddressBufferPointer*, <2 x i32>, i32, <3 x float>, <3 x float>, <3 x i32>, <3 x float>, <2 x float>*, float*, i32*)* @"\01?TestLeafNodeIntersections@@YA_NURWByteAddressBufferPointer@@V?$vector@I$01@@IV?$vector@M$02@@2V?$vector@H$02@@2AIAV?$vector@M$01@@AIAMAIAI@Z", !177, i1 (<3 x float>, <3 x float>, float*, %struct.ProceduralPrimitiveAttributes*)* @"\01?intersectBox@@YA_NV?$vector@M$02@@0AIAMUProceduralPrimitiveAttributes@@@Z", !179, void (%struct.ByteAddressBuffer*, i32, i32*, i32*)* @"\01?GetAnyHitAndIntersectionStateId@@YAXUByteAddressBuffer@@IAIAI1@Z", !165, void (%struct.RWByteAddressBuffer*, i32, i32, <2 x i32>)* @"\01?WriteOnlyFlagToBuffer@@YAXURWByteAddressBuffer@@IIV?$vector@I$01@@@Z", !180, void (%struct.Primitive*, %struct.Triangle*)* @"\01?CreateTrianglePrimitive@@YA?AUPrimitive@@UTriangle@@@Z", !167, %class.matrix.float.3.4 ([3 x <4 x float>]*)* @"\01?CreateMatrix@@YA?AV?$matrix@M$02$03@@Y02V?$vector@M$03@@@Z", !181, i32 (<2 x i32>)* @"\01?GetLeftNodeIndex@@YAIV?$vector@I$01@@@Z", !182, i32 (%struct.ByteAddressBuffer*, i32)* @"\01?GetAnyHitStateId@@YAIUByteAddressBuffer@@I@Z", !183, i32 (<2 x i32>)* @"\01?GetRightNodeIndex@@YAIV?$vector@I$01@@@Z", !182, i32 (%struct.RaytracingInstanceDesc*)* @"\01?GetInstanceID@@YAIURaytracingInstanceDesc@@@Z", !172, void (i32*, i1, i32, i32, i32, i32)* @"\01?StackPush2@@YAXAIAH_NIIII@Z", !184, i32 (%struct.RaytracingInstanceDesc*)* @"\01?GetInstanceMask@@YAIURaytracingInstanceDesc@@@Z", !172, void (%struct.Triangle*, <4 x i32>*, <4 x i32>*, i32*)* @"\01?TriangleToRawData@@YAXUTriangle@@AIAV?$vector@I$03@@1AIAI@Z", !185, void (%struct.AABB*, <4 x i32>, <4 x i32>)* @"\01?RawDataToAABB@@YA?AUAABB@@V?$vector@H$03@@0@Z", !186, void (%struct.HitData*, %struct.ProceduralPrimitiveAttributes*)* @"\01?MyClosestHitShader_AABB@@YAXUHitData@@UProceduralPrimitiveAttributes@@@Z", !146, void ()* @main, !187, void (%struct.AABB*, %struct.BoundingBox*)* @"\01?BoundingBoxToAABB@@YA?AUAABB@@UBoundingBox@@@Z", !167, void (float)* @"\01?LogFloat@@YAXM@Z", !188, void (%struct.PrimitiveMetaData*, %struct.RWByteAddressBufferPointer*, i32)* @"\01?BVHReadPrimitiveMetaData@@YA?AUPrimitiveMetaData@@URWByteAddressBufferPointer@@H@Z", !189, void (%struct.BoundingBox*, %struct.BoundingBox*, i32, %struct.BoundingBox*, i32, <2 x i32>*)* @"\01?GetBoxFromChildBoxes@@YA?AUBoundingBox@@U1@H0HAIAV?$vector@I$01@@@Z", !190, i1 (i32, i32)* @"\01?GetBoolFlag@@YA_NII@Z", !191, i32 (%struct.RaytracingInstanceDesc*)* @"\01?GetInstanceFlags@@YAIURaytracingInstanceDesc@@@Z", !172, i32 (%struct.RWByteAddressBufferPointer*)* @"\01?GetOffsetToVertices@@YAHURWByteAddressBufferPointer@@@Z", !192, void (%struct.AABB*, <4 x i32>*, <2 x i32>*)* @"\01?AABBToRawData@@YAXUAABB@@AIAV?$vector@I$03@@AIAV?$vector@I$01@@@Z", !193, void (i32)* @"\01?LogInt@@YAXH@Z", !175, void (<3 x i32>)* @"\01?LogInt3@@YAXV?$vector@H$02@@@Z", !175, void (i32*, i32, i1)* @"\01?SetBoolFlag@@YAXAIAII_N@Z", !194, i1 (i32, i32, i32)* @"\01?Traverse@@YA_NIII@Z", !195, <3 x float> (<3 x float>, <3 x i32>)* @"\01?Swizzle@@YA?AV?$vector@M$02@@V1@V?$vector@H$02@@@Z", !196, <4 x float> (<3 x float>, <3 x float>)* @"\01?CalculateDiffuseLighting@@YA?AV?$vector@M$03@@V?$vector@M$02@@0@Z", !159, void (%struct.Primitive*)* @"\01?NullPrimitive@@YA?AUPrimitive@@XZ", !197, void (float*, float*)* @"\01?swap@@YAXAIAM0@Z", !198, i32 (i32, i32)* @"\01?GetPrimitiveMetaDataAddress@@YAIII@Z", !112, void (%struct.AABB*, <4 x i32>, <2 x i32>)* @"\01?RawDataToAABB@@YA?AUAABB@@V?$vector@I$03@@V?$vector@I$01@@@Z", !199, void (%struct.RWByteAddressBuffer*, i32, %struct.BVHMetadata*)* @"\01?StoreBVHMetadataToRawData@@YAXURWByteAddressBuffer@@IUBVHMetadata@@@Z", !200, i32 (<2 x i32>)* @"\01?GetLeafIndexFromFlag@@YAHV?$vector@I$01@@@Z", !201, void (<3 x float>)* @"\01?LogFloat3@@YAXV?$vector@M$02@@@Z", !188, i1 (float, float, float, float*, float*)* @"\01?solveQuadratic@@YA_NMMMAIAM0@Z", !202, void (%struct.HitData*)* @"\01?MyMissShader@@YAXUHitData@@@Z", !170, void (%struct.RWByteAddressBuffer*, <2 x i32>)* @"\01?PointerGetBuffer@@YA?AURWByteAddressBuffer@@V?$vector@I$01@@@Z", !166, void (%struct.BoundingBox*, <4 x i32>, <4 x i32>, <2 x i32>*)* @"\01?RawDataToBoundingBox@@YA?AUBoundingBox@@V?$vector@H$03@@0AIAV?$vector@I$01@@@Z", !203, void (%struct.AABB*, %struct.AABB*, %class.matrix.float.3.4)* @"\01?TransformAABB@@YA?AUAABB@@U1@V?$matrix@M$02$03@@@Z", !204, <4 x i32> (<2 x i32>)* @"\01?Load4@@YA?AV?$vector@I$03@@V?$vector@I$01@@@Z", !182, i32 (<2 x i32>)* @"\01?PointerGetBufferStartOffset@@YAIV?$vector@I$01@@@Z", !182, <3 x i32> (i32)* @"\01?Load3x16BitIndices@@YA?AV?$vector@I$02@@I@Z", !182, void ()* @"\01?Fallback_AcceptHitAndEndSearch@@YAXXZ", !117, void (i32*, i32*)* @"\01?swap@@YAXAIAH0@Z", !205, <3 x float> ()* @"\01?HitWorldPosition@@YA?AV?$vector@M$02@@XZ", !206, void (%struct.RWByteAddressBuffer*, i32, i32, %struct.BoundingBox*, <2 x i32>)* @"\01?WriteBoxToBuffer@@YAXURWByteAddressBuffer@@IIUBoundingBox@@V?$vector@I$01@@@Z", !207, i32 (%struct.RWByteAddressBufferPointer*)* @"\01?GetOffsetToPrimitiveMetaData@@YAHURWByteAddressBufferPointer@@@Z", !192, void (%struct.HitData*, %struct.BuiltInTriangleIntersectionAttributes*)* @"\01?MyClosestHitShader_Triangle@@YAXUHitData@@UBuiltInTriangleIntersectionAttributes@@@Z", !146, void (<3 x i32>, i32)* @"\01?main@@YAXV?$vector@I$02@@I@Z", !208, i32 (%struct.RWByteAddressBufferPointer*)* @"\01?GetOffsetToBoxes@@YAHURWByteAddressBufferPointer@@@Z", !192}
!112 = !{!113, !116, !116}
!113 = !{i32 1, !114, !115}
!114 = !{i32 7, i32 5}
!115 = !{}
!116 = !{i32 0, !114, !115}
!117 = !{!118}
!118 = !{i32 1, !115, !115}
!119 = !{!118, !116, !120, !122, !120, !122, !124}
!120 = !{i32 2, !121, !115}
!121 = !{i32 7, i32 1}
!122 = !{i32 0, !123, !115}
!123 = !{i32 7, i32 9}
!124 = !{i32 2, !123, !115}
!125 = !{!126, !118, !126, !116}
!126 = !{i32 0, !115, !115}
!127 = !{!128, !129, !116, !116}
!128 = !{i32 1, !121, !115}
!129 = !{i32 0, !121, !115}
!130 = !{!126, !118, !126, !116, !116}
!131 = !{!132, !133}
!132 = !{i32 1, !123, !115}
!133 = !{i32 0, !134, !115}
!134 = !{i32 2, !135, i32 7, i32 9}
!135 = !{i32 3, i32 4, i32 1}
!136 = !{!132, !116, !116}
!137 = !{!138, !122, !116}
!138 = !{i32 1, !139, !115}
!139 = !{i32 7, i32 4}
!140 = !{!126, !118, !122, !122, !122, !141, !113}
!141 = !{i32 0, !139, !115}
!142 = !{!126, !118, !126, !141, !113}
!143 = !{!128, !129, !116}
!144 = !{!128, !122, !122, !122, !122, !124, !118}
!145 = !{!126, !118, !116, !116, !116, !116}
!146 = !{!118, !147, !149}
!147 = !{i32 2, !148, !115}
!148 = !{i32 4, !"SV_RayPayload"}
!149 = !{i32 0, !150, !115}
!150 = !{i32 4, !"SV_IntersectionAttributes"}
!151 = !{!118, !152, !116, !116, !116}
!152 = !{i32 2, !139, !115}
!153 = !{!118, !126, !116}
!154 = !{!128, !122, !122, !124, !155}
!155 = !{i32 2, !115, !115}
!156 = !{!113, !152, !113, !116}
!157 = !{!158, !133}
!158 = !{i32 1, !134, !115}
!159 = !{!132, !122, !122}
!160 = !{!118, !116, !116, !116, !116, !116, !122, !122, !122, !122, !122, !122, !122, !122, !116}
!161 = !{!138, !122}
!162 = !{!118, !126, !132, !132, !132, !116}
!163 = !{!118, !116}
!164 = !{!118, !122, !122, !133, !133}
!165 = !{!118, !126, !116, !113, !113}
!166 = !{!126, !118, !116}
!167 = !{!126, !118, !126}
!168 = !{!128, !122, !122, !124, !155, !122, !122}
!169 = !{!132, !126}
!170 = !{!118, !147}
!171 = !{!128, !116}
!172 = !{!113, !126}
!173 = !{!126, !118, !116, !116, !116}
!174 = !{!126, !118, !122, !122}
!175 = !{!118, !141}
!176 = !{!138, !141}
!177 = !{!128, !126, !116, !116, !122, !122, !141, !122, !124, !124, !178}
!178 = !{i32 2, !114, !115}
!179 = !{!128, !122, !122, !132, !155}
!180 = !{!118, !126, !116, !116, !116}
!181 = !{!158, !122}
!182 = !{!113, !116}
!183 = !{!113, !126, !116}
!184 = !{!118, !152, !129, !116, !116, !116, !116}
!185 = !{!118, !126, !113, !113, !113}
!186 = !{!126, !118, !141, !141}
!187 = !{!126}
!188 = !{!118, !122}
!189 = !{!126, !118, !126, !141}
!190 = !{!126, !118, !126, !141, !126, !141, !113}
!191 = !{!128, !116, !116}
!192 = !{!138, !126}
!193 = !{!118, !126, !113, !113}
!194 = !{!118, !178, !116, !129}
!195 = !{!128, !116, !116, !116}
!196 = !{!132, !122, !141}
!197 = !{!126, !118}
!198 = !{!118, !124, !124}
!199 = !{!126, !118, !116, !116}
!200 = !{!118, !126, !116, !126}
!201 = !{!138, !116}
!202 = !{!128, !122, !122, !122, !132, !132}
!203 = !{!126, !118, !141, !141, !113}
!204 = !{!126, !118, !126, !133}
!205 = !{!118, !152, !152}
!206 = !{!132}
!207 = !{!118, !126, !116, !116, !126, !116}
!208 = !{!118, !116, !116}
!209 = !{null, !"", null, !6, null}
!210 = !{void ()* @"\01?IntersectionShader_Box@@YAXXZ", i32 8}
!211 = !{void (%struct.HitData*, %struct.ProceduralPrimitiveAttributes*)* @"\01?MyClosestHitShader_AABB@@YAXUHitData@@UProceduralPrimitiveAttributes@@@Z", i32 10, i32 16, i32 12}
!212 = !{void (%struct.ShadowPayload*, %struct.ProceduralPrimitiveAttributes*)* @"\01?MyClosestHitShader_ShadowAABB@@YAXUShadowPayload@@UProceduralPrimitiveAttributes@@@Z", i32 10, i32 4, i32 12}
!213 = !{void (%struct.HitData*, %struct.BuiltInTriangleIntersectionAttributes*)* @"\01?MyClosestHitShader_Triangle@@YAXUHitData@@UBuiltInTriangleIntersectionAttributes@@@Z", i32 10, i32 16, i32 8}
!214 = !{void ()* @"\01?MyIntersectionShader_AABB@@YAXXZ", i32 8}
!215 = !{void (%struct.HitData*)* @"\01?MyMissShader@@YAXUHitData@@@Z", i32 11, i32 16}
!216 = !{void (%struct.ShadowPayload*)* @"\01?MyMissShader_Shadow@@YAXUShadowPayload@@@Z", i32 11, i32 4}
!217 = !{void ()* @"\01?MyRaygenShader@@YAXXZ", i32 7}
!218 = !{void ()* @main, i32 5, i32 8, i32 8, i32 1}
!219 = !{void ()* @main, null}
!220 = !{!221, !221, i64 0}
!221 = !{!"omnipotent char", !222, i64 0}
!222 = !{!"Simple C/C++ TBAA"}
!223 = !{!224, !224, i64 0}
!224 = !{!"int", !221, i64 0}
!225 = !{!226}
!226 = distinct !{!226, !227, !"\01?RawDataToRaytracingInstanceDesc@@YA?AURaytracingInstanceDesc@@V?$vector@I$03@@000@Z: %agg.result"}
!227 = distinct !{!227, !"\01?RawDataToRaytracingInstanceDesc@@YA?AURaytracingInstanceDesc@@V?$vector@I$03@@000@Z"}
!228 = !{!229, !229, i64 0}
!229 = !{!"float", !221, i64 0}
!230 = !{!231}
!231 = distinct !{!231, !232, !"\01?GetRayData@@YA?AURayData@@V?$vector@M$02@@0@Z: %agg.result"}
!232 = distinct !{!232, !"\01?GetRayData@@YA?AURayData@@V?$vector@M$02@@0@Z"}
!233 = !{i32 1}
!234 = !{!235}
!235 = distinct !{!235, !236, !"\01?StackPush@@YAXAIAHIII@Z: %stackTop"}
!236 = distinct !{!236, !"\01?StackPush@@YAXAIAHIII@Z"}
!237 = !{!238}
!238 = distinct !{!238, !239, !"\01?StackPop@@YAIAIAHAIAII@Z: %stackTop"}
!239 = distinct !{!239, !"\01?StackPop@@YAIAIAHAIAII@Z"}
!240 = !{!241, !243}
!241 = distinct !{!241, !242, !"\01?PointerGetBuffer@@YA?AURWByteAddressBuffer@@V?$vector@I$01@@@Z: %agg.result"}
!242 = distinct !{!242, !"\01?PointerGetBuffer@@YA?AURWByteAddressBuffer@@V?$vector@I$01@@@Z"}
!243 = distinct !{!243, !244, !"\01?CreateRWByteAddressBufferPointerFromGpuVA@@YA?AURWByteAddressBufferPointer@@V?$vector@I$01@@@Z: %agg.result"}
!244 = distinct !{!244, !"\01?CreateRWByteAddressBufferPointerFromGpuVA@@YA?AURWByteAddressBufferPointer@@V?$vector@I$01@@@Z"}
!245 = !{!246}
!246 = distinct !{!246, !247, !"\01?StackPush@@YAXAIAHIII@Z: %stackTop"}
!247 = distinct !{!247, !"\01?StackPush@@YAXAIAHIII@Z"}
!248 = !{!249}
!249 = distinct !{!249, !250, !"\01?GetRayData@@YA?AURayData@@V?$vector@M$02@@0@Z: %agg.result"}
!250 = distinct !{!250, !"\01?GetRayData@@YA?AURayData@@V?$vector@M$02@@0@Z"}
!251 = !{!252, !254}
!252 = distinct !{!252, !253, !"\01?PointerGetBuffer@@YA?AURWByteAddressBuffer@@V?$vector@I$01@@@Z: %agg.result"}
!253 = distinct !{!253, !"\01?PointerGetBuffer@@YA?AURWByteAddressBuffer@@V?$vector@I$01@@@Z"}
!254 = distinct !{!254, !255, !"\01?CreateRWByteAddressBufferPointerFromGpuVA@@YA?AURWByteAddressBufferPointer@@V?$vector@I$01@@@Z: %agg.result"}
!255 = distinct !{!255, !"\01?CreateRWByteAddressBufferPointerFromGpuVA@@YA?AURWByteAddressBufferPointer@@V?$vector@I$01@@@Z"}
!256 = !{!257, !259, !260, !262, !263}
!257 = distinct !{!257, !258, !"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z: %hitT"}
!258 = distinct !{!258, !"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z"}
!259 = distinct !{!259, !258, !"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z: %bary"}
!260 = distinct !{!260, !261, !"\01?TestLeafNodeIntersections@@YA_NURWByteAddressBufferPointer@@V?$vector@I$01@@IV?$vector@M$02@@2V?$vector@H$02@@2AIAV?$vector@M$01@@AIAMAIAI@Z: %resultBary"}
!261 = distinct !{!261, !"\01?TestLeafNodeIntersections@@YA_NURWByteAddressBufferPointer@@V?$vector@I$01@@IV?$vector@M$02@@2V?$vector@H$02@@2AIAV?$vector@M$01@@AIAMAIAI@Z"}
!262 = distinct !{!262, !261, !"\01?TestLeafNodeIntersections@@YA_NURWByteAddressBufferPointer@@V?$vector@I$01@@IV?$vector@M$02@@2V?$vector@H$02@@2AIAV?$vector@M$01@@AIAMAIAI@Z: %resultT"}
!263 = distinct !{!263, !261, !"\01?TestLeafNodeIntersections@@YA_NURWByteAddressBufferPointer@@V?$vector@I$01@@IV?$vector@M$02@@2V?$vector@H$02@@2AIAV?$vector@M$01@@AIAMAIAI@Z: %resultTriId"}
!264 = !{!265}
!265 = distinct !{!265, !266, !"\01?StackPush2@@YAXAIAH_NIIII@Z: %stackTop"}
!266 = distinct !{!266, !"\01?StackPush2@@YAXAIAH_NIIII@Z"}
!267 = !{!268}
!268 = distinct !{!268, !269, !"\01?StackPush@@YAXAIAHIII@Z: %stackTop"}
!269 = distinct !{!269, !"\01?StackPush@@YAXAIAHIII@Z"}
!270 = !{!271}
!271 = distinct !{!271, !272, !"\01?GetRayData@@YA?AURayData@@V?$vector@M$02@@0@Z: %agg.result"}
!272 = distinct !{!272, !"\01?GetRayData@@YA?AURayData@@V?$vector@M$02@@0@Z"}
!273 = !{!274}
!274 = distinct !{!274, !275, !"\01?CreateRWByteAddressBufferPointer@@YA?AURWByteAddressBufferPointer@@URWByteAddressBuffer@@I@Z: %agg.result"}
!275 = distinct !{!275, !"\01?CreateRWByteAddressBufferPointer@@YA?AURWByteAddressBufferPointer@@URWByteAddressBuffer@@I@Z"}
!276 = !{!277}
!277 = distinct !{!277, !278, !"\01?RawDataToRaytracingInstanceDesc@@YA?AURaytracingInstanceDesc@@V?$vector@I$03@@000@Z: %agg.result"}
!278 = distinct !{!278, !"\01?RawDataToRaytracingInstanceDesc@@YA?AURaytracingInstanceDesc@@V?$vector@I$03@@000@Z"}
!279 = !{!280}
!280 = distinct !{!280, !281, !"\01?RawDataToAABB@@YA?AUAABB@@V?$vector@I$03@@V?$vector@I$01@@@Z: %agg.result"}
!281 = distinct !{!281, !"\01?RawDataToAABB@@YA?AUAABB@@V?$vector@I$03@@V?$vector@I$01@@@Z"}
!282 = !{!283}
!283 = distinct !{!283, !284, !"\01?RawDataToTriangle@@YA?AUTriangle@@V?$vector@I$03@@0I@Z: %agg.result"}
!284 = distinct !{!284, !"\01?RawDataToTriangle@@YA?AUTriangle@@V?$vector@I$03@@0I@Z"}
!285 = !{!286, !288}
!286 = distinct !{!286, !287, !"\01?AABBToRawData@@YAXUAABB@@AIAV?$vector@I$03@@AIAV?$vector@I$01@@@Z: %a"}
!287 = distinct !{!287, !"\01?AABBToRawData@@YAXUAABB@@AIAV?$vector@I$03@@AIAV?$vector@I$01@@@Z"}
!288 = distinct !{!288, !287, !"\01?AABBToRawData@@YAXUAABB@@AIAV?$vector@I$03@@AIAV?$vector@I$01@@@Z: %b"}
!289 = !{!290, !292, !293}
!290 = distinct !{!290, !291, !"\01?BVHReadTriangle@@YAXURWByteAddressBufferPointer@@AIAV?$vector@M$02@@11I@Z: %v0"}
!291 = distinct !{!291, !"\01?BVHReadTriangle@@YAXURWByteAddressBufferPointer@@AIAV?$vector@M$02@@11I@Z"}
!292 = distinct !{!292, !291, !"\01?BVHReadTriangle@@YAXURWByteAddressBufferPointer@@AIAV?$vector@M$02@@11I@Z: %v1"}
!293 = distinct !{!293, !291, !"\01?BVHReadTriangle@@YAXURWByteAddressBufferPointer@@AIAV?$vector@M$02@@11I@Z: %v2"}
!294 = !{!295, !297}
!295 = distinct !{!295, !296, !"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z: %hitT"}
!296 = distinct !{!296, !"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z"}
!297 = distinct !{!297, !296, !"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z: %bary"}
!298 = !{!299, !301, !302}
!299 = distinct !{!299, !300, !"\01?TriangleToRawData@@YAXUTriangle@@AIAV?$vector@I$03@@1AIAI@Z: %a"}
!300 = distinct !{!300, !"\01?TriangleToRawData@@YAXUTriangle@@AIAV?$vector@I$03@@1AIAI@Z"}
!301 = distinct !{!301, !300, !"\01?TriangleToRawData@@YAXUTriangle@@AIAV?$vector@I$03@@1AIAI@Z: %b"}
!302 = distinct !{!302, !300, !"\01?TriangleToRawData@@YAXUTriangle@@AIAV?$vector@I$03@@1AIAI@Z: %c"}
!303 = !{!304}
!304 = distinct !{!304, !305, !"\01?BoundingBoxToAABB@@YA?AUAABB@@UBoundingBox@@@Z: %agg.result"}
!305 = distinct !{!305, !"\01?BoundingBoxToAABB@@YA?AUAABB@@UBoundingBox@@@Z"}
!306 = !{!307}
!307 = distinct !{!307, !308, !"\01?GetRayData@@YA?AURayData@@V?$vector@M$02@@0@Z: %agg.result"}
!308 = distinct !{!308, !"\01?GetRayData@@YA?AURayData@@V?$vector@M$02@@0@Z"}
!309 = !{!310}
!310 = distinct !{!310, !311, !"\01?StackPush@@YAXAIAHIII@Z: %stackTop"}
!311 = distinct !{!311, !"\01?StackPush@@YAXAIAHIII@Z"}
!312 = !{!313}
!313 = distinct !{!313, !314, !"\01?StackPop@@YAIAIAHAIAII@Z: %stackTop"}
!314 = distinct !{!314, !"\01?StackPop@@YAIAIAHAIAII@Z"}
!315 = !{!316, !318}
!316 = distinct !{!316, !317, !"\01?PointerGetBuffer@@YA?AURWByteAddressBuffer@@V?$vector@I$01@@@Z: %agg.result"}
!317 = distinct !{!317, !"\01?PointerGetBuffer@@YA?AURWByteAddressBuffer@@V?$vector@I$01@@@Z"}
!318 = distinct !{!318, !319, !"\01?CreateRWByteAddressBufferPointerFromGpuVA@@YA?AURWByteAddressBufferPointer@@V?$vector@I$01@@@Z: %agg.result"}
!319 = distinct !{!319, !"\01?CreateRWByteAddressBufferPointerFromGpuVA@@YA?AURWByteAddressBufferPointer@@V?$vector@I$01@@@Z"}
!320 = !{!321}
!321 = distinct !{!321, !322, !"\01?StackPush@@YAXAIAHIII@Z: %stackTop"}
!322 = distinct !{!322, !"\01?StackPush@@YAXAIAHIII@Z"}
!323 = !{!324}
!324 = distinct !{!324, !325, !"\01?GetRayData@@YA?AURayData@@V?$vector@M$02@@0@Z: %agg.result"}
!325 = distinct !{!325, !"\01?GetRayData@@YA?AURayData@@V?$vector@M$02@@0@Z"}
!326 = !{!327, !329}
!327 = distinct !{!327, !328, !"\01?PointerGetBuffer@@YA?AURWByteAddressBuffer@@V?$vector@I$01@@@Z: %agg.result"}
!328 = distinct !{!328, !"\01?PointerGetBuffer@@YA?AURWByteAddressBuffer@@V?$vector@I$01@@@Z"}
!329 = distinct !{!329, !330, !"\01?CreateRWByteAddressBufferPointerFromGpuVA@@YA?AURWByteAddressBufferPointer@@V?$vector@I$01@@@Z: %agg.result"}
!330 = distinct !{!330, !"\01?CreateRWByteAddressBufferPointerFromGpuVA@@YA?AURWByteAddressBufferPointer@@V?$vector@I$01@@@Z"}
!331 = !{!332, !334, !335, !337, !338}
!332 = distinct !{!332, !333, !"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z: %hitT"}
!333 = distinct !{!333, !"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z"}
!334 = distinct !{!334, !333, !"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z: %bary"}
!335 = distinct !{!335, !336, !"\01?TestLeafNodeIntersections@@YA_NURWByteAddressBufferPointer@@V?$vector@I$01@@IV?$vector@M$02@@2V?$vector@H$02@@2AIAV?$vector@M$01@@AIAMAIAI@Z: %resultBary"}
!336 = distinct !{!336, !"\01?TestLeafNodeIntersections@@YA_NURWByteAddressBufferPointer@@V?$vector@I$01@@IV?$vector@M$02@@2V?$vector@H$02@@2AIAV?$vector@M$01@@AIAMAIAI@Z"}
!337 = distinct !{!337, !336, !"\01?TestLeafNodeIntersections@@YA_NURWByteAddressBufferPointer@@V?$vector@I$01@@IV?$vector@M$02@@2V?$vector@H$02@@2AIAV?$vector@M$01@@AIAMAIAI@Z: %resultT"}
!338 = distinct !{!338, !336, !"\01?TestLeafNodeIntersections@@YA_NURWByteAddressBufferPointer@@V?$vector@I$01@@IV?$vector@M$02@@2V?$vector@H$02@@2AIAV?$vector@M$01@@AIAMAIAI@Z: %resultTriId"}
!339 = !{!340}
!340 = distinct !{!340, !341, !"\01?StackPush2@@YAXAIAH_NIIII@Z: %stackTop"}
!341 = distinct !{!341, !"\01?StackPush2@@YAXAIAH_NIIII@Z"}
!342 = !{!343}
!343 = distinct !{!343, !344, !"\01?StackPush@@YAXAIAHIII@Z: %stackTop"}
!344 = distinct !{!344, !"\01?StackPush@@YAXAIAHIII@Z"}
!345 = !{!346}
!346 = distinct !{!346, !347, !"\01?GetRayData@@YA?AURayData@@V?$vector@M$02@@0@Z: %agg.result"}
!347 = distinct !{!347, !"\01?GetRayData@@YA?AURayData@@V?$vector@M$02@@0@Z"}
!348 = !{!349}
!349 = distinct !{!349, !350, !"\01?PointerGetBuffer@@YA?AURWByteAddressBuffer@@V?$vector@I$01@@@Z: %agg.result"}
!350 = distinct !{!350, !"\01?PointerGetBuffer@@YA?AURWByteAddressBuffer@@V?$vector@I$01@@@Z"}
!351 = !{!352, !354}
!352 = distinct !{!352, !353, !"\01?CompressBox@@YAXUBoundingBox@@V?$vector@I$01@@AIAV?$vector@I$03@@2@Z: %data1"}
!353 = distinct !{!353, !"\01?CompressBox@@YAXUBoundingBox@@V?$vector@I$01@@AIAV?$vector@I$03@@2@Z"}
!354 = distinct !{!354, !353, !"\01?CompressBox@@YAXUBoundingBox@@V?$vector@I$01@@AIAV?$vector@I$03@@2@Z: %data2"}
