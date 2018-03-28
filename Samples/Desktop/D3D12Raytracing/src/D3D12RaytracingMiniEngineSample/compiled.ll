; ModuleID = 'merged_lib'
target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f:64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%"class.RWTexture2D<vector<float, 4> >" = type { <4 x float> }
%AccelerationStructureList = type { <2 x i32> }
%Constants = type { i32, i32, i32, i32, i32, i32, i32, i32 }
%struct.ByteAddressBuffer = type { i32 }
%struct.RWByteAddressBuffer = type { i32 }
%"class.Texture2D<vector<float, 4> >" = type { <4 x float>, %"class.Texture2D<vector<float, 4> >::mips_type" }
%"class.Texture2D<vector<float, 4> >::mips_type" = type { i32 }
%"class.Texture2D<float>" = type { float, %"class.Texture2D<float>::mips_type" }
%"class.Texture2D<float>::mips_type" = type { i32 }
%struct.RaytracingAccelerationStructure = type { i32 }
%"class.StructuredBuffer<RayTraceMeshInfo>" = type { %struct.RayTraceMeshInfo }
%struct.RayTraceMeshInfo = type { i32, i32, i32, i32, i32, i32, i32 }
%struct.SamplerState = type { i32 }
%struct.SamplerComparisonState = type { i32 }
%HitShaderConstants = type { <3 x float>, <3 x float>, <3 x float>, <4 x float>, %class.matrix.float.4.4, i32, i32 }
%class.matrix.float.4.4 = type { [4 x <4 x float>] }
%b1 = type { %struct.DynamicCB }
%struct.DynamicCB = type { %class.matrix.float.4.4, <3 x float>, i32, <2 x float> }
%"class.Texture2D<vector<float, 3> >" = type { <3 x float>, %"class.Texture2D<vector<float, 3> >::mips_type" }
%"class.Texture2D<vector<float, 3> >::mips_type" = type { i32 }
%dx.types.CBufRet.f32 = type { float, float, float, float }
%dx.types.Handle = type { i8* }
%dx.types.ResRet.f32 = type { float, float, float, float, i32 }
%struct.BuiltInTriangleIntersectionAttributes = type { <2 x float> }
%dx.types.ResRet.i32 = type { i32, i32, i32, i32, i32 }
%class.matrix.float.3.4 = type { [3 x <4 x float>] }
%dx.types.CBufRet.i32 = type { i32, i32, i32, i32 }
%struct.RayPayload = type { i32, float }
%struct.RaytracingInstanceDesc = type { [3 x <4 x float>], i32, i32, <2 x i32> }
%struct.RWByteAddressBufferPointer = type { %struct.RWByteAddressBuffer, i32 }
%struct.BoundingBox = type { <3 x float>, <3 x float> }
%struct.BVHMetadata = type { %struct.RaytracingInstanceDesc, [3 x <4 x float>] }
%struct.AABB = type { <3 x float>, <3 x float> }
%struct.TriangleMetaData = type { i32, i32 }
%class.matrix.float.2.2 = type { [2 x <2 x float>] }
%struct.RayData = type { <3 x float>, <3 x float>, <3 x float>, <3 x float>, <3 x float>, <3 x i32> }
%struct.RuntimeDataStruct = type { [2 x i32], [2 x i32], float, float, i32, [3 x float], [3 x float], [3 x float], [3 x float], [12 x float], [12 x float], i32, i32, i32, i32, i32, float, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, [256 x i32]* }
%Material = type { i32 }

@"\01?g_screenOutput@@3V?$RWTexture2D@V?$vector@M$03@@@@A" = external global %"class.RWTexture2D<vector<float, 4> >"
@AccelerationStructureList = external constant %AccelerationStructureList
@Constants = external constant %Constants
@"\01?stack@@3PAIA" = addrspace(3) global [2048 x i32] zeroinitializer
@"\01?HitGroupShaderTable@@3UByteAddressBuffer@@A" = external global %struct.ByteAddressBuffer
@"\01?MissShaderTable@@3UByteAddressBuffer@@A" = external global %struct.ByteAddressBuffer
@"\01?DescriptorHeapBufferTable@@3PAURWByteAddressBuffer@@A" = external global [0 x %struct.RWByteAddressBuffer]
@"\01?RayGenShaderTable@@3UByteAddressBuffer@@A" = external global %struct.ByteAddressBuffer
@"\01?g_texNormal@@3PAV?$Texture2D@V?$vector@M$03@@@@A" = external global [27 x %"class.Texture2D<vector<float, 4> >"]
@"\01?texSSAO@@3V?$Texture2D@M@@A" = external global %"class.Texture2D<float>"
@"\01?g_attributes@@3UByteAddressBuffer@@A" = external global %struct.ByteAddressBuffer
@"\01?texShadow@@3V?$Texture2D@M@@A" = external global %"class.Texture2D<float>"
@"\01?g_accel@@3URaytracingAccelerationStructure@@A" = external global %struct.RaytracingAccelerationStructure
@"\01?g_meshInfo@@3V?$StructuredBuffer@URayTraceMeshInfo@@@@A" = external global %"class.StructuredBuffer<RayTraceMeshInfo>"
@"\01?g_indices@@3UByteAddressBuffer@@A" = external global %struct.ByteAddressBuffer
@"\01?g_texDiffuse@@3PAV?$Texture2D@V?$vector@M$03@@@@A" = external global [27 x %"class.Texture2D<vector<float, 4> >"]
@"\01?g_s0@@3USamplerState@@A" = external global %struct.SamplerState
@"\01?shadowSampler@@3USamplerComparisonState@@A" = external global %struct.SamplerComparisonState
@HitShaderConstants = external constant %HitShaderConstants
@b1 = external constant %b1
@"3boxVertices.3.hca" = internal constant [8 x float] [float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00]
@"\01?depth@@3V?$Texture2D@M@@A" = external global %"class.Texture2D<float>"
@"\01?normals@@3V?$Texture2D@V?$vector@M$02@@@@A" = external global %"class.Texture2D<vector<float, 3> >"

; Function Attrs: nounwind readnone
declare float @dx.op.unary.f32(i32, float) #0

; Function Attrs: nounwind readonly
declare %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32, %dx.types.Handle, i32) #1

; Function Attrs: alwaysinline
declare void @"\01?Fallback_TraceRayBegin@@YAXV?$vector@M$02@@M0M@Z"(<3 x float>, float, <3 x float>, float) #2

; Function Attrs: nounwind readnone
declare float @dx.op.tertiary.f32(i32, float, float, float) #0

; Function Attrs: nounwind readonly
declare %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RaytracingAccelerationStructure(i32, %struct.RaytracingAccelerationStructure) #1

; Function Attrs: nounwind readonly
declare %dx.types.ResRet.f32 @dx.op.sampleCmpLevelZero.f32(i32, %dx.types.Handle, %dx.types.Handle, float, float, float, float, i32, i32, i32, float) #1

; Function Attrs: alwaysinline
declare void @"\01?Fallback_SetPendingTriVals@@YAXMIIIII@Z"(float, i32, i32, i32, i32, i32) #2

; Function Attrs: nounwind readnone
declare float @dx.op.worldRayOrigin.f32(i32, i8) #0

; Function Attrs: nounwind readonly
declare %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.ByteAddressBuffer(i32, %struct.ByteAddressBuffer) #1

; Function Attrs: alwaysinline
declare i32 @"\01?Fallback_AnyHitResult@@YAHXZ"() #2

; Function Attrs: alwaysinline
declare void @"\01?Fallback_SetObjectRayOrigin@@YAXV?$vector@M$02@@@Z"(<3 x float>) #2

; Function Attrs: nounwind readonly
declare float @dx.op.rayTCurrent.f32(i32) #1

; Function Attrs: nounwind readnone
declare float @dx.op.rayTMin.f32(i32) #0

; Function Attrs: nounwind readonly
declare %dx.types.Handle @"dx.op.createHandleFromResourceStructForLib.class.RWTexture2D<vector<float, 4> >"(i32, %"class.RWTexture2D<vector<float, 4> >") #1

; Function Attrs: alwaysinline
declare void @"\01?Fallback_Scheduler@@YAXHII@Z"(i32, i32, i32) #2

; Function Attrs: nounwind
declare void @dx.op.textureStore.f32(i32, %dx.types.Handle, i32, i32, i32, float, float, float, float, i8) #3

; Function Attrs: alwaysinline
declare void @"\01?Fallback_SetAnyHitResult@@YAXH@Z"(i32) #2

; Function Attrs: alwaysinline
declare void @"\01?Fallback_SetInstanceIndex@@YAXI@Z"(i32) #2

; Function Attrs: nounwind readnone
declare i32 @dx.op.instanceIndex.i32(i32) #0

; Function Attrs: alwaysinline
declare void @"\01?Fallback_CommitHit@@YAXXZ"() #2

; Function Attrs: nounwind readonly
declare %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.Constants(i32, %Constants) #1

; Function Attrs: alwaysinline
declare void @"\01?Fallback_SetPendingAttr@@YAXUBuiltInTriangleIntersectionAttributes@@@Z"(%struct.BuiltInTriangleIntersectionAttributes*) #2

; Function Attrs: nounwind readonly
declare %dx.types.Handle @"dx.op.createHandleFromResourceStructForLib.class.Texture2D<vector<float, 4> >"(i32, %"class.Texture2D<vector<float, 4> >") #1

; Function Attrs: nounwind readonly
declare %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32, %dx.types.Handle, i32, i32) #1

; Function Attrs: alwaysinline
declare void @"\01?Fallback_SetWorldToObject@@YAXV?$matrix@M$02$03@@@Z"(%class.matrix.float.3.4) #2

; Function Attrs: nounwind readonly
declare %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32, %dx.types.Handle, i32) #1

; Function Attrs: alwaysinline
declare i32 @"\01?Fallback_GroupIndex@@YAIXZ"() #2

; Function Attrs: nounwind readonly
declare %dx.types.Handle @"dx.op.createHandleFromResourceStructForLib.class.Texture2D<vector<float, 3> >"(i32, %"class.Texture2D<vector<float, 3> >") #1

; Function Attrs: nounwind readonly
declare %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.SamplerState(i32, %struct.SamplerState) #1

; Function Attrs: nounwind readonly
declare %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32, %struct.RWByteAddressBuffer) #1

; Function Attrs: nounwind readonly
declare %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.b1(i32, %b1) #1

; Function Attrs: nounwind readonly
declare %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.AccelerationStructureList(i32, %AccelerationStructureList) #1

; Function Attrs: nounwind readonly
declare %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.HitShaderConstants(i32, %HitShaderConstants) #1

; Function Attrs: nounwind readonly
declare %dx.types.ResRet.f32 @dx.op.textureLoad.f32(i32, %dx.types.Handle, i32, i32, i32, i32, i32, i32, i32) #1

; Function Attrs: nounwind readnone
declare float @dx.op.worldRayDirection.f32(i32, i8) #0

; Function Attrs: nounwind readonly
declare %dx.types.Handle @"dx.op.createHandleFromResourceStructForLib.class.StructuredBuffer<RayTraceMeshInfo>"(i32, %"class.StructuredBuffer<RayTraceMeshInfo>") #1

; Function Attrs: nounwind readonly
declare %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.SamplerComparisonState(i32, %struct.SamplerComparisonState) #1

declare i32 @"\01?Fallback_GeometryIndex@@YAIXZ"()

; Function Attrs: alwaysinline
declare void @"\01?Fallback_SetRayFlags@@YAXI@Z"(i32) #2

; Function Attrs: alwaysinline
declare void @"\01?Fallback_SetObjectRayDirection@@YAXV?$vector@M$02@@@Z"(<3 x float>) #2

; Function Attrs: nounwind readnone
declare float @dx.op.dot3.f32(i32, float, float, float, float, float, float) #0

; Function Attrs: nounwind readnone
declare i32 @dx.op.rayFlags.i32(i32) #0

; Function Attrs: nounwind
declare void @dx.op.traceRay.struct.RayPayload(i32, %dx.types.Handle, i32, i32, i32, i32, i32, float, float, float, float, float, float, float, float, %struct.RayPayload*) #3

; Function Attrs: nounwind readonly
declare %dx.types.ResRet.f32 @dx.op.sampleGrad.f32(i32, %dx.types.Handle, %dx.types.Handle, float, float, float, float, i32, i32, i32, float, float, float, float, float, float, float) #1

; Function Attrs: alwaysinline
declare void @"\01?Fallback_SetObjectToWorld@@YAXV?$matrix@M$02$03@@@Z"(%class.matrix.float.3.4) #2

; Function Attrs: alwaysinline
declare void @"\01?Fallback_CallIndirect@@YAXH@Z"(i32) #2

; Function Attrs: alwaysinline
declare i32 @"\01?Fallback_SetPayloadOffset@@YAII@Z"(i32) #2

; Function Attrs: nounwind readnone
declare float @dx.op.binary.f32(i32, float, float) #0

; Function Attrs: nounwind readnone
declare i32 @dx.op.primitiveID.i32(i32) #0

; Function Attrs: nounwind
declare void @dx.op.bufferStore.i32(i32, %dx.types.Handle, i32, i32, i32, i32, i32, i32, i8) #3

; Function Attrs: nounwind readnone
declare i32 @dx.op.dispatchRaysIndex.i32(i32, i8) #0

; Function Attrs: nounwind readonly
declare %dx.types.Handle @"dx.op.createHandleFromResourceStructForLib.class.Texture2D<float>"(i32, %"class.Texture2D<float>") #1

; Function Attrs: alwaysinline nounwind readonly
define %class.matrix.float.3.4 @"\01?CreateMatrix@@YA?AV?$matrix@M$02$03@@Y02V?$vector@M$03@@@Z"([3 x <4 x float>]* nocapture readonly) #4 {
  %2 = getelementptr inbounds [3 x <4 x float>], [3 x <4 x float>]* %0, i32 0, i32 0
  %3 = load <4 x float>, <4 x float>* %2, align 4, !tbaa !218
  %4 = getelementptr inbounds [3 x <4 x float>], [3 x <4 x float>]* %0, i32 0, i32 1
  %5 = load <4 x float>, <4 x float>* %4, align 4, !tbaa !218
  %6 = getelementptr inbounds [3 x <4 x float>], [3 x <4 x float>]* %0, i32 0, i32 2
  %7 = load <4 x float>, <4 x float>* %6, align 4, !tbaa !218
  %.upto0 = insertelement <4 x float> undef, float %12, i32 0
  %.upto1 = insertelement <4 x float> %.upto0, float %11, i32 1
  %.upto2 = insertelement <4 x float> %.upto1, float %10, i32 2
  %8 = insertelement <4 x float> %.upto2, float %9, i32 3
  %9 = extractelement <4 x float> %3, i64 3
  %10 = extractelement <4 x float> %3, i64 2
  %11 = extractelement <4 x float> %3, i64 1
  %12 = extractelement <4 x float> %3, i64 0
  %13 = insertvalue %class.matrix.float.3.4 undef, <4 x float> %8, 0, 0
  %.upto01 = insertelement <4 x float> undef, float %18, i32 0
  %.upto12 = insertelement <4 x float> %.upto01, float %17, i32 1
  %.upto23 = insertelement <4 x float> %.upto12, float %16, i32 2
  %14 = insertelement <4 x float> %.upto23, float %15, i32 3
  %15 = extractelement <4 x float> %5, i64 3
  %16 = extractelement <4 x float> %5, i64 2
  %17 = extractelement <4 x float> %5, i64 1
  %18 = extractelement <4 x float> %5, i64 0
  %19 = insertvalue %class.matrix.float.3.4 %13, <4 x float> %14, 0, 1
  %.upto04 = insertelement <4 x float> undef, float %24, i32 0
  %.upto15 = insertelement <4 x float> %.upto04, float %23, i32 1
  %.upto26 = insertelement <4 x float> %.upto15, float %22, i32 2
  %20 = insertelement <4 x float> %.upto26, float %21, i32 3
  %21 = extractelement <4 x float> %7, i64 3
  %22 = extractelement <4 x float> %7, i64 2
  %23 = extractelement <4 x float> %7, i64 1
  %24 = extractelement <4 x float> %7, i64 0
  %25 = insertvalue %class.matrix.float.3.4 %19, <4 x float> %20, 0, 2
  ret %class.matrix.float.3.4 %25
}

; Function Attrs: alwaysinline nounwind readonly
define i32 @"\01?GetInstanceContributionToHitGroupIndex@@YAIURaytracingInstanceDesc@@@Z"(%struct.RaytracingInstanceDesc* nocapture readonly) #4 {
  %2 = getelementptr inbounds %struct.RaytracingInstanceDesc, %struct.RaytracingInstanceDesc* %0, i32 0, i32 2
  %3 = load i32, i32* %2, align 4, !tbaa !221
  %4 = and i32 %3, 16777215
  ret i32 %4
}

; Function Attrs: alwaysinline nounwind
define void @"\01?BVHReadTriangle@@YAXURWByteAddressBufferPointer@@AIAV?$vector@M$02@@11I@Z"(%struct.RWByteAddressBufferPointer* nocapture readonly, <3 x float>* noalias nocapture dereferenceable(12), <3 x float>* noalias nocapture dereferenceable(12), <3 x float>* noalias nocapture dereferenceable(12), i32) #5 {
  %6 = getelementptr inbounds %struct.RWByteAddressBufferPointer, %struct.RWByteAddressBufferPointer* %0, i32 0, i32 1
  %7 = load i32, i32* %6, align 4, !tbaa !221
  %8 = add i32 %7, 4
  %9 = getelementptr inbounds %struct.RWByteAddressBufferPointer, %struct.RWByteAddressBufferPointer* %0, i32 0, i32 0
  %10 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %9, align 4
  %11 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %10)
  %12 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %11, i32 %8, i32 undef)
  %13 = extractvalue %dx.types.ResRet.i32 %12, 0
  %14 = mul i32 %4, 36
  %15 = add i32 %7, %14
  %16 = add i32 %15, %13
  %17 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %11, i32 %16, i32 undef)
  %18 = extractvalue %dx.types.ResRet.i32 %17, 0
  %19 = extractvalue %dx.types.ResRet.i32 %17, 1
  %20 = extractvalue %dx.types.ResRet.i32 %17, 2
  %21 = extractvalue %dx.types.ResRet.i32 %17, 3
  %.i0 = bitcast i32 %18 to float
  %.i1 = bitcast i32 %19 to float
  %.i2 = bitcast i32 %20 to float
  %.i3 = bitcast i32 %21 to float
  %22 = add i32 %16, 16
  %23 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %11, i32 %22, i32 undef)
  %24 = extractvalue %dx.types.ResRet.i32 %23, 0
  %25 = extractvalue %dx.types.ResRet.i32 %23, 1
  %26 = extractvalue %dx.types.ResRet.i32 %23, 2
  %27 = extractvalue %dx.types.ResRet.i32 %23, 3
  %.i04 = bitcast i32 %24 to float
  %.i15 = bitcast i32 %25 to float
  %.i26 = bitcast i32 %26 to float
  %.i37 = bitcast i32 %27 to float
  %28 = add i32 %16, 32
  %29 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %11, i32 %28, i32 undef)
  %30 = extractvalue %dx.types.ResRet.i32 %29, 0
  %31 = bitcast i32 %30 to float
  %.upto08 = insertelement <3 x float> undef, float %.i0, i32 0
  %.upto19 = insertelement <3 x float> %.upto08, float %.i1, i32 1
  %32 = insertelement <3 x float> %.upto19, float %.i2, i32 2
  store <3 x float> %32, <3 x float>* %1, align 4, !tbaa !218
  %33 = insertelement <3 x float> undef, float %.i3, i64 0
  %34 = insertelement <3 x float> %33, float %.i04, i64 1
  %35 = insertelement <3 x float> %34, float %.i15, i64 2
  store <3 x float> %35, <3 x float>* %2, align 4, !tbaa !218
  %36 = insertelement <3 x float> undef, float %.i26, i64 0
  %37 = insertelement <3 x float> %36, float %.i37, i64 1
  %38 = insertelement <3 x float> %37, float %31, i64 2
  store <3 x float> %38, <3 x float>* %3, align 4, !tbaa !218
  ret void
}

; Function Attrs: alwaysinline nounwind
define void @"\01?PointerGetBuffer@@YA?AURWByteAddressBuffer@@V?$vector@I$01@@@Z"(%struct.RWByteAddressBuffer* noalias nocapture sret, <2 x i32>) #5 {
  ret void
}

; Function Attrs: alwaysinline nounwind readnone
define float @"\01?Determinant@@YAMV?$matrix@M$02$03@@@Z"(%class.matrix.float.3.4) #6 {
  %2 = extractvalue %class.matrix.float.3.4 %0, 0, 0
  %3 = extractelement <4 x float> %2, i32 2
  %4 = extractelement <4 x float> %2, i32 1
  %5 = extractelement <4 x float> %2, i32 0
  %6 = extractvalue %class.matrix.float.3.4 %0, 0, 1
  %7 = extractelement <4 x float> %6, i32 2
  %8 = extractelement <4 x float> %6, i32 1
  %9 = extractelement <4 x float> %6, i32 0
  %10 = extractvalue %class.matrix.float.3.4 %0, 0, 2
  %11 = extractelement <4 x float> %10, i32 2
  %12 = extractelement <4 x float> %10, i32 1
  %13 = extractelement <4 x float> %10, i32 0
  %14 = fmul fast float %5, %8
  %15 = fmul fast float %14, %11
  %16 = fmul fast float %5, %12
  %17 = fmul fast float %16, %7
  %18 = fsub fast float %15, %17
  %19 = fmul fast float %9, %4
  %20 = fmul fast float %19, %11
  %21 = fsub fast float %18, %20
  %22 = fmul fast float %9, %12
  %23 = fmul fast float %22, %3
  %24 = fadd fast float %21, %23
  %25 = fmul fast float %13, %4
  %26 = fmul fast float %25, %7
  %27 = fadd fast float %24, %26
  %28 = fmul fast float %13, %8
  %29 = fmul fast float %28, %3
  %30 = fsub fast float %27, %29
  ret float %30
}

; Function Attrs: alwaysinline nounwind
define void @"\01?CalculateTrianglePartialDerivatives@@YAXV?$vector@M$01@@00V?$vector@M$02@@11AIAV2@2@Z"(<2 x float>, <2 x float>, <2 x float>, <3 x float>, <3 x float>, <3 x float>, <3 x float>* noalias nocapture dereferenceable(12), <3 x float>* noalias nocapture dereferenceable(12)) #5 {
  %p1.i0 = extractelement <3 x float> %4, i32 0
  %p1.i1 = extractelement <3 x float> %4, i32 1
  %p1.i2 = extractelement <3 x float> %4, i32 2
  %p2.i0 = extractelement <3 x float> %5, i32 0
  %p0.i0 = extractelement <3 x float> %3, i32 0
  %p2.i1 = extractelement <3 x float> %5, i32 1
  %p0.i1 = extractelement <3 x float> %3, i32 1
  %p2.i2 = extractelement <3 x float> %5, i32 2
  %p0.i2 = extractelement <3 x float> %3, i32 2
  %uv1.i0 = extractelement <2 x float> %1, i32 0
  %uv1.i1 = extractelement <2 x float> %1, i32 1
  %uv2.i0 = extractelement <2 x float> %2, i32 0
  %uv0.i0 = extractelement <2 x float> %0, i32 0
  %uv2.i1 = extractelement <2 x float> %2, i32 1
  %uv0.i1 = extractelement <2 x float> %0, i32 1
  %.i0 = fsub fast float %uv0.i0, %uv2.i0
  %.i1 = fsub fast float %uv0.i1, %uv2.i1
  %.i04 = fsub fast float %uv1.i0, %uv2.i0
  %.i15 = fsub fast float %uv1.i1, %uv2.i1
  %.i06 = fsub fast float %p0.i0, %p2.i0
  %.i17 = fsub fast float %p0.i1, %p2.i1
  %.i2 = fsub fast float %p0.i2, %p2.i2
  %.i08 = fsub fast float %p1.i0, %p2.i0
  %.i19 = fsub fast float %p1.i1, %p2.i1
  %.i210 = fsub fast float %p1.i2, %p2.i2
  %9 = fmul fast float %.i0, %.i15
  %10 = fmul fast float %.i04, %.i1
  %11 = fsub fast float %9, %10
  %12 = fdiv fast float 1.000000e+00, %11
  %.i011 = fmul fast float %12, %.i15
  %13 = fmul fast float %.i04, %12
  %.i112 = fsub fast float -0.000000e+00, %13
  %14 = fmul fast float %.i1, %12
  %.i213 = fsub fast float -0.000000e+00, %14
  %.i3 = fmul fast float %12, %.i0
  %.i014 = fmul fast float %.i011, %.i06
  %.i115 = fmul fast float %.i011, %.i17
  %.i216 = fmul fast float %.i011, %.i2
  %.i017 = fmul fast float %.i08, %.i112
  %.i118 = fmul fast float %.i19, %.i112
  %.i219 = fmul fast float %.i210, %.i112
  %.i020 = fadd fast float %.i014, %.i017
  %.i121 = fadd fast float %.i115, %.i118
  %.i222 = fadd fast float %.i216, %.i219
  %.upto038 = insertelement <3 x float> undef, float %.i020, i32 0
  %.upto139 = insertelement <3 x float> %.upto038, float %.i121, i32 1
  %15 = insertelement <3 x float> %.upto139, float %.i222, i32 2
  store <3 x float> %15, <3 x float>* %6, align 4, !tbaa !218
  %.i023 = fmul fast float %.i06, %.i213
  %.i124 = fmul fast float %.i17, %.i213
  %.i225 = fmul fast float %.i2, %.i213
  %.i026 = fmul fast float %.i3, %.i08
  %.i127 = fmul fast float %.i3, %.i19
  %.i228 = fmul fast float %.i3, %.i210
  %.i029 = fadd fast float %.i023, %.i026
  %.i130 = fadd fast float %.i124, %.i127
  %.i231 = fadd fast float %.i225, %.i228
  %.upto041 = insertelement <3 x float> undef, float %.i029, i32 0
  %.upto142 = insertelement <3 x float> %.upto041, float %.i130, i32 1
  %16 = insertelement <3 x float> %.upto142, float %.i231, i32 2
  store <3 x float> %16, <3 x float>* %7, align 4, !tbaa !218
  ret void
}

; Function Attrs: alwaysinline nounwind readnone
define i32 @"\01?GetLeafIndexFromFlag@@YAHV?$vector@I$01@@@Z"(<2 x i32>) #6 {
  %2 = extractelement <2 x i32> %0, i32 0
  %3 = and i32 %2, 2147483647
  ret i32 %3
}

; Function Attrs: alwaysinline nounwind readnone
define i32 @"\01?GetLeftNodeIndex@@YAIV?$vector@I$01@@@Z"(<2 x i32>) #6 {
  %2 = extractelement <2 x i32> %0, i32 0
  %3 = and i32 %2, 16777215
  ret i32 %3
}

; Function Attrs: alwaysinline nounwind readnone
define i1 @"\01?Cull@@YA_N_NI@Z"(i1 zeroext, i32) #6 {
  %3 = and i32 %1, 64
  %4 = icmp ne i32 %3, 0
  %5 = and i1 %4, %0
  %6 = xor i1 %0, true
  %7 = and i32 %1, 128
  %8 = icmp ne i32 %7, 0
  %9 = and i1 %8, %6
  %10 = or i1 %5, %9
  ret i1 %10
}

; Function Attrs: alwaysinline nounwind readonly
define i32 @"\01?GetOffsetToVertices@@YAHURWByteAddressBufferPointer@@@Z"(%struct.RWByteAddressBufferPointer* nocapture readonly) #4 {
  %2 = getelementptr inbounds %struct.RWByteAddressBufferPointer, %struct.RWByteAddressBufferPointer* %0, i32 0, i32 1
  %3 = load i32, i32* %2, align 4, !tbaa !221
  %4 = add i32 %3, 4
  %5 = getelementptr inbounds %struct.RWByteAddressBufferPointer, %struct.RWByteAddressBufferPointer* %0, i32 0, i32 0
  %6 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %5, align 4
  %7 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %6)
  %8 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %7, i32 %4, i32 undef)
  %9 = extractvalue %dx.types.ResRet.i32 %8, 0
  %10 = add i32 %9, %3
  ret i32 %10
}

; Function Attrs: alwaysinline nounwind readonly
define i32 @"\01?GetInstanceMask@@YAIURaytracingInstanceDesc@@@Z"(%struct.RaytracingInstanceDesc* nocapture readonly) #4 {
  %2 = getelementptr inbounds %struct.RaytracingInstanceDesc, %struct.RaytracingInstanceDesc* %0, i32 0, i32 1
  %3 = load i32, i32* %2, align 4, !tbaa !221
  %4 = lshr i32 %3, 24
  ret i32 %4
}

; Function Attrs: alwaysinline nounwind
define void @"\01?GetBoxFromBuffer@@YA?AUBoundingBox@@URWByteAddressBuffer@@II@Z"(%struct.BoundingBox* noalias nocapture sret, %struct.RWByteAddressBuffer* nocapture readonly, i32, i32) #5 {
  %5 = shl i32 %3, 5
  %6 = add i32 %5, %2
  %7 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %1, align 4
  %8 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %7)
  %9 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %8, i32 %6, i32 undef)
  %10 = extractvalue %dx.types.ResRet.i32 %9, 0
  %11 = extractvalue %dx.types.ResRet.i32 %9, 1
  %12 = extractvalue %dx.types.ResRet.i32 %9, 2
  %13 = add i32 %6, 16
  %14 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %8, i32 %13, i32 undef)
  %15 = extractvalue %dx.types.ResRet.i32 %14, 0
  %16 = extractvalue %dx.types.ResRet.i32 %14, 1
  %17 = extractvalue %dx.types.ResRet.i32 %14, 2
  %18 = bitcast i32 %10 to float
  %19 = insertelement <3 x float> undef, float %18, i32 0
  %20 = bitcast i32 %11 to float
  %21 = insertelement <3 x float> %19, float %20, i32 1
  %22 = bitcast i32 %12 to float
  %23 = insertelement <3 x float> %21, float %22, i32 2
  %24 = bitcast i32 %15 to float
  %25 = insertelement <3 x float> undef, float %24, i32 0
  %26 = bitcast i32 %16 to float
  %27 = insertelement <3 x float> %25, float %26, i32 1
  %28 = bitcast i32 %17 to float
  %29 = insertelement <3 x float> %27, float %28, i32 2
  %30 = getelementptr inbounds %struct.BoundingBox, %struct.BoundingBox* %0, i32 0, i32 0
  store <3 x float> %23, <3 x float>* %30, align 4
  %31 = getelementptr inbounds %struct.BoundingBox, %struct.BoundingBox* %0, i32 0, i32 1
  store <3 x float> %29, <3 x float>* %31, align 4
  ret void
}

; Function Attrs: alwaysinline nounwind
declare void @Miss(%struct.RayPayload* noalias nocapture) #5

; Function Attrs: alwaysinline nounwind
define void @"\01?BVHReadBoundingBox@@YA?AUBoundingBox@@URWByteAddressBufferPointer@@HAIAV?$vector@I$01@@@Z"(%struct.BoundingBox* noalias nocapture sret, %struct.RWByteAddressBufferPointer* nocapture readonly, i32, <2 x i32>* noalias nocapture dereferenceable(8)) #5 {
  %5 = getelementptr inbounds %struct.RWByteAddressBufferPointer, %struct.RWByteAddressBufferPointer* %1, i32 0, i32 1
  %6 = load i32, i32* %5, align 4, !tbaa !221
  %7 = shl i32 %2, 5
  %8 = or i32 %7, 16
  %9 = add i32 %8, %6
  %10 = getelementptr inbounds %struct.RWByteAddressBufferPointer, %struct.RWByteAddressBufferPointer* %1, i32 0, i32 0
  %11 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %10, align 4
  %12 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %11)
  %13 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %12, i32 %9, i32 undef)
  %14 = extractvalue %dx.types.ResRet.i32 %13, 0
  %15 = extractvalue %dx.types.ResRet.i32 %13, 1
  %16 = extractvalue %dx.types.ResRet.i32 %13, 2
  %17 = extractvalue %dx.types.ResRet.i32 %13, 3
  %18 = add i32 %9, 16
  %19 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %12, i32 %18, i32 undef)
  %20 = extractvalue %dx.types.ResRet.i32 %19, 0
  %21 = extractvalue %dx.types.ResRet.i32 %19, 1
  %22 = extractvalue %dx.types.ResRet.i32 %19, 2
  %23 = extractvalue %dx.types.ResRet.i32 %19, 3
  %24 = bitcast i32 %14 to float
  %25 = insertelement <3 x float> undef, float %24, i32 0
  %26 = bitcast i32 %15 to float
  %27 = insertelement <3 x float> %25, float %26, i32 1
  %28 = bitcast i32 %16 to float
  %29 = insertelement <3 x float> %27, float %28, i32 2
  %30 = bitcast i32 %20 to float
  %31 = insertelement <3 x float> undef, float %30, i32 0
  %32 = bitcast i32 %21 to float
  %33 = insertelement <3 x float> %31, float %32, i32 1
  %34 = bitcast i32 %22 to float
  %35 = insertelement <3 x float> %33, float %34, i32 2
  %36 = insertelement <2 x i32> undef, i32 %17, i64 0
  %37 = insertelement <2 x i32> %36, i32 %23, i64 1
  store <2 x i32> %37, <2 x i32>* %3, align 4, !tbaa !218
  %38 = getelementptr inbounds %struct.BoundingBox, %struct.BoundingBox* %0, i32 0, i32 0
  store <3 x float> %29, <3 x float>* %38, align 4
  %39 = getelementptr inbounds %struct.BoundingBox, %struct.BoundingBox* %0, i32 0, i32 1
  store <3 x float> %35, <3 x float>* %39, align 4
  ret void
}

; Function Attrs: alwaysinline nounwind
declare void @Fallback_TraceRay(i32, i32, i32, i32, i32, float, float, float, float, float, float, float, float, i32) #7

; Function Attrs: alwaysinline nounwind
define void @"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z"(%struct.BVHMetadata* noalias nocapture sret, %struct.RWByteAddressBuffer* nocapture readonly, i32) #5 {
  %4 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %1, align 4
  %5 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %4)
  %6 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %5, i32 %2, i32 undef)
  %7 = extractvalue %dx.types.ResRet.i32 %6, 0
  %8 = extractvalue %dx.types.ResRet.i32 %6, 1
  %9 = extractvalue %dx.types.ResRet.i32 %6, 2
  %10 = extractvalue %dx.types.ResRet.i32 %6, 3
  %11 = add i32 %2, 16
  %12 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %5, i32 %11, i32 undef)
  %13 = extractvalue %dx.types.ResRet.i32 %12, 0
  %14 = extractvalue %dx.types.ResRet.i32 %12, 1
  %15 = extractvalue %dx.types.ResRet.i32 %12, 2
  %16 = extractvalue %dx.types.ResRet.i32 %12, 3
  %17 = add i32 %2, 32
  %18 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %5, i32 %17, i32 undef)
  %19 = extractvalue %dx.types.ResRet.i32 %18, 0
  %20 = extractvalue %dx.types.ResRet.i32 %18, 1
  %21 = extractvalue %dx.types.ResRet.i32 %18, 2
  %22 = extractvalue %dx.types.ResRet.i32 %18, 3
  %23 = add i32 %2, 48
  %24 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %5, i32 %23, i32 undef)
  %25 = extractvalue %dx.types.ResRet.i32 %24, 0
  %26 = extractvalue %dx.types.ResRet.i32 %24, 1
  %27 = extractvalue %dx.types.ResRet.i32 %24, 2
  %28 = extractvalue %dx.types.ResRet.i32 %24, 3
  %29 = add i32 %2, 64
  %30 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %5, i32 %29, i32 undef)
  %31 = extractvalue %dx.types.ResRet.i32 %30, 0
  %32 = extractvalue %dx.types.ResRet.i32 %30, 1
  %33 = extractvalue %dx.types.ResRet.i32 %30, 2
  %34 = extractvalue %dx.types.ResRet.i32 %30, 3
  %35 = add i32 %2, 80
  %36 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %5, i32 %35, i32 undef)
  %37 = extractvalue %dx.types.ResRet.i32 %36, 0
  %38 = extractvalue %dx.types.ResRet.i32 %36, 1
  %39 = extractvalue %dx.types.ResRet.i32 %36, 2
  %40 = extractvalue %dx.types.ResRet.i32 %36, 3
  %41 = add i32 %2, 96
  %42 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %5, i32 %41, i32 undef)
  %43 = extractvalue %dx.types.ResRet.i32 %42, 0
  %44 = extractvalue %dx.types.ResRet.i32 %42, 1
  %45 = extractvalue %dx.types.ResRet.i32 %42, 2
  %46 = extractvalue %dx.types.ResRet.i32 %42, 3
  %.upto0112 = insertelement <2 x i32> undef, i32 %27, i32 0
  %47 = insertelement <2 x i32> %.upto0112, i32 %28, i32 1
  %48 = getelementptr inbounds %struct.BVHMetadata, %struct.BVHMetadata* %0, i32 0, i32 0, i32 0, i32 0
  %49 = bitcast i32 %7 to float
  %insert56 = insertelement <4 x float> undef, float %49, i64 0
  %50 = bitcast i32 %8 to float
  %insert58 = insertelement <4 x float> %insert56, float %50, i64 1
  %51 = bitcast i32 %9 to float
  %insert60 = insertelement <4 x float> %insert58, float %51, i64 2
  %52 = bitcast i32 %10 to float
  %insert62 = insertelement <4 x float> %insert60, float %52, i64 3
  store <4 x float> %insert62, <4 x float>* %48, align 4
  %53 = getelementptr inbounds %struct.BVHMetadata, %struct.BVHMetadata* %0, i32 0, i32 0, i32 0, i32 1
  %54 = bitcast i32 %13 to float
  %insert64 = insertelement <4 x float> undef, float %54, i64 0
  %55 = bitcast i32 %14 to float
  %insert66 = insertelement <4 x float> %insert64, float %55, i64 1
  %56 = bitcast i32 %15 to float
  %insert68 = insertelement <4 x float> %insert66, float %56, i64 2
  %57 = bitcast i32 %16 to float
  %insert70 = insertelement <4 x float> %insert68, float %57, i64 3
  store <4 x float> %insert70, <4 x float>* %53, align 4
  %58 = getelementptr inbounds %struct.BVHMetadata, %struct.BVHMetadata* %0, i32 0, i32 0, i32 0, i32 2
  %59 = bitcast i32 %19 to float
  %insert72 = insertelement <4 x float> undef, float %59, i64 0
  %60 = bitcast i32 %20 to float
  %insert74 = insertelement <4 x float> %insert72, float %60, i64 1
  %61 = bitcast i32 %21 to float
  %insert76 = insertelement <4 x float> %insert74, float %61, i64 2
  %62 = bitcast i32 %22 to float
  %insert78 = insertelement <4 x float> %insert76, float %62, i64 3
  store <4 x float> %insert78, <4 x float>* %58, align 4
  %63 = getelementptr inbounds %struct.BVHMetadata, %struct.BVHMetadata* %0, i32 0, i32 0, i32 1
  store i32 %25, i32* %63, align 4
  %64 = getelementptr inbounds %struct.BVHMetadata, %struct.BVHMetadata* %0, i32 0, i32 0, i32 2
  store i32 %26, i32* %64, align 4
  %65 = getelementptr inbounds %struct.BVHMetadata, %struct.BVHMetadata* %0, i32 0, i32 0, i32 3
  store <2 x i32> %47, <2 x i32>* %65, align 4
  %.i094 = bitcast i32 %31 to float
  %.i195 = bitcast i32 %32 to float
  %.i296 = bitcast i32 %33 to float
  %.i397 = bitcast i32 %34 to float
  %.upto0114 = insertelement <4 x float> undef, float %.i094, i32 0
  %.upto1115 = insertelement <4 x float> %.upto0114, float %.i195, i32 1
  %.upto2116 = insertelement <4 x float> %.upto1115, float %.i296, i32 2
  %66 = insertelement <4 x float> %.upto2116, float %.i397, i32 3
  %67 = getelementptr inbounds %struct.BVHMetadata, %struct.BVHMetadata* %0, i32 0, i32 1, i32 0
  store <4 x float> %66, <4 x float>* %67, align 4, !tbaa !218
  %.i098 = bitcast i32 %37 to float
  %.i199 = bitcast i32 %38 to float
  %.i2100 = bitcast i32 %39 to float
  %.i3101 = bitcast i32 %40 to float
  %.upto0117 = insertelement <4 x float> undef, float %.i098, i32 0
  %.upto1118 = insertelement <4 x float> %.upto0117, float %.i199, i32 1
  %.upto2119 = insertelement <4 x float> %.upto1118, float %.i2100, i32 2
  %68 = insertelement <4 x float> %.upto2119, float %.i3101, i32 3
  %69 = getelementptr inbounds %struct.BVHMetadata, %struct.BVHMetadata* %0, i32 0, i32 1, i32 1
  store <4 x float> %68, <4 x float>* %69, align 4, !tbaa !218
  %.i0102 = bitcast i32 %43 to float
  %.i1103 = bitcast i32 %44 to float
  %.i2104 = bitcast i32 %45 to float
  %.i3105 = bitcast i32 %46 to float
  %.upto0120 = insertelement <4 x float> undef, float %.i0102, i32 0
  %.upto1121 = insertelement <4 x float> %.upto0120, float %.i1103, i32 1
  %.upto2122 = insertelement <4 x float> %.upto1121, float %.i2104, i32 2
  %70 = insertelement <4 x float> %.upto2122, float %.i3105, i32 3
  %71 = getelementptr inbounds %struct.BVHMetadata, %struct.BVHMetadata* %0, i32 0, i32 1, i32 2
  store <4 x float> %70, <4 x float>* %71, align 4, !tbaa !218
  ret void
}

; Function Attrs: alwaysinline nounwind readnone
define <2 x i32> @"\01?PointerAdd@@YA?AV?$vector@I$01@@V1@I@Z"(<2 x i32>, i32) #6 {
  %3 = extractelement <2 x i32> %0, i32 0
  %4 = add i32 %3, %1
  %5 = insertelement <2 x i32> %0, i32 %4, i32 0
  ret <2 x i32> %5
}

; Function Attrs: alwaysinline nounwind
define void @"\01?LoadRaytracingInstanceDesc@@YA?AURaytracingInstanceDesc@@UByteAddressBuffer@@I@Z"(%struct.RaytracingInstanceDesc* noalias nocapture sret, %struct.ByteAddressBuffer* nocapture readonly, i32) #5 {
  %4 = load %struct.ByteAddressBuffer, %struct.ByteAddressBuffer* %1, align 4
  %5 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.ByteAddressBuffer(i32 160, %struct.ByteAddressBuffer %4)
  %6 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %5, i32 %2, i32 undef)
  %7 = extractvalue %dx.types.ResRet.i32 %6, 0
  %8 = extractvalue %dx.types.ResRet.i32 %6, 1
  %9 = extractvalue %dx.types.ResRet.i32 %6, 2
  %10 = extractvalue %dx.types.ResRet.i32 %6, 3
  %11 = add i32 %2, 16
  %12 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %5, i32 %11, i32 undef)
  %13 = extractvalue %dx.types.ResRet.i32 %12, 0
  %14 = extractvalue %dx.types.ResRet.i32 %12, 1
  %15 = extractvalue %dx.types.ResRet.i32 %12, 2
  %16 = extractvalue %dx.types.ResRet.i32 %12, 3
  %17 = add i32 %2, 32
  %18 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %5, i32 %17, i32 undef)
  %19 = extractvalue %dx.types.ResRet.i32 %18, 0
  %20 = extractvalue %dx.types.ResRet.i32 %18, 1
  %21 = extractvalue %dx.types.ResRet.i32 %18, 2
  %22 = extractvalue %dx.types.ResRet.i32 %18, 3
  %23 = add i32 %2, 48
  %24 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %5, i32 %23, i32 undef)
  %25 = extractvalue %dx.types.ResRet.i32 %24, 0
  %26 = extractvalue %dx.types.ResRet.i32 %24, 1
  %27 = extractvalue %dx.types.ResRet.i32 %24, 2
  %28 = extractvalue %dx.types.ResRet.i32 %24, 3
  %.i0 = bitcast i32 %7 to float
  %.i1 = bitcast i32 %8 to float
  %.i2 = bitcast i32 %9 to float
  %.i3 = bitcast i32 %10 to float
  %.upto0 = insertelement <4 x float> undef, float %.i0, i32 0
  %.upto1 = insertelement <4 x float> %.upto0, float %.i1, i32 1
  %.upto2 = insertelement <4 x float> %.upto1, float %.i2, i32 2
  %29 = insertelement <4 x float> %.upto2, float %.i3, i32 3
  %30 = getelementptr inbounds %struct.RaytracingInstanceDesc, %struct.RaytracingInstanceDesc* %0, i32 0, i32 0, i32 0
  store <4 x float> %29, <4 x float>* %30, align 4, !tbaa !218, !alias.scope !223
  %.i035 = bitcast i32 %13 to float
  %.i136 = bitcast i32 %14 to float
  %.i237 = bitcast i32 %15 to float
  %.i338 = bitcast i32 %16 to float
  %.upto043 = insertelement <4 x float> undef, float %.i035, i32 0
  %.upto144 = insertelement <4 x float> %.upto043, float %.i136, i32 1
  %.upto245 = insertelement <4 x float> %.upto144, float %.i237, i32 2
  %31 = insertelement <4 x float> %.upto245, float %.i338, i32 3
  %32 = getelementptr inbounds %struct.RaytracingInstanceDesc, %struct.RaytracingInstanceDesc* %0, i32 0, i32 0, i32 1
  store <4 x float> %31, <4 x float>* %32, align 4, !tbaa !218, !alias.scope !223
  %.i039 = bitcast i32 %19 to float
  %.i140 = bitcast i32 %20 to float
  %.i241 = bitcast i32 %21 to float
  %.i342 = bitcast i32 %22 to float
  %.upto046 = insertelement <4 x float> undef, float %.i039, i32 0
  %.upto147 = insertelement <4 x float> %.upto046, float %.i140, i32 1
  %.upto248 = insertelement <4 x float> %.upto147, float %.i241, i32 2
  %33 = insertelement <4 x float> %.upto248, float %.i342, i32 3
  %34 = getelementptr inbounds %struct.RaytracingInstanceDesc, %struct.RaytracingInstanceDesc* %0, i32 0, i32 0, i32 2
  store <4 x float> %33, <4 x float>* %34, align 4, !tbaa !218, !alias.scope !223
  %35 = getelementptr inbounds %struct.RaytracingInstanceDesc, %struct.RaytracingInstanceDesc* %0, i32 0, i32 1
  store i32 %25, i32* %35, align 4, !tbaa !221, !alias.scope !223
  %36 = getelementptr inbounds %struct.RaytracingInstanceDesc, %struct.RaytracingInstanceDesc* %0, i32 0, i32 2
  store i32 %26, i32* %36, align 4, !tbaa !221, !alias.scope !223
  %.upto049 = insertelement <2 x i32> undef, i32 %27, i32 0
  %37 = insertelement <2 x i32> %.upto049, i32 %28, i32 1
  %38 = getelementptr inbounds %struct.RaytracingInstanceDesc, %struct.RaytracingInstanceDesc* %0, i32 0, i32 3
  store <2 x i32> %37, <2 x i32>* %38, align 4, !tbaa !218, !alias.scope !223
  ret void
}

; Function Attrs: alwaysinline nounwind
define void @"\01?Fallback_IgnoreHit@@YAXXZ"() #5 {
  call void @"\01?Fallback_SetAnyHitResult@@YAXH@Z"(i32 0) #3
  ret void
}

; Function Attrs: alwaysinline nounwind
define void @"\01?RawDataToRaytracingInstanceDesc@@YA?AURaytracingInstanceDesc@@V?$vector@I$03@@000@Z"(%struct.RaytracingInstanceDesc* noalias nocapture sret, <4 x i32>, <4 x i32>, <4 x i32>, <4 x i32>) #5 {
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
  %6 = insertelement <4 x float> %.upto2, float %.i3, i32 3
  %7 = getelementptr inbounds %struct.RaytracingInstanceDesc, %struct.RaytracingInstanceDesc* %0, i32 0, i32 0, i32 0
  store <4 x float> %6, <4 x float>* %7, align 4, !tbaa !218
  %.i01 = bitcast i32 %b.i0 to float
  %.i12 = bitcast i32 %b.i1 to float
  %.i23 = bitcast i32 %b.i2 to float
  %.i34 = bitcast i32 %b.i3 to float
  %.upto09 = insertelement <4 x float> undef, float %.i01, i32 0
  %.upto110 = insertelement <4 x float> %.upto09, float %.i12, i32 1
  %.upto211 = insertelement <4 x float> %.upto110, float %.i23, i32 2
  %8 = insertelement <4 x float> %.upto211, float %.i34, i32 3
  %9 = getelementptr inbounds %struct.RaytracingInstanceDesc, %struct.RaytracingInstanceDesc* %0, i32 0, i32 0, i32 1
  store <4 x float> %8, <4 x float>* %9, align 4, !tbaa !218
  %.i05 = bitcast i32 %c.i0 to float
  %.i16 = bitcast i32 %c.i1 to float
  %.i27 = bitcast i32 %c.i2 to float
  %.i38 = bitcast i32 %c.i3 to float
  %.upto012 = insertelement <4 x float> undef, float %.i05, i32 0
  %.upto113 = insertelement <4 x float> %.upto012, float %.i16, i32 1
  %.upto214 = insertelement <4 x float> %.upto113, float %.i27, i32 2
  %10 = insertelement <4 x float> %.upto214, float %.i38, i32 3
  %11 = getelementptr inbounds %struct.RaytracingInstanceDesc, %struct.RaytracingInstanceDesc* %0, i32 0, i32 0, i32 2
  store <4 x float> %10, <4 x float>* %11, align 4, !tbaa !218
  %12 = extractelement <4 x i32> %4, i32 0
  %13 = getelementptr inbounds %struct.RaytracingInstanceDesc, %struct.RaytracingInstanceDesc* %0, i32 0, i32 1
  store i32 %12, i32* %13, align 4, !tbaa !221
  %14 = extractelement <4 x i32> %4, i32 1
  %15 = getelementptr inbounds %struct.RaytracingInstanceDesc, %struct.RaytracingInstanceDesc* %0, i32 0, i32 2
  store i32 %14, i32* %15, align 4, !tbaa !221
  %.upto03 = insertelement <2 x i32> undef, i32 %18, i32 0
  %16 = insertelement <2 x i32> %.upto03, i32 %17, i32 1
  %17 = extractelement <4 x i32> %4, i32 3
  %18 = extractelement <4 x i32> %4, i32 2
  %19 = getelementptr inbounds %struct.RaytracingInstanceDesc, %struct.RaytracingInstanceDesc* %0, i32 0, i32 3
  store <2 x i32> %16, <2 x i32>* %19, align 4, !tbaa !218
  ret void
}

; Function Attrs: alwaysinline nounwind
define void @"\01?BoundingBoxToAABB@@YA?AUAABB@@UBoundingBox@@@Z"(%struct.AABB* noalias nocapture sret, %struct.BoundingBox* nocapture readonly) #5 {
  %3 = getelementptr inbounds %struct.BoundingBox, %struct.BoundingBox* %1, i32 0, i32 0
  %4 = load <3 x float>, <3 x float>* %3, align 4, !tbaa !218
  %.i01 = extractelement <3 x float> %4, i32 0
  %.i13 = extractelement <3 x float> %4, i32 1
  %.i25 = extractelement <3 x float> %4, i32 2
  %5 = getelementptr inbounds %struct.BoundingBox, %struct.BoundingBox* %1, i32 0, i32 1
  %6 = load <3 x float>, <3 x float>* %5, align 4, !tbaa !218
  %.i0 = extractelement <3 x float> %6, i32 0
  %.i02 = fsub fast float %.i01, %.i0
  %.i1 = extractelement <3 x float> %6, i32 1
  %.i14 = fsub fast float %.i13, %.i1
  %.i2 = extractelement <3 x float> %6, i32 2
  %.i26 = fsub fast float %.i25, %.i2
  %.upto0 = insertelement <3 x float> undef, float %.i02, i32 0
  %.upto1 = insertelement <3 x float> %.upto0, float %.i14, i32 1
  %7 = insertelement <3 x float> %.upto1, float %.i26, i32 2
  %8 = getelementptr inbounds %struct.AABB, %struct.AABB* %0, i32 0, i32 0
  store <3 x float> %7, <3 x float>* %8, align 4, !tbaa !218
  %.i09 = fadd fast float %.i0, %.i01
  %.i112 = fadd fast float %.i1, %.i13
  %.i215 = fadd fast float %.i2, %.i25
  %.upto016 = insertelement <3 x float> undef, float %.i09, i32 0
  %.upto117 = insertelement <3 x float> %.upto016, float %.i112, i32 1
  %9 = insertelement <3 x float> %.upto117, float %.i215, i32 2
  %10 = getelementptr inbounds %struct.AABB, %struct.AABB* %0, i32 0, i32 1
  store <3 x float> %9, <3 x float>* %10, align 4, !tbaa !218
  ret void
}

; Function Attrs: alwaysinline nounwind readnone
define <2 x i32> @"\01?CreateFlag@@YA?AV?$vector@I$01@@II@Z"(i32, i32) #6 {
  %3 = and i32 %0, 16777215
  %4 = insertelement <2 x i32> undef, i32 %3, i32 0
  %5 = insertelement <2 x i32> %4, i32 %1, i32 1
  ret <2 x i32> %5
}

; Function Attrs: alwaysinline nounwind
define void @"\01?BVHReadTriangleMetadata@@YA?AUTriangleMetaData@@URWByteAddressBufferPointer@@H@Z"(%struct.TriangleMetaData* noalias nocapture sret, %struct.RWByteAddressBufferPointer* nocapture readonly, i32) #5 {
  %4 = getelementptr inbounds %struct.RWByteAddressBufferPointer, %struct.RWByteAddressBufferPointer* %1, i32 0, i32 1
  %5 = load i32, i32* %4, align 4, !tbaa !221
  %6 = add i32 %5, 8
  %7 = getelementptr inbounds %struct.RWByteAddressBufferPointer, %struct.RWByteAddressBufferPointer* %1, i32 0, i32 0
  %8 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %7, align 4
  %9 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %8)
  %10 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %9, i32 %6, i32 undef)
  %11 = extractvalue %dx.types.ResRet.i32 %10, 0
  %12 = shl i32 %2, 3
  %13 = add i32 %5, %12
  %14 = add i32 %13, %11
  %15 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %9, i32 %14, i32 undef)
  %16 = extractvalue %dx.types.ResRet.i32 %15, 0
  %17 = extractvalue %dx.types.ResRet.i32 %15, 1
  %18 = getelementptr inbounds %struct.TriangleMetaData, %struct.TriangleMetaData* %0, i32 0, i32 0
  store i32 %16, i32* %18, align 4, !tbaa !221
  %19 = getelementptr inbounds %struct.TriangleMetaData, %struct.TriangleMetaData* %0, i32 0, i32 1
  store i32 %17, i32* %19, align 4, !tbaa !221
  ret void
}

; Function Attrs: alwaysinline nounwind
define i1 @"\01?Inverse2x2@@YA_NV?$matrix@M$01$01@@AIAV1@@Z"(%class.matrix.float.2.2, %class.matrix.float.2.2* noalias nocapture dereferenceable(16)) #5 {
  %3 = extractvalue %class.matrix.float.2.2 %0, 0, 0
  %4 = extractelement <2 x float> %3, i32 1
  %5 = extractelement <2 x float> %3, i32 0
  %6 = extractvalue %class.matrix.float.2.2 %0, 0, 1
  %7 = extractelement <2 x float> %6, i32 1
  %8 = extractelement <2 x float> %6, i32 0
  %9 = fmul fast float %5, %7
  %10 = fmul fast float %8, %4
  %11 = fsub fast float %9, %10
  %12 = fdiv fast float 1.000000e+00, %11
  %.i0 = fmul fast float %12, %7
  %13 = fmul fast float %12, %8
  %.i1 = fsub fast float -0.000000e+00, %13
  %14 = fmul fast float %12, %4
  %.i2 = fsub fast float -0.000000e+00, %14
  %.i3 = fmul fast float %12, %5
  %.upto0 = insertelement <4 x float> undef, float %.i0, i32 0
  %.upto1 = insertelement <4 x float> %.upto0, float %.i1, i32 1
  %.upto2 = insertelement <4 x float> %.upto1, float %.i2, i32 2
  %15 = insertelement <4 x float> %.upto2, float %.i3, i32 3
  %FAbs = call float @dx.op.unary.f32(i32 6, float %11)
  %16 = fcmp fast ogt float %FAbs, 0x3E45798EE0000000
  %17 = bitcast %class.matrix.float.2.2* %1 to <4 x float>*
  store <4 x float> %15, <4 x float>* %17, align 4
  ret i1 %16
}

; Function Attrs: alwaysinline nounwind readnone
define i32 @"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z"(<3 x float>) #6 {
  %2 = extractelement <3 x float> %0, i32 0
  %3 = extractelement <3 x float> %0, i32 1
  %4 = fcmp fast ogt float %2, %3
  %5 = extractelement <3 x float> %0, i32 2
  %6 = fcmp fast ogt float %2, %5
  %7 = and i1 %4, %6
  br i1 %7, label %11, label %8

; <label>:8                                       ; preds = %1
  %9 = fcmp fast ogt float %3, %5
  br i1 %9, label %11, label %10

; <label>:10                                      ; preds = %8
  br label %11

; <label>:11                                      ; preds = %10, %8, %1
  %.0 = phi i32 [ 2, %10 ], [ 0, %1 ], [ 1, %8 ]
  ret i32 %.0
}

; Function Attrs: alwaysinline nounwind
define i1 @"\01?TestLeafNodeIntersections@@YA_NURWByteAddressBufferPointer@@V?$vector@I$01@@IV?$vector@M$02@@2V?$vector@H$02@@2AIAV?$vector@M$01@@AIAMAIAI@Z"(%struct.RWByteAddressBufferPointer* nocapture readonly, <2 x i32>, i32, <3 x float>, <3 x float>, <3 x i32>, <3 x float>, <2 x float>* noalias nocapture dereferenceable(8), float* noalias nocapture dereferenceable(4), i32* noalias nocapture dereferenceable(4)) #5 {
  %shear.i0 = extractelement <3 x float> %6, i32 0
  %shear.i1 = extractelement <3 x float> %6, i32 1
  %rayOrigin.i0 = extractelement <3 x float> %3, i32 0
  %rayOrigin.i1 = extractelement <3 x float> %3, i32 1
  %rayOrigin.i2 = extractelement <3 x float> %3, i32 2
  %11 = load i32, i32* %9, align 4
  %12 = load <2 x float>, <2 x float>* %7, align 4
  %.i038 = extractelement <2 x float> %12, i32 0
  %.i139 = extractelement <2 x float> %12, i32 1
  %13 = alloca [3 x float], align 4
  %14 = alloca [3 x float], align 4
  %15 = alloca [3 x float], align 4
  %16 = extractelement <2 x i32> %1, i32 0
  %17 = and i32 %16, 16777215
  %18 = getelementptr inbounds %struct.RWByteAddressBufferPointer, %struct.RWByteAddressBufferPointer* %0, i32 0, i32 1
  %19 = load i32, i32* %18, align 4, !tbaa !221, !noalias !226
  %20 = add i32 %19, 4
  %21 = getelementptr inbounds %struct.RWByteAddressBufferPointer, %struct.RWByteAddressBufferPointer* %0, i32 0, i32 0
  %22 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %21, align 4, !noalias !226
  %23 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %22)
  %24 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %23, i32 %20, i32 undef)
  %25 = extractvalue %dx.types.ResRet.i32 %24, 0
  %26 = mul nuw nsw i32 %17, 36
  %27 = add i32 %19, %26
  %28 = add i32 %27, %25
  %29 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %23, i32 %28, i32 undef)
  %30 = extractvalue %dx.types.ResRet.i32 %29, 0
  %31 = extractvalue %dx.types.ResRet.i32 %29, 1
  %32 = extractvalue %dx.types.ResRet.i32 %29, 2
  %33 = extractvalue %dx.types.ResRet.i32 %29, 3
  %.i0 = bitcast i32 %30 to float
  %.i1 = bitcast i32 %31 to float
  %.i2 = bitcast i32 %32 to float
  %.i3 = bitcast i32 %33 to float
  %34 = add i32 %28, 16
  %35 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %23, i32 %34, i32 undef)
  %36 = extractvalue %dx.types.ResRet.i32 %35, 0
  %37 = extractvalue %dx.types.ResRet.i32 %35, 1
  %38 = extractvalue %dx.types.ResRet.i32 %35, 2
  %39 = extractvalue %dx.types.ResRet.i32 %35, 3
  %.i010 = bitcast i32 %36 to float
  %.i111 = bitcast i32 %37 to float
  %.i212 = bitcast i32 %38 to float
  %.i313 = bitcast i32 %39 to float
  %40 = add i32 %28, 32
  %41 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %23, i32 %40, i32 undef)
  %42 = extractvalue %dx.types.ResRet.i32 %41, 0
  %43 = bitcast i32 %42 to float
  %44 = load float, float* %8, align 4, !tbaa !231
  %45 = and i32 %2, 1
  %46 = icmp eq i32 %45, 0
  %47 = shl i32 %2, 3
  %48 = and i32 %47, 16
  %49 = add nuw nsw i32 %48, 16
  %50 = xor i32 %48, 16
  %51 = add nuw nsw i32 %50, 16
  %RayFlags9 = call i32 @dx.op.rayFlags.i32(i32 144)
  %52 = and i32 %RayFlags9, %49
  %53 = icmp ne i32 %52, 0
  %54 = and i1 %46, %53
  %55 = and i32 %RayFlags9, %51
  %56 = icmp ne i32 %55, 0
  %57 = and i1 %46, %56
  %.i014 = fsub fast float %.i0, %rayOrigin.i0
  %.i115 = fsub fast float %.i1, %rayOrigin.i1
  %.i216 = fsub fast float %.i2, %rayOrigin.i2
  %58 = getelementptr inbounds [3 x float], [3 x float]* %15, i32 0, i32 0
  store float %.i014, float* %58, align 4
  %59 = getelementptr inbounds [3 x float], [3 x float]* %15, i32 0, i32 1
  store float %.i115, float* %59, align 4
  %60 = getelementptr inbounds [3 x float], [3 x float]* %15, i32 0, i32 2
  store float %.i216, float* %60, align 4
  %61 = extractelement <3 x i32> %5, i32 0
  %62 = getelementptr [3 x float], [3 x float]* %15, i32 0, i32 %61
  %63 = load float, float* %62, align 4, !tbaa !231, !noalias !233
  %64 = extractelement <3 x i32> %5, i32 1
  %65 = getelementptr [3 x float], [3 x float]* %15, i32 0, i32 %64
  %66 = load float, float* %65, align 4, !tbaa !231, !noalias !233
  %67 = extractelement <3 x i32> %5, i32 2
  %68 = getelementptr [3 x float], [3 x float]* %15, i32 0, i32 %67
  %69 = load float, float* %68, align 4, !tbaa !231, !noalias !233
  %.i017 = fsub fast float %.i3, %rayOrigin.i0
  %.i118 = fsub fast float %.i010, %rayOrigin.i1
  %.i219 = fsub fast float %.i111, %rayOrigin.i2
  %70 = getelementptr inbounds [3 x float], [3 x float]* %13, i32 0, i32 0
  store float %.i017, float* %70, align 4
  %71 = getelementptr inbounds [3 x float], [3 x float]* %13, i32 0, i32 1
  store float %.i118, float* %71, align 4
  %72 = getelementptr inbounds [3 x float], [3 x float]* %13, i32 0, i32 2
  store float %.i219, float* %72, align 4
  %73 = getelementptr [3 x float], [3 x float]* %13, i32 0, i32 %61
  %74 = load float, float* %73, align 4, !tbaa !231, !noalias !233
  %75 = getelementptr [3 x float], [3 x float]* %13, i32 0, i32 %64
  %76 = load float, float* %75, align 4, !tbaa !231, !noalias !233
  %77 = getelementptr [3 x float], [3 x float]* %13, i32 0, i32 %67
  %78 = load float, float* %77, align 4, !tbaa !231, !noalias !233
  %.i020 = fsub fast float %.i212, %rayOrigin.i0
  %.i121 = fsub fast float %.i313, %rayOrigin.i1
  %.i222 = fsub fast float %43, %rayOrigin.i2
  %79 = getelementptr inbounds [3 x float], [3 x float]* %14, i32 0, i32 0
  store float %.i020, float* %79, align 4
  %80 = getelementptr inbounds [3 x float], [3 x float]* %14, i32 0, i32 1
  store float %.i121, float* %80, align 4
  %81 = getelementptr inbounds [3 x float], [3 x float]* %14, i32 0, i32 2
  store float %.i222, float* %81, align 4
  %82 = getelementptr [3 x float], [3 x float]* %14, i32 0, i32 %61
  %83 = load float, float* %82, align 4, !tbaa !231, !noalias !233
  %84 = getelementptr [3 x float], [3 x float]* %14, i32 0, i32 %64
  %85 = load float, float* %84, align 4, !tbaa !231, !noalias !233
  %86 = getelementptr [3 x float], [3 x float]* %14, i32 0, i32 %67
  %87 = load float, float* %86, align 4, !tbaa !231, !noalias !233
  %.i023 = fmul float %shear.i0, %69
  %.i124 = fmul float %shear.i1, %69
  %.i026 = fsub float %63, %.i023
  %.i127 = fsub float %66, %.i124
  %.i028 = fmul float %shear.i0, %78
  %.i129 = fmul float %shear.i1, %78
  %.i031 = fsub float %74, %.i028
  %.i132 = fsub float %76, %.i129
  %.i033 = fmul float %shear.i0, %87
  %.i134 = fmul float %shear.i1, %87
  %.i036 = fsub float %83, %.i033
  %.i137 = fsub float %85, %.i134
  %88 = fmul float %.i132, %.i036
  %89 = fmul float %.i031, %.i137
  %90 = fsub float %88, %89
  %91 = fmul float %.i026, %.i137
  %92 = fmul float %.i127, %.i036
  %93 = fsub float %91, %92
  %94 = fmul float %.i127, %.i031
  %95 = fmul float %.i026, %.i132
  %96 = fsub float %94, %95
  %97 = fadd fast float %93, %96
  %98 = fadd fast float %97, %90
  br i1 %57, label %99, label %105

; <label>:99                                      ; preds = %10
  %100 = fcmp fast ogt float %90, 0.000000e+00
  %101 = fcmp fast ogt float %93, 0.000000e+00
  %102 = or i1 %100, %101
  %103 = fcmp fast ogt float %96, 0.000000e+00
  %104 = or i1 %103, %102
  %.old.old = fcmp fast oeq float %98, 0.000000e+00
  %or.cond5 = or i1 %104, %.old.old
  br i1 %or.cond5, label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit", label %125

; <label>:105                                     ; preds = %10
  br i1 %54, label %106, label %112

; <label>:106                                     ; preds = %105
  %107 = fcmp fast olt float %90, 0.000000e+00
  %108 = fcmp fast olt float %93, 0.000000e+00
  %109 = or i1 %107, %108
  %110 = fcmp fast olt float %96, 0.000000e+00
  %111 = or i1 %110, %109
  %.old = fcmp fast oeq float %98, 0.000000e+00
  %or.cond4 = or i1 %111, %.old
  br i1 %or.cond4, label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit", label %125

; <label>:112                                     ; preds = %105
  %113 = fcmp fast olt float %90, 0.000000e+00
  %114 = fcmp fast olt float %93, 0.000000e+00
  %115 = or i1 %113, %114
  %116 = fcmp fast olt float %96, 0.000000e+00
  %117 = or i1 %116, %115
  %118 = fcmp fast ogt float %90, 0.000000e+00
  %119 = fcmp fast ogt float %93, 0.000000e+00
  %120 = or i1 %118, %119
  %121 = fcmp fast ogt float %96, 0.000000e+00
  %122 = or i1 %121, %120
  %123 = and i1 %117, %122
  %124 = fcmp fast oeq float %98, 0.000000e+00
  %or.cond = or i1 %124, %123
  br i1 %or.cond, label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit", label %125

; <label>:125                                     ; preds = %112, %106, %99
  %126 = extractelement <3 x float> %6, i32 2
  %127 = fmul fast float %69, %126
  %128 = fmul fast float %78, %126
  %129 = fmul fast float %87, %126
  %130 = fmul fast float %127, %90
  %131 = fmul fast float %128, %93
  %132 = fmul fast float %129, %96
  %133 = fadd fast float %131, %132
  %134 = fadd fast float %133, %130
  br i1 %57, label %135, label %140

; <label>:135                                     ; preds = %125
  %136 = fcmp fast ogt float %134, 0.000000e+00
  %137 = fmul fast float %98, %44
  %138 = fcmp fast olt float %134, %137
  %139 = or i1 %136, %138
  br i1 %139, label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit", label %157

; <label>:140                                     ; preds = %125
  br i1 %54, label %141, label %146

; <label>:141                                     ; preds = %140
  %142 = fcmp fast olt float %134, 0.000000e+00
  %143 = fmul fast float %98, %44
  %144 = fcmp fast ogt float %134, %143
  %145 = or i1 %142, %144
  br i1 %145, label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit", label %157

; <label>:146                                     ; preds = %140
  %147 = fcmp fast ogt float %98, 0.000000e+00
  %148 = select i1 %147, i32 1, i32 -1
  %149 = bitcast float %134 to i32
  %150 = xor i32 %149, %148
  %151 = uitofp i32 %150 to float
  %152 = bitcast float %98 to i32
  %153 = xor i32 %148, %152
  %154 = uitofp i32 %153 to float
  %155 = fmul fast float %154, %44
  %156 = fcmp fast ogt float %151, %155
  br i1 %156, label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit", label %157

; <label>:157                                     ; preds = %146, %141, %135
  %158 = fdiv fast float 1.000000e+00, %98
  %159 = fmul fast float %158, %93
  %160 = fmul fast float %158, %96
  %161 = fmul fast float %158, %134
  br label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit"

"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit": ; preds = %157, %146, %141, %135, %112, %106, %99
  %.03.i0 = phi float [ undef, %99 ], [ undef, %135 ], [ %159, %157 ], [ undef, %141 ], [ undef, %146 ], [ undef, %106 ], [ undef, %112 ]
  %.03.i1 = phi float [ undef, %99 ], [ undef, %135 ], [ %160, %157 ], [ undef, %141 ], [ undef, %146 ], [ undef, %106 ], [ undef, %112 ]
  %.02 = phi float [ %44, %99 ], [ %44, %135 ], [ %161, %157 ], [ %44, %141 ], [ %44, %146 ], [ %44, %106 ], [ %44, %112 ]
  %162 = fcmp fast olt float %.02, %44
  %RayTMin = call float @dx.op.rayTMin.f32(i32 153)
  %163 = fcmp fast ogt float %.02, %RayTMin
  %164 = and i1 %162, %163
  br i1 %164, label %165, label %166

; <label>:165                                     ; preds = %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit"
  store float %.02, float* %8, align 4, !tbaa !231
  br label %166

; <label>:166                                     ; preds = %165, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit"
  %bIsIntersect.0 = phi i1 [ true, %165 ], [ false, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit" ]
  %.01.i0 = phi float [ %.03.i0, %165 ], [ %.i038, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit" ]
  %.01.i1 = phi float [ %.03.i1, %165 ], [ %.i139, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit" ]
  %.0 = phi i32 [ %17, %165 ], [ %11, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit" ]
  %.01.upto0 = insertelement <2 x float> undef, float %.01.i0, i32 0
  %.01 = insertelement <2 x float> %.01.upto0, float %.01.i1, i32 1
  store <2 x float> %.01, <2 x float>* %7, align 4
  store i32 %.0, i32* %9, align 4
  ret i1 %bIsIntersect.0
}

; Function Attrs: alwaysinline nounwind
define i1 @"\01?Traverse@@YA_NIII@Z"(i32, i32, i32) #5 {
  %4 = load %AccelerationStructureList, %AccelerationStructureList* @AccelerationStructureList, align 4
  %AccelerationStructureList = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.AccelerationStructureList(i32 160, %AccelerationStructureList %4)
  %5 = alloca [3 x float], align 4
  %6 = alloca [3 x float], align 4
  %7 = alloca [3 x float], align 4
  %8 = alloca [3 x float], align 4
  %9 = alloca [3 x float], align 4
  %nodesToProcess = alloca [2 x i32], align 4
  %attr = alloca %struct.BuiltInTriangleIntersectionAttributes, align 4
  %10 = call i32 @"\01?Fallback_GroupIndex@@YAIXZ"() #3
  %WorldRayDirection = call float @dx.op.worldRayDirection.f32(i32 148, i8 0)
  %WorldRayDirection280 = call float @dx.op.worldRayDirection.f32(i32 148, i8 1)
  %WorldRayDirection281 = call float @dx.op.worldRayDirection.f32(i32 148, i8 2)
  %WorldRayOrigin = call float @dx.op.worldRayOrigin.f32(i32 147, i8 0)
  %WorldRayOrigin275 = call float @dx.op.worldRayOrigin.f32(i32 147, i8 1)
  %WorldRayOrigin276 = call float @dx.op.worldRayOrigin.f32(i32 147, i8 2)
  %11 = getelementptr inbounds [3 x float], [3 x float]* %9, i32 0, i32 0
  store float %WorldRayDirection, float* %11, align 4
  %12 = getelementptr inbounds [3 x float], [3 x float]* %9, i32 0, i32 1
  store float %WorldRayDirection280, float* %12, align 4
  %13 = getelementptr inbounds [3 x float], [3 x float]* %9, i32 0, i32 2
  store float %WorldRayDirection281, float* %13, align 4
  %.i0 = fdiv fast float 1.000000e+00, %WorldRayDirection
  %.i1 = fdiv fast float 1.000000e+00, %WorldRayDirection280
  %.i2 = fdiv fast float 1.000000e+00, %WorldRayDirection281
  %.i0325 = fmul fast float %.i0, %WorldRayOrigin
  %.i1326 = fmul fast float %.i1, %WorldRayOrigin275
  %.i2327 = fmul fast float %.i2, %WorldRayOrigin276
  %FAbs272 = call float @dx.op.unary.f32(i32 6, float %WorldRayDirection)
  %FAbs273 = call float @dx.op.unary.f32(i32 6, float %WorldRayDirection280)
  %FAbs274 = call float @dx.op.unary.f32(i32 6, float %WorldRayDirection281)
  %14 = fcmp fast ogt float %FAbs272, %FAbs273
  %15 = fcmp fast ogt float %FAbs272, %FAbs274
  %16 = and i1 %14, %15
  br i1 %16, label %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i", label %17

; <label>:17                                      ; preds = %3
  %18 = fcmp fast ogt float %FAbs273, %FAbs274
  br i1 %18, label %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i", label %19

; <label>:19                                      ; preds = %17
  br label %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i"

"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i": ; preds = %19, %17, %3
  %.0203 = phi i32 [ 2, %19 ], [ 0, %3 ], [ 1, %17 ]
  %20 = add nuw nsw i32 %.0203, 1
  %21 = urem i32 %20, 3
  %22 = add nuw nsw i32 %.0203, 2
  %23 = urem i32 %22, 3
  %24 = getelementptr [3 x float], [3 x float]* %9, i32 0, i32 %.0203
  %25 = load float, float* %24, align 4, !tbaa !231, !noalias !237
  %26 = fcmp fast olt float %25, 0.000000e+00
  %worldRayData.5.0.i0 = select i1 %26, i32 %23, i32 %21
  %worldRayData.5.0.i1 = select i1 %26, i32 %21, i32 %23
  %27 = getelementptr [3 x float], [3 x float]* %9, i32 0, i32 %worldRayData.5.0.i0
  %28 = load float, float* %27, align 4, !tbaa !231, !noalias !237
  %29 = fdiv float %28, %25
  %30 = getelementptr [3 x float], [3 x float]* %9, i32 0, i32 %worldRayData.5.0.i1
  %31 = load float, float* %30, align 4, !tbaa !231, !noalias !237
  %32 = fdiv float %31, %25
  %33 = fdiv float 1.000000e+00, %25
  %34 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %AccelerationStructureList, i32 0)
  %35 = extractvalue %dx.types.CBufRet.i32 %34, 0
  %36 = extractvalue %dx.types.CBufRet.i32 %34, 1
  %37 = getelementptr inbounds [2 x i32], [2 x i32]* %nodesToProcess, i32 0, i32 0
  store i32 0, i32* %37, align 4, !tbaa !221
  %38 = getelementptr inbounds [0 x %struct.RWByteAddressBuffer], [0 x %struct.RWByteAddressBuffer]* @"\01?DescriptorHeapBufferTable@@3PAURWByteAddressBuffer@@A", i32 0, i32 %36, !dx.nonuniform !240
  %39 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %38, align 4, !noalias !125
  %40 = add i32 %35, 4
  %41 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %39)
  %42 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %41, i32 %40, i32 undef)
  %43 = extractvalue %dx.types.ResRet.i32 %42, 0
  %44 = add i32 %43, %35
  %45 = add i32 %35, 16
  %46 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %41, i32 %45, i32 undef)
  %47 = extractvalue %dx.types.ResRet.i32 %46, 0
  %48 = extractvalue %dx.types.ResRet.i32 %46, 1
  %49 = extractvalue %dx.types.ResRet.i32 %46, 2
  %50 = add i32 %35, 32
  %51 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %41, i32 %50, i32 undef)
  %52 = extractvalue %dx.types.ResRet.i32 %51, 0
  %53 = extractvalue %dx.types.ResRet.i32 %51, 1
  %54 = extractvalue %dx.types.ResRet.i32 %51, 2
  %55 = bitcast i32 %47 to float
  %56 = bitcast i32 %48 to float
  %57 = bitcast i32 %49 to float
  %58 = bitcast i32 %52 to float
  %59 = bitcast i32 %53 to float
  %60 = bitcast i32 %54 to float
  %RayTCurrent = call float @dx.op.rayTCurrent.f32(i32 154)
  %61 = fsub fast float %55, %WorldRayOrigin
  %62 = fmul fast float %61, %.i0
  %63 = fsub fast float %56, %WorldRayOrigin275
  %64 = fmul fast float %63, %.i1
  %65 = fsub fast float %57, %WorldRayOrigin276
  %66 = fmul fast float %65, %.i2
  %FAbs269 = call float @dx.op.unary.f32(i32 6, float %.i0)
  %FAbs270 = call float @dx.op.unary.f32(i32 6, float %.i1)
  %FAbs271 = call float @dx.op.unary.f32(i32 6, float %.i2)
  %.i0334 = fmul fast float %58, %FAbs269
  %.i1335 = fmul fast float %FAbs270, %59
  %.i2336 = fmul fast float %FAbs271, %60
  %.i0337 = fadd fast float %.i0334, %62
  %.i1338 = fadd fast float %.i1335, %64
  %.i2339 = fadd fast float %.i2336, %66
  %.i0343 = fsub fast float %62, %.i0334
  %.i1344 = fsub fast float %64, %.i1335
  %.i2345 = fsub fast float %66, %.i2336
  %FMax306 = call float @dx.op.binary.f32(i32 35, float %.i0343, float %.i1344)
  %FMax305 = call float @dx.op.binary.f32(i32 35, float %FMax306, float %.i2345)
  %FMin304 = call float @dx.op.binary.f32(i32 36, float %.i0337, float %.i1338)
  %FMin303 = call float @dx.op.binary.f32(i32 36, float %FMin304, float %.i2339)
  %FMax302 = call float @dx.op.binary.f32(i32 35, float %FMax305, float 0.000000e+00)
  %FMin301 = call float @dx.op.binary.f32(i32 36, float %FMin303, float %RayTCurrent)
  %67 = fcmp fast olt float %FMax302, %FMin301
  br i1 %67, label %.lr.ph.preheader.critedge, label %83

.lr.ph.preheader.critedge:                        ; preds = %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i"
  %68 = getelementptr [2048 x i32], [2048 x i32] addrspace(3)* @"\01?stack@@3PAIA", i32 0, i32 %10
  store i32 0, i32 addrspace(3)* %68, align 4, !tbaa !221, !noalias !241
  store i32 1, i32* %37, align 4, !tbaa !221
  call void @"\01?Fallback_SetInstanceIndex@@YAXI@Z"(i32 -1) #3
  %69 = getelementptr inbounds [3 x float], [3 x float]* %8, i32 0, i32 0
  %70 = getelementptr inbounds [3 x float], [3 x float]* %8, i32 0, i32 1
  %71 = getelementptr inbounds [3 x float], [3 x float]* %8, i32 0, i32 2
  %72 = getelementptr inbounds [2 x i32], [2 x i32]* %nodesToProcess, i32 0, i32 1
  %73 = getelementptr inbounds [3 x float], [3 x float]* %7, i32 0, i32 0
  %74 = getelementptr inbounds [3 x float], [3 x float]* %7, i32 0, i32 1
  %75 = getelementptr inbounds [3 x float], [3 x float]* %7, i32 0, i32 2
  %76 = getelementptr inbounds [3 x float], [3 x float]* %5, i32 0, i32 0
  %77 = getelementptr inbounds [3 x float], [3 x float]* %5, i32 0, i32 1
  %78 = getelementptr inbounds [3 x float], [3 x float]* %5, i32 0, i32 2
  %79 = getelementptr inbounds [3 x float], [3 x float]* %6, i32 0, i32 0
  %80 = getelementptr inbounds [3 x float], [3 x float]* %6, i32 0, i32 1
  %81 = getelementptr inbounds [3 x float], [3 x float]* %6, i32 0, i32 2
  %82 = getelementptr inbounds %struct.BuiltInTriangleIntersectionAttributes, %struct.BuiltInTriangleIntersectionAttributes* %attr, i32 0, i32 0
  br label %.lr.ph

; <label>:83                                      ; preds = %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i"
  call void @"\01?Fallback_SetInstanceIndex@@YAXI@Z"(i32 -1) #3
  br label %._crit_edge

.lr.ph:                                           ; preds = %426, %.lr.ph.preheader.critedge
  %resultTriId.0220 = phi i32 [ %resultTriId.2, %426 ], [ undef, %.lr.ph.preheader.critedge ]
  %resultBary.0219.i0 = phi float [ %resultBary.2.i0, %426 ], [ undef, %.lr.ph.preheader.critedge ]
  %resultBary.0219.i1 = phi float [ %resultBary.2.i1, %426 ], [ undef, %.lr.ph.preheader.critedge ]
  %stackPointer.1217 = phi i32 [ %stackPointer.3, %426 ], [ 1, %.lr.ph.preheader.critedge ]
  %instId.0216 = phi i32 [ %instId.2, %426 ], [ 0, %.lr.ph.preheader.critedge ]
  %instOffset.0215 = phi i32 [ %instOffset.2, %426 ], [ 0, %.lr.ph.preheader.critedge ]
  %instFlags.0214 = phi i32 [ %instFlags.2, %426 ], [ 0, %.lr.ph.preheader.critedge ]
  %instIdx.0213 = phi i32 [ %instIdx.2, %426 ], [ 0, %.lr.ph.preheader.critedge ]
  %currentGpuVA.0212.i0 = phi i32 [ %429, %426 ], [ %35, %.lr.ph.preheader.critedge ]
  %currentGpuVA.0212.i1 = phi i32 [ %430, %426 ], [ %36, %.lr.ph.preheader.critedge ]
  %currentBVHIndex.0211 = phi i32 [ %427, %426 ], [ 0, %.lr.ph.preheader.critedge ]
  %CurrentWorldToObject200.0210.i0 = phi float [ %CurrentWorldToObject200.2.i0, %426 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentWorldToObject200.0210.i1 = phi float [ %CurrentWorldToObject200.2.i1, %426 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentWorldToObject200.0210.i2 = phi float [ %CurrentWorldToObject200.2.i2, %426 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentWorldToObject200.0210.i3 = phi float [ %CurrentWorldToObject200.2.i3, %426 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentWorldToObject200.0210.i4 = phi float [ %CurrentWorldToObject200.2.i4, %426 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentWorldToObject200.0210.i5 = phi float [ %CurrentWorldToObject200.2.i5, %426 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentWorldToObject200.0210.i6 = phi float [ %CurrentWorldToObject200.2.i6, %426 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentWorldToObject200.0210.i7 = phi float [ %CurrentWorldToObject200.2.i7, %426 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentWorldToObject200.0210.i8 = phi float [ %CurrentWorldToObject200.2.i8, %426 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentWorldToObject200.0210.i9 = phi float [ %CurrentWorldToObject200.2.i9, %426 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentWorldToObject200.0210.i10 = phi float [ %CurrentWorldToObject200.2.i10, %426 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentWorldToObject200.0210.i11 = phi float [ %CurrentWorldToObject200.2.i11, %426 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentObjectToWorld199.0209.i0 = phi float [ %CurrentObjectToWorld199.2.i0, %426 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentObjectToWorld199.0209.i1 = phi float [ %CurrentObjectToWorld199.2.i1, %426 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentObjectToWorld199.0209.i2 = phi float [ %CurrentObjectToWorld199.2.i2, %426 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentObjectToWorld199.0209.i3 = phi float [ %CurrentObjectToWorld199.2.i3, %426 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentObjectToWorld199.0209.i4 = phi float [ %CurrentObjectToWorld199.2.i4, %426 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentObjectToWorld199.0209.i5 = phi float [ %CurrentObjectToWorld199.2.i5, %426 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentObjectToWorld199.0209.i6 = phi float [ %CurrentObjectToWorld199.2.i6, %426 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentObjectToWorld199.0209.i7 = phi float [ %CurrentObjectToWorld199.2.i7, %426 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentObjectToWorld199.0209.i8 = phi float [ %CurrentObjectToWorld199.2.i8, %426 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentObjectToWorld199.0209.i9 = phi float [ %CurrentObjectToWorld199.2.i9, %426 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentObjectToWorld199.0209.i10 = phi float [ %CurrentObjectToWorld199.2.i10, %426 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentObjectToWorld199.0209.i11 = phi float [ %CurrentObjectToWorld199.2.i11, %426 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %.0206208.i0 = phi float [ %.3.i0, %426 ], [ undef, %.lr.ph.preheader.critedge ]
  %.0206208.i1 = phi float [ %.3.i1, %426 ], [ undef, %.lr.ph.preheader.critedge ]
  %.phi.trans.insert = getelementptr inbounds [2 x i32], [2 x i32]* %nodesToProcess, i32 0, i32 %currentBVHIndex.0211
  %.pre = load i32, i32* %.phi.trans.insert, align 4, !tbaa !221
  br label %84

; <label>:84                                      ; preds = %422, %.lr.ph
  %85 = phi i32 [ %.pre, %.lr.ph ], [ %424, %422 ]
  %.1.i0 = phi float [ %.0206208.i0, %.lr.ph ], [ %.3.i0, %422 ]
  %.1.i1 = phi float [ %.0206208.i1, %.lr.ph ], [ %.3.i1, %422 ]
  %currentRayData.0.1.i0 = phi float [ %WorldRayOrigin, %.lr.ph ], [ %currentRayData.0.2.i0, %422 ]
  %currentRayData.0.1.i1 = phi float [ %WorldRayOrigin275, %.lr.ph ], [ %currentRayData.0.2.i1, %422 ]
  %currentRayData.0.1.i2 = phi float [ %WorldRayOrigin276, %.lr.ph ], [ %currentRayData.0.2.i2, %422 ]
  %currentRayData.1.1.i0 = phi float [ %WorldRayDirection, %.lr.ph ], [ %currentRayData.1.2.i0, %422 ]
  %currentRayData.1.1.i1 = phi float [ %WorldRayDirection280, %.lr.ph ], [ %currentRayData.1.2.i1, %422 ]
  %currentRayData.1.1.i2 = phi float [ %WorldRayDirection281, %.lr.ph ], [ %currentRayData.1.2.i2, %422 ]
  %currentRayData.2.1.i0 = phi float [ %.i0, %.lr.ph ], [ %currentRayData.2.2.i0, %422 ]
  %currentRayData.2.1.i1 = phi float [ %.i1, %.lr.ph ], [ %currentRayData.2.2.i1, %422 ]
  %currentRayData.2.1.i2 = phi float [ %.i2, %.lr.ph ], [ %currentRayData.2.2.i2, %422 ]
  %currentRayData.3.1.i0 = phi float [ %.i0325, %.lr.ph ], [ %currentRayData.3.2.i0, %422 ]
  %currentRayData.3.1.i1 = phi float [ %.i1326, %.lr.ph ], [ %currentRayData.3.2.i1, %422 ]
  %currentRayData.3.1.i2 = phi float [ %.i2327, %.lr.ph ], [ %currentRayData.3.2.i2, %422 ]
  %currentRayData.4.1.i0 = phi float [ %29, %.lr.ph ], [ %currentRayData.4.2.i0, %422 ]
  %currentRayData.4.1.i1 = phi float [ %32, %.lr.ph ], [ %currentRayData.4.2.i1, %422 ]
  %currentRayData.4.1.i2 = phi float [ %33, %.lr.ph ], [ %currentRayData.4.2.i2, %422 ]
  %currentRayData.5.1.i0 = phi i32 [ %worldRayData.5.0.i0, %.lr.ph ], [ %currentRayData.5.2.i0, %422 ]
  %currentRayData.5.1.i1 = phi i32 [ %worldRayData.5.0.i1, %.lr.ph ], [ %currentRayData.5.2.i1, %422 ]
  %currentRayData.5.1.i2 = phi i32 [ %.0203, %.lr.ph ], [ %currentRayData.5.2.i2, %422 ]
  %CurrentObjectToWorld199.1.i0 = phi float [ %CurrentObjectToWorld199.0209.i0, %.lr.ph ], [ %CurrentObjectToWorld199.2.i0, %422 ]
  %CurrentObjectToWorld199.1.i1 = phi float [ %CurrentObjectToWorld199.0209.i1, %.lr.ph ], [ %CurrentObjectToWorld199.2.i1, %422 ]
  %CurrentObjectToWorld199.1.i2 = phi float [ %CurrentObjectToWorld199.0209.i2, %.lr.ph ], [ %CurrentObjectToWorld199.2.i2, %422 ]
  %CurrentObjectToWorld199.1.i3 = phi float [ %CurrentObjectToWorld199.0209.i3, %.lr.ph ], [ %CurrentObjectToWorld199.2.i3, %422 ]
  %CurrentObjectToWorld199.1.i4 = phi float [ %CurrentObjectToWorld199.0209.i4, %.lr.ph ], [ %CurrentObjectToWorld199.2.i4, %422 ]
  %CurrentObjectToWorld199.1.i5 = phi float [ %CurrentObjectToWorld199.0209.i5, %.lr.ph ], [ %CurrentObjectToWorld199.2.i5, %422 ]
  %CurrentObjectToWorld199.1.i6 = phi float [ %CurrentObjectToWorld199.0209.i6, %.lr.ph ], [ %CurrentObjectToWorld199.2.i6, %422 ]
  %CurrentObjectToWorld199.1.i7 = phi float [ %CurrentObjectToWorld199.0209.i7, %.lr.ph ], [ %CurrentObjectToWorld199.2.i7, %422 ]
  %CurrentObjectToWorld199.1.i8 = phi float [ %CurrentObjectToWorld199.0209.i8, %.lr.ph ], [ %CurrentObjectToWorld199.2.i8, %422 ]
  %CurrentObjectToWorld199.1.i9 = phi float [ %CurrentObjectToWorld199.0209.i9, %.lr.ph ], [ %CurrentObjectToWorld199.2.i9, %422 ]
  %CurrentObjectToWorld199.1.i10 = phi float [ %CurrentObjectToWorld199.0209.i10, %.lr.ph ], [ %CurrentObjectToWorld199.2.i10, %422 ]
  %CurrentObjectToWorld199.1.i11 = phi float [ %CurrentObjectToWorld199.0209.i11, %.lr.ph ], [ %CurrentObjectToWorld199.2.i11, %422 ]
  %CurrentWorldToObject200.1.i0 = phi float [ %CurrentWorldToObject200.0210.i0, %.lr.ph ], [ %CurrentWorldToObject200.2.i0, %422 ]
  %CurrentWorldToObject200.1.i1 = phi float [ %CurrentWorldToObject200.0210.i1, %.lr.ph ], [ %CurrentWorldToObject200.2.i1, %422 ]
  %CurrentWorldToObject200.1.i2 = phi float [ %CurrentWorldToObject200.0210.i2, %.lr.ph ], [ %CurrentWorldToObject200.2.i2, %422 ]
  %CurrentWorldToObject200.1.i3 = phi float [ %CurrentWorldToObject200.0210.i3, %.lr.ph ], [ %CurrentWorldToObject200.2.i3, %422 ]
  %CurrentWorldToObject200.1.i4 = phi float [ %CurrentWorldToObject200.0210.i4, %.lr.ph ], [ %CurrentWorldToObject200.2.i4, %422 ]
  %CurrentWorldToObject200.1.i5 = phi float [ %CurrentWorldToObject200.0210.i5, %.lr.ph ], [ %CurrentWorldToObject200.2.i5, %422 ]
  %CurrentWorldToObject200.1.i6 = phi float [ %CurrentWorldToObject200.0210.i6, %.lr.ph ], [ %CurrentWorldToObject200.2.i6, %422 ]
  %CurrentWorldToObject200.1.i7 = phi float [ %CurrentWorldToObject200.0210.i7, %.lr.ph ], [ %CurrentWorldToObject200.2.i7, %422 ]
  %CurrentWorldToObject200.1.i8 = phi float [ %CurrentWorldToObject200.0210.i8, %.lr.ph ], [ %CurrentWorldToObject200.2.i8, %422 ]
  %CurrentWorldToObject200.1.i9 = phi float [ %CurrentWorldToObject200.0210.i9, %.lr.ph ], [ %CurrentWorldToObject200.2.i9, %422 ]
  %CurrentWorldToObject200.1.i10 = phi float [ %CurrentWorldToObject200.0210.i10, %.lr.ph ], [ %CurrentWorldToObject200.2.i10, %422 ]
  %CurrentWorldToObject200.1.i11 = phi float [ %CurrentWorldToObject200.0210.i11, %.lr.ph ], [ %CurrentWorldToObject200.2.i11, %422 ]
  %ResetMatrices.1 = phi i32 [ 1, %.lr.ph ], [ %ResetMatrices.3, %422 ]
  %currentBVHIndex.1 = phi i32 [ %currentBVHIndex.0211, %.lr.ph ], [ %currentBVHIndex.2, %422 ]
  %currentGpuVA.1.i0 = phi i32 [ %currentGpuVA.0212.i0, %.lr.ph ], [ %currentGpuVA.2.i0, %422 ]
  %currentGpuVA.1.i1 = phi i32 [ %currentGpuVA.0212.i1, %.lr.ph ], [ %currentGpuVA.2.i1, %422 ]
  %instIdx.1 = phi i32 [ %instIdx.0213, %.lr.ph ], [ %instIdx.2, %422 ]
  %instFlags.1 = phi i32 [ %instFlags.0214, %.lr.ph ], [ %instFlags.2, %422 ]
  %instOffset.1 = phi i32 [ %instOffset.0215, %.lr.ph ], [ %instOffset.2, %422 ]
  %instId.1 = phi i32 [ %instId.0216, %.lr.ph ], [ %instId.2, %422 ]
  %stackPointer.2 = phi i32 [ %stackPointer.1217, %.lr.ph ], [ %stackPointer.3, %422 ]
  %resultBary.1.i0 = phi float [ %resultBary.0219.i0, %.lr.ph ], [ %resultBary.2.i0, %422 ]
  %resultBary.1.i1 = phi float [ %resultBary.0219.i1, %.lr.ph ], [ %resultBary.2.i1, %422 ]
  %resultTriId.1 = phi i32 [ %resultTriId.0220, %.lr.ph ], [ %resultTriId.2, %422 ]
  %currentRayData.1.1.upto0 = insertelement <3 x float> undef, float %currentRayData.1.1.i0, i32 0
  %currentRayData.1.1.upto1 = insertelement <3 x float> %currentRayData.1.1.upto0, float %currentRayData.1.1.i1, i32 1
  %currentRayData.1.1 = insertelement <3 x float> %currentRayData.1.1.upto1, float %currentRayData.1.1.i2, i32 2
  %currentRayData.0.1.upto0 = insertelement <3 x float> undef, float %currentRayData.0.1.i0, i32 0
  %currentRayData.0.1.upto1 = insertelement <3 x float> %currentRayData.0.1.upto0, float %currentRayData.0.1.i1, i32 1
  %currentRayData.0.1 = insertelement <3 x float> %currentRayData.0.1.upto1, float %currentRayData.0.1.i2, i32 2
  %86 = add nsw i32 %stackPointer.2, -1
  %87 = shl i32 %86, 6
  %88 = add i32 %87, %10
  %89 = getelementptr [2048 x i32], [2048 x i32] addrspace(3)* @"\01?stack@@3PAIA", i32 0, i32 %88
  %90 = load i32, i32 addrspace(3)* %89, align 4, !tbaa !221, !noalias !244
  %91 = getelementptr inbounds [2 x i32], [2 x i32]* %nodesToProcess, i32 0, i32 %currentBVHIndex.1
  %92 = add i32 %85, -1
  store i32 %92, i32* %91, align 4, !tbaa !221
  %93 = getelementptr inbounds [0 x %struct.RWByteAddressBuffer], [0 x %struct.RWByteAddressBuffer]* @"\01?DescriptorHeapBufferTable@@3PAURWByteAddressBuffer@@A", i32 0, i32 %currentGpuVA.1.i1, !dx.nonuniform !240
  %94 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %93, align 4, !noalias !247
  %95 = add i32 %currentGpuVA.1.i0, 16
  %96 = shl i32 %90, 5
  %97 = add i32 %95, %96
  %98 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %94)
  %99 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %98, i32 %97, i32 undef)
  %100 = extractvalue %dx.types.ResRet.i32 %99, 3
  %101 = add i32 %97, 16
  %102 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %98, i32 %101, i32 undef)
  %103 = extractvalue %dx.types.ResRet.i32 %102, 3
  %104 = icmp slt i32 %100, 0
  br i1 %104, label %105, label %370

; <label>:105                                     ; preds = %84
  %106 = icmp eq i32 %currentBVHIndex.1, 0
  br i1 %106, label %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit221", label %"\01?IsOpaque@@YA_N_NII@Z.exit"

"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit221": ; preds = %105
  %107 = and i32 %100, 2147483647
  %108 = mul i32 %107, 112
  %109 = add i32 %44, %108
  %110 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %39)
  %111 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %110, i32 %109, i32 undef)
  %112 = extractvalue %dx.types.ResRet.i32 %111, 0
  %113 = extractvalue %dx.types.ResRet.i32 %111, 1
  %114 = extractvalue %dx.types.ResRet.i32 %111, 2
  %115 = extractvalue %dx.types.ResRet.i32 %111, 3
  %116 = add i32 %109, 16
  %117 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %110, i32 %116, i32 undef)
  %118 = extractvalue %dx.types.ResRet.i32 %117, 0
  %119 = extractvalue %dx.types.ResRet.i32 %117, 1
  %120 = extractvalue %dx.types.ResRet.i32 %117, 2
  %121 = extractvalue %dx.types.ResRet.i32 %117, 3
  %122 = add i32 %109, 32
  %123 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %110, i32 %122, i32 undef)
  %124 = extractvalue %dx.types.ResRet.i32 %123, 0
  %125 = extractvalue %dx.types.ResRet.i32 %123, 1
  %126 = extractvalue %dx.types.ResRet.i32 %123, 2
  %127 = extractvalue %dx.types.ResRet.i32 %123, 3
  %128 = add i32 %109, 48
  %129 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %110, i32 %128, i32 undef)
  %130 = extractvalue %dx.types.ResRet.i32 %129, 0
  %131 = extractvalue %dx.types.ResRet.i32 %129, 1
  %132 = extractvalue %dx.types.ResRet.i32 %129, 2
  %133 = extractvalue %dx.types.ResRet.i32 %129, 3
  %134 = add i32 %109, 64
  %135 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %110, i32 %134, i32 undef)
  %136 = extractvalue %dx.types.ResRet.i32 %135, 0
  %137 = extractvalue %dx.types.ResRet.i32 %135, 1
  %138 = extractvalue %dx.types.ResRet.i32 %135, 2
  %139 = extractvalue %dx.types.ResRet.i32 %135, 3
  %140 = add i32 %109, 80
  %141 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %110, i32 %140, i32 undef)
  %142 = extractvalue %dx.types.ResRet.i32 %141, 0
  %143 = extractvalue %dx.types.ResRet.i32 %141, 1
  %144 = extractvalue %dx.types.ResRet.i32 %141, 2
  %145 = extractvalue %dx.types.ResRet.i32 %141, 3
  %146 = add i32 %109, 96
  %147 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %110, i32 %146, i32 undef)
  %148 = extractvalue %dx.types.ResRet.i32 %147, 0
  %149 = extractvalue %dx.types.ResRet.i32 %147, 1
  %150 = extractvalue %dx.types.ResRet.i32 %147, 2
  %151 = extractvalue %dx.types.ResRet.i32 %147, 3
  %152 = bitcast i32 %112 to float
  %153 = bitcast i32 %113 to float
  %154 = bitcast i32 %114 to float
  %155 = bitcast i32 %115 to float
  %156 = bitcast i32 %118 to float
  %157 = bitcast i32 %119 to float
  %158 = bitcast i32 %120 to float
  %159 = bitcast i32 %121 to float
  %160 = bitcast i32 %124 to float
  %161 = bitcast i32 %125 to float
  %162 = bitcast i32 %126 to float
  %163 = bitcast i32 %127 to float
  %164 = and i32 %131, 16777215
  %165 = and i32 %130, 16777215
  %166 = lshr i32 %130, 24
  %167 = and i32 %166, %0
  %168 = icmp eq i32 %167, 0
  %169 = bitcast i32 %136 to float
  %170 = bitcast i32 %137 to float
  %171 = bitcast i32 %138 to float
  %172 = bitcast i32 %139 to float
  %173 = bitcast i32 %142 to float
  %174 = bitcast i32 %143 to float
  %175 = bitcast i32 %144 to float
  %176 = bitcast i32 %145 to float
  %177 = bitcast i32 %148 to float
  %178 = bitcast i32 %149 to float
  %179 = bitcast i32 %150 to float
  %180 = bitcast i32 %151 to float
  br i1 %168, label %422, label %181

; <label>:181                                     ; preds = %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit221"
  store i32 0, i32 addrspace(3)* %89, align 4, !tbaa !221, !noalias !252
  %182 = lshr i32 %131, 24
  %183 = fmul fast float %WorldRayDirection, %152
  %FMad315 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayDirection280, float %153, float %183)
  %FMad314 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayDirection281, float %154, float %FMad315)
  %FMad313 = call float @dx.op.tertiary.f32(i32 46, float 0.000000e+00, float %155, float %FMad314)
  %184 = fmul fast float %WorldRayDirection, %156
  %FMad312 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayDirection280, float %157, float %184)
  %FMad311 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayDirection281, float %158, float %FMad312)
  %FMad310 = call float @dx.op.tertiary.f32(i32 46, float 0.000000e+00, float %159, float %FMad311)
  %185 = fmul fast float %WorldRayDirection, %160
  %FMad309 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayDirection280, float %161, float %185)
  %FMad308 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayDirection281, float %162, float %FMad309)
  %FMad = call float @dx.op.tertiary.f32(i32 46, float 0.000000e+00, float %163, float %FMad308)
  %186 = fmul fast float %WorldRayOrigin, %152
  %FMad324 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayOrigin275, float %153, float %186)
  %FMad323 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayOrigin276, float %154, float %FMad324)
  %FMad322 = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %155, float %FMad323)
  %187 = fmul fast float %WorldRayOrigin, %156
  %FMad321 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayOrigin275, float %157, float %187)
  %FMad320 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayOrigin276, float %158, float %FMad321)
  %FMad319 = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %159, float %FMad320)
  %188 = fmul fast float %WorldRayOrigin, %160
  %FMad318 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayOrigin275, float %161, float %188)
  %FMad317 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayOrigin276, float %162, float %FMad318)
  %FMad316 = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %163, float %FMad317)
  store float %FMad313, float* %69, align 4
  store float %FMad310, float* %70, align 4
  store float %FMad, float* %71, align 4
  %.i0369 = fdiv fast float 1.000000e+00, %FMad313
  %.i1370 = fdiv fast float 1.000000e+00, %FMad310
  %.i2371 = fdiv fast float 1.000000e+00, %FMad
  %.i0372 = fmul fast float %.i0369, %FMad322
  %.i1373 = fmul fast float %.i1370, %FMad319
  %.i2374 = fmul fast float %.i2371, %FMad316
  %FAbs263 = call float @dx.op.unary.f32(i32 6, float %FMad313)
  %FAbs264 = call float @dx.op.unary.f32(i32 6, float %FMad310)
  %FAbs265 = call float @dx.op.unary.f32(i32 6, float %FMad)
  %189 = fcmp fast ogt float %FAbs263, %FAbs264
  %190 = fcmp fast ogt float %FAbs263, %FAbs265
  %191 = and i1 %189, %190
  br i1 %191, label %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12", label %192

; <label>:192                                     ; preds = %181
  %193 = fcmp fast ogt float %FAbs264, %FAbs265
  br i1 %193, label %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12", label %194

; <label>:194                                     ; preds = %192
  br label %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12"

"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12": ; preds = %194, %192, %181
  %.0204 = phi i32 [ 2, %194 ], [ 0, %181 ], [ 1, %192 ]
  %195 = add nuw nsw i32 %.0204, 1
  %196 = urem i32 %195, 3
  %197 = add nuw nsw i32 %.0204, 2
  %198 = urem i32 %197, 3
  %199 = getelementptr [3 x float], [3 x float]* %8, i32 0, i32 %.0204
  %200 = load float, float* %199, align 4, !tbaa !231, !noalias !255
  %201 = fcmp fast olt float %200, 0.000000e+00
  %.5.0.i0 = select i1 %201, i32 %198, i32 %196
  %.5.0.i1 = select i1 %201, i32 %196, i32 %198
  %202 = getelementptr [3 x float], [3 x float]* %8, i32 0, i32 %.5.0.i0
  %203 = load float, float* %202, align 4, !tbaa !231, !noalias !255
  %204 = fdiv float %203, %200
  %205 = getelementptr [3 x float], [3 x float]* %8, i32 0, i32 %.5.0.i1
  %206 = load float, float* %205, align 4, !tbaa !231, !noalias !255
  %207 = fdiv float %206, %200
  %208 = fdiv float 1.000000e+00, %200
  store i32 1, i32* %72, align 4, !tbaa !221
  br label %422

"\01?IsOpaque@@YA_N_NII@Z.exit":                  ; preds = %105
  %RayFlags287 = call i32 @dx.op.rayFlags.i32(i32 144)
  %209 = and i32 %RayFlags287, 64
  %RayTCurrent249 = call float @dx.op.rayTCurrent.f32(i32 154)
  %210 = icmp eq i32 %209, 0
  %211 = and i32 %100, 16777215
  %212 = add i32 %currentGpuVA.1.i0, 4
  %213 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %94)
  %214 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %213, i32 %212, i32 undef)
  %215 = extractvalue %dx.types.ResRet.i32 %214, 0
  %216 = mul nuw nsw i32 %211, 36
  %217 = add i32 %216, %currentGpuVA.1.i0
  %218 = add i32 %217, %215
  %219 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %213, i32 %218, i32 undef)
  %220 = extractvalue %dx.types.ResRet.i32 %219, 0
  %221 = extractvalue %dx.types.ResRet.i32 %219, 1
  %222 = extractvalue %dx.types.ResRet.i32 %219, 2
  %223 = extractvalue %dx.types.ResRet.i32 %219, 3
  %.i0375 = bitcast i32 %220 to float
  %.i1376 = bitcast i32 %221 to float
  %.i2377 = bitcast i32 %222 to float
  %.i3378 = bitcast i32 %223 to float
  %224 = add i32 %218, 16
  %225 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %213, i32 %224, i32 undef)
  %226 = extractvalue %dx.types.ResRet.i32 %225, 0
  %227 = extractvalue %dx.types.ResRet.i32 %225, 1
  %228 = extractvalue %dx.types.ResRet.i32 %225, 2
  %229 = extractvalue %dx.types.ResRet.i32 %225, 3
  %.i0379 = bitcast i32 %226 to float
  %.i1380 = bitcast i32 %227 to float
  %.i2381 = bitcast i32 %228 to float
  %.i3382 = bitcast i32 %229 to float
  %230 = add i32 %218, 32
  %231 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %213, i32 %230, i32 undef)
  %232 = extractvalue %dx.types.ResRet.i32 %231, 0
  %233 = bitcast i32 %232 to float
  %234 = and i32 %instFlags.1, 1
  %235 = icmp eq i32 %234, 0
  %236 = shl i32 %instFlags.1, 3
  %237 = and i32 %236, 16
  %238 = add nuw nsw i32 %237, 16
  %239 = xor i32 %237, 16
  %240 = add nuw nsw i32 %239, 16
  %241 = and i32 %RayFlags287, %238
  %242 = icmp ne i32 %241, 0
  %243 = and i1 %235, %242
  %244 = and i32 %RayFlags287, %240
  %245 = icmp ne i32 %244, 0
  %246 = and i1 %235, %245
  %.i0383 = fsub fast float %.i0375, %currentRayData.0.1.i0
  %.i1384 = fsub fast float %.i1376, %currentRayData.0.1.i1
  %.i2385 = fsub fast float %.i2377, %currentRayData.0.1.i2
  store float %.i0383, float* %73, align 4
  store float %.i1384, float* %74, align 4
  store float %.i2385, float* %75, align 4
  %247 = getelementptr [3 x float], [3 x float]* %7, i32 0, i32 %currentRayData.5.1.i0
  %248 = load float, float* %247, align 4, !tbaa !231, !noalias !258
  %249 = getelementptr [3 x float], [3 x float]* %7, i32 0, i32 %currentRayData.5.1.i1
  %250 = load float, float* %249, align 4, !tbaa !231, !noalias !258
  %251 = getelementptr [3 x float], [3 x float]* %7, i32 0, i32 %currentRayData.5.1.i2
  %252 = load float, float* %251, align 4, !tbaa !231, !noalias !258
  %.i0386 = fsub fast float %.i3378, %currentRayData.0.1.i0
  %.i1387 = fsub fast float %.i0379, %currentRayData.0.1.i1
  %.i2388 = fsub fast float %.i1380, %currentRayData.0.1.i2
  store float %.i0386, float* %76, align 4
  store float %.i1387, float* %77, align 4
  store float %.i2388, float* %78, align 4
  %253 = getelementptr [3 x float], [3 x float]* %5, i32 0, i32 %currentRayData.5.1.i0
  %254 = load float, float* %253, align 4, !tbaa !231, !noalias !258
  %255 = getelementptr [3 x float], [3 x float]* %5, i32 0, i32 %currentRayData.5.1.i1
  %256 = load float, float* %255, align 4, !tbaa !231, !noalias !258
  %257 = getelementptr [3 x float], [3 x float]* %5, i32 0, i32 %currentRayData.5.1.i2
  %258 = load float, float* %257, align 4, !tbaa !231, !noalias !258
  %.i0389 = fsub fast float %.i2381, %currentRayData.0.1.i0
  %.i1390 = fsub fast float %.i3382, %currentRayData.0.1.i1
  %.i2391 = fsub fast float %233, %currentRayData.0.1.i2
  store float %.i0389, float* %79, align 4
  store float %.i1390, float* %80, align 4
  store float %.i2391, float* %81, align 4
  %259 = getelementptr [3 x float], [3 x float]* %6, i32 0, i32 %currentRayData.5.1.i0
  %260 = load float, float* %259, align 4, !tbaa !231, !noalias !258
  %261 = getelementptr [3 x float], [3 x float]* %6, i32 0, i32 %currentRayData.5.1.i1
  %262 = load float, float* %261, align 4, !tbaa !231, !noalias !258
  %263 = getelementptr [3 x float], [3 x float]* %6, i32 0, i32 %currentRayData.5.1.i2
  %264 = load float, float* %263, align 4, !tbaa !231, !noalias !258
  %.i0392 = fmul float %currentRayData.4.1.i0, %252
  %.i1393 = fmul float %currentRayData.4.1.i1, %252
  %.i0395 = fsub float %248, %.i0392
  %.i1396 = fsub float %250, %.i1393
  %.i0397 = fmul float %currentRayData.4.1.i0, %258
  %.i1398 = fmul float %currentRayData.4.1.i1, %258
  %.i0400 = fsub float %254, %.i0397
  %.i1401 = fsub float %256, %.i1398
  %.i0402 = fmul float %currentRayData.4.1.i0, %264
  %.i1403 = fmul float %currentRayData.4.1.i1, %264
  %.i0405 = fsub float %260, %.i0402
  %.i1406 = fsub float %262, %.i1403
  %265 = fmul float %.i1401, %.i0405
  %266 = fmul float %.i0400, %.i1406
  %267 = fsub float %265, %266
  %268 = fmul float %.i0395, %.i1406
  %269 = fmul float %.i1396, %.i0405
  %270 = fsub float %268, %269
  %271 = fmul float %.i1396, %.i0400
  %272 = fmul float %.i0395, %.i1401
  %273 = fsub float %271, %272
  %274 = fadd fast float %270, %273
  %275 = fadd fast float %274, %267
  br i1 %246, label %276, label %282

; <label>:276                                     ; preds = %"\01?IsOpaque@@YA_N_NII@Z.exit"
  %277 = fcmp fast ogt float %267, 0.000000e+00
  %278 = fcmp fast ogt float %270, 0.000000e+00
  %279 = or i1 %277, %278
  %280 = fcmp fast ogt float %273, 0.000000e+00
  %281 = or i1 %280, %279
  %.old.old = fcmp fast oeq float %275, 0.000000e+00
  %or.cond227 = or i1 %281, %.old.old
  br i1 %or.cond227, label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i", label %302

; <label>:282                                     ; preds = %"\01?IsOpaque@@YA_N_NII@Z.exit"
  br i1 %243, label %283, label %289

; <label>:283                                     ; preds = %282
  %284 = fcmp fast olt float %267, 0.000000e+00
  %285 = fcmp fast olt float %270, 0.000000e+00
  %286 = or i1 %284, %285
  %287 = fcmp fast olt float %273, 0.000000e+00
  %288 = or i1 %287, %286
  %.old = fcmp fast oeq float %275, 0.000000e+00
  %or.cond226 = or i1 %288, %.old
  br i1 %or.cond226, label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i", label %302

; <label>:289                                     ; preds = %282
  %290 = fcmp fast olt float %267, 0.000000e+00
  %291 = fcmp fast olt float %270, 0.000000e+00
  %292 = or i1 %290, %291
  %293 = fcmp fast olt float %273, 0.000000e+00
  %294 = or i1 %293, %292
  %295 = fcmp fast ogt float %267, 0.000000e+00
  %296 = fcmp fast ogt float %270, 0.000000e+00
  %297 = or i1 %295, %296
  %298 = fcmp fast ogt float %273, 0.000000e+00
  %299 = or i1 %298, %297
  %300 = and i1 %294, %299
  %301 = fcmp fast oeq float %275, 0.000000e+00
  %or.cond225 = or i1 %301, %300
  br i1 %or.cond225, label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i", label %302

; <label>:302                                     ; preds = %289, %283, %276
  %303 = fmul fast float %267, %252
  %304 = fmul fast float %270, %258
  %305 = fmul fast float %273, %264
  %tmp = fadd fast float %304, %305
  %tmp546 = fadd fast float %tmp, %303
  %tmp547 = fmul fast float %tmp546, %currentRayData.4.1.i2
  br i1 %246, label %306, label %311

; <label>:306                                     ; preds = %302
  %307 = fcmp fast ogt float %tmp547, 0.000000e+00
  %308 = fmul fast float %275, %RayTCurrent249
  %309 = fcmp fast olt float %tmp547, %308
  %310 = or i1 %307, %309
  br i1 %310, label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i", label %328

; <label>:311                                     ; preds = %302
  br i1 %243, label %312, label %317

; <label>:312                                     ; preds = %311
  %313 = fcmp fast olt float %tmp547, 0.000000e+00
  %314 = fmul fast float %275, %RayTCurrent249
  %315 = fcmp fast ogt float %tmp547, %314
  %316 = or i1 %313, %315
  br i1 %316, label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i", label %328

; <label>:317                                     ; preds = %311
  %318 = fcmp fast ogt float %275, 0.000000e+00
  %319 = select i1 %318, i32 1, i32 -1
  %320 = bitcast float %tmp547 to i32
  %321 = xor i32 %320, %319
  %322 = uitofp i32 %321 to float
  %323 = bitcast float %275 to i32
  %324 = xor i32 %319, %323
  %325 = uitofp i32 %324 to float
  %326 = fmul fast float %325, %RayTCurrent249
  %327 = fcmp fast ogt float %322, %326
  br i1 %327, label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i", label %328

; <label>:328                                     ; preds = %317, %312, %306
  %329 = fdiv fast float 1.000000e+00, %275
  %330 = fmul fast float %329, %270
  %331 = fmul fast float %329, %273
  %332 = fmul fast float %329, %tmp547
  br label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i"

"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i": ; preds = %328, %317, %312, %306, %289, %283, %276
  %.2.i0 = phi float [ %.1.i0, %276 ], [ %.1.i0, %306 ], [ %330, %328 ], [ %.1.i0, %312 ], [ %.1.i0, %317 ], [ %.1.i0, %283 ], [ %.1.i0, %289 ]
  %.2.i1 = phi float [ %.1.i1, %276 ], [ %.1.i1, %306 ], [ %331, %328 ], [ %.1.i1, %312 ], [ %.1.i1, %317 ], [ %.1.i1, %283 ], [ %.1.i1, %289 ]
  %.0205 = phi float [ %RayTCurrent249, %276 ], [ %RayTCurrent249, %306 ], [ %332, %328 ], [ %RayTCurrent249, %312 ], [ %RayTCurrent249, %317 ], [ %RayTCurrent249, %283 ], [ %RayTCurrent249, %289 ]
  %333 = fcmp fast olt float %.0205, %RayTCurrent249
  %RayTMin = call float @dx.op.rayTMin.f32(i32 153)
  %334 = fcmp fast ogt float %.0205, %RayTMin
  %335 = and i1 %333, %334
  %. = select i1 %335, i1 true, i1 false
  %.resultTriId.1 = select i1 %335, i32 %211, i32 %resultTriId.1
  %.0205.RayTCurrent249 = select i1 %335, float %.0205, float %RayTCurrent249
  %.2.i0.resultBary.1.i0 = select i1 %335, float %.2.i0, float %resultBary.1.i0
  %.2.i1.resultBary.1.i1 = select i1 %335, float %.2.i1, float %resultBary.1.i1
  %336 = and i1 %210, %.
  br i1 %336, label %337, label %422

; <label>:337                                     ; preds = %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i"
  %.0.upto0 = insertelement <2 x float> undef, float %.2.i0.resultBary.1.i0, i32 0
  %.0 = insertelement <2 x float> %.0.upto0, float %.2.i1.resultBary.1.i1, i32 1
  %338 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %93, align 4, !noalias !266
  %339 = add i32 %currentGpuVA.1.i0, 8
  %340 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %338)
  %341 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %340, i32 %339, i32 undef)
  %342 = extractvalue %dx.types.ResRet.i32 %341, 0
  %343 = shl i32 %.resultTriId.1, 3
  %344 = add i32 %343, %currentGpuVA.1.i0
  %345 = add i32 %344, %342
  %346 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %340, i32 %345, i32 undef)
  %347 = extractvalue %dx.types.ResRet.i32 %346, 0
  %348 = extractvalue %dx.types.ResRet.i32 %346, 1
  %349 = mul i32 %347, %2
  %350 = add i32 %instOffset.1, %1
  %351 = add i32 %350, %349
  store <2 x float> %.0, <2 x float>* %82, align 4, !tbaa !218
  call void @"\01?Fallback_SetPendingAttr@@YAXUBuiltInTriangleIntersectionAttributes@@@Z"(%struct.BuiltInTriangleIntersectionAttributes* nonnull %attr) #3
  call void @"\01?Fallback_SetPendingTriVals@@YAXMIIIII@Z"(float %.0205.RayTCurrent249, i32 %348, i32 %351, i32 %instIdx.1, i32 %instId.1, i32 254) #3
  %352 = icmp eq i32 %ResetMatrices.1, 0
  br i1 %352, label %366, label %353

; <label>:353                                     ; preds = %337
  call void @"\01?Fallback_SetObjectRayOrigin@@YAXV?$vector@M$02@@@Z"(<3 x float> %currentRayData.0.1) #3
  call void @"\01?Fallback_SetObjectRayDirection@@YAXV?$vector@M$02@@@Z"(<3 x float> %currentRayData.1.1) #3
  %.upto0 = insertelement <4 x float> undef, float %CurrentWorldToObject200.1.i0, i32 0
  %.upto1 = insertelement <4 x float> %.upto0, float %CurrentWorldToObject200.1.i1, i32 1
  %.upto2 = insertelement <4 x float> %.upto1, float %CurrentWorldToObject200.1.i2, i32 2
  %354 = insertelement <4 x float> %.upto2, float %CurrentWorldToObject200.1.i3, i32 3
  %355 = insertvalue %class.matrix.float.3.4 undef, <4 x float> %354, 0, 0
  %.upto01 = insertelement <4 x float> undef, float %CurrentWorldToObject200.1.i4, i32 0
  %.upto12 = insertelement <4 x float> %.upto01, float %CurrentWorldToObject200.1.i5, i32 1
  %.upto23 = insertelement <4 x float> %.upto12, float %CurrentWorldToObject200.1.i6, i32 2
  %356 = insertelement <4 x float> %.upto23, float %CurrentWorldToObject200.1.i7, i32 3
  %357 = insertvalue %class.matrix.float.3.4 %355, <4 x float> %356, 0, 1
  %.upto04 = insertelement <4 x float> undef, float %CurrentWorldToObject200.1.i8, i32 0
  %.upto15 = insertelement <4 x float> %.upto04, float %CurrentWorldToObject200.1.i9, i32 1
  %.upto26 = insertelement <4 x float> %.upto15, float %CurrentWorldToObject200.1.i10, i32 2
  %358 = insertelement <4 x float> %.upto26, float %CurrentWorldToObject200.1.i11, i32 3
  %359 = insertvalue %class.matrix.float.3.4 %357, <4 x float> %358, 0, 2
  call void @"\01?Fallback_SetWorldToObject@@YAXV?$matrix@M$02$03@@@Z"(%class.matrix.float.3.4 %359) #3
  %.upto07 = insertelement <4 x float> undef, float %CurrentObjectToWorld199.1.i0, i32 0
  %.upto18 = insertelement <4 x float> %.upto07, float %CurrentObjectToWorld199.1.i1, i32 1
  %.upto29 = insertelement <4 x float> %.upto18, float %CurrentObjectToWorld199.1.i2, i32 2
  %360 = insertelement <4 x float> %.upto29, float %CurrentObjectToWorld199.1.i3, i32 3
  %361 = insertvalue %class.matrix.float.3.4 undef, <4 x float> %360, 0, 0
  %.upto010 = insertelement <4 x float> undef, float %CurrentObjectToWorld199.1.i4, i32 0
  %.upto111 = insertelement <4 x float> %.upto010, float %CurrentObjectToWorld199.1.i5, i32 1
  %.upto212 = insertelement <4 x float> %.upto111, float %CurrentObjectToWorld199.1.i6, i32 2
  %362 = insertelement <4 x float> %.upto212, float %CurrentObjectToWorld199.1.i7, i32 3
  %363 = insertvalue %class.matrix.float.3.4 %361, <4 x float> %362, 0, 1
  %.upto013 = insertelement <4 x float> undef, float %CurrentObjectToWorld199.1.i8, i32 0
  %.upto114 = insertelement <4 x float> %.upto013, float %CurrentObjectToWorld199.1.i9, i32 1
  %.upto215 = insertelement <4 x float> %.upto114, float %CurrentObjectToWorld199.1.i10, i32 2
  %364 = insertelement <4 x float> %.upto215, float %CurrentObjectToWorld199.1.i11, i32 3
  %365 = insertvalue %class.matrix.float.3.4 %363, <4 x float> %364, 0, 2
  call void @"\01?Fallback_SetObjectToWorld@@YAXV?$matrix@M$02$03@@@Z"(%class.matrix.float.3.4 %365) #3
  br label %366

; <label>:366                                     ; preds = %353, %337
  call void @"\01?Fallback_CommitHit@@YAXXZ"() #3
  %367 = and i32 %RayFlags287, 4
  %368 = icmp eq i32 %367, 0
  br i1 %368, label %422, label %369

; <label>:369                                     ; preds = %366
  store i32 0, i32* %72, align 4, !tbaa !221
  store i32 0, i32* %37, align 4, !tbaa !221
  br label %422

; <label>:370                                     ; preds = %84
  %371 = and i32 %100, 16777215
  %RayTCurrent250 = call float @dx.op.rayTCurrent.f32(i32 154)
  %372 = shl nuw nsw i32 %371, 5
  %373 = add i32 %95, %372
  %374 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %94)
  %375 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %374, i32 %373, i32 undef)
  %376 = extractvalue %dx.types.ResRet.i32 %375, 0
  %377 = extractvalue %dx.types.ResRet.i32 %375, 1
  %378 = extractvalue %dx.types.ResRet.i32 %375, 2
  %379 = add i32 %373, 16
  %380 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %374, i32 %379, i32 undef)
  %381 = extractvalue %dx.types.ResRet.i32 %380, 0
  %382 = extractvalue %dx.types.ResRet.i32 %380, 1
  %383 = extractvalue %dx.types.ResRet.i32 %380, 2
  %384 = bitcast i32 %376 to float
  %385 = bitcast i32 %377 to float
  %386 = bitcast i32 %378 to float
  %387 = bitcast i32 %381 to float
  %388 = bitcast i32 %382 to float
  %389 = bitcast i32 %383 to float
  %390 = shl i32 %103, 5
  %391 = add i32 %95, %390
  %392 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %374, i32 %391, i32 undef)
  %393 = extractvalue %dx.types.ResRet.i32 %392, 0
  %394 = extractvalue %dx.types.ResRet.i32 %392, 1
  %395 = extractvalue %dx.types.ResRet.i32 %392, 2
  %396 = add i32 %391, 16
  %397 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %374, i32 %396, i32 undef)
  %398 = extractvalue %dx.types.ResRet.i32 %397, 0
  %399 = extractvalue %dx.types.ResRet.i32 %397, 1
  %400 = extractvalue %dx.types.ResRet.i32 %397, 2
  %401 = bitcast i32 %393 to float
  %402 = bitcast i32 %394 to float
  %403 = bitcast i32 %395 to float
  %404 = bitcast i32 %398 to float
  %405 = bitcast i32 %399 to float
  %406 = bitcast i32 %400 to float
  %.i0407 = fmul fast float %384, %currentRayData.2.1.i0
  %.i1408 = fmul fast float %385, %currentRayData.2.1.i1
  %.i2409 = fmul fast float %386, %currentRayData.2.1.i2
  %.i0410 = fsub fast float %.i0407, %currentRayData.3.1.i0
  %.i1411 = fsub fast float %.i1408, %currentRayData.3.1.i1
  %.i2412 = fsub fast float %.i2409, %currentRayData.3.1.i2
  %FAbs260 = call float @dx.op.unary.f32(i32 6, float %currentRayData.2.1.i0)
  %FAbs261 = call float @dx.op.unary.f32(i32 6, float %currentRayData.2.1.i1)
  %FAbs262 = call float @dx.op.unary.f32(i32 6, float %currentRayData.2.1.i2)
  %.i0413 = fmul fast float %FAbs260, %387
  %.i1414 = fmul fast float %FAbs261, %388
  %.i2415 = fmul fast float %FAbs262, %389
  %.i0416 = fadd fast float %.i0413, %.i0410
  %.i1417 = fadd fast float %.i1414, %.i1411
  %.i2418 = fadd fast float %.i2415, %.i2412
  %.i0422 = fsub fast float %.i0410, %.i0413
  %.i1423 = fsub fast float %.i1411, %.i1414
  %.i2424 = fsub fast float %.i2412, %.i2415
  %FMax300 = call float @dx.op.binary.f32(i32 35, float %.i0422, float %.i1423)
  %FMax299 = call float @dx.op.binary.f32(i32 35, float %FMax300, float %.i2424)
  %FMin298 = call float @dx.op.binary.f32(i32 36, float %.i0416, float %.i1417)
  %FMin297 = call float @dx.op.binary.f32(i32 36, float %FMin298, float %.i2418)
  %FMax295 = call float @dx.op.binary.f32(i32 35, float %FMax299, float 0.000000e+00)
  %FMin294 = call float @dx.op.binary.f32(i32 36, float %FMin297, float %RayTCurrent250)
  %407 = fcmp fast olt float %FMax295, %FMin294
  %.i0425 = fmul fast float %401, %currentRayData.2.1.i0
  %.i1426 = fmul fast float %402, %currentRayData.2.1.i1
  %.i2427 = fmul fast float %403, %currentRayData.2.1.i2
  %.i0428 = fsub fast float %.i0425, %currentRayData.3.1.i0
  %.i1429 = fsub fast float %.i1426, %currentRayData.3.1.i1
  %.i2430 = fsub fast float %.i2427, %currentRayData.3.1.i2
  %.i0431 = fmul fast float %FAbs260, %404
  %.i1432 = fmul fast float %FAbs261, %405
  %.i2433 = fmul fast float %FAbs262, %406
  %.i0434 = fadd fast float %.i0431, %.i0428
  %.i1435 = fadd fast float %.i1432, %.i1429
  %.i2436 = fadd fast float %.i2433, %.i2430
  %.i0440 = fsub fast float %.i0428, %.i0431
  %.i1441 = fsub fast float %.i1429, %.i1432
  %.i2442 = fsub fast float %.i2430, %.i2433
  %FMax293 = call float @dx.op.binary.f32(i32 35, float %.i0440, float %.i1441)
  %FMax292 = call float @dx.op.binary.f32(i32 35, float %FMax293, float %.i2442)
  %FMin291 = call float @dx.op.binary.f32(i32 36, float %.i0434, float %.i1435)
  %FMin290 = call float @dx.op.binary.f32(i32 36, float %FMin291, float %.i2436)
  %FMax = call float @dx.op.binary.f32(i32 35, float %FMax292, float 0.000000e+00)
  %FMin = call float @dx.op.binary.f32(i32 36, float %FMin290, float %RayTCurrent250)
  %408 = fcmp fast olt float %FMax, %FMin
  %409 = and i1 %407, %408
  br i1 %409, label %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit", label %418

"\01?StackPush2@@YAXAIAH_NIIII@Z.exit":           ; preds = %370
  %410 = fcmp fast olt float %FMax, %FMax295
  %411 = select i1 %410, i32 %371, i32 %103
  %412 = select i1 %410, i32 %103, i32 %371
  %413 = shl i32 %stackPointer.2, 6
  %414 = add i32 %413, %10
  store i32 %411, i32 addrspace(3)* %89, align 4, !tbaa !221, !noalias !271
  %415 = getelementptr [2048 x i32], [2048 x i32] addrspace(3)* @"\01?stack@@3PAIA", i32 0, i32 %414
  store i32 %412, i32 addrspace(3)* %415, align 4, !tbaa !221, !noalias !271
  %416 = add nsw i32 %stackPointer.2, 1
  %417 = add i32 %85, 1
  store i32 %417, i32* %91, align 4, !tbaa !221
  br label %422

; <label>:418                                     ; preds = %370
  %419 = or i1 %407, %408
  br i1 %419, label %420, label %422

; <label>:420                                     ; preds = %418
  %421 = select i1 %408, i32 %103, i32 %371
  store i32 %421, i32 addrspace(3)* %89, align 4, !tbaa !221, !noalias !274
  store i32 %85, i32* %91, align 4, !tbaa !221
  br label %422

; <label>:422                                     ; preds = %420, %418, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit", %369, %366, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i", %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12", %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit221"
  %.3.i0 = phi float [ %.1.i0, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12" ], [ %.1.i0, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit221" ], [ %.2.i0, %369 ], [ %.2.i0, %366 ], [ %.2.i0, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i" ], [ %.1.i0, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit" ], [ %.1.i0, %420 ], [ %.1.i0, %418 ]
  %.3.i1 = phi float [ %.1.i1, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12" ], [ %.1.i1, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit221" ], [ %.2.i1, %369 ], [ %.2.i1, %366 ], [ %.2.i1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i" ], [ %.1.i1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit" ], [ %.1.i1, %420 ], [ %.1.i1, %418 ]
  %currentRayData.0.2.i0 = phi float [ %FMad322, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12" ], [ %currentRayData.0.1.i0, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit221" ], [ %currentRayData.0.1.i0, %369 ], [ %currentRayData.0.1.i0, %366 ], [ %currentRayData.0.1.i0, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i" ], [ %currentRayData.0.1.i0, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit" ], [ %currentRayData.0.1.i0, %420 ], [ %currentRayData.0.1.i0, %418 ]
  %currentRayData.0.2.i1 = phi float [ %FMad319, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12" ], [ %currentRayData.0.1.i1, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit221" ], [ %currentRayData.0.1.i1, %369 ], [ %currentRayData.0.1.i1, %366 ], [ %currentRayData.0.1.i1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i" ], [ %currentRayData.0.1.i1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit" ], [ %currentRayData.0.1.i1, %420 ], [ %currentRayData.0.1.i1, %418 ]
  %currentRayData.0.2.i2 = phi float [ %FMad316, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12" ], [ %currentRayData.0.1.i2, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit221" ], [ %currentRayData.0.1.i2, %369 ], [ %currentRayData.0.1.i2, %366 ], [ %currentRayData.0.1.i2, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i" ], [ %currentRayData.0.1.i2, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit" ], [ %currentRayData.0.1.i2, %420 ], [ %currentRayData.0.1.i2, %418 ]
  %currentRayData.1.2.i0 = phi float [ %FMad313, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12" ], [ %currentRayData.1.1.i0, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit221" ], [ %currentRayData.1.1.i0, %369 ], [ %currentRayData.1.1.i0, %366 ], [ %currentRayData.1.1.i0, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i" ], [ %currentRayData.1.1.i0, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit" ], [ %currentRayData.1.1.i0, %420 ], [ %currentRayData.1.1.i0, %418 ]
  %currentRayData.1.2.i1 = phi float [ %FMad310, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12" ], [ %currentRayData.1.1.i1, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit221" ], [ %currentRayData.1.1.i1, %369 ], [ %currentRayData.1.1.i1, %366 ], [ %currentRayData.1.1.i1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i" ], [ %currentRayData.1.1.i1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit" ], [ %currentRayData.1.1.i1, %420 ], [ %currentRayData.1.1.i1, %418 ]
  %currentRayData.1.2.i2 = phi float [ %FMad, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12" ], [ %currentRayData.1.1.i2, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit221" ], [ %currentRayData.1.1.i2, %369 ], [ %currentRayData.1.1.i2, %366 ], [ %currentRayData.1.1.i2, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i" ], [ %currentRayData.1.1.i2, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit" ], [ %currentRayData.1.1.i2, %420 ], [ %currentRayData.1.1.i2, %418 ]
  %currentRayData.2.2.i0 = phi float [ %.i0369, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12" ], [ %currentRayData.2.1.i0, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit221" ], [ %currentRayData.2.1.i0, %369 ], [ %currentRayData.2.1.i0, %366 ], [ %currentRayData.2.1.i0, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i" ], [ %currentRayData.2.1.i0, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit" ], [ %currentRayData.2.1.i0, %420 ], [ %currentRayData.2.1.i0, %418 ]
  %currentRayData.2.2.i1 = phi float [ %.i1370, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12" ], [ %currentRayData.2.1.i1, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit221" ], [ %currentRayData.2.1.i1, %369 ], [ %currentRayData.2.1.i1, %366 ], [ %currentRayData.2.1.i1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i" ], [ %currentRayData.2.1.i1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit" ], [ %currentRayData.2.1.i1, %420 ], [ %currentRayData.2.1.i1, %418 ]
  %currentRayData.2.2.i2 = phi float [ %.i2371, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12" ], [ %currentRayData.2.1.i2, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit221" ], [ %currentRayData.2.1.i2, %369 ], [ %currentRayData.2.1.i2, %366 ], [ %currentRayData.2.1.i2, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i" ], [ %currentRayData.2.1.i2, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit" ], [ %currentRayData.2.1.i2, %420 ], [ %currentRayData.2.1.i2, %418 ]
  %currentRayData.3.2.i0 = phi float [ %.i0372, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12" ], [ %currentRayData.3.1.i0, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit221" ], [ %currentRayData.3.1.i0, %369 ], [ %currentRayData.3.1.i0, %366 ], [ %currentRayData.3.1.i0, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i" ], [ %currentRayData.3.1.i0, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit" ], [ %currentRayData.3.1.i0, %420 ], [ %currentRayData.3.1.i0, %418 ]
  %currentRayData.3.2.i1 = phi float [ %.i1373, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12" ], [ %currentRayData.3.1.i1, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit221" ], [ %currentRayData.3.1.i1, %369 ], [ %currentRayData.3.1.i1, %366 ], [ %currentRayData.3.1.i1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i" ], [ %currentRayData.3.1.i1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit" ], [ %currentRayData.3.1.i1, %420 ], [ %currentRayData.3.1.i1, %418 ]
  %currentRayData.3.2.i2 = phi float [ %.i2374, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12" ], [ %currentRayData.3.1.i2, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit221" ], [ %currentRayData.3.1.i2, %369 ], [ %currentRayData.3.1.i2, %366 ], [ %currentRayData.3.1.i2, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i" ], [ %currentRayData.3.1.i2, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit" ], [ %currentRayData.3.1.i2, %420 ], [ %currentRayData.3.1.i2, %418 ]
  %currentRayData.4.2.i0 = phi float [ %204, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12" ], [ %currentRayData.4.1.i0, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit221" ], [ %currentRayData.4.1.i0, %369 ], [ %currentRayData.4.1.i0, %366 ], [ %currentRayData.4.1.i0, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i" ], [ %currentRayData.4.1.i0, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit" ], [ %currentRayData.4.1.i0, %420 ], [ %currentRayData.4.1.i0, %418 ]
  %currentRayData.4.2.i1 = phi float [ %207, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12" ], [ %currentRayData.4.1.i1, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit221" ], [ %currentRayData.4.1.i1, %369 ], [ %currentRayData.4.1.i1, %366 ], [ %currentRayData.4.1.i1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i" ], [ %currentRayData.4.1.i1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit" ], [ %currentRayData.4.1.i1, %420 ], [ %currentRayData.4.1.i1, %418 ]
  %currentRayData.4.2.i2 = phi float [ %208, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12" ], [ %currentRayData.4.1.i2, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit221" ], [ %currentRayData.4.1.i2, %369 ], [ %currentRayData.4.1.i2, %366 ], [ %currentRayData.4.1.i2, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i" ], [ %currentRayData.4.1.i2, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit" ], [ %currentRayData.4.1.i2, %420 ], [ %currentRayData.4.1.i2, %418 ]
  %currentRayData.5.2.i0 = phi i32 [ %.5.0.i0, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12" ], [ %currentRayData.5.1.i0, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit221" ], [ %currentRayData.5.1.i0, %369 ], [ %currentRayData.5.1.i0, %366 ], [ %currentRayData.5.1.i0, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i" ], [ %currentRayData.5.1.i0, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit" ], [ %currentRayData.5.1.i0, %420 ], [ %currentRayData.5.1.i0, %418 ]
  %currentRayData.5.2.i1 = phi i32 [ %.5.0.i1, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12" ], [ %currentRayData.5.1.i1, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit221" ], [ %currentRayData.5.1.i1, %369 ], [ %currentRayData.5.1.i1, %366 ], [ %currentRayData.5.1.i1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i" ], [ %currentRayData.5.1.i1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit" ], [ %currentRayData.5.1.i1, %420 ], [ %currentRayData.5.1.i1, %418 ]
  %currentRayData.5.2.i2 = phi i32 [ %.0204, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12" ], [ %currentRayData.5.1.i2, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit221" ], [ %currentRayData.5.1.i2, %369 ], [ %currentRayData.5.1.i2, %366 ], [ %currentRayData.5.1.i2, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i" ], [ %currentRayData.5.1.i2, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit" ], [ %currentRayData.5.1.i2, %420 ], [ %currentRayData.5.1.i2, %418 ]
  %CurrentObjectToWorld199.2.i0 = phi float [ %169, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12" ], [ %CurrentObjectToWorld199.1.i0, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit221" ], [ %CurrentObjectToWorld199.1.i0, %369 ], [ %CurrentObjectToWorld199.1.i0, %366 ], [ %CurrentObjectToWorld199.1.i0, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i" ], [ %CurrentObjectToWorld199.1.i0, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit" ], [ %CurrentObjectToWorld199.1.i0, %420 ], [ %CurrentObjectToWorld199.1.i0, %418 ]
  %CurrentObjectToWorld199.2.i1 = phi float [ %170, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12" ], [ %CurrentObjectToWorld199.1.i1, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit221" ], [ %CurrentObjectToWorld199.1.i1, %369 ], [ %CurrentObjectToWorld199.1.i1, %366 ], [ %CurrentObjectToWorld199.1.i1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i" ], [ %CurrentObjectToWorld199.1.i1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit" ], [ %CurrentObjectToWorld199.1.i1, %420 ], [ %CurrentObjectToWorld199.1.i1, %418 ]
  %CurrentObjectToWorld199.2.i2 = phi float [ %171, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12" ], [ %CurrentObjectToWorld199.1.i2, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit221" ], [ %CurrentObjectToWorld199.1.i2, %369 ], [ %CurrentObjectToWorld199.1.i2, %366 ], [ %CurrentObjectToWorld199.1.i2, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i" ], [ %CurrentObjectToWorld199.1.i2, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit" ], [ %CurrentObjectToWorld199.1.i2, %420 ], [ %CurrentObjectToWorld199.1.i2, %418 ]
  %CurrentObjectToWorld199.2.i3 = phi float [ %172, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12" ], [ %CurrentObjectToWorld199.1.i3, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit221" ], [ %CurrentObjectToWorld199.1.i3, %369 ], [ %CurrentObjectToWorld199.1.i3, %366 ], [ %CurrentObjectToWorld199.1.i3, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i" ], [ %CurrentObjectToWorld199.1.i3, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit" ], [ %CurrentObjectToWorld199.1.i3, %420 ], [ %CurrentObjectToWorld199.1.i3, %418 ]
  %CurrentObjectToWorld199.2.i4 = phi float [ %173, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12" ], [ %CurrentObjectToWorld199.1.i4, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit221" ], [ %CurrentObjectToWorld199.1.i4, %369 ], [ %CurrentObjectToWorld199.1.i4, %366 ], [ %CurrentObjectToWorld199.1.i4, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i" ], [ %CurrentObjectToWorld199.1.i4, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit" ], [ %CurrentObjectToWorld199.1.i4, %420 ], [ %CurrentObjectToWorld199.1.i4, %418 ]
  %CurrentObjectToWorld199.2.i5 = phi float [ %174, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12" ], [ %CurrentObjectToWorld199.1.i5, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit221" ], [ %CurrentObjectToWorld199.1.i5, %369 ], [ %CurrentObjectToWorld199.1.i5, %366 ], [ %CurrentObjectToWorld199.1.i5, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i" ], [ %CurrentObjectToWorld199.1.i5, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit" ], [ %CurrentObjectToWorld199.1.i5, %420 ], [ %CurrentObjectToWorld199.1.i5, %418 ]
  %CurrentObjectToWorld199.2.i6 = phi float [ %175, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12" ], [ %CurrentObjectToWorld199.1.i6, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit221" ], [ %CurrentObjectToWorld199.1.i6, %369 ], [ %CurrentObjectToWorld199.1.i6, %366 ], [ %CurrentObjectToWorld199.1.i6, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i" ], [ %CurrentObjectToWorld199.1.i6, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit" ], [ %CurrentObjectToWorld199.1.i6, %420 ], [ %CurrentObjectToWorld199.1.i6, %418 ]
  %CurrentObjectToWorld199.2.i7 = phi float [ %176, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12" ], [ %CurrentObjectToWorld199.1.i7, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit221" ], [ %CurrentObjectToWorld199.1.i7, %369 ], [ %CurrentObjectToWorld199.1.i7, %366 ], [ %CurrentObjectToWorld199.1.i7, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i" ], [ %CurrentObjectToWorld199.1.i7, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit" ], [ %CurrentObjectToWorld199.1.i7, %420 ], [ %CurrentObjectToWorld199.1.i7, %418 ]
  %CurrentObjectToWorld199.2.i8 = phi float [ %177, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12" ], [ %CurrentObjectToWorld199.1.i8, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit221" ], [ %CurrentObjectToWorld199.1.i8, %369 ], [ %CurrentObjectToWorld199.1.i8, %366 ], [ %CurrentObjectToWorld199.1.i8, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i" ], [ %CurrentObjectToWorld199.1.i8, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit" ], [ %CurrentObjectToWorld199.1.i8, %420 ], [ %CurrentObjectToWorld199.1.i8, %418 ]
  %CurrentObjectToWorld199.2.i9 = phi float [ %178, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12" ], [ %CurrentObjectToWorld199.1.i9, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit221" ], [ %CurrentObjectToWorld199.1.i9, %369 ], [ %CurrentObjectToWorld199.1.i9, %366 ], [ %CurrentObjectToWorld199.1.i9, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i" ], [ %CurrentObjectToWorld199.1.i9, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit" ], [ %CurrentObjectToWorld199.1.i9, %420 ], [ %CurrentObjectToWorld199.1.i9, %418 ]
  %CurrentObjectToWorld199.2.i10 = phi float [ %179, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12" ], [ %CurrentObjectToWorld199.1.i10, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit221" ], [ %CurrentObjectToWorld199.1.i10, %369 ], [ %CurrentObjectToWorld199.1.i10, %366 ], [ %CurrentObjectToWorld199.1.i10, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i" ], [ %CurrentObjectToWorld199.1.i10, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit" ], [ %CurrentObjectToWorld199.1.i10, %420 ], [ %CurrentObjectToWorld199.1.i10, %418 ]
  %CurrentObjectToWorld199.2.i11 = phi float [ %180, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12" ], [ %CurrentObjectToWorld199.1.i11, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit221" ], [ %CurrentObjectToWorld199.1.i11, %369 ], [ %CurrentObjectToWorld199.1.i11, %366 ], [ %CurrentObjectToWorld199.1.i11, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i" ], [ %CurrentObjectToWorld199.1.i11, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit" ], [ %CurrentObjectToWorld199.1.i11, %420 ], [ %CurrentObjectToWorld199.1.i11, %418 ]
  %CurrentWorldToObject200.2.i0 = phi float [ %152, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12" ], [ %CurrentWorldToObject200.1.i0, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit221" ], [ %CurrentWorldToObject200.1.i0, %369 ], [ %CurrentWorldToObject200.1.i0, %366 ], [ %CurrentWorldToObject200.1.i0, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i" ], [ %CurrentWorldToObject200.1.i0, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit" ], [ %CurrentWorldToObject200.1.i0, %420 ], [ %CurrentWorldToObject200.1.i0, %418 ]
  %CurrentWorldToObject200.2.i1 = phi float [ %153, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12" ], [ %CurrentWorldToObject200.1.i1, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit221" ], [ %CurrentWorldToObject200.1.i1, %369 ], [ %CurrentWorldToObject200.1.i1, %366 ], [ %CurrentWorldToObject200.1.i1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i" ], [ %CurrentWorldToObject200.1.i1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit" ], [ %CurrentWorldToObject200.1.i1, %420 ], [ %CurrentWorldToObject200.1.i1, %418 ]
  %CurrentWorldToObject200.2.i2 = phi float [ %154, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12" ], [ %CurrentWorldToObject200.1.i2, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit221" ], [ %CurrentWorldToObject200.1.i2, %369 ], [ %CurrentWorldToObject200.1.i2, %366 ], [ %CurrentWorldToObject200.1.i2, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i" ], [ %CurrentWorldToObject200.1.i2, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit" ], [ %CurrentWorldToObject200.1.i2, %420 ], [ %CurrentWorldToObject200.1.i2, %418 ]
  %CurrentWorldToObject200.2.i3 = phi float [ %155, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12" ], [ %CurrentWorldToObject200.1.i3, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit221" ], [ %CurrentWorldToObject200.1.i3, %369 ], [ %CurrentWorldToObject200.1.i3, %366 ], [ %CurrentWorldToObject200.1.i3, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i" ], [ %CurrentWorldToObject200.1.i3, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit" ], [ %CurrentWorldToObject200.1.i3, %420 ], [ %CurrentWorldToObject200.1.i3, %418 ]
  %CurrentWorldToObject200.2.i4 = phi float [ %156, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12" ], [ %CurrentWorldToObject200.1.i4, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit221" ], [ %CurrentWorldToObject200.1.i4, %369 ], [ %CurrentWorldToObject200.1.i4, %366 ], [ %CurrentWorldToObject200.1.i4, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i" ], [ %CurrentWorldToObject200.1.i4, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit" ], [ %CurrentWorldToObject200.1.i4, %420 ], [ %CurrentWorldToObject200.1.i4, %418 ]
  %CurrentWorldToObject200.2.i5 = phi float [ %157, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12" ], [ %CurrentWorldToObject200.1.i5, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit221" ], [ %CurrentWorldToObject200.1.i5, %369 ], [ %CurrentWorldToObject200.1.i5, %366 ], [ %CurrentWorldToObject200.1.i5, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i" ], [ %CurrentWorldToObject200.1.i5, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit" ], [ %CurrentWorldToObject200.1.i5, %420 ], [ %CurrentWorldToObject200.1.i5, %418 ]
  %CurrentWorldToObject200.2.i6 = phi float [ %158, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12" ], [ %CurrentWorldToObject200.1.i6, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit221" ], [ %CurrentWorldToObject200.1.i6, %369 ], [ %CurrentWorldToObject200.1.i6, %366 ], [ %CurrentWorldToObject200.1.i6, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i" ], [ %CurrentWorldToObject200.1.i6, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit" ], [ %CurrentWorldToObject200.1.i6, %420 ], [ %CurrentWorldToObject200.1.i6, %418 ]
  %CurrentWorldToObject200.2.i7 = phi float [ %159, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12" ], [ %CurrentWorldToObject200.1.i7, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit221" ], [ %CurrentWorldToObject200.1.i7, %369 ], [ %CurrentWorldToObject200.1.i7, %366 ], [ %CurrentWorldToObject200.1.i7, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i" ], [ %CurrentWorldToObject200.1.i7, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit" ], [ %CurrentWorldToObject200.1.i7, %420 ], [ %CurrentWorldToObject200.1.i7, %418 ]
  %CurrentWorldToObject200.2.i8 = phi float [ %160, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12" ], [ %CurrentWorldToObject200.1.i8, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit221" ], [ %CurrentWorldToObject200.1.i8, %369 ], [ %CurrentWorldToObject200.1.i8, %366 ], [ %CurrentWorldToObject200.1.i8, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i" ], [ %CurrentWorldToObject200.1.i8, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit" ], [ %CurrentWorldToObject200.1.i8, %420 ], [ %CurrentWorldToObject200.1.i8, %418 ]
  %CurrentWorldToObject200.2.i9 = phi float [ %161, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12" ], [ %CurrentWorldToObject200.1.i9, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit221" ], [ %CurrentWorldToObject200.1.i9, %369 ], [ %CurrentWorldToObject200.1.i9, %366 ], [ %CurrentWorldToObject200.1.i9, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i" ], [ %CurrentWorldToObject200.1.i9, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit" ], [ %CurrentWorldToObject200.1.i9, %420 ], [ %CurrentWorldToObject200.1.i9, %418 ]
  %CurrentWorldToObject200.2.i10 = phi float [ %162, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12" ], [ %CurrentWorldToObject200.1.i10, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit221" ], [ %CurrentWorldToObject200.1.i10, %369 ], [ %CurrentWorldToObject200.1.i10, %366 ], [ %CurrentWorldToObject200.1.i10, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i" ], [ %CurrentWorldToObject200.1.i10, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit" ], [ %CurrentWorldToObject200.1.i10, %420 ], [ %CurrentWorldToObject200.1.i10, %418 ]
  %CurrentWorldToObject200.2.i11 = phi float [ %163, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12" ], [ %CurrentWorldToObject200.1.i11, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit221" ], [ %CurrentWorldToObject200.1.i11, %369 ], [ %CurrentWorldToObject200.1.i11, %366 ], [ %CurrentWorldToObject200.1.i11, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i" ], [ %CurrentWorldToObject200.1.i11, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit" ], [ %CurrentWorldToObject200.1.i11, %420 ], [ %CurrentWorldToObject200.1.i11, %418 ]
  %ResetMatrices.3 = phi i32 [ %ResetMatrices.1, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12" ], [ %ResetMatrices.1, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit221" ], [ 0, %369 ], [ 0, %366 ], [ %ResetMatrices.1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i" ], [ %ResetMatrices.1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit" ], [ %ResetMatrices.1, %420 ], [ %ResetMatrices.1, %418 ]
  %currentBVHIndex.2 = phi i32 [ 1, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12" ], [ 0, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit221" ], [ %currentBVHIndex.1, %369 ], [ %currentBVHIndex.1, %366 ], [ %currentBVHIndex.1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i" ], [ %currentBVHIndex.1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit" ], [ %currentBVHIndex.1, %420 ], [ %currentBVHIndex.1, %418 ]
  %currentGpuVA.2.i0 = phi i32 [ %132, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12" ], [ %currentGpuVA.1.i0, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit221" ], [ %currentGpuVA.1.i0, %369 ], [ %currentGpuVA.1.i0, %366 ], [ %currentGpuVA.1.i0, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i" ], [ %currentGpuVA.1.i0, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit" ], [ %currentGpuVA.1.i0, %420 ], [ %currentGpuVA.1.i0, %418 ]
  %currentGpuVA.2.i1 = phi i32 [ %133, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12" ], [ %currentGpuVA.1.i1, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit221" ], [ %currentGpuVA.1.i1, %369 ], [ %currentGpuVA.1.i1, %366 ], [ %currentGpuVA.1.i1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i" ], [ %currentGpuVA.1.i1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit" ], [ %currentGpuVA.1.i1, %420 ], [ %currentGpuVA.1.i1, %418 ]
  %instIdx.2 = phi i32 [ %107, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12" ], [ %107, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit221" ], [ %instIdx.1, %369 ], [ %instIdx.1, %366 ], [ %instIdx.1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i" ], [ %instIdx.1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit" ], [ %instIdx.1, %420 ], [ %instIdx.1, %418 ]
  %instFlags.2 = phi i32 [ %182, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12" ], [ %instFlags.1, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit221" ], [ %instFlags.1, %369 ], [ %instFlags.1, %366 ], [ %instFlags.1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i" ], [ %instFlags.1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit" ], [ %instFlags.1, %420 ], [ %instFlags.1, %418 ]
  %instOffset.2 = phi i32 [ %164, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12" ], [ %164, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit221" ], [ %instOffset.1, %369 ], [ %instOffset.1, %366 ], [ %instOffset.1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i" ], [ %instOffset.1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit" ], [ %instOffset.1, %420 ], [ %instOffset.1, %418 ]
  %instId.2 = phi i32 [ %165, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12" ], [ %165, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit221" ], [ %instId.1, %369 ], [ %instId.1, %366 ], [ %instId.1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i" ], [ %instId.1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit" ], [ %instId.1, %420 ], [ %instId.1, %418 ]
  %stackPointer.3 = phi i32 [ %stackPointer.2, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12" ], [ %86, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit221" ], [ %86, %369 ], [ %86, %366 ], [ %86, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i" ], [ %416, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit" ], [ %stackPointer.2, %420 ], [ %86, %418 ]
  %resultBary.2.i0 = phi float [ %resultBary.1.i0, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12" ], [ %resultBary.1.i0, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit221" ], [ %.2.i0.resultBary.1.i0, %369 ], [ %.2.i0.resultBary.1.i0, %366 ], [ %.2.i0.resultBary.1.i0, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i" ], [ %resultBary.1.i0, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit" ], [ %resultBary.1.i0, %420 ], [ %resultBary.1.i0, %418 ]
  %resultBary.2.i1 = phi float [ %resultBary.1.i1, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12" ], [ %resultBary.1.i1, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit221" ], [ %.2.i1.resultBary.1.i1, %369 ], [ %.2.i1.resultBary.1.i1, %366 ], [ %.2.i1.resultBary.1.i1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i" ], [ %resultBary.1.i1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit" ], [ %resultBary.1.i1, %420 ], [ %resultBary.1.i1, %418 ]
  %resultTriId.2 = phi i32 [ %resultTriId.1, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12" ], [ %resultTriId.1, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit221" ], [ %.resultTriId.1, %369 ], [ %.resultTriId.1, %366 ], [ %.resultTriId.1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i" ], [ %resultTriId.1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit" ], [ %resultTriId.1, %420 ], [ %resultTriId.1, %418 ]
  %423 = getelementptr inbounds [2 x i32], [2 x i32]* %nodesToProcess, i32 0, i32 %currentBVHIndex.2
  %424 = load i32, i32* %423, align 4, !tbaa !221
  %425 = icmp eq i32 %424, 0
  br i1 %425, label %426, label %84

; <label>:426                                     ; preds = %422
  %427 = add i32 %currentBVHIndex.2, -1
  %428 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %AccelerationStructureList, i32 0)
  %429 = extractvalue %dx.types.CBufRet.i32 %428, 0
  %430 = extractvalue %dx.types.CBufRet.i32 %428, 1
  %431 = load i32, i32* %37, align 4, !tbaa !221
  %432 = icmp eq i32 %431, 0
  br i1 %432, label %._crit_edge, label %.lr.ph

._crit_edge:                                      ; preds = %426, %83
  %InstanceIndex = call i32 @dx.op.instanceIndex.i32(i32 142)
  %433 = icmp ne i32 %InstanceIndex, -1
  ret i1 %433
}

; Function Attrs: alwaysinline nounwind
define i32 @"\01?StackPop@@YAIAIAHAIAII@Z"(i32* noalias nocapture dereferenceable(4), i32* noalias nocapture readnone dereferenceable(4), i32) #5 {
  %4 = load i32, i32* %0, align 4, !tbaa !221
  %5 = add nsw i32 %4, -1
  store i32 %5, i32* %0, align 4, !tbaa !221
  %6 = shl i32 %5, 6
  %7 = add i32 %6, %2
  %8 = getelementptr [2048 x i32], [2048 x i32] addrspace(3)* @"\01?stack@@3PAIA", i32 0, i32 %7
  %9 = load i32, i32 addrspace(3)* %8, align 4, !tbaa !221
  ret i32 %9
}

; Function Attrs: alwaysinline nounwind readnone
define i32 @"\01?GetRightNodeIndex@@YAIV?$vector@I$01@@@Z"(<2 x i32>) #6 {
  %2 = extractelement <2 x i32> %0, i32 1
  ret i32 %2
}

; Function Attrs: alwaysinline nounwind
define void @"\01?CreateRWByteAddressBufferPointer@@YA?AURWByteAddressBufferPointer@@URWByteAddressBuffer@@I@Z"(%struct.RWByteAddressBufferPointer* noalias nocapture sret, %struct.RWByteAddressBuffer* nocapture readonly, i32) #5 {
  %4 = getelementptr inbounds %struct.RWByteAddressBufferPointer, %struct.RWByteAddressBufferPointer* %0, i32 0, i32 0
  %5 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %1, align 4
  store %struct.RWByteAddressBuffer %5, %struct.RWByteAddressBuffer* %4, align 4
  %6 = getelementptr inbounds %struct.RWByteAddressBufferPointer, %struct.RWByteAddressBufferPointer* %0, i32 0, i32 1
  store i32 %2, i32* %6, align 4, !tbaa !221
  ret void
}

; Function Attrs: alwaysinline nounwind
define void @"\01?StackPush@@YAXAIAHIII@Z"(i32* noalias nocapture dereferenceable(4), i32, i32, i32) #5 {
  %5 = load i32, i32* %0, align 4, !tbaa !221
  %6 = shl i32 %5, 6
  %7 = add i32 %6, %3
  %8 = getelementptr [2048 x i32], [2048 x i32] addrspace(3)* @"\01?stack@@3PAIA", i32 0, i32 %7
  store i32 %1, i32 addrspace(3)* %8, align 4, !tbaa !221
  %9 = add nsw i32 %5, 1
  store i32 %9, i32* %0, align 4, !tbaa !221
  ret void
}

; Function Attrs: alwaysinline nounwind readnone
define void @"\01?LogFloat@@YAXM@Z"(float) #6 {
  ret void
}

; Function Attrs: alwaysinline nounwind readnone
define i32 @"\01?GetBoxAddress@@YAIII@Z"(i32, i32) #6 {
  %3 = shl i32 %1, 5
  %4 = add i32 %3, %0
  ret i32 %4
}

; Function Attrs: alwaysinline nounwind
define void @"\01?StackPush2@@YAXAIAH_NIIII@Z"(i32* noalias nocapture dereferenceable(4), i1 zeroext, i32, i32, i32, i32) #5 {
  %. = select i1 %1, i32 %2, i32 %3
  %7 = select i1 %1, i32 %3, i32 %2
  %8 = load i32, i32* %0, align 4, !tbaa !221
  %9 = shl i32 %8, 6
  %10 = add i32 %9, %5
  %11 = add i32 %5, 64
  %12 = add i32 %11, %9
  %13 = getelementptr [2048 x i32], [2048 x i32] addrspace(3)* @"\01?stack@@3PAIA", i32 0, i32 %10
  store i32 %., i32 addrspace(3)* %13, align 4, !tbaa !221
  %14 = getelementptr [2048 x i32], [2048 x i32] addrspace(3)* @"\01?stack@@3PAIA", i32 0, i32 %12
  store i32 %7, i32 addrspace(3)* %14, align 4, !tbaa !221
  %15 = add nsw i32 %8, 2
  store i32 %15, i32* %0, align 4, !tbaa !221
  ret void
}

; Function Attrs: alwaysinline nounwind readnone
define void @"\01?LogTraceRayEnd@@YAXXZ"() #6 {
  ret void
}

; Function Attrs: alwaysinline nounwind readnone
define void @"\01?LogFloat3@@YAXV?$vector@M$02@@@Z"(<3 x float>) #6 {
  ret void
}

; Function Attrs: alwaysinline nounwind readonly
define i32 @"\01?GetInstanceID@@YAIURaytracingInstanceDesc@@@Z"(%struct.RaytracingInstanceDesc* nocapture readonly) #4 {
  %2 = getelementptr inbounds %struct.RaytracingInstanceDesc, %struct.RaytracingInstanceDesc* %0, i32 0, i32 1
  %3 = load i32, i32* %2, align 4, !tbaa !221
  %4 = and i32 %3, 16777215
  ret i32 %4
}

; Function Attrs: alwaysinline nounwind
define void @"\01?dump@@YAXUBoundingBox@@V?$vector@I$01@@@Z"(%struct.BoundingBox* nocapture readonly, <2 x i32>) #5 {
  ret void
}

; Function Attrs: alwaysinline nounwind
define i32 @"\01?InvokeAnyHit@@YAHH@Z"(i32) #5 {
  call void @"\01?Fallback_SetAnyHitResult@@YAXH@Z"(i32 1) #3
  call void @"\01?Fallback_CallIndirect@@YAXH@Z"(i32 %0) #3
  %2 = call i32 @"\01?Fallback_AnyHitResult@@YAHXZ"() #3
  ret i32 %2
}

; Function Attrs: alwaysinline nounwind readnone
define float @"\01?ComputeCullFaceDir@@YAMII@Z"(i32, i32) #6 {
  %3 = and i32 %1, 32
  %4 = icmp eq i32 %3, 0
  br i1 %4, label %5, label %8

; <label>:5                                       ; preds = %2
  %6 = and i32 %1, 16
  %7 = icmp eq i32 %6, 0
  %. = select i1 %7, float 0.000000e+00, float -1.000000e+00
  br label %8

; <label>:8                                       ; preds = %5, %2
  %cullFaceDir.0 = phi float [ 1.000000e+00, %2 ], [ %., %5 ]
  %9 = and i32 %0, 1
  %10 = icmp eq i32 %9, 0
  %cullFaceDir.0. = select i1 %10, float %cullFaceDir.0, float 0.000000e+00
  ret float %cullFaceDir.0.
}

; Function Attrs: alwaysinline nounwind
define void @"\01?CompressBox@@YAXUBoundingBox@@V?$vector@I$01@@AIAV?$vector@I$03@@2@Z"(%struct.BoundingBox* nocapture readonly, <2 x i32>, <4 x i32>* noalias nocapture dereferenceable(16), <4 x i32>* noalias nocapture dereferenceable(16)) #5 {
  %5 = getelementptr inbounds %struct.BoundingBox, %struct.BoundingBox* %0, i32 0, i32 0
  %6 = load <3 x float>, <3 x float>* %5, align 4
  %7 = extractelement <3 x float> %6, i32 0
  %8 = bitcast float %7 to i32
  %9 = insertelement <4 x i32> undef, i32 %8, i32 0
  %10 = extractelement <3 x float> %6, i32 1
  %11 = bitcast float %10 to i32
  %12 = insertelement <4 x i32> %9, i32 %11, i32 1
  %13 = extractelement <3 x float> %6, i32 2
  %14 = bitcast float %13 to i32
  %15 = insertelement <4 x i32> %12, i32 %14, i32 2
  %16 = extractelement <2 x i32> %1, i32 0
  %17 = insertelement <4 x i32> %15, i32 %16, i32 3
  %18 = getelementptr inbounds %struct.BoundingBox, %struct.BoundingBox* %0, i32 0, i32 1
  %19 = load <3 x float>, <3 x float>* %18, align 4
  %20 = extractelement <3 x float> %19, i32 0
  %21 = bitcast float %20 to i32
  %22 = insertelement <4 x i32> undef, i32 %21, i32 0
  %23 = extractelement <3 x float> %19, i32 1
  %24 = bitcast float %23 to i32
  %25 = insertelement <4 x i32> %22, i32 %24, i32 1
  %26 = extractelement <3 x float> %19, i32 2
  %27 = bitcast float %26 to i32
  %28 = insertelement <4 x i32> %25, i32 %27, i32 2
  %29 = extractelement <2 x i32> %1, i32 1
  %30 = insertelement <4 x i32> %28, i32 %29, i32 3
  store <4 x i32> %17, <4 x i32>* %2, align 4
  store <4 x i32> %30, <4 x i32>* %3, align 4
  ret void
}

; Function Attrs: alwaysinline nounwind
define void @"\01?CreateRWByteAddressBufferPointerFromGpuVA@@YA?AURWByteAddressBufferPointer@@V?$vector@I$01@@@Z"(%struct.RWByteAddressBufferPointer* noalias nocapture sret, <2 x i32>) #5 {
  %3 = extractelement <2 x i32> %1, i32 0
  %4 = getelementptr inbounds %struct.RWByteAddressBufferPointer, %struct.RWByteAddressBufferPointer* %0, i32 0, i32 1
  store i32 %3, i32* %4, align 4, !tbaa !221, !alias.scope !277
  ret void
}

; Function Attrs: alwaysinline nounwind
define void @"\01?AABBtoBoundingBox@@YA?AUBoundingBox@@UAABB@@@Z"(%struct.BoundingBox* noalias nocapture sret, %struct.AABB* nocapture readonly) #5 {
  %3 = getelementptr inbounds %struct.AABB, %struct.AABB* %1, i32 0, i32 0
  %4 = load <3 x float>, <3 x float>* %3, align 4, !tbaa !218
  %.i01 = extractelement <3 x float> %4, i32 0
  %.i13 = extractelement <3 x float> %4, i32 1
  %.i25 = extractelement <3 x float> %4, i32 2
  %5 = getelementptr inbounds %struct.AABB, %struct.AABB* %1, i32 0, i32 1
  %6 = load <3 x float>, <3 x float>* %5, align 4, !tbaa !218
  %.i0 = extractelement <3 x float> %6, i32 0
  %.i02 = fadd fast float %.i0, %.i01
  %.i1 = extractelement <3 x float> %6, i32 1
  %.i14 = fadd fast float %.i1, %.i13
  %.i2 = extractelement <3 x float> %6, i32 2
  %.i26 = fadd fast float %.i2, %.i25
  %.i07 = fmul fast float %.i02, 5.000000e-01
  %.i18 = fmul fast float %.i14, 5.000000e-01
  %.i29 = fmul fast float %.i26, 5.000000e-01
  %.upto0 = insertelement <3 x float> undef, float %.i07, i32 0
  %.upto1 = insertelement <3 x float> %.upto0, float %.i18, i32 1
  %7 = insertelement <3 x float> %.upto1, float %.i29, i32 2
  %.i011 = fsub fast float %.i0, %.i07
  %.i113 = fsub fast float %.i1, %.i18
  %.i215 = fsub fast float %.i2, %.i29
  %.upto016 = insertelement <3 x float> undef, float %.i011, i32 0
  %.upto117 = insertelement <3 x float> %.upto016, float %.i113, i32 1
  %8 = insertelement <3 x float> %.upto117, float %.i215, i32 2
  %9 = getelementptr inbounds %struct.BoundingBox, %struct.BoundingBox* %0, i32 0, i32 0
  store <3 x float> %7, <3 x float>* %9, align 4
  %10 = getelementptr inbounds %struct.BoundingBox, %struct.BoundingBox* %0, i32 0, i32 1
  store <3 x float> %8, <3 x float>* %10, align 4
  ret void
}

; Function Attrs: alwaysinline nounwind
define void @"\01?RawDataToAABB@@YA?AUAABB@@V?$vector@H$03@@0@Z"(%struct.AABB* noalias nocapture sret, <4 x i32>, <4 x i32>) #5 {
  %4 = extractelement <4 x i32> %1, i32 0
  %5 = bitcast i32 %4 to float
  %6 = extractelement <4 x i32> %1, i32 1
  %7 = bitcast i32 %6 to float
  %8 = extractelement <4 x i32> %1, i32 2
  %9 = bitcast i32 %8 to float
  %10 = extractelement <4 x i32> %2, i32 0
  %11 = bitcast i32 %10 to float
  %12 = extractelement <4 x i32> %2, i32 1
  %13 = bitcast i32 %12 to float
  %14 = extractelement <4 x i32> %2, i32 2
  %15 = bitcast i32 %14 to float
  %.i0 = fsub fast float %5, %11
  %.i1 = fsub fast float %7, %13
  %.i2 = fsub fast float %9, %15
  %.upto0 = insertelement <3 x float> undef, float %.i0, i32 0
  %.upto1 = insertelement <3 x float> %.upto0, float %.i1, i32 1
  %16 = insertelement <3 x float> %.upto1, float %.i2, i32 2
  %17 = getelementptr inbounds %struct.AABB, %struct.AABB* %0, i32 0, i32 0
  store <3 x float> %16, <3 x float>* %17, align 4, !tbaa !218, !alias.scope !280
  %.i03 = fadd fast float %11, %5
  %.i14 = fadd fast float %13, %7
  %.i25 = fadd fast float %15, %9
  %.upto06 = insertelement <3 x float> undef, float %.i03, i32 0
  %.upto17 = insertelement <3 x float> %.upto06, float %.i14, i32 1
  %18 = insertelement <3 x float> %.upto17, float %.i25, i32 2
  %19 = getelementptr inbounds %struct.AABB, %struct.AABB* %0, i32 0, i32 1
  store <3 x float> %18, <3 x float>* %19, align 4, !tbaa !218, !alias.scope !280
  ret void
}

; Function Attrs: alwaysinline nounwind readonly
define <3 x float> @"\01?GetMinCorner@@YA?AV?$vector@M$02@@UBoundingBox@@@Z"(%struct.BoundingBox* nocapture readonly) #4 {
  %2 = getelementptr inbounds %struct.BoundingBox, %struct.BoundingBox* %0, i32 0, i32 0
  %3 = load <3 x float>, <3 x float>* %2, align 4, !tbaa !218
  %.i01 = extractelement <3 x float> %3, i32 0
  %.i13 = extractelement <3 x float> %3, i32 1
  %.i25 = extractelement <3 x float> %3, i32 2
  %4 = getelementptr inbounds %struct.BoundingBox, %struct.BoundingBox* %0, i32 0, i32 1
  %5 = load <3 x float>, <3 x float>* %4, align 4, !tbaa !218
  %.i0 = extractelement <3 x float> %5, i32 0
  %.i02 = fsub fast float %.i01, %.i0
  %.i1 = extractelement <3 x float> %5, i32 1
  %.i14 = fsub fast float %.i13, %.i1
  %.i2 = extractelement <3 x float> %5, i32 2
  %.i26 = fsub fast float %.i25, %.i2
  %.upto0 = insertelement <3 x float> undef, float %.i02, i32 0
  %.upto1 = insertelement <3 x float> %.upto0, float %.i14, i32 1
  %6 = insertelement <3 x float> %.upto1, float %.i26, i32 2
  ret <3 x float> %6
}

; Function Attrs: alwaysinline nounwind readnone
define i1 @"\01?IsOpaque@@YA_N_NII@Z"(i1 zeroext, i32, i32) #6 {
  %4 = and i32 %1, 4
  %5 = icmp eq i32 %4, 0
  br i1 %5, label %6, label %10

; <label>:6                                       ; preds = %3
  %7 = zext i1 %0 to i32
  %8 = and i32 %1, 8
  %9 = icmp eq i32 %8, 0
  %. = select i1 %9, i32 %7, i32 0
  br label %10

; <label>:10                                      ; preds = %6, %3
  %opaque.0 = phi i32 [ 1, %3 ], [ %., %6 ]
  %11 = and i32 %2, 1
  %12 = icmp eq i32 %11, 0
  br i1 %12, label %13, label %16

; <label>:13                                      ; preds = %10
  %14 = and i32 %2, 2
  %15 = icmp eq i32 %14, 0
  %opaque.0. = select i1 %15, i32 %opaque.0, i32 0
  br label %16

; <label>:16                                      ; preds = %13, %10
  %opaque.1 = phi i32 [ 1, %10 ], [ %opaque.0., %13 ]
  %17 = icmp ne i32 %opaque.1, 0
  ret i1 %17
}

; Function Attrs: alwaysinline
define void @main() #8 {
  %1 = load %struct.ByteAddressBuffer, %struct.ByteAddressBuffer* @"\01?RayGenShaderTable@@3UByteAddressBuffer@@A", align 4
  %2 = load %Constants, %Constants* @Constants, align 4
  %Constants = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.Constants(i32 160, %Constants %2)
  %3 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.ByteAddressBuffer(i32 160, %struct.ByteAddressBuffer %1)
  %4 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %3, i32 0, i32 undef)
  %5 = extractvalue %dx.types.ResRet.i32 %4, 0
  %6 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %Constants, i32 0)
  %7 = extractvalue %dx.types.CBufRet.i32 %6, 1
  %8 = extractvalue %dx.types.CBufRet.i32 %6, 0
  call void @fb_Fallback_Scheduler(i32 %5, i32 %8, i32 %7)
  ret void
}

; Function Attrs: alwaysinline nounwind readnone
define void @"\01?LogInt3@@YAXV?$vector@H$02@@@Z"(<3 x i32>) #6 {
  ret void
}

; Function Attrs: alwaysinline nounwind readonly
define <4 x i32> @"\01?Load4@@YA?AV?$vector@I$03@@V?$vector@I$01@@@Z"(<2 x i32>) #4 {
  %2 = extractelement <2 x i32> %0, i32 1
  %3 = getelementptr inbounds [0 x %struct.RWByteAddressBuffer], [0 x %struct.RWByteAddressBuffer]* @"\01?DescriptorHeapBufferTable@@3PAURWByteAddressBuffer@@A", i32 0, i32 %2, !dx.nonuniform !240
  %4 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %3, align 4, !noalias !283
  %5 = extractelement <2 x i32> %0, i32 0
  %6 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %4)
  %7 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %6, i32 %5, i32 undef)
  %8 = extractvalue %dx.types.ResRet.i32 %7, 0
  %9 = insertelement <4 x i32> undef, i32 %8, i64 0
  %10 = extractvalue %dx.types.ResRet.i32 %7, 1
  %11 = insertelement <4 x i32> %9, i32 %10, i64 1
  %12 = extractvalue %dx.types.ResRet.i32 %7, 2
  %13 = insertelement <4 x i32> %11, i32 %12, i64 2
  %14 = extractvalue %dx.types.ResRet.i32 %7, 3
  %15 = insertelement <4 x i32> %13, i32 %14, i64 3
  ret <4 x i32> %15
}

; Function Attrs: alwaysinline nounwind
define void @"\01?LoadRaytracingInstanceDesc@@YA?AURaytracingInstanceDesc@@URWByteAddressBuffer@@I@Z"(%struct.RaytracingInstanceDesc* noalias nocapture sret, %struct.RWByteAddressBuffer* nocapture readonly, i32) #5 {
  %4 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %1, align 4
  %5 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %4)
  %6 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %5, i32 %2, i32 undef)
  %7 = extractvalue %dx.types.ResRet.i32 %6, 0
  %8 = extractvalue %dx.types.ResRet.i32 %6, 1
  %9 = extractvalue %dx.types.ResRet.i32 %6, 2
  %10 = extractvalue %dx.types.ResRet.i32 %6, 3
  %11 = add i32 %2, 16
  %12 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %5, i32 %11, i32 undef)
  %13 = extractvalue %dx.types.ResRet.i32 %12, 0
  %14 = extractvalue %dx.types.ResRet.i32 %12, 1
  %15 = extractvalue %dx.types.ResRet.i32 %12, 2
  %16 = extractvalue %dx.types.ResRet.i32 %12, 3
  %17 = add i32 %2, 32
  %18 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %5, i32 %17, i32 undef)
  %19 = extractvalue %dx.types.ResRet.i32 %18, 0
  %20 = extractvalue %dx.types.ResRet.i32 %18, 1
  %21 = extractvalue %dx.types.ResRet.i32 %18, 2
  %22 = extractvalue %dx.types.ResRet.i32 %18, 3
  %23 = add i32 %2, 48
  %24 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %5, i32 %23, i32 undef)
  %25 = extractvalue %dx.types.ResRet.i32 %24, 0
  %26 = extractvalue %dx.types.ResRet.i32 %24, 1
  %27 = extractvalue %dx.types.ResRet.i32 %24, 2
  %28 = extractvalue %dx.types.ResRet.i32 %24, 3
  %.i0 = bitcast i32 %7 to float
  %.i1 = bitcast i32 %8 to float
  %.i2 = bitcast i32 %9 to float
  %.i3 = bitcast i32 %10 to float
  %.upto0 = insertelement <4 x float> undef, float %.i0, i32 0
  %.upto1 = insertelement <4 x float> %.upto0, float %.i1, i32 1
  %.upto2 = insertelement <4 x float> %.upto1, float %.i2, i32 2
  %29 = insertelement <4 x float> %.upto2, float %.i3, i32 3
  %30 = getelementptr inbounds %struct.RaytracingInstanceDesc, %struct.RaytracingInstanceDesc* %0, i32 0, i32 0, i32 0
  store <4 x float> %29, <4 x float>* %30, align 4, !tbaa !218, !alias.scope !286
  %.i035 = bitcast i32 %13 to float
  %.i136 = bitcast i32 %14 to float
  %.i237 = bitcast i32 %15 to float
  %.i338 = bitcast i32 %16 to float
  %.upto043 = insertelement <4 x float> undef, float %.i035, i32 0
  %.upto144 = insertelement <4 x float> %.upto043, float %.i136, i32 1
  %.upto245 = insertelement <4 x float> %.upto144, float %.i237, i32 2
  %31 = insertelement <4 x float> %.upto245, float %.i338, i32 3
  %32 = getelementptr inbounds %struct.RaytracingInstanceDesc, %struct.RaytracingInstanceDesc* %0, i32 0, i32 0, i32 1
  store <4 x float> %31, <4 x float>* %32, align 4, !tbaa !218, !alias.scope !286
  %.i039 = bitcast i32 %19 to float
  %.i140 = bitcast i32 %20 to float
  %.i241 = bitcast i32 %21 to float
  %.i342 = bitcast i32 %22 to float
  %.upto046 = insertelement <4 x float> undef, float %.i039, i32 0
  %.upto147 = insertelement <4 x float> %.upto046, float %.i140, i32 1
  %.upto248 = insertelement <4 x float> %.upto147, float %.i241, i32 2
  %33 = insertelement <4 x float> %.upto248, float %.i342, i32 3
  %34 = getelementptr inbounds %struct.RaytracingInstanceDesc, %struct.RaytracingInstanceDesc* %0, i32 0, i32 0, i32 2
  store <4 x float> %33, <4 x float>* %34, align 4, !tbaa !218, !alias.scope !286
  %35 = getelementptr inbounds %struct.RaytracingInstanceDesc, %struct.RaytracingInstanceDesc* %0, i32 0, i32 1
  store i32 %25, i32* %35, align 4, !tbaa !221, !alias.scope !286
  %36 = getelementptr inbounds %struct.RaytracingInstanceDesc, %struct.RaytracingInstanceDesc* %0, i32 0, i32 2
  store i32 %26, i32* %36, align 4, !tbaa !221, !alias.scope !286
  %.upto049 = insertelement <2 x i32> undef, i32 %27, i32 0
  %37 = insertelement <2 x i32> %.upto049, i32 %28, i32 1
  %38 = getelementptr inbounds %struct.RaytracingInstanceDesc, %struct.RaytracingInstanceDesc* %0, i32 0, i32 3
  store <2 x i32> %37, <2 x i32>* %38, align 4, !tbaa !218, !alias.scope !286
  ret void
}

; Function Attrs: alwaysinline nounwind readonly
define <3 x float> @"\01?GetMaxCorner@@YA?AV?$vector@M$02@@UBoundingBox@@@Z"(%struct.BoundingBox* nocapture readonly) #4 {
  %2 = getelementptr inbounds %struct.BoundingBox, %struct.BoundingBox* %0, i32 0, i32 0
  %3 = load <3 x float>, <3 x float>* %2, align 4, !tbaa !218
  %.i01 = extractelement <3 x float> %3, i32 0
  %.i13 = extractelement <3 x float> %3, i32 1
  %.i25 = extractelement <3 x float> %3, i32 2
  %4 = getelementptr inbounds %struct.BoundingBox, %struct.BoundingBox* %0, i32 0, i32 1
  %5 = load <3 x float>, <3 x float>* %4, align 4, !tbaa !218
  %.i0 = extractelement <3 x float> %5, i32 0
  %.i02 = fadd fast float %.i0, %.i01
  %.i1 = extractelement <3 x float> %5, i32 1
  %.i14 = fadd fast float %.i1, %.i13
  %.i2 = extractelement <3 x float> %5, i32 2
  %.i26 = fadd fast float %.i2, %.i25
  %.upto0 = insertelement <3 x float> undef, float %.i02, i32 0
  %.upto1 = insertelement <3 x float> %.upto0, float %.i14, i32 1
  %6 = insertelement <3 x float> %.upto1, float %.i26, i32 2
  ret <3 x float> %6
}

; Function Attrs: alwaysinline nounwind
declare void @Hit(%struct.RayPayload* noalias nocapture, %struct.BuiltInTriangleIntersectionAttributes* nocapture readonly) #5

; Function Attrs: alwaysinline nounwind
define void @"\01?RecordClosestBox@@YAXIAIA_NM0MAIAM@Z"(i32, i32* noalias nocapture dereferenceable(4), float, i32* noalias nocapture dereferenceable(4), float, float* noalias nocapture dereferenceable(4)) #5 {
  ret void
}

; Function Attrs: alwaysinline nounwind
define void @"\01?GetBoxDataFromTriangle@@YA?AUBoundingBox@@V?$vector@M$02@@00HAIAV?$vector@I$01@@@Z"(%struct.BoundingBox* noalias nocapture sret, <3 x float>, <3 x float>, <3 x float>, i32, <2 x i32>* noalias nocapture dereferenceable(8)) #5 {
  %7 = extractelement <3 x float> %1, i64 0
  %8 = extractelement <3 x float> %2, i64 0
  %FMin = call float @dx.op.binary.f32(i32 36, float %7, float %8)
  %9 = extractelement <3 x float> %1, i64 1
  %10 = extractelement <3 x float> %2, i64 1
  %FMin6 = call float @dx.op.binary.f32(i32 36, float %9, float %10)
  %11 = extractelement <3 x float> %1, i64 2
  %12 = extractelement <3 x float> %2, i64 2
  %FMin7 = call float @dx.op.binary.f32(i32 36, float %11, float %12)
  %13 = extractelement <3 x float> %3, i64 0
  %FMin8 = call float @dx.op.binary.f32(i32 36, float %FMin, float %13)
  %14 = extractelement <3 x float> %3, i64 1
  %FMin9 = call float @dx.op.binary.f32(i32 36, float %FMin6, float %14)
  %15 = extractelement <3 x float> %3, i64 2
  %FMin10 = call float @dx.op.binary.f32(i32 36, float %FMin7, float %15)
  %FMax = call float @dx.op.binary.f32(i32 35, float %7, float %8)
  %FMax1 = call float @dx.op.binary.f32(i32 35, float %9, float %10)
  %FMax2 = call float @dx.op.binary.f32(i32 35, float %11, float %12)
  %FMax3 = call float @dx.op.binary.f32(i32 35, float %FMax, float %13)
  %FMax4 = call float @dx.op.binary.f32(i32 35, float %FMax1, float %14)
  %FMax5 = call float @dx.op.binary.f32(i32 35, float %FMax2, float %15)
  %.i0 = fadd fast float %FMax3, 0xBF50624DE0000000
  %.i1 = fadd fast float %FMax4, 0xBF50624DE0000000
  %.i2 = fadd fast float %FMax5, 0xBF50624DE0000000
  %FMin11 = call float @dx.op.binary.f32(i32 36, float %FMin8, float %.i0)
  %FMin12 = call float @dx.op.binary.f32(i32 36, float %FMin9, float %.i1)
  %FMin13 = call float @dx.op.binary.f32(i32 36, float %FMin10, float %.i2)
  %.i014 = fadd fast float %FMin11, %FMax3
  %.i115 = fadd fast float %FMin12, %FMax4
  %.i216 = fadd fast float %FMin13, %FMax5
  %.i017 = fmul fast float %.i014, 5.000000e-01
  %.i118 = fmul fast float %.i115, 5.000000e-01
  %.i219 = fmul fast float %.i216, 5.000000e-01
  %.upto023 = insertelement <3 x float> undef, float %.i017, i32 0
  %.upto124 = insertelement <3 x float> %.upto023, float %.i118, i32 1
  %16 = insertelement <3 x float> %.upto124, float %.i219, i32 2
  %.i020 = fsub fast float %FMax3, %.i017
  %.i121 = fsub fast float %FMax4, %.i118
  %.i222 = fsub fast float %FMax5, %.i219
  %.upto025 = insertelement <3 x float> undef, float %.i020, i32 0
  %.upto126 = insertelement <3 x float> %.upto025, float %.i121, i32 1
  %17 = insertelement <3 x float> %.upto126, float %.i222, i32 2
  %18 = or i32 %4, -2147483648
  %19 = insertelement <2 x i32> undef, i32 %18, i32 0
  %20 = insertelement <2 x i32> %19, i32 1, i32 1
  %21 = getelementptr inbounds %struct.BoundingBox, %struct.BoundingBox* %0, i32 0, i32 0
  store <3 x float> %16, <3 x float>* %21, align 4
  %22 = getelementptr inbounds %struct.BoundingBox, %struct.BoundingBox* %0, i32 0, i32 1
  store <3 x float> %17, <3 x float>* %22, align 4
  store <2 x i32> %20, <2 x i32>* %5, align 4
  ret void
}

; Function Attrs: alwaysinline nounwind
define void @"\01?GetBoxFromChildBoxes@@YA?AUBoundingBox@@U1@H0HAIAV?$vector@I$01@@@Z"(%struct.BoundingBox* noalias nocapture sret, %struct.BoundingBox* nocapture readonly, i32, %struct.BoundingBox* nocapture readonly, i32, <2 x i32>* noalias nocapture dereferenceable(8)) #5 {
  %7 = getelementptr inbounds %struct.BoundingBox, %struct.BoundingBox* %3, i32 0, i32 0
  %8 = load <3 x float>, <3 x float>* %7, align 4, !tbaa !218
  %.i05 = extractelement <3 x float> %8, i32 0
  %.i17 = extractelement <3 x float> %8, i32 1
  %.i29 = extractelement <3 x float> %8, i32 2
  %9 = getelementptr inbounds %struct.BoundingBox, %struct.BoundingBox* %3, i32 0, i32 1
  %10 = load <3 x float>, <3 x float>* %9, align 4, !tbaa !218
  %.i0 = extractelement <3 x float> %10, i32 0
  %.i06 = fsub fast float %.i05, %.i0
  %.i1 = extractelement <3 x float> %10, i32 1
  %.i18 = fsub fast float %.i17, %.i1
  %.i2 = extractelement <3 x float> %10, i32 2
  %.i210 = fsub fast float %.i29, %.i2
  %11 = getelementptr inbounds %struct.BoundingBox, %struct.BoundingBox* %1, i32 0, i32 0
  %12 = load <3 x float>, <3 x float>* %11, align 4, !tbaa !218
  %.i012 = extractelement <3 x float> %12, i32 0
  %.i115 = extractelement <3 x float> %12, i32 1
  %.i218 = extractelement <3 x float> %12, i32 2
  %13 = getelementptr inbounds %struct.BoundingBox, %struct.BoundingBox* %1, i32 0, i32 1
  %14 = load <3 x float>, <3 x float>* %13, align 4, !tbaa !218
  %.i011 = extractelement <3 x float> %14, i32 0
  %.i013 = fsub fast float %.i012, %.i011
  %.i114 = extractelement <3 x float> %14, i32 1
  %.i116 = fsub fast float %.i115, %.i114
  %.i217 = extractelement <3 x float> %14, i32 2
  %.i219 = fsub fast float %.i218, %.i217
  %FMin = call float @dx.op.binary.f32(i32 36, float %.i013, float %.i06)
  %FMin3 = call float @dx.op.binary.f32(i32 36, float %.i116, float %.i18)
  %FMin4 = call float @dx.op.binary.f32(i32 36, float %.i219, float %.i210)
  %.i022 = fadd fast float %.i0, %.i05
  %.i125 = fadd fast float %.i1, %.i17
  %.i228 = fadd fast float %.i2, %.i29
  %.i031 = fadd fast float %.i011, %.i012
  %.i134 = fadd fast float %.i114, %.i115
  %.i237 = fadd fast float %.i217, %.i218
  %FMax = call float @dx.op.binary.f32(i32 35, float %.i031, float %.i022)
  %FMax1 = call float @dx.op.binary.f32(i32 35, float %.i134, float %.i125)
  %FMax2 = call float @dx.op.binary.f32(i32 35, float %.i237, float %.i228)
  %.i038 = fadd fast float %FMax, %FMin
  %.i139 = fadd fast float %FMax1, %FMin3
  %.i240 = fadd fast float %FMax2, %FMin4
  %.i041 = fmul fast float %.i038, 5.000000e-01
  %.i142 = fmul fast float %.i139, 5.000000e-01
  %.i243 = fmul fast float %.i240, 5.000000e-01
  %.upto053 = insertelement <3 x float> undef, float %.i041, i32 0
  %.upto154 = insertelement <3 x float> %.upto053, float %.i142, i32 1
  %15 = insertelement <3 x float> %.upto154, float %.i243, i32 2
  %.i044 = fsub fast float %FMax, %.i041
  %.i145 = fsub fast float %FMax1, %.i142
  %.i246 = fsub fast float %FMax2, %.i243
  %.upto055 = insertelement <3 x float> undef, float %.i044, i32 0
  %.upto156 = insertelement <3 x float> %.upto055, float %.i145, i32 1
  %16 = insertelement <3 x float> %.upto156, float %.i246, i32 2
  %17 = and i32 %2, 16777215
  %18 = insertelement <2 x i32> undef, i32 %17, i32 0
  %19 = insertelement <2 x i32> %18, i32 %4, i32 1
  store <2 x i32> %19, <2 x i32>* %5, align 4, !tbaa !218
  %20 = getelementptr inbounds %struct.BoundingBox, %struct.BoundingBox* %0, i32 0, i32 0
  store <3 x float> %15, <3 x float>* %20, align 4
  %21 = getelementptr inbounds %struct.BoundingBox, %struct.BoundingBox* %0, i32 0, i32 1
  store <3 x float> %16, <3 x float>* %21, align 4
  ret void
}

; Function Attrs: alwaysinline nounwind readonly
define i32 @"\01?GetOffsetToBoxes@@YAHURWByteAddressBufferPointer@@@Z"(%struct.RWByteAddressBufferPointer* nocapture readonly) #4 {
  %2 = getelementptr inbounds %struct.RWByteAddressBufferPointer, %struct.RWByteAddressBufferPointer* %0, i32 0, i32 1
  %3 = load i32, i32* %2, align 4, !tbaa !221
  %4 = add i32 %3, 16
  ret i32 %4
}

; Function Attrs: alwaysinline nounwind readonly
define i32 @"\01?GetOffsetToTriangleMetadata@@YAHURWByteAddressBufferPointer@@@Z"(%struct.RWByteAddressBufferPointer* nocapture readonly) #4 {
  %2 = getelementptr inbounds %struct.RWByteAddressBufferPointer, %struct.RWByteAddressBufferPointer* %0, i32 0, i32 1
  %3 = load i32, i32* %2, align 4, !tbaa !221
  %4 = add i32 %3, 8
  %5 = getelementptr inbounds %struct.RWByteAddressBufferPointer, %struct.RWByteAddressBufferPointer* %0, i32 0, i32 0
  %6 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %5, align 4
  %7 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %6)
  %8 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %7, i32 %4, i32 undef)
  %9 = extractvalue %dx.types.ResRet.i32 %8, 0
  %10 = add i32 %9, %3
  ret i32 %10
}

; Function Attrs: alwaysinline nounwind readnone
define void @"\01?Log@@YAXV?$vector@I$03@@@Z"(<4 x i32>) #6 {
  ret void
}

; Function Attrs: alwaysinline nounwind readonly
define <2 x float> @"\01?GetUVAttribute@@YA?AV?$vector@M$01@@I@Z"(i32) #4 {
  %2 = load %struct.ByteAddressBuffer, %struct.ByteAddressBuffer* @"\01?g_attributes@@3UByteAddressBuffer@@A", align 4
  %3 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.ByteAddressBuffer(i32 160, %struct.ByteAddressBuffer %2)
  %4 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %3, i32 %0, i32 undef)
  %5 = extractvalue %dx.types.ResRet.i32 %4, 0
  %6 = extractvalue %dx.types.ResRet.i32 %4, 1
  %.i0 = bitcast i32 %5 to float
  %.i1 = bitcast i32 %6 to float
  %.upto0 = insertelement <2 x float> undef, float %.i0, i32 0
  %7 = insertelement <2 x float> %.upto0, float %.i1, i32 1
  ret <2 x float> %7
}

; Function Attrs: alwaysinline nounwind readonly
define i32 @"\01?GetInstanceFlags@@YAIURaytracingInstanceDesc@@@Z"(%struct.RaytracingInstanceDesc* nocapture readonly) #4 {
  %2 = getelementptr inbounds %struct.RaytracingInstanceDesc, %struct.RaytracingInstanceDesc* %0, i32 0, i32 2
  %3 = load i32, i32* %2, align 4, !tbaa !221
  %4 = lshr i32 %3, 24
  ret i32 %4
}

; Function Attrs: alwaysinline nounwind readnone
define void @"\01?LogInt@@YAXH@Z"(i32) #6 {
  ret void
}

; Function Attrs: alwaysinline nounwind readnone
define <3 x float> @"\01?ApplyLightCommon@@YA?AV?$vector@M$02@@V1@0MM0000@Z"(<3 x float>, <3 x float>, float, float, <3 x float>, <3 x float>, <3 x float>, <3 x float>) #6 {
  %lightColor.i0 = extractelement <3 x float> %7, i32 0
  %lightColor.i1 = extractelement <3 x float> %7, i32 1
  %lightColor.i2 = extractelement <3 x float> %7, i32 2
  %diffuseColor.i0 = extractelement <3 x float> %0, i32 0
  %diffuseColor.i1 = extractelement <3 x float> %0, i32 1
  %diffuseColor.i2 = extractelement <3 x float> %0, i32 2
  %specularColor.i0 = extractelement <3 x float> %1, i32 0
  %specularColor.i1 = extractelement <3 x float> %1, i32 1
  %specularColor.i2 = extractelement <3 x float> %1, i32 2
  %viewDir.i0 = extractelement <3 x float> %5, i32 0
  %lightDir.i0 = extractelement <3 x float> %6, i32 0
  %viewDir.i1 = extractelement <3 x float> %5, i32 1
  %lightDir.i1 = extractelement <3 x float> %6, i32 1
  %viewDir.i2 = extractelement <3 x float> %5, i32 2
  %lightDir.i2 = extractelement <3 x float> %6, i32 2
  %.i0 = fsub fast float %lightDir.i0, %viewDir.i0
  %.i1 = fsub fast float %lightDir.i1, %viewDir.i1
  %.i2 = fsub fast float %lightDir.i2, %viewDir.i2
  %9 = fmul fast float %.i0, %.i0
  %10 = fmul fast float %.i1, %.i1
  %11 = fadd fast float %9, %10
  %12 = fmul fast float %.i2, %.i2
  %13 = fadd fast float %11, %12
  %Sqrt = call float @dx.op.unary.f32(i32 24, float %13)
  %.i05 = fdiv fast float %.i0, %Sqrt
  %.i16 = fdiv fast float %.i1, %Sqrt
  %.i27 = fdiv fast float %.i2, %Sqrt
  %14 = extractelement <3 x float> %4, i64 0
  %15 = extractelement <3 x float> %4, i64 1
  %16 = extractelement <3 x float> %4, i64 2
  %17 = call float @dx.op.dot3.f32(i32 55, float %.i05, float %.i16, float %.i27, float %14, float %15, float %16)
  %Saturate1 = call float @dx.op.unary.f32(i32 7, float %17)
  %18 = extractelement <3 x float> %6, i64 0
  %19 = extractelement <3 x float> %6, i64 1
  %20 = extractelement <3 x float> %6, i64 2
  %21 = call float @dx.op.dot3.f32(i32 55, float %18, float %19, float %20, float %.i05, float %.i16, float %.i27)
  %Saturate = call float @dx.op.unary.f32(i32 7, float %21)
  %22 = fsub fast float 1.000000e+00, %Saturate
  %Log = call float @dx.op.unary.f32(i32 23, float %22)
  %23 = fmul fast float %Log, 5.000000e+00
  %Exp = call float @dx.op.unary.f32(i32 21, float %23)
  %.i08 = fsub fast float 1.000000e+00, %specularColor.i0
  %.i19 = fsub fast float 1.000000e+00, %specularColor.i1
  %.i210 = fsub fast float 1.000000e+00, %specularColor.i2
  %.i011 = fmul fast float %Exp, %.i08
  %.i112 = fmul fast float %Exp, %.i19
  %.i213 = fmul fast float %Exp, %.i210
  %.i014 = fadd fast float %.i011, %specularColor.i0
  %.i115 = fadd fast float %.i112, %specularColor.i1
  %.i216 = fadd fast float %.i213, %specularColor.i2
  %24 = fmul fast float %Exp, %diffuseColor.i0
  %25 = fmul fast float %Exp, %diffuseColor.i1
  %26 = fmul fast float %Exp, %diffuseColor.i2
  %.i023 = fsub fast float %diffuseColor.i0, %24
  %.i124 = fsub fast float %diffuseColor.i1, %25
  %.i225 = fsub fast float %diffuseColor.i2, %26
  %Log3 = call float @dx.op.unary.f32(i32 23, float %Saturate1)
  %27 = fmul fast float %Log3, %3
  %Exp4 = call float @dx.op.unary.f32(i32 21, float %27)
  %28 = fadd fast float %3, 2.000000e+00
  %29 = fmul fast float %2, 1.250000e-01
  %30 = fmul fast float %29, %28
  %31 = fmul fast float %30, %Exp4
  %32 = call float @dx.op.dot3.f32(i32 55, float %14, float %15, float %16, float %18, float %19, float %20)
  %Saturate2 = call float @dx.op.unary.f32(i32 7, float %32)
  %.i026 = fmul fast float %Saturate2, %lightColor.i0
  %.i127 = fmul fast float %Saturate2, %lightColor.i1
  %.i228 = fmul fast float %Saturate2, %lightColor.i2
  %.i029 = fmul fast float %31, %.i014
  %.i130 = fmul fast float %31, %.i115
  %.i231 = fmul fast float %31, %.i216
  %.i032 = fadd fast float %.i023, %.i029
  %.i133 = fadd fast float %.i124, %.i130
  %.i234 = fadd fast float %.i225, %.i231
  %.i035 = fmul fast float %.i026, %.i032
  %.i136 = fmul fast float %.i127, %.i133
  %.i237 = fmul fast float %.i228, %.i234
  %.upto040 = insertelement <3 x float> undef, float %.i035, i32 0
  %.upto141 = insertelement <3 x float> %.upto040, float %.i136, i32 1
  %33 = insertelement <3 x float> %.upto141, float %.i237, i32 2
  ret <3 x float> %33
}

; Function Attrs: alwaysinline nounwind readnone
define <3 x float> @"\01?Swizzle@@YA?AV?$vector@M$02@@V1@V?$vector@H$02@@@Z"(<3 x float>, <3 x i32>) #6 {
  %3 = alloca [3 x float], align 4
  %4 = extractelement <3 x float> %0, i64 0
  %5 = getelementptr inbounds [3 x float], [3 x float]* %3, i32 0, i32 0
  store float %4, float* %5, align 4
  %6 = extractelement <3 x float> %0, i64 1
  %7 = getelementptr inbounds [3 x float], [3 x float]* %3, i32 0, i32 1
  store float %6, float* %7, align 4
  %8 = extractelement <3 x float> %0, i64 2
  %9 = getelementptr inbounds [3 x float], [3 x float]* %3, i32 0, i32 2
  store float %8, float* %9, align 4
  %10 = extractelement <3 x i32> %1, i32 0
  %11 = getelementptr [3 x float], [3 x float]* %3, i32 0, i32 %10
  %12 = load float, float* %11, align 4, !tbaa !231
  %13 = extractelement <3 x i32> %1, i32 1
  %14 = getelementptr [3 x float], [3 x float]* %3, i32 0, i32 %13
  %15 = load float, float* %14, align 4, !tbaa !231
  %16 = extractelement <3 x i32> %1, i32 2
  %17 = getelementptr [3 x float], [3 x float]* %3, i32 0, i32 %16
  %18 = load float, float* %17, align 4, !tbaa !231
  %19 = insertelement <3 x float> undef, float %12, i64 0
  %20 = insertelement <3 x float> %19, float %15, i64 1
  %21 = insertelement <3 x float> %20, float %18, i64 2
  ret <3 x float> %21
}

; Function Attrs: alwaysinline nounwind
define i32 @"\01?Traverse@@YAHIIII@Z"(i32, i32, i32, i32) #5 {
  %5 = load %struct.ByteAddressBuffer, %struct.ByteAddressBuffer* @"\01?MissShaderTable@@3UByteAddressBuffer@@A", align 4
  %6 = load %struct.ByteAddressBuffer, %struct.ByteAddressBuffer* @"\01?HitGroupShaderTable@@3UByteAddressBuffer@@A", align 4
  %7 = load %AccelerationStructureList, %AccelerationStructureList* @AccelerationStructureList, align 4
  %8 = load %Constants, %Constants* @Constants, align 4
  %AccelerationStructureList = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.AccelerationStructureList(i32 160, %AccelerationStructureList %7)
  %Constants295 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.Constants(i32 160, %Constants %8)
  %9 = alloca [3 x float], align 4
  %10 = alloca [3 x float], align 4
  %11 = alloca [3 x float], align 4
  %12 = alloca [3 x float], align 4
  %13 = alloca [3 x float], align 4
  %nodesToProcess.i = alloca [2 x i32], align 4
  %attr.i = alloca %struct.BuiltInTriangleIntersectionAttributes, align 4
  %14 = call i32 @"\01?Fallback_GroupIndex@@YAIXZ"() #3
  %WorldRayDirection251 = call float @dx.op.worldRayDirection.f32(i32 148, i8 0)
  %WorldRayDirection252 = call float @dx.op.worldRayDirection.f32(i32 148, i8 1)
  %WorldRayDirection253 = call float @dx.op.worldRayDirection.f32(i32 148, i8 2)
  %WorldRayOrigin248 = call float @dx.op.worldRayOrigin.f32(i32 147, i8 0)
  %WorldRayOrigin249 = call float @dx.op.worldRayOrigin.f32(i32 147, i8 1)
  %WorldRayOrigin250 = call float @dx.op.worldRayOrigin.f32(i32 147, i8 2)
  %15 = getelementptr inbounds [3 x float], [3 x float]* %13, i32 0, i32 0
  store float %WorldRayDirection251, float* %15, align 4
  %16 = getelementptr inbounds [3 x float], [3 x float]* %13, i32 0, i32 1
  store float %WorldRayDirection252, float* %16, align 4
  %17 = getelementptr inbounds [3 x float], [3 x float]* %13, i32 0, i32 2
  store float %WorldRayDirection253, float* %17, align 4
  %.i0 = fdiv fast float 1.000000e+00, %WorldRayDirection251
  %.i1 = fdiv fast float 1.000000e+00, %WorldRayDirection252
  %.i2 = fdiv fast float 1.000000e+00, %WorldRayDirection253
  %.i0296 = fmul fast float %.i0, %WorldRayOrigin248
  %.i1297 = fmul fast float %.i1, %WorldRayOrigin249
  %.i2298 = fmul fast float %.i2, %WorldRayOrigin250
  %FAbs241 = call float @dx.op.unary.f32(i32 6, float %WorldRayDirection251)
  %FAbs242 = call float @dx.op.unary.f32(i32 6, float %WorldRayDirection252)
  %FAbs243 = call float @dx.op.unary.f32(i32 6, float %WorldRayDirection253)
  %18 = fcmp fast ogt float %FAbs241, %FAbs242
  %19 = fcmp fast ogt float %FAbs241, %FAbs243
  %20 = and i1 %18, %19
  br i1 %20, label %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i", label %21

; <label>:21                                      ; preds = %4
  %22 = fcmp fast ogt float %FAbs242, %FAbs243
  br i1 %22, label %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i", label %23

; <label>:23                                      ; preds = %21
  br label %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i"

"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i": ; preds = %23, %21, %4
  %.0171 = phi i32 [ 2, %23 ], [ 0, %4 ], [ 1, %21 ]
  %24 = add nuw nsw i32 %.0171, 1
  %25 = urem i32 %24, 3
  %26 = add nuw nsw i32 %.0171, 2
  %27 = urem i32 %26, 3
  %28 = getelementptr [3 x float], [3 x float]* %13, i32 0, i32 %.0171
  %29 = load float, float* %28, align 4, !tbaa !231, !noalias !289
  %30 = fcmp fast olt float %29, 0.000000e+00
  %worldRayData.i.5.0.i0 = select i1 %30, i32 %27, i32 %25
  %worldRayData.i.5.0.i1 = select i1 %30, i32 %25, i32 %27
  %31 = getelementptr [3 x float], [3 x float]* %13, i32 0, i32 %worldRayData.i.5.0.i0
  %32 = load float, float* %31, align 4, !tbaa !231, !noalias !289
  %33 = fdiv float %32, %29
  %34 = getelementptr [3 x float], [3 x float]* %13, i32 0, i32 %worldRayData.i.5.0.i1
  %35 = load float, float* %34, align 4, !tbaa !231, !noalias !289
  %36 = fdiv float %35, %29
  %37 = fdiv float 1.000000e+00, %29
  %38 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %AccelerationStructureList, i32 0)
  %39 = extractvalue %dx.types.CBufRet.i32 %38, 0
  %40 = extractvalue %dx.types.CBufRet.i32 %38, 1
  %41 = getelementptr inbounds [2 x i32], [2 x i32]* %nodesToProcess.i, i32 0, i32 0
  store i32 0, i32* %41, align 4, !tbaa !221
  %42 = getelementptr inbounds [0 x %struct.RWByteAddressBuffer], [0 x %struct.RWByteAddressBuffer]* @"\01?DescriptorHeapBufferTable@@3PAURWByteAddressBuffer@@A", i32 0, i32 %40, !dx.nonuniform !240
  %43 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %42, align 4, !noalias !125
  %44 = add i32 %39, 4
  %45 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %43)
  %46 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %45, i32 %44, i32 undef)
  %47 = extractvalue %dx.types.ResRet.i32 %46, 0
  %48 = add i32 %47, %39
  %49 = add i32 %39, 16
  %50 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %45, i32 %49, i32 undef)
  %51 = extractvalue %dx.types.ResRet.i32 %50, 0
  %52 = extractvalue %dx.types.ResRet.i32 %50, 1
  %53 = extractvalue %dx.types.ResRet.i32 %50, 2
  %54 = add i32 %39, 32
  %55 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %45, i32 %54, i32 undef)
  %56 = extractvalue %dx.types.ResRet.i32 %55, 0
  %57 = extractvalue %dx.types.ResRet.i32 %55, 1
  %58 = extractvalue %dx.types.ResRet.i32 %55, 2
  %59 = bitcast i32 %51 to float
  %60 = bitcast i32 %52 to float
  %61 = bitcast i32 %53 to float
  %62 = bitcast i32 %56 to float
  %63 = bitcast i32 %57 to float
  %64 = bitcast i32 %58 to float
  %RayTCurrent220 = call float @dx.op.rayTCurrent.f32(i32 154)
  %65 = fsub fast float %59, %WorldRayOrigin248
  %66 = fmul fast float %65, %.i0
  %67 = fsub fast float %60, %WorldRayOrigin249
  %68 = fmul fast float %67, %.i1
  %69 = fsub fast float %61, %WorldRayOrigin250
  %70 = fmul fast float %69, %.i2
  %FAbs238 = call float @dx.op.unary.f32(i32 6, float %.i0)
  %FAbs239 = call float @dx.op.unary.f32(i32 6, float %.i1)
  %FAbs240 = call float @dx.op.unary.f32(i32 6, float %.i2)
  %.i0305 = fmul fast float %62, %FAbs238
  %.i1306 = fmul fast float %FAbs239, %63
  %.i2307 = fmul fast float %FAbs240, %64
  %.i0308 = fadd fast float %.i0305, %66
  %.i1309 = fadd fast float %.i1306, %68
  %.i2310 = fadd fast float %.i2307, %70
  %.i0314 = fsub fast float %66, %.i0305
  %.i1315 = fsub fast float %68, %.i1306
  %.i2316 = fsub fast float %70, %.i2307
  %FMax277 = call float @dx.op.binary.f32(i32 35, float %.i0314, float %.i1315)
  %FMax276 = call float @dx.op.binary.f32(i32 35, float %FMax277, float %.i2316)
  %FMin275 = call float @dx.op.binary.f32(i32 36, float %.i0308, float %.i1309)
  %FMin274 = call float @dx.op.binary.f32(i32 36, float %FMin275, float %.i2310)
  %FMax273 = call float @dx.op.binary.f32(i32 35, float %FMax276, float 0.000000e+00)
  %FMin272 = call float @dx.op.binary.f32(i32 36, float %FMin274, float %RayTCurrent220)
  %71 = fcmp fast olt float %FMax273, %FMin272
  br i1 %71, label %.lr.ph.preheader.critedge, label %87

.lr.ph.preheader.critedge:                        ; preds = %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i"
  %72 = getelementptr [2048 x i32], [2048 x i32] addrspace(3)* @"\01?stack@@3PAIA", i32 0, i32 %14
  store i32 0, i32 addrspace(3)* %72, align 4, !tbaa !221, !noalias !292
  store i32 1, i32* %41, align 4, !tbaa !221
  call void @"\01?Fallback_SetInstanceIndex@@YAXI@Z"(i32 -1) #3
  %73 = getelementptr inbounds [3 x float], [3 x float]* %12, i32 0, i32 0
  %74 = getelementptr inbounds [3 x float], [3 x float]* %12, i32 0, i32 1
  %75 = getelementptr inbounds [3 x float], [3 x float]* %12, i32 0, i32 2
  %76 = getelementptr inbounds [2 x i32], [2 x i32]* %nodesToProcess.i, i32 0, i32 1
  %77 = getelementptr inbounds [3 x float], [3 x float]* %11, i32 0, i32 0
  %78 = getelementptr inbounds [3 x float], [3 x float]* %11, i32 0, i32 1
  %79 = getelementptr inbounds [3 x float], [3 x float]* %11, i32 0, i32 2
  %80 = getelementptr inbounds [3 x float], [3 x float]* %9, i32 0, i32 0
  %81 = getelementptr inbounds [3 x float], [3 x float]* %9, i32 0, i32 1
  %82 = getelementptr inbounds [3 x float], [3 x float]* %9, i32 0, i32 2
  %83 = getelementptr inbounds [3 x float], [3 x float]* %10, i32 0, i32 0
  %84 = getelementptr inbounds [3 x float], [3 x float]* %10, i32 0, i32 1
  %85 = getelementptr inbounds [3 x float], [3 x float]* %10, i32 0, i32 2
  %86 = getelementptr inbounds %struct.BuiltInTriangleIntersectionAttributes, %struct.BuiltInTriangleIntersectionAttributes* %attr.i, i32 0, i32 0
  br label %.lr.ph

; <label>:87                                      ; preds = %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i"
  call void @"\01?Fallback_SetInstanceIndex@@YAXI@Z"(i32 -1) #3
  br label %._crit_edge

.lr.ph:                                           ; preds = %430, %.lr.ph.preheader.critedge
  %resultTriId.i.0188 = phi i32 [ %resultTriId.i.2, %430 ], [ undef, %.lr.ph.preheader.critedge ]
  %resultBary.i.0187.i0 = phi float [ %resultBary.i.2.i0, %430 ], [ undef, %.lr.ph.preheader.critedge ]
  %resultBary.i.0187.i1 = phi float [ %resultBary.i.2.i1, %430 ], [ undef, %.lr.ph.preheader.critedge ]
  %stackPointer.i.1185 = phi i32 [ %stackPointer.i.3, %430 ], [ 1, %.lr.ph.preheader.critedge ]
  %instId.i.0184 = phi i32 [ %instId.i.2, %430 ], [ 0, %.lr.ph.preheader.critedge ]
  %instOffset.i.0183 = phi i32 [ %instOffset.i.2, %430 ], [ 0, %.lr.ph.preheader.critedge ]
  %instFlags.i.0182 = phi i32 [ %instFlags.i.2, %430 ], [ 0, %.lr.ph.preheader.critedge ]
  %instIdx.i.0181 = phi i32 [ %instIdx.i.2, %430 ], [ 0, %.lr.ph.preheader.critedge ]
  %currentGpuVA.i.0180.i0 = phi i32 [ %433, %430 ], [ %39, %.lr.ph.preheader.critedge ]
  %currentGpuVA.i.0180.i1 = phi i32 [ %434, %430 ], [ %40, %.lr.ph.preheader.critedge ]
  %currentBVHIndex.i.0179 = phi i32 [ %431, %430 ], [ 0, %.lr.ph.preheader.critedge ]
  %CurrentWorldToObject.i168.0178.i0 = phi float [ %CurrentWorldToObject.i168.2.i0, %430 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentWorldToObject.i168.0178.i1 = phi float [ %CurrentWorldToObject.i168.2.i1, %430 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentWorldToObject.i168.0178.i2 = phi float [ %CurrentWorldToObject.i168.2.i2, %430 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentWorldToObject.i168.0178.i3 = phi float [ %CurrentWorldToObject.i168.2.i3, %430 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentWorldToObject.i168.0178.i4 = phi float [ %CurrentWorldToObject.i168.2.i4, %430 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentWorldToObject.i168.0178.i5 = phi float [ %CurrentWorldToObject.i168.2.i5, %430 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentWorldToObject.i168.0178.i6 = phi float [ %CurrentWorldToObject.i168.2.i6, %430 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentWorldToObject.i168.0178.i7 = phi float [ %CurrentWorldToObject.i168.2.i7, %430 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentWorldToObject.i168.0178.i8 = phi float [ %CurrentWorldToObject.i168.2.i8, %430 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentWorldToObject.i168.0178.i9 = phi float [ %CurrentWorldToObject.i168.2.i9, %430 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentWorldToObject.i168.0178.i10 = phi float [ %CurrentWorldToObject.i168.2.i10, %430 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentWorldToObject.i168.0178.i11 = phi float [ %CurrentWorldToObject.i168.2.i11, %430 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentObjectToWorld.i167.0177.i0 = phi float [ %CurrentObjectToWorld.i167.2.i0, %430 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentObjectToWorld.i167.0177.i1 = phi float [ %CurrentObjectToWorld.i167.2.i1, %430 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentObjectToWorld.i167.0177.i2 = phi float [ %CurrentObjectToWorld.i167.2.i2, %430 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentObjectToWorld.i167.0177.i3 = phi float [ %CurrentObjectToWorld.i167.2.i3, %430 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentObjectToWorld.i167.0177.i4 = phi float [ %CurrentObjectToWorld.i167.2.i4, %430 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentObjectToWorld.i167.0177.i5 = phi float [ %CurrentObjectToWorld.i167.2.i5, %430 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentObjectToWorld.i167.0177.i6 = phi float [ %CurrentObjectToWorld.i167.2.i6, %430 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentObjectToWorld.i167.0177.i7 = phi float [ %CurrentObjectToWorld.i167.2.i7, %430 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentObjectToWorld.i167.0177.i8 = phi float [ %CurrentObjectToWorld.i167.2.i8, %430 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentObjectToWorld.i167.0177.i9 = phi float [ %CurrentObjectToWorld.i167.2.i9, %430 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentObjectToWorld.i167.0177.i10 = phi float [ %CurrentObjectToWorld.i167.2.i10, %430 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentObjectToWorld.i167.0177.i11 = phi float [ %CurrentObjectToWorld.i167.2.i11, %430 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %.0174176.i0 = phi float [ %.3.i0, %430 ], [ undef, %.lr.ph.preheader.critedge ]
  %.0174176.i1 = phi float [ %.3.i1, %430 ], [ undef, %.lr.ph.preheader.critedge ]
  %.phi.trans.insert = getelementptr inbounds [2 x i32], [2 x i32]* %nodesToProcess.i, i32 0, i32 %currentBVHIndex.i.0179
  %.pre = load i32, i32* %.phi.trans.insert, align 4, !tbaa !221
  br label %88

; <label>:88                                      ; preds = %426, %.lr.ph
  %89 = phi i32 [ %.pre, %.lr.ph ], [ %428, %426 ]
  %.1.i0 = phi float [ %.0174176.i0, %.lr.ph ], [ %.3.i0, %426 ]
  %.1.i1 = phi float [ %.0174176.i1, %.lr.ph ], [ %.3.i1, %426 ]
  %currentRayData.i.0.1.i0 = phi float [ %WorldRayOrigin248, %.lr.ph ], [ %currentRayData.i.0.2.i0, %426 ]
  %currentRayData.i.0.1.i1 = phi float [ %WorldRayOrigin249, %.lr.ph ], [ %currentRayData.i.0.2.i1, %426 ]
  %currentRayData.i.0.1.i2 = phi float [ %WorldRayOrigin250, %.lr.ph ], [ %currentRayData.i.0.2.i2, %426 ]
  %currentRayData.i.1.1.i0 = phi float [ %WorldRayDirection251, %.lr.ph ], [ %currentRayData.i.1.2.i0, %426 ]
  %currentRayData.i.1.1.i1 = phi float [ %WorldRayDirection252, %.lr.ph ], [ %currentRayData.i.1.2.i1, %426 ]
  %currentRayData.i.1.1.i2 = phi float [ %WorldRayDirection253, %.lr.ph ], [ %currentRayData.i.1.2.i2, %426 ]
  %currentRayData.i.2.1.i0 = phi float [ %.i0, %.lr.ph ], [ %currentRayData.i.2.2.i0, %426 ]
  %currentRayData.i.2.1.i1 = phi float [ %.i1, %.lr.ph ], [ %currentRayData.i.2.2.i1, %426 ]
  %currentRayData.i.2.1.i2 = phi float [ %.i2, %.lr.ph ], [ %currentRayData.i.2.2.i2, %426 ]
  %currentRayData.i.3.1.i0 = phi float [ %.i0296, %.lr.ph ], [ %currentRayData.i.3.2.i0, %426 ]
  %currentRayData.i.3.1.i1 = phi float [ %.i1297, %.lr.ph ], [ %currentRayData.i.3.2.i1, %426 ]
  %currentRayData.i.3.1.i2 = phi float [ %.i2298, %.lr.ph ], [ %currentRayData.i.3.2.i2, %426 ]
  %currentRayData.i.4.1.i0 = phi float [ %33, %.lr.ph ], [ %currentRayData.i.4.2.i0, %426 ]
  %currentRayData.i.4.1.i1 = phi float [ %36, %.lr.ph ], [ %currentRayData.i.4.2.i1, %426 ]
  %currentRayData.i.4.1.i2 = phi float [ %37, %.lr.ph ], [ %currentRayData.i.4.2.i2, %426 ]
  %currentRayData.i.5.1.i0 = phi i32 [ %worldRayData.i.5.0.i0, %.lr.ph ], [ %currentRayData.i.5.2.i0, %426 ]
  %currentRayData.i.5.1.i1 = phi i32 [ %worldRayData.i.5.0.i1, %.lr.ph ], [ %currentRayData.i.5.2.i1, %426 ]
  %currentRayData.i.5.1.i2 = phi i32 [ %.0171, %.lr.ph ], [ %currentRayData.i.5.2.i2, %426 ]
  %CurrentObjectToWorld.i167.1.i0 = phi float [ %CurrentObjectToWorld.i167.0177.i0, %.lr.ph ], [ %CurrentObjectToWorld.i167.2.i0, %426 ]
  %CurrentObjectToWorld.i167.1.i1 = phi float [ %CurrentObjectToWorld.i167.0177.i1, %.lr.ph ], [ %CurrentObjectToWorld.i167.2.i1, %426 ]
  %CurrentObjectToWorld.i167.1.i2 = phi float [ %CurrentObjectToWorld.i167.0177.i2, %.lr.ph ], [ %CurrentObjectToWorld.i167.2.i2, %426 ]
  %CurrentObjectToWorld.i167.1.i3 = phi float [ %CurrentObjectToWorld.i167.0177.i3, %.lr.ph ], [ %CurrentObjectToWorld.i167.2.i3, %426 ]
  %CurrentObjectToWorld.i167.1.i4 = phi float [ %CurrentObjectToWorld.i167.0177.i4, %.lr.ph ], [ %CurrentObjectToWorld.i167.2.i4, %426 ]
  %CurrentObjectToWorld.i167.1.i5 = phi float [ %CurrentObjectToWorld.i167.0177.i5, %.lr.ph ], [ %CurrentObjectToWorld.i167.2.i5, %426 ]
  %CurrentObjectToWorld.i167.1.i6 = phi float [ %CurrentObjectToWorld.i167.0177.i6, %.lr.ph ], [ %CurrentObjectToWorld.i167.2.i6, %426 ]
  %CurrentObjectToWorld.i167.1.i7 = phi float [ %CurrentObjectToWorld.i167.0177.i7, %.lr.ph ], [ %CurrentObjectToWorld.i167.2.i7, %426 ]
  %CurrentObjectToWorld.i167.1.i8 = phi float [ %CurrentObjectToWorld.i167.0177.i8, %.lr.ph ], [ %CurrentObjectToWorld.i167.2.i8, %426 ]
  %CurrentObjectToWorld.i167.1.i9 = phi float [ %CurrentObjectToWorld.i167.0177.i9, %.lr.ph ], [ %CurrentObjectToWorld.i167.2.i9, %426 ]
  %CurrentObjectToWorld.i167.1.i10 = phi float [ %CurrentObjectToWorld.i167.0177.i10, %.lr.ph ], [ %CurrentObjectToWorld.i167.2.i10, %426 ]
  %CurrentObjectToWorld.i167.1.i11 = phi float [ %CurrentObjectToWorld.i167.0177.i11, %.lr.ph ], [ %CurrentObjectToWorld.i167.2.i11, %426 ]
  %CurrentWorldToObject.i168.1.i0 = phi float [ %CurrentWorldToObject.i168.0178.i0, %.lr.ph ], [ %CurrentWorldToObject.i168.2.i0, %426 ]
  %CurrentWorldToObject.i168.1.i1 = phi float [ %CurrentWorldToObject.i168.0178.i1, %.lr.ph ], [ %CurrentWorldToObject.i168.2.i1, %426 ]
  %CurrentWorldToObject.i168.1.i2 = phi float [ %CurrentWorldToObject.i168.0178.i2, %.lr.ph ], [ %CurrentWorldToObject.i168.2.i2, %426 ]
  %CurrentWorldToObject.i168.1.i3 = phi float [ %CurrentWorldToObject.i168.0178.i3, %.lr.ph ], [ %CurrentWorldToObject.i168.2.i3, %426 ]
  %CurrentWorldToObject.i168.1.i4 = phi float [ %CurrentWorldToObject.i168.0178.i4, %.lr.ph ], [ %CurrentWorldToObject.i168.2.i4, %426 ]
  %CurrentWorldToObject.i168.1.i5 = phi float [ %CurrentWorldToObject.i168.0178.i5, %.lr.ph ], [ %CurrentWorldToObject.i168.2.i5, %426 ]
  %CurrentWorldToObject.i168.1.i6 = phi float [ %CurrentWorldToObject.i168.0178.i6, %.lr.ph ], [ %CurrentWorldToObject.i168.2.i6, %426 ]
  %CurrentWorldToObject.i168.1.i7 = phi float [ %CurrentWorldToObject.i168.0178.i7, %.lr.ph ], [ %CurrentWorldToObject.i168.2.i7, %426 ]
  %CurrentWorldToObject.i168.1.i8 = phi float [ %CurrentWorldToObject.i168.0178.i8, %.lr.ph ], [ %CurrentWorldToObject.i168.2.i8, %426 ]
  %CurrentWorldToObject.i168.1.i9 = phi float [ %CurrentWorldToObject.i168.0178.i9, %.lr.ph ], [ %CurrentWorldToObject.i168.2.i9, %426 ]
  %CurrentWorldToObject.i168.1.i10 = phi float [ %CurrentWorldToObject.i168.0178.i10, %.lr.ph ], [ %CurrentWorldToObject.i168.2.i10, %426 ]
  %CurrentWorldToObject.i168.1.i11 = phi float [ %CurrentWorldToObject.i168.0178.i11, %.lr.ph ], [ %CurrentWorldToObject.i168.2.i11, %426 ]
  %ResetMatrices.i.1 = phi i32 [ 1, %.lr.ph ], [ %ResetMatrices.i.3, %426 ]
  %currentBVHIndex.i.1 = phi i32 [ %currentBVHIndex.i.0179, %.lr.ph ], [ %currentBVHIndex.i.2, %426 ]
  %currentGpuVA.i.1.i0 = phi i32 [ %currentGpuVA.i.0180.i0, %.lr.ph ], [ %currentGpuVA.i.2.i0, %426 ]
  %currentGpuVA.i.1.i1 = phi i32 [ %currentGpuVA.i.0180.i1, %.lr.ph ], [ %currentGpuVA.i.2.i1, %426 ]
  %instIdx.i.1 = phi i32 [ %instIdx.i.0181, %.lr.ph ], [ %instIdx.i.2, %426 ]
  %instFlags.i.1 = phi i32 [ %instFlags.i.0182, %.lr.ph ], [ %instFlags.i.2, %426 ]
  %instOffset.i.1 = phi i32 [ %instOffset.i.0183, %.lr.ph ], [ %instOffset.i.2, %426 ]
  %instId.i.1 = phi i32 [ %instId.i.0184, %.lr.ph ], [ %instId.i.2, %426 ]
  %stackPointer.i.2 = phi i32 [ %stackPointer.i.1185, %.lr.ph ], [ %stackPointer.i.3, %426 ]
  %resultBary.i.1.i0 = phi float [ %resultBary.i.0187.i0, %.lr.ph ], [ %resultBary.i.2.i0, %426 ]
  %resultBary.i.1.i1 = phi float [ %resultBary.i.0187.i1, %.lr.ph ], [ %resultBary.i.2.i1, %426 ]
  %resultTriId.i.1 = phi i32 [ %resultTriId.i.0188, %.lr.ph ], [ %resultTriId.i.2, %426 ]
  %currentRayData.i.1.1.upto0 = insertelement <3 x float> undef, float %currentRayData.i.1.1.i0, i32 0
  %currentRayData.i.1.1.upto1 = insertelement <3 x float> %currentRayData.i.1.1.upto0, float %currentRayData.i.1.1.i1, i32 1
  %currentRayData.i.1.1 = insertelement <3 x float> %currentRayData.i.1.1.upto1, float %currentRayData.i.1.1.i2, i32 2
  %currentRayData.i.0.1.upto0 = insertelement <3 x float> undef, float %currentRayData.i.0.1.i0, i32 0
  %currentRayData.i.0.1.upto1 = insertelement <3 x float> %currentRayData.i.0.1.upto0, float %currentRayData.i.0.1.i1, i32 1
  %currentRayData.i.0.1 = insertelement <3 x float> %currentRayData.i.0.1.upto1, float %currentRayData.i.0.1.i2, i32 2
  %90 = add nsw i32 %stackPointer.i.2, -1
  %91 = shl i32 %90, 6
  %92 = add i32 %91, %14
  %93 = getelementptr [2048 x i32], [2048 x i32] addrspace(3)* @"\01?stack@@3PAIA", i32 0, i32 %92
  %94 = load i32, i32 addrspace(3)* %93, align 4, !tbaa !221, !noalias !295
  %95 = getelementptr inbounds [2 x i32], [2 x i32]* %nodesToProcess.i, i32 0, i32 %currentBVHIndex.i.1
  %96 = add i32 %89, -1
  store i32 %96, i32* %95, align 4, !tbaa !221
  %97 = getelementptr inbounds [0 x %struct.RWByteAddressBuffer], [0 x %struct.RWByteAddressBuffer]* @"\01?DescriptorHeapBufferTable@@3PAURWByteAddressBuffer@@A", i32 0, i32 %currentGpuVA.i.1.i1, !dx.nonuniform !240
  %98 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %97, align 4, !noalias !298
  %99 = add i32 %currentGpuVA.i.1.i0, 16
  %100 = shl i32 %94, 5
  %101 = add i32 %99, %100
  %102 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %98)
  %103 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %102, i32 %101, i32 undef)
  %104 = extractvalue %dx.types.ResRet.i32 %103, 3
  %105 = add i32 %101, 16
  %106 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %102, i32 %105, i32 undef)
  %107 = extractvalue %dx.types.ResRet.i32 %106, 3
  %108 = icmp slt i32 %104, 0
  br i1 %108, label %109, label %374

; <label>:109                                     ; preds = %88
  %110 = icmp eq i32 %currentBVHIndex.i.1, 0
  br i1 %110, label %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i189", label %"\01?IsOpaque@@YA_N_NII@Z.exit.i"

"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i189": ; preds = %109
  %111 = and i32 %104, 2147483647
  %112 = mul i32 %111, 112
  %113 = add i32 %48, %112
  %114 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %43)
  %115 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %114, i32 %113, i32 undef)
  %116 = extractvalue %dx.types.ResRet.i32 %115, 0
  %117 = extractvalue %dx.types.ResRet.i32 %115, 1
  %118 = extractvalue %dx.types.ResRet.i32 %115, 2
  %119 = extractvalue %dx.types.ResRet.i32 %115, 3
  %120 = add i32 %113, 16
  %121 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %114, i32 %120, i32 undef)
  %122 = extractvalue %dx.types.ResRet.i32 %121, 0
  %123 = extractvalue %dx.types.ResRet.i32 %121, 1
  %124 = extractvalue %dx.types.ResRet.i32 %121, 2
  %125 = extractvalue %dx.types.ResRet.i32 %121, 3
  %126 = add i32 %113, 32
  %127 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %114, i32 %126, i32 undef)
  %128 = extractvalue %dx.types.ResRet.i32 %127, 0
  %129 = extractvalue %dx.types.ResRet.i32 %127, 1
  %130 = extractvalue %dx.types.ResRet.i32 %127, 2
  %131 = extractvalue %dx.types.ResRet.i32 %127, 3
  %132 = add i32 %113, 48
  %133 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %114, i32 %132, i32 undef)
  %134 = extractvalue %dx.types.ResRet.i32 %133, 0
  %135 = extractvalue %dx.types.ResRet.i32 %133, 1
  %136 = extractvalue %dx.types.ResRet.i32 %133, 2
  %137 = extractvalue %dx.types.ResRet.i32 %133, 3
  %138 = add i32 %113, 64
  %139 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %114, i32 %138, i32 undef)
  %140 = extractvalue %dx.types.ResRet.i32 %139, 0
  %141 = extractvalue %dx.types.ResRet.i32 %139, 1
  %142 = extractvalue %dx.types.ResRet.i32 %139, 2
  %143 = extractvalue %dx.types.ResRet.i32 %139, 3
  %144 = add i32 %113, 80
  %145 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %114, i32 %144, i32 undef)
  %146 = extractvalue %dx.types.ResRet.i32 %145, 0
  %147 = extractvalue %dx.types.ResRet.i32 %145, 1
  %148 = extractvalue %dx.types.ResRet.i32 %145, 2
  %149 = extractvalue %dx.types.ResRet.i32 %145, 3
  %150 = add i32 %113, 96
  %151 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %114, i32 %150, i32 undef)
  %152 = extractvalue %dx.types.ResRet.i32 %151, 0
  %153 = extractvalue %dx.types.ResRet.i32 %151, 1
  %154 = extractvalue %dx.types.ResRet.i32 %151, 2
  %155 = extractvalue %dx.types.ResRet.i32 %151, 3
  %156 = bitcast i32 %116 to float
  %157 = bitcast i32 %117 to float
  %158 = bitcast i32 %118 to float
  %159 = bitcast i32 %119 to float
  %160 = bitcast i32 %122 to float
  %161 = bitcast i32 %123 to float
  %162 = bitcast i32 %124 to float
  %163 = bitcast i32 %125 to float
  %164 = bitcast i32 %128 to float
  %165 = bitcast i32 %129 to float
  %166 = bitcast i32 %130 to float
  %167 = bitcast i32 %131 to float
  %168 = and i32 %135, 16777215
  %169 = and i32 %134, 16777215
  %170 = lshr i32 %134, 24
  %171 = and i32 %170, %0
  %172 = icmp eq i32 %171, 0
  %173 = bitcast i32 %140 to float
  %174 = bitcast i32 %141 to float
  %175 = bitcast i32 %142 to float
  %176 = bitcast i32 %143 to float
  %177 = bitcast i32 %146 to float
  %178 = bitcast i32 %147 to float
  %179 = bitcast i32 %148 to float
  %180 = bitcast i32 %149 to float
  %181 = bitcast i32 %152 to float
  %182 = bitcast i32 %153 to float
  %183 = bitcast i32 %154 to float
  %184 = bitcast i32 %155 to float
  br i1 %172, label %426, label %185

; <label>:185                                     ; preds = %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i189"
  store i32 0, i32 addrspace(3)* %93, align 4, !tbaa !221, !noalias !303
  %186 = lshr i32 %135, 24
  %187 = fmul fast float %WorldRayDirection251, %156
  %FMad294 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayDirection252, float %157, float %187)
  %FMad293 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayDirection253, float %158, float %FMad294)
  %FMad292 = call float @dx.op.tertiary.f32(i32 46, float 0.000000e+00, float %159, float %FMad293)
  %188 = fmul fast float %WorldRayDirection251, %160
  %FMad291 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayDirection252, float %161, float %188)
  %FMad290 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayDirection253, float %162, float %FMad291)
  %FMad289 = call float @dx.op.tertiary.f32(i32 46, float 0.000000e+00, float %163, float %FMad290)
  %189 = fmul fast float %WorldRayDirection251, %164
  %FMad288 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayDirection252, float %165, float %189)
  %FMad287 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayDirection253, float %166, float %FMad288)
  %FMad286 = call float @dx.op.tertiary.f32(i32 46, float 0.000000e+00, float %167, float %FMad287)
  %190 = fmul fast float %WorldRayOrigin248, %156
  %FMad285 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayOrigin249, float %157, float %190)
  %FMad284 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayOrigin250, float %158, float %FMad285)
  %FMad283 = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %159, float %FMad284)
  %191 = fmul fast float %WorldRayOrigin248, %160
  %FMad282 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayOrigin249, float %161, float %191)
  %FMad281 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayOrigin250, float %162, float %FMad282)
  %FMad280 = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %163, float %FMad281)
  %192 = fmul fast float %WorldRayOrigin248, %164
  %FMad279 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayOrigin249, float %165, float %192)
  %FMad278 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayOrigin250, float %166, float %FMad279)
  %FMad = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %167, float %FMad278)
  store float %FMad292, float* %73, align 4
  store float %FMad289, float* %74, align 4
  store float %FMad286, float* %75, align 4
  %.i0340 = fdiv fast float 1.000000e+00, %FMad292
  %.i1341 = fdiv fast float 1.000000e+00, %FMad289
  %.i2342 = fdiv fast float 1.000000e+00, %FMad286
  %.i0343 = fmul fast float %.i0340, %FMad283
  %.i1344 = fmul fast float %.i1341, %FMad280
  %.i2345 = fmul fast float %.i2342, %FMad
  %FAbs = call float @dx.op.unary.f32(i32 6, float %FMad292)
  %FAbs221 = call float @dx.op.unary.f32(i32 6, float %FMad289)
  %FAbs222 = call float @dx.op.unary.f32(i32 6, float %FMad286)
  %193 = fcmp fast ogt float %FAbs, %FAbs221
  %194 = fcmp fast ogt float %FAbs, %FAbs222
  %195 = and i1 %193, %194
  br i1 %195, label %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i", label %196

; <label>:196                                     ; preds = %185
  %197 = fcmp fast ogt float %FAbs221, %FAbs222
  br i1 %197, label %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i", label %198

; <label>:198                                     ; preds = %196
  br label %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i"

"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i": ; preds = %198, %196, %185
  %.0172 = phi i32 [ 2, %198 ], [ 0, %185 ], [ 1, %196 ]
  %199 = add nuw nsw i32 %.0172, 1
  %200 = urem i32 %199, 3
  %201 = add nuw nsw i32 %.0172, 2
  %202 = urem i32 %201, 3
  %203 = getelementptr [3 x float], [3 x float]* %12, i32 0, i32 %.0172
  %204 = load float, float* %203, align 4, !tbaa !231, !noalias !306
  %205 = fcmp fast olt float %204, 0.000000e+00
  %.5.0.i0 = select i1 %205, i32 %202, i32 %200
  %.5.0.i1 = select i1 %205, i32 %200, i32 %202
  %206 = getelementptr [3 x float], [3 x float]* %12, i32 0, i32 %.5.0.i0
  %207 = load float, float* %206, align 4, !tbaa !231, !noalias !306
  %208 = fdiv float %207, %204
  %209 = getelementptr [3 x float], [3 x float]* %12, i32 0, i32 %.5.0.i1
  %210 = load float, float* %209, align 4, !tbaa !231, !noalias !306
  %211 = fdiv float %210, %204
  %212 = fdiv float 1.000000e+00, %204
  store i32 1, i32* %76, align 4, !tbaa !221
  br label %426

"\01?IsOpaque@@YA_N_NII@Z.exit.i":                ; preds = %109
  %RayFlags256 = call i32 @dx.op.rayFlags.i32(i32 144)
  %213 = and i32 %RayFlags256, 64
  %RayTCurrent = call float @dx.op.rayTCurrent.f32(i32 154)
  %214 = icmp eq i32 %213, 0
  %215 = and i32 %104, 16777215
  %216 = add i32 %currentGpuVA.i.1.i0, 4
  %217 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %98)
  %218 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %217, i32 %216, i32 undef)
  %219 = extractvalue %dx.types.ResRet.i32 %218, 0
  %220 = mul nuw nsw i32 %215, 36
  %221 = add i32 %220, %currentGpuVA.i.1.i0
  %222 = add i32 %221, %219
  %223 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %217, i32 %222, i32 undef)
  %224 = extractvalue %dx.types.ResRet.i32 %223, 0
  %225 = extractvalue %dx.types.ResRet.i32 %223, 1
  %226 = extractvalue %dx.types.ResRet.i32 %223, 2
  %227 = extractvalue %dx.types.ResRet.i32 %223, 3
  %.i0346 = bitcast i32 %224 to float
  %.i1347 = bitcast i32 %225 to float
  %.i2348 = bitcast i32 %226 to float
  %.i3349 = bitcast i32 %227 to float
  %228 = add i32 %222, 16
  %229 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %217, i32 %228, i32 undef)
  %230 = extractvalue %dx.types.ResRet.i32 %229, 0
  %231 = extractvalue %dx.types.ResRet.i32 %229, 1
  %232 = extractvalue %dx.types.ResRet.i32 %229, 2
  %233 = extractvalue %dx.types.ResRet.i32 %229, 3
  %.i0350 = bitcast i32 %230 to float
  %.i1351 = bitcast i32 %231 to float
  %.i2352 = bitcast i32 %232 to float
  %.i3353 = bitcast i32 %233 to float
  %234 = add i32 %222, 32
  %235 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %217, i32 %234, i32 undef)
  %236 = extractvalue %dx.types.ResRet.i32 %235, 0
  %237 = bitcast i32 %236 to float
  %238 = and i32 %instFlags.i.1, 1
  %239 = icmp eq i32 %238, 0
  %240 = shl i32 %instFlags.i.1, 3
  %241 = and i32 %240, 16
  %242 = add nuw nsw i32 %241, 16
  %243 = xor i32 %241, 16
  %244 = add nuw nsw i32 %243, 16
  %245 = and i32 %RayFlags256, %242
  %246 = icmp ne i32 %245, 0
  %247 = and i1 %239, %246
  %248 = and i32 %RayFlags256, %244
  %249 = icmp ne i32 %248, 0
  %250 = and i1 %239, %249
  %.i0354 = fsub fast float %.i0346, %currentRayData.i.0.1.i0
  %.i1355 = fsub fast float %.i1347, %currentRayData.i.0.1.i1
  %.i2356 = fsub fast float %.i2348, %currentRayData.i.0.1.i2
  store float %.i0354, float* %77, align 4
  store float %.i1355, float* %78, align 4
  store float %.i2356, float* %79, align 4
  %251 = getelementptr [3 x float], [3 x float]* %11, i32 0, i32 %currentRayData.i.5.1.i0
  %252 = load float, float* %251, align 4, !tbaa !231, !noalias !309
  %253 = getelementptr [3 x float], [3 x float]* %11, i32 0, i32 %currentRayData.i.5.1.i1
  %254 = load float, float* %253, align 4, !tbaa !231, !noalias !309
  %255 = getelementptr [3 x float], [3 x float]* %11, i32 0, i32 %currentRayData.i.5.1.i2
  %256 = load float, float* %255, align 4, !tbaa !231, !noalias !309
  %.i0357 = fsub fast float %.i3349, %currentRayData.i.0.1.i0
  %.i1358 = fsub fast float %.i0350, %currentRayData.i.0.1.i1
  %.i2359 = fsub fast float %.i1351, %currentRayData.i.0.1.i2
  store float %.i0357, float* %80, align 4
  store float %.i1358, float* %81, align 4
  store float %.i2359, float* %82, align 4
  %257 = getelementptr [3 x float], [3 x float]* %9, i32 0, i32 %currentRayData.i.5.1.i0
  %258 = load float, float* %257, align 4, !tbaa !231, !noalias !309
  %259 = getelementptr [3 x float], [3 x float]* %9, i32 0, i32 %currentRayData.i.5.1.i1
  %260 = load float, float* %259, align 4, !tbaa !231, !noalias !309
  %261 = getelementptr [3 x float], [3 x float]* %9, i32 0, i32 %currentRayData.i.5.1.i2
  %262 = load float, float* %261, align 4, !tbaa !231, !noalias !309
  %.i0360 = fsub fast float %.i2352, %currentRayData.i.0.1.i0
  %.i1361 = fsub fast float %.i3353, %currentRayData.i.0.1.i1
  %.i2362 = fsub fast float %237, %currentRayData.i.0.1.i2
  store float %.i0360, float* %83, align 4
  store float %.i1361, float* %84, align 4
  store float %.i2362, float* %85, align 4
  %263 = getelementptr [3 x float], [3 x float]* %10, i32 0, i32 %currentRayData.i.5.1.i0
  %264 = load float, float* %263, align 4, !tbaa !231, !noalias !309
  %265 = getelementptr [3 x float], [3 x float]* %10, i32 0, i32 %currentRayData.i.5.1.i1
  %266 = load float, float* %265, align 4, !tbaa !231, !noalias !309
  %267 = getelementptr [3 x float], [3 x float]* %10, i32 0, i32 %currentRayData.i.5.1.i2
  %268 = load float, float* %267, align 4, !tbaa !231, !noalias !309
  %.i0363 = fmul float %currentRayData.i.4.1.i0, %256
  %.i1364 = fmul float %currentRayData.i.4.1.i1, %256
  %.i0366 = fsub float %252, %.i0363
  %.i1367 = fsub float %254, %.i1364
  %.i0368 = fmul float %currentRayData.i.4.1.i0, %262
  %.i1369 = fmul float %currentRayData.i.4.1.i1, %262
  %.i0371 = fsub float %258, %.i0368
  %.i1372 = fsub float %260, %.i1369
  %.i0373 = fmul float %currentRayData.i.4.1.i0, %268
  %.i1374 = fmul float %currentRayData.i.4.1.i1, %268
  %.i0376 = fsub float %264, %.i0373
  %.i1377 = fsub float %266, %.i1374
  %269 = fmul float %.i1372, %.i0376
  %270 = fmul float %.i0371, %.i1377
  %271 = fsub float %269, %270
  %272 = fmul float %.i0366, %.i1377
  %273 = fmul float %.i1367, %.i0376
  %274 = fsub float %272, %273
  %275 = fmul float %.i1367, %.i0371
  %276 = fmul float %.i0366, %.i1372
  %277 = fsub float %275, %276
  %278 = fadd fast float %274, %277
  %279 = fadd fast float %278, %271
  br i1 %250, label %280, label %286

; <label>:280                                     ; preds = %"\01?IsOpaque@@YA_N_NII@Z.exit.i"
  %281 = fcmp fast ogt float %271, 0.000000e+00
  %282 = fcmp fast ogt float %274, 0.000000e+00
  %283 = or i1 %281, %282
  %284 = fcmp fast ogt float %277, 0.000000e+00
  %285 = or i1 %284, %283
  %.old.old = fcmp fast oeq float %279, 0.000000e+00
  %or.cond195 = or i1 %285, %.old.old
  br i1 %or.cond195, label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i", label %306

; <label>:286                                     ; preds = %"\01?IsOpaque@@YA_N_NII@Z.exit.i"
  br i1 %247, label %287, label %293

; <label>:287                                     ; preds = %286
  %288 = fcmp fast olt float %271, 0.000000e+00
  %289 = fcmp fast olt float %274, 0.000000e+00
  %290 = or i1 %288, %289
  %291 = fcmp fast olt float %277, 0.000000e+00
  %292 = or i1 %291, %290
  %.old = fcmp fast oeq float %279, 0.000000e+00
  %or.cond194 = or i1 %292, %.old
  br i1 %or.cond194, label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i", label %306

; <label>:293                                     ; preds = %286
  %294 = fcmp fast olt float %271, 0.000000e+00
  %295 = fcmp fast olt float %274, 0.000000e+00
  %296 = or i1 %294, %295
  %297 = fcmp fast olt float %277, 0.000000e+00
  %298 = or i1 %297, %296
  %299 = fcmp fast ogt float %271, 0.000000e+00
  %300 = fcmp fast ogt float %274, 0.000000e+00
  %301 = or i1 %299, %300
  %302 = fcmp fast ogt float %277, 0.000000e+00
  %303 = or i1 %302, %301
  %304 = and i1 %298, %303
  %305 = fcmp fast oeq float %279, 0.000000e+00
  %or.cond193 = or i1 %305, %304
  br i1 %or.cond193, label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i", label %306

; <label>:306                                     ; preds = %293, %287, %280
  %307 = fmul fast float %271, %256
  %308 = fmul fast float %274, %262
  %309 = fmul fast float %277, %268
  %tmp = fadd fast float %308, %309
  %tmp517 = fadd fast float %tmp, %307
  %tmp518 = fmul fast float %tmp517, %currentRayData.i.4.1.i2
  br i1 %250, label %310, label %315

; <label>:310                                     ; preds = %306
  %311 = fcmp fast ogt float %tmp518, 0.000000e+00
  %312 = fmul fast float %279, %RayTCurrent
  %313 = fcmp fast olt float %tmp518, %312
  %314 = or i1 %311, %313
  br i1 %314, label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i", label %332

; <label>:315                                     ; preds = %306
  br i1 %247, label %316, label %321

; <label>:316                                     ; preds = %315
  %317 = fcmp fast olt float %tmp518, 0.000000e+00
  %318 = fmul fast float %279, %RayTCurrent
  %319 = fcmp fast ogt float %tmp518, %318
  %320 = or i1 %317, %319
  br i1 %320, label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i", label %332

; <label>:321                                     ; preds = %315
  %322 = fcmp fast ogt float %279, 0.000000e+00
  %323 = select i1 %322, i32 1, i32 -1
  %324 = bitcast float %tmp518 to i32
  %325 = xor i32 %324, %323
  %326 = uitofp i32 %325 to float
  %327 = bitcast float %279 to i32
  %328 = xor i32 %323, %327
  %329 = uitofp i32 %328 to float
  %330 = fmul fast float %329, %RayTCurrent
  %331 = fcmp fast ogt float %326, %330
  br i1 %331, label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i", label %332

; <label>:332                                     ; preds = %321, %316, %310
  %333 = fdiv fast float 1.000000e+00, %279
  %334 = fmul fast float %333, %274
  %335 = fmul fast float %333, %277
  %336 = fmul fast float %333, %tmp518
  br label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i"

"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i": ; preds = %332, %321, %316, %310, %293, %287, %280
  %.2.i0 = phi float [ %.1.i0, %280 ], [ %.1.i0, %310 ], [ %334, %332 ], [ %.1.i0, %316 ], [ %.1.i0, %321 ], [ %.1.i0, %287 ], [ %.1.i0, %293 ]
  %.2.i1 = phi float [ %.1.i1, %280 ], [ %.1.i1, %310 ], [ %335, %332 ], [ %.1.i1, %316 ], [ %.1.i1, %321 ], [ %.1.i1, %287 ], [ %.1.i1, %293 ]
  %.0173 = phi float [ %RayTCurrent, %280 ], [ %RayTCurrent, %310 ], [ %336, %332 ], [ %RayTCurrent, %316 ], [ %RayTCurrent, %321 ], [ %RayTCurrent, %287 ], [ %RayTCurrent, %293 ]
  %337 = fcmp fast olt float %.0173, %RayTCurrent
  %RayTMin = call float @dx.op.rayTMin.f32(i32 153)
  %338 = fcmp fast ogt float %.0173, %RayTMin
  %339 = and i1 %337, %338
  %. = select i1 %339, i1 true, i1 false
  %.resultTriId.i.1 = select i1 %339, i32 %215, i32 %resultTriId.i.1
  %.0173.RayTCurrent = select i1 %339, float %.0173, float %RayTCurrent
  %.2.i0.resultBary.i.1.i0 = select i1 %339, float %.2.i0, float %resultBary.i.1.i0
  %.2.i1.resultBary.i.1.i1 = select i1 %339, float %.2.i1, float %resultBary.i.1.i1
  %340 = and i1 %214, %.
  br i1 %340, label %341, label %426

; <label>:341                                     ; preds = %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i"
  %.0.upto0 = insertelement <2 x float> undef, float %.2.i0.resultBary.i.1.i0, i32 0
  %.0 = insertelement <2 x float> %.0.upto0, float %.2.i1.resultBary.i.1.i1, i32 1
  %342 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %97, align 4, !noalias !317
  %343 = add i32 %currentGpuVA.i.1.i0, 8
  %344 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %342)
  %345 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %344, i32 %343, i32 undef)
  %346 = extractvalue %dx.types.ResRet.i32 %345, 0
  %347 = shl i32 %.resultTriId.i.1, 3
  %348 = add i32 %347, %currentGpuVA.i.1.i0
  %349 = add i32 %348, %346
  %350 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %344, i32 %349, i32 undef)
  %351 = extractvalue %dx.types.ResRet.i32 %350, 0
  %352 = extractvalue %dx.types.ResRet.i32 %350, 1
  %353 = mul i32 %351, %2
  %354 = add i32 %instOffset.i.1, %1
  %355 = add i32 %354, %353
  store <2 x float> %.0, <2 x float>* %86, align 4, !tbaa !218
  call void @"\01?Fallback_SetPendingAttr@@YAXUBuiltInTriangleIntersectionAttributes@@@Z"(%struct.BuiltInTriangleIntersectionAttributes* nonnull %attr.i) #3
  call void @"\01?Fallback_SetPendingTriVals@@YAXMIIIII@Z"(float %.0173.RayTCurrent, i32 %352, i32 %355, i32 %instIdx.i.1, i32 %instId.i.1, i32 254) #3
  %356 = icmp eq i32 %ResetMatrices.i.1, 0
  br i1 %356, label %370, label %357

; <label>:357                                     ; preds = %341
  call void @"\01?Fallback_SetObjectRayOrigin@@YAXV?$vector@M$02@@@Z"(<3 x float> %currentRayData.i.0.1) #3
  call void @"\01?Fallback_SetObjectRayDirection@@YAXV?$vector@M$02@@@Z"(<3 x float> %currentRayData.i.1.1) #3
  %.upto0 = insertelement <4 x float> undef, float %CurrentWorldToObject.i168.1.i0, i32 0
  %.upto1 = insertelement <4 x float> %.upto0, float %CurrentWorldToObject.i168.1.i1, i32 1
  %.upto2 = insertelement <4 x float> %.upto1, float %CurrentWorldToObject.i168.1.i2, i32 2
  %358 = insertelement <4 x float> %.upto2, float %CurrentWorldToObject.i168.1.i3, i32 3
  %359 = insertvalue %class.matrix.float.3.4 undef, <4 x float> %358, 0, 0
  %.upto01 = insertelement <4 x float> undef, float %CurrentWorldToObject.i168.1.i4, i32 0
  %.upto12 = insertelement <4 x float> %.upto01, float %CurrentWorldToObject.i168.1.i5, i32 1
  %.upto23 = insertelement <4 x float> %.upto12, float %CurrentWorldToObject.i168.1.i6, i32 2
  %360 = insertelement <4 x float> %.upto23, float %CurrentWorldToObject.i168.1.i7, i32 3
  %361 = insertvalue %class.matrix.float.3.4 %359, <4 x float> %360, 0, 1
  %.upto04 = insertelement <4 x float> undef, float %CurrentWorldToObject.i168.1.i8, i32 0
  %.upto15 = insertelement <4 x float> %.upto04, float %CurrentWorldToObject.i168.1.i9, i32 1
  %.upto26 = insertelement <4 x float> %.upto15, float %CurrentWorldToObject.i168.1.i10, i32 2
  %362 = insertelement <4 x float> %.upto26, float %CurrentWorldToObject.i168.1.i11, i32 3
  %363 = insertvalue %class.matrix.float.3.4 %361, <4 x float> %362, 0, 2
  call void @"\01?Fallback_SetWorldToObject@@YAXV?$matrix@M$02$03@@@Z"(%class.matrix.float.3.4 %363) #3
  %.upto07 = insertelement <4 x float> undef, float %CurrentObjectToWorld.i167.1.i0, i32 0
  %.upto18 = insertelement <4 x float> %.upto07, float %CurrentObjectToWorld.i167.1.i1, i32 1
  %.upto29 = insertelement <4 x float> %.upto18, float %CurrentObjectToWorld.i167.1.i2, i32 2
  %364 = insertelement <4 x float> %.upto29, float %CurrentObjectToWorld.i167.1.i3, i32 3
  %365 = insertvalue %class.matrix.float.3.4 undef, <4 x float> %364, 0, 0
  %.upto010 = insertelement <4 x float> undef, float %CurrentObjectToWorld.i167.1.i4, i32 0
  %.upto111 = insertelement <4 x float> %.upto010, float %CurrentObjectToWorld.i167.1.i5, i32 1
  %.upto212 = insertelement <4 x float> %.upto111, float %CurrentObjectToWorld.i167.1.i6, i32 2
  %366 = insertelement <4 x float> %.upto212, float %CurrentObjectToWorld.i167.1.i7, i32 3
  %367 = insertvalue %class.matrix.float.3.4 %365, <4 x float> %366, 0, 1
  %.upto013 = insertelement <4 x float> undef, float %CurrentObjectToWorld.i167.1.i8, i32 0
  %.upto114 = insertelement <4 x float> %.upto013, float %CurrentObjectToWorld.i167.1.i9, i32 1
  %.upto215 = insertelement <4 x float> %.upto114, float %CurrentObjectToWorld.i167.1.i10, i32 2
  %368 = insertelement <4 x float> %.upto215, float %CurrentObjectToWorld.i167.1.i11, i32 3
  %369 = insertvalue %class.matrix.float.3.4 %367, <4 x float> %368, 0, 2
  call void @"\01?Fallback_SetObjectToWorld@@YAXV?$matrix@M$02$03@@@Z"(%class.matrix.float.3.4 %369) #3
  br label %370

; <label>:370                                     ; preds = %357, %341
  call void @"\01?Fallback_CommitHit@@YAXXZ"() #3
  %371 = and i32 %RayFlags256, 4
  %372 = icmp eq i32 %371, 0
  br i1 %372, label %426, label %373

; <label>:373                                     ; preds = %370
  store i32 0, i32* %76, align 4, !tbaa !221
  store i32 0, i32* %41, align 4, !tbaa !221
  br label %426

; <label>:374                                     ; preds = %88
  %375 = and i32 %104, 16777215
  %RayTCurrent219 = call float @dx.op.rayTCurrent.f32(i32 154)
  %376 = shl nuw nsw i32 %375, 5
  %377 = add i32 %99, %376
  %378 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %98)
  %379 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %378, i32 %377, i32 undef)
  %380 = extractvalue %dx.types.ResRet.i32 %379, 0
  %381 = extractvalue %dx.types.ResRet.i32 %379, 1
  %382 = extractvalue %dx.types.ResRet.i32 %379, 2
  %383 = add i32 %377, 16
  %384 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %378, i32 %383, i32 undef)
  %385 = extractvalue %dx.types.ResRet.i32 %384, 0
  %386 = extractvalue %dx.types.ResRet.i32 %384, 1
  %387 = extractvalue %dx.types.ResRet.i32 %384, 2
  %388 = bitcast i32 %380 to float
  %389 = bitcast i32 %381 to float
  %390 = bitcast i32 %382 to float
  %391 = bitcast i32 %385 to float
  %392 = bitcast i32 %386 to float
  %393 = bitcast i32 %387 to float
  %394 = shl i32 %107, 5
  %395 = add i32 %99, %394
  %396 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %378, i32 %395, i32 undef)
  %397 = extractvalue %dx.types.ResRet.i32 %396, 0
  %398 = extractvalue %dx.types.ResRet.i32 %396, 1
  %399 = extractvalue %dx.types.ResRet.i32 %396, 2
  %400 = add i32 %395, 16
  %401 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %378, i32 %400, i32 undef)
  %402 = extractvalue %dx.types.ResRet.i32 %401, 0
  %403 = extractvalue %dx.types.ResRet.i32 %401, 1
  %404 = extractvalue %dx.types.ResRet.i32 %401, 2
  %405 = bitcast i32 %397 to float
  %406 = bitcast i32 %398 to float
  %407 = bitcast i32 %399 to float
  %408 = bitcast i32 %402 to float
  %409 = bitcast i32 %403 to float
  %410 = bitcast i32 %404 to float
  %.i0378 = fmul fast float %388, %currentRayData.i.2.1.i0
  %.i1379 = fmul fast float %389, %currentRayData.i.2.1.i1
  %.i2380 = fmul fast float %390, %currentRayData.i.2.1.i2
  %.i0381 = fsub fast float %.i0378, %currentRayData.i.3.1.i0
  %.i1382 = fsub fast float %.i1379, %currentRayData.i.3.1.i1
  %.i2383 = fsub fast float %.i2380, %currentRayData.i.3.1.i2
  %FAbs232 = call float @dx.op.unary.f32(i32 6, float %currentRayData.i.2.1.i0)
  %FAbs233 = call float @dx.op.unary.f32(i32 6, float %currentRayData.i.2.1.i1)
  %FAbs234 = call float @dx.op.unary.f32(i32 6, float %currentRayData.i.2.1.i2)
  %.i0384 = fmul fast float %FAbs232, %391
  %.i1385 = fmul fast float %FAbs233, %392
  %.i2386 = fmul fast float %FAbs234, %393
  %.i0387 = fadd fast float %.i0384, %.i0381
  %.i1388 = fadd fast float %.i1385, %.i1382
  %.i2389 = fadd fast float %.i2386, %.i2383
  %.i0393 = fsub fast float %.i0381, %.i0384
  %.i1394 = fsub fast float %.i1382, %.i1385
  %.i2395 = fsub fast float %.i2383, %.i2386
  %FMax271 = call float @dx.op.binary.f32(i32 35, float %.i0393, float %.i1394)
  %FMax270 = call float @dx.op.binary.f32(i32 35, float %FMax271, float %.i2395)
  %FMin269 = call float @dx.op.binary.f32(i32 36, float %.i0387, float %.i1388)
  %FMin268 = call float @dx.op.binary.f32(i32 36, float %FMin269, float %.i2389)
  %FMax266 = call float @dx.op.binary.f32(i32 35, float %FMax270, float 0.000000e+00)
  %FMin265 = call float @dx.op.binary.f32(i32 36, float %FMin268, float %RayTCurrent219)
  %411 = fcmp fast olt float %FMax266, %FMin265
  %.i0396 = fmul fast float %405, %currentRayData.i.2.1.i0
  %.i1397 = fmul fast float %406, %currentRayData.i.2.1.i1
  %.i2398 = fmul fast float %407, %currentRayData.i.2.1.i2
  %.i0399 = fsub fast float %.i0396, %currentRayData.i.3.1.i0
  %.i1400 = fsub fast float %.i1397, %currentRayData.i.3.1.i1
  %.i2401 = fsub fast float %.i2398, %currentRayData.i.3.1.i2
  %.i0402 = fmul fast float %FAbs232, %408
  %.i1403 = fmul fast float %FAbs233, %409
  %.i2404 = fmul fast float %FAbs234, %410
  %.i0405 = fadd fast float %.i0402, %.i0399
  %.i1406 = fadd fast float %.i1403, %.i1400
  %.i2407 = fadd fast float %.i2404, %.i2401
  %.i0411 = fsub fast float %.i0399, %.i0402
  %.i1412 = fsub fast float %.i1400, %.i1403
  %.i2413 = fsub fast float %.i2401, %.i2404
  %FMax264 = call float @dx.op.binary.f32(i32 35, float %.i0411, float %.i1412)
  %FMax263 = call float @dx.op.binary.f32(i32 35, float %FMax264, float %.i2413)
  %FMin262 = call float @dx.op.binary.f32(i32 36, float %.i0405, float %.i1406)
  %FMin261 = call float @dx.op.binary.f32(i32 36, float %FMin262, float %.i2407)
  %FMax = call float @dx.op.binary.f32(i32 35, float %FMax263, float 0.000000e+00)
  %FMin259 = call float @dx.op.binary.f32(i32 36, float %FMin261, float %RayTCurrent219)
  %412 = fcmp fast olt float %FMax, %FMin259
  %413 = and i1 %411, %412
  br i1 %413, label %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i", label %422

"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i":         ; preds = %374
  %414 = fcmp fast olt float %FMax, %FMax266
  %415 = select i1 %414, i32 %375, i32 %107
  %416 = select i1 %414, i32 %107, i32 %375
  %417 = shl i32 %stackPointer.i.2, 6
  %418 = add i32 %417, %14
  store i32 %415, i32 addrspace(3)* %93, align 4, !tbaa !221, !noalias !322
  %419 = getelementptr [2048 x i32], [2048 x i32] addrspace(3)* @"\01?stack@@3PAIA", i32 0, i32 %418
  store i32 %416, i32 addrspace(3)* %419, align 4, !tbaa !221, !noalias !322
  %420 = add nsw i32 %stackPointer.i.2, 1
  %421 = add i32 %89, 1
  store i32 %421, i32* %95, align 4, !tbaa !221
  br label %426

; <label>:422                                     ; preds = %374
  %423 = or i1 %411, %412
  br i1 %423, label %424, label %426

; <label>:424                                     ; preds = %422
  %425 = select i1 %412, i32 %107, i32 %375
  store i32 %425, i32 addrspace(3)* %93, align 4, !tbaa !221, !noalias !325
  store i32 %89, i32* %95, align 4, !tbaa !221
  br label %426

; <label>:426                                     ; preds = %424, %422, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i", %373, %370, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i", %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i", %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i189"
  %.3.i0 = phi float [ %.1.i0, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i" ], [ %.1.i0, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i189" ], [ %.2.i0, %373 ], [ %.2.i0, %370 ], [ %.2.i0, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i" ], [ %.1.i0, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i" ], [ %.1.i0, %424 ], [ %.1.i0, %422 ]
  %.3.i1 = phi float [ %.1.i1, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i" ], [ %.1.i1, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i189" ], [ %.2.i1, %373 ], [ %.2.i1, %370 ], [ %.2.i1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i" ], [ %.1.i1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i" ], [ %.1.i1, %424 ], [ %.1.i1, %422 ]
  %currentRayData.i.0.2.i0 = phi float [ %FMad283, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i" ], [ %currentRayData.i.0.1.i0, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i189" ], [ %currentRayData.i.0.1.i0, %373 ], [ %currentRayData.i.0.1.i0, %370 ], [ %currentRayData.i.0.1.i0, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i" ], [ %currentRayData.i.0.1.i0, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i" ], [ %currentRayData.i.0.1.i0, %424 ], [ %currentRayData.i.0.1.i0, %422 ]
  %currentRayData.i.0.2.i1 = phi float [ %FMad280, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i" ], [ %currentRayData.i.0.1.i1, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i189" ], [ %currentRayData.i.0.1.i1, %373 ], [ %currentRayData.i.0.1.i1, %370 ], [ %currentRayData.i.0.1.i1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i" ], [ %currentRayData.i.0.1.i1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i" ], [ %currentRayData.i.0.1.i1, %424 ], [ %currentRayData.i.0.1.i1, %422 ]
  %currentRayData.i.0.2.i2 = phi float [ %FMad, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i" ], [ %currentRayData.i.0.1.i2, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i189" ], [ %currentRayData.i.0.1.i2, %373 ], [ %currentRayData.i.0.1.i2, %370 ], [ %currentRayData.i.0.1.i2, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i" ], [ %currentRayData.i.0.1.i2, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i" ], [ %currentRayData.i.0.1.i2, %424 ], [ %currentRayData.i.0.1.i2, %422 ]
  %currentRayData.i.1.2.i0 = phi float [ %FMad292, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i" ], [ %currentRayData.i.1.1.i0, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i189" ], [ %currentRayData.i.1.1.i0, %373 ], [ %currentRayData.i.1.1.i0, %370 ], [ %currentRayData.i.1.1.i0, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i" ], [ %currentRayData.i.1.1.i0, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i" ], [ %currentRayData.i.1.1.i0, %424 ], [ %currentRayData.i.1.1.i0, %422 ]
  %currentRayData.i.1.2.i1 = phi float [ %FMad289, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i" ], [ %currentRayData.i.1.1.i1, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i189" ], [ %currentRayData.i.1.1.i1, %373 ], [ %currentRayData.i.1.1.i1, %370 ], [ %currentRayData.i.1.1.i1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i" ], [ %currentRayData.i.1.1.i1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i" ], [ %currentRayData.i.1.1.i1, %424 ], [ %currentRayData.i.1.1.i1, %422 ]
  %currentRayData.i.1.2.i2 = phi float [ %FMad286, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i" ], [ %currentRayData.i.1.1.i2, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i189" ], [ %currentRayData.i.1.1.i2, %373 ], [ %currentRayData.i.1.1.i2, %370 ], [ %currentRayData.i.1.1.i2, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i" ], [ %currentRayData.i.1.1.i2, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i" ], [ %currentRayData.i.1.1.i2, %424 ], [ %currentRayData.i.1.1.i2, %422 ]
  %currentRayData.i.2.2.i0 = phi float [ %.i0340, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i" ], [ %currentRayData.i.2.1.i0, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i189" ], [ %currentRayData.i.2.1.i0, %373 ], [ %currentRayData.i.2.1.i0, %370 ], [ %currentRayData.i.2.1.i0, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i" ], [ %currentRayData.i.2.1.i0, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i" ], [ %currentRayData.i.2.1.i0, %424 ], [ %currentRayData.i.2.1.i0, %422 ]
  %currentRayData.i.2.2.i1 = phi float [ %.i1341, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i" ], [ %currentRayData.i.2.1.i1, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i189" ], [ %currentRayData.i.2.1.i1, %373 ], [ %currentRayData.i.2.1.i1, %370 ], [ %currentRayData.i.2.1.i1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i" ], [ %currentRayData.i.2.1.i1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i" ], [ %currentRayData.i.2.1.i1, %424 ], [ %currentRayData.i.2.1.i1, %422 ]
  %currentRayData.i.2.2.i2 = phi float [ %.i2342, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i" ], [ %currentRayData.i.2.1.i2, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i189" ], [ %currentRayData.i.2.1.i2, %373 ], [ %currentRayData.i.2.1.i2, %370 ], [ %currentRayData.i.2.1.i2, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i" ], [ %currentRayData.i.2.1.i2, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i" ], [ %currentRayData.i.2.1.i2, %424 ], [ %currentRayData.i.2.1.i2, %422 ]
  %currentRayData.i.3.2.i0 = phi float [ %.i0343, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i" ], [ %currentRayData.i.3.1.i0, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i189" ], [ %currentRayData.i.3.1.i0, %373 ], [ %currentRayData.i.3.1.i0, %370 ], [ %currentRayData.i.3.1.i0, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i" ], [ %currentRayData.i.3.1.i0, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i" ], [ %currentRayData.i.3.1.i0, %424 ], [ %currentRayData.i.3.1.i0, %422 ]
  %currentRayData.i.3.2.i1 = phi float [ %.i1344, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i" ], [ %currentRayData.i.3.1.i1, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i189" ], [ %currentRayData.i.3.1.i1, %373 ], [ %currentRayData.i.3.1.i1, %370 ], [ %currentRayData.i.3.1.i1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i" ], [ %currentRayData.i.3.1.i1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i" ], [ %currentRayData.i.3.1.i1, %424 ], [ %currentRayData.i.3.1.i1, %422 ]
  %currentRayData.i.3.2.i2 = phi float [ %.i2345, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i" ], [ %currentRayData.i.3.1.i2, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i189" ], [ %currentRayData.i.3.1.i2, %373 ], [ %currentRayData.i.3.1.i2, %370 ], [ %currentRayData.i.3.1.i2, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i" ], [ %currentRayData.i.3.1.i2, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i" ], [ %currentRayData.i.3.1.i2, %424 ], [ %currentRayData.i.3.1.i2, %422 ]
  %currentRayData.i.4.2.i0 = phi float [ %208, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i" ], [ %currentRayData.i.4.1.i0, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i189" ], [ %currentRayData.i.4.1.i0, %373 ], [ %currentRayData.i.4.1.i0, %370 ], [ %currentRayData.i.4.1.i0, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i" ], [ %currentRayData.i.4.1.i0, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i" ], [ %currentRayData.i.4.1.i0, %424 ], [ %currentRayData.i.4.1.i0, %422 ]
  %currentRayData.i.4.2.i1 = phi float [ %211, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i" ], [ %currentRayData.i.4.1.i1, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i189" ], [ %currentRayData.i.4.1.i1, %373 ], [ %currentRayData.i.4.1.i1, %370 ], [ %currentRayData.i.4.1.i1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i" ], [ %currentRayData.i.4.1.i1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i" ], [ %currentRayData.i.4.1.i1, %424 ], [ %currentRayData.i.4.1.i1, %422 ]
  %currentRayData.i.4.2.i2 = phi float [ %212, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i" ], [ %currentRayData.i.4.1.i2, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i189" ], [ %currentRayData.i.4.1.i2, %373 ], [ %currentRayData.i.4.1.i2, %370 ], [ %currentRayData.i.4.1.i2, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i" ], [ %currentRayData.i.4.1.i2, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i" ], [ %currentRayData.i.4.1.i2, %424 ], [ %currentRayData.i.4.1.i2, %422 ]
  %currentRayData.i.5.2.i0 = phi i32 [ %.5.0.i0, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i" ], [ %currentRayData.i.5.1.i0, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i189" ], [ %currentRayData.i.5.1.i0, %373 ], [ %currentRayData.i.5.1.i0, %370 ], [ %currentRayData.i.5.1.i0, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i" ], [ %currentRayData.i.5.1.i0, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i" ], [ %currentRayData.i.5.1.i0, %424 ], [ %currentRayData.i.5.1.i0, %422 ]
  %currentRayData.i.5.2.i1 = phi i32 [ %.5.0.i1, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i" ], [ %currentRayData.i.5.1.i1, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i189" ], [ %currentRayData.i.5.1.i1, %373 ], [ %currentRayData.i.5.1.i1, %370 ], [ %currentRayData.i.5.1.i1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i" ], [ %currentRayData.i.5.1.i1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i" ], [ %currentRayData.i.5.1.i1, %424 ], [ %currentRayData.i.5.1.i1, %422 ]
  %currentRayData.i.5.2.i2 = phi i32 [ %.0172, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i" ], [ %currentRayData.i.5.1.i2, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i189" ], [ %currentRayData.i.5.1.i2, %373 ], [ %currentRayData.i.5.1.i2, %370 ], [ %currentRayData.i.5.1.i2, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i" ], [ %currentRayData.i.5.1.i2, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i" ], [ %currentRayData.i.5.1.i2, %424 ], [ %currentRayData.i.5.1.i2, %422 ]
  %CurrentObjectToWorld.i167.2.i0 = phi float [ %173, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i" ], [ %CurrentObjectToWorld.i167.1.i0, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i189" ], [ %CurrentObjectToWorld.i167.1.i0, %373 ], [ %CurrentObjectToWorld.i167.1.i0, %370 ], [ %CurrentObjectToWorld.i167.1.i0, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i" ], [ %CurrentObjectToWorld.i167.1.i0, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i" ], [ %CurrentObjectToWorld.i167.1.i0, %424 ], [ %CurrentObjectToWorld.i167.1.i0, %422 ]
  %CurrentObjectToWorld.i167.2.i1 = phi float [ %174, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i" ], [ %CurrentObjectToWorld.i167.1.i1, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i189" ], [ %CurrentObjectToWorld.i167.1.i1, %373 ], [ %CurrentObjectToWorld.i167.1.i1, %370 ], [ %CurrentObjectToWorld.i167.1.i1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i" ], [ %CurrentObjectToWorld.i167.1.i1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i" ], [ %CurrentObjectToWorld.i167.1.i1, %424 ], [ %CurrentObjectToWorld.i167.1.i1, %422 ]
  %CurrentObjectToWorld.i167.2.i2 = phi float [ %175, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i" ], [ %CurrentObjectToWorld.i167.1.i2, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i189" ], [ %CurrentObjectToWorld.i167.1.i2, %373 ], [ %CurrentObjectToWorld.i167.1.i2, %370 ], [ %CurrentObjectToWorld.i167.1.i2, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i" ], [ %CurrentObjectToWorld.i167.1.i2, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i" ], [ %CurrentObjectToWorld.i167.1.i2, %424 ], [ %CurrentObjectToWorld.i167.1.i2, %422 ]
  %CurrentObjectToWorld.i167.2.i3 = phi float [ %176, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i" ], [ %CurrentObjectToWorld.i167.1.i3, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i189" ], [ %CurrentObjectToWorld.i167.1.i3, %373 ], [ %CurrentObjectToWorld.i167.1.i3, %370 ], [ %CurrentObjectToWorld.i167.1.i3, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i" ], [ %CurrentObjectToWorld.i167.1.i3, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i" ], [ %CurrentObjectToWorld.i167.1.i3, %424 ], [ %CurrentObjectToWorld.i167.1.i3, %422 ]
  %CurrentObjectToWorld.i167.2.i4 = phi float [ %177, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i" ], [ %CurrentObjectToWorld.i167.1.i4, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i189" ], [ %CurrentObjectToWorld.i167.1.i4, %373 ], [ %CurrentObjectToWorld.i167.1.i4, %370 ], [ %CurrentObjectToWorld.i167.1.i4, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i" ], [ %CurrentObjectToWorld.i167.1.i4, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i" ], [ %CurrentObjectToWorld.i167.1.i4, %424 ], [ %CurrentObjectToWorld.i167.1.i4, %422 ]
  %CurrentObjectToWorld.i167.2.i5 = phi float [ %178, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i" ], [ %CurrentObjectToWorld.i167.1.i5, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i189" ], [ %CurrentObjectToWorld.i167.1.i5, %373 ], [ %CurrentObjectToWorld.i167.1.i5, %370 ], [ %CurrentObjectToWorld.i167.1.i5, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i" ], [ %CurrentObjectToWorld.i167.1.i5, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i" ], [ %CurrentObjectToWorld.i167.1.i5, %424 ], [ %CurrentObjectToWorld.i167.1.i5, %422 ]
  %CurrentObjectToWorld.i167.2.i6 = phi float [ %179, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i" ], [ %CurrentObjectToWorld.i167.1.i6, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i189" ], [ %CurrentObjectToWorld.i167.1.i6, %373 ], [ %CurrentObjectToWorld.i167.1.i6, %370 ], [ %CurrentObjectToWorld.i167.1.i6, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i" ], [ %CurrentObjectToWorld.i167.1.i6, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i" ], [ %CurrentObjectToWorld.i167.1.i6, %424 ], [ %CurrentObjectToWorld.i167.1.i6, %422 ]
  %CurrentObjectToWorld.i167.2.i7 = phi float [ %180, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i" ], [ %CurrentObjectToWorld.i167.1.i7, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i189" ], [ %CurrentObjectToWorld.i167.1.i7, %373 ], [ %CurrentObjectToWorld.i167.1.i7, %370 ], [ %CurrentObjectToWorld.i167.1.i7, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i" ], [ %CurrentObjectToWorld.i167.1.i7, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i" ], [ %CurrentObjectToWorld.i167.1.i7, %424 ], [ %CurrentObjectToWorld.i167.1.i7, %422 ]
  %CurrentObjectToWorld.i167.2.i8 = phi float [ %181, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i" ], [ %CurrentObjectToWorld.i167.1.i8, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i189" ], [ %CurrentObjectToWorld.i167.1.i8, %373 ], [ %CurrentObjectToWorld.i167.1.i8, %370 ], [ %CurrentObjectToWorld.i167.1.i8, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i" ], [ %CurrentObjectToWorld.i167.1.i8, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i" ], [ %CurrentObjectToWorld.i167.1.i8, %424 ], [ %CurrentObjectToWorld.i167.1.i8, %422 ]
  %CurrentObjectToWorld.i167.2.i9 = phi float [ %182, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i" ], [ %CurrentObjectToWorld.i167.1.i9, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i189" ], [ %CurrentObjectToWorld.i167.1.i9, %373 ], [ %CurrentObjectToWorld.i167.1.i9, %370 ], [ %CurrentObjectToWorld.i167.1.i9, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i" ], [ %CurrentObjectToWorld.i167.1.i9, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i" ], [ %CurrentObjectToWorld.i167.1.i9, %424 ], [ %CurrentObjectToWorld.i167.1.i9, %422 ]
  %CurrentObjectToWorld.i167.2.i10 = phi float [ %183, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i" ], [ %CurrentObjectToWorld.i167.1.i10, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i189" ], [ %CurrentObjectToWorld.i167.1.i10, %373 ], [ %CurrentObjectToWorld.i167.1.i10, %370 ], [ %CurrentObjectToWorld.i167.1.i10, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i" ], [ %CurrentObjectToWorld.i167.1.i10, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i" ], [ %CurrentObjectToWorld.i167.1.i10, %424 ], [ %CurrentObjectToWorld.i167.1.i10, %422 ]
  %CurrentObjectToWorld.i167.2.i11 = phi float [ %184, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i" ], [ %CurrentObjectToWorld.i167.1.i11, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i189" ], [ %CurrentObjectToWorld.i167.1.i11, %373 ], [ %CurrentObjectToWorld.i167.1.i11, %370 ], [ %CurrentObjectToWorld.i167.1.i11, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i" ], [ %CurrentObjectToWorld.i167.1.i11, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i" ], [ %CurrentObjectToWorld.i167.1.i11, %424 ], [ %CurrentObjectToWorld.i167.1.i11, %422 ]
  %CurrentWorldToObject.i168.2.i0 = phi float [ %156, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i" ], [ %CurrentWorldToObject.i168.1.i0, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i189" ], [ %CurrentWorldToObject.i168.1.i0, %373 ], [ %CurrentWorldToObject.i168.1.i0, %370 ], [ %CurrentWorldToObject.i168.1.i0, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i" ], [ %CurrentWorldToObject.i168.1.i0, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i" ], [ %CurrentWorldToObject.i168.1.i0, %424 ], [ %CurrentWorldToObject.i168.1.i0, %422 ]
  %CurrentWorldToObject.i168.2.i1 = phi float [ %157, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i" ], [ %CurrentWorldToObject.i168.1.i1, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i189" ], [ %CurrentWorldToObject.i168.1.i1, %373 ], [ %CurrentWorldToObject.i168.1.i1, %370 ], [ %CurrentWorldToObject.i168.1.i1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i" ], [ %CurrentWorldToObject.i168.1.i1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i" ], [ %CurrentWorldToObject.i168.1.i1, %424 ], [ %CurrentWorldToObject.i168.1.i1, %422 ]
  %CurrentWorldToObject.i168.2.i2 = phi float [ %158, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i" ], [ %CurrentWorldToObject.i168.1.i2, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i189" ], [ %CurrentWorldToObject.i168.1.i2, %373 ], [ %CurrentWorldToObject.i168.1.i2, %370 ], [ %CurrentWorldToObject.i168.1.i2, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i" ], [ %CurrentWorldToObject.i168.1.i2, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i" ], [ %CurrentWorldToObject.i168.1.i2, %424 ], [ %CurrentWorldToObject.i168.1.i2, %422 ]
  %CurrentWorldToObject.i168.2.i3 = phi float [ %159, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i" ], [ %CurrentWorldToObject.i168.1.i3, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i189" ], [ %CurrentWorldToObject.i168.1.i3, %373 ], [ %CurrentWorldToObject.i168.1.i3, %370 ], [ %CurrentWorldToObject.i168.1.i3, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i" ], [ %CurrentWorldToObject.i168.1.i3, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i" ], [ %CurrentWorldToObject.i168.1.i3, %424 ], [ %CurrentWorldToObject.i168.1.i3, %422 ]
  %CurrentWorldToObject.i168.2.i4 = phi float [ %160, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i" ], [ %CurrentWorldToObject.i168.1.i4, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i189" ], [ %CurrentWorldToObject.i168.1.i4, %373 ], [ %CurrentWorldToObject.i168.1.i4, %370 ], [ %CurrentWorldToObject.i168.1.i4, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i" ], [ %CurrentWorldToObject.i168.1.i4, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i" ], [ %CurrentWorldToObject.i168.1.i4, %424 ], [ %CurrentWorldToObject.i168.1.i4, %422 ]
  %CurrentWorldToObject.i168.2.i5 = phi float [ %161, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i" ], [ %CurrentWorldToObject.i168.1.i5, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i189" ], [ %CurrentWorldToObject.i168.1.i5, %373 ], [ %CurrentWorldToObject.i168.1.i5, %370 ], [ %CurrentWorldToObject.i168.1.i5, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i" ], [ %CurrentWorldToObject.i168.1.i5, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i" ], [ %CurrentWorldToObject.i168.1.i5, %424 ], [ %CurrentWorldToObject.i168.1.i5, %422 ]
  %CurrentWorldToObject.i168.2.i6 = phi float [ %162, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i" ], [ %CurrentWorldToObject.i168.1.i6, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i189" ], [ %CurrentWorldToObject.i168.1.i6, %373 ], [ %CurrentWorldToObject.i168.1.i6, %370 ], [ %CurrentWorldToObject.i168.1.i6, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i" ], [ %CurrentWorldToObject.i168.1.i6, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i" ], [ %CurrentWorldToObject.i168.1.i6, %424 ], [ %CurrentWorldToObject.i168.1.i6, %422 ]
  %CurrentWorldToObject.i168.2.i7 = phi float [ %163, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i" ], [ %CurrentWorldToObject.i168.1.i7, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i189" ], [ %CurrentWorldToObject.i168.1.i7, %373 ], [ %CurrentWorldToObject.i168.1.i7, %370 ], [ %CurrentWorldToObject.i168.1.i7, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i" ], [ %CurrentWorldToObject.i168.1.i7, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i" ], [ %CurrentWorldToObject.i168.1.i7, %424 ], [ %CurrentWorldToObject.i168.1.i7, %422 ]
  %CurrentWorldToObject.i168.2.i8 = phi float [ %164, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i" ], [ %CurrentWorldToObject.i168.1.i8, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i189" ], [ %CurrentWorldToObject.i168.1.i8, %373 ], [ %CurrentWorldToObject.i168.1.i8, %370 ], [ %CurrentWorldToObject.i168.1.i8, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i" ], [ %CurrentWorldToObject.i168.1.i8, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i" ], [ %CurrentWorldToObject.i168.1.i8, %424 ], [ %CurrentWorldToObject.i168.1.i8, %422 ]
  %CurrentWorldToObject.i168.2.i9 = phi float [ %165, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i" ], [ %CurrentWorldToObject.i168.1.i9, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i189" ], [ %CurrentWorldToObject.i168.1.i9, %373 ], [ %CurrentWorldToObject.i168.1.i9, %370 ], [ %CurrentWorldToObject.i168.1.i9, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i" ], [ %CurrentWorldToObject.i168.1.i9, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i" ], [ %CurrentWorldToObject.i168.1.i9, %424 ], [ %CurrentWorldToObject.i168.1.i9, %422 ]
  %CurrentWorldToObject.i168.2.i10 = phi float [ %166, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i" ], [ %CurrentWorldToObject.i168.1.i10, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i189" ], [ %CurrentWorldToObject.i168.1.i10, %373 ], [ %CurrentWorldToObject.i168.1.i10, %370 ], [ %CurrentWorldToObject.i168.1.i10, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i" ], [ %CurrentWorldToObject.i168.1.i10, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i" ], [ %CurrentWorldToObject.i168.1.i10, %424 ], [ %CurrentWorldToObject.i168.1.i10, %422 ]
  %CurrentWorldToObject.i168.2.i11 = phi float [ %167, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i" ], [ %CurrentWorldToObject.i168.1.i11, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i189" ], [ %CurrentWorldToObject.i168.1.i11, %373 ], [ %CurrentWorldToObject.i168.1.i11, %370 ], [ %CurrentWorldToObject.i168.1.i11, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i" ], [ %CurrentWorldToObject.i168.1.i11, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i" ], [ %CurrentWorldToObject.i168.1.i11, %424 ], [ %CurrentWorldToObject.i168.1.i11, %422 ]
  %ResetMatrices.i.3 = phi i32 [ %ResetMatrices.i.1, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i" ], [ %ResetMatrices.i.1, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i189" ], [ 0, %373 ], [ 0, %370 ], [ %ResetMatrices.i.1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i" ], [ %ResetMatrices.i.1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i" ], [ %ResetMatrices.i.1, %424 ], [ %ResetMatrices.i.1, %422 ]
  %currentBVHIndex.i.2 = phi i32 [ 1, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i" ], [ 0, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i189" ], [ %currentBVHIndex.i.1, %373 ], [ %currentBVHIndex.i.1, %370 ], [ %currentBVHIndex.i.1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i" ], [ %currentBVHIndex.i.1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i" ], [ %currentBVHIndex.i.1, %424 ], [ %currentBVHIndex.i.1, %422 ]
  %currentGpuVA.i.2.i0 = phi i32 [ %136, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i" ], [ %currentGpuVA.i.1.i0, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i189" ], [ %currentGpuVA.i.1.i0, %373 ], [ %currentGpuVA.i.1.i0, %370 ], [ %currentGpuVA.i.1.i0, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i" ], [ %currentGpuVA.i.1.i0, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i" ], [ %currentGpuVA.i.1.i0, %424 ], [ %currentGpuVA.i.1.i0, %422 ]
  %currentGpuVA.i.2.i1 = phi i32 [ %137, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i" ], [ %currentGpuVA.i.1.i1, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i189" ], [ %currentGpuVA.i.1.i1, %373 ], [ %currentGpuVA.i.1.i1, %370 ], [ %currentGpuVA.i.1.i1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i" ], [ %currentGpuVA.i.1.i1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i" ], [ %currentGpuVA.i.1.i1, %424 ], [ %currentGpuVA.i.1.i1, %422 ]
  %instIdx.i.2 = phi i32 [ %111, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i" ], [ %111, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i189" ], [ %instIdx.i.1, %373 ], [ %instIdx.i.1, %370 ], [ %instIdx.i.1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i" ], [ %instIdx.i.1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i" ], [ %instIdx.i.1, %424 ], [ %instIdx.i.1, %422 ]
  %instFlags.i.2 = phi i32 [ %186, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i" ], [ %instFlags.i.1, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i189" ], [ %instFlags.i.1, %373 ], [ %instFlags.i.1, %370 ], [ %instFlags.i.1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i" ], [ %instFlags.i.1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i" ], [ %instFlags.i.1, %424 ], [ %instFlags.i.1, %422 ]
  %instOffset.i.2 = phi i32 [ %168, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i" ], [ %168, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i189" ], [ %instOffset.i.1, %373 ], [ %instOffset.i.1, %370 ], [ %instOffset.i.1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i" ], [ %instOffset.i.1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i" ], [ %instOffset.i.1, %424 ], [ %instOffset.i.1, %422 ]
  %instId.i.2 = phi i32 [ %169, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i" ], [ %169, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i189" ], [ %instId.i.1, %373 ], [ %instId.i.1, %370 ], [ %instId.i.1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i" ], [ %instId.i.1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i" ], [ %instId.i.1, %424 ], [ %instId.i.1, %422 ]
  %stackPointer.i.3 = phi i32 [ %stackPointer.i.2, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i" ], [ %90, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i189" ], [ %90, %373 ], [ %90, %370 ], [ %90, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i" ], [ %420, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i" ], [ %stackPointer.i.2, %424 ], [ %90, %422 ]
  %resultBary.i.2.i0 = phi float [ %resultBary.i.1.i0, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i" ], [ %resultBary.i.1.i0, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i189" ], [ %.2.i0.resultBary.i.1.i0, %373 ], [ %.2.i0.resultBary.i.1.i0, %370 ], [ %.2.i0.resultBary.i.1.i0, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i" ], [ %resultBary.i.1.i0, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i" ], [ %resultBary.i.1.i0, %424 ], [ %resultBary.i.1.i0, %422 ]
  %resultBary.i.2.i1 = phi float [ %resultBary.i.1.i1, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i" ], [ %resultBary.i.1.i1, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i189" ], [ %.2.i1.resultBary.i.1.i1, %373 ], [ %.2.i1.resultBary.i.1.i1, %370 ], [ %.2.i1.resultBary.i.1.i1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i" ], [ %resultBary.i.1.i1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i" ], [ %resultBary.i.1.i1, %424 ], [ %resultBary.i.1.i1, %422 ]
  %resultTriId.i.2 = phi i32 [ %resultTriId.i.1, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i" ], [ %resultTriId.i.1, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i189" ], [ %.resultTriId.i.1, %373 ], [ %.resultTriId.i.1, %370 ], [ %.resultTriId.i.1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i" ], [ %resultTriId.i.1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i" ], [ %resultTriId.i.1, %424 ], [ %resultTriId.i.1, %422 ]
  %427 = getelementptr inbounds [2 x i32], [2 x i32]* %nodesToProcess.i, i32 0, i32 %currentBVHIndex.i.2
  %428 = load i32, i32* %427, align 4, !tbaa !221
  %429 = icmp eq i32 %428, 0
  br i1 %429, label %430, label %88

; <label>:430                                     ; preds = %426
  %431 = add i32 %currentBVHIndex.i.2, -1
  %432 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %AccelerationStructureList, i32 0)
  %433 = extractvalue %dx.types.CBufRet.i32 %432, 0
  %434 = extractvalue %dx.types.CBufRet.i32 %432, 1
  %435 = load i32, i32* %41, align 4, !tbaa !221
  %436 = icmp eq i32 %435, 0
  br i1 %436, label %._crit_edge, label %.lr.ph

._crit_edge:                                      ; preds = %430, %87
  %InstanceIndex = call i32 @dx.op.instanceIndex.i32(i32 142)
  %437 = icmp eq i32 %InstanceIndex, -1
  br i1 %437, label %449, label %438

; <label>:438                                     ; preds = %._crit_edge
  %RayFlags258 = call i32 @dx.op.rayFlags.i32(i32 144)
  %439 = and i32 %RayFlags258, 8
  %440 = icmp eq i32 %439, 0
  br i1 %440, label %441, label %456

; <label>:441                                     ; preds = %438
  %442 = call i32 @"\01?Fallback_GeometryIndex@@YAIXZ"() #3
  %443 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %Constants295, i32 0)
  %444 = extractvalue %dx.types.CBufRet.i32 %443, 2
  %445 = mul i32 %444, %442
  %446 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.ByteAddressBuffer(i32 160, %struct.ByteAddressBuffer %6)
  %447 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %446, i32 %445, i32 undef)
  %448 = extractvalue %dx.types.ResRet.i32 %447, 0
  br label %456

; <label>:449                                     ; preds = %._crit_edge
  %450 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %Constants295, i32 0)
  %451 = extractvalue %dx.types.CBufRet.i32 %450, 3
  %452 = mul i32 %451, %3
  %453 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.ByteAddressBuffer(i32 160, %struct.ByteAddressBuffer %5)
  %454 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %453, i32 %452, i32 undef)
  %455 = extractvalue %dx.types.ResRet.i32 %454, 0
  br label %456

; <label>:456                                     ; preds = %449, %441, %438
  %stateID.0 = phi i32 [ %448, %441 ], [ %455, %449 ], [ 0, %438 ]
  ret i32 %stateID.0
}

; Function Attrs: alwaysinline nounwind readnone
define void @"\01?LogNoData@@YAXI@Z"(i32) #6 {
  ret void
}

; Function Attrs: alwaysinline nounwind readnone
define i1 @"\01?IsLeaf@@YA_NV?$vector@I$01@@@Z"(<2 x i32>) #6 {
  %2 = extractelement <2 x i32> %0, i32 0
  %3 = icmp slt i32 %2, 0
  ret i1 %3
}

; Function Attrs: alwaysinline nounwind
define void @"\01?StoreBVHMetadataToRawData@@YAXURWByteAddressBuffer@@IUBVHMetadata@@@Z"(%struct.RWByteAddressBuffer* nocapture readonly, i32, %struct.BVHMetadata* nocapture readonly) #5 {
  %4 = getelementptr inbounds %struct.BVHMetadata, %struct.BVHMetadata* %2, i32 0, i32 0, i32 0, i32 0
  %5 = load <4 x float>, <4 x float>* %4, align 4, !tbaa !218
  %.i0 = extractelement <4 x float> %5, i32 0
  %.i1 = extractelement <4 x float> %5, i32 1
  %.i2 = extractelement <4 x float> %5, i32 2
  %.i3 = extractelement <4 x float> %5, i32 3
  %6 = bitcast float %.i0 to i32
  %7 = bitcast float %.i1 to i32
  %8 = bitcast float %.i2 to i32
  %9 = bitcast float %.i3 to i32
  %10 = getelementptr inbounds %struct.BVHMetadata, %struct.BVHMetadata* %2, i32 0, i32 0, i32 0, i32 1
  %11 = load <4 x float>, <4 x float>* %10, align 4, !tbaa !218
  %.i036 = extractelement <4 x float> %11, i32 0
  %.i138 = extractelement <4 x float> %11, i32 1
  %.i240 = extractelement <4 x float> %11, i32 2
  %.i342 = extractelement <4 x float> %11, i32 3
  %12 = bitcast float %.i036 to i32
  %13 = bitcast float %.i138 to i32
  %14 = bitcast float %.i240 to i32
  %15 = getelementptr inbounds %struct.BVHMetadata, %struct.BVHMetadata* %2, i32 0, i32 0, i32 0, i32 2
  %16 = load <4 x float>, <4 x float>* %15, align 4, !tbaa !218
  %.i044 = extractelement <4 x float> %16, i32 0
  %.i146 = extractelement <4 x float> %16, i32 1
  %.i248 = extractelement <4 x float> %16, i32 2
  %.i350 = extractelement <4 x float> %16, i32 3
  %17 = getelementptr inbounds %struct.BVHMetadata, %struct.BVHMetadata* %2, i32 0, i32 0, i32 1
  %18 = load i32, i32* %17, align 4, !tbaa !221
  %19 = getelementptr inbounds %struct.BVHMetadata, %struct.BVHMetadata* %2, i32 0, i32 0, i32 2
  %20 = load i32, i32* %19, align 4, !tbaa !221
  %21 = getelementptr inbounds %struct.BVHMetadata, %struct.BVHMetadata* %2, i32 0, i32 0, i32 3
  %22 = load <2 x i32>, <2 x i32>* %21, align 4, !tbaa !218
  %23 = extractelement <2 x i32> %22, i64 0
  %24 = extractelement <2 x i32> %22, i64 1
  %25 = getelementptr inbounds %struct.BVHMetadata, %struct.BVHMetadata* %2, i32 0, i32 1, i32 0
  %26 = load <4 x float>, <4 x float>* %25, align 4, !tbaa !218
  %.i052 = extractelement <4 x float> %26, i32 0
  %.i154 = extractelement <4 x float> %26, i32 1
  %.i256 = extractelement <4 x float> %26, i32 2
  %.i358 = extractelement <4 x float> %26, i32 3
  %27 = getelementptr inbounds %struct.BVHMetadata, %struct.BVHMetadata* %2, i32 0, i32 1, i32 1
  %28 = load <4 x float>, <4 x float>* %27, align 4, !tbaa !218
  %.i060 = extractelement <4 x float> %28, i32 0
  %.i162 = extractelement <4 x float> %28, i32 1
  %.i264 = extractelement <4 x float> %28, i32 2
  %.i366 = extractelement <4 x float> %28, i32 3
  %29 = getelementptr inbounds %struct.BVHMetadata, %struct.BVHMetadata* %2, i32 0, i32 1, i32 2
  %30 = load <4 x float>, <4 x float>* %29, align 4, !tbaa !218
  %.i068 = extractelement <4 x float> %30, i32 0
  %.i170 = extractelement <4 x float> %30, i32 1
  %.i272 = extractelement <4 x float> %30, i32 2
  %.i374 = extractelement <4 x float> %30, i32 3
  %31 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %0, align 4
  %32 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %31)
  call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %32, i32 %1, i32 undef, i32 %6, i32 %7, i32 %8, i32 %9, i8 15)
  %33 = bitcast float %.i342 to i32
  %34 = add i32 %1, 16
  %35 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %0, align 4
  %36 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %35)
  call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %36, i32 %34, i32 undef, i32 %12, i32 %13, i32 %14, i32 %33, i8 15)
  %37 = bitcast float %.i044 to i32
  %38 = bitcast float %.i146 to i32
  %39 = bitcast float %.i248 to i32
  %40 = bitcast float %.i350 to i32
  %41 = add i32 %1, 32
  %42 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %0, align 4
  %43 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %42)
  call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %43, i32 %41, i32 undef, i32 %37, i32 %38, i32 %39, i32 %40, i8 15)
  %44 = add i32 %1, 48
  %45 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %0, align 4
  %46 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %45)
  call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %46, i32 %44, i32 undef, i32 %18, i32 %20, i32 %23, i32 %24, i8 15)
  %47 = bitcast float %.i052 to i32
  %48 = bitcast float %.i154 to i32
  %49 = bitcast float %.i256 to i32
  %50 = bitcast float %.i358 to i32
  %51 = add i32 %1, 64
  %52 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %0, align 4
  %53 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %52)
  call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %53, i32 %51, i32 undef, i32 %47, i32 %48, i32 %49, i32 %50, i8 15)
  %54 = bitcast float %.i060 to i32
  %55 = bitcast float %.i162 to i32
  %56 = bitcast float %.i264 to i32
  %57 = bitcast float %.i366 to i32
  %58 = add i32 %1, 80
  %59 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %0, align 4
  %60 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %59)
  call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %60, i32 %58, i32 undef, i32 %54, i32 %55, i32 %56, i32 %57, i8 15)
  %61 = bitcast float %.i068 to i32
  %62 = bitcast float %.i170 to i32
  %63 = bitcast float %.i272 to i32
  %64 = bitcast float %.i374 to i32
  %65 = add i32 %1, 96
  %66 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %0, align 4
  %67 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %66)
  call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %67, i32 %65, i32 undef, i32 %61, i32 %62, i32 %63, i32 %64, i8 15)
  ret void
}

; Function Attrs: alwaysinline nounwind readnone
define <3 x float> @"\01?RayPlaneIntersection@@YA?AV?$vector@M$02@@V1@000@Z"(<3 x float>, <3 x float>, <3 x float>, <3 x float>) #6 {
  %rayDirection.i0 = extractelement <3 x float> %3, i32 0
  %rayDirection.i1 = extractelement <3 x float> %3, i32 1
  %rayDirection.i2 = extractelement <3 x float> %3, i32 2
  %planeNormal.i0 = extractelement <3 x float> %1, i32 0
  %planeNormal.i1 = extractelement <3 x float> %1, i32 1
  %planeNormal.i2 = extractelement <3 x float> %1, i32 2
  %planeOrigin.i0 = extractelement <3 x float> %0, i32 0
  %rayOrigin.i0 = extractelement <3 x float> %2, i32 0
  %planeOrigin.i1 = extractelement <3 x float> %0, i32 1
  %rayOrigin.i1 = extractelement <3 x float> %2, i32 1
  %planeOrigin.i2 = extractelement <3 x float> %0, i32 2
  %rayOrigin.i2 = extractelement <3 x float> %2, i32 2
  %.i0 = fsub fast float %rayOrigin.i0, %planeOrigin.i0
  %.i1 = fsub fast float %rayOrigin.i1, %planeOrigin.i1
  %.i2 = fsub fast float %rayOrigin.i2, %planeOrigin.i2
  %.i01 = fsub fast float -0.000000e+00, %planeNormal.i0
  %.i12 = fsub fast float -0.000000e+00, %planeNormal.i1
  %.i23 = fsub fast float -0.000000e+00, %planeNormal.i2
  %5 = call float @dx.op.dot3.f32(i32 55, float %.i01, float %.i12, float %.i23, float %.i0, float %.i1, float %.i2)
  %6 = extractelement <3 x float> %1, i64 0
  %7 = extractelement <3 x float> %1, i64 1
  %8 = extractelement <3 x float> %1, i64 2
  %9 = extractelement <3 x float> %3, i64 0
  %10 = extractelement <3 x float> %3, i64 1
  %11 = extractelement <3 x float> %3, i64 2
  %12 = call float @dx.op.dot3.f32(i32 55, float %6, float %7, float %8, float %9, float %10, float %11)
  %13 = fdiv fast float %5, %12
  %.i04 = fmul fast float %13, %rayDirection.i0
  %.i15 = fmul fast float %13, %rayDirection.i1
  %.i26 = fmul fast float %13, %rayDirection.i2
  %.i07 = fadd fast float %.i04, %rayOrigin.i0
  %.i18 = fadd fast float %.i15, %rayOrigin.i1
  %.i29 = fadd fast float %.i26, %rayOrigin.i2
  %.upto012 = insertelement <3 x float> undef, float %.i07, i32 0
  %.upto113 = insertelement <3 x float> %.upto012, float %.i18, i32 1
  %14 = insertelement <3 x float> %.upto113, float %.i29, i32 2
  ret <3 x float> %14
}

; Function Attrs: alwaysinline nounwind
define void @"\01?FSchlick@@YAXAIAV?$vector@M$02@@0V1@1@Z"(<3 x float>* noalias nocapture dereferenceable(12), <3 x float>* noalias nocapture dereferenceable(12), <3 x float>, <3 x float>) #5 {
  %5 = extractelement <3 x float> %2, i64 0
  %6 = extractelement <3 x float> %2, i64 1
  %7 = extractelement <3 x float> %2, i64 2
  %8 = extractelement <3 x float> %3, i64 0
  %9 = extractelement <3 x float> %3, i64 1
  %10 = extractelement <3 x float> %3, i64 2
  %11 = call float @dx.op.dot3.f32(i32 55, float %5, float %6, float %7, float %8, float %9, float %10)
  %Saturate = call float @dx.op.unary.f32(i32 7, float %11)
  %12 = fsub fast float 1.000000e+00, %Saturate
  %Log = call float @dx.op.unary.f32(i32 23, float %12)
  %13 = fmul fast float %Log, 5.000000e+00
  %Exp = call float @dx.op.unary.f32(i32 21, float %13)
  %14 = load <3 x float>, <3 x float>* %0, align 4, !tbaa !218
  %.i0 = extractelement <3 x float> %14, i32 0
  %.i01 = fsub fast float 1.000000e+00, %.i0
  %.i1 = extractelement <3 x float> %14, i32 1
  %.i12 = fsub fast float 1.000000e+00, %.i1
  %.i2 = extractelement <3 x float> %14, i32 2
  %.i23 = fsub fast float 1.000000e+00, %.i2
  %.i04 = fmul fast float %.i01, %Exp
  %.i15 = fmul fast float %.i12, %Exp
  %.i26 = fmul fast float %.i23, %Exp
  %.i07 = fadd fast float %.i04, %.i0
  %.i18 = fadd fast float %.i15, %.i1
  %.i29 = fadd fast float %.i26, %.i2
  %.upto0 = insertelement <3 x float> undef, float %.i07, i32 0
  %.upto1 = insertelement <3 x float> %.upto0, float %.i18, i32 1
  %15 = insertelement <3 x float> %.upto1, float %.i29, i32 2
  store <3 x float> %15, <3 x float>* %0, align 4, !tbaa !218
  %16 = load <3 x float>, <3 x float>* %1, align 4, !tbaa !218
  %.i010 = extractelement <3 x float> %16, i32 0
  %.i112 = extractelement <3 x float> %16, i32 1
  %.i214 = extractelement <3 x float> %16, i32 2
  %17 = fmul fast float %.i010, %Exp
  %18 = fmul fast float %.i112, %Exp
  %19 = fmul fast float %.i214, %Exp
  %.i019 = fsub fast float %.i010, %17
  %.i120 = fsub fast float %.i112, %18
  %.i221 = fsub fast float %.i214, %19
  %.upto022 = insertelement <3 x float> undef, float %.i019, i32 0
  %.upto123 = insertelement <3 x float> %.upto022, float %.i120, i32 1
  %20 = insertelement <3 x float> %.upto123, float %.i221, i32 2
  store <3 x float> %20, <3 x float>* %1, align 4, !tbaa !218
  ret void
}

; Function Attrs: alwaysinline nounwind
define void @"\01?RawDataToBoundingBox@@YA?AUBoundingBox@@V?$vector@H$03@@0AIAV?$vector@I$01@@@Z"(%struct.BoundingBox* noalias nocapture sret, <4 x i32>, <4 x i32>, <2 x i32>* noalias nocapture dereferenceable(8)) #5 {
  %5 = extractelement <4 x i32> %1, i32 0
  %6 = bitcast i32 %5 to float
  %7 = insertelement <3 x float> undef, float %6, i32 0
  %8 = extractelement <4 x i32> %1, i32 1
  %9 = bitcast i32 %8 to float
  %10 = insertelement <3 x float> %7, float %9, i32 1
  %11 = extractelement <4 x i32> %1, i32 2
  %12 = bitcast i32 %11 to float
  %13 = insertelement <3 x float> %10, float %12, i32 2
  %14 = extractelement <4 x i32> %2, i32 0
  %15 = bitcast i32 %14 to float
  %16 = insertelement <3 x float> undef, float %15, i32 0
  %17 = extractelement <4 x i32> %2, i32 1
  %18 = bitcast i32 %17 to float
  %19 = insertelement <3 x float> %16, float %18, i32 1
  %20 = extractelement <4 x i32> %2, i32 2
  %21 = bitcast i32 %20 to float
  %22 = insertelement <3 x float> %19, float %21, i32 2
  %.upto0 = insertelement <2 x i32> undef, i32 %25, i32 0
  %23 = insertelement <2 x i32> %.upto0, i32 %24, i32 1
  %24 = extractelement <4 x i32> %2, i32 3
  %25 = extractelement <4 x i32> %1, i32 3
  store <2 x i32> %23, <2 x i32>* %3, align 4, !tbaa !218
  %26 = getelementptr inbounds %struct.BoundingBox, %struct.BoundingBox* %0, i32 0, i32 0
  store <3 x float> %13, <3 x float>* %26, align 4
  %27 = getelementptr inbounds %struct.BoundingBox, %struct.BoundingBox* %0, i32 0, i32 1
  store <3 x float> %22, <3 x float>* %27, align 4
  ret void
}

; Function Attrs: alwaysinline nounwind
define void @"\01?TransformAABB@@YA?AUAABB@@U1@V?$matrix@M$02$03@@@Z"(%struct.AABB* noalias nocapture sret, %struct.AABB* nocapture readonly, %class.matrix.float.3.4) #5 {
  %4 = extractvalue %class.matrix.float.3.4 %2, 0, 0
  %5 = extractelement <4 x float> %4, i32 3
  %6 = extractelement <4 x float> %4, i32 2
  %7 = extractelement <4 x float> %4, i32 1
  %8 = extractelement <4 x float> %4, i32 0
  %9 = extractvalue %class.matrix.float.3.4 %2, 0, 1
  %10 = extractelement <4 x float> %9, i32 3
  %11 = extractelement <4 x float> %9, i32 2
  %12 = extractelement <4 x float> %9, i32 1
  %13 = extractelement <4 x float> %9, i32 0
  %14 = extractvalue %class.matrix.float.3.4 %2, 0, 2
  %15 = extractelement <4 x float> %14, i32 3
  %16 = extractelement <4 x float> %14, i32 2
  %17 = extractelement <4 x float> %14, i32 1
  %18 = extractelement <4 x float> %14, i32 0
  %boxVertices.0 = alloca [8 x float], align 4
  %boxVertices.1 = alloca [8 x float], align 4
  %boxVertices.2 = alloca [8 x float], align 4
  %19 = getelementptr inbounds %struct.AABB, %struct.AABB* %1, i32 0, i32 0
  %20 = load <3 x float>, <3 x float>* %19, align 4
  %21 = extractelement <3 x float> %20, i64 0
  %22 = extractelement <3 x float> %20, i64 1
  %23 = extractelement <3 x float> %20, i64 2
  %24 = getelementptr [8 x float], [8 x float]* %boxVertices.0, i32 0, i32 0
  %25 = getelementptr [8 x float], [8 x float]* %boxVertices.1, i32 0, i32 0
  %26 = getelementptr [8 x float], [8 x float]* %boxVertices.2, i32 0, i32 0
  store float %21, float* %24, align 4
  store float %22, float* %25, align 4
  store float %23, float* %26, align 4
  %27 = extractelement <3 x float> %20, i32 1
  %28 = extractelement <3 x float> %20, i32 0
  %29 = getelementptr inbounds %struct.AABB, %struct.AABB* %1, i32 0, i32 1
  %30 = load <3 x float>, <3 x float>* %29, align 4
  %31 = extractelement <3 x float> %30, i32 2
  %32 = getelementptr [8 x float], [8 x float]* %boxVertices.0, i32 0, i32 1
  %33 = getelementptr [8 x float], [8 x float]* %boxVertices.1, i32 0, i32 1
  %34 = getelementptr [8 x float], [8 x float]* %boxVertices.2, i32 0, i32 1
  store float %28, float* %32, align 4
  store float %27, float* %33, align 4
  store float %31, float* %34, align 4
  %35 = extractelement <3 x float> %30, i32 1
  %36 = getelementptr [8 x float], [8 x float]* %boxVertices.0, i32 0, i32 2
  %37 = getelementptr [8 x float], [8 x float]* %boxVertices.1, i32 0, i32 2
  %38 = getelementptr [8 x float], [8 x float]* %boxVertices.2, i32 0, i32 2
  store float %28, float* %36, align 4
  store float %35, float* %37, align 4
  store float %31, float* %38, align 4
  %39 = extractelement <3 x float> %20, i32 2
  %40 = getelementptr [8 x float], [8 x float]* %boxVertices.0, i32 0, i32 3
  %41 = getelementptr [8 x float], [8 x float]* %boxVertices.1, i32 0, i32 3
  %42 = getelementptr [8 x float], [8 x float]* %boxVertices.2, i32 0, i32 3
  store float %28, float* %40, align 4
  store float %35, float* %41, align 4
  store float %39, float* %42, align 4
  %43 = extractelement <3 x float> %30, i32 0
  %44 = getelementptr [8 x float], [8 x float]* %boxVertices.0, i32 0, i32 4
  %45 = getelementptr [8 x float], [8 x float]* %boxVertices.1, i32 0, i32 4
  %46 = getelementptr [8 x float], [8 x float]* %boxVertices.2, i32 0, i32 4
  store float %43, float* %44, align 4
  store float %27, float* %45, align 4
  store float %39, float* %46, align 4
  %47 = getelementptr [8 x float], [8 x float]* %boxVertices.0, i32 0, i32 6
  %48 = getelementptr [8 x float], [8 x float]* %boxVertices.1, i32 0, i32 6
  %49 = getelementptr [8 x float], [8 x float]* %boxVertices.2, i32 0, i32 6
  store float %43, float* %47, align 4
  store float %27, float* %48, align 4
  store float %31, float* %49, align 4
  %50 = getelementptr [8 x float], [8 x float]* %boxVertices.0, i32 0, i32 5
  %51 = getelementptr [8 x float], [8 x float]* %boxVertices.1, i32 0, i32 5
  %52 = getelementptr [8 x float], [8 x float]* %boxVertices.2, i32 0, i32 5
  store float %43, float* %50, align 4
  store float %35, float* %51, align 4
  store float %39, float* %52, align 4
  %53 = extractelement <3 x float> %30, i64 0
  %54 = extractelement <3 x float> %30, i64 1
  %55 = extractelement <3 x float> %30, i64 2
  %56 = getelementptr [8 x float], [8 x float]* %boxVertices.0, i32 0, i32 7
  %57 = getelementptr [8 x float], [8 x float]* %boxVertices.1, i32 0, i32 7
  %58 = getelementptr [8 x float], [8 x float]* %boxVertices.2, i32 0, i32 7
  store float %53, float* %56, align 4
  store float %54, float* %57, align 4
  store float %55, float* %58, align 4
  br label %59

; <label>:59                                      ; preds = %._crit_edge, %3
  %load5 = phi float [ 1.000000e+00, %3 ], [ %load5.pre, %._crit_edge ]
  %load3 = phi float [ %23, %3 ], [ %load3.pre, %._crit_edge ]
  %load1 = phi float [ %22, %3 ], [ %load1.pre, %._crit_edge ]
  %load = phi float [ %21, %3 ], [ %load.pre, %._crit_edge ]
  %.0.09.i0 = phi float [ 0x47EFFFFFE0000000, %3 ], [ %FMin, %._crit_edge ]
  %.0.09.i1 = phi float [ 0x47EFFFFFE0000000, %3 ], [ %FMin12, %._crit_edge ]
  %.0.09.i2 = phi float [ 0x47EFFFFFE0000000, %3 ], [ %FMin13, %._crit_edge ]
  %.1.08.i0 = phi float [ 0xC7EFFFFFE0000000, %3 ], [ %FMax, %._crit_edge ]
  %.1.08.i1 = phi float [ 0xC7EFFFFFE0000000, %3 ], [ %FMax10, %._crit_edge ]
  %.1.08.i2 = phi float [ 0xC7EFFFFFE0000000, %3 ], [ %FMax11, %._crit_edge ]
  %i.07 = phi i32 [ 0, %3 ], [ %63, %._crit_edge ]
  %60 = fmul fast float %load, %8
  %FMad22 = call float @dx.op.tertiary.f32(i32 46, float %load1, float %7, float %60)
  %FMad21 = call float @dx.op.tertiary.f32(i32 46, float %load3, float %6, float %FMad22)
  %FMad20 = call float @dx.op.tertiary.f32(i32 46, float %load5, float %5, float %FMad21)
  %61 = fmul fast float %load, %13
  %FMad19 = call float @dx.op.tertiary.f32(i32 46, float %load1, float %12, float %61)
  %FMad18 = call float @dx.op.tertiary.f32(i32 46, float %load3, float %11, float %FMad19)
  %FMad17 = call float @dx.op.tertiary.f32(i32 46, float %load5, float %10, float %FMad18)
  %62 = fmul fast float %load, %18
  %FMad16 = call float @dx.op.tertiary.f32(i32 46, float %load1, float %17, float %62)
  %FMad15 = call float @dx.op.tertiary.f32(i32 46, float %load3, float %16, float %FMad16)
  %FMad = call float @dx.op.tertiary.f32(i32 46, float %load5, float %15, float %FMad15)
  %FMin = call float @dx.op.binary.f32(i32 36, float %.0.09.i0, float %FMad20)
  %FMin12 = call float @dx.op.binary.f32(i32 36, float %.0.09.i1, float %FMad17)
  %FMin13 = call float @dx.op.binary.f32(i32 36, float %.0.09.i2, float %FMad)
  %FMax = call float @dx.op.binary.f32(i32 35, float %.1.08.i0, float %FMad20)
  %FMax10 = call float @dx.op.binary.f32(i32 35, float %.1.08.i1, float %FMad17)
  %FMax11 = call float @dx.op.binary.f32(i32 35, float %.1.08.i2, float %FMad)
  %63 = add nuw nsw i32 %i.07, 1
  %exitcond = icmp eq i32 %63, 8
  br i1 %exitcond, label %64, label %._crit_edge

._crit_edge:                                      ; preds = %59
  %.phi.trans.insert = getelementptr [8 x float], [8 x float]* %boxVertices.0, i32 0, i32 %63
  %load.pre = load float, float* %.phi.trans.insert, align 4
  %.phi.trans.insert32 = getelementptr [8 x float], [8 x float]* %boxVertices.1, i32 0, i32 %63
  %load1.pre = load float, float* %.phi.trans.insert32, align 4
  %.phi.trans.insert34 = getelementptr [8 x float], [8 x float]* %boxVertices.2, i32 0, i32 %63
  %load3.pre = load float, float* %.phi.trans.insert34, align 4
  %.phi.trans.insert36 = getelementptr [8 x float], [8 x float]* @"3boxVertices.3.hca", i32 0, i32 %63
  %load5.pre = load float, float* %.phi.trans.insert36, align 4
  br label %59

; <label>:64                                      ; preds = %59
  %65 = insertelement <3 x float> undef, float %FMax, i64 0
  %66 = insertelement <3 x float> %65, float %FMax10, i64 1
  %67 = insertelement <3 x float> %66, float %FMax11, i64 2
  %68 = insertelement <3 x float> undef, float %FMin, i64 0
  %69 = insertelement <3 x float> %68, float %FMin12, i64 1
  %70 = insertelement <3 x float> %69, float %FMin13, i64 2
  %71 = getelementptr inbounds %struct.AABB, %struct.AABB* %0, i32 0, i32 0
  store <3 x float> %70, <3 x float>* %71, align 4
  %72 = getelementptr inbounds %struct.AABB, %struct.AABB* %0, i32 0, i32 1
  store <3 x float> %67, <3 x float>* %72, align 4
  ret void
}

; Function Attrs: alwaysinline nounwind readnone
define i32 @"\01?PointerGetBufferStartOffset@@YAIV?$vector@I$01@@@Z"(<2 x i32>) #6 {
  %2 = extractelement <2 x i32> %0, i32 0
  ret i32 %2
}

; Function Attrs: alwaysinline nounwind
define void @"\01?AntiAliasSpecular@@YAXAIAV?$vector@M$02@@AIAM@Z"(<3 x float>* noalias nocapture dereferenceable(12), float* noalias nocapture dereferenceable(4)) #5 {
  %3 = load <3 x float>, <3 x float>* %0, align 4, !tbaa !218
  %4 = extractelement <3 x float> %3, i64 0
  %5 = extractelement <3 x float> %3, i64 1
  %6 = extractelement <3 x float> %3, i64 2
  %7 = call float @dx.op.dot3.f32(i32 55, float %4, float %5, float %6, float %4, float %5, float %6)
  %Rsqrt = call float @dx.op.unary.f32(i32 25, float %7)
  %.i0 = extractelement <3 x float> %3, i32 0
  %.i01 = fmul fast float %.i0, %Rsqrt
  %.i1 = extractelement <3 x float> %3, i32 1
  %.i12 = fmul fast float %.i1, %Rsqrt
  %.i2 = extractelement <3 x float> %3, i32 2
  %.i23 = fmul fast float %.i2, %Rsqrt
  %.upto0 = insertelement <3 x float> undef, float %.i01, i32 0
  %.upto1 = insertelement <3 x float> %.upto0, float %.i12, i32 1
  %8 = insertelement <3 x float> %.upto1, float %.i23, i32 2
  store <3 x float> %8, <3 x float>* %0, align 4, !tbaa !218
  %9 = fdiv fast float 1.000000e+00, %Rsqrt
  %10 = load float, float* %1, align 4, !tbaa !231
  %11 = fadd fast float %10, -1.000000e+00
  %12 = fmul fast float %11, %9
  %13 = fadd fast float %12, 1.000000e+00
  store float %13, float* %1, align 4, !tbaa !231
  ret void
}

; Function Attrs: alwaysinline nounwind readnone
define %class.matrix.float.3.4 @"\01?InverseAffineTransform@@YA?AV?$matrix@M$02$03@@V1@@Z"(%class.matrix.float.3.4) #6 {
  %2 = extractvalue %class.matrix.float.3.4 %0, 0, 0
  %3 = extractelement <4 x float> %2, i32 3
  %4 = extractelement <4 x float> %2, i32 2
  %5 = extractelement <4 x float> %2, i32 1
  %6 = extractelement <4 x float> %2, i32 0
  %7 = extractvalue %class.matrix.float.3.4 %0, 0, 1
  %8 = extractelement <4 x float> %7, i32 3
  %9 = extractelement <4 x float> %7, i32 2
  %10 = extractelement <4 x float> %7, i32 1
  %11 = extractelement <4 x float> %7, i32 0
  %12 = extractvalue %class.matrix.float.3.4 %0, 0, 2
  %13 = extractelement <4 x float> %12, i32 3
  %14 = extractelement <4 x float> %12, i32 2
  %15 = extractelement <4 x float> %12, i32 1
  %16 = extractelement <4 x float> %12, i32 0
  %17 = fmul fast float %6, %10
  %18 = fmul fast float %17, %14
  %19 = fmul fast float %6, %15
  %20 = fmul fast float %19, %9
  %21 = fsub fast float %18, %20
  %22 = fmul fast float %11, %5
  %23 = fmul fast float %22, %14
  %24 = fsub fast float %21, %23
  %25 = fmul fast float %11, %15
  %26 = fmul fast float %25, %4
  %27 = fadd fast float %24, %26
  %28 = fmul fast float %16, %5
  %29 = fmul fast float %28, %9
  %30 = fadd fast float %27, %29
  %31 = fmul fast float %16, %10
  %32 = fmul fast float %31, %4
  %33 = fsub fast float %30, %32
  %34 = fdiv fast float 1.000000e+00, %33
  %35 = fmul fast float %10, %14
  %36 = fmul fast float %9, %15
  %37 = fsub fast float %35, %36
  %38 = fmul fast float %37, %34
  %39 = fmul fast float %9, %16
  %40 = fmul fast float %11, %14
  %41 = fsub fast float %39, %40
  %42 = fmul fast float %41, %34
  %43 = fsub fast float %25, %31
  %44 = fmul fast float %43, %34
  %45 = fmul fast float %15, %4
  %46 = fmul fast float %14, %5
  %47 = fsub fast float %45, %46
  %48 = fmul fast float %47, %34
  %49 = fmul fast float %14, %6
  %50 = fmul fast float %16, %4
  %51 = fsub fast float %49, %50
  %52 = fmul fast float %51, %34
  %53 = fsub fast float %28, %19
  %54 = fmul fast float %53, %34
  %55 = fmul fast float %5, %9
  %56 = fmul fast float %4, %10
  %57 = fsub fast float %55, %56
  %58 = fmul fast float %57, %34
  %59 = fmul fast float %4, %11
  %60 = fmul fast float %6, %9
  %61 = fsub fast float %59, %60
  %62 = fmul fast float %61, %34
  %63 = fsub fast float %17, %22
  %64 = fmul fast float %63, %34
  %65 = fmul fast float %14, %8
  %66 = fmul fast float %9, %13
  %67 = fsub fast float %65, %66
  %68 = fmul fast float %67, %5
  %69 = fmul fast float %4, %13
  %70 = fmul fast float %14, %3
  %71 = fsub fast float %69, %70
  %72 = fmul fast float %71, %10
  %73 = fadd fast float %68, %72
  %74 = fmul fast float %9, %3
  %75 = fmul fast float %4, %8
  %76 = fsub fast float %74, %75
  %77 = fmul fast float %76, %15
  %78 = fadd fast float %73, %77
  %79 = fmul fast float %78, %34
  %80 = fmul fast float %16, %8
  %81 = fmul fast float %11, %13
  %82 = fsub fast float %80, %81
  %83 = fmul fast float %82, %4
  %84 = fmul fast float %6, %13
  %85 = fmul fast float %16, %3
  %86 = fsub fast float %84, %85
  %87 = fmul fast float %86, %9
  %88 = fadd fast float %83, %87
  %89 = fmul fast float %11, %3
  %90 = fmul fast float %6, %8
  %91 = fsub fast float %89, %90
  %92 = fmul fast float %91, %14
  %93 = fadd fast float %88, %92
  %94 = fmul fast float %93, %34
  %95 = fsub fast float %31, %25
  %96 = fmul fast float %95, %3
  %97 = fsub fast float %19, %28
  %98 = fmul fast float %97, %8
  %99 = fadd fast float %96, %98
  %100 = fsub fast float %22, %17
  %101 = fmul fast float %100, %13
  %102 = fadd fast float %99, %101
  %103 = fmul fast float %102, %34
  %.upto025 = insertelement <4 x float> undef, float %38, i32 0
  %.upto126 = insertelement <4 x float> %.upto025, float %48, i32 1
  %.upto227 = insertelement <4 x float> %.upto126, float %58, i32 2
  %104 = insertelement <4 x float> %.upto227, float %79, i32 3
  %105 = insertvalue %class.matrix.float.3.4 undef, <4 x float> %104, 0, 0
  %.upto029 = insertelement <4 x float> undef, float %42, i32 0
  %.upto130 = insertelement <4 x float> %.upto029, float %52, i32 1
  %.upto231 = insertelement <4 x float> %.upto130, float %62, i32 2
  %106 = insertelement <4 x float> %.upto231, float %94, i32 3
  %107 = insertvalue %class.matrix.float.3.4 %105, <4 x float> %106, 0, 1
  %.upto033 = insertelement <4 x float> undef, float %44, i32 0
  %.upto134 = insertelement <4 x float> %.upto033, float %54, i32 1
  %.upto235 = insertelement <4 x float> %.upto134, float %64, i32 2
  %108 = insertelement <4 x float> %.upto235, float %103, i32 3
  %109 = insertvalue %class.matrix.float.3.4 %107, <4 x float> %108, 0, 2
  ret %class.matrix.float.3.4 %109
}

; Function Attrs: alwaysinline nounwind readonly
define float @"\01?GetShadow@@YAMV?$vector@M$02@@@Z"(<3 x float>) #4 {
  %2 = load %struct.SamplerComparisonState, %struct.SamplerComparisonState* @"\01?shadowSampler@@3USamplerComparisonState@@A", align 4
  %3 = load %"class.Texture2D<float>", %"class.Texture2D<float>"* @"\01?texShadow@@3V?$Texture2D@M@@A", align 4
  %4 = load %HitShaderConstants, %HitShaderConstants* @HitShaderConstants, align 4
  %HitShaderConstants = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.HitShaderConstants(i32 160, %HitShaderConstants %4)
  %5 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %HitShaderConstants, i32 3)
  %6 = extractvalue %dx.types.CBufRet.f32 %5, 0
  %7 = fmul fast float %6, 2.500000e-01
  %8 = fmul fast float %6, 1.750000e+00
  %9 = fmul fast float %6, 1.250000e+00
  %10 = fmul fast float %6, 7.500000e-01
  %11 = extractelement <3 x float> %0, i32 2
  %12 = extractelement <3 x float> %0, i32 1
  %13 = extractelement <3 x float> %0, i32 0
  %14 = call %dx.types.Handle @"dx.op.createHandleFromResourceStructForLib.class.Texture2D<float>"(i32 160, %"class.Texture2D<float>" %3)
  %15 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.SamplerComparisonState(i32 160, %struct.SamplerComparisonState %2)
  %16 = call %dx.types.ResRet.f32 @dx.op.sampleCmpLevelZero.f32(i32 65, %dx.types.Handle %14, %dx.types.Handle %15, float %13, float %12, float undef, float undef, i32 undef, i32 undef, i32 undef, float %11)
  %17 = extractvalue %dx.types.ResRet.f32 %16, 0
  %18 = fmul fast float %17, 2.000000e+00
  %.i0 = fsub fast float %13, %8
  %.i1 = fadd fast float %7, %12
  %19 = call %dx.types.ResRet.f32 @dx.op.sampleCmpLevelZero.f32(i32 65, %dx.types.Handle %14, %dx.types.Handle %15, float %.i0, float %.i1, float undef, float undef, i32 undef, i32 undef, i32 undef, float %11)
  %20 = extractvalue %dx.types.ResRet.f32 %19, 0
  %21 = fadd fast float %20, %18
  %.i01 = fsub fast float %13, %7
  %.i12 = fsub fast float %12, %8
  %22 = call %dx.types.ResRet.f32 @dx.op.sampleCmpLevelZero.f32(i32 65, %dx.types.Handle %14, %dx.types.Handle %15, float %.i01, float %.i12, float undef, float undef, i32 undef, i32 undef, i32 undef, float %11)
  %23 = extractvalue %dx.types.ResRet.f32 %22, 0
  %24 = fadd fast float %21, %23
  %.i03 = fadd fast float %8, %13
  %.i14 = fsub fast float %12, %7
  %25 = call %dx.types.ResRet.f32 @dx.op.sampleCmpLevelZero.f32(i32 65, %dx.types.Handle %14, %dx.types.Handle %15, float %.i03, float %.i14, float undef, float undef, i32 undef, i32 undef, i32 undef, float %11)
  %26 = extractvalue %dx.types.ResRet.f32 %25, 0
  %27 = fadd fast float %24, %26
  %.i05 = fadd fast float %7, %13
  %.i16 = fadd fast float %8, %12
  %28 = call %dx.types.ResRet.f32 @dx.op.sampleCmpLevelZero.f32(i32 65, %dx.types.Handle %14, %dx.types.Handle %15, float %.i05, float %.i16, float undef, float undef, i32 undef, i32 undef, i32 undef, float %11)
  %29 = extractvalue %dx.types.ResRet.f32 %28, 0
  %30 = fadd fast float %27, %29
  %.i07 = fsub fast float %13, %10
  %.i18 = fadd fast float %9, %12
  %31 = call %dx.types.ResRet.f32 @dx.op.sampleCmpLevelZero.f32(i32 65, %dx.types.Handle %14, %dx.types.Handle %15, float %.i07, float %.i18, float undef, float undef, i32 undef, i32 undef, i32 undef, float %11)
  %32 = extractvalue %dx.types.ResRet.f32 %31, 0
  %33 = fadd fast float %30, %32
  %.i09 = fsub fast float %13, %9
  %.i110 = fsub fast float %12, %10
  %34 = call %dx.types.ResRet.f32 @dx.op.sampleCmpLevelZero.f32(i32 65, %dx.types.Handle %14, %dx.types.Handle %15, float %.i09, float %.i110, float undef, float undef, i32 undef, i32 undef, i32 undef, float %11)
  %35 = extractvalue %dx.types.ResRet.f32 %34, 0
  %36 = fadd fast float %33, %35
  %.i011 = fadd fast float %10, %13
  %.i112 = fsub fast float %12, %9
  %37 = call %dx.types.ResRet.f32 @dx.op.sampleCmpLevelZero.f32(i32 65, %dx.types.Handle %14, %dx.types.Handle %15, float %.i011, float %.i112, float undef, float undef, i32 undef, i32 undef, i32 undef, float %11)
  %38 = extractvalue %dx.types.ResRet.f32 %37, 0
  %39 = fadd fast float %36, %38
  %.i013 = fadd fast float %9, %13
  %.i114 = fadd fast float %10, %12
  %40 = call %dx.types.ResRet.f32 @dx.op.sampleCmpLevelZero.f32(i32 65, %dx.types.Handle %14, %dx.types.Handle %15, float %.i013, float %.i114, float undef, float undef, i32 undef, i32 undef, i32 undef, float %11)
  %41 = extractvalue %dx.types.ResRet.f32 %40, 0
  %42 = fadd fast float %39, %41
  %43 = fmul fast float %42, 0x3FB99999A0000000
  %44 = fmul fast float %43, %43
  ret float %44
}

; Function Attrs: alwaysinline nounwind
define void @"\01?GetRayData@@YA?AURayData@@V?$vector@M$02@@0@Z"(%struct.RayData* noalias nocapture sret, <3 x float>, <3 x float>) #5 {
  %rayOrigin.i0 = extractelement <3 x float> %1, i32 0
  %rayOrigin.i1 = extractelement <3 x float> %1, i32 1
  %rayOrigin.i2 = extractelement <3 x float> %1, i32 2
  %4 = alloca [3 x float], align 4
  %5 = extractelement <3 x float> %2, i64 0
  %6 = getelementptr inbounds [3 x float], [3 x float]* %4, i32 0, i32 0
  store float %5, float* %6, align 4
  %7 = extractelement <3 x float> %2, i64 1
  %8 = getelementptr inbounds [3 x float], [3 x float]* %4, i32 0, i32 1
  store float %7, float* %8, align 4
  %9 = extractelement <3 x float> %2, i64 2
  %10 = getelementptr inbounds [3 x float], [3 x float]* %4, i32 0, i32 2
  store float %9, float* %10, align 4
  %.i0 = fdiv fast float 1.000000e+00, %5
  %.i1 = fdiv fast float 1.000000e+00, %7
  %.i2 = fdiv fast float 1.000000e+00, %9
  %.upto0 = insertelement <3 x float> undef, float %.i0, i32 0
  %.upto1 = insertelement <3 x float> %.upto0, float %.i1, i32 1
  %11 = insertelement <3 x float> %.upto1, float %.i2, i32 2
  %.i03 = fmul fast float %.i0, %rayOrigin.i0
  %.i14 = fmul fast float %.i1, %rayOrigin.i1
  %.i25 = fmul fast float %.i2, %rayOrigin.i2
  %.upto06 = insertelement <3 x float> undef, float %.i03, i32 0
  %.upto17 = insertelement <3 x float> %.upto06, float %.i14, i32 1
  %12 = insertelement <3 x float> %.upto17, float %.i25, i32 2
  %FAbs = call float @dx.op.unary.f32(i32 6, float %5)
  %FAbs1 = call float @dx.op.unary.f32(i32 6, float %7)
  %FAbs2 = call float @dx.op.unary.f32(i32 6, float %9)
  %13 = fcmp fast ogt float %FAbs, %FAbs1
  %14 = fcmp fast ogt float %FAbs, %FAbs2
  %15 = and i1 %13, %14
  br i1 %15, label %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit", label %16

; <label>:16                                      ; preds = %3
  %17 = fcmp fast ogt float %FAbs1, %FAbs2
  br i1 %17, label %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit", label %18

; <label>:18                                      ; preds = %16
  br label %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit"

"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit": ; preds = %18, %16, %3
  %.0 = phi i32 [ 2, %18 ], [ 0, %3 ], [ 1, %16 ]
  %19 = add nuw nsw i32 %.0, 1
  %20 = urem i32 %19, 3
  %21 = add nuw nsw i32 %.0, 2
  %22 = urem i32 %21, 3
  %23 = getelementptr [3 x float], [3 x float]* %4, i32 0, i32 %.0
  %24 = load float, float* %23, align 4, !tbaa !231
  %25 = fcmp fast olt float %24, 0.000000e+00
  %.5.0.i0 = select i1 %25, i32 %22, i32 %20
  %.5.0.i1 = select i1 %25, i32 %20, i32 %22
  %.5.0.upto0 = insertelement <3 x i32> undef, i32 %.5.0.i0, i32 0
  %.5.0.upto1 = insertelement <3 x i32> %.5.0.upto0, i32 %.5.0.i1, i32 1
  %.5.0 = insertelement <3 x i32> %.5.0.upto1, i32 %.0, i32 2
  %26 = getelementptr [3 x float], [3 x float]* %4, i32 0, i32 %.5.0.i0
  %27 = load float, float* %26, align 4, !tbaa !231
  %28 = fdiv fast float %27, %24
  %29 = getelementptr [3 x float], [3 x float]* %4, i32 0, i32 %.5.0.i1
  %30 = load float, float* %29, align 4, !tbaa !231
  %31 = fdiv fast float %30, %24
  %32 = fdiv fast float 1.000000e+00, %24
  %33 = insertelement <3 x float> undef, float %28, i64 0
  %34 = insertelement <3 x float> %33, float %31, i64 1
  %35 = insertelement <3 x float> %34, float %32, i64 2
  %36 = getelementptr inbounds %struct.RayData, %struct.RayData* %0, i32 0, i32 0
  store <3 x float> %1, <3 x float>* %36, align 4
  %37 = getelementptr inbounds %struct.RayData, %struct.RayData* %0, i32 0, i32 1
  store <3 x float> %2, <3 x float>* %37, align 4
  %38 = getelementptr inbounds %struct.RayData, %struct.RayData* %0, i32 0, i32 2
  store <3 x float> %11, <3 x float>* %38, align 4
  %39 = getelementptr inbounds %struct.RayData, %struct.RayData* %0, i32 0, i32 3
  store <3 x float> %12, <3 x float>* %39, align 4
  %40 = getelementptr inbounds %struct.RayData, %struct.RayData* %0, i32 0, i32 4
  store <3 x float> %35, <3 x float>* %40, align 4
  %41 = getelementptr inbounds %struct.RayData, %struct.RayData* %0, i32 0, i32 5
  store <3 x i32> %.5.0, <3 x i32>* %41, align 4
  ret void
}

; Function Attrs: alwaysinline nounwind readonly
define <3 x i32> @"\01?Load3x16BitIndices@@YA?AV?$vector@I$02@@I@Z"(i32) #4 {
  %2 = load %struct.ByteAddressBuffer, %struct.ByteAddressBuffer* @"\01?g_indices@@3UByteAddressBuffer@@A", align 4
  %3 = and i32 %0, -4
  %4 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.ByteAddressBuffer(i32 160, %struct.ByteAddressBuffer %2)
  %5 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %4, i32 %3, i32 undef)
  %6 = extractvalue %dx.types.ResRet.i32 %5, 0
  %7 = extractvalue %dx.types.ResRet.i32 %5, 1
  %8 = icmp eq i32 %3, %0
  %9 = and i32 %6, 65535
  %10 = lshr i32 %6, 16
  %11 = lshr i32 %6, 16
  %12 = lshr i32 %7, 16
  %.sink = select i1 %8, i32 %7, i32 %12
  %.0.i0 = select i1 %8, i32 %9, i32 %11
  %.0.i1.in = select i1 %8, i32 %10, i32 %7
  %.0.i1 = and i32 %.0.i1.in, 65535
  %13 = and i32 %.sink, 65535
  %.0.upto0 = insertelement <3 x i32> undef, i32 %.0.i0, i32 0
  %.0.upto1 = insertelement <3 x i32> %.0.upto0, i32 %.0.i1, i32 1
  %.0 = insertelement <3 x i32> %.0.upto1, i32 %13, i32 2
  ret <3 x i32> %.0
}

; Function Attrs: alwaysinline nounwind
define void @"\01?Fallback_AcceptHitAndEndSearch@@YAXXZ"() #5 {
  call void @"\01?Fallback_SetAnyHitResult@@YAXH@Z"(i32 -1) #3
  ret void
}

; Function Attrs: alwaysinline nounwind
declare void @RayGen() #5

; Function Attrs: alwaysinline nounwind
define void @"\01?swap@@YAXAIAH0@Z"(i32* noalias nocapture dereferenceable(4), i32* noalias nocapture dereferenceable(4)) #5 {
  %3 = load i32, i32* %0, align 4, !tbaa !221
  %4 = load i32, i32* %1, align 4, !tbaa !221
  store i32 %4, i32* %0, align 4, !tbaa !221
  store i32 %3, i32* %1, align 4, !tbaa !221
  ret void
}

; Function Attrs: alwaysinline nounwind readnone
define i32 @"\01?GetTriangleMetadataAddress@@YAIII@Z"(i32, i32) #6 {
  %3 = shl i32 %1, 3
  %4 = add i32 %3, %0
  ret i32 %4
}

; Function Attrs: alwaysinline nounwind
define void @"\01?WriteOnlyFlagToBuffer@@YAXURWByteAddressBuffer@@IIV?$vector@I$01@@@Z"(%struct.RWByteAddressBuffer* nocapture readonly, i32, i32, <2 x i32>) #5 {
  %5 = shl i32 %2, 5
  %6 = add i32 %5, %1
  %7 = extractelement <2 x i32> %3, i32 0
  %8 = add i32 %6, 12
  %9 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %0, align 4
  %10 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %9)
  call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %10, i32 %8, i32 undef, i32 %7, i32 undef, i32 undef, i32 undef, i8 1)
  %11 = extractelement <2 x i32> %3, i32 1
  %12 = add i32 %6, 28
  %13 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %0, align 4
  %14 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %13)
  call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %14, i32 %12, i32 undef, i32 %11, i32 undef, i32 undef, i32 undef, i8 1)
  ret void
}

; Function Attrs: alwaysinline nounwind
define void @"\01?WriteBoxToBuffer@@YAXURWByteAddressBuffer@@IIUBoundingBox@@V?$vector@I$01@@@Z"(%struct.RWByteAddressBuffer* nocapture readonly, i32, i32, %struct.BoundingBox* nocapture readonly, <2 x i32>) #5 {
  %6 = shl i32 %2, 5
  %7 = add i32 %6, %1
  %8 = getelementptr inbounds %struct.BoundingBox, %struct.BoundingBox* %3, i32 0, i32 0
  %9 = load <3 x float>, <3 x float>* %8, align 4, !noalias !328
  %10 = extractelement <3 x float> %9, i32 0
  %11 = bitcast float %10 to i32
  %12 = extractelement <3 x float> %9, i32 1
  %13 = bitcast float %12 to i32
  %14 = extractelement <3 x float> %9, i32 2
  %15 = bitcast float %14 to i32
  %16 = extractelement <2 x i32> %4, i32 0
  %17 = getelementptr inbounds %struct.BoundingBox, %struct.BoundingBox* %3, i32 0, i32 1
  %18 = load <3 x float>, <3 x float>* %17, align 4, !noalias !328
  %19 = extractelement <3 x float> %18, i32 0
  %20 = bitcast float %19 to i32
  %21 = extractelement <3 x float> %18, i32 1
  %22 = bitcast float %21 to i32
  %23 = extractelement <3 x float> %18, i32 2
  %24 = bitcast float %23 to i32
  %25 = extractelement <2 x i32> %4, i32 1
  %26 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %0, align 4
  %27 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %26)
  call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %27, i32 %7, i32 undef, i32 %11, i32 %13, i32 %15, i32 %16, i8 15)
  %28 = add i32 %7, 16
  %29 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %0, align 4
  %30 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %29)
  call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %30, i32 %28, i32 undef, i32 %20, i32 %22, i32 %24, i32 %25, i8 15)
  ret void
}

; Function Attrs: alwaysinline nounwind readnone
define void @"\01?LogTraceRayStart@@YAXXZ"() #6 {
  ret void
}

; Function Attrs: alwaysinline nounwind readnone
define void @"\01?LogInt2@@YAXV?$vector@H$01@@@Z"(<2 x i32>) #6 {
  ret void
}

; Function Attrs: alwaysinline nounwind
define void @"\01?CalculateUVDerivatives@@YAXV?$vector@M$02@@00000AIAV?$vector@M$01@@1@Z"(<3 x float>, <3 x float>, <3 x float>, <3 x float>, <3 x float>, <3 x float>, <2 x float>* noalias nocapture dereferenceable(8), <2 x float>* noalias nocapture dereferenceable(8)) #5 {
  %9 = alloca [3 x float], align 4
  %10 = alloca [3 x float], align 4
  %11 = alloca [3 x float], align 4
  %12 = alloca [3 x float], align 4
  %13 = alloca [3 x float], align 4
  %14 = extractelement <3 x float> %5, i64 0
  %15 = getelementptr inbounds [3 x float], [3 x float]* %9, i32 0, i32 0
  store float %14, float* %15, align 4
  %16 = extractelement <3 x float> %5, i64 1
  %17 = getelementptr inbounds [3 x float], [3 x float]* %9, i32 0, i32 1
  store float %16, float* %17, align 4
  %18 = extractelement <3 x float> %5, i64 2
  %19 = getelementptr inbounds [3 x float], [3 x float]* %9, i32 0, i32 2
  store float %18, float* %19, align 4
  %20 = extractelement <3 x float> %4, i64 0
  %21 = getelementptr inbounds [3 x float], [3 x float]* %10, i32 0, i32 0
  store float %20, float* %21, align 4
  %22 = extractelement <3 x float> %4, i64 1
  %23 = getelementptr inbounds [3 x float], [3 x float]* %10, i32 0, i32 1
  store float %22, float* %23, align 4
  %24 = extractelement <3 x float> %4, i64 2
  %25 = getelementptr inbounds [3 x float], [3 x float]* %10, i32 0, i32 2
  store float %24, float* %25, align 4
  %26 = extractelement <3 x float> %3, i64 0
  %27 = getelementptr inbounds [3 x float], [3 x float]* %11, i32 0, i32 0
  store float %26, float* %27, align 4
  %28 = extractelement <3 x float> %3, i64 1
  %29 = getelementptr inbounds [3 x float], [3 x float]* %11, i32 0, i32 1
  store float %28, float* %29, align 4
  %30 = extractelement <3 x float> %3, i64 2
  %31 = getelementptr inbounds [3 x float], [3 x float]* %11, i32 0, i32 2
  store float %30, float* %31, align 4
  %32 = extractelement <3 x float> %2, i64 0
  %33 = getelementptr inbounds [3 x float], [3 x float]* %12, i32 0, i32 0
  store float %32, float* %33, align 4
  %34 = extractelement <3 x float> %2, i64 1
  %35 = getelementptr inbounds [3 x float], [3 x float]* %12, i32 0, i32 1
  store float %34, float* %35, align 4
  %36 = extractelement <3 x float> %2, i64 2
  %37 = getelementptr inbounds [3 x float], [3 x float]* %12, i32 0, i32 2
  store float %36, float* %37, align 4
  %38 = extractelement <3 x float> %1, i64 0
  %39 = getelementptr inbounds [3 x float], [3 x float]* %13, i32 0, i32 0
  store float %38, float* %39, align 4
  %40 = extractelement <3 x float> %1, i64 1
  %41 = getelementptr inbounds [3 x float], [3 x float]* %13, i32 0, i32 1
  store float %40, float* %41, align 4
  %42 = extractelement <3 x float> %1, i64 2
  %43 = getelementptr inbounds [3 x float], [3 x float]* %13, i32 0, i32 2
  store float %42, float* %43, align 4
  %44 = extractelement <3 x float> %0, i64 0
  %FAbs = call float @dx.op.unary.f32(i32 6, float %44)
  %45 = extractelement <3 x float> %0, i64 1
  %FAbs6 = call float @dx.op.unary.f32(i32 6, float %45)
  %46 = extractelement <3 x float> %0, i64 2
  %FAbs7 = call float @dx.op.unary.f32(i32 6, float %46)
  %47 = fcmp fast ogt float %FAbs, %FAbs6
  %48 = fcmp fast ogt float %FAbs, %FAbs7
  %49 = and i1 %47, %48
  br i1 %49, label %53, label %50

; <label>:50                                      ; preds = %8
  %51 = fcmp fast ogt float %FAbs6, %FAbs7
  br i1 %51, label %53, label %52

; <label>:52                                      ; preds = %50
  br label %53

; <label>:53                                      ; preds = %52, %50, %8
  %54 = phi float [ %16, %52 ], [ %18, %8 ], [ %18, %50 ]
  %55 = phi float [ %28, %52 ], [ %30, %8 ], [ %30, %50 ]
  %56 = phi float [ %22, %52 ], [ %24, %8 ], [ %24, %50 ]
  %57 = phi float [ %34, %52 ], [ %36, %8 ], [ %36, %50 ]
  %58 = phi float [ %40, %52 ], [ %42, %8 ], [ %42, %50 ]
  %indices.0.i0 = phi i32 [ 0, %52 ], [ 1, %8 ], [ 0, %50 ]
  %59 = getelementptr [3 x float], [3 x float]* %13, i32 0, i32 %indices.0.i0
  %60 = load float, float* %59, align 4, !tbaa !231
  %61 = getelementptr [3 x float], [3 x float]* %12, i32 0, i32 %indices.0.i0
  %62 = load float, float* %61, align 4, !tbaa !231
  %63 = fmul fast float %57, %60
  %64 = fmul fast float %58, %62
  %65 = fsub fast float %63, %64
  %66 = fdiv fast float 1.000000e+00, %65
  %.i0 = fmul fast float %66, %57
  %67 = fmul fast float %58, %66
  %.i1 = fsub fast float -0.000000e+00, %67
  %68 = fmul fast float %62, %66
  %.i2 = fsub fast float -0.000000e+00, %68
  %.i3 = fmul fast float %66, %60
  %69 = getelementptr [3 x float], [3 x float]* %10, i32 0, i32 %indices.0.i0
  %70 = load float, float* %69, align 4, !tbaa !231
  %71 = getelementptr [3 x float], [3 x float]* %11, i32 0, i32 %indices.0.i0
  %72 = load float, float* %71, align 4, !tbaa !231
  %73 = fsub fast float %70, %72
  %74 = fsub fast float %56, %55
  %75 = fmul fast float %73, %.i0
  %FMad5 = call float @dx.op.tertiary.f32(i32 46, float %74, float %.i1, float %75)
  %76 = fmul fast float %73, %.i2
  %FMad4 = call float @dx.op.tertiary.f32(i32 46, float %74, float %.i3, float %76)
  %FAbs8 = call float @dx.op.unary.f32(i32 6, float %FMad5)
  %77 = insertelement <2 x float> undef, float %FAbs8, i64 0
  %FAbs9 = call float @dx.op.unary.f32(i32 6, float %FMad4)
  %78 = insertelement <2 x float> %77, float %FAbs9, i64 1
  store <2 x float> %78, <2 x float>* %6, align 4, !tbaa !218
  %79 = getelementptr [3 x float], [3 x float]* %9, i32 0, i32 %indices.0.i0
  %80 = load float, float* %79, align 4, !tbaa !231
  %81 = fsub fast float %80, %72
  %82 = fsub fast float %54, %55
  %83 = fmul fast float %81, %.i0
  %FMad3 = call float @dx.op.tertiary.f32(i32 46, float %82, float %.i1, float %83)
  %84 = fmul fast float %81, %.i2
  %FMad = call float @dx.op.tertiary.f32(i32 46, float %82, float %.i3, float %84)
  %FAbs10 = call float @dx.op.unary.f32(i32 6, float %FMad3)
  %85 = insertelement <2 x float> undef, float %FAbs10, i64 0
  %FAbs11 = call float @dx.op.unary.f32(i32 6, float %FMad)
  %86 = insertelement <2 x float> %85, float %FAbs11, i64 1
  store <2 x float> %86, <2 x float>* %7, align 4, !tbaa !218
  ret void
}

; Function Attrs: alwaysinline nounwind
define void @"\01?main@@YAXV?$vector@I$02@@I@Z"(<3 x i32>, i32) #5 {
  %3 = load %struct.ByteAddressBuffer, %struct.ByteAddressBuffer* @"\01?RayGenShaderTable@@3UByteAddressBuffer@@A", align 4
  %4 = load %Constants, %Constants* @Constants, align 4
  %Constants = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.Constants(i32 160, %Constants %4)
  %5 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.ByteAddressBuffer(i32 160, %struct.ByteAddressBuffer %3)
  %6 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %5, i32 0, i32 undef)
  %7 = extractvalue %dx.types.ResRet.i32 %6, 0
  %8 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %Constants, i32 0)
  %9 = extractvalue %dx.types.CBufRet.i32 %8, 1
  %10 = extractvalue %dx.types.CBufRet.i32 %8, 0
  call void @fb_Fallback_Scheduler(i32 %7, i32 %10, i32 %9)
  ret void
}

declare i32 @movePayloadToStack(%struct.RayPayload*)

; Function Attrs: nounwind
define i32 @asint(float %v) #3 {
entry:
  %v.addr = alloca float, align 4
  store float %v, float* %v.addr, align 4
  %0 = bitcast float* %v.addr to i32*
  %1 = load i32, i32* %0, align 4
  ret i32 %1
}

; Function Attrs: nounwind
define float @asfloat(i32 %v) #3 {
entry:
  %v.addr = alloca i32, align 4
  store i32 %v, i32* %v.addr, align 4
  %0 = bitcast i32* %v.addr to float*
  %1 = load float, float* %0, align 4
  ret float %1
}

; Function Attrs: nounwind
define void @stackInit(%struct.RuntimeDataStruct* %runtimeData, [256 x i32]* %theStack, i32 %stackSize) #3 {
entry:
  %Stack = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 29
  store [256 x i32]* %theStack, [256 x i32]** %Stack, align 4
  %div = udiv i32 %stackSize, 4
  %sub = sub i32 %div, 1
  %StackOffset = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 28
  store i32 %sub, i32* %StackOffset, align 4
  %PayloadOffset = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 25
  store i32 1111, i32* %PayloadOffset, align 4
  %CommittedAttrOffset = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 26
  store i32 2222, i32* %CommittedAttrOffset, align 4
  %PendingAttrOffset = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 27
  store i32 3333, i32* %PendingAttrOffset, align 4
  ret void
}

; Function Attrs: nounwind
define void @stackFramePush(%struct.RuntimeDataStruct* %runtimeData, i32 %size) #3 {
entry:
  %StackOffset = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 28
  %0 = load i32, i32* %StackOffset, align 4
  %sub = sub nsw i32 %0, %size
  store i32 %sub, i32* %StackOffset, align 4
  ret void
}

; Function Attrs: nounwind
define void @stackFramePop(%struct.RuntimeDataStruct* %runtimeData, i32 %size) #3 {
entry:
  %StackOffset = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 28
  %0 = load i32, i32* %StackOffset, align 4
  %add = add nsw i32 %0, %size
  store i32 %add, i32* %StackOffset, align 4
  ret void
}

; Function Attrs: nounwind
define i32 @stackFrameOffset(%struct.RuntimeDataStruct* %runtimeData) #3 {
entry:
  %StackOffset = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 28
  %0 = load i32, i32* %StackOffset, align 4
  ret i32 %0
}

; Function Attrs: nounwind
define i32 @payloadOffset(%struct.RuntimeDataStruct* %runtimeData) #3 {
entry:
  %PayloadOffset = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 25
  %0 = load i32, i32* %PayloadOffset, align 4
  ret i32 %0
}

; Function Attrs: nounwind
define i32 @committedAttrOffset(%struct.RuntimeDataStruct* %runtimeData) #3 {
entry:
  %CommittedAttrOffset = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 26
  %0 = load i32, i32* %CommittedAttrOffset, align 4
  ret i32 %0
}

; Function Attrs: nounwind
define i32 @pendingAttrOffset(%struct.RuntimeDataStruct* %runtimeData) #3 {
entry:
  %PendingAttrOffset = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 27
  %0 = load i32, i32* %PendingAttrOffset, align 4
  ret i32 %0
}

; Function Attrs: nounwind
define i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %baseOffset, i32 %offset) #3 {
entry:
  %add = add nsw i32 %baseOffset, %offset
  %Stack = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 29
  %0 = load [256 x i32]*, [256 x i32]** %Stack, align 4
  %arrayidx = getelementptr inbounds [256 x i32], [256 x i32]* %0, i32 0, i32 %add
  ret i32* %arrayidx
}

; Function Attrs: nounwind
define void @traceFramePush(%struct.RuntimeDataStruct* %runtimeData, i32 %attrSize) #3 {
entry:
  %CommittedAttrOffset = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 26
  %0 = load i32, i32* %CommittedAttrOffset, align 4
  %StackOffset = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 28
  %1 = load i32, i32* %StackOffset, align 4
  %add = add nsw i32 %1, -1
  %Stack = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 29
  %2 = load [256 x i32]*, [256 x i32]** %Stack, align 4
  %arrayidx = getelementptr inbounds [256 x i32], [256 x i32]* %2, i32 0, i32 %add
  store i32 %0, i32* %arrayidx, align 4
  %PendingAttrOffset = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 27
  %3 = load i32, i32* %PendingAttrOffset, align 4
  %StackOffset1 = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 28
  %4 = load i32, i32* %StackOffset1, align 4
  %add2 = add nsw i32 %4, -2
  %Stack3 = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 29
  %5 = load [256 x i32]*, [256 x i32]** %Stack3, align 4
  %arrayidx4 = getelementptr inbounds [256 x i32], [256 x i32]* %5, i32 0, i32 %add2
  store i32 %3, i32* %arrayidx4, align 4
  %StackOffset5 = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 28
  %6 = load i32, i32* %StackOffset5, align 4
  %sub = sub nsw i32 %6, 2
  %sub6 = sub nsw i32 %sub, %attrSize
  %CommittedAttrOffset7 = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 26
  store i32 %sub6, i32* %CommittedAttrOffset7, align 4
  %StackOffset8 = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 28
  %7 = load i32, i32* %StackOffset8, align 4
  %sub9 = sub nsw i32 %7, 2
  %mul = mul nsw i32 2, %attrSize
  %sub10 = sub nsw i32 %sub9, %mul
  %PendingAttrOffset11 = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 27
  store i32 %sub10, i32* %PendingAttrOffset11, align 4
  ret void
}

; Function Attrs: nounwind
define void @traceFramePop(%struct.RuntimeDataStruct* %runtimeData) #3 {
entry:
  %StackOffset = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 28
  %0 = load i32, i32* %StackOffset, align 4
  %add = add nsw i32 %0, -1
  %Stack = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 29
  %1 = load [256 x i32]*, [256 x i32]** %Stack, align 4
  %arrayidx = getelementptr inbounds [256 x i32], [256 x i32]* %1, i32 0, i32 %add
  %2 = load i32, i32* %arrayidx, align 4
  %CommittedAttrOffset = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 26
  store i32 %2, i32* %CommittedAttrOffset, align 4
  %StackOffset1 = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 28
  %3 = load i32, i32* %StackOffset1, align 4
  %add2 = add nsw i32 %3, -2
  %Stack3 = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 29
  %4 = load [256 x i32]*, [256 x i32]** %Stack3, align 4
  %arrayidx4 = getelementptr inbounds [256 x i32], [256 x i32]* %4, i32 0, i32 %add2
  %5 = load i32, i32* %arrayidx4, align 4
  %PendingAttrOffset = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 27
  store i32 %5, i32* %PendingAttrOffset, align 4
  ret void
}

; Function Attrs: nounwind
define void @fb_Fallback_Scheduler(i32 %initialStateId, i32 %dimx, i32 %dimy) #3 {
entry:
  %theRuntimeData = alloca %struct.RuntimeDataStruct, align 4
  %theStack = alloca [256 x i32], align 4
  %DTidx = call i32 @dx.op.threadId.i32(i32 93, i32 0)
  %DTidy = call i32 @dx.op.threadId.i32(i32 93, i32 1)
  %groupIndex = call i32 @dx.op.flattenedThreadIdInGroup.i32(i32 96)
  call void @fb_Fallback_SetLaunchParams(%struct.RuntimeDataStruct* %theRuntimeData, i32 %DTidx, i32 %DTidy, i32 %dimx, i32 %dimy, i32 %groupIndex)
  %DispatchRaysIndex = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %theRuntimeData, i32 0, i32 0
  %arrayidx = getelementptr inbounds [2 x i32], [2 x i32]* %DispatchRaysIndex, i32 0, i32 0
  %0 = load i32, i32* %arrayidx, align 4
  %DispatchRaysDimensions = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %theRuntimeData, i32 0, i32 1
  %arrayidx2 = getelementptr inbounds [2 x i32], [2 x i32]* %DispatchRaysDimensions, i32 0, i32 0
  %1 = load i32, i32* %arrayidx2, align 4
  %cmp = icmp sge i32 %0, %1
  br i1 %cmp, label %if.then, label %lor.lhs.false

lor.lhs.false:                                    ; preds = %entry
  %DispatchRaysIndex3 = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %theRuntimeData, i32 0, i32 0
  %arrayidx4 = getelementptr inbounds [2 x i32], [2 x i32]* %DispatchRaysIndex3, i32 0, i32 1
  %2 = load i32, i32* %arrayidx4, align 4
  %DispatchRaysDimensions5 = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %theRuntimeData, i32 0, i32 1
  %arrayidx6 = getelementptr inbounds [2 x i32], [2 x i32]* %DispatchRaysDimensions5, i32 0, i32 1
  %3 = load i32, i32* %arrayidx6, align 4
  %cmp7 = icmp sge i32 %2, %3
  br i1 %cmp7, label %if.then, label %if.end

if.then:                                          ; preds = %lor.lhs.false, %entry
  br label %while.end

if.end:                                           ; preds = %lor.lhs.false
  call void @stackInit(%struct.RuntimeDataStruct* %theRuntimeData, [256 x i32]* %theStack, i32 1024)
  %call9 = call i32 @stackFrameOffset(%struct.RuntimeDataStruct* %theRuntimeData)
  %call10 = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %theRuntimeData, i32 %call9, i32 0)
  store i32 -1, i32* %call10, align 4
  br label %while.cond

while.cond:                                       ; preds = %while.body, %if.end
  %stateId.0 = phi i32 [ %initialStateId, %if.end ], [ %call12, %while.body ]
  %cmp11 = icmp sge i32 %stateId.0, 0
  br i1 %cmp11, label %while.body, label %while.end

while.body:                                       ; preds = %while.cond
  %call12 = call i32 @dispatch(%struct.RuntimeDataStruct* %theRuntimeData, i32 %stateId.0)
  br label %while.cond

while.end:                                        ; preds = %while.cond, %if.then
  ret void
}

; Function Attrs: nounwind
define void @fb_Fallback_SetLaunchParams(%struct.RuntimeDataStruct* %runtimeData, i32 %DTidx, i32 %DTidy, i32 %dimx, i32 %dimy, i32 %groupIndex) #3 {
entry:
  %DispatchRaysIndex = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 0
  %arrayidx = getelementptr inbounds [2 x i32], [2 x i32]* %DispatchRaysIndex, i32 0, i32 0
  store i32 %DTidx, i32* %arrayidx, align 4
  %DispatchRaysIndex1 = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 0
  %arrayidx2 = getelementptr inbounds [2 x i32], [2 x i32]* %DispatchRaysIndex1, i32 0, i32 1
  store i32 %DTidy, i32* %arrayidx2, align 4
  %DispatchRaysDimensions = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 1
  %arrayidx3 = getelementptr inbounds [2 x i32], [2 x i32]* %DispatchRaysDimensions, i32 0, i32 0
  store i32 %dimx, i32* %arrayidx3, align 4
  %DispatchRaysDimensions4 = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 1
  %arrayidx5 = getelementptr inbounds [2 x i32], [2 x i32]* %DispatchRaysDimensions4, i32 0, i32 1
  store i32 %dimy, i32* %arrayidx5, align 4
  %GroupIndex = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 22
  store i32 %groupIndex, i32* %GroupIndex, align 4
  ret void
}

; Function Attrs: nounwind
define i32 @fb_Fallback_SetPayloadOffset(%struct.RuntimeDataStruct* %runtimeData, i32 %newOffset) #3 {
entry:
  %PayloadOffset = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 25
  %0 = load i32, i32* %PayloadOffset, align 4
  %PayloadOffset1 = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 25
  store i32 %newOffset, i32* %PayloadOffset1, align 4
  ret i32 %0
}

; Function Attrs: nounwind
define void @fb_Fallback_TraceRayBegin(%struct.RuntimeDataStruct* %runtimeData, float %ox, float %oy, float %oz, float %tmin, float %dx, float %dy, float %dz, float %tmax) #3 {
entry:
  %WorldRayOrigin = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 5
  %arrayidx = getelementptr inbounds [3 x float], [3 x float]* %WorldRayOrigin, i32 0, i32 0
  store float %ox, float* %arrayidx, align 4
  %WorldRayOrigin1 = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 5
  %arrayidx2 = getelementptr inbounds [3 x float], [3 x float]* %WorldRayOrigin1, i32 0, i32 1
  store float %oy, float* %arrayidx2, align 4
  %WorldRayOrigin3 = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 5
  %arrayidx4 = getelementptr inbounds [3 x float], [3 x float]* %WorldRayOrigin3, i32 0, i32 2
  store float %oz, float* %arrayidx4, align 4
  %WorldRayDirection = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 6
  %arrayidx5 = getelementptr inbounds [3 x float], [3 x float]* %WorldRayDirection, i32 0, i32 0
  store float %dx, float* %arrayidx5, align 4
  %WorldRayDirection6 = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 6
  %arrayidx7 = getelementptr inbounds [3 x float], [3 x float]* %WorldRayDirection6, i32 0, i32 1
  store float %dy, float* %arrayidx7, align 4
  %WorldRayDirection8 = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 6
  %arrayidx9 = getelementptr inbounds [3 x float], [3 x float]* %WorldRayDirection8, i32 0, i32 2
  store float %dz, float* %arrayidx9, align 4
  %RayTCurrent = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 3
  store float %tmax, float* %RayTCurrent, align 4
  %RayTMin = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 2
  store float %tmin, float* %RayTMin, align 4
  ret void
}

; Function Attrs: nounwind
define void @fb_Fallback_SetPendingTriVals(%struct.RuntimeDataStruct* %runtimeData, float %t, i32 %primitiveIndex, i32 %geometryIndex, i32 %instanceIndex, i32 %instanceID, i32 %hitKind) #3 {
entry:
  %PendingRayTCurrent = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 16
  store float %t, float* %PendingRayTCurrent, align 4
  %PendingPrimitiveIndex = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 17
  store i32 %primitiveIndex, i32* %PendingPrimitiveIndex, align 4
  %PendingGeometryIndex = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 18
  store i32 %geometryIndex, i32* %PendingGeometryIndex, align 4
  %PendingInstanceIndex = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 19
  store i32 %instanceIndex, i32* %PendingInstanceIndex, align 4
  %PendingInstanceID = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 20
  store i32 %instanceID, i32* %PendingInstanceID, align 4
  %PendingHitKind = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 21
  store i32 %hitKind, i32* %PendingHitKind, align 4
  ret void
}

; Function Attrs: nounwind
define void @fb_Fallback_SetPendingCustomVals(%struct.RuntimeDataStruct* %runtimeData, i32 %primitiveIndex, i32 %geometryIndex, i32 %instanceIndex, i32 %instanceID) #3 {
entry:
  %PendingPrimitiveIndex = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 17
  store i32 %primitiveIndex, i32* %PendingPrimitiveIndex, align 4
  %PendingGeometryIndex = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 18
  store i32 %geometryIndex, i32* %PendingGeometryIndex, align 4
  %PendingInstanceIndex = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 19
  store i32 %instanceIndex, i32* %PendingInstanceIndex, align 4
  %PendingInstanceID = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 20
  store i32 %instanceID, i32* %PendingInstanceID, align 4
  ret void
}

; Function Attrs: nounwind
define void @fb_Fallback_CommitHit(%struct.RuntimeDataStruct* %runtimeData) #3 {
entry:
  %PendingRayTCurrent = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 16
  %0 = load float, float* %PendingRayTCurrent, align 4
  %RayTCurrent = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 3
  store float %0, float* %RayTCurrent, align 4
  %PendingPrimitiveIndex = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 17
  %1 = load i32, i32* %PendingPrimitiveIndex, align 4
  %PrimitiveIndex = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 11
  store i32 %1, i32* %PrimitiveIndex, align 4
  %PendingGeometryIndex = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 18
  %2 = load i32, i32* %PendingGeometryIndex, align 4
  %GeometryIndex = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 12
  store i32 %2, i32* %GeometryIndex, align 4
  %PendingInstanceIndex = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 19
  %3 = load i32, i32* %PendingInstanceIndex, align 4
  %InstanceIndex = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 13
  store i32 %3, i32* %InstanceIndex, align 4
  %PendingInstanceID = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 20
  %4 = load i32, i32* %PendingInstanceID, align 4
  %InstanceID = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 14
  store i32 %4, i32* %InstanceID, align 4
  %PendingHitKind = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 21
  %5 = load i32, i32* %PendingHitKind, align 4
  %HitKind = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 15
  store i32 %5, i32* %HitKind, align 4
  %PendingAttrOffset1 = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 27
  %6 = load i32, i32* %PendingAttrOffset1, align 4
  %CommittedAttrOffset = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 26
  %7 = load i32, i32* %CommittedAttrOffset, align 4
  %PendingAttrOffset2 = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 27
  store i32 %7, i32* %PendingAttrOffset2, align 4
  %CommittedAttrOffset3 = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 26
  store i32 %6, i32* %CommittedAttrOffset3, align 4
  ret void
}

; Function Attrs: nounwind
define i32 @fb_Fallback_RuntimeDataLoadInt(%struct.RuntimeDataStruct* %runtimeData, i32 %offset) #3 {
entry:
  %Stack = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 29
  %0 = load [256 x i32]*, [256 x i32]** %Stack, align 4
  %arrayidx = getelementptr inbounds [256 x i32], [256 x i32]* %0, i32 0, i32 %offset
  %1 = load i32, i32* %arrayidx, align 4
  ret i32 %1
}

; Function Attrs: nounwind
define void @fb_Fallback_RuntimeDataStoreInt(%struct.RuntimeDataStruct* %runtimeData, i32 %offset, i32 %val) #3 {
entry:
  %Stack = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 29
  %0 = load [256 x i32]*, [256 x i32]** %Stack, align 4
  %arrayidx = getelementptr inbounds [256 x i32], [256 x i32]* %0, i32 0, i32 %offset
  store i32 %val, i32* %arrayidx, align 4
  ret void
}

; Function Attrs: nounwind
define i32 @fb_dxop_dispatchRaysIndex(%struct.RuntimeDataStruct* %runtimeData, i8 zeroext %i) #3 {
entry:
  %idxprom = zext i8 %i to i32
  %DispatchRaysIndex = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 0
  %arrayidx = getelementptr inbounds [2 x i32], [2 x i32]* %DispatchRaysIndex, i32 0, i32 %idxprom
  %0 = load i32, i32* %arrayidx, align 4
  ret i32 %0
}

; Function Attrs: nounwind
define i32 @fb_dxop_dispatchRaysDimensions(%struct.RuntimeDataStruct* %runtimeData, i8 zeroext %i) #3 {
entry:
  %idxprom = zext i8 %i to i32
  %DispatchRaysDimensions = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 1
  %arrayidx = getelementptr inbounds [2 x i32], [2 x i32]* %DispatchRaysDimensions, i32 0, i32 %idxprom
  %0 = load i32, i32* %arrayidx, align 4
  ret i32 %0
}

; Function Attrs: nounwind
define float @fb_dxop_rayTMin(%struct.RuntimeDataStruct* %runtimeData) #3 {
entry:
  %RayTMin = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 2
  %0 = load float, float* %RayTMin, align 4
  ret float %0
}

; Function Attrs: nounwind
define void @fb_Fallback_SetRayTMin(%struct.RuntimeDataStruct* %runtimeData, float %t) #3 {
entry:
  %RayTMin = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 2
  store float %t, float* %RayTMin, align 4
  ret void
}

; Function Attrs: nounwind
define float @fb_Fallback_GetRayTCurrent(%struct.RuntimeDataStruct* %runtimeData) #3 {
entry:
  %RayTCurrent = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 3
  %0 = load float, float* %RayTCurrent, align 4
  ret float %0
}

; Function Attrs: nounwind
define float @fb_dxop_rayTCurrent(%struct.RuntimeDataStruct* %runtimeData) #3 {
entry:
  %RayTCurrent = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 3
  %0 = load float, float* %RayTCurrent, align 4
  ret float %0
}

; Function Attrs: nounwind
define void @fb_Fallback_SetRayTCurrent(%struct.RuntimeDataStruct* %runtimeData, float %t) #3 {
entry:
  %RayTCurrent = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 3
  store float %t, float* %RayTCurrent, align 4
  ret void
}

; Function Attrs: nounwind
define i32 @fb_dxop_rayFlags(%struct.RuntimeDataStruct* %runtimeData) #3 {
entry:
  %RayFlags = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 4
  %0 = load i32, i32* %RayFlags, align 4
  ret i32 %0
}

; Function Attrs: nounwind
define void @fb_Fallback_SetRayFlags(%struct.RuntimeDataStruct* %runtimeData, i32 %flags) #3 {
entry:
  %RayFlags = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 4
  store i32 %flags, i32* %RayFlags, align 4
  ret void
}

; Function Attrs: nounwind
define float @fb_dxop_worldRayOrigin(%struct.RuntimeDataStruct* %runtimeData, i8 zeroext %i) #3 {
entry:
  %idxprom = zext i8 %i to i32
  %WorldRayOrigin = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 5
  %arrayidx = getelementptr inbounds [3 x float], [3 x float]* %WorldRayOrigin, i32 0, i32 %idxprom
  %0 = load float, float* %arrayidx, align 4
  ret float %0
}

; Function Attrs: nounwind
define void @fb_Fallback_SetWorldRayOrigin(%struct.RuntimeDataStruct* %runtimeData, float %x, float %y, float %z) #3 {
entry:
  %WorldRayOrigin = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 5
  %arrayidx = getelementptr inbounds [3 x float], [3 x float]* %WorldRayOrigin, i32 0, i32 0
  store float %x, float* %arrayidx, align 4
  %WorldRayOrigin1 = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 5
  %arrayidx2 = getelementptr inbounds [3 x float], [3 x float]* %WorldRayOrigin1, i32 0, i32 1
  store float %y, float* %arrayidx2, align 4
  %WorldRayOrigin3 = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 5
  %arrayidx4 = getelementptr inbounds [3 x float], [3 x float]* %WorldRayOrigin3, i32 0, i32 2
  store float %z, float* %arrayidx4, align 4
  ret void
}

; Function Attrs: nounwind
define float @fb_dxop_worldRayDirection(%struct.RuntimeDataStruct* %runtimeData, i8 zeroext %i) #3 {
entry:
  %idxprom = zext i8 %i to i32
  %WorldRayDirection = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 6
  %arrayidx = getelementptr inbounds [3 x float], [3 x float]* %WorldRayDirection, i32 0, i32 %idxprom
  %0 = load float, float* %arrayidx, align 4
  ret float %0
}

; Function Attrs: nounwind
define void @fb_Fallback_SetWorldRayDirection(%struct.RuntimeDataStruct* %runtimeData, float %x, float %y, float %z) #3 {
entry:
  %WorldRayDirection = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 6
  %arrayidx = getelementptr inbounds [3 x float], [3 x float]* %WorldRayDirection, i32 0, i32 0
  store float %x, float* %arrayidx, align 4
  %WorldRayDirection1 = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 6
  %arrayidx2 = getelementptr inbounds [3 x float], [3 x float]* %WorldRayDirection1, i32 0, i32 1
  store float %y, float* %arrayidx2, align 4
  %WorldRayDirection3 = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 6
  %arrayidx4 = getelementptr inbounds [3 x float], [3 x float]* %WorldRayDirection3, i32 0, i32 2
  store float %z, float* %arrayidx4, align 4
  ret void
}

; Function Attrs: nounwind
define float @fb_dxop_objectRayOrigin(%struct.RuntimeDataStruct* %runtimeData, i8 zeroext %i) #3 {
entry:
  %idxprom = zext i8 %i to i32
  %ObjectRayOrigin = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 7
  %arrayidx = getelementptr inbounds [3 x float], [3 x float]* %ObjectRayOrigin, i32 0, i32 %idxprom
  %0 = load float, float* %arrayidx, align 4
  ret float %0
}

; Function Attrs: nounwind
define void @fb_Fallback_SetObjectRayOrigin(%struct.RuntimeDataStruct* %runtimeData, float %x, float %y, float %z) #3 {
entry:
  %ObjectRayOrigin = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 7
  %arrayidx = getelementptr inbounds [3 x float], [3 x float]* %ObjectRayOrigin, i32 0, i32 0
  store float %x, float* %arrayidx, align 4
  %ObjectRayOrigin1 = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 7
  %arrayidx2 = getelementptr inbounds [3 x float], [3 x float]* %ObjectRayOrigin1, i32 0, i32 1
  store float %y, float* %arrayidx2, align 4
  %ObjectRayOrigin3 = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 7
  %arrayidx4 = getelementptr inbounds [3 x float], [3 x float]* %ObjectRayOrigin3, i32 0, i32 2
  store float %z, float* %arrayidx4, align 4
  ret void
}

; Function Attrs: nounwind
define float @fb_dxop_objectRayDirection(%struct.RuntimeDataStruct* %runtimeData, i8 zeroext %i) #3 {
entry:
  %idxprom = zext i8 %i to i32
  %ObjectRayDirection = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 8
  %arrayidx = getelementptr inbounds [3 x float], [3 x float]* %ObjectRayDirection, i32 0, i32 %idxprom
  %0 = load float, float* %arrayidx, align 4
  ret float %0
}

; Function Attrs: nounwind
define void @fb_Fallback_SetObjectRayDirection(%struct.RuntimeDataStruct* %runtimeData, float %x, float %y, float %z) #3 {
entry:
  %ObjectRayDirection = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 8
  %arrayidx = getelementptr inbounds [3 x float], [3 x float]* %ObjectRayDirection, i32 0, i32 0
  store float %x, float* %arrayidx, align 4
  %ObjectRayDirection1 = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 8
  %arrayidx2 = getelementptr inbounds [3 x float], [3 x float]* %ObjectRayDirection1, i32 0, i32 1
  store float %y, float* %arrayidx2, align 4
  %ObjectRayDirection3 = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 8
  %arrayidx4 = getelementptr inbounds [3 x float], [3 x float]* %ObjectRayDirection3, i32 0, i32 2
  store float %z, float* %arrayidx4, align 4
  ret void
}

; Function Attrs: nounwind
define float @fb_dxop_objectToWorld(%struct.RuntimeDataStruct* %runtimeData, i32 %r, i8 zeroext %c) #3 {
entry:
  %mul = mul nsw i32 %r, 4
  %conv = zext i8 %c to i32
  %add = add nsw i32 %mul, %conv
  %ObjectToWorld = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 9
  %arrayidx = getelementptr inbounds [12 x float], [12 x float]* %ObjectToWorld, i32 0, i32 %add
  %0 = load float, float* %arrayidx, align 4
  ret float %0
}

; Function Attrs: nounwind
define void @fb_Fallback_SetObjectToWorld(%struct.RuntimeDataStruct* %runtimeData, <12 x float> %M) #3 {
entry:
  %vecext = extractelement <12 x float> %M, i32 0
  %ObjectToWorld = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 9
  %arrayidx = getelementptr inbounds [12 x float], [12 x float]* %ObjectToWorld, i32 0, i32 0
  store float %vecext, float* %arrayidx, align 4
  %vecext1 = extractelement <12 x float> %M, i32 1
  %ObjectToWorld2 = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 9
  %arrayidx3 = getelementptr inbounds [12 x float], [12 x float]* %ObjectToWorld2, i32 0, i32 1
  store float %vecext1, float* %arrayidx3, align 4
  %vecext4 = extractelement <12 x float> %M, i32 2
  %ObjectToWorld5 = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 9
  %arrayidx6 = getelementptr inbounds [12 x float], [12 x float]* %ObjectToWorld5, i32 0, i32 2
  store float %vecext4, float* %arrayidx6, align 4
  %vecext7 = extractelement <12 x float> %M, i32 3
  %ObjectToWorld8 = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 9
  %arrayidx9 = getelementptr inbounds [12 x float], [12 x float]* %ObjectToWorld8, i32 0, i32 3
  store float %vecext7, float* %arrayidx9, align 4
  %vecext10 = extractelement <12 x float> %M, i32 4
  %ObjectToWorld11 = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 9
  %arrayidx12 = getelementptr inbounds [12 x float], [12 x float]* %ObjectToWorld11, i32 0, i32 4
  store float %vecext10, float* %arrayidx12, align 4
  %vecext13 = extractelement <12 x float> %M, i32 5
  %ObjectToWorld14 = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 9
  %arrayidx15 = getelementptr inbounds [12 x float], [12 x float]* %ObjectToWorld14, i32 0, i32 5
  store float %vecext13, float* %arrayidx15, align 4
  %vecext16 = extractelement <12 x float> %M, i32 6
  %ObjectToWorld17 = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 9
  %arrayidx18 = getelementptr inbounds [12 x float], [12 x float]* %ObjectToWorld17, i32 0, i32 6
  store float %vecext16, float* %arrayidx18, align 4
  %vecext19 = extractelement <12 x float> %M, i32 7
  %ObjectToWorld20 = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 9
  %arrayidx21 = getelementptr inbounds [12 x float], [12 x float]* %ObjectToWorld20, i32 0, i32 7
  store float %vecext19, float* %arrayidx21, align 4
  %vecext22 = extractelement <12 x float> %M, i32 8
  %ObjectToWorld23 = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 9
  %arrayidx24 = getelementptr inbounds [12 x float], [12 x float]* %ObjectToWorld23, i32 0, i32 8
  store float %vecext22, float* %arrayidx24, align 4
  %vecext25 = extractelement <12 x float> %M, i32 9
  %ObjectToWorld26 = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 9
  %arrayidx27 = getelementptr inbounds [12 x float], [12 x float]* %ObjectToWorld26, i32 0, i32 9
  store float %vecext25, float* %arrayidx27, align 4
  %vecext28 = extractelement <12 x float> %M, i32 10
  %ObjectToWorld29 = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 9
  %arrayidx30 = getelementptr inbounds [12 x float], [12 x float]* %ObjectToWorld29, i32 0, i32 10
  store float %vecext28, float* %arrayidx30, align 4
  %vecext31 = extractelement <12 x float> %M, i32 11
  %ObjectToWorld32 = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 9
  %arrayidx33 = getelementptr inbounds [12 x float], [12 x float]* %ObjectToWorld32, i32 0, i32 11
  store float %vecext31, float* %arrayidx33, align 4
  ret void
}

; Function Attrs: nounwind
define float @fb_dxop_worldToObject(%struct.RuntimeDataStruct* %runtimeData, i32 %r, i8 zeroext %c) #3 {
entry:
  %mul = mul nsw i32 %r, 4
  %conv = zext i8 %c to i32
  %add = add nsw i32 %mul, %conv
  %WorldToObject = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 10
  %arrayidx = getelementptr inbounds [12 x float], [12 x float]* %WorldToObject, i32 0, i32 %add
  %0 = load float, float* %arrayidx, align 4
  ret float %0
}

; Function Attrs: nounwind
define void @fb_Fallback_SetWorldToObject(%struct.RuntimeDataStruct* %runtimeData, <12 x float> %M) #3 {
entry:
  %vecext = extractelement <12 x float> %M, i32 0
  %WorldToObject = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 10
  %arrayidx = getelementptr inbounds [12 x float], [12 x float]* %WorldToObject, i32 0, i32 0
  store float %vecext, float* %arrayidx, align 4
  %vecext1 = extractelement <12 x float> %M, i32 1
  %WorldToObject2 = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 10
  %arrayidx3 = getelementptr inbounds [12 x float], [12 x float]* %WorldToObject2, i32 0, i32 1
  store float %vecext1, float* %arrayidx3, align 4
  %vecext4 = extractelement <12 x float> %M, i32 2
  %WorldToObject5 = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 10
  %arrayidx6 = getelementptr inbounds [12 x float], [12 x float]* %WorldToObject5, i32 0, i32 2
  store float %vecext4, float* %arrayidx6, align 4
  %vecext7 = extractelement <12 x float> %M, i32 3
  %WorldToObject8 = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 10
  %arrayidx9 = getelementptr inbounds [12 x float], [12 x float]* %WorldToObject8, i32 0, i32 3
  store float %vecext7, float* %arrayidx9, align 4
  %vecext10 = extractelement <12 x float> %M, i32 4
  %WorldToObject11 = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 10
  %arrayidx12 = getelementptr inbounds [12 x float], [12 x float]* %WorldToObject11, i32 0, i32 4
  store float %vecext10, float* %arrayidx12, align 4
  %vecext13 = extractelement <12 x float> %M, i32 5
  %WorldToObject14 = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 10
  %arrayidx15 = getelementptr inbounds [12 x float], [12 x float]* %WorldToObject14, i32 0, i32 5
  store float %vecext13, float* %arrayidx15, align 4
  %vecext16 = extractelement <12 x float> %M, i32 6
  %WorldToObject17 = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 10
  %arrayidx18 = getelementptr inbounds [12 x float], [12 x float]* %WorldToObject17, i32 0, i32 6
  store float %vecext16, float* %arrayidx18, align 4
  %vecext19 = extractelement <12 x float> %M, i32 7
  %WorldToObject20 = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 10
  %arrayidx21 = getelementptr inbounds [12 x float], [12 x float]* %WorldToObject20, i32 0, i32 7
  store float %vecext19, float* %arrayidx21, align 4
  %vecext22 = extractelement <12 x float> %M, i32 8
  %WorldToObject23 = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 10
  %arrayidx24 = getelementptr inbounds [12 x float], [12 x float]* %WorldToObject23, i32 0, i32 8
  store float %vecext22, float* %arrayidx24, align 4
  %vecext25 = extractelement <12 x float> %M, i32 9
  %WorldToObject26 = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 10
  %arrayidx27 = getelementptr inbounds [12 x float], [12 x float]* %WorldToObject26, i32 0, i32 9
  store float %vecext25, float* %arrayidx27, align 4
  %vecext28 = extractelement <12 x float> %M, i32 10
  %WorldToObject29 = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 10
  %arrayidx30 = getelementptr inbounds [12 x float], [12 x float]* %WorldToObject29, i32 0, i32 10
  store float %vecext28, float* %arrayidx30, align 4
  %vecext31 = extractelement <12 x float> %M, i32 11
  %WorldToObject32 = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 10
  %arrayidx33 = getelementptr inbounds [12 x float], [12 x float]* %WorldToObject32, i32 0, i32 11
  store float %vecext31, float* %arrayidx33, align 4
  ret void
}

; Function Attrs: nounwind
define i32 @fb_dxop_primitiveID(%struct.RuntimeDataStruct* %runtimeData) #3 {
entry:
  %PrimitiveIndex = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 11
  %0 = load i32, i32* %PrimitiveIndex, align 4
  ret i32 %0
}

; Function Attrs: nounwind
define void @fb_Fallback_SetPrimitiveIndex(%struct.RuntimeDataStruct* %runtimeData, i32 %i) #3 {
entry:
  %PrimitiveIndex = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 11
  store i32 %i, i32* %PrimitiveIndex, align 4
  ret void
}

; Function Attrs: nounwind
define void @fb_Fallback_SetGeometryIndex(%struct.RuntimeDataStruct* %runtimeData, i32 %geometryIndex) #3 {
entry:
  %GeometryIndex = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 12
  store i32 %geometryIndex, i32* %GeometryIndex, align 4
  ret void
}

; Function Attrs: nounwind
define i32 @fb_Fallback_GeometryIndex(%struct.RuntimeDataStruct* %runtimeData) #3 {
entry:
  %GeometryIndex = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 12
  %0 = load i32, i32* %GeometryIndex, align 4
  ret i32 %0
}

; Function Attrs: nounwind
define i32 @fb_dxop_instanceIndex(%struct.RuntimeDataStruct* %runtimeData) #3 {
entry:
  %InstanceIndex = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 13
  %0 = load i32, i32* %InstanceIndex, align 4
  ret i32 %0
}

; Function Attrs: nounwind
define void @fb_Fallback_SetInstanceIndex(%struct.RuntimeDataStruct* %runtimeData, i32 %i) #3 {
entry:
  %InstanceIndex = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 13
  store i32 %i, i32* %InstanceIndex, align 4
  ret void
}

; Function Attrs: nounwind
define i32 @fb_dxop_instanceID(%struct.RuntimeDataStruct* %runtimeData) #3 {
entry:
  %InstanceID = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 14
  %0 = load i32, i32* %InstanceID, align 4
  ret i32 %0
}

; Function Attrs: nounwind
define void @fb_Fallback_SetInstanceID(%struct.RuntimeDataStruct* %runtimeData, i32 %i) #3 {
entry:
  %InstanceID = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 14
  store i32 %i, i32* %InstanceID, align 4
  ret void
}

; Function Attrs: nounwind
define i32 @fb_dxop_hitKind(%struct.RuntimeDataStruct* %runtimeData) #3 {
entry:
  %HitKind = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 15
  %0 = load i32, i32* %HitKind, align 4
  ret i32 %0
}

; Function Attrs: nounwind
define void @fb_Fallback_SetHitKind(%struct.RuntimeDataStruct* %runtimeData, i32 %i) #3 {
entry:
  %HitKind = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 15
  store i32 %i, i32* %HitKind, align 4
  ret void
}

; Function Attrs: nounwind
define float @fb_dxop_pending_rayTCurrent(%struct.RuntimeDataStruct* %runtimeData) #3 {
entry:
  %PendingRayTCurrent = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 16
  %0 = load float, float* %PendingRayTCurrent, align 4
  ret float %0
}

; Function Attrs: nounwind
define void @fb_Fallback_SetPendingRayTCurrent(%struct.RuntimeDataStruct* %runtimeData, float %t) #3 {
entry:
  %PendingRayTCurrent = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 16
  store float %t, float* %PendingRayTCurrent, align 4
  ret void
}

; Function Attrs: nounwind
define i32 @fb_dxop_pending_primitiveIndex(%struct.RuntimeDataStruct* %runtimeData) #3 {
entry:
  %PendingPrimitiveIndex = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 17
  %0 = load i32, i32* %PendingPrimitiveIndex, align 4
  ret i32 %0
}

; Function Attrs: nounwind
define i32 @fb_Fallback_PendingGeometryIndex(%struct.RuntimeDataStruct* %runtimeData) #3 {
entry:
  %PendingGeometryIndex = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 18
  %0 = load i32, i32* %PendingGeometryIndex, align 4
  ret i32 %0
}

; Function Attrs: nounwind
define i32 @fb_dxop_pending_instanceIndex(%struct.RuntimeDataStruct* %runtimeData) #3 {
entry:
  %PendingInstanceIndex = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 19
  %0 = load i32, i32* %PendingInstanceIndex, align 4
  ret i32 %0
}

; Function Attrs: nounwind
define i32 @fb_dxop_pending_instanceID(%struct.RuntimeDataStruct* %runtimeData) #3 {
entry:
  %PendingInstanceID = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 20
  %0 = load i32, i32* %PendingInstanceID, align 4
  ret i32 %0
}

; Function Attrs: nounwind
define i32 @fb_dxop_pending_hitKind(%struct.RuntimeDataStruct* %runtimeData) #3 {
entry:
  %PendingHitKind = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 21
  %0 = load i32, i32* %PendingHitKind, align 4
  ret i32 %0
}

; Function Attrs: nounwind
define void @fb_Fallback_SetPendingHitKind(%struct.RuntimeDataStruct* %runtimeData, i32 %i) #3 {
entry:
  %PendingHitKind = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 21
  store i32 %i, i32* %PendingHitKind, align 4
  ret void
}

; Function Attrs: nounwind
define i32 @fb_Fallback_GroupIndex(%struct.RuntimeDataStruct* %runtimeData) #3 {
entry:
  %GroupIndex = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 22
  %0 = load i32, i32* %GroupIndex, align 4
  ret i32 %0
}

; Function Attrs: nounwind
define i32 @fb_Fallback_AnyHitResult(%struct.RuntimeDataStruct* %runtimeData) #3 {
entry:
  %AnyHitResult = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 23
  %0 = load i32, i32* %AnyHitResult, align 4
  ret i32 %0
}

; Function Attrs: nounwind
define void @fb_Fallback_SetAnyHitResult(%struct.RuntimeDataStruct* %runtimeData, i32 %result) #3 {
entry:
  %AnyHitResult = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 23
  store i32 %result, i32* %AnyHitResult, align 4
  ret void
}

; Function Attrs: nounwind
define i32 @fb_Fallback_AnyHitStateId(%struct.RuntimeDataStruct* %runtimeData) #3 {
entry:
  %AnyHitStateId = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 24
  %0 = load i32, i32* %AnyHitStateId, align 4
  ret i32 %0
}

; Function Attrs: nounwind
define void @fb_Fallback_SetAnyHitStateId(%struct.RuntimeDataStruct* %runtimeData, i32 %id) #3 {
entry:
  %AnyHitStateId = getelementptr inbounds %struct.RuntimeDataStruct, %struct.RuntimeDataStruct* %runtimeData, i32 0, i32 24
  store i32 %id, i32* %AnyHitStateId, align 4
  ret void
}

define i32 @RayGen.ss_0(%struct.RuntimeDataStruct* %runtimeData) #9 {
RayGen.BB0:
  %payload.offset = call i32 @payloadOffset(%struct.RuntimeDataStruct* %runtimeData)
  %committedAttr.offset = call i32 @committedAttrOffset(%struct.RuntimeDataStruct* %runtimeData)
  %pendingAttr.offset = call i32 @pendingAttrOffset(%struct.RuntimeDataStruct* %runtimeData)
  call void @stackFramePush(%struct.RuntimeDataStruct* %runtimeData, i32 20)
  %stackFrame.offset = call i32 @stackFrameOffset(%struct.RuntimeDataStruct* %runtimeData)
  %v = load %"class.Texture2D<vector<float, 3> >", %"class.Texture2D<vector<float, 3> >"* @"\01?normals@@3V?$Texture2D@V?$vector@M$02@@@@A", align 4
  %v1 = load %"class.Texture2D<float>", %"class.Texture2D<float>"* @"\01?depth@@3V?$Texture2D@M@@A", align 4
  %v2 = load %b1, %b1* @b1, align 4
  %b1 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.b1(i32 160, %b1 %v2)
  %DispatchRaysIndex = call i32 @fb_dxop_dispatchRaysIndex(%struct.RuntimeDataStruct* %runtimeData, i8 0)
  %DispatchRaysIndex1 = call i32 @fb_dxop_dispatchRaysIndex(%struct.RuntimeDataStruct* %runtimeData, i8 1)
  %.i0 = uitofp i32 %DispatchRaysIndex to float
  %.i1 = uitofp i32 %DispatchRaysIndex1 to float
  %.i017 = fadd fast float %.i0, 5.000000e-01
  %.i118 = fadd fast float %.i1, 5.000000e-01
  %v4 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %b1, i32 5)
  %v5 = extractvalue %dx.types.CBufRet.f32 %v4, 0
  %v6 = extractvalue %dx.types.CBufRet.f32 %v4, 1
  %.i019 = fdiv fast float %.i017, %v5
  %.i120 = fdiv fast float %.i118, %v6
  %.i021 = fmul fast float %.i019, 2.000000e+00
  %.i122 = fmul fast float %.i120, 2.000000e+00
  %.i023 = fadd fast float %.i021, -1.000000e+00
  %.i124 = fadd fast float %.i122, -1.000000e+00
  %v7 = fsub fast float -0.000000e+00, %.i124
  %v8 = fptosi float %.i017 to i32
  %v9 = fptosi float %.i118 to i32
  %v10 = call %dx.types.Handle @"dx.op.createHandleFromResourceStructForLib.class.Texture2D<float>"(i32 160, %"class.Texture2D<float>" %v1)
  %TextureLoad = call %dx.types.ResRet.f32 @dx.op.textureLoad.f32(i32 66, %dx.types.Handle %v10, i32 0, i32 %v8, i32 %v9, i32 undef, i32 undef, i32 undef, i32 undef)
  %v11 = extractvalue %dx.types.ResRet.f32 %TextureLoad, 0
  %v12 = call %dx.types.Handle @"dx.op.createHandleFromResourceStructForLib.class.Texture2D<vector<float, 3> >"(i32 160, %"class.Texture2D<vector<float, 3> >" %v)
  %TextureLoad2 = call %dx.types.ResRet.f32 @dx.op.textureLoad.f32(i32 66, %dx.types.Handle %v12, i32 0, i32 %v8, i32 %v9, i32 undef, i32 undef, i32 undef, i32 undef)
  %v13 = extractvalue %dx.types.ResRet.f32 %TextureLoad2, 0
  %v14 = extractvalue %dx.types.ResRet.f32 %TextureLoad2, 1
  %v15 = extractvalue %dx.types.ResRet.f32 %TextureLoad2, 2
  %v16 = fmul fast float %v13, %v13
  %v17 = fmul fast float %v14, %v14
  %v18 = fadd fast float %v16, %v17
  %v19 = fmul fast float %v15, %v15
  %v20 = fadd fast float %v18, %v19
  %Sqrt = call float @dx.op.unary.f32(i32 24, float %v20)
  %v21 = fcmp fast olt float %Sqrt, 0x3FB99999A0000000
  br i1 %v21, label %8, label %remat_begin

remat_begin:                                      ; preds = %RayGen.BB0
  %v22 = load %struct.RaytracingAccelerationStructure, %struct.RaytracingAccelerationStructure* @"\01?g_accel@@3URaytracingAccelerationStructure@@A", align 4
  %.i025 = fdiv fast float %v13, %Sqrt
  %.i126 = fdiv fast float %v14, %Sqrt
  %.i2 = fdiv fast float %v15, %Sqrt
  %v23 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %b1, i32 0)
  %v24 = extractvalue %dx.types.CBufRet.f32 %v23, 0
  %v25 = extractvalue %dx.types.CBufRet.f32 %v23, 1
  %v26 = extractvalue %dx.types.CBufRet.f32 %v23, 2
  %v27 = extractvalue %dx.types.CBufRet.f32 %v23, 3
  %v28 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %b1, i32 1)
  %v29 = extractvalue %dx.types.CBufRet.f32 %v28, 0
  %v30 = extractvalue %dx.types.CBufRet.f32 %v28, 1
  %v31 = extractvalue %dx.types.CBufRet.f32 %v28, 2
  %v32 = extractvalue %dx.types.CBufRet.f32 %v28, 3
  %v33 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %b1, i32 2)
  %v34 = extractvalue %dx.types.CBufRet.f32 %v33, 0
  %v35 = extractvalue %dx.types.CBufRet.f32 %v33, 1
  %v36 = extractvalue %dx.types.CBufRet.f32 %v33, 2
  %v37 = extractvalue %dx.types.CBufRet.f32 %v33, 3
  %v38 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %b1, i32 3)
  %v39 = extractvalue %dx.types.CBufRet.f32 %v38, 0
  %v40 = extractvalue %dx.types.CBufRet.f32 %v38, 1
  %v41 = extractvalue %dx.types.CBufRet.f32 %v38, 2
  %v42 = extractvalue %dx.types.CBufRet.f32 %v38, 3
  %v43 = fmul fast float %v24, %.i023
  %FMad16 = call float @dx.op.tertiary.f32(i32 46, float %v7, float %v25, float %v43)
  %FMad15 = call float @dx.op.tertiary.f32(i32 46, float %v11, float %v26, float %FMad16)
  %FMad14 = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %v27, float %FMad15)
  %v44 = fmul fast float %v29, %.i023
  %FMad13 = call float @dx.op.tertiary.f32(i32 46, float %v7, float %v30, float %v44)
  %FMad12 = call float @dx.op.tertiary.f32(i32 46, float %v11, float %v31, float %FMad13)
  %FMad11 = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %v32, float %FMad12)
  %v45 = fmul fast float %v34, %.i023
  %FMad10 = call float @dx.op.tertiary.f32(i32 46, float %v7, float %v35, float %v45)
  %FMad9 = call float @dx.op.tertiary.f32(i32 46, float %v11, float %v36, float %FMad10)
  %FMad8 = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %v37, float %FMad9)
  %v46 = fmul fast float %v39, %.i023
  %FMad7 = call float @dx.op.tertiary.f32(i32 46, float %v7, float %v40, float %v46)
  %FMad6 = call float @dx.op.tertiary.f32(i32 46, float %v11, float %v41, float %FMad7)
  %FMad = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %v42, float %FMad6)
  %.i027 = fdiv fast float %FMad14, %FMad
  %.i128 = fdiv fast float %FMad11, %FMad
  %.i229 = fdiv fast float %FMad8, %FMad
  %v47 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %b1, i32 4)
  %v48 = extractvalue %dx.types.CBufRet.f32 %v47, 0
  %v49 = extractvalue %dx.types.CBufRet.f32 %v47, 1
  %v50 = extractvalue %dx.types.CBufRet.f32 %v47, 2
  %.i030 = fsub fast float %v48, %.i027
  %.i131 = fsub fast float %v49, %.i128
  %.i232 = fsub fast float %v50, %.i229
  %v51 = fmul fast float %.i030, %.i030
  %v52 = fmul fast float %.i131, %.i131
  %v53 = fadd fast float %v51, %v52
  %v54 = fmul fast float %.i232, %.i232
  %v55 = fadd fast float %v53, %v54
  %Sqrt4 = call float @dx.op.unary.f32(i32 24, float %v55)
  %.i033 = fdiv fast float %.i030, %Sqrt4
  %.i134 = fdiv fast float %.i131, %Sqrt4
  %.i235 = fdiv fast float %.i232, %Sqrt4
  %.i036 = fsub fast float -0.000000e+00, %.i033
  %.i137 = fsub fast float -0.000000e+00, %.i134
  %.i238 = fsub fast float -0.000000e+00, %.i235
  %v56 = call float @dx.op.dot3.f32(i32 55, float %.i036, float %.i137, float %.i238, float %.i025, float %.i126, float %.i2)
  %v57 = fmul fast float %v56, 2.000000e+00
  %.i042 = fmul fast float %v57, %.i025
  %.i143 = fmul fast float %v57, %.i126
  %.i244 = fmul fast float %v57, %.i2
  %.i045 = fsub fast float %.i036, %.i042
  %.i146 = fsub fast float %.i137, %.i143
  %.i247 = fsub fast float %.i238, %.i244
  %v58 = fmul fast float %.i045, %.i045
  %v59 = fmul fast float %.i146, %.i146
  %v60 = fadd fast float %v58, %v59
  %v61 = fmul fast float %.i247, %.i247
  %v62 = fadd fast float %v60, %v61
  %Sqrt5 = call float @dx.op.unary.f32(i32 24, float %v62)
  %.i048 = fdiv fast float %.i045, %Sqrt5
  %.i149 = fdiv fast float %.i146, %Sqrt5
  %.i250 = fdiv fast float %.i247, %Sqrt5
  %.i051 = fmul fast float %.i033, 0x3FB99999A0000000
  %.i152 = fmul fast float %.i134, 0x3FB99999A0000000
  %.i253 = fmul fast float %.i235, 0x3FB99999A0000000
  %.i054 = fsub fast float %.i027, %.i051
  %.i155 = fsub fast float %.i128, %.i152
  %.i256 = fsub fast float %.i229, %.i253
  %offs = add i32 16, 0
  %v63 = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %stackFrame.offset, i32 %offs)
  store i32 0, i32* %v63, align 8
  %offs15 = add i32 16, 1
  %v64.ptr = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %stackFrame.offset, i32 %offs15)
  %v64 = bitcast i32* %v64.ptr to float*
  store float 0x47EFFFFFE0000000, float* %v64, align 4
  %v65 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RaytracingAccelerationStructure(i32 160, %struct.RaytracingAccelerationStructure %v22)
  %new.payload.offset = add i32 %stackFrame.offset, 16
  %intPtr6 = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %stackFrame.offset, i32 0)
  store i32 1001, i32* %intPtr6
  %intPtr5 = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %stackFrame.offset, i32 1)
  store i32 16, i32* %intPtr5
  %intPtr4 = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %stackFrame.offset, i32 2)
  store i32 -1, i32* %intPtr4
  %intPtr3 = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %stackFrame.offset, i32 3)
  store i32 0, i32* %intPtr3
  %intPtr2 = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %stackFrame.offset, i32 4)
  store i32 1, i32* %intPtr2
  %intPtr1 = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %stackFrame.offset, i32 5)
  store i32 0, i32* %intPtr1
  %0 = bitcast float %.i054 to i32
  %intPtr14 = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %stackFrame.offset, i32 6)
  store i32 %0, i32* %intPtr14
  %1 = bitcast float %.i155 to i32
  %intPtr13 = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %stackFrame.offset, i32 7)
  store i32 %1, i32* %intPtr13
  %2 = bitcast float %.i256 to i32
  %intPtr12 = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %stackFrame.offset, i32 8)
  store i32 %2, i32* %intPtr12
  %3 = bitcast float 0.000000e+00 to i32
  %intPtr11 = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %stackFrame.offset, i32 9)
  store i32 %3, i32* %intPtr11
  %4 = bitcast float %.i048 to i32
  %intPtr10 = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %stackFrame.offset, i32 10)
  store i32 %4, i32* %intPtr10
  %5 = bitcast float %.i149 to i32
  %intPtr9 = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %stackFrame.offset, i32 11)
  store i32 %5, i32* %intPtr9
  %6 = bitcast float %.i250 to i32
  %intPtr8 = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %stackFrame.offset, i32 12)
  store i32 %6, i32* %intPtr8
  %7 = bitcast float 0x47EFFFFFE0000000 to i32
  %intPtr7 = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %stackFrame.offset, i32 13)
  store i32 %7, i32* %intPtr7
  %intPtr = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %stackFrame.offset, i32 14)
  store i32 %new.payload.offset, i32* %intPtr
  ret i32 1005

; <label>:8                                       ; preds = %RayGen.BB0
  call void @stackFramePop(%struct.RuntimeDataStruct* %runtimeData, i32 20)
  %ret.stackFrame.offset = call i32 @stackFrameOffset(%struct.RuntimeDataStruct* %runtimeData)
  %ret.stateId.ptr = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %ret.stackFrame.offset, i32 0)
  %ret.stateId = load i32, i32* %ret.stateId.ptr
  ret i32 %ret.stateId
}

define i32 @RayGen.ss_1(%struct.RuntimeDataStruct* %runtimeData) #9 {
RayGen.BB1.from.Fallback_TraceRay:
  %stackFrame.offset.remat = call i32 @stackFrameOffset(%struct.RuntimeDataStruct* %runtimeData)
  br label %0

; <label>:0                                       ; preds = %RayGen.BB1.from.Fallback_TraceRay
  call void @stackFramePop(%struct.RuntimeDataStruct* %runtimeData, i32 20)
  %ret.stackFrame.offset = call i32 @stackFrameOffset(%struct.RuntimeDataStruct* %runtimeData)
  %ret.stateId.ptr = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %ret.stackFrame.offset, i32 0)
  %ret.stateId = load i32, i32* %ret.stateId.ptr
  ret i32 %ret.stateId
}

define i32 @Hit.ss_0(%struct.RuntimeDataStruct* %runtimeData) #9 {
Hit.BB0:
  %payload.offset = call i32 @payloadOffset(%struct.RuntimeDataStruct* %runtimeData)
  %committedAttr.offset = call i32 @committedAttrOffset(%struct.RuntimeDataStruct* %runtimeData)
  %pendingAttr.offset = call i32 @pendingAttrOffset(%struct.RuntimeDataStruct* %runtimeData)
  call void @stackFramePush(%struct.RuntimeDataStruct* %runtimeData, i32 32)
  %stackFrame.offset = call i32 @stackFrameOffset(%struct.RuntimeDataStruct* %runtimeData)
  %LoadShaderTableHandle = load %struct.ByteAddressBuffer, %struct.ByteAddressBuffer* @"\01?HitGroupShaderTable@@3UByteAddressBuffer@@A"
  %ShaderTableHandle = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.ByteAddressBuffer(i32 160, %struct.ByteAddressBuffer %LoadShaderTableHandle)
  %DispatchRaysConstants = load %Constants, %Constants* @Constants
  %Constants = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.Constants(i32 160, %Constants %DispatchRaysConstants)
  %geometry.index = call i32 @fb_Fallback_GeometryIndex(%struct.RuntimeDataStruct* %runtimeData)
  %ConstantBuffer = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %Constants, i32 0)
  %ShaderTableStride = extractvalue %dx.types.CBufRet.i32 %ConstantBuffer, 2
  %baseShaderRecordOffset = mul i32 %geometry.index, %ShaderTableStride
  %v = load %struct.SamplerComparisonState, %struct.SamplerComparisonState* @"\01?shadowSampler@@3USamplerComparisonState@@A", align 4
  %v1 = load %struct.SamplerState, %struct.SamplerState* @"\01?g_s0@@3USamplerState@@A", align 4
  %v2 = load %"class.Texture2D<float>", %"class.Texture2D<float>"* @"\01?texSSAO@@3V?$Texture2D@M@@A", align 4
  %v3 = load %"class.Texture2D<float>", %"class.Texture2D<float>"* @"\01?texShadow@@3V?$Texture2D@M@@A", align 4
  %v4 = load %struct.ByteAddressBuffer, %struct.ByteAddressBuffer* @"\01?g_attributes@@3UByteAddressBuffer@@A", align 4
  %v5 = load %struct.ByteAddressBuffer, %struct.ByteAddressBuffer* @"\01?g_indices@@3UByteAddressBuffer@@A", align 4
  %v6 = load %"class.StructuredBuffer<RayTraceMeshInfo>", %"class.StructuredBuffer<RayTraceMeshInfo>"* @"\01?g_meshInfo@@3V?$StructuredBuffer@URayTraceMeshInfo@@@@A", align 4
  %v7 = load %struct.RaytracingAccelerationStructure, %struct.RaytracingAccelerationStructure* @"\01?g_accel@@3URaytracingAccelerationStructure@@A", align 4
  %v8 = load %"class.RWTexture2D<vector<float, 4> >", %"class.RWTexture2D<vector<float, 4> >"* @"\01?g_screenOutput@@3V?$RWTexture2D@V?$vector@M$03@@@@A", align 4
  %v9 = load %b1, %b1* @b1, align 4
  %v10 = load %HitShaderConstants, %HitShaderConstants* @HitShaderConstants, align 4
  %b1115 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.b1(i32 160, %b1 %v9)
  %HitShaderConstants114 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.HitShaderConstants(i32 160, %HitShaderConstants %v10)
  %v11 = alloca [3 x float], align 4
  %v12 = alloca [3 x float], align 4
  %v13 = alloca [3 x float], align 4
  %v14 = alloca [3 x float], align 4
  %v15 = alloca [3 x float], align 4
  %RayTCurrent = call float @fb_dxop_rayTCurrent(%struct.RuntimeDataStruct* %runtimeData)
  %offs29 = add i32 0, 1
  %v17.ptr = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %payload.offset, i32 %offs29)
  %v17 = bitcast i32* %v17.ptr to float*
  store float %RayTCurrent, float* %v17, align 4, !tbaa !231
  %offs = add i32 0, 0
  %v18 = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %payload.offset, i32 %offs)
  %v19 = load i32, i32* %v18, align 1, !tbaa !332, !range !334
  %v20 = icmp eq i32 %v19, 0
  br i1 %v20, label %"\01?Load3x16BitIndices@@YA?AV?$vector@I$02@@I@Z.exit", label %Hit.BB0._crit_edge

Hit.BB0._crit_edge:                               ; preds = %Hit.BB0
  br label %0

; <label>:0                                       ; preds = %Hit.BB0._crit_edge, %26
  call void @stackFramePop(%struct.RuntimeDataStruct* %runtimeData, i32 32)
  %ret.stackFrame.offset = call i32 @stackFrameOffset(%struct.RuntimeDataStruct* %runtimeData)
  %ret.stateId.ptr = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %ret.stackFrame.offset, i32 0)
  %ret.stateId = load i32, i32* %ret.stateId.ptr
  ret i32 %ret.stateId

"\01?Load3x16BitIndices@@YA?AV?$vector@I$02@@I@Z.exit": ; preds = %Hit.BB0
  %v21 = add i32 8, %baseShaderRecordOffset
  %ShaderRecordBuffer = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %ShaderTableHandle, i32 %v21, i32 undef)
  %v22 = extractvalue %dx.types.ResRet.i32 %ShaderRecordBuffer, 0
  %v23 = call %dx.types.Handle @"dx.op.createHandleFromResourceStructForLib.class.StructuredBuffer<RayTraceMeshInfo>"(i32 160, %"class.StructuredBuffer<RayTraceMeshInfo>" %v6)
  %v24 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %v23, i32 %v22, i32 0)
  %v25 = extractvalue %dx.types.ResRet.i32 %v24, 0
  %v26 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %v23, i32 %v22, i32 4)
  %v27 = extractvalue %dx.types.ResRet.i32 %v26, 0
  %v28 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %v23, i32 %v22, i32 8)
  %v29 = extractvalue %dx.types.ResRet.i32 %v28, 0
  %v30 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %v23, i32 %v22, i32 12)
  %v31 = extractvalue %dx.types.ResRet.i32 %v30, 0
  %v32 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %v23, i32 %v22, i32 16)
  %v33 = extractvalue %dx.types.ResRet.i32 %v32, 0
  %v34 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %v23, i32 %v22, i32 20)
  %v35 = extractvalue %dx.types.ResRet.i32 %v34, 0
  %v36 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %v23, i32 %v22, i32 24)
  %v37 = extractvalue %dx.types.ResRet.i32 %v36, 0
  %PrimitiveID82 = call i32 @fb_dxop_primitiveID(%struct.RuntimeDataStruct* %runtimeData)
  %v38 = mul i32 %PrimitiveID82, 6
  %v39 = add i32 %v38, %v25
  %v40 = and i32 %v39, -4
  %v41 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.ByteAddressBuffer(i32 160, %struct.ByteAddressBuffer %v5)
  %v42 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %v41, i32 %v40, i32 undef)
  %v43 = extractvalue %dx.types.ResRet.i32 %v42, 0
  %v44 = extractvalue %dx.types.ResRet.i32 %v42, 1
  %v45 = icmp eq i32 %v40, %v39
  %v46 = and i32 %v43, 65535
  %v47 = lshr i32 %v43, 16
  %v48 = lshr i32 %v43, 16
  %v49 = lshr i32 %v44, 16
  %.sink = select i1 %v45, i32 %v44, i32 %v49
  %.0.i0 = select i1 %v45, i32 %v46, i32 %v48
  %.0.i1.in = select i1 %v45, i32 %v47, i32 %v44
  %.0.i1 = and i32 %.0.i1.in, 65535
  %v50 = and i32 %.sink, 65535
  %v51 = mul i32 %.0.i0, %v35
  %v52 = add i32 %v51, %v27
  %v53 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.ByteAddressBuffer(i32 160, %struct.ByteAddressBuffer %v4)
  %v54 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %v53, i32 %v52, i32 undef)
  %v55 = extractvalue %dx.types.ResRet.i32 %v54, 0
  %v56 = extractvalue %dx.types.ResRet.i32 %v54, 1
  %.i0 = bitcast i32 %v55 to float
  %.i1 = bitcast i32 %v56 to float
  %v57 = mul i32 %.0.i1, %v35
  %v58 = add i32 %v57, %v27
  %v59 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %v53, i32 %v58, i32 undef)
  %v60 = extractvalue %dx.types.ResRet.i32 %v59, 0
  %v61 = extractvalue %dx.types.ResRet.i32 %v59, 1
  %.i0116 = bitcast i32 %v60 to float
  %.i1117 = bitcast i32 %v61 to float
  %v62 = mul i32 %v50, %v35
  %v63 = add i32 %v62, %v27
  %v64 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %v53, i32 %v63, i32 undef)
  %v65 = extractvalue %dx.types.ResRet.i32 %v64, 0
  %v66 = extractvalue %dx.types.ResRet.i32 %v64, 1
  %.i0118 = bitcast i32 %v65 to float
  %.i1119 = bitcast i32 %v66 to float
  %offs32 = add i32 0, 0
  %v67.v.ptr = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %committedAttr.offset, i32 %offs32)
  %v67.v = bitcast i32* %v67.v.ptr to float*
  %offs33 = add i32 %offs32, 1
  %v67.v34.ptr = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %committedAttr.offset, i32 %offs33)
  %v67.v34 = bitcast i32* %v67.v34.ptr to float*
  %el = load float, float* %v67.v
  %vec = insertelement <2 x float> undef, float %el, i32 0
  %el35 = load float, float* %v67.v34
  %vec36 = insertelement <2 x float> %vec, float %el35, i32 1
  %v69 = extractelement <2 x float> %vec36, i32 0
  %v70 = fsub fast float 1.000000e+00, %v69
  %v71 = extractelement <2 x float> %vec36, i32 1
  %v72 = fsub fast float %v70, %v71
  %.i0120 = fmul fast float %v72, %.i0
  %.i1121 = fmul fast float %v72, %.i1
  %.i0122 = fmul fast float %v69, %.i0116
  %.i1123 = fmul fast float %v69, %.i1117
  %.i0126 = fmul fast float %v71, %.i0118
  %.i1127 = fmul fast float %v71, %.i1119
  %.i0124 = fadd fast float %.i0126, %.i0122
  %.i0128 = fadd fast float %.i0124, %.i0120
  %.i1125 = fadd fast float %.i1127, %.i1123
  %.i1129 = fadd fast float %.i1125, %.i1121
  %v73 = add i32 %v51, %v29
  %v74 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.ByteAddressBuffer(i32 160, %struct.ByteAddressBuffer %v4)
  %v75 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %v74, i32 %v73, i32 undef)
  %v76 = extractvalue %dx.types.ResRet.i32 %v75, 0
  %v77 = extractvalue %dx.types.ResRet.i32 %v75, 1
  %v78 = extractvalue %dx.types.ResRet.i32 %v75, 2
  %.i0130 = bitcast i32 %v76 to float
  %.i1131 = bitcast i32 %v77 to float
  %.i2 = bitcast i32 %v78 to float
  %v79 = add i32 %v57, %v29
  %v80 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %v74, i32 %v79, i32 undef)
  %v81 = extractvalue %dx.types.ResRet.i32 %v80, 0
  %v82 = extractvalue %dx.types.ResRet.i32 %v80, 1
  %v83 = extractvalue %dx.types.ResRet.i32 %v80, 2
  %.i0132 = bitcast i32 %v81 to float
  %.i1133 = bitcast i32 %v82 to float
  %.i2134 = bitcast i32 %v83 to float
  %v84 = add i32 %v62, %v29
  %v85 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %v74, i32 %v84, i32 undef)
  %v86 = extractvalue %dx.types.ResRet.i32 %v85, 0
  %v87 = extractvalue %dx.types.ResRet.i32 %v85, 1
  %v88 = extractvalue %dx.types.ResRet.i32 %v85, 2
  %.i0135 = bitcast i32 %v86 to float
  %.i1136 = bitcast i32 %v87 to float
  %.i2137 = bitcast i32 %v88 to float
  %.i0138 = fmul fast float %.i0130, %v72
  %.i1139 = fmul fast float %.i1131, %v72
  %.i2140 = fmul fast float %.i2, %v72
  %.i0141 = fmul fast float %.i0132, %v69
  %.i1142 = fmul fast float %.i1133, %v69
  %.i2143 = fmul fast float %.i2134, %v69
  %.i0144 = fadd fast float %.i0141, %.i0138
  %.i1145 = fadd fast float %.i1142, %.i1139
  %.i2146 = fadd fast float %.i2143, %.i2140
  %.i0147 = fmul fast float %.i0135, %v71
  %.i1148 = fmul fast float %.i1136, %v71
  %.i2149 = fmul fast float %.i2137, %v71
  %.i0150 = fadd fast float %.i0144, %.i0147
  %.i1151 = fadd fast float %.i1145, %.i1148
  %.i2152 = fadd fast float %.i2146, %.i2149
  %v89 = fmul fast float %.i0150, %.i0150
  %v90 = fmul fast float %.i1151, %.i1151
  %v91 = fadd fast float %v89, %v90
  %v92 = fmul fast float %.i2152, %.i2152
  %v93 = fadd fast float %v91, %v92
  %Sqrt72 = call float @dx.op.unary.f32(i32 24, float %v93)
  %.i0153 = fdiv fast float %.i0150, %Sqrt72
  %.i1154 = fdiv fast float %.i1151, %Sqrt72
  %.i2155 = fdiv fast float %.i2152, %Sqrt72
  %v94 = add i32 %v51, %v31
  %v95 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %v74, i32 %v94, i32 undef)
  %v96 = extractvalue %dx.types.ResRet.i32 %v95, 0
  %v97 = extractvalue %dx.types.ResRet.i32 %v95, 1
  %v98 = extractvalue %dx.types.ResRet.i32 %v95, 2
  %.i0156 = bitcast i32 %v96 to float
  %.i1157 = bitcast i32 %v97 to float
  %.i2158 = bitcast i32 %v98 to float
  %v99 = add i32 %v57, %v31
  %v100 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %v74, i32 %v99, i32 undef)
  %v101 = extractvalue %dx.types.ResRet.i32 %v100, 0
  %v102 = extractvalue %dx.types.ResRet.i32 %v100, 1
  %v103 = extractvalue %dx.types.ResRet.i32 %v100, 2
  %.i0159 = bitcast i32 %v101 to float
  %.i1160 = bitcast i32 %v102 to float
  %.i2161 = bitcast i32 %v103 to float
  %v104 = add i32 %v62, %v31
  %v105 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %v74, i32 %v104, i32 undef)
  %v106 = extractvalue %dx.types.ResRet.i32 %v105, 0
  %v107 = extractvalue %dx.types.ResRet.i32 %v105, 1
  %v108 = extractvalue %dx.types.ResRet.i32 %v105, 2
  %.i0162 = bitcast i32 %v106 to float
  %.i1163 = bitcast i32 %v107 to float
  %.i2164 = bitcast i32 %v108 to float
  %.i0165 = fmul fast float %.i0156, %v72
  %.i1166 = fmul fast float %.i1157, %v72
  %.i2167 = fmul fast float %.i2158, %v72
  %.i0168 = fmul fast float %.i0159, %v69
  %.i1169 = fmul fast float %.i1160, %v69
  %.i2170 = fmul fast float %.i2161, %v69
  %.i0171 = fadd fast float %.i0168, %.i0165
  %.i1172 = fadd fast float %.i1169, %.i1166
  %.i2173 = fadd fast float %.i2170, %.i2167
  %.i0174 = fmul fast float %.i0162, %v71
  %.i1175 = fmul fast float %.i1163, %v71
  %.i2176 = fmul fast float %.i2164, %v71
  %.i0177 = fadd fast float %.i0171, %.i0174
  %.i1178 = fadd fast float %.i1172, %.i1175
  %.i2179 = fadd fast float %.i2173, %.i2176
  %v109 = fmul fast float %.i0177, %.i0177
  %v110 = fmul fast float %.i1178, %.i1178
  %v111 = fadd fast float %v109, %v110
  %v112 = fmul fast float %.i2179, %.i2179
  %v113 = fadd fast float %v111, %v112
  %Sqrt73 = call float @dx.op.unary.f32(i32 24, float %v113)
  %.i0180 = fdiv fast float %.i0177, %Sqrt73
  %.i1181 = fdiv fast float %.i1178, %Sqrt73
  %.i2182 = fdiv fast float %.i2179, %Sqrt73
  %v114 = add i32 %v51, %v33
  %v115 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %v74, i32 %v114, i32 undef)
  %v116 = extractvalue %dx.types.ResRet.i32 %v115, 0
  %v117 = extractvalue %dx.types.ResRet.i32 %v115, 1
  %v118 = extractvalue %dx.types.ResRet.i32 %v115, 2
  %.i0183 = bitcast i32 %v116 to float
  %.i1184 = bitcast i32 %v117 to float
  %.i2185 = bitcast i32 %v118 to float
  %v119 = add i32 %v57, %v33
  %v120 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.ByteAddressBuffer(i32 160, %struct.ByteAddressBuffer %v4)
  %v121 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %v120, i32 %v119, i32 undef)
  %v122 = extractvalue %dx.types.ResRet.i32 %v121, 0
  %v123 = extractvalue %dx.types.ResRet.i32 %v121, 1
  %v124 = extractvalue %dx.types.ResRet.i32 %v121, 2
  %.i0186 = bitcast i32 %v122 to float
  %.i1187 = bitcast i32 %v123 to float
  %.i2188 = bitcast i32 %v124 to float
  %v125 = add i32 %v62, %v33
  %v126 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %v120, i32 %v125, i32 undef)
  %v127 = extractvalue %dx.types.ResRet.i32 %v126, 0
  %v128 = extractvalue %dx.types.ResRet.i32 %v126, 1
  %v129 = extractvalue %dx.types.ResRet.i32 %v126, 2
  %.i0189 = bitcast i32 %v127 to float
  %.i1190 = bitcast i32 %v128 to float
  %.i2191 = bitcast i32 %v129 to float
  %WorldRayOrigin = call float @fb_dxop_worldRayOrigin(%struct.RuntimeDataStruct* %runtimeData, i8 0)
  %WorldRayOrigin104 = call float @fb_dxop_worldRayOrigin(%struct.RuntimeDataStruct* %runtimeData, i8 1)
  %WorldRayOrigin105 = call float @fb_dxop_worldRayOrigin(%struct.RuntimeDataStruct* %runtimeData, i8 2)
  %WorldRayDirection = call float @fb_dxop_worldRayDirection(%struct.RuntimeDataStruct* %runtimeData, i8 0)
  %WorldRayDirection99 = call float @fb_dxop_worldRayDirection(%struct.RuntimeDataStruct* %runtimeData, i8 1)
  %WorldRayDirection100 = call float @fb_dxop_worldRayDirection(%struct.RuntimeDataStruct* %runtimeData, i8 2)
  %RayTCurrent81 = call float @fb_dxop_rayTCurrent(%struct.RuntimeDataStruct* %runtimeData)
  %.i0192 = fmul fast float %RayTCurrent81, %WorldRayDirection
  %.i1193 = fmul fast float %RayTCurrent81, %WorldRayDirection99
  %.i2194 = fmul fast float %RayTCurrent81, %WorldRayDirection100
  %.i0195 = fadd fast float %.i0192, %WorldRayOrigin
  %.i1196 = fadd fast float %.i1193, %WorldRayOrigin104
  %.i2197 = fadd fast float %.i2194, %WorldRayOrigin105
  %DispatchRaysIndex = call i32 @fb_dxop_dispatchRaysIndex(%struct.RuntimeDataStruct* %runtimeData, i8 0)
  %DispatchRaysIndex106 = call i32 @fb_dxop_dispatchRaysIndex(%struct.RuntimeDataStruct* %runtimeData, i8 1)
  %v130 = add i32 %DispatchRaysIndex, 1
  %.i0198 = uitofp i32 %v130 to float
  %.i1199 = uitofp i32 %DispatchRaysIndex106 to float
  %.i0200 = fadd fast float %.i0198, 5.000000e-01
  %.i1201 = fadd fast float %.i1199, 5.000000e-01
  %v131 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %b1115, i32 5)
  %v132 = extractvalue %dx.types.CBufRet.f32 %v131, 0
  %v133 = extractvalue %dx.types.CBufRet.f32 %v131, 1
  %.i0202 = fdiv fast float %.i0200, %v132
  %.i1203 = fdiv fast float %.i1201, %v133
  %.i0204 = fmul fast float %.i0202, 2.000000e+00
  %.i1205 = fmul fast float %.i1203, 2.000000e+00
  %.i0206 = fadd fast float %.i0204, -1.000000e+00
  %.i1207 = fadd fast float %.i1205, -1.000000e+00
  %v134 = fsub fast float -0.000000e+00, %.i1207
  %v135 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %b1115, i32 0)
  %v136 = extractvalue %dx.types.CBufRet.f32 %v135, 0
  %v137 = extractvalue %dx.types.CBufRet.f32 %v135, 1
  %v138 = extractvalue %dx.types.CBufRet.f32 %v135, 2
  %v139 = extractvalue %dx.types.CBufRet.f32 %v135, 3
  %v140 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %b1115, i32 1)
  %v141 = extractvalue %dx.types.CBufRet.f32 %v140, 0
  %v142 = extractvalue %dx.types.CBufRet.f32 %v140, 1
  %v143 = extractvalue %dx.types.CBufRet.f32 %v140, 2
  %v144 = extractvalue %dx.types.CBufRet.f32 %v140, 3
  %v145 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %b1115, i32 2)
  %v146 = extractvalue %dx.types.CBufRet.f32 %v145, 0
  %v147 = extractvalue %dx.types.CBufRet.f32 %v145, 1
  %v148 = extractvalue %dx.types.CBufRet.f32 %v145, 2
  %v149 = extractvalue %dx.types.CBufRet.f32 %v145, 3
  %v150 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %b1115, i32 3)
  %v151 = extractvalue %dx.types.CBufRet.f32 %v150, 0
  %v152 = extractvalue %dx.types.CBufRet.f32 %v150, 1
  %v153 = extractvalue %dx.types.CBufRet.f32 %v150, 2
  %v154 = extractvalue %dx.types.CBufRet.f32 %v150, 3
  %v155 = fmul fast float %v136, %.i0206
  %FMad33 = call float @dx.op.tertiary.f32(i32 46, float %v134, float %v137, float %v155)
  %FMad32 = call float @dx.op.tertiary.f32(i32 46, float 0.000000e+00, float %v138, float %FMad33)
  %FMad31 = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %v139, float %FMad32)
  %v156 = fmul fast float %v141, %.i0206
  %FMad30 = call float @dx.op.tertiary.f32(i32 46, float %v134, float %v142, float %v156)
  %FMad29 = call float @dx.op.tertiary.f32(i32 46, float 0.000000e+00, float %v143, float %FMad30)
  %FMad28 = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %v144, float %FMad29)
  %v157 = fmul fast float %v146, %.i0206
  %FMad27 = call float @dx.op.tertiary.f32(i32 46, float %v134, float %v147, float %v157)
  %FMad26 = call float @dx.op.tertiary.f32(i32 46, float 0.000000e+00, float %v148, float %FMad27)
  %FMad25 = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %v149, float %FMad26)
  %v158 = fmul fast float %v151, %.i0206
  %FMad24 = call float @dx.op.tertiary.f32(i32 46, float %v134, float %v152, float %v158)
  %FMad23 = call float @dx.op.tertiary.f32(i32 46, float 0.000000e+00, float %v153, float %FMad24)
  %FMad22 = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %v154, float %FMad23)
  %.i0208 = fdiv fast float %FMad31, %FMad22
  %.i1209 = fdiv fast float %FMad28, %FMad22
  %.i2210 = fdiv fast float %FMad25, %FMad22
  %v159 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %b1115, i32 4)
  %v160 = extractvalue %dx.types.CBufRet.f32 %v159, 0
  %v161 = extractvalue %dx.types.CBufRet.f32 %v159, 1
  %v162 = extractvalue %dx.types.CBufRet.f32 %v159, 2
  %.i0211 = fsub fast float %.i0208, %v160
  %.i1212 = fsub fast float %.i1209, %v161
  %.i2213 = fsub fast float %.i2210, %v162
  %v163 = fmul fast float %.i0211, %.i0211
  %v164 = fmul fast float %.i1212, %.i1212
  %v165 = fadd fast float %v163, %v164
  %v166 = fmul fast float %.i2213, %.i2213
  %v167 = fadd fast float %v165, %v166
  %Sqrt71 = call float @dx.op.unary.f32(i32 24, float %v167)
  %.i0214 = fdiv fast float %.i0211, %Sqrt71
  %.i1215 = fdiv fast float %.i1212, %Sqrt71
  %.i2216 = fdiv fast float %.i2213, %Sqrt71
  %v168 = add i32 %DispatchRaysIndex106, 1
  %.i0217 = uitofp i32 %DispatchRaysIndex to float
  %.i1218 = uitofp i32 %v168 to float
  %.i0219 = fadd fast float %.i0217, 5.000000e-01
  %.i1220 = fadd fast float %.i1218, 5.000000e-01
  %.i0221 = fdiv fast float %.i0219, %v132
  %.i1222 = fdiv fast float %.i1220, %v133
  %.i0223 = fmul fast float %.i0221, 2.000000e+00
  %.i1224 = fmul fast float %.i1222, 2.000000e+00
  %.i0225 = fadd fast float %.i0223, -1.000000e+00
  %.i1226 = fadd fast float %.i1224, -1.000000e+00
  %v169 = fsub fast float -0.000000e+00, %.i1226
  %v170 = fmul fast float %v136, %.i0225
  %FMad65 = call float @dx.op.tertiary.f32(i32 46, float %v169, float %v137, float %v170)
  %FMad64 = call float @dx.op.tertiary.f32(i32 46, float 0.000000e+00, float %v138, float %FMad65)
  %FMad63 = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %v139, float %FMad64)
  %v171 = fmul fast float %v141, %.i0225
  %FMad62 = call float @dx.op.tertiary.f32(i32 46, float %v169, float %v142, float %v171)
  %FMad61 = call float @dx.op.tertiary.f32(i32 46, float 0.000000e+00, float %v143, float %FMad62)
  %FMad60 = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %v144, float %FMad61)
  %v172 = fmul fast float %v146, %.i0225
  %FMad59 = call float @dx.op.tertiary.f32(i32 46, float %v169, float %v147, float %v172)
  %FMad58 = call float @dx.op.tertiary.f32(i32 46, float 0.000000e+00, float %v148, float %FMad59)
  %FMad57 = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %v149, float %FMad58)
  %v173 = fmul fast float %v151, %.i0225
  %FMad56 = call float @dx.op.tertiary.f32(i32 46, float %v169, float %v152, float %v173)
  %FMad55 = call float @dx.op.tertiary.f32(i32 46, float 0.000000e+00, float %v153, float %FMad56)
  %FMad54 = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %v154, float %FMad55)
  %.i0227 = fdiv fast float %FMad63, %FMad54
  %.i1228 = fdiv fast float %FMad60, %FMad54
  %.i2229 = fdiv fast float %FMad57, %FMad54
  %.i0230 = fsub fast float %.i0227, %v160
  %.i1231 = fsub fast float %.i1228, %v161
  %.i2232 = fsub fast float %.i2229, %v162
  %v174 = fmul fast float %.i0230, %.i0230
  %v175 = fmul fast float %.i1231, %.i1231
  %v176 = fadd fast float %v174, %v175
  %v177 = fmul fast float %.i2232, %.i2232
  %v178 = fadd fast float %v176, %v177
  %Sqrt70 = call float @dx.op.unary.f32(i32 24, float %v178)
  %.i0233 = fdiv fast float %.i0230, %Sqrt70
  %.i1234 = fdiv fast float %.i1231, %Sqrt70
  %.i2235 = fdiv fast float %.i2232, %Sqrt70
  %.i0236 = fsub fast float %.i0186, %.i0183
  %.i1237 = fsub fast float %.i1187, %.i1184
  %.i2238 = fsub fast float %.i2188, %.i2185
  %.i0239 = fsub fast float %.i0189, %.i0183
  %.i1240 = fsub fast float %.i1190, %.i1184
  %.i2241 = fsub fast float %.i2191, %.i2185
  %v179 = fmul fast float %.i1240, %.i2238
  %v180 = fmul fast float %.i2241, %.i1237
  %v181 = fsub fast float %v179, %v180
  %v182 = fmul fast float %.i2241, %.i0236
  %v183 = fmul fast float %.i0239, %.i2238
  %v184 = fsub fast float %v182, %v183
  %v185 = fmul fast float %.i0239, %.i1237
  %v186 = fmul fast float %.i1240, %.i0236
  %v187 = fsub fast float %v185, %v186
  %v188 = fmul fast float %v181, %v181
  %v189 = fmul fast float %v184, %v184
  %v190 = fadd fast float %v188, %v189
  %v191 = fmul fast float %v187, %v187
  %v192 = fadd fast float %v190, %v191
  %Sqrt74 = call float @dx.op.unary.f32(i32 24, float %v192)
  %.i0242 = fdiv fast float %v181, %Sqrt74
  %.i1243 = fdiv fast float %v184, %Sqrt74
  %.i2244 = fdiv fast float %v187, %Sqrt74
  %.i0245 = fsub fast float %v160, %.i0195
  %.i1246 = fsub fast float %v161, %.i1196
  %.i2247 = fsub fast float %v162, %.i2197
  %.i0248 = fsub fast float -0.000000e+00, %.i0242
  %.i1249 = fsub fast float -0.000000e+00, %.i1243
  %.i2250 = fsub fast float -0.000000e+00, %.i2244
  %v193 = call float @dx.op.dot3.f32(i32 55, float %.i0248, float %.i1249, float %.i2250, float %.i0245, float %.i1246, float %.i2247)
  %v194 = call float @dx.op.dot3.f32(i32 55, float %.i0242, float %.i1243, float %.i2244, float %.i0214, float %.i1215, float %.i2216)
  %v195 = fdiv fast float %v193, %v194
  %.i0251 = fmul fast float %v195, %.i0214
  %.i1252 = fmul fast float %v195, %.i1215
  %.i2253 = fmul fast float %v195, %.i2216
  %.i0254 = fadd fast float %.i0251, %v160
  %.i1255 = fadd fast float %.i1252, %v161
  %.i2256 = fadd fast float %.i2253, %v162
  %v196 = call float @dx.op.dot3.f32(i32 55, float %.i0242, float %.i1243, float %.i2244, float %.i0233, float %.i1234, float %.i2235)
  %v197 = fdiv fast float %v193, %v196
  %.i0263 = fmul fast float %v197, %.i0233
  %.i1264 = fmul fast float %v197, %.i1234
  %.i2265 = fmul fast float %v197, %.i2235
  %.i0266 = fadd fast float %.i0263, %v160
  %.i1267 = fadd fast float %.i1264, %v161
  %.i2268 = fadd fast float %.i2265, %v162
  %.i0269 = fsub fast float %.i0, %.i0118
  %.i1270 = fsub fast float %.i1, %.i1119
  %.i0271 = fsub fast float %.i0116, %.i0118
  %.i1272 = fsub fast float %.i1117, %.i1119
  %.i0273 = fsub fast float %.i0183, %.i0189
  %.i1274 = fsub fast float %.i1184, %.i1190
  %.i2275 = fsub fast float %.i2185, %.i2191
  %.i0276 = fsub fast float %.i0186, %.i0189
  %.i1277 = fsub fast float %.i1187, %.i1190
  %.i2278 = fsub fast float %.i2188, %.i2191
  %v198 = fmul fast float %.i0269, %.i1272
  %v199 = fmul fast float %.i0271, %.i1270
  %v200 = fsub fast float %v198, %v199
  %v201 = fdiv fast float 1.000000e+00, %v200
  %.i0279 = fmul fast float %v201, %.i1272
  %v202 = fmul fast float %.i0271, %v201
  %.i1280 = fsub fast float -0.000000e+00, %v202
  %v203 = fmul fast float %.i1270, %v201
  %.i2281 = fsub fast float -0.000000e+00, %v203
  %.i3 = fmul fast float %v201, %.i0269
  %.i0282 = fmul fast float %.i0279, %.i0273
  %.i1283 = fmul fast float %.i0279, %.i1274
  %.i2284 = fmul fast float %.i0279, %.i2275
  %.i0285 = fmul fast float %.i0276, %.i1280
  %.i1286 = fmul fast float %.i1277, %.i1280
  %.i2287 = fmul fast float %.i2278, %.i1280
  %.i0288 = fadd fast float %.i0282, %.i0285
  %.i1289 = fadd fast float %.i1283, %.i1286
  %.i2290 = fadd fast float %.i2284, %.i2287
  %.i0291 = fmul fast float %.i0273, %.i2281
  %.i1292 = fmul fast float %.i1274, %.i2281
  %.i2293 = fmul fast float %.i2275, %.i2281
  %.i0294 = fmul fast float %.i3, %.i0276
  %.i1295 = fmul fast float %.i3, %.i1277
  %.i2296 = fmul fast float %.i3, %.i2278
  %.i0297 = fadd fast float %.i0291, %.i0294
  %.i1298 = fadd fast float %.i1292, %.i1295
  %.i2299 = fadd fast float %.i2293, %.i2296
  %v204 = getelementptr inbounds [3 x float], [3 x float]* %v11, i32 0, i32 0
  store float %.i0266, float* %v204, align 4
  %v205 = getelementptr inbounds [3 x float], [3 x float]* %v11, i32 0, i32 1
  store float %.i1267, float* %v205, align 4
  %v206 = getelementptr inbounds [3 x float], [3 x float]* %v11, i32 0, i32 2
  store float %.i2268, float* %v206, align 4
  %v207 = getelementptr inbounds [3 x float], [3 x float]* %v12, i32 0, i32 0
  store float %.i0254, float* %v207, align 4
  %v208 = getelementptr inbounds [3 x float], [3 x float]* %v12, i32 0, i32 1
  store float %.i1255, float* %v208, align 4
  %v209 = getelementptr inbounds [3 x float], [3 x float]* %v12, i32 0, i32 2
  store float %.i2256, float* %v209, align 4
  %v210 = getelementptr inbounds [3 x float], [3 x float]* %v13, i32 0, i32 0
  store float %.i0195, float* %v210, align 4
  %v211 = getelementptr inbounds [3 x float], [3 x float]* %v13, i32 0, i32 1
  store float %.i1196, float* %v211, align 4
  %v212 = getelementptr inbounds [3 x float], [3 x float]* %v13, i32 0, i32 2
  store float %.i2197, float* %v212, align 4
  %v213 = getelementptr inbounds [3 x float], [3 x float]* %v14, i32 0, i32 0
  store float %.i0297, float* %v213, align 4
  %v214 = getelementptr inbounds [3 x float], [3 x float]* %v14, i32 0, i32 1
  store float %.i1298, float* %v214, align 4
  %v215 = getelementptr inbounds [3 x float], [3 x float]* %v14, i32 0, i32 2
  store float %.i2299, float* %v215, align 4
  %v216 = getelementptr inbounds [3 x float], [3 x float]* %v15, i32 0, i32 0
  store float %.i0288, float* %v216, align 4
  %v217 = getelementptr inbounds [3 x float], [3 x float]* %v15, i32 0, i32 1
  store float %.i1289, float* %v217, align 4
  %v218 = getelementptr inbounds [3 x float], [3 x float]* %v15, i32 0, i32 2
  store float %.i2290, float* %v218, align 4
  %FAbs = call float @dx.op.unary.f32(i32 6, float %.i0242)
  %FAbs68 = call float @dx.op.unary.f32(i32 6, float %.i1243)
  %FAbs69 = call float @dx.op.unary.f32(i32 6, float %.i2244)
  %v219 = fcmp fast ogt float %FAbs, %FAbs68
  %v220 = fcmp fast ogt float %FAbs, %FAbs69
  %v221 = and i1 %v219, %v220
  br i1 %v221, label %"\01?Load3x16BitIndices@@YA?AV?$vector@I$02@@I@Z.exit.\01?CalculateUVDerivatives@@YAXV?$vector@M$02@@00000AIAV?$vector@M$01@@1@Z.exit_crit_edge", label %1

"\01?Load3x16BitIndices@@YA?AV?$vector@I$02@@I@Z.exit.\01?CalculateUVDerivatives@@YAXV?$vector@M$02@@00000AIAV?$vector@M$01@@1@Z.exit_crit_edge": ; preds = %"\01?Load3x16BitIndices@@YA?AV?$vector@I$02@@I@Z.exit"
  br label %"\01?CalculateUVDerivatives@@YAXV?$vector@M$02@@00000AIAV?$vector@M$01@@1@Z.exit"

; <label>:1                                       ; preds = %"\01?Load3x16BitIndices@@YA?AV?$vector@I$02@@I@Z.exit"
  %v222 = fcmp fast ogt float %FAbs68, %FAbs69
  br i1 %v222, label %".\01?CalculateUVDerivatives@@YAXV?$vector@M$02@@00000AIAV?$vector@M$01@@1@Z.exit_crit_edge", label %2

".\01?CalculateUVDerivatives@@YAXV?$vector@M$02@@00000AIAV?$vector@M$01@@1@Z.exit_crit_edge": ; preds = %1
  br label %"\01?CalculateUVDerivatives@@YAXV?$vector@M$02@@00000AIAV?$vector@M$01@@1@Z.exit"

; <label>:2                                       ; preds = %1
  br label %"\01?CalculateUVDerivatives@@YAXV?$vector@M$02@@00000AIAV?$vector@M$01@@1@Z.exit"

"\01?CalculateUVDerivatives@@YAXV?$vector@M$02@@00000AIAV?$vector@M$01@@1@Z.exit": ; preds = %".\01?CalculateUVDerivatives@@YAXV?$vector@M$02@@00000AIAV?$vector@M$01@@1@Z.exit_crit_edge", %"\01?Load3x16BitIndices@@YA?AV?$vector@I$02@@I@Z.exit.\01?CalculateUVDerivatives@@YAXV?$vector@M$02@@00000AIAV?$vector@M$01@@1@Z.exit_crit_edge", %2
  %v223 = phi float [ %.i1267, %2 ], [ %.i2268, %"\01?Load3x16BitIndices@@YA?AV?$vector@I$02@@I@Z.exit.\01?CalculateUVDerivatives@@YAXV?$vector@M$02@@00000AIAV?$vector@M$01@@1@Z.exit_crit_edge" ], [ %.i2268, %".\01?CalculateUVDerivatives@@YAXV?$vector@M$02@@00000AIAV?$vector@M$01@@1@Z.exit_crit_edge" ]
  %v224 = phi float [ %.i1196, %2 ], [ %.i2197, %"\01?Load3x16BitIndices@@YA?AV?$vector@I$02@@I@Z.exit.\01?CalculateUVDerivatives@@YAXV?$vector@M$02@@00000AIAV?$vector@M$01@@1@Z.exit_crit_edge" ], [ %.i2197, %".\01?CalculateUVDerivatives@@YAXV?$vector@M$02@@00000AIAV?$vector@M$01@@1@Z.exit_crit_edge" ]
  %v225 = phi float [ %.i1255, %2 ], [ %.i2256, %"\01?Load3x16BitIndices@@YA?AV?$vector@I$02@@I@Z.exit.\01?CalculateUVDerivatives@@YAXV?$vector@M$02@@00000AIAV?$vector@M$01@@1@Z.exit_crit_edge" ], [ %.i2256, %".\01?CalculateUVDerivatives@@YAXV?$vector@M$02@@00000AIAV?$vector@M$01@@1@Z.exit_crit_edge" ]
  %v226 = phi float [ %.i1298, %2 ], [ %.i2299, %"\01?Load3x16BitIndices@@YA?AV?$vector@I$02@@I@Z.exit.\01?CalculateUVDerivatives@@YAXV?$vector@M$02@@00000AIAV?$vector@M$01@@1@Z.exit_crit_edge" ], [ %.i2299, %".\01?CalculateUVDerivatives@@YAXV?$vector@M$02@@00000AIAV?$vector@M$01@@1@Z.exit_crit_edge" ]
  %v227 = phi float [ %.i1289, %2 ], [ %.i2290, %"\01?Load3x16BitIndices@@YA?AV?$vector@I$02@@I@Z.exit.\01?CalculateUVDerivatives@@YAXV?$vector@M$02@@00000AIAV?$vector@M$01@@1@Z.exit_crit_edge" ], [ %.i2290, %".\01?CalculateUVDerivatives@@YAXV?$vector@M$02@@00000AIAV?$vector@M$01@@1@Z.exit_crit_edge" ]
  %indices.i.0.i0 = phi i32 [ 0, %2 ], [ 1, %"\01?Load3x16BitIndices@@YA?AV?$vector@I$02@@I@Z.exit.\01?CalculateUVDerivatives@@YAXV?$vector@M$02@@00000AIAV?$vector@M$01@@1@Z.exit_crit_edge" ], [ 0, %".\01?CalculateUVDerivatives@@YAXV?$vector@M$02@@00000AIAV?$vector@M$01@@1@Z.exit_crit_edge" ]
  %v228 = getelementptr [3 x float], [3 x float]* %v15, i32 0, i32 %indices.i.0.i0
  %v229 = load float, float* %v228, align 4, !tbaa !231, !noalias !335
  %v230 = getelementptr [3 x float], [3 x float]* %v14, i32 0, i32 %indices.i.0.i0
  %v231 = load float, float* %v230, align 4, !tbaa !231, !noalias !335
  %v232 = fmul fast float %v226, %v229
  %v233 = fmul fast float %v227, %v231
  %v234 = fsub fast float %v232, %v233
  %v235 = fdiv fast float 1.000000e+00, %v234
  %.i0300 = fmul fast float %v235, %v226
  %v236 = fmul fast float %v227, %v235
  %.i1301 = fsub fast float -0.000000e+00, %v236
  %v237 = fmul fast float %v231, %v235
  %.i2302 = fsub fast float -0.000000e+00, %v237
  %.i3303 = fmul fast float %v235, %v229
  %v238 = getelementptr [3 x float], [3 x float]* %v12, i32 0, i32 %indices.i.0.i0
  %v239 = load float, float* %v238, align 4, !tbaa !231, !noalias !335
  %v240 = getelementptr [3 x float], [3 x float]* %v13, i32 0, i32 %indices.i.0.i0
  %v241 = load float, float* %v240, align 4, !tbaa !231, !noalias !335
  %v242 = fsub fast float %v239, %v241
  %v243 = fsub fast float %v225, %v224
  %v244 = fmul fast float %v242, %.i0300
  %FMad21 = call float @dx.op.tertiary.f32(i32 46, float %v243, float %.i1301, float %v244)
  %v245 = fmul fast float %v242, %.i2302
  %FMad = call float @dx.op.tertiary.f32(i32 46, float %v243, float %.i3303, float %v245)
  %FAbs79 = call float @dx.op.unary.f32(i32 6, float %FMad21)
  %FAbs80 = call float @dx.op.unary.f32(i32 6, float %FMad)
  %v246 = getelementptr [3 x float], [3 x float]* %v11, i32 0, i32 %indices.i.0.i0
  %v247 = load float, float* %v246, align 4, !tbaa !231, !noalias !335
  %v248 = fsub fast float %v247, %v241
  %v249 = fsub fast float %v223, %v224
  %v250 = fmul fast float %v248, %.i0300
  %FMad53 = call float @dx.op.tertiary.f32(i32 46, float %v249, float %.i1301, float %v250)
  %v251 = fmul fast float %v248, %.i2302
  %FMad52 = call float @dx.op.tertiary.f32(i32 46, float %v249, float %.i3303, float %v251)
  %FAbs77 = call float @dx.op.unary.f32(i32 6, float %FMad53)
  %FAbs78 = call float @dx.op.unary.f32(i32 6, float %FMad52)
  %.i0304 = fsub fast float -0.000000e+00, %WorldRayDirection
  %.i1305 = fsub fast float -0.000000e+00, %WorldRayDirection99
  %.i2306 = fsub fast float -0.000000e+00, %WorldRayDirection100
  %v252 = fmul fast float %WorldRayDirection, %WorldRayDirection
  %v253 = fmul fast float %WorldRayDirection99, %WorldRayDirection99
  %v254 = fadd fast float %v253, %v252
  %v255 = fmul fast float %WorldRayDirection100, %WorldRayDirection100
  %v256 = fadd fast float %v254, %v255
  %Sqrt75 = call float @dx.op.unary.f32(i32 24, float %v256)
  %.i0307 = fdiv fast float %.i0304, %Sqrt75
  %.i1308 = fdiv fast float %.i1305, %Sqrt75
  %.i2309 = fdiv fast float %.i2306, %Sqrt75
  %v257 = getelementptr inbounds [27 x %"class.Texture2D<vector<float, 4> >"], [27 x %"class.Texture2D<vector<float, 4> >"]* @"\01?g_texDiffuse@@3PAV?$Texture2D@V?$vector@M$03@@@@A", i32 0, i32 %v37, !dx.nonuniform !240
  %v258 = load %"class.Texture2D<vector<float, 4> >", %"class.Texture2D<vector<float, 4> >"* %v257, align 4
  %v259 = call %dx.types.Handle @"dx.op.createHandleFromResourceStructForLib.class.Texture2D<vector<float, 4> >"(i32 160, %"class.Texture2D<vector<float, 4> >" %v258)
  %v260 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.SamplerState(i32 160, %struct.SamplerState %v1)
  %v261 = call %dx.types.ResRet.f32 @dx.op.sampleGrad.f32(i32 63, %dx.types.Handle %v259, %dx.types.Handle %v260, float %.i0128, float %.i1129, float undef, float undef, i32 undef, i32 undef, i32 undef, float %FAbs79, float %FAbs80, float undef, float %FAbs77, float %FAbs78, float undef, float undef)
  %v262 = extractvalue %dx.types.ResRet.f32 %v261, 0
  %v263 = extractvalue %dx.types.ResRet.f32 %v261, 1
  %v264 = extractvalue %dx.types.ResRet.f32 %v261, 2
  %v265 = getelementptr inbounds [27 x %"class.Texture2D<vector<float, 4> >"], [27 x %"class.Texture2D<vector<float, 4> >"]* @"\01?g_texNormal@@3PAV?$Texture2D@V?$vector@M$03@@@@A", i32 0, i32 %v37, !dx.nonuniform !240
  %v266 = load %"class.Texture2D<vector<float, 4> >", %"class.Texture2D<vector<float, 4> >"* %v265, align 4
  %v267 = call %dx.types.Handle @"dx.op.createHandleFromResourceStructForLib.class.Texture2D<vector<float, 4> >"(i32 160, %"class.Texture2D<vector<float, 4> >" %v266)
  %v268 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.SamplerState(i32 160, %struct.SamplerState %v1)
  %v269 = call %dx.types.ResRet.f32 @dx.op.sampleGrad.f32(i32 63, %dx.types.Handle %v267, %dx.types.Handle %v268, float %.i0128, float %.i1129, float undef, float undef, i32 undef, i32 undef, i32 undef, float %FAbs79, float %FAbs80, float undef, float %FAbs77, float %FAbs78, float undef, float undef)
  %v270 = extractvalue %dx.types.ResRet.f32 %v269, 0
  %v271 = extractvalue %dx.types.ResRet.f32 %v269, 1
  %v272 = extractvalue %dx.types.ResRet.f32 %v269, 2
  %.i0310 = fmul fast float %v270, 2.000000e+00
  %.i1311 = fmul fast float %v271, 2.000000e+00
  %.i2312 = fmul fast float %v272, 2.000000e+00
  %.i0313 = fadd fast float %.i0310, -1.000000e+00
  %.i1314 = fadd fast float %.i1311, -1.000000e+00
  %.i2315 = fadd fast float %.i2312, -1.000000e+00
  %v273 = call float @dx.op.dot3.f32(i32 55, float %.i0313, float %.i1314, float %.i2315, float %.i0313, float %.i1314, float %.i2315)
  %Rsqrt = call float @dx.op.unary.f32(i32 25, float %v273)
  %.i0316 = fmul fast float %.i0313, %Rsqrt
  %.i1317 = fmul fast float %.i1314, %Rsqrt
  %.i2318 = fmul fast float %.i2315, %Rsqrt
  %v274 = fmul fast float %.i2182, %.i1154
  %v275 = fmul fast float %.i1181, %.i2155
  %v276 = fsub fast float %v274, %v275
  %v277 = fmul fast float %.i0180, %.i2155
  %v278 = fmul fast float %.i2182, %.i0153
  %v279 = fsub fast float %v277, %v278
  %v280 = fmul fast float %.i1181, %.i0153
  %v281 = fmul fast float %.i0180, %.i1154
  %v282 = fsub fast float %v280, %v281
  %v283 = fmul fast float %.i0316, %.i0180
  %FMad51 = call float @dx.op.tertiary.f32(i32 46, float %.i1317, float %v276, float %v283)
  %FMad50 = call float @dx.op.tertiary.f32(i32 46, float %.i2318, float %.i0153, float %FMad51)
  %v284 = fmul fast float %.i0316, %.i1181
  %FMad49 = call float @dx.op.tertiary.f32(i32 46, float %.i1317, float %v279, float %v284)
  %FMad48 = call float @dx.op.tertiary.f32(i32 46, float %.i2318, float %.i1154, float %FMad49)
  %v285 = fmul fast float %.i0316, %.i2182
  %FMad47 = call float @dx.op.tertiary.f32(i32 46, float %.i1317, float %v282, float %v285)
  %FMad46 = call float @dx.op.tertiary.f32(i32 46, float %.i2318, float %.i2155, float %FMad47)
  %v286 = fmul fast float %FMad50, %FMad50
  %v287 = fmul fast float %FMad48, %FMad48
  %v288 = fadd fast float %v287, %v286
  %v289 = fmul fast float %FMad46, %FMad46
  %v290 = fadd fast float %v288, %v289
  %Sqrt76 = call float @dx.op.unary.f32(i32 24, float %v290)
  %.i0319 = fdiv fast float %FMad50, %Sqrt76
  %.i1320 = fdiv fast float %FMad48, %Sqrt76
  %.i2321 = fdiv fast float %FMad46, %Sqrt76
  %v291 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %HitShaderConstants114, i32 2)
  %v292 = extractvalue %dx.types.CBufRet.f32 %v291, 0
  %v293 = extractvalue %dx.types.CBufRet.f32 %v291, 1
  %v294 = extractvalue %dx.types.CBufRet.f32 %v291, 2
  %.i0322 = fmul fast float %v292, %v262
  %.i1323 = fmul fast float %v293, %v263
  %.i2324 = fmul fast float %v294, %v264
  %v295 = call %dx.types.Handle @"dx.op.createHandleFromResourceStructForLib.class.Texture2D<float>"(i32 160, %"class.Texture2D<float>" %v2)
  %TextureLoad = call %dx.types.ResRet.f32 @dx.op.textureLoad.f32(i32 66, %dx.types.Handle %v295, i32 0, i32 %DispatchRaysIndex, i32 %DispatchRaysIndex106, i32 undef, i32 undef, i32 undef, i32 undef)
  %v296 = extractvalue %dx.types.ResRet.f32 %TextureLoad, 0
  %.i0325 = fmul fast float %.i0322, %v296
  %.i1326 = fmul fast float %.i1323, %v296
  %.i2327 = fmul fast float %.i2324, %v296
  %v297 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %HitShaderConstants114, i32 8)
  %v298 = extractvalue %dx.types.CBufRet.i32 %v297, 1
  %v299 = icmp eq i32 %v298, 0
  br i1 %v299, label %23, label %remat_begin

remat_begin:                                      ; preds = %"\01?CalculateUVDerivatives@@YAXV?$vector@M$02@@00000AIAV?$vector@M$01@@1@Z.exit"
  %v300 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %HitShaderConstants114, i32 0)
  %v301 = extractvalue %dx.types.CBufRet.f32 %v300, 0
  %v302 = extractvalue %dx.types.CBufRet.f32 %v300, 1
  %v303 = extractvalue %dx.types.CBufRet.f32 %v300, 2
  %offs30 = add i32 16, 0
  %v304 = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %stackFrame.offset, i32 %offs30)
  store i32 1, i32* %v304, align 8
  %offs31 = add i32 16, 1
  %v305.ptr = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %stackFrame.offset, i32 %offs31)
  %v305 = bitcast i32* %v305.ptr to float*
  store float 0x47EFFFFFE0000000, float* %v305, align 4
  %v306 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RaytracingAccelerationStructure(i32 160, %struct.RaytracingAccelerationStructure %v7)
  %new.payload.offset = add i32 %stackFrame.offset, 16
  %intPtr8 = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %stackFrame.offset, i32 0)
  store i32 1003, i32* %intPtr8
  %intPtr7 = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %stackFrame.offset, i32 1)
  store i32 4, i32* %intPtr7
  %intPtr6 = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %stackFrame.offset, i32 2)
  store i32 -1, i32* %intPtr6
  %intPtr5 = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %stackFrame.offset, i32 3)
  store i32 0, i32* %intPtr5
  %intPtr4 = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %stackFrame.offset, i32 4)
  store i32 1, i32* %intPtr4
  %intPtr3 = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %stackFrame.offset, i32 5)
  store i32 0, i32* %intPtr3
  %3 = bitcast float %.i0195 to i32
  %intPtr28 = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %stackFrame.offset, i32 6)
  store i32 %3, i32* %intPtr28
  %4 = bitcast float %.i1196 to i32
  %intPtr27 = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %stackFrame.offset, i32 7)
  store i32 %4, i32* %intPtr27
  %5 = bitcast float %.i2197 to i32
  %intPtr26 = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %stackFrame.offset, i32 8)
  store i32 %5, i32* %intPtr26
  %6 = bitcast float 0x3FB99999A0000000 to i32
  %intPtr25 = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %stackFrame.offset, i32 9)
  store i32 %6, i32* %intPtr25
  %7 = bitcast float %v301 to i32
  %intPtr24 = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %stackFrame.offset, i32 10)
  store i32 %7, i32* %intPtr24
  %8 = bitcast float %v302 to i32
  %intPtr23 = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %stackFrame.offset, i32 11)
  store i32 %8, i32* %intPtr23
  %9 = bitcast float %v303 to i32
  %intPtr22 = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %stackFrame.offset, i32 12)
  store i32 %9, i32* %intPtr22
  %10 = bitcast float 0x47EFFFFFE0000000 to i32
  %intPtr21 = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %stackFrame.offset, i32 13)
  store i32 %10, i32* %intPtr21
  %intPtr2 = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %stackFrame.offset, i32 14)
  store i32 %new.payload.offset, i32* %intPtr2
  %intPtr1 = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %stackFrame.offset, i32 18)
  store i32 %DispatchRaysIndex, i32* %intPtr1
  %intPtr = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %stackFrame.offset, i32 19)
  store i32 %DispatchRaysIndex106, i32* %intPtr
  %11 = bitcast float %.i0307 to i32
  %intPtr20 = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %stackFrame.offset, i32 20)
  store i32 %11, i32* %intPtr20
  %12 = bitcast float %.i1308 to i32
  %intPtr19 = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %stackFrame.offset, i32 21)
  store i32 %12, i32* %intPtr19
  %13 = bitcast float %.i2309 to i32
  %intPtr18 = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %stackFrame.offset, i32 22)
  store i32 %13, i32* %intPtr18
  %14 = bitcast float %v262 to i32
  %intPtr17 = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %stackFrame.offset, i32 23)
  store i32 %14, i32* %intPtr17
  %15 = bitcast float %v263 to i32
  %intPtr16 = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %stackFrame.offset, i32 24)
  store i32 %15, i32* %intPtr16
  %16 = bitcast float %v264 to i32
  %intPtr15 = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %stackFrame.offset, i32 25)
  store i32 %16, i32* %intPtr15
  %17 = bitcast float %.i0319 to i32
  %intPtr14 = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %stackFrame.offset, i32 26)
  store i32 %17, i32* %intPtr14
  %18 = bitcast float %.i1320 to i32
  %intPtr13 = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %stackFrame.offset, i32 27)
  store i32 %18, i32* %intPtr13
  %19 = bitcast float %.i2321 to i32
  %intPtr12 = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %stackFrame.offset, i32 28)
  store i32 %19, i32* %intPtr12
  %20 = bitcast float %.i0325 to i32
  %intPtr11 = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %stackFrame.offset, i32 29)
  store i32 %20, i32* %intPtr11
  %21 = bitcast float %.i1326 to i32
  %intPtr10 = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %stackFrame.offset, i32 30)
  store i32 %21, i32* %intPtr10
  %22 = bitcast float %.i2327 to i32
  %intPtr9 = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %stackFrame.offset, i32 31)
  store i32 %22, i32* %intPtr9
  ret i32 1005

; <label>:23                                      ; preds = %"\01?CalculateUVDerivatives@@YAXV?$vector@M$02@@00000AIAV?$vector@M$01@@1@Z.exit"
  %v309 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %HitShaderConstants114, i32 4)
  %v310 = extractvalue %dx.types.CBufRet.f32 %v309, 0
  %v311 = extractvalue %dx.types.CBufRet.f32 %v309, 1
  %v312 = extractvalue %dx.types.CBufRet.f32 %v309, 2
  %v313 = extractvalue %dx.types.CBufRet.f32 %v309, 3
  %v314 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %HitShaderConstants114, i32 5)
  %v315 = extractvalue %dx.types.CBufRet.f32 %v314, 0
  %v316 = extractvalue %dx.types.CBufRet.f32 %v314, 1
  %v317 = extractvalue %dx.types.CBufRet.f32 %v314, 2
  %v318 = extractvalue %dx.types.CBufRet.f32 %v314, 3
  %v319 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %HitShaderConstants114, i32 6)
  %v320 = extractvalue %dx.types.CBufRet.f32 %v319, 0
  %v321 = extractvalue %dx.types.CBufRet.f32 %v319, 1
  %v322 = extractvalue %dx.types.CBufRet.f32 %v319, 2
  %v323 = extractvalue %dx.types.CBufRet.f32 %v319, 3
  %v324 = fmul fast float %v310, %.i0195
  %FMad45 = call float @dx.op.tertiary.f32(i32 46, float %.i1196, float %v311, float %v324)
  %FMad44 = call float @dx.op.tertiary.f32(i32 46, float %.i2197, float %v312, float %FMad45)
  %FMad43 = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %v313, float %FMad44)
  %v325 = fmul fast float %v315, %.i0195
  %FMad42 = call float @dx.op.tertiary.f32(i32 46, float %.i1196, float %v316, float %v325)
  %FMad41 = call float @dx.op.tertiary.f32(i32 46, float %.i2197, float %v317, float %FMad42)
  %FMad40 = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %v318, float %FMad41)
  %v326 = fmul fast float %v320, %.i0195
  %FMad39 = call float @dx.op.tertiary.f32(i32 46, float %.i1196, float %v321, float %v326)
  %FMad38 = call float @dx.op.tertiary.f32(i32 46, float %.i2197, float %v322, float %FMad39)
  %FMad37 = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %v323, float %FMad38)
  %v327 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %HitShaderConstants114, i32 3)
  %v328 = extractvalue %dx.types.CBufRet.f32 %v327, 0
  %v329 = fmul fast float %v328, 2.500000e-01
  %v330 = fmul fast float %v328, 1.750000e+00
  %v331 = fmul fast float %v328, 1.250000e+00
  %v332 = fmul fast float %v328, 7.500000e-01
  %v333 = call %dx.types.Handle @"dx.op.createHandleFromResourceStructForLib.class.Texture2D<float>"(i32 160, %"class.Texture2D<float>" %v3)
  %v334 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.SamplerComparisonState(i32 160, %struct.SamplerComparisonState %v)
  %v335 = call %dx.types.ResRet.f32 @dx.op.sampleCmpLevelZero.f32(i32 65, %dx.types.Handle %v333, %dx.types.Handle %v334, float %FMad43, float %FMad40, float undef, float undef, i32 undef, i32 undef, i32 undef, float %FMad37)
  %v336 = extractvalue %dx.types.ResRet.f32 %v335, 0
  %v337 = fmul fast float %v336, 2.000000e+00
  %.i0328 = fsub fast float %FMad43, %v330
  %.i1329 = fadd fast float %v329, %FMad40
  %v338 = call %dx.types.ResRet.f32 @dx.op.sampleCmpLevelZero.f32(i32 65, %dx.types.Handle %v333, %dx.types.Handle %v334, float %.i0328, float %.i1329, float undef, float undef, i32 undef, i32 undef, i32 undef, float %FMad37)
  %v339 = extractvalue %dx.types.ResRet.f32 %v338, 0
  %v340 = fadd fast float %v339, %v337
  %.i0330 = fsub fast float %FMad43, %v329
  %.i1331 = fsub fast float %FMad40, %v330
  %v341 = call %dx.types.ResRet.f32 @dx.op.sampleCmpLevelZero.f32(i32 65, %dx.types.Handle %v333, %dx.types.Handle %v334, float %.i0330, float %.i1331, float undef, float undef, i32 undef, i32 undef, i32 undef, float %FMad37)
  %v342 = extractvalue %dx.types.ResRet.f32 %v341, 0
  %v343 = fadd fast float %v340, %v342
  %.i0332 = fadd fast float %v330, %FMad43
  %.i1333 = fsub fast float %FMad40, %v329
  %v344 = call %dx.types.ResRet.f32 @dx.op.sampleCmpLevelZero.f32(i32 65, %dx.types.Handle %v333, %dx.types.Handle %v334, float %.i0332, float %.i1333, float undef, float undef, i32 undef, i32 undef, i32 undef, float %FMad37)
  %v345 = extractvalue %dx.types.ResRet.f32 %v344, 0
  %v346 = fadd fast float %v343, %v345
  %.i0334 = fadd fast float %v329, %FMad43
  %.i1335 = fadd fast float %v330, %FMad40
  %v347 = call %dx.types.ResRet.f32 @dx.op.sampleCmpLevelZero.f32(i32 65, %dx.types.Handle %v333, %dx.types.Handle %v334, float %.i0334, float %.i1335, float undef, float undef, i32 undef, i32 undef, i32 undef, float %FMad37)
  %v348 = extractvalue %dx.types.ResRet.f32 %v347, 0
  %v349 = fadd fast float %v346, %v348
  %.i0336 = fsub fast float %FMad43, %v332
  %.i1337 = fadd fast float %v331, %FMad40
  %v350 = call %dx.types.ResRet.f32 @dx.op.sampleCmpLevelZero.f32(i32 65, %dx.types.Handle %v333, %dx.types.Handle %v334, float %.i0336, float %.i1337, float undef, float undef, i32 undef, i32 undef, i32 undef, float %FMad37)
  %v351 = extractvalue %dx.types.ResRet.f32 %v350, 0
  %v352 = fadd fast float %v349, %v351
  %.i0338 = fsub fast float %FMad43, %v331
  %.i1339 = fsub fast float %FMad40, %v332
  %v353 = call %dx.types.ResRet.f32 @dx.op.sampleCmpLevelZero.f32(i32 65, %dx.types.Handle %v333, %dx.types.Handle %v334, float %.i0338, float %.i1339, float undef, float undef, i32 undef, i32 undef, i32 undef, float %FMad37)
  %v354 = extractvalue %dx.types.ResRet.f32 %v353, 0
  %v355 = fadd fast float %v352, %v354
  %.i0340 = fadd fast float %v332, %FMad43
  %.i1341 = fsub fast float %FMad40, %v331
  %v356 = call %dx.types.ResRet.f32 @dx.op.sampleCmpLevelZero.f32(i32 65, %dx.types.Handle %v333, %dx.types.Handle %v334, float %.i0340, float %.i1341, float undef, float undef, i32 undef, i32 undef, i32 undef, float %FMad37)
  %v357 = extractvalue %dx.types.ResRet.f32 %v356, 0
  %v358 = fadd fast float %v355, %v357
  %.i0342 = fadd fast float %v331, %FMad43
  %.i1343 = fadd fast float %v332, %FMad40
  %v359 = call %dx.types.ResRet.f32 @dx.op.sampleCmpLevelZero.f32(i32 65, %dx.types.Handle %v333, %dx.types.Handle %v334, float %.i0342, float %.i1343, float undef, float undef, i32 undef, i32 undef, i32 undef, float %FMad37)
  %v360 = extractvalue %dx.types.ResRet.f32 %v359, 0
  %v361 = fadd fast float %v358, %v360
  %v362 = fmul fast float %v361, 0x3FB99999A0000000
  %v363 = fmul fast float %v362, %v362
  br label %24

; <label>:24                                      ; preds = %23
  %shadow.0 = phi float [ %v363, %23 ]
  %v364 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %HitShaderConstants114, i32 1)
  %v365 = extractvalue %dx.types.CBufRet.f32 %v364, 0
  %v366 = extractvalue %dx.types.CBufRet.f32 %v364, 1
  %v367 = extractvalue %dx.types.CBufRet.f32 %v364, 2
  %v368 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %HitShaderConstants114, i32 0)
  %v369 = extractvalue %dx.types.CBufRet.f32 %v368, 0
  %v370 = extractvalue %dx.types.CBufRet.f32 %v368, 1
  %v371 = extractvalue %dx.types.CBufRet.f32 %v368, 2
  %.i0344 = fsub fast float %v369, %.i0307
  %.i1345 = fsub fast float %v370, %.i1308
  %.i2346 = fsub fast float %v371, %.i2309
  %v372 = fmul fast float %.i0344, %.i0344
  %v373 = fmul fast float %.i1345, %.i1345
  %v374 = fadd fast float %v372, %v373
  %v375 = fmul fast float %.i2346, %.i2346
  %v376 = fadd fast float %v374, %v375
  %Sqrt = call float @dx.op.unary.f32(i32 24, float %v376)
  %.i0347 = fdiv fast float %.i0344, %Sqrt
  %.i1348 = fdiv fast float %.i1345, %Sqrt
  %.i2349 = fdiv fast float %.i2346, %Sqrt
  %v377 = call float @dx.op.dot3.f32(i32 55, float %v369, float %v370, float %v371, float %.i0347, float %.i1348, float %.i2349)
  %Saturate19 = call float @dx.op.unary.f32(i32 7, float %v377)
  %v378 = fsub fast float 1.000000e+00, %Saturate19
  %Log66 = call float @dx.op.unary.f32(i32 23, float %v378)
  %v379 = fmul fast float %Log66, 5.000000e+00
  %Exp67 = call float @dx.op.unary.f32(i32 21, float %v379)
  %v380 = fmul fast float %Exp67, %v262
  %v381 = fmul fast float %Exp67, %v263
  %v382 = fmul fast float %Exp67, %v264
  %.i0362 = fsub fast float %v262, %v380
  %.i1363 = fsub fast float %v263, %v381
  %.i2364 = fsub fast float %v264, %v382
  %v383 = call float @dx.op.dot3.f32(i32 55, float %.i0319, float %.i1320, float %.i2321, float %v369, float %v370, float %v371)
  %Saturate = call float @dx.op.unary.f32(i32 7, float %v383)
  %.i0365 = fmul fast float %v365, %shadow.0
  %.i0368 = fmul fast float %.i0365, %Saturate
  %.i0371 = fmul fast float %.i0368, %.i0362
  %.i1366 = fmul fast float %v366, %shadow.0
  %.i1369 = fmul fast float %.i1366, %Saturate
  %.i1372 = fmul fast float %.i1369, %.i1363
  %.i2367 = fmul fast float %v367, %shadow.0
  %.i2370 = fmul fast float %.i2367, %Saturate
  %.i2373 = fmul fast float %.i2370, %.i2364
  %.i0374 = fadd fast float %.i0371, %.i0325
  %.i1375 = fadd fast float %.i1372, %.i1326
  %.i2376 = fadd fast float %.i2373, %.i2327
  %v384 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %HitShaderConstants114, i32 8)
  %v385 = extractvalue %dx.types.CBufRet.i32 %v384, 0
  %v386 = icmp eq i32 %v385, 0
  br i1 %v386, label %._crit_edge, label %25

._crit_edge:                                      ; preds = %24
  br label %26

; <label>:25                                      ; preds = %24
  %.i0377 = fmul fast float %.i0374, 0x3FD3333340000000
  %.i1378 = fmul fast float %.i1375, 0x3FD3333340000000
  %.i2379 = fmul fast float %.i2376, 0x3FD3333340000000
  %v387 = call %dx.types.Handle @"dx.op.createHandleFromResourceStructForLib.class.RWTexture2D<vector<float, 4> >"(i32 160, %"class.RWTexture2D<vector<float, 4> >" %v8)
  %TextureLoad113 = call %dx.types.ResRet.f32 @dx.op.textureLoad.f32(i32 66, %dx.types.Handle %v387, i32 undef, i32 %DispatchRaysIndex, i32 %DispatchRaysIndex106, i32 undef, i32 undef, i32 undef, i32 undef)
  %v388 = extractvalue %dx.types.ResRet.f32 %TextureLoad113, 0
  %v389 = extractvalue %dx.types.ResRet.f32 %TextureLoad113, 1
  %v390 = extractvalue %dx.types.ResRet.f32 %TextureLoad113, 2
  %.i0380 = fadd fast float %v388, %.i0377
  %.i1381 = fadd fast float %v389, %.i1378
  %.i2382 = fadd fast float %v390, %.i2379
  br label %26

; <label>:26                                      ; preds = %._crit_edge, %25
  %outputColor.0.i0 = phi float [ %.i0380, %25 ], [ %.i0374, %._crit_edge ]
  %outputColor.0.i1 = phi float [ %.i1381, %25 ], [ %.i1375, %._crit_edge ]
  %outputColor.0.i2 = phi float [ %.i2382, %25 ], [ %.i2376, %._crit_edge ]
  %v391 = call %dx.types.Handle @"dx.op.createHandleFromResourceStructForLib.class.RWTexture2D<vector<float, 4> >"(i32 160, %"class.RWTexture2D<vector<float, 4> >" %v8)
  call void @dx.op.textureStore.f32(i32 67, %dx.types.Handle %v391, i32 %DispatchRaysIndex, i32 %DispatchRaysIndex106, i32 undef, float %outputColor.0.i0, float %outputColor.0.i1, float %outputColor.0.i2, float 1.000000e+00, i8 15)
  br label %0
}

define i32 @Hit.ss_1(%struct.RuntimeDataStruct* %runtimeData) #9 {
Hit.BB1.from.Fallback_TraceRay:
  %stackFrame.offset.remat = call i32 @stackFrameOffset(%struct.RuntimeDataStruct* %runtimeData)
  %payload.offset.remat = call i32 @payloadOffset(%struct.RuntimeDataStruct* %runtimeData)
  %committedAttr.offset.remat = call i32 @committedAttrOffset(%struct.RuntimeDataStruct* %runtimeData)
  %DispatchRaysIndex.ptr = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %stackFrame.offset.remat, i32 18)
  %DispatchRaysIndex.int = load i32, i32* %DispatchRaysIndex.ptr
  %DispatchRaysIndex106.ptr = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %stackFrame.offset.remat, i32 19)
  %DispatchRaysIndex106.int = load i32, i32* %DispatchRaysIndex106.ptr
  %.i0307.ptr = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %stackFrame.offset.remat, i32 20)
  %.i0307.int = load i32, i32* %.i0307.ptr
  %0 = bitcast i32 %.i0307.int to float
  %.i1308.ptr = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %stackFrame.offset.remat, i32 21)
  %.i1308.int = load i32, i32* %.i1308.ptr
  %1 = bitcast i32 %.i1308.int to float
  %.i2309.ptr = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %stackFrame.offset.remat, i32 22)
  %.i2309.int = load i32, i32* %.i2309.ptr
  %2 = bitcast i32 %.i2309.int to float
  %v262.ptr = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %stackFrame.offset.remat, i32 23)
  %v262.int = load i32, i32* %v262.ptr
  %3 = bitcast i32 %v262.int to float
  %v263.ptr = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %stackFrame.offset.remat, i32 24)
  %v263.int = load i32, i32* %v263.ptr
  %4 = bitcast i32 %v263.int to float
  %v264.ptr = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %stackFrame.offset.remat, i32 25)
  %v264.int = load i32, i32* %v264.ptr
  %5 = bitcast i32 %v264.int to float
  %.i0319.ptr = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %stackFrame.offset.remat, i32 26)
  %.i0319.int = load i32, i32* %.i0319.ptr
  %6 = bitcast i32 %.i0319.int to float
  %.i1320.ptr = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %stackFrame.offset.remat, i32 27)
  %.i1320.int = load i32, i32* %.i1320.ptr
  %7 = bitcast i32 %.i1320.int to float
  %.i2321.ptr = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %stackFrame.offset.remat, i32 28)
  %.i2321.int = load i32, i32* %.i2321.ptr
  %8 = bitcast i32 %.i2321.int to float
  %.i0325.ptr = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %stackFrame.offset.remat, i32 29)
  %.i0325.int = load i32, i32* %.i0325.ptr
  %9 = bitcast i32 %.i0325.int to float
  %.i1326.ptr = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %stackFrame.offset.remat, i32 30)
  %.i1326.int = load i32, i32* %.i1326.ptr
  %10 = bitcast i32 %.i1326.int to float
  %.i2327.ptr = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %stackFrame.offset.remat, i32 31)
  %.i2327.int = load i32, i32* %.i2327.ptr
  %11 = bitcast i32 %.i2327.int to float
  %v8.remat = load %"class.RWTexture2D<vector<float, 4> >", %"class.RWTexture2D<vector<float, 4> >"* @"\01?g_screenOutput@@3V?$RWTexture2D@V?$vector@M$03@@@@A", align 4
  %v10.remat = load %HitShaderConstants, %HitShaderConstants* @HitShaderConstants, align 4
  %HitShaderConstants114.remat = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.HitShaderConstants(i32 160, %HitShaderConstants %v10.remat)
  %offs = add i32 16, 1
  %v305.remat.ptr = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %stackFrame.offset.remat, i32 %offs)
  %v305.remat = bitcast i32* %v305.remat.ptr to float*
  %v307 = load float, float* %v305.remat, align 4
  %v308 = fcmp fast olt float %v307, 0x47EFFFFFE0000000
  br i1 %v308, label %16, label %Hit.BB1.from.Fallback_TraceRay._crit_edge

Hit.BB1.from.Fallback_TraceRay._crit_edge:        ; preds = %Hit.BB1.from.Fallback_TraceRay
  br label %12

; <label>:12                                      ; preds = %Hit.BB1.from.Fallback_TraceRay._crit_edge, %16
  %shadow.0 = phi float [ 0.000000e+00, %16 ], [ 1.000000e+00, %Hit.BB1.from.Fallback_TraceRay._crit_edge ]
  %v364 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %HitShaderConstants114.remat, i32 1)
  %v365 = extractvalue %dx.types.CBufRet.f32 %v364, 0
  %v366 = extractvalue %dx.types.CBufRet.f32 %v364, 1
  %v367 = extractvalue %dx.types.CBufRet.f32 %v364, 2
  %v368 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %HitShaderConstants114.remat, i32 0)
  %v369 = extractvalue %dx.types.CBufRet.f32 %v368, 0
  %v370 = extractvalue %dx.types.CBufRet.f32 %v368, 1
  %v371 = extractvalue %dx.types.CBufRet.f32 %v368, 2
  %.i0344 = fsub fast float %v369, %0
  %.i1345 = fsub fast float %v370, %1
  %.i2346 = fsub fast float %v371, %2
  %v372 = fmul fast float %.i0344, %.i0344
  %v373 = fmul fast float %.i1345, %.i1345
  %v374 = fadd fast float %v372, %v373
  %v375 = fmul fast float %.i2346, %.i2346
  %v376 = fadd fast float %v374, %v375
  %Sqrt = call float @dx.op.unary.f32(i32 24, float %v376)
  %.i0347 = fdiv fast float %.i0344, %Sqrt
  %.i1348 = fdiv fast float %.i1345, %Sqrt
  %.i2349 = fdiv fast float %.i2346, %Sqrt
  %v377 = call float @dx.op.dot3.f32(i32 55, float %v369, float %v370, float %v371, float %.i0347, float %.i1348, float %.i2349)
  %Saturate19 = call float @dx.op.unary.f32(i32 7, float %v377)
  %v378 = fsub fast float 1.000000e+00, %Saturate19
  %Log66 = call float @dx.op.unary.f32(i32 23, float %v378)
  %v379 = fmul fast float %Log66, 5.000000e+00
  %Exp67 = call float @dx.op.unary.f32(i32 21, float %v379)
  %v380 = fmul fast float %Exp67, %3
  %v381 = fmul fast float %Exp67, %4
  %v382 = fmul fast float %Exp67, %5
  %.i0362 = fsub fast float %3, %v380
  %.i1363 = fsub fast float %4, %v381
  %.i2364 = fsub fast float %5, %v382
  %v383 = call float @dx.op.dot3.f32(i32 55, float %6, float %7, float %8, float %v369, float %v370, float %v371)
  %Saturate = call float @dx.op.unary.f32(i32 7, float %v383)
  %.i0365 = fmul fast float %v365, %shadow.0
  %.i0368 = fmul fast float %.i0365, %Saturate
  %.i0371 = fmul fast float %.i0368, %.i0362
  %.i1366 = fmul fast float %v366, %shadow.0
  %.i1369 = fmul fast float %.i1366, %Saturate
  %.i1372 = fmul fast float %.i1369, %.i1363
  %.i2367 = fmul fast float %v367, %shadow.0
  %.i2370 = fmul fast float %.i2367, %Saturate
  %.i2373 = fmul fast float %.i2370, %.i2364
  %.i0374 = fadd fast float %.i0371, %9
  %.i1375 = fadd fast float %.i1372, %10
  %.i2376 = fadd fast float %.i2373, %11
  %v384 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %HitShaderConstants114.remat, i32 8)
  %v385 = extractvalue %dx.types.CBufRet.i32 %v384, 0
  %v386 = icmp eq i32 %v385, 0
  br i1 %v386, label %._crit_edge, label %13

._crit_edge:                                      ; preds = %12
  br label %14

; <label>:13                                      ; preds = %12
  %.i0377 = fmul fast float %.i0374, 0x3FD3333340000000
  %.i1378 = fmul fast float %.i1375, 0x3FD3333340000000
  %.i2379 = fmul fast float %.i2376, 0x3FD3333340000000
  %v387 = call %dx.types.Handle @"dx.op.createHandleFromResourceStructForLib.class.RWTexture2D<vector<float, 4> >"(i32 160, %"class.RWTexture2D<vector<float, 4> >" %v8.remat)
  %TextureLoad113 = call %dx.types.ResRet.f32 @dx.op.textureLoad.f32(i32 66, %dx.types.Handle %v387, i32 undef, i32 %DispatchRaysIndex.int, i32 %DispatchRaysIndex106.int, i32 undef, i32 undef, i32 undef, i32 undef)
  %v388 = extractvalue %dx.types.ResRet.f32 %TextureLoad113, 0
  %v389 = extractvalue %dx.types.ResRet.f32 %TextureLoad113, 1
  %v390 = extractvalue %dx.types.ResRet.f32 %TextureLoad113, 2
  %.i0380 = fadd fast float %v388, %.i0377
  %.i1381 = fadd fast float %v389, %.i1378
  %.i2382 = fadd fast float %v390, %.i2379
  br label %14

; <label>:14                                      ; preds = %._crit_edge, %13
  %outputColor.0.i0 = phi float [ %.i0380, %13 ], [ %.i0374, %._crit_edge ]
  %outputColor.0.i1 = phi float [ %.i1381, %13 ], [ %.i1375, %._crit_edge ]
  %outputColor.0.i2 = phi float [ %.i2382, %13 ], [ %.i2376, %._crit_edge ]
  %v391 = call %dx.types.Handle @"dx.op.createHandleFromResourceStructForLib.class.RWTexture2D<vector<float, 4> >"(i32 160, %"class.RWTexture2D<vector<float, 4> >" %v8.remat)
  call void @dx.op.textureStore.f32(i32 67, %dx.types.Handle %v391, i32 %DispatchRaysIndex.int, i32 %DispatchRaysIndex106.int, i32 undef, float %outputColor.0.i0, float %outputColor.0.i1, float %outputColor.0.i2, float 1.000000e+00, i8 15)
  br label %15

; <label>:15                                      ; preds = %14
  call void @stackFramePop(%struct.RuntimeDataStruct* %runtimeData, i32 32)
  %ret.stackFrame.offset = call i32 @stackFrameOffset(%struct.RuntimeDataStruct* %runtimeData)
  %ret.stateId.ptr = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %ret.stackFrame.offset, i32 0)
  %ret.stateId = load i32, i32* %ret.stateId.ptr
  ret i32 %ret.stateId

; <label>:16                                      ; preds = %Hit.BB1.from.Fallback_TraceRay
  br label %12
}

define i32 @Miss.ss_0(%struct.RuntimeDataStruct* %runtimeData) #9 {
Miss.BB0:
  %payload.offset = call i32 @payloadOffset(%struct.RuntimeDataStruct* %runtimeData)
  %committedAttr.offset = call i32 @committedAttrOffset(%struct.RuntimeDataStruct* %runtimeData)
  %pendingAttr.offset = call i32 @pendingAttrOffset(%struct.RuntimeDataStruct* %runtimeData)
  %stackFrame.offset = call i32 @stackFrameOffset(%struct.RuntimeDataStruct* %runtimeData)
  %offs1 = add i32 0, 0
  %v = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %payload.offset, i32 %offs1)
  %v1 = load i32, i32* %v, align 4
  %offs = add i32 0, 1
  %v2.ptr = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %payload.offset, i32 %offs)
  %v2 = bitcast i32* %v2.ptr to float*
  %v3 = load float, float* %v2, align 4
  %v4 = icmp eq i32 %v1, 0
  br i1 %v4, label %1, label %Miss.BB0._crit_edge

Miss.BB0._crit_edge:                              ; preds = %Miss.BB0
  br label %0

; <label>:0                                       ; preds = %Miss.BB0._crit_edge, %1
  store i32 %v1, i32* %v, align 4
  store float %v3, float* %v2, align 4
  %ret.stateId.ptr = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %stackFrame.offset, i32 0)
  %ret.stateId = load i32, i32* %ret.stateId.ptr
  ret i32 %ret.stateId

; <label>:1                                       ; preds = %Miss.BB0
  %v5 = load %"class.RWTexture2D<vector<float, 4> >", %"class.RWTexture2D<vector<float, 4> >"* @"\01?g_screenOutput@@3V?$RWTexture2D@V?$vector@M$03@@@@A", align 4
  %DispatchRaysIndex = call i32 @fb_dxop_dispatchRaysIndex(%struct.RuntimeDataStruct* %runtimeData, i8 0)
  %DispatchRaysIndex1 = call i32 @fb_dxop_dispatchRaysIndex(%struct.RuntimeDataStruct* %runtimeData, i8 1)
  %v6 = call %dx.types.Handle @"dx.op.createHandleFromResourceStructForLib.class.RWTexture2D<vector<float, 4> >"(i32 160, %"class.RWTexture2D<vector<float, 4> >" %v5)
  call void @dx.op.textureStore.f32(i32 67, %dx.types.Handle %v6, i32 %DispatchRaysIndex, i32 %DispatchRaysIndex1, i32 undef, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 1.000000e+00, i8 15)
  br label %0
}

define i32 @Fallback_TraceRay.ss_0(%struct.RuntimeDataStruct* %runtimeData) #9 {
Fallback_TraceRay.BB0:
  %tmp30 = alloca %class.matrix.float.3.4
  %tmp29 = alloca %class.matrix.float.3.4
  call void @traceFramePush(%struct.RuntimeDataStruct* %runtimeData, i32 8)
  %payload.offset = call i32 @payloadOffset(%struct.RuntimeDataStruct* %runtimeData)
  %committedAttr.offset = call i32 @committedAttrOffset(%struct.RuntimeDataStruct* %runtimeData)
  %pendingAttr.offset = call i32 @pendingAttrOffset(%struct.RuntimeDataStruct* %runtimeData)
  call void @stackFramePush(%struct.RuntimeDataStruct* %runtimeData, i32 24)
  %stackFrame.offset = call i32 @stackFrameOffset(%struct.RuntimeDataStruct* %runtimeData)
  %callerArgFrame.offset = add i32 %stackFrame.offset, 24
  %arg0.ptr = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %callerArgFrame.offset, i32 1)
  %arg0.int = load i32, i32* %arg0.ptr
  %arg1.ptr = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %callerArgFrame.offset, i32 2)
  %arg1.int = load i32, i32* %arg1.ptr
  %arg2.ptr = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %callerArgFrame.offset, i32 3)
  %arg2.int = load i32, i32* %arg2.ptr
  %arg3.ptr = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %callerArgFrame.offset, i32 4)
  %arg3.int = load i32, i32* %arg3.ptr
  %arg4.ptr = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %callerArgFrame.offset, i32 5)
  %arg4.int = load i32, i32* %arg4.ptr
  %arg5.ptr = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %callerArgFrame.offset, i32 6)
  %arg5.int = load i32, i32* %arg5.ptr
  %0 = bitcast i32 %arg5.int to float
  %arg6.ptr = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %callerArgFrame.offset, i32 7)
  %arg6.int = load i32, i32* %arg6.ptr
  %1 = bitcast i32 %arg6.int to float
  %arg7.ptr = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %callerArgFrame.offset, i32 8)
  %arg7.int = load i32, i32* %arg7.ptr
  %2 = bitcast i32 %arg7.int to float
  %arg8.ptr = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %callerArgFrame.offset, i32 9)
  %arg8.int = load i32, i32* %arg8.ptr
  %3 = bitcast i32 %arg8.int to float
  %arg9.ptr = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %callerArgFrame.offset, i32 10)
  %arg9.int = load i32, i32* %arg9.ptr
  %4 = bitcast i32 %arg9.int to float
  %arg10.ptr = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %callerArgFrame.offset, i32 11)
  %arg10.int = load i32, i32* %arg10.ptr
  %5 = bitcast i32 %arg10.int to float
  %arg11.ptr = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %callerArgFrame.offset, i32 12)
  %arg11.int = load i32, i32* %arg11.ptr
  %6 = bitcast i32 %arg11.int to float
  %arg12.ptr = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %callerArgFrame.offset, i32 13)
  %arg12.int = load i32, i32* %arg12.ptr
  %7 = bitcast i32 %arg12.int to float
  %arg13.ptr = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %callerArgFrame.offset, i32 14)
  %arg13.int = load i32, i32* %arg13.ptr
  %v = load %struct.ByteAddressBuffer, %struct.ByteAddressBuffer* @"\01?MissShaderTable@@3UByteAddressBuffer@@A", align 4
  %v16 = load %struct.ByteAddressBuffer, %struct.ByteAddressBuffer* @"\01?HitGroupShaderTable@@3UByteAddressBuffer@@A", align 4
  %v17 = load %AccelerationStructureList, %AccelerationStructureList* @AccelerationStructureList, align 4
  %v18 = load %Constants, %Constants* @Constants, align 4
  %AccelerationStructureList = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.AccelerationStructureList(i32 160, %AccelerationStructureList %v17)
  %Constants295 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.Constants(i32 160, %Constants %v18)
  %v19 = alloca [3 x float], align 4
  %v20 = alloca [3 x float], align 4
  %v21 = alloca [3 x float], align 4
  %v22 = alloca [3 x float], align 4
  %v23 = alloca [3 x float], align 4
  %nodesToProcess.i.i = alloca [2 x i32], align 4
  %attr.i.i = alloca %struct.BuiltInTriangleIntersectionAttributes, align 4
  %v24 = call i32 @fb_Fallback_SetPayloadOffset(%struct.RuntimeDataStruct* %runtimeData, i32 %arg13.int)
  call void @fb_Fallback_SetRayFlags(%struct.RuntimeDataStruct* %runtimeData, i32 %arg0.int)
  %v25 = insertelement <3 x float> undef, float %4, i64 0
  %v26 = insertelement <3 x float> %v25, float %5, i64 1
  %v27 = insertelement <3 x float> %v26, float %6, i64 2
  %v28 = insertelement <3 x float> undef, float %0, i64 0
  %v29 = insertelement <3 x float> %v28, float %1, i64 1
  %v30 = insertelement <3 x float> %v29, float %2, i64 2
  %vec.x = extractelement <3 x float> %v30, i32 0
  %vec.y = extractelement <3 x float> %v30, i32 1
  %vec.z = extractelement <3 x float> %v30, i32 2
  %vec.x20 = extractelement <3 x float> %v27, i32 0
  %vec.y21 = extractelement <3 x float> %v27, i32 1
  %vec.z22 = extractelement <3 x float> %v27, i32 2
  call void @fb_Fallback_TraceRayBegin(%struct.RuntimeDataStruct* %runtimeData, float %vec.x, float %vec.y, float %vec.z, float %3, float %vec.x20, float %vec.y21, float %vec.z22, float %7)
  %v31 = call i32 @fb_Fallback_GroupIndex(%struct.RuntimeDataStruct* %runtimeData)
  %WorldRayDirection251 = call float @fb_dxop_worldRayDirection(%struct.RuntimeDataStruct* %runtimeData, i8 0)
  %WorldRayDirection252 = call float @fb_dxop_worldRayDirection(%struct.RuntimeDataStruct* %runtimeData, i8 1)
  %WorldRayDirection253 = call float @fb_dxop_worldRayDirection(%struct.RuntimeDataStruct* %runtimeData, i8 2)
  %WorldRayOrigin248 = call float @fb_dxop_worldRayOrigin(%struct.RuntimeDataStruct* %runtimeData, i8 0)
  %WorldRayOrigin249 = call float @fb_dxop_worldRayOrigin(%struct.RuntimeDataStruct* %runtimeData, i8 1)
  %WorldRayOrigin250 = call float @fb_dxop_worldRayOrigin(%struct.RuntimeDataStruct* %runtimeData, i8 2)
  %v32 = getelementptr inbounds [3 x float], [3 x float]* %v23, i32 0, i32 0
  store float %WorldRayDirection251, float* %v32, align 4
  %v33 = getelementptr inbounds [3 x float], [3 x float]* %v23, i32 0, i32 1
  store float %WorldRayDirection252, float* %v33, align 4
  %v34 = getelementptr inbounds [3 x float], [3 x float]* %v23, i32 0, i32 2
  store float %WorldRayDirection253, float* %v34, align 4
  %.i0 = fdiv fast float 1.000000e+00, %WorldRayDirection251
  %.i1 = fdiv fast float 1.000000e+00, %WorldRayDirection252
  %.i2 = fdiv fast float 1.000000e+00, %WorldRayDirection253
  %.i0296 = fmul fast float %.i0, %WorldRayOrigin248
  %.i1297 = fmul fast float %.i1, %WorldRayOrigin249
  %.i2298 = fmul fast float %.i2, %WorldRayOrigin250
  %FAbs241 = call float @dx.op.unary.f32(i32 6, float %WorldRayDirection251)
  %FAbs242 = call float @dx.op.unary.f32(i32 6, float %WorldRayDirection252)
  %FAbs243 = call float @dx.op.unary.f32(i32 6, float %WorldRayDirection253)
  %v35 = fcmp fast ogt float %FAbs241, %FAbs242
  %v36 = fcmp fast ogt float %FAbs241, %FAbs243
  %v37 = and i1 %v35, %v36
  br i1 %v37, label %"Fallback_TraceRay.BB0.\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i_crit_edge", label %8

"Fallback_TraceRay.BB0.\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i_crit_edge": ; preds = %Fallback_TraceRay.BB0
  br label %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i"

; <label>:8                                       ; preds = %Fallback_TraceRay.BB0
  %v38 = fcmp fast ogt float %FAbs242, %FAbs243
  br i1 %v38, label %".\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i_crit_edge", label %9

".\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i_crit_edge": ; preds = %8
  br label %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i"

; <label>:9                                       ; preds = %8
  br label %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i"

"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i": ; preds = %".\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i_crit_edge", %"Fallback_TraceRay.BB0.\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i_crit_edge", %9
  %.0171 = phi i32 [ 2, %9 ], [ 0, %"Fallback_TraceRay.BB0.\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i_crit_edge" ], [ 1, %".\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i_crit_edge" ]
  %v39 = add nuw nsw i32 %.0171, 1
  %v40 = urem i32 %v39, 3
  %v41 = add nuw nsw i32 %.0171, 2
  %v42 = urem i32 %v41, 3
  %v43 = getelementptr [3 x float], [3 x float]* %v23, i32 0, i32 %.0171
  %v44 = load float, float* %v43, align 4, !tbaa !231, !noalias !339
  %v45 = fcmp fast olt float %v44, 0.000000e+00
  %worldRayData.i.i.5.0.i0 = select i1 %v45, i32 %v42, i32 %v40
  %worldRayData.i.i.5.0.i1 = select i1 %v45, i32 %v40, i32 %v42
  %v46 = getelementptr [3 x float], [3 x float]* %v23, i32 0, i32 %worldRayData.i.i.5.0.i0
  %v47 = load float, float* %v46, align 4, !tbaa !231, !noalias !339
  %v48 = fdiv float %v47, %v44
  %v49 = getelementptr [3 x float], [3 x float]* %v23, i32 0, i32 %worldRayData.i.i.5.0.i1
  %v50 = load float, float* %v49, align 4, !tbaa !231, !noalias !339
  %v51 = fdiv float %v50, %v44
  %v52 = fdiv float 1.000000e+00, %v44
  %v53 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %AccelerationStructureList, i32 0)
  %v54 = extractvalue %dx.types.CBufRet.i32 %v53, 0
  %v55 = extractvalue %dx.types.CBufRet.i32 %v53, 1
  %v56 = getelementptr inbounds [2 x i32], [2 x i32]* %nodesToProcess.i.i, i32 0, i32 0
  store i32 0, i32* %v56, align 4, !tbaa !221
  %v57 = getelementptr inbounds [0 x %struct.RWByteAddressBuffer], [0 x %struct.RWByteAddressBuffer]* @"\01?DescriptorHeapBufferTable@@3PAURWByteAddressBuffer@@A", i32 0, i32 %v55, !dx.nonuniform !240
  %v58 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %v57, align 4, !noalias !125
  %v59 = add i32 %v54, 4
  %v60 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %v58)
  %v61 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %v60, i32 %v59, i32 undef)
  %v62 = extractvalue %dx.types.ResRet.i32 %v61, 0
  %v63 = add i32 %v62, %v54
  %v64 = add i32 %v54, 16
  %v65 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %v60, i32 %v64, i32 undef)
  %v66 = extractvalue %dx.types.ResRet.i32 %v65, 0
  %v67 = extractvalue %dx.types.ResRet.i32 %v65, 1
  %v68 = extractvalue %dx.types.ResRet.i32 %v65, 2
  %v69 = add i32 %v54, 32
  %v70 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %v60, i32 %v69, i32 undef)
  %v71 = extractvalue %dx.types.ResRet.i32 %v70, 0
  %v72 = extractvalue %dx.types.ResRet.i32 %v70, 1
  %v73 = extractvalue %dx.types.ResRet.i32 %v70, 2
  %v74 = bitcast i32 %v66 to float
  %v75 = bitcast i32 %v67 to float
  %v76 = bitcast i32 %v68 to float
  %v77 = bitcast i32 %v71 to float
  %v78 = bitcast i32 %v72 to float
  %v79 = bitcast i32 %v73 to float
  %RayTCurrent220 = call float @fb_dxop_rayTCurrent(%struct.RuntimeDataStruct* %runtimeData)
  %v80 = fsub fast float %v74, %WorldRayOrigin248
  %v81 = fmul fast float %v80, %.i0
  %v82 = fsub fast float %v75, %WorldRayOrigin249
  %v83 = fmul fast float %v82, %.i1
  %v84 = fsub fast float %v76, %WorldRayOrigin250
  %v85 = fmul fast float %v84, %.i2
  %FAbs238 = call float @dx.op.unary.f32(i32 6, float %.i0)
  %FAbs239 = call float @dx.op.unary.f32(i32 6, float %.i1)
  %FAbs240 = call float @dx.op.unary.f32(i32 6, float %.i2)
  %.i0305 = fmul fast float %v77, %FAbs238
  %.i1306 = fmul fast float %FAbs239, %v78
  %.i2307 = fmul fast float %FAbs240, %v79
  %.i0308 = fadd fast float %.i0305, %v81
  %.i1309 = fadd fast float %.i1306, %v83
  %.i2310 = fadd fast float %.i2307, %v85
  %.i0314 = fsub fast float %v81, %.i0305
  %.i1315 = fsub fast float %v83, %.i1306
  %.i2316 = fsub fast float %v85, %.i2307
  %FMax277 = call float @dx.op.binary.f32(i32 35, float %.i0314, float %.i1315)
  %FMax276 = call float @dx.op.binary.f32(i32 35, float %FMax277, float %.i2316)
  %FMin275 = call float @dx.op.binary.f32(i32 36, float %.i0308, float %.i1309)
  %FMin274 = call float @dx.op.binary.f32(i32 36, float %FMin275, float %.i2310)
  %FMax273 = call float @dx.op.binary.f32(i32 35, float %FMax276, float 0.000000e+00)
  %FMin272 = call float @dx.op.binary.f32(i32 36, float %FMin274, float %RayTCurrent220)
  %v86 = fcmp fast olt float %FMax273, %FMin272
  br i1 %v86, label %.lr.ph.preheader.critedge, label %10

; <label>:10                                      ; preds = %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i"
  call void @fb_Fallback_SetInstanceIndex(%struct.RuntimeDataStruct* %runtimeData, i32 -1)
  br label %._crit_edge

._crit_edge:                                      ; preds = %.._crit_edge_crit_edge, %10
  %InstanceIndex = call i32 @fb_dxop_instanceIndex(%struct.RuntimeDataStruct* %runtimeData)
  %v427 = icmp eq i32 %InstanceIndex, -1
  br i1 %v427, label %14, label %11

; <label>:11                                      ; preds = %._crit_edge
  %RayFlags258 = call i32 @fb_dxop_rayFlags(%struct.RuntimeDataStruct* %runtimeData)
  %v428 = and i32 %RayFlags258, 8
  %v429 = icmp eq i32 %v428, 0
  br i1 %v429, label %13, label %".\01?Traverse@@YAHIIII@Z.exit_crit_edge"

".\01?Traverse@@YAHIIII@Z.exit_crit_edge":        ; preds = %11
  br label %"\01?Traverse@@YAHIIII@Z.exit"

"\01?Traverse@@YAHIIII@Z.exit":                   ; preds = %".\01?Traverse@@YAHIIII@Z.exit_crit_edge", %14, %13
  %stateID.i.0 = phi i32 [ %v436, %13 ], [ %v442, %14 ], [ 0, %".\01?Traverse@@YAHIIII@Z.exit_crit_edge" ]
  %v443 = icmp eq i32 %stateID.i.0, 0
  br i1 %v443, label %12, label %remat_begin

remat_begin:                                      ; preds = %"\01?Traverse@@YAHIIII@Z.exit"
  %intPtr17 = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %stackFrame.offset, i32 0)
  store i32 1006, i32* %intPtr17
  %intPtr = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %stackFrame.offset, i32 4)
  store i32 %v24, i32* %intPtr
  ret i32 %stateID.i.0

; <label>:12                                      ; preds = %"\01?Traverse@@YAHIIII@Z.exit"
  %v444 = call i32 @fb_Fallback_SetPayloadOffset(%struct.RuntimeDataStruct* %runtimeData, i32 %v24)
  call void @stackFramePop(%struct.RuntimeDataStruct* %runtimeData, i32 24)
  call void @traceFramePop(%struct.RuntimeDataStruct* %runtimeData)
  %ret.stackFrame.offset = call i32 @stackFrameOffset(%struct.RuntimeDataStruct* %runtimeData)
  %ret.stateId.ptr = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %ret.stackFrame.offset, i32 0)
  %ret.stateId = load i32, i32* %ret.stateId.ptr
  ret i32 %ret.stateId

; <label>:13                                      ; preds = %11
  %v430 = call i32 @fb_Fallback_GeometryIndex(%struct.RuntimeDataStruct* %runtimeData)
  %v431 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %Constants295, i32 0)
  %v432 = extractvalue %dx.types.CBufRet.i32 %v431, 2
  %v433 = mul i32 %v432, %v430
  %v434 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.ByteAddressBuffer(i32 160, %struct.ByteAddressBuffer %v16)
  %v435 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %v434, i32 %v433, i32 undef)
  %v436 = extractvalue %dx.types.ResRet.i32 %v435, 0
  br label %"\01?Traverse@@YAHIIII@Z.exit"

; <label>:14                                      ; preds = %._crit_edge
  %v437 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %Constants295, i32 0)
  %v438 = extractvalue %dx.types.CBufRet.i32 %v437, 3
  %v439 = mul i32 %v438, %arg4.int
  %v440 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.ByteAddressBuffer(i32 160, %struct.ByteAddressBuffer %v)
  %v441 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %v440, i32 %v439, i32 undef)
  %v442 = extractvalue %dx.types.ResRet.i32 %v441, 0
  br label %"\01?Traverse@@YAHIIII@Z.exit"

.lr.ph.preheader.critedge:                        ; preds = %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i"
  %v87 = getelementptr [2048 x i32], [2048 x i32] addrspace(3)* @"\01?stack@@3PAIA", i32 0, i32 %v31
  store i32 0, i32 addrspace(3)* %v87, align 4, !tbaa !221, !noalias !342
  store i32 1, i32* %v56, align 4, !tbaa !221
  call void @fb_Fallback_SetInstanceIndex(%struct.RuntimeDataStruct* %runtimeData, i32 -1)
  %v88 = getelementptr inbounds [3 x float], [3 x float]* %v22, i32 0, i32 0
  %v89 = getelementptr inbounds [3 x float], [3 x float]* %v22, i32 0, i32 1
  %v90 = getelementptr inbounds [3 x float], [3 x float]* %v22, i32 0, i32 2
  %v91 = getelementptr inbounds [2 x i32], [2 x i32]* %nodesToProcess.i.i, i32 0, i32 1
  %v92 = getelementptr inbounds [3 x float], [3 x float]* %v21, i32 0, i32 0
  %v93 = getelementptr inbounds [3 x float], [3 x float]* %v21, i32 0, i32 1
  %v94 = getelementptr inbounds [3 x float], [3 x float]* %v21, i32 0, i32 2
  %v95 = getelementptr inbounds [3 x float], [3 x float]* %v19, i32 0, i32 0
  %v96 = getelementptr inbounds [3 x float], [3 x float]* %v19, i32 0, i32 1
  %v97 = getelementptr inbounds [3 x float], [3 x float]* %v19, i32 0, i32 2
  %v98 = getelementptr inbounds [3 x float], [3 x float]* %v20, i32 0, i32 0
  %v99 = getelementptr inbounds [3 x float], [3 x float]* %v20, i32 0, i32 1
  %v100 = getelementptr inbounds [3 x float], [3 x float]* %v20, i32 0, i32 2
  %v101 = getelementptr inbounds %struct.BuiltInTriangleIntersectionAttributes, %struct.BuiltInTriangleIntersectionAttributes* %attr.i.i, i32 0, i32 0
  br label %.lr.ph

.lr.ph:                                           ; preds = %..lr.ph_crit_edge, %.lr.ph.preheader.critedge
  %resultTriId.i.i.0188 = phi i32 [ %resultTriId.i.i.2, %..lr.ph_crit_edge ], [ undef, %.lr.ph.preheader.critedge ]
  %resultBary.i.i.0187.i0 = phi float [ %resultBary.i.i.2.i0, %..lr.ph_crit_edge ], [ undef, %.lr.ph.preheader.critedge ]
  %resultBary.i.i.0187.i1 = phi float [ %resultBary.i.i.2.i1, %..lr.ph_crit_edge ], [ undef, %.lr.ph.preheader.critedge ]
  %stackPointer.i.i.1185 = phi i32 [ %stackPointer.i.i.3, %..lr.ph_crit_edge ], [ 1, %.lr.ph.preheader.critedge ]
  %instId.i.i.0184 = phi i32 [ %instId.i.i.2, %..lr.ph_crit_edge ], [ 0, %.lr.ph.preheader.critedge ]
  %instOffset.i.i.0183 = phi i32 [ %instOffset.i.i.2, %..lr.ph_crit_edge ], [ 0, %.lr.ph.preheader.critedge ]
  %instFlags.i.i.0182 = phi i32 [ %instFlags.i.i.2, %..lr.ph_crit_edge ], [ 0, %.lr.ph.preheader.critedge ]
  %instIdx.i.i.0181 = phi i32 [ %instIdx.i.i.2, %..lr.ph_crit_edge ], [ 0, %.lr.ph.preheader.critedge ]
  %currentGpuVA.i.i.0180.i0 = phi i32 [ %v423, %..lr.ph_crit_edge ], [ %v54, %.lr.ph.preheader.critedge ]
  %currentGpuVA.i.i.0180.i1 = phi i32 [ %v424, %..lr.ph_crit_edge ], [ %v55, %.lr.ph.preheader.critedge ]
  %currentBVHIndex.i.i.0179 = phi i32 [ %v421, %..lr.ph_crit_edge ], [ 0, %.lr.ph.preheader.critedge ]
  %CurrentWorldToObject.i.i168.0178.i0 = phi float [ %CurrentWorldToObject.i.i168.2.i0, %..lr.ph_crit_edge ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentWorldToObject.i.i168.0178.i1 = phi float [ %CurrentWorldToObject.i.i168.2.i1, %..lr.ph_crit_edge ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentWorldToObject.i.i168.0178.i2 = phi float [ %CurrentWorldToObject.i.i168.2.i2, %..lr.ph_crit_edge ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentWorldToObject.i.i168.0178.i3 = phi float [ %CurrentWorldToObject.i.i168.2.i3, %..lr.ph_crit_edge ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentWorldToObject.i.i168.0178.i4 = phi float [ %CurrentWorldToObject.i.i168.2.i4, %..lr.ph_crit_edge ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentWorldToObject.i.i168.0178.i5 = phi float [ %CurrentWorldToObject.i.i168.2.i5, %..lr.ph_crit_edge ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentWorldToObject.i.i168.0178.i6 = phi float [ %CurrentWorldToObject.i.i168.2.i6, %..lr.ph_crit_edge ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentWorldToObject.i.i168.0178.i7 = phi float [ %CurrentWorldToObject.i.i168.2.i7, %..lr.ph_crit_edge ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentWorldToObject.i.i168.0178.i8 = phi float [ %CurrentWorldToObject.i.i168.2.i8, %..lr.ph_crit_edge ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentWorldToObject.i.i168.0178.i9 = phi float [ %CurrentWorldToObject.i.i168.2.i9, %..lr.ph_crit_edge ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentWorldToObject.i.i168.0178.i10 = phi float [ %CurrentWorldToObject.i.i168.2.i10, %..lr.ph_crit_edge ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentWorldToObject.i.i168.0178.i11 = phi float [ %CurrentWorldToObject.i.i168.2.i11, %..lr.ph_crit_edge ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentObjectToWorld.i.i167.0177.i0 = phi float [ %CurrentObjectToWorld.i.i167.2.i0, %..lr.ph_crit_edge ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentObjectToWorld.i.i167.0177.i1 = phi float [ %CurrentObjectToWorld.i.i167.2.i1, %..lr.ph_crit_edge ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentObjectToWorld.i.i167.0177.i2 = phi float [ %CurrentObjectToWorld.i.i167.2.i2, %..lr.ph_crit_edge ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentObjectToWorld.i.i167.0177.i3 = phi float [ %CurrentObjectToWorld.i.i167.2.i3, %..lr.ph_crit_edge ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentObjectToWorld.i.i167.0177.i4 = phi float [ %CurrentObjectToWorld.i.i167.2.i4, %..lr.ph_crit_edge ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentObjectToWorld.i.i167.0177.i5 = phi float [ %CurrentObjectToWorld.i.i167.2.i5, %..lr.ph_crit_edge ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentObjectToWorld.i.i167.0177.i6 = phi float [ %CurrentObjectToWorld.i.i167.2.i6, %..lr.ph_crit_edge ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentObjectToWorld.i.i167.0177.i7 = phi float [ %CurrentObjectToWorld.i.i167.2.i7, %..lr.ph_crit_edge ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentObjectToWorld.i.i167.0177.i8 = phi float [ %CurrentObjectToWorld.i.i167.2.i8, %..lr.ph_crit_edge ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentObjectToWorld.i.i167.0177.i9 = phi float [ %CurrentObjectToWorld.i.i167.2.i9, %..lr.ph_crit_edge ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentObjectToWorld.i.i167.0177.i10 = phi float [ %CurrentObjectToWorld.i.i167.2.i10, %..lr.ph_crit_edge ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentObjectToWorld.i.i167.0177.i11 = phi float [ %CurrentObjectToWorld.i.i167.2.i11, %..lr.ph_crit_edge ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %.0174176.i0 = phi float [ %.3.i0, %..lr.ph_crit_edge ], [ undef, %.lr.ph.preheader.critedge ]
  %.0174176.i1 = phi float [ %.3.i1, %..lr.ph_crit_edge ], [ undef, %.lr.ph.preheader.critedge ]
  %.phi.trans.insert = getelementptr inbounds [2 x i32], [2 x i32]* %nodesToProcess.i.i, i32 0, i32 %currentBVHIndex.i.i.0179
  %.pre = load i32, i32* %.phi.trans.insert, align 4, !tbaa !221
  br label %15

; <label>:15                                      ; preds = %._crit_edge.2, %.lr.ph
  %v102 = phi i32 [ %.pre, %.lr.ph ], [ %v419, %._crit_edge.2 ]
  %.1.i0 = phi float [ %.0174176.i0, %.lr.ph ], [ %.3.i0, %._crit_edge.2 ]
  %.1.i1 = phi float [ %.0174176.i1, %.lr.ph ], [ %.3.i1, %._crit_edge.2 ]
  %currentRayData.i.i.0.1.i0 = phi float [ %WorldRayOrigin248, %.lr.ph ], [ %currentRayData.i.i.0.2.i0, %._crit_edge.2 ]
  %currentRayData.i.i.0.1.i1 = phi float [ %WorldRayOrigin249, %.lr.ph ], [ %currentRayData.i.i.0.2.i1, %._crit_edge.2 ]
  %currentRayData.i.i.0.1.i2 = phi float [ %WorldRayOrigin250, %.lr.ph ], [ %currentRayData.i.i.0.2.i2, %._crit_edge.2 ]
  %currentRayData.i.i.1.1.i0 = phi float [ %WorldRayDirection251, %.lr.ph ], [ %currentRayData.i.i.1.2.i0, %._crit_edge.2 ]
  %currentRayData.i.i.1.1.i1 = phi float [ %WorldRayDirection252, %.lr.ph ], [ %currentRayData.i.i.1.2.i1, %._crit_edge.2 ]
  %currentRayData.i.i.1.1.i2 = phi float [ %WorldRayDirection253, %.lr.ph ], [ %currentRayData.i.i.1.2.i2, %._crit_edge.2 ]
  %currentRayData.i.i.2.1.i0 = phi float [ %.i0, %.lr.ph ], [ %currentRayData.i.i.2.2.i0, %._crit_edge.2 ]
  %currentRayData.i.i.2.1.i1 = phi float [ %.i1, %.lr.ph ], [ %currentRayData.i.i.2.2.i1, %._crit_edge.2 ]
  %currentRayData.i.i.2.1.i2 = phi float [ %.i2, %.lr.ph ], [ %currentRayData.i.i.2.2.i2, %._crit_edge.2 ]
  %currentRayData.i.i.3.1.i0 = phi float [ %.i0296, %.lr.ph ], [ %currentRayData.i.i.3.2.i0, %._crit_edge.2 ]
  %currentRayData.i.i.3.1.i1 = phi float [ %.i1297, %.lr.ph ], [ %currentRayData.i.i.3.2.i1, %._crit_edge.2 ]
  %currentRayData.i.i.3.1.i2 = phi float [ %.i2298, %.lr.ph ], [ %currentRayData.i.i.3.2.i2, %._crit_edge.2 ]
  %currentRayData.i.i.4.1.i0 = phi float [ %v48, %.lr.ph ], [ %currentRayData.i.i.4.2.i0, %._crit_edge.2 ]
  %currentRayData.i.i.4.1.i1 = phi float [ %v51, %.lr.ph ], [ %currentRayData.i.i.4.2.i1, %._crit_edge.2 ]
  %currentRayData.i.i.4.1.i2 = phi float [ %v52, %.lr.ph ], [ %currentRayData.i.i.4.2.i2, %._crit_edge.2 ]
  %currentRayData.i.i.5.1.i0 = phi i32 [ %worldRayData.i.i.5.0.i0, %.lr.ph ], [ %currentRayData.i.i.5.2.i0, %._crit_edge.2 ]
  %currentRayData.i.i.5.1.i1 = phi i32 [ %worldRayData.i.i.5.0.i1, %.lr.ph ], [ %currentRayData.i.i.5.2.i1, %._crit_edge.2 ]
  %currentRayData.i.i.5.1.i2 = phi i32 [ %.0171, %.lr.ph ], [ %currentRayData.i.i.5.2.i2, %._crit_edge.2 ]
  %CurrentObjectToWorld.i.i167.1.i0 = phi float [ %CurrentObjectToWorld.i.i167.0177.i0, %.lr.ph ], [ %CurrentObjectToWorld.i.i167.2.i0, %._crit_edge.2 ]
  %CurrentObjectToWorld.i.i167.1.i1 = phi float [ %CurrentObjectToWorld.i.i167.0177.i1, %.lr.ph ], [ %CurrentObjectToWorld.i.i167.2.i1, %._crit_edge.2 ]
  %CurrentObjectToWorld.i.i167.1.i2 = phi float [ %CurrentObjectToWorld.i.i167.0177.i2, %.lr.ph ], [ %CurrentObjectToWorld.i.i167.2.i2, %._crit_edge.2 ]
  %CurrentObjectToWorld.i.i167.1.i3 = phi float [ %CurrentObjectToWorld.i.i167.0177.i3, %.lr.ph ], [ %CurrentObjectToWorld.i.i167.2.i3, %._crit_edge.2 ]
  %CurrentObjectToWorld.i.i167.1.i4 = phi float [ %CurrentObjectToWorld.i.i167.0177.i4, %.lr.ph ], [ %CurrentObjectToWorld.i.i167.2.i4, %._crit_edge.2 ]
  %CurrentObjectToWorld.i.i167.1.i5 = phi float [ %CurrentObjectToWorld.i.i167.0177.i5, %.lr.ph ], [ %CurrentObjectToWorld.i.i167.2.i5, %._crit_edge.2 ]
  %CurrentObjectToWorld.i.i167.1.i6 = phi float [ %CurrentObjectToWorld.i.i167.0177.i6, %.lr.ph ], [ %CurrentObjectToWorld.i.i167.2.i6, %._crit_edge.2 ]
  %CurrentObjectToWorld.i.i167.1.i7 = phi float [ %CurrentObjectToWorld.i.i167.0177.i7, %.lr.ph ], [ %CurrentObjectToWorld.i.i167.2.i7, %._crit_edge.2 ]
  %CurrentObjectToWorld.i.i167.1.i8 = phi float [ %CurrentObjectToWorld.i.i167.0177.i8, %.lr.ph ], [ %CurrentObjectToWorld.i.i167.2.i8, %._crit_edge.2 ]
  %CurrentObjectToWorld.i.i167.1.i9 = phi float [ %CurrentObjectToWorld.i.i167.0177.i9, %.lr.ph ], [ %CurrentObjectToWorld.i.i167.2.i9, %._crit_edge.2 ]
  %CurrentObjectToWorld.i.i167.1.i10 = phi float [ %CurrentObjectToWorld.i.i167.0177.i10, %.lr.ph ], [ %CurrentObjectToWorld.i.i167.2.i10, %._crit_edge.2 ]
  %CurrentObjectToWorld.i.i167.1.i11 = phi float [ %CurrentObjectToWorld.i.i167.0177.i11, %.lr.ph ], [ %CurrentObjectToWorld.i.i167.2.i11, %._crit_edge.2 ]
  %CurrentWorldToObject.i.i168.1.i0 = phi float [ %CurrentWorldToObject.i.i168.0178.i0, %.lr.ph ], [ %CurrentWorldToObject.i.i168.2.i0, %._crit_edge.2 ]
  %CurrentWorldToObject.i.i168.1.i1 = phi float [ %CurrentWorldToObject.i.i168.0178.i1, %.lr.ph ], [ %CurrentWorldToObject.i.i168.2.i1, %._crit_edge.2 ]
  %CurrentWorldToObject.i.i168.1.i2 = phi float [ %CurrentWorldToObject.i.i168.0178.i2, %.lr.ph ], [ %CurrentWorldToObject.i.i168.2.i2, %._crit_edge.2 ]
  %CurrentWorldToObject.i.i168.1.i3 = phi float [ %CurrentWorldToObject.i.i168.0178.i3, %.lr.ph ], [ %CurrentWorldToObject.i.i168.2.i3, %._crit_edge.2 ]
  %CurrentWorldToObject.i.i168.1.i4 = phi float [ %CurrentWorldToObject.i.i168.0178.i4, %.lr.ph ], [ %CurrentWorldToObject.i.i168.2.i4, %._crit_edge.2 ]
  %CurrentWorldToObject.i.i168.1.i5 = phi float [ %CurrentWorldToObject.i.i168.0178.i5, %.lr.ph ], [ %CurrentWorldToObject.i.i168.2.i5, %._crit_edge.2 ]
  %CurrentWorldToObject.i.i168.1.i6 = phi float [ %CurrentWorldToObject.i.i168.0178.i6, %.lr.ph ], [ %CurrentWorldToObject.i.i168.2.i6, %._crit_edge.2 ]
  %CurrentWorldToObject.i.i168.1.i7 = phi float [ %CurrentWorldToObject.i.i168.0178.i7, %.lr.ph ], [ %CurrentWorldToObject.i.i168.2.i7, %._crit_edge.2 ]
  %CurrentWorldToObject.i.i168.1.i8 = phi float [ %CurrentWorldToObject.i.i168.0178.i8, %.lr.ph ], [ %CurrentWorldToObject.i.i168.2.i8, %._crit_edge.2 ]
  %CurrentWorldToObject.i.i168.1.i9 = phi float [ %CurrentWorldToObject.i.i168.0178.i9, %.lr.ph ], [ %CurrentWorldToObject.i.i168.2.i9, %._crit_edge.2 ]
  %CurrentWorldToObject.i.i168.1.i10 = phi float [ %CurrentWorldToObject.i.i168.0178.i10, %.lr.ph ], [ %CurrentWorldToObject.i.i168.2.i10, %._crit_edge.2 ]
  %CurrentWorldToObject.i.i168.1.i11 = phi float [ %CurrentWorldToObject.i.i168.0178.i11, %.lr.ph ], [ %CurrentWorldToObject.i.i168.2.i11, %._crit_edge.2 ]
  %ResetMatrices.i.i.1 = phi i32 [ 1, %.lr.ph ], [ %ResetMatrices.i.i.3, %._crit_edge.2 ]
  %currentBVHIndex.i.i.1 = phi i32 [ %currentBVHIndex.i.i.0179, %.lr.ph ], [ %currentBVHIndex.i.i.2, %._crit_edge.2 ]
  %currentGpuVA.i.i.1.i0 = phi i32 [ %currentGpuVA.i.i.0180.i0, %.lr.ph ], [ %currentGpuVA.i.i.2.i0, %._crit_edge.2 ]
  %currentGpuVA.i.i.1.i1 = phi i32 [ %currentGpuVA.i.i.0180.i1, %.lr.ph ], [ %currentGpuVA.i.i.2.i1, %._crit_edge.2 ]
  %instIdx.i.i.1 = phi i32 [ %instIdx.i.i.0181, %.lr.ph ], [ %instIdx.i.i.2, %._crit_edge.2 ]
  %instFlags.i.i.1 = phi i32 [ %instFlags.i.i.0182, %.lr.ph ], [ %instFlags.i.i.2, %._crit_edge.2 ]
  %instOffset.i.i.1 = phi i32 [ %instOffset.i.i.0183, %.lr.ph ], [ %instOffset.i.i.2, %._crit_edge.2 ]
  %instId.i.i.1 = phi i32 [ %instId.i.i.0184, %.lr.ph ], [ %instId.i.i.2, %._crit_edge.2 ]
  %stackPointer.i.i.2 = phi i32 [ %stackPointer.i.i.1185, %.lr.ph ], [ %stackPointer.i.i.3, %._crit_edge.2 ]
  %resultBary.i.i.1.i0 = phi float [ %resultBary.i.i.0187.i0, %.lr.ph ], [ %resultBary.i.i.2.i0, %._crit_edge.2 ]
  %resultBary.i.i.1.i1 = phi float [ %resultBary.i.i.0187.i1, %.lr.ph ], [ %resultBary.i.i.2.i1, %._crit_edge.2 ]
  %resultTriId.i.i.1 = phi i32 [ %resultTriId.i.i.0188, %.lr.ph ], [ %resultTriId.i.i.2, %._crit_edge.2 ]
  %currentRayData.i.i.1.1.upto0 = insertelement <3 x float> undef, float %currentRayData.i.i.1.1.i0, i32 0
  %currentRayData.i.i.1.1.upto1 = insertelement <3 x float> %currentRayData.i.i.1.1.upto0, float %currentRayData.i.i.1.1.i1, i32 1
  %currentRayData.i.i.1.1 = insertelement <3 x float> %currentRayData.i.i.1.1.upto1, float %currentRayData.i.i.1.1.i2, i32 2
  %currentRayData.i.i.0.1.upto0 = insertelement <3 x float> undef, float %currentRayData.i.i.0.1.i0, i32 0
  %currentRayData.i.i.0.1.upto1 = insertelement <3 x float> %currentRayData.i.i.0.1.upto0, float %currentRayData.i.i.0.1.i1, i32 1
  %currentRayData.i.i.0.1 = insertelement <3 x float> %currentRayData.i.i.0.1.upto1, float %currentRayData.i.i.0.1.i2, i32 2
  %v103 = add nsw i32 %stackPointer.i.i.2, -1
  %v104 = shl i32 %v103, 6
  %v105 = add i32 %v104, %v31
  %v106 = getelementptr [2048 x i32], [2048 x i32] addrspace(3)* @"\01?stack@@3PAIA", i32 0, i32 %v105
  %v107 = load i32, i32 addrspace(3)* %v106, align 4, !tbaa !221, !noalias !345
  %v108 = getelementptr inbounds [2 x i32], [2 x i32]* %nodesToProcess.i.i, i32 0, i32 %currentBVHIndex.i.i.1
  %v109 = add i32 %v102, -1
  store i32 %v109, i32* %v108, align 4, !tbaa !221
  %v110 = getelementptr inbounds [0 x %struct.RWByteAddressBuffer], [0 x %struct.RWByteAddressBuffer]* @"\01?DescriptorHeapBufferTable@@3PAURWByteAddressBuffer@@A", i32 0, i32 %currentGpuVA.i.i.1.i1, !dx.nonuniform !240
  %v111 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %v110, align 4, !noalias !348
  %v112 = add i32 %currentGpuVA.i.i.1.i0, 16
  %v113 = shl i32 %v107, 5
  %v114 = add i32 %v112, %v113
  %v115 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %v111)
  %v116 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %v115, i32 %v114, i32 undef)
  %v117 = extractvalue %dx.types.ResRet.i32 %v116, 3
  %v118 = add i32 %v114, 16
  %v119 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %v115, i32 %v118, i32 undef)
  %v120 = extractvalue %dx.types.ResRet.i32 %v119, 3
  %v121 = icmp slt i32 %v117, 0
  br i1 %v121, label %21, label %16

; <label>:16                                      ; preds = %15
  %v369 = and i32 %v117, 16777215
  %RayTCurrent219 = call float @fb_dxop_rayTCurrent(%struct.RuntimeDataStruct* %runtimeData)
  %v370 = shl nuw nsw i32 %v369, 5
  %v371 = add i32 %v112, %v370
  %v372 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %v111)
  %v373 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %v372, i32 %v371, i32 undef)
  %v374 = extractvalue %dx.types.ResRet.i32 %v373, 0
  %v375 = extractvalue %dx.types.ResRet.i32 %v373, 1
  %v376 = extractvalue %dx.types.ResRet.i32 %v373, 2
  %v377 = add i32 %v371, 16
  %v378 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %v372, i32 %v377, i32 undef)
  %v379 = extractvalue %dx.types.ResRet.i32 %v378, 0
  %v380 = extractvalue %dx.types.ResRet.i32 %v378, 1
  %v381 = extractvalue %dx.types.ResRet.i32 %v378, 2
  %v382 = bitcast i32 %v374 to float
  %v383 = bitcast i32 %v375 to float
  %v384 = bitcast i32 %v376 to float
  %v385 = bitcast i32 %v379 to float
  %v386 = bitcast i32 %v380 to float
  %v387 = bitcast i32 %v381 to float
  %v388 = shl i32 %v120, 5
  %v389 = add i32 %v112, %v388
  %v390 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %v372, i32 %v389, i32 undef)
  %v391 = extractvalue %dx.types.ResRet.i32 %v390, 0
  %v392 = extractvalue %dx.types.ResRet.i32 %v390, 1
  %v393 = extractvalue %dx.types.ResRet.i32 %v390, 2
  %v394 = add i32 %v389, 16
  %v395 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %v372, i32 %v394, i32 undef)
  %v396 = extractvalue %dx.types.ResRet.i32 %v395, 0
  %v397 = extractvalue %dx.types.ResRet.i32 %v395, 1
  %v398 = extractvalue %dx.types.ResRet.i32 %v395, 2
  %v399 = bitcast i32 %v391 to float
  %v400 = bitcast i32 %v392 to float
  %v401 = bitcast i32 %v393 to float
  %v402 = bitcast i32 %v396 to float
  %v403 = bitcast i32 %v397 to float
  %v404 = bitcast i32 %v398 to float
  %.i0378 = fmul fast float %v382, %currentRayData.i.i.2.1.i0
  %.i1379 = fmul fast float %v383, %currentRayData.i.i.2.1.i1
  %.i2380 = fmul fast float %v384, %currentRayData.i.i.2.1.i2
  %.i0381 = fsub fast float %.i0378, %currentRayData.i.i.3.1.i0
  %.i1382 = fsub fast float %.i1379, %currentRayData.i.i.3.1.i1
  %.i2383 = fsub fast float %.i2380, %currentRayData.i.i.3.1.i2
  %FAbs232 = call float @dx.op.unary.f32(i32 6, float %currentRayData.i.i.2.1.i0)
  %FAbs233 = call float @dx.op.unary.f32(i32 6, float %currentRayData.i.i.2.1.i1)
  %FAbs234 = call float @dx.op.unary.f32(i32 6, float %currentRayData.i.i.2.1.i2)
  %.i0384 = fmul fast float %FAbs232, %v385
  %.i1385 = fmul fast float %FAbs233, %v386
  %.i2386 = fmul fast float %FAbs234, %v387
  %.i0387 = fadd fast float %.i0384, %.i0381
  %.i1388 = fadd fast float %.i1385, %.i1382
  %.i2389 = fadd fast float %.i2386, %.i2383
  %.i0393 = fsub fast float %.i0381, %.i0384
  %.i1394 = fsub fast float %.i1382, %.i1385
  %.i2395 = fsub fast float %.i2383, %.i2386
  %FMax271 = call float @dx.op.binary.f32(i32 35, float %.i0393, float %.i1394)
  %FMax270 = call float @dx.op.binary.f32(i32 35, float %FMax271, float %.i2395)
  %FMin269 = call float @dx.op.binary.f32(i32 36, float %.i0387, float %.i1388)
  %FMin268 = call float @dx.op.binary.f32(i32 36, float %FMin269, float %.i2389)
  %FMax266 = call float @dx.op.binary.f32(i32 35, float %FMax270, float 0.000000e+00)
  %FMin265 = call float @dx.op.binary.f32(i32 36, float %FMin268, float %RayTCurrent219)
  %v405 = fcmp fast olt float %FMax266, %FMin265
  %.i0396 = fmul fast float %v399, %currentRayData.i.i.2.1.i0
  %.i1397 = fmul fast float %v400, %currentRayData.i.i.2.1.i1
  %.i2398 = fmul fast float %v401, %currentRayData.i.i.2.1.i2
  %.i0399 = fsub fast float %.i0396, %currentRayData.i.i.3.1.i0
  %.i1400 = fsub fast float %.i1397, %currentRayData.i.i.3.1.i1
  %.i2401 = fsub fast float %.i2398, %currentRayData.i.i.3.1.i2
  %.i0402 = fmul fast float %FAbs232, %v402
  %.i1403 = fmul fast float %FAbs233, %v403
  %.i2404 = fmul fast float %FAbs234, %v404
  %.i0405 = fadd fast float %.i0402, %.i0399
  %.i1406 = fadd fast float %.i1403, %.i1400
  %.i2407 = fadd fast float %.i2404, %.i2401
  %.i0411 = fsub fast float %.i0399, %.i0402
  %.i1412 = fsub fast float %.i1400, %.i1403
  %.i2413 = fsub fast float %.i2401, %.i2404
  %FMax264 = call float @dx.op.binary.f32(i32 35, float %.i0411, float %.i1412)
  %FMax263 = call float @dx.op.binary.f32(i32 35, float %FMax264, float %.i2413)
  %FMin262 = call float @dx.op.binary.f32(i32 36, float %.i0405, float %.i1406)
  %FMin261 = call float @dx.op.binary.f32(i32 36, float %FMin262, float %.i2407)
  %FMax = call float @dx.op.binary.f32(i32 35, float %FMax263, float 0.000000e+00)
  %FMin259 = call float @dx.op.binary.f32(i32 36, float %FMin261, float %RayTCurrent219)
  %v406 = fcmp fast olt float %FMax, %FMin259
  %v407 = and i1 %v405, %v406
  br i1 %v407, label %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i", label %17

; <label>:17                                      ; preds = %16
  %v416 = or i1 %v405, %v406
  br i1 %v416, label %20, label %._crit_edge.1

._crit_edge.1:                                    ; preds = %17
  br label %18

; <label>:18                                      ; preds = %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189._crit_edge", %._crit_edge.11, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i._crit_edge", %._crit_edge.1, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i", %37, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i", %20
  %.3.i0 = phi float [ %.1.i0, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %.1.i0, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189._crit_edge" ], [ %.2.i0, %37 ], [ %.2.i0, %._crit_edge.11 ], [ %.2.i0, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i._crit_edge" ], [ %.1.i0, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %.1.i0, %20 ], [ %.1.i0, %._crit_edge.1 ]
  %.3.i1 = phi float [ %.1.i1, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %.1.i1, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189._crit_edge" ], [ %.2.i1, %37 ], [ %.2.i1, %._crit_edge.11 ], [ %.2.i1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i._crit_edge" ], [ %.1.i1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %.1.i1, %20 ], [ %.1.i1, %._crit_edge.1 ]
  %currentRayData.i.i.0.2.i0 = phi float [ %FMad292, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %currentRayData.i.i.0.1.i0, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189._crit_edge" ], [ %currentRayData.i.i.0.1.i0, %37 ], [ %currentRayData.i.i.0.1.i0, %._crit_edge.11 ], [ %currentRayData.i.i.0.1.i0, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i._crit_edge" ], [ %currentRayData.i.i.0.1.i0, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %currentRayData.i.i.0.1.i0, %20 ], [ %currentRayData.i.i.0.1.i0, %._crit_edge.1 ]
  %currentRayData.i.i.0.2.i1 = phi float [ %FMad289, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %currentRayData.i.i.0.1.i1, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189._crit_edge" ], [ %currentRayData.i.i.0.1.i1, %37 ], [ %currentRayData.i.i.0.1.i1, %._crit_edge.11 ], [ %currentRayData.i.i.0.1.i1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i._crit_edge" ], [ %currentRayData.i.i.0.1.i1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %currentRayData.i.i.0.1.i1, %20 ], [ %currentRayData.i.i.0.1.i1, %._crit_edge.1 ]
  %currentRayData.i.i.0.2.i2 = phi float [ %FMad286, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %currentRayData.i.i.0.1.i2, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189._crit_edge" ], [ %currentRayData.i.i.0.1.i2, %37 ], [ %currentRayData.i.i.0.1.i2, %._crit_edge.11 ], [ %currentRayData.i.i.0.1.i2, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i._crit_edge" ], [ %currentRayData.i.i.0.1.i2, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %currentRayData.i.i.0.1.i2, %20 ], [ %currentRayData.i.i.0.1.i2, %._crit_edge.1 ]
  %currentRayData.i.i.1.2.i0 = phi float [ %FMad283, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %currentRayData.i.i.1.1.i0, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189._crit_edge" ], [ %currentRayData.i.i.1.1.i0, %37 ], [ %currentRayData.i.i.1.1.i0, %._crit_edge.11 ], [ %currentRayData.i.i.1.1.i0, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i._crit_edge" ], [ %currentRayData.i.i.1.1.i0, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %currentRayData.i.i.1.1.i0, %20 ], [ %currentRayData.i.i.1.1.i0, %._crit_edge.1 ]
  %currentRayData.i.i.1.2.i1 = phi float [ %FMad280, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %currentRayData.i.i.1.1.i1, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189._crit_edge" ], [ %currentRayData.i.i.1.1.i1, %37 ], [ %currentRayData.i.i.1.1.i1, %._crit_edge.11 ], [ %currentRayData.i.i.1.1.i1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i._crit_edge" ], [ %currentRayData.i.i.1.1.i1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %currentRayData.i.i.1.1.i1, %20 ], [ %currentRayData.i.i.1.1.i1, %._crit_edge.1 ]
  %currentRayData.i.i.1.2.i2 = phi float [ %FMad, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %currentRayData.i.i.1.1.i2, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189._crit_edge" ], [ %currentRayData.i.i.1.1.i2, %37 ], [ %currentRayData.i.i.1.1.i2, %._crit_edge.11 ], [ %currentRayData.i.i.1.1.i2, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i._crit_edge" ], [ %currentRayData.i.i.1.1.i2, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %currentRayData.i.i.1.1.i2, %20 ], [ %currentRayData.i.i.1.1.i2, %._crit_edge.1 ]
  %currentRayData.i.i.2.2.i0 = phi float [ %.i0340, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %currentRayData.i.i.2.1.i0, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189._crit_edge" ], [ %currentRayData.i.i.2.1.i0, %37 ], [ %currentRayData.i.i.2.1.i0, %._crit_edge.11 ], [ %currentRayData.i.i.2.1.i0, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i._crit_edge" ], [ %currentRayData.i.i.2.1.i0, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %currentRayData.i.i.2.1.i0, %20 ], [ %currentRayData.i.i.2.1.i0, %._crit_edge.1 ]
  %currentRayData.i.i.2.2.i1 = phi float [ %.i1341, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %currentRayData.i.i.2.1.i1, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189._crit_edge" ], [ %currentRayData.i.i.2.1.i1, %37 ], [ %currentRayData.i.i.2.1.i1, %._crit_edge.11 ], [ %currentRayData.i.i.2.1.i1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i._crit_edge" ], [ %currentRayData.i.i.2.1.i1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %currentRayData.i.i.2.1.i1, %20 ], [ %currentRayData.i.i.2.1.i1, %._crit_edge.1 ]
  %currentRayData.i.i.2.2.i2 = phi float [ %.i2342, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %currentRayData.i.i.2.1.i2, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189._crit_edge" ], [ %currentRayData.i.i.2.1.i2, %37 ], [ %currentRayData.i.i.2.1.i2, %._crit_edge.11 ], [ %currentRayData.i.i.2.1.i2, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i._crit_edge" ], [ %currentRayData.i.i.2.1.i2, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %currentRayData.i.i.2.1.i2, %20 ], [ %currentRayData.i.i.2.1.i2, %._crit_edge.1 ]
  %currentRayData.i.i.3.2.i0 = phi float [ %.i0343, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %currentRayData.i.i.3.1.i0, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189._crit_edge" ], [ %currentRayData.i.i.3.1.i0, %37 ], [ %currentRayData.i.i.3.1.i0, %._crit_edge.11 ], [ %currentRayData.i.i.3.1.i0, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i._crit_edge" ], [ %currentRayData.i.i.3.1.i0, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %currentRayData.i.i.3.1.i0, %20 ], [ %currentRayData.i.i.3.1.i0, %._crit_edge.1 ]
  %currentRayData.i.i.3.2.i1 = phi float [ %.i1344, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %currentRayData.i.i.3.1.i1, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189._crit_edge" ], [ %currentRayData.i.i.3.1.i1, %37 ], [ %currentRayData.i.i.3.1.i1, %._crit_edge.11 ], [ %currentRayData.i.i.3.1.i1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i._crit_edge" ], [ %currentRayData.i.i.3.1.i1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %currentRayData.i.i.3.1.i1, %20 ], [ %currentRayData.i.i.3.1.i1, %._crit_edge.1 ]
  %currentRayData.i.i.3.2.i2 = phi float [ %.i2345, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %currentRayData.i.i.3.1.i2, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189._crit_edge" ], [ %currentRayData.i.i.3.1.i2, %37 ], [ %currentRayData.i.i.3.1.i2, %._crit_edge.11 ], [ %currentRayData.i.i.3.1.i2, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i._crit_edge" ], [ %currentRayData.i.i.3.1.i2, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %currentRayData.i.i.3.1.i2, %20 ], [ %currentRayData.i.i.3.1.i2, %._crit_edge.1 ]
  %currentRayData.i.i.4.2.i0 = phi float [ %v217, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %currentRayData.i.i.4.1.i0, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189._crit_edge" ], [ %currentRayData.i.i.4.1.i0, %37 ], [ %currentRayData.i.i.4.1.i0, %._crit_edge.11 ], [ %currentRayData.i.i.4.1.i0, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i._crit_edge" ], [ %currentRayData.i.i.4.1.i0, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %currentRayData.i.i.4.1.i0, %20 ], [ %currentRayData.i.i.4.1.i0, %._crit_edge.1 ]
  %currentRayData.i.i.4.2.i1 = phi float [ %v220, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %currentRayData.i.i.4.1.i1, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189._crit_edge" ], [ %currentRayData.i.i.4.1.i1, %37 ], [ %currentRayData.i.i.4.1.i1, %._crit_edge.11 ], [ %currentRayData.i.i.4.1.i1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i._crit_edge" ], [ %currentRayData.i.i.4.1.i1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %currentRayData.i.i.4.1.i1, %20 ], [ %currentRayData.i.i.4.1.i1, %._crit_edge.1 ]
  %currentRayData.i.i.4.2.i2 = phi float [ %v221, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %currentRayData.i.i.4.1.i2, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189._crit_edge" ], [ %currentRayData.i.i.4.1.i2, %37 ], [ %currentRayData.i.i.4.1.i2, %._crit_edge.11 ], [ %currentRayData.i.i.4.1.i2, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i._crit_edge" ], [ %currentRayData.i.i.4.1.i2, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %currentRayData.i.i.4.1.i2, %20 ], [ %currentRayData.i.i.4.1.i2, %._crit_edge.1 ]
  %currentRayData.i.i.5.2.i0 = phi i32 [ %.5.0.i0, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %currentRayData.i.i.5.1.i0, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189._crit_edge" ], [ %currentRayData.i.i.5.1.i0, %37 ], [ %currentRayData.i.i.5.1.i0, %._crit_edge.11 ], [ %currentRayData.i.i.5.1.i0, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i._crit_edge" ], [ %currentRayData.i.i.5.1.i0, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %currentRayData.i.i.5.1.i0, %20 ], [ %currentRayData.i.i.5.1.i0, %._crit_edge.1 ]
  %currentRayData.i.i.5.2.i1 = phi i32 [ %.5.0.i1, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %currentRayData.i.i.5.1.i1, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189._crit_edge" ], [ %currentRayData.i.i.5.1.i1, %37 ], [ %currentRayData.i.i.5.1.i1, %._crit_edge.11 ], [ %currentRayData.i.i.5.1.i1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i._crit_edge" ], [ %currentRayData.i.i.5.1.i1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %currentRayData.i.i.5.1.i1, %20 ], [ %currentRayData.i.i.5.1.i1, %._crit_edge.1 ]
  %currentRayData.i.i.5.2.i2 = phi i32 [ %.0172, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %currentRayData.i.i.5.1.i2, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189._crit_edge" ], [ %currentRayData.i.i.5.1.i2, %37 ], [ %currentRayData.i.i.5.1.i2, %._crit_edge.11 ], [ %currentRayData.i.i.5.1.i2, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i._crit_edge" ], [ %currentRayData.i.i.5.1.i2, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %currentRayData.i.i.5.1.i2, %20 ], [ %currentRayData.i.i.5.1.i2, %._crit_edge.1 ]
  %CurrentObjectToWorld.i.i167.2.i0 = phi float [ %v185, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i0, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189._crit_edge" ], [ %CurrentObjectToWorld.i.i167.1.i0, %37 ], [ %CurrentObjectToWorld.i.i167.1.i0, %._crit_edge.11 ], [ %CurrentObjectToWorld.i.i167.1.i0, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i._crit_edge" ], [ %CurrentObjectToWorld.i.i167.1.i0, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i0, %20 ], [ %CurrentObjectToWorld.i.i167.1.i0, %._crit_edge.1 ]
  %CurrentObjectToWorld.i.i167.2.i1 = phi float [ %v186, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i1, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189._crit_edge" ], [ %CurrentObjectToWorld.i.i167.1.i1, %37 ], [ %CurrentObjectToWorld.i.i167.1.i1, %._crit_edge.11 ], [ %CurrentObjectToWorld.i.i167.1.i1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i._crit_edge" ], [ %CurrentObjectToWorld.i.i167.1.i1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i1, %20 ], [ %CurrentObjectToWorld.i.i167.1.i1, %._crit_edge.1 ]
  %CurrentObjectToWorld.i.i167.2.i2 = phi float [ %v187, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i2, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189._crit_edge" ], [ %CurrentObjectToWorld.i.i167.1.i2, %37 ], [ %CurrentObjectToWorld.i.i167.1.i2, %._crit_edge.11 ], [ %CurrentObjectToWorld.i.i167.1.i2, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i._crit_edge" ], [ %CurrentObjectToWorld.i.i167.1.i2, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i2, %20 ], [ %CurrentObjectToWorld.i.i167.1.i2, %._crit_edge.1 ]
  %CurrentObjectToWorld.i.i167.2.i3 = phi float [ %v188, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i3, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189._crit_edge" ], [ %CurrentObjectToWorld.i.i167.1.i3, %37 ], [ %CurrentObjectToWorld.i.i167.1.i3, %._crit_edge.11 ], [ %CurrentObjectToWorld.i.i167.1.i3, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i._crit_edge" ], [ %CurrentObjectToWorld.i.i167.1.i3, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i3, %20 ], [ %CurrentObjectToWorld.i.i167.1.i3, %._crit_edge.1 ]
  %CurrentObjectToWorld.i.i167.2.i4 = phi float [ %v189, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i4, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189._crit_edge" ], [ %CurrentObjectToWorld.i.i167.1.i4, %37 ], [ %CurrentObjectToWorld.i.i167.1.i4, %._crit_edge.11 ], [ %CurrentObjectToWorld.i.i167.1.i4, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i._crit_edge" ], [ %CurrentObjectToWorld.i.i167.1.i4, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i4, %20 ], [ %CurrentObjectToWorld.i.i167.1.i4, %._crit_edge.1 ]
  %CurrentObjectToWorld.i.i167.2.i5 = phi float [ %v190, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i5, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189._crit_edge" ], [ %CurrentObjectToWorld.i.i167.1.i5, %37 ], [ %CurrentObjectToWorld.i.i167.1.i5, %._crit_edge.11 ], [ %CurrentObjectToWorld.i.i167.1.i5, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i._crit_edge" ], [ %CurrentObjectToWorld.i.i167.1.i5, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i5, %20 ], [ %CurrentObjectToWorld.i.i167.1.i5, %._crit_edge.1 ]
  %CurrentObjectToWorld.i.i167.2.i6 = phi float [ %v191, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i6, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189._crit_edge" ], [ %CurrentObjectToWorld.i.i167.1.i6, %37 ], [ %CurrentObjectToWorld.i.i167.1.i6, %._crit_edge.11 ], [ %CurrentObjectToWorld.i.i167.1.i6, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i._crit_edge" ], [ %CurrentObjectToWorld.i.i167.1.i6, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i6, %20 ], [ %CurrentObjectToWorld.i.i167.1.i6, %._crit_edge.1 ]
  %CurrentObjectToWorld.i.i167.2.i7 = phi float [ %v192, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i7, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189._crit_edge" ], [ %CurrentObjectToWorld.i.i167.1.i7, %37 ], [ %CurrentObjectToWorld.i.i167.1.i7, %._crit_edge.11 ], [ %CurrentObjectToWorld.i.i167.1.i7, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i._crit_edge" ], [ %CurrentObjectToWorld.i.i167.1.i7, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i7, %20 ], [ %CurrentObjectToWorld.i.i167.1.i7, %._crit_edge.1 ]
  %CurrentObjectToWorld.i.i167.2.i8 = phi float [ %v193, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i8, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189._crit_edge" ], [ %CurrentObjectToWorld.i.i167.1.i8, %37 ], [ %CurrentObjectToWorld.i.i167.1.i8, %._crit_edge.11 ], [ %CurrentObjectToWorld.i.i167.1.i8, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i._crit_edge" ], [ %CurrentObjectToWorld.i.i167.1.i8, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i8, %20 ], [ %CurrentObjectToWorld.i.i167.1.i8, %._crit_edge.1 ]
  %CurrentObjectToWorld.i.i167.2.i9 = phi float [ %v194, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i9, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189._crit_edge" ], [ %CurrentObjectToWorld.i.i167.1.i9, %37 ], [ %CurrentObjectToWorld.i.i167.1.i9, %._crit_edge.11 ], [ %CurrentObjectToWorld.i.i167.1.i9, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i._crit_edge" ], [ %CurrentObjectToWorld.i.i167.1.i9, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i9, %20 ], [ %CurrentObjectToWorld.i.i167.1.i9, %._crit_edge.1 ]
  %CurrentObjectToWorld.i.i167.2.i10 = phi float [ %v195, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i10, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189._crit_edge" ], [ %CurrentObjectToWorld.i.i167.1.i10, %37 ], [ %CurrentObjectToWorld.i.i167.1.i10, %._crit_edge.11 ], [ %CurrentObjectToWorld.i.i167.1.i10, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i._crit_edge" ], [ %CurrentObjectToWorld.i.i167.1.i10, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i10, %20 ], [ %CurrentObjectToWorld.i.i167.1.i10, %._crit_edge.1 ]
  %CurrentObjectToWorld.i.i167.2.i11 = phi float [ %v196, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i11, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189._crit_edge" ], [ %CurrentObjectToWorld.i.i167.1.i11, %37 ], [ %CurrentObjectToWorld.i.i167.1.i11, %._crit_edge.11 ], [ %CurrentObjectToWorld.i.i167.1.i11, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i._crit_edge" ], [ %CurrentObjectToWorld.i.i167.1.i11, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i11, %20 ], [ %CurrentObjectToWorld.i.i167.1.i11, %._crit_edge.1 ]
  %CurrentWorldToObject.i.i168.2.i0 = phi float [ %v168, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %CurrentWorldToObject.i.i168.1.i0, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189._crit_edge" ], [ %CurrentWorldToObject.i.i168.1.i0, %37 ], [ %CurrentWorldToObject.i.i168.1.i0, %._crit_edge.11 ], [ %CurrentWorldToObject.i.i168.1.i0, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i._crit_edge" ], [ %CurrentWorldToObject.i.i168.1.i0, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %CurrentWorldToObject.i.i168.1.i0, %20 ], [ %CurrentWorldToObject.i.i168.1.i0, %._crit_edge.1 ]
  %CurrentWorldToObject.i.i168.2.i1 = phi float [ %v169, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %CurrentWorldToObject.i.i168.1.i1, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189._crit_edge" ], [ %CurrentWorldToObject.i.i168.1.i1, %37 ], [ %CurrentWorldToObject.i.i168.1.i1, %._crit_edge.11 ], [ %CurrentWorldToObject.i.i168.1.i1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i._crit_edge" ], [ %CurrentWorldToObject.i.i168.1.i1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %CurrentWorldToObject.i.i168.1.i1, %20 ], [ %CurrentWorldToObject.i.i168.1.i1, %._crit_edge.1 ]
  %CurrentWorldToObject.i.i168.2.i2 = phi float [ %v170, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %CurrentWorldToObject.i.i168.1.i2, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189._crit_edge" ], [ %CurrentWorldToObject.i.i168.1.i2, %37 ], [ %CurrentWorldToObject.i.i168.1.i2, %._crit_edge.11 ], [ %CurrentWorldToObject.i.i168.1.i2, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i._crit_edge" ], [ %CurrentWorldToObject.i.i168.1.i2, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %CurrentWorldToObject.i.i168.1.i2, %20 ], [ %CurrentWorldToObject.i.i168.1.i2, %._crit_edge.1 ]
  %CurrentWorldToObject.i.i168.2.i3 = phi float [ %v171, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %CurrentWorldToObject.i.i168.1.i3, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189._crit_edge" ], [ %CurrentWorldToObject.i.i168.1.i3, %37 ], [ %CurrentWorldToObject.i.i168.1.i3, %._crit_edge.11 ], [ %CurrentWorldToObject.i.i168.1.i3, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i._crit_edge" ], [ %CurrentWorldToObject.i.i168.1.i3, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %CurrentWorldToObject.i.i168.1.i3, %20 ], [ %CurrentWorldToObject.i.i168.1.i3, %._crit_edge.1 ]
  %CurrentWorldToObject.i.i168.2.i4 = phi float [ %v172, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %CurrentWorldToObject.i.i168.1.i4, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189._crit_edge" ], [ %CurrentWorldToObject.i.i168.1.i4, %37 ], [ %CurrentWorldToObject.i.i168.1.i4, %._crit_edge.11 ], [ %CurrentWorldToObject.i.i168.1.i4, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i._crit_edge" ], [ %CurrentWorldToObject.i.i168.1.i4, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %CurrentWorldToObject.i.i168.1.i4, %20 ], [ %CurrentWorldToObject.i.i168.1.i4, %._crit_edge.1 ]
  %CurrentWorldToObject.i.i168.2.i5 = phi float [ %v173, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %CurrentWorldToObject.i.i168.1.i5, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189._crit_edge" ], [ %CurrentWorldToObject.i.i168.1.i5, %37 ], [ %CurrentWorldToObject.i.i168.1.i5, %._crit_edge.11 ], [ %CurrentWorldToObject.i.i168.1.i5, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i._crit_edge" ], [ %CurrentWorldToObject.i.i168.1.i5, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %CurrentWorldToObject.i.i168.1.i5, %20 ], [ %CurrentWorldToObject.i.i168.1.i5, %._crit_edge.1 ]
  %CurrentWorldToObject.i.i168.2.i6 = phi float [ %v174, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %CurrentWorldToObject.i.i168.1.i6, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189._crit_edge" ], [ %CurrentWorldToObject.i.i168.1.i6, %37 ], [ %CurrentWorldToObject.i.i168.1.i6, %._crit_edge.11 ], [ %CurrentWorldToObject.i.i168.1.i6, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i._crit_edge" ], [ %CurrentWorldToObject.i.i168.1.i6, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %CurrentWorldToObject.i.i168.1.i6, %20 ], [ %CurrentWorldToObject.i.i168.1.i6, %._crit_edge.1 ]
  %CurrentWorldToObject.i.i168.2.i7 = phi float [ %v175, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %CurrentWorldToObject.i.i168.1.i7, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189._crit_edge" ], [ %CurrentWorldToObject.i.i168.1.i7, %37 ], [ %CurrentWorldToObject.i.i168.1.i7, %._crit_edge.11 ], [ %CurrentWorldToObject.i.i168.1.i7, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i._crit_edge" ], [ %CurrentWorldToObject.i.i168.1.i7, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %CurrentWorldToObject.i.i168.1.i7, %20 ], [ %CurrentWorldToObject.i.i168.1.i7, %._crit_edge.1 ]
  %CurrentWorldToObject.i.i168.2.i8 = phi float [ %v176, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %CurrentWorldToObject.i.i168.1.i8, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189._crit_edge" ], [ %CurrentWorldToObject.i.i168.1.i8, %37 ], [ %CurrentWorldToObject.i.i168.1.i8, %._crit_edge.11 ], [ %CurrentWorldToObject.i.i168.1.i8, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i._crit_edge" ], [ %CurrentWorldToObject.i.i168.1.i8, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %CurrentWorldToObject.i.i168.1.i8, %20 ], [ %CurrentWorldToObject.i.i168.1.i8, %._crit_edge.1 ]
  %CurrentWorldToObject.i.i168.2.i9 = phi float [ %v177, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %CurrentWorldToObject.i.i168.1.i9, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189._crit_edge" ], [ %CurrentWorldToObject.i.i168.1.i9, %37 ], [ %CurrentWorldToObject.i.i168.1.i9, %._crit_edge.11 ], [ %CurrentWorldToObject.i.i168.1.i9, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i._crit_edge" ], [ %CurrentWorldToObject.i.i168.1.i9, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %CurrentWorldToObject.i.i168.1.i9, %20 ], [ %CurrentWorldToObject.i.i168.1.i9, %._crit_edge.1 ]
  %CurrentWorldToObject.i.i168.2.i10 = phi float [ %v178, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %CurrentWorldToObject.i.i168.1.i10, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189._crit_edge" ], [ %CurrentWorldToObject.i.i168.1.i10, %37 ], [ %CurrentWorldToObject.i.i168.1.i10, %._crit_edge.11 ], [ %CurrentWorldToObject.i.i168.1.i10, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i._crit_edge" ], [ %CurrentWorldToObject.i.i168.1.i10, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %CurrentWorldToObject.i.i168.1.i10, %20 ], [ %CurrentWorldToObject.i.i168.1.i10, %._crit_edge.1 ]
  %CurrentWorldToObject.i.i168.2.i11 = phi float [ %v179, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %CurrentWorldToObject.i.i168.1.i11, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189._crit_edge" ], [ %CurrentWorldToObject.i.i168.1.i11, %37 ], [ %CurrentWorldToObject.i.i168.1.i11, %._crit_edge.11 ], [ %CurrentWorldToObject.i.i168.1.i11, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i._crit_edge" ], [ %CurrentWorldToObject.i.i168.1.i11, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %CurrentWorldToObject.i.i168.1.i11, %20 ], [ %CurrentWorldToObject.i.i168.1.i11, %._crit_edge.1 ]
  %ResetMatrices.i.i.3 = phi i32 [ %ResetMatrices.i.i.1, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %ResetMatrices.i.i.1, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189._crit_edge" ], [ 0, %37 ], [ 0, %._crit_edge.11 ], [ %ResetMatrices.i.i.1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i._crit_edge" ], [ %ResetMatrices.i.i.1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %ResetMatrices.i.i.1, %20 ], [ %ResetMatrices.i.i.1, %._crit_edge.1 ]
  %currentBVHIndex.i.i.2 = phi i32 [ 1, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ 0, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189._crit_edge" ], [ %currentBVHIndex.i.i.1, %37 ], [ %currentBVHIndex.i.i.1, %._crit_edge.11 ], [ %currentBVHIndex.i.i.1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i._crit_edge" ], [ %currentBVHIndex.i.i.1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %currentBVHIndex.i.i.1, %20 ], [ %currentBVHIndex.i.i.1, %._crit_edge.1 ]
  %currentGpuVA.i.i.2.i0 = phi i32 [ %v148, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %currentGpuVA.i.i.1.i0, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189._crit_edge" ], [ %currentGpuVA.i.i.1.i0, %37 ], [ %currentGpuVA.i.i.1.i0, %._crit_edge.11 ], [ %currentGpuVA.i.i.1.i0, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i._crit_edge" ], [ %currentGpuVA.i.i.1.i0, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %currentGpuVA.i.i.1.i0, %20 ], [ %currentGpuVA.i.i.1.i0, %._crit_edge.1 ]
  %currentGpuVA.i.i.2.i1 = phi i32 [ %v149, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %currentGpuVA.i.i.1.i1, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189._crit_edge" ], [ %currentGpuVA.i.i.1.i1, %37 ], [ %currentGpuVA.i.i.1.i1, %._crit_edge.11 ], [ %currentGpuVA.i.i.1.i1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i._crit_edge" ], [ %currentGpuVA.i.i.1.i1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %currentGpuVA.i.i.1.i1, %20 ], [ %currentGpuVA.i.i.1.i1, %._crit_edge.1 ]
  %instIdx.i.i.2 = phi i32 [ %v123, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %v123, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189._crit_edge" ], [ %instIdx.i.i.1, %37 ], [ %instIdx.i.i.1, %._crit_edge.11 ], [ %instIdx.i.i.1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i._crit_edge" ], [ %instIdx.i.i.1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %instIdx.i.i.1, %20 ], [ %instIdx.i.i.1, %._crit_edge.1 ]
  %instFlags.i.i.2 = phi i32 [ %v197, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %instFlags.i.i.1, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189._crit_edge" ], [ %instFlags.i.i.1, %37 ], [ %instFlags.i.i.1, %._crit_edge.11 ], [ %instFlags.i.i.1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i._crit_edge" ], [ %instFlags.i.i.1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %instFlags.i.i.1, %20 ], [ %instFlags.i.i.1, %._crit_edge.1 ]
  %instOffset.i.i.2 = phi i32 [ %v180, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %v180, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189._crit_edge" ], [ %instOffset.i.i.1, %37 ], [ %instOffset.i.i.1, %._crit_edge.11 ], [ %instOffset.i.i.1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i._crit_edge" ], [ %instOffset.i.i.1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %instOffset.i.i.1, %20 ], [ %instOffset.i.i.1, %._crit_edge.1 ]
  %instId.i.i.2 = phi i32 [ %v181, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %v181, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189._crit_edge" ], [ %instId.i.i.1, %37 ], [ %instId.i.i.1, %._crit_edge.11 ], [ %instId.i.i.1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i._crit_edge" ], [ %instId.i.i.1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %instId.i.i.1, %20 ], [ %instId.i.i.1, %._crit_edge.1 ]
  %stackPointer.i.i.3 = phi i32 [ %stackPointer.i.i.2, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %v103, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189._crit_edge" ], [ %v103, %37 ], [ %v103, %._crit_edge.11 ], [ %v103, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i._crit_edge" ], [ %v414, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %stackPointer.i.i.2, %20 ], [ %v103, %._crit_edge.1 ]
  %resultBary.i.i.2.i0 = phi float [ %resultBary.i.i.1.i0, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %resultBary.i.i.1.i0, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189._crit_edge" ], [ %.2.i0.resultBary.i.i.1.i0, %37 ], [ %.2.i0.resultBary.i.i.1.i0, %._crit_edge.11 ], [ %.2.i0.resultBary.i.i.1.i0, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i._crit_edge" ], [ %resultBary.i.i.1.i0, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %resultBary.i.i.1.i0, %20 ], [ %resultBary.i.i.1.i0, %._crit_edge.1 ]
  %resultBary.i.i.2.i1 = phi float [ %resultBary.i.i.1.i1, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %resultBary.i.i.1.i1, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189._crit_edge" ], [ %.2.i1.resultBary.i.i.1.i1, %37 ], [ %.2.i1.resultBary.i.i.1.i1, %._crit_edge.11 ], [ %.2.i1.resultBary.i.i.1.i1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i._crit_edge" ], [ %resultBary.i.i.1.i1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %resultBary.i.i.1.i1, %20 ], [ %resultBary.i.i.1.i1, %._crit_edge.1 ]
  %resultTriId.i.i.2 = phi i32 [ %resultTriId.i.i.1, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %resultTriId.i.i.1, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189._crit_edge" ], [ %.resultTriId.i.i.1, %37 ], [ %.resultTriId.i.i.1, %._crit_edge.11 ], [ %.resultTriId.i.i.1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i._crit_edge" ], [ %resultTriId.i.i.1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %resultTriId.i.i.1, %20 ], [ %resultTriId.i.i.1, %._crit_edge.1 ]
  %v418 = getelementptr inbounds [2 x i32], [2 x i32]* %nodesToProcess.i.i, i32 0, i32 %currentBVHIndex.i.i.2
  %v419 = load i32, i32* %v418, align 4, !tbaa !221
  %v420 = icmp eq i32 %v419, 0
  br i1 %v420, label %19, label %._crit_edge.2

._crit_edge.2:                                    ; preds = %18
  br label %15

; <label>:19                                      ; preds = %18
  %v421 = add i32 %currentBVHIndex.i.i.2, -1
  %v422 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %AccelerationStructureList, i32 0)
  %v423 = extractvalue %dx.types.CBufRet.i32 %v422, 0
  %v424 = extractvalue %dx.types.CBufRet.i32 %v422, 1
  %v425 = load i32, i32* %v56, align 4, !tbaa !221
  %v426 = icmp eq i32 %v425, 0
  br i1 %v426, label %.._crit_edge_crit_edge, label %..lr.ph_crit_edge

..lr.ph_crit_edge:                                ; preds = %19
  br label %.lr.ph

.._crit_edge_crit_edge:                           ; preds = %19
  br label %._crit_edge

; <label>:20                                      ; preds = %17
  %v417 = select i1 %v406, i32 %v120, i32 %v369
  store i32 %v417, i32 addrspace(3)* %v106, align 4, !tbaa !221, !noalias !353
  store i32 %v102, i32* %v108, align 4, !tbaa !221
  br label %18

"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i":       ; preds = %16
  %v408 = fcmp fast olt float %FMax, %FMax266
  %v409 = select i1 %v408, i32 %v369, i32 %v120
  %v410 = select i1 %v408, i32 %v120, i32 %v369
  %v411 = shl i32 %stackPointer.i.i.2, 6
  %v412 = add i32 %v411, %v31
  store i32 %v409, i32 addrspace(3)* %v106, align 4, !tbaa !221, !noalias !356
  %v413 = getelementptr [2048 x i32], [2048 x i32] addrspace(3)* @"\01?stack@@3PAIA", i32 0, i32 %v412
  store i32 %v410, i32 addrspace(3)* %v413, align 4, !tbaa !221, !noalias !356
  %v414 = add nsw i32 %stackPointer.i.i.2, 1
  %v415 = add i32 %v102, 1
  store i32 %v415, i32* %v108, align 4, !tbaa !221
  br label %18

; <label>:21                                      ; preds = %15
  %v122 = icmp eq i32 %currentBVHIndex.i.i.1, 0
  br i1 %v122, label %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189", label %"\01?IsOpaque@@YA_N_NII@Z.exit.i.i"

"\01?IsOpaque@@YA_N_NII@Z.exit.i.i":              ; preds = %21
  %RayFlags256 = call i32 @fb_dxop_rayFlags(%struct.RuntimeDataStruct* %runtimeData)
  %v222 = and i32 %RayFlags256, 64
  %RayTCurrent = call float @fb_dxop_rayTCurrent(%struct.RuntimeDataStruct* %runtimeData)
  %v223 = icmp eq i32 %v222, 0
  %v224 = and i32 %v117, 16777215
  %v225 = add i32 %currentGpuVA.i.i.1.i0, 4
  %v226 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %v111)
  %v227 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %v226, i32 %v225, i32 undef)
  %v228 = extractvalue %dx.types.ResRet.i32 %v227, 0
  %v229 = mul nuw nsw i32 %v224, 36
  %v230 = add i32 %v229, %currentGpuVA.i.i.1.i0
  %v231 = add i32 %v230, %v228
  %v232 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %v226, i32 %v231, i32 undef)
  %v233 = extractvalue %dx.types.ResRet.i32 %v232, 0
  %v234 = extractvalue %dx.types.ResRet.i32 %v232, 1
  %v235 = extractvalue %dx.types.ResRet.i32 %v232, 2
  %v236 = extractvalue %dx.types.ResRet.i32 %v232, 3
  %.i0346 = bitcast i32 %v233 to float
  %.i1347 = bitcast i32 %v234 to float
  %.i2348 = bitcast i32 %v235 to float
  %.i3349 = bitcast i32 %v236 to float
  %v237 = add i32 %v231, 16
  %v238 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %v226, i32 %v237, i32 undef)
  %v239 = extractvalue %dx.types.ResRet.i32 %v238, 0
  %v240 = extractvalue %dx.types.ResRet.i32 %v238, 1
  %v241 = extractvalue %dx.types.ResRet.i32 %v238, 2
  %v242 = extractvalue %dx.types.ResRet.i32 %v238, 3
  %.i0350 = bitcast i32 %v239 to float
  %.i1351 = bitcast i32 %v240 to float
  %.i2352 = bitcast i32 %v241 to float
  %.i3353 = bitcast i32 %v242 to float
  %v243 = add i32 %v231, 32
  %v244 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %v226, i32 %v243, i32 undef)
  %v245 = extractvalue %dx.types.ResRet.i32 %v244, 0
  %v246 = bitcast i32 %v245 to float
  %v247 = and i32 %instFlags.i.i.1, 1
  %v248 = icmp eq i32 %v247, 0
  %v249 = shl i32 %instFlags.i.i.1, 3
  %v250 = and i32 %v249, 16
  %v251 = add nuw nsw i32 %v250, 16
  %v252 = xor i32 %v250, 16
  %v253 = add nuw nsw i32 %v252, 16
  %v254 = and i32 %RayFlags256, %v251
  %v255 = icmp ne i32 %v254, 0
  %v256 = and i1 %v248, %v255
  %v257 = and i32 %RayFlags256, %v253
  %v258 = icmp ne i32 %v257, 0
  %v259 = and i1 %v248, %v258
  %.i0354 = fsub fast float %.i0346, %currentRayData.i.i.0.1.i0
  %.i1355 = fsub fast float %.i1347, %currentRayData.i.i.0.1.i1
  %.i2356 = fsub fast float %.i2348, %currentRayData.i.i.0.1.i2
  store float %.i0354, float* %v92, align 4
  store float %.i1355, float* %v93, align 4
  store float %.i2356, float* %v94, align 4
  %v260 = getelementptr [3 x float], [3 x float]* %v21, i32 0, i32 %currentRayData.i.i.5.1.i0
  %v261 = load float, float* %v260, align 4, !tbaa !231, !noalias !359
  %v262 = getelementptr [3 x float], [3 x float]* %v21, i32 0, i32 %currentRayData.i.i.5.1.i1
  %v263 = load float, float* %v262, align 4, !tbaa !231, !noalias !359
  %v264 = getelementptr [3 x float], [3 x float]* %v21, i32 0, i32 %currentRayData.i.i.5.1.i2
  %v265 = load float, float* %v264, align 4, !tbaa !231, !noalias !359
  %.i0357 = fsub fast float %.i3349, %currentRayData.i.i.0.1.i0
  %.i1358 = fsub fast float %.i0350, %currentRayData.i.i.0.1.i1
  %.i2359 = fsub fast float %.i1351, %currentRayData.i.i.0.1.i2
  store float %.i0357, float* %v95, align 4
  store float %.i1358, float* %v96, align 4
  store float %.i2359, float* %v97, align 4
  %v266 = getelementptr [3 x float], [3 x float]* %v19, i32 0, i32 %currentRayData.i.i.5.1.i0
  %v267 = load float, float* %v266, align 4, !tbaa !231, !noalias !359
  %v268 = getelementptr [3 x float], [3 x float]* %v19, i32 0, i32 %currentRayData.i.i.5.1.i1
  %v269 = load float, float* %v268, align 4, !tbaa !231, !noalias !359
  %v270 = getelementptr [3 x float], [3 x float]* %v19, i32 0, i32 %currentRayData.i.i.5.1.i2
  %v271 = load float, float* %v270, align 4, !tbaa !231, !noalias !359
  %.i0360 = fsub fast float %.i2352, %currentRayData.i.i.0.1.i0
  %.i1361 = fsub fast float %.i3353, %currentRayData.i.i.0.1.i1
  %.i2362 = fsub fast float %v246, %currentRayData.i.i.0.1.i2
  store float %.i0360, float* %v98, align 4
  store float %.i1361, float* %v99, align 4
  store float %.i2362, float* %v100, align 4
  %v272 = getelementptr [3 x float], [3 x float]* %v20, i32 0, i32 %currentRayData.i.i.5.1.i0
  %v273 = load float, float* %v272, align 4, !tbaa !231, !noalias !359
  %v274 = getelementptr [3 x float], [3 x float]* %v20, i32 0, i32 %currentRayData.i.i.5.1.i1
  %v275 = load float, float* %v274, align 4, !tbaa !231, !noalias !359
  %v276 = getelementptr [3 x float], [3 x float]* %v20, i32 0, i32 %currentRayData.i.i.5.1.i2
  %v277 = load float, float* %v276, align 4, !tbaa !231, !noalias !359
  %.i0363 = fmul float %currentRayData.i.i.4.1.i0, %v265
  %.i1364 = fmul float %currentRayData.i.i.4.1.i1, %v265
  %.i0366 = fsub float %v261, %.i0363
  %.i1367 = fsub float %v263, %.i1364
  %.i0368 = fmul float %currentRayData.i.i.4.1.i0, %v271
  %.i1369 = fmul float %currentRayData.i.i.4.1.i1, %v271
  %.i0371 = fsub float %v267, %.i0368
  %.i1372 = fsub float %v269, %.i1369
  %.i0373 = fmul float %currentRayData.i.i.4.1.i0, %v277
  %.i1374 = fmul float %currentRayData.i.i.4.1.i1, %v277
  %.i0376 = fsub float %v273, %.i0373
  %.i1377 = fsub float %v275, %.i1374
  %v278 = fmul float %.i1372, %.i0376
  %v279 = fmul float %.i0371, %.i1377
  %v280 = fsub float %v278, %v279
  %v281 = fmul float %.i0366, %.i1377
  %v282 = fmul float %.i1367, %.i0376
  %v283 = fsub float %v281, %v282
  %v284 = fmul float %.i1367, %.i0371
  %v285 = fmul float %.i0366, %.i1372
  %v286 = fsub float %v284, %v285
  %v287 = fadd fast float %v283, %v286
  %v288 = fadd fast float %v287, %v280
  br i1 %v259, label %39, label %22

; <label>:22                                      ; preds = %"\01?IsOpaque@@YA_N_NII@Z.exit.i.i"
  br i1 %v256, label %38, label %23

; <label>:23                                      ; preds = %22
  %v299 = fcmp fast olt float %v280, 0.000000e+00
  %v300 = fcmp fast olt float %v283, 0.000000e+00
  %v301 = or i1 %v299, %v300
  %v302 = fcmp fast olt float %v286, 0.000000e+00
  %v303 = or i1 %v302, %v301
  %v304 = fcmp fast ogt float %v280, 0.000000e+00
  %v305 = fcmp fast ogt float %v283, 0.000000e+00
  %v306 = or i1 %v304, %v305
  %v307 = fcmp fast ogt float %v286, 0.000000e+00
  %v308 = or i1 %v307, %v306
  %v309 = and i1 %v303, %v308
  %v310 = fcmp fast oeq float %v288, 0.000000e+00
  %or.cond193 = or i1 %v310, %v309
  br i1 %or.cond193, label %".\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i_crit_edge", label %._crit_edge.3

._crit_edge.3:                                    ; preds = %23
  br label %24

".\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i_crit_edge": ; preds = %23
  br label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i"

; <label>:24                                      ; preds = %._crit_edge.15, %._crit_edge.13, %._crit_edge.3
  %v311 = fmul fast float %v280, %v265
  %v312 = fmul fast float %v283, %v271
  %v313 = fmul fast float %v286, %v277
  %tmp = fadd fast float %v312, %v313
  %tmp517 = fadd fast float %tmp, %v311
  %tmp518 = fmul fast float %tmp517, %currentRayData.i.i.4.1.i2
  br i1 %v259, label %29, label %25

; <label>:25                                      ; preds = %24
  br i1 %v256, label %28, label %26

; <label>:26                                      ; preds = %25
  %v322 = fcmp fast ogt float %v288, 0.000000e+00
  %v323 = select i1 %v322, i32 1, i32 -1
  %v324 = bitcast float %tmp518 to i32
  %v325 = xor i32 %v324, %v323
  %v326 = uitofp i32 %v325 to float
  %v327 = bitcast float %v288 to i32
  %v328 = xor i32 %v323, %v327
  %v329 = uitofp i32 %v328 to float
  %v330 = fmul fast float %v329, %RayTCurrent
  %v331 = fcmp fast ogt float %v326, %v330
  br i1 %v331, label %".\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i_crit_edge.4", label %._crit_edge.5

._crit_edge.5:                                    ; preds = %26
  br label %27

".\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i_crit_edge.4": ; preds = %26
  br label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i"

; <label>:27                                      ; preds = %._crit_edge.9, %._crit_edge.7, %._crit_edge.5
  %v332 = fdiv fast float 1.000000e+00, %v288
  %v333 = fmul fast float %v332, %v283
  %v334 = fmul fast float %v332, %v286
  %v335 = fmul fast float %v332, %tmp518
  br label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i"

; <label>:28                                      ; preds = %25
  %v318 = fcmp fast olt float %tmp518, 0.000000e+00
  %v319 = fmul fast float %v288, %RayTCurrent
  %v320 = fcmp fast ogt float %tmp518, %v319
  %v321 = or i1 %v318, %v320
  br i1 %v321, label %".\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i_crit_edge.6", label %._crit_edge.7

._crit_edge.7:                                    ; preds = %28
  br label %27

".\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i_crit_edge.6": ; preds = %28
  br label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i"

; <label>:29                                      ; preds = %24
  %v314 = fcmp fast ogt float %tmp518, 0.000000e+00
  %v315 = fmul fast float %v288, %RayTCurrent
  %v316 = fcmp fast olt float %tmp518, %v315
  %v317 = or i1 %v314, %v316
  br i1 %v317, label %".\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i_crit_edge.8", label %._crit_edge.9

._crit_edge.9:                                    ; preds = %29
  br label %27

".\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i_crit_edge.8": ; preds = %29
  br label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i"

"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i": ; preds = %".\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i_crit_edge.14", %".\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i_crit_edge.12", %".\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i_crit_edge.8", %".\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i_crit_edge.6", %".\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i_crit_edge.4", %".\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i_crit_edge", %27
  %.2.i0 = phi float [ %.1.i0, %".\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i_crit_edge.14" ], [ %.1.i0, %".\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i_crit_edge.8" ], [ %v333, %27 ], [ %.1.i0, %".\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i_crit_edge.6" ], [ %.1.i0, %".\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i_crit_edge.4" ], [ %.1.i0, %".\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i_crit_edge.12" ], [ %.1.i0, %".\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i_crit_edge" ]
  %.2.i1 = phi float [ %.1.i1, %".\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i_crit_edge.14" ], [ %.1.i1, %".\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i_crit_edge.8" ], [ %v334, %27 ], [ %.1.i1, %".\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i_crit_edge.6" ], [ %.1.i1, %".\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i_crit_edge.4" ], [ %.1.i1, %".\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i_crit_edge.12" ], [ %.1.i1, %".\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i_crit_edge" ]
  %.0173 = phi float [ %RayTCurrent, %".\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i_crit_edge.14" ], [ %RayTCurrent, %".\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i_crit_edge.8" ], [ %v335, %27 ], [ %RayTCurrent, %".\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i_crit_edge.6" ], [ %RayTCurrent, %".\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i_crit_edge.4" ], [ %RayTCurrent, %".\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i_crit_edge.12" ], [ %RayTCurrent, %".\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i_crit_edge" ]
  %v336 = fcmp fast olt float %.0173, %RayTCurrent
  %RayTMin = call float @fb_dxop_rayTMin(%struct.RuntimeDataStruct* %runtimeData)
  %v337 = fcmp fast ogt float %.0173, %RayTMin
  %v338 = and i1 %v336, %v337
  %. = select i1 %v338, i1 true, i1 false
  %.resultTriId.i.i.1 = select i1 %v338, i32 %v224, i32 %resultTriId.i.i.1
  %.0173.RayTCurrent = select i1 %v338, float %.0173, float %RayTCurrent
  %.2.i0.resultBary.i.i.1.i0 = select i1 %v338, float %.2.i0, float %resultBary.i.i.1.i0
  %.2.i1.resultBary.i.i.1.i1 = select i1 %v338, float %.2.i1, float %resultBary.i.i.1.i1
  %v339 = and i1 %v223, %.
  br i1 %v339, label %30, label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i._crit_edge"

"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i._crit_edge": ; preds = %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i"
  br label %18

; <label>:30                                      ; preds = %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i"
  %.0.upto0 = insertelement <2 x float> undef, float %.2.i0.resultBary.i.i.1.i0, i32 0
  %.0 = insertelement <2 x float> %.0.upto0, float %.2.i1.resultBary.i.i.1.i1, i32 1
  %v340 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %v110, align 4, !noalias !367
  %v341 = add i32 %currentGpuVA.i.i.1.i0, 8
  %v342 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %v340)
  %v343 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %v342, i32 %v341, i32 undef)
  %v344 = extractvalue %dx.types.ResRet.i32 %v343, 0
  %v345 = shl i32 %.resultTriId.i.i.1, 3
  %v346 = add i32 %v345, %currentGpuVA.i.i.1.i0
  %v347 = add i32 %v346, %v344
  %v348 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %v342, i32 %v347, i32 undef)
  %v349 = extractvalue %dx.types.ResRet.i32 %v348, 0
  %v350 = extractvalue %dx.types.ResRet.i32 %v348, 1
  %v351 = mul i32 %v349, %arg3.int
  %v352 = add i32 %instOffset.i.i.1, %arg2.int
  %v353 = add i32 %v352, %v351
  store <2 x float> %.0, <2 x float>* %v101, align 4, !tbaa !218
  %cur.pendingAttr.offset = call i32 @pendingAttrOffset(%struct.RuntimeDataStruct* %runtimeData)
  %vec.ptr = getelementptr inbounds %struct.BuiltInTriangleIntersectionAttributes, %struct.BuiltInTriangleIntersectionAttributes* %attr.i.i, i32 0, i32 0
  %vec = load <2 x float>, <2 x float>* %vec.ptr
  %31 = extractelement <2 x float> %vec, i32 0
  %32 = bitcast float %31 to i32
  %intPtr18 = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %cur.pendingAttr.offset, i32 0)
  store i32 %32, i32* %intPtr18
  %33 = extractelement <2 x float> %vec, i32 1
  %34 = bitcast float %33 to i32
  %intPtr19 = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %cur.pendingAttr.offset, i32 1)
  store i32 %34, i32* %intPtr19
  call void @fb_Fallback_SetPendingTriVals(%struct.RuntimeDataStruct* %runtimeData, float %.0173.RayTCurrent, i32 %v350, i32 %v353, i32 %instIdx.i.i.1, i32 %instId.i.i.1, i32 254)
  %v354 = icmp eq i32 %ResetMatrices.i.i.1, 0
  br i1 %v354, label %._crit_edge.10, label %35

._crit_edge.10:                                   ; preds = %30
  br label %36

; <label>:35                                      ; preds = %30
  %vec.x23 = extractelement <3 x float> %currentRayData.i.i.0.1, i32 0
  %vec.y24 = extractelement <3 x float> %currentRayData.i.i.0.1, i32 1
  %vec.z25 = extractelement <3 x float> %currentRayData.i.i.0.1, i32 2
  call void @fb_Fallback_SetObjectRayOrigin(%struct.RuntimeDataStruct* %runtimeData, float %vec.x23, float %vec.y24, float %vec.z25)
  %vec.x26 = extractelement <3 x float> %currentRayData.i.i.1.1, i32 0
  %vec.y27 = extractelement <3 x float> %currentRayData.i.i.1.1, i32 1
  %vec.z28 = extractelement <3 x float> %currentRayData.i.i.1.1, i32 2
  call void @fb_Fallback_SetObjectRayDirection(%struct.RuntimeDataStruct* %runtimeData, float %vec.x26, float %vec.y27, float %vec.z28)
  %.upto0 = insertelement <4 x float> undef, float %CurrentWorldToObject.i.i168.1.i0, i32 0
  %.upto1 = insertelement <4 x float> %.upto0, float %CurrentWorldToObject.i.i168.1.i1, i32 1
  %.upto2 = insertelement <4 x float> %.upto1, float %CurrentWorldToObject.i.i168.1.i2, i32 2
  %v355 = insertelement <4 x float> %.upto2, float %CurrentWorldToObject.i.i168.1.i3, i32 3
  %v356 = insertvalue %class.matrix.float.3.4 undef, <4 x float> %v355, 0, 0
  %.upto01 = insertelement <4 x float> undef, float %CurrentWorldToObject.i.i168.1.i4, i32 0
  %.upto12 = insertelement <4 x float> %.upto01, float %CurrentWorldToObject.i.i168.1.i5, i32 1
  %.upto23 = insertelement <4 x float> %.upto12, float %CurrentWorldToObject.i.i168.1.i6, i32 2
  %v357 = insertelement <4 x float> %.upto23, float %CurrentWorldToObject.i.i168.1.i7, i32 3
  %v358 = insertvalue %class.matrix.float.3.4 %v356, <4 x float> %v357, 0, 1
  %.upto04 = insertelement <4 x float> undef, float %CurrentWorldToObject.i.i168.1.i8, i32 0
  %.upto15 = insertelement <4 x float> %.upto04, float %CurrentWorldToObject.i.i168.1.i9, i32 1
  %.upto26 = insertelement <4 x float> %.upto15, float %CurrentWorldToObject.i.i168.1.i10, i32 2
  %v359 = insertelement <4 x float> %.upto26, float %CurrentWorldToObject.i.i168.1.i11, i32 3
  %v360 = insertvalue %class.matrix.float.3.4 %v358, <4 x float> %v359, 0, 2
  store %class.matrix.float.3.4 %v360, %class.matrix.float.3.4* %tmp30
  %vec12.ptr31 = bitcast %class.matrix.float.3.4* %tmp30 to <12 x float>*
  %vec12.32 = load <12 x float>, <12 x float>* %vec12.ptr31
  call void @fb_Fallback_SetWorldToObject(%struct.RuntimeDataStruct* %runtimeData, <12 x float> %vec12.32)
  %.upto07 = insertelement <4 x float> undef, float %CurrentObjectToWorld.i.i167.1.i0, i32 0
  %.upto18 = insertelement <4 x float> %.upto07, float %CurrentObjectToWorld.i.i167.1.i1, i32 1
  %.upto29 = insertelement <4 x float> %.upto18, float %CurrentObjectToWorld.i.i167.1.i2, i32 2
  %v361 = insertelement <4 x float> %.upto29, float %CurrentObjectToWorld.i.i167.1.i3, i32 3
  %v362 = insertvalue %class.matrix.float.3.4 undef, <4 x float> %v361, 0, 0
  %.upto010 = insertelement <4 x float> undef, float %CurrentObjectToWorld.i.i167.1.i4, i32 0
  %.upto111 = insertelement <4 x float> %.upto010, float %CurrentObjectToWorld.i.i167.1.i5, i32 1
  %.upto212 = insertelement <4 x float> %.upto111, float %CurrentObjectToWorld.i.i167.1.i6, i32 2
  %v363 = insertelement <4 x float> %.upto212, float %CurrentObjectToWorld.i.i167.1.i7, i32 3
  %v364 = insertvalue %class.matrix.float.3.4 %v362, <4 x float> %v363, 0, 1
  %.upto013 = insertelement <4 x float> undef, float %CurrentObjectToWorld.i.i167.1.i8, i32 0
  %.upto114 = insertelement <4 x float> %.upto013, float %CurrentObjectToWorld.i.i167.1.i9, i32 1
  %.upto215 = insertelement <4 x float> %.upto114, float %CurrentObjectToWorld.i.i167.1.i10, i32 2
  %v365 = insertelement <4 x float> %.upto215, float %CurrentObjectToWorld.i.i167.1.i11, i32 3
  %v366 = insertvalue %class.matrix.float.3.4 %v364, <4 x float> %v365, 0, 2
  store %class.matrix.float.3.4 %v366, %class.matrix.float.3.4* %tmp29
  %vec12.ptr = bitcast %class.matrix.float.3.4* %tmp29 to <12 x float>*
  %vec12. = load <12 x float>, <12 x float>* %vec12.ptr
  call void @fb_Fallback_SetObjectToWorld(%struct.RuntimeDataStruct* %runtimeData, <12 x float> %vec12.)
  br label %36

; <label>:36                                      ; preds = %._crit_edge.10, %35
  call void @fb_Fallback_CommitHit(%struct.RuntimeDataStruct* %runtimeData)
  %v367 = and i32 %RayFlags256, 4
  %v368 = icmp eq i32 %v367, 0
  br i1 %v368, label %._crit_edge.11, label %37

._crit_edge.11:                                   ; preds = %36
  br label %18

; <label>:37                                      ; preds = %36
  store i32 0, i32* %v91, align 4, !tbaa !221
  store i32 0, i32* %v56, align 4, !tbaa !221
  br label %18

; <label>:38                                      ; preds = %22
  %v294 = fcmp fast olt float %v280, 0.000000e+00
  %v295 = fcmp fast olt float %v283, 0.000000e+00
  %v296 = or i1 %v294, %v295
  %v297 = fcmp fast olt float %v286, 0.000000e+00
  %v298 = or i1 %v297, %v296
  %.old = fcmp fast oeq float %v288, 0.000000e+00
  %or.cond194 = or i1 %v298, %.old
  br i1 %or.cond194, label %".\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i_crit_edge.12", label %._crit_edge.13

._crit_edge.13:                                   ; preds = %38
  br label %24

".\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i_crit_edge.12": ; preds = %38
  br label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i"

; <label>:39                                      ; preds = %"\01?IsOpaque@@YA_N_NII@Z.exit.i.i"
  %v289 = fcmp fast ogt float %v280, 0.000000e+00
  %v290 = fcmp fast ogt float %v283, 0.000000e+00
  %v291 = or i1 %v289, %v290
  %v292 = fcmp fast ogt float %v286, 0.000000e+00
  %v293 = or i1 %v292, %v291
  %.old.old = fcmp fast oeq float %v288, 0.000000e+00
  %or.cond195 = or i1 %v293, %.old.old
  br i1 %or.cond195, label %".\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i_crit_edge.14", label %._crit_edge.15

._crit_edge.15:                                   ; preds = %39
  br label %24

".\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i_crit_edge.14": ; preds = %39
  br label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i"

"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189": ; preds = %21
  %v123 = and i32 %v117, 2147483647
  %v124 = mul i32 %v123, 112
  %v125 = add i32 %v63, %v124
  %v126 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %v58)
  %v127 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %v126, i32 %v125, i32 undef)
  %v128 = extractvalue %dx.types.ResRet.i32 %v127, 0
  %v129 = extractvalue %dx.types.ResRet.i32 %v127, 1
  %v130 = extractvalue %dx.types.ResRet.i32 %v127, 2
  %v131 = extractvalue %dx.types.ResRet.i32 %v127, 3
  %v132 = add i32 %v125, 16
  %v133 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %v126, i32 %v132, i32 undef)
  %v134 = extractvalue %dx.types.ResRet.i32 %v133, 0
  %v135 = extractvalue %dx.types.ResRet.i32 %v133, 1
  %v136 = extractvalue %dx.types.ResRet.i32 %v133, 2
  %v137 = extractvalue %dx.types.ResRet.i32 %v133, 3
  %v138 = add i32 %v125, 32
  %v139 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %v126, i32 %v138, i32 undef)
  %v140 = extractvalue %dx.types.ResRet.i32 %v139, 0
  %v141 = extractvalue %dx.types.ResRet.i32 %v139, 1
  %v142 = extractvalue %dx.types.ResRet.i32 %v139, 2
  %v143 = extractvalue %dx.types.ResRet.i32 %v139, 3
  %v144 = add i32 %v125, 48
  %v145 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %v126, i32 %v144, i32 undef)
  %v146 = extractvalue %dx.types.ResRet.i32 %v145, 0
  %v147 = extractvalue %dx.types.ResRet.i32 %v145, 1
  %v148 = extractvalue %dx.types.ResRet.i32 %v145, 2
  %v149 = extractvalue %dx.types.ResRet.i32 %v145, 3
  %v150 = add i32 %v125, 64
  %v151 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %v126, i32 %v150, i32 undef)
  %v152 = extractvalue %dx.types.ResRet.i32 %v151, 0
  %v153 = extractvalue %dx.types.ResRet.i32 %v151, 1
  %v154 = extractvalue %dx.types.ResRet.i32 %v151, 2
  %v155 = extractvalue %dx.types.ResRet.i32 %v151, 3
  %v156 = add i32 %v125, 80
  %v157 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %v126, i32 %v156, i32 undef)
  %v158 = extractvalue %dx.types.ResRet.i32 %v157, 0
  %v159 = extractvalue %dx.types.ResRet.i32 %v157, 1
  %v160 = extractvalue %dx.types.ResRet.i32 %v157, 2
  %v161 = extractvalue %dx.types.ResRet.i32 %v157, 3
  %v162 = add i32 %v125, 96
  %v163 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %v126, i32 %v162, i32 undef)
  %v164 = extractvalue %dx.types.ResRet.i32 %v163, 0
  %v165 = extractvalue %dx.types.ResRet.i32 %v163, 1
  %v166 = extractvalue %dx.types.ResRet.i32 %v163, 2
  %v167 = extractvalue %dx.types.ResRet.i32 %v163, 3
  %v168 = bitcast i32 %v128 to float
  %v169 = bitcast i32 %v129 to float
  %v170 = bitcast i32 %v130 to float
  %v171 = bitcast i32 %v131 to float
  %v172 = bitcast i32 %v134 to float
  %v173 = bitcast i32 %v135 to float
  %v174 = bitcast i32 %v136 to float
  %v175 = bitcast i32 %v137 to float
  %v176 = bitcast i32 %v140 to float
  %v177 = bitcast i32 %v141 to float
  %v178 = bitcast i32 %v142 to float
  %v179 = bitcast i32 %v143 to float
  %v180 = and i32 %v147, 16777215
  %v181 = and i32 %v146, 16777215
  %v182 = lshr i32 %v146, 24
  %v183 = and i32 %v182, %arg1.int
  %v184 = icmp eq i32 %v183, 0
  %v185 = bitcast i32 %v152 to float
  %v186 = bitcast i32 %v153 to float
  %v187 = bitcast i32 %v154 to float
  %v188 = bitcast i32 %v155 to float
  %v189 = bitcast i32 %v158 to float
  %v190 = bitcast i32 %v159 to float
  %v191 = bitcast i32 %v160 to float
  %v192 = bitcast i32 %v161 to float
  %v193 = bitcast i32 %v164 to float
  %v194 = bitcast i32 %v165 to float
  %v195 = bitcast i32 %v166 to float
  %v196 = bitcast i32 %v167 to float
  br i1 %v184, label %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189._crit_edge", label %40

"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189._crit_edge": ; preds = %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189"
  br label %18

; <label>:40                                      ; preds = %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189"
  store i32 0, i32 addrspace(3)* %v106, align 4, !tbaa !221, !noalias !372
  %v197 = lshr i32 %v147, 24
  %v198 = fmul fast float %WorldRayDirection251, %v168
  %FMad285 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayDirection252, float %v169, float %v198)
  %FMad284 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayDirection253, float %v170, float %FMad285)
  %FMad283 = call float @dx.op.tertiary.f32(i32 46, float 0.000000e+00, float %v171, float %FMad284)
  %v199 = fmul fast float %WorldRayDirection251, %v172
  %FMad282 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayDirection252, float %v173, float %v199)
  %FMad281 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayDirection253, float %v174, float %FMad282)
  %FMad280 = call float @dx.op.tertiary.f32(i32 46, float 0.000000e+00, float %v175, float %FMad281)
  %v200 = fmul fast float %WorldRayDirection251, %v176
  %FMad279 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayDirection252, float %v177, float %v200)
  %FMad278 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayDirection253, float %v178, float %FMad279)
  %FMad = call float @dx.op.tertiary.f32(i32 46, float 0.000000e+00, float %v179, float %FMad278)
  %v201 = fmul fast float %WorldRayOrigin248, %v168
  %FMad294 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayOrigin249, float %v169, float %v201)
  %FMad293 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayOrigin250, float %v170, float %FMad294)
  %FMad292 = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %v171, float %FMad293)
  %v202 = fmul fast float %WorldRayOrigin248, %v172
  %FMad291 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayOrigin249, float %v173, float %v202)
  %FMad290 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayOrigin250, float %v174, float %FMad291)
  %FMad289 = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %v175, float %FMad290)
  %v203 = fmul fast float %WorldRayOrigin248, %v176
  %FMad288 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayOrigin249, float %v177, float %v203)
  %FMad287 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayOrigin250, float %v178, float %FMad288)
  %FMad286 = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %v179, float %FMad287)
  store float %FMad283, float* %v88, align 4
  store float %FMad280, float* %v89, align 4
  store float %FMad, float* %v90, align 4
  %.i0340 = fdiv fast float 1.000000e+00, %FMad283
  %.i1341 = fdiv fast float 1.000000e+00, %FMad280
  %.i2342 = fdiv fast float 1.000000e+00, %FMad
  %.i0343 = fmul fast float %.i0340, %FMad292
  %.i1344 = fmul fast float %.i1341, %FMad289
  %.i2345 = fmul fast float %.i2342, %FMad286
  %FAbs = call float @dx.op.unary.f32(i32 6, float %FMad283)
  %FAbs221 = call float @dx.op.unary.f32(i32 6, float %FMad280)
  %FAbs222 = call float @dx.op.unary.f32(i32 6, float %FMad)
  %v204 = fcmp fast ogt float %FAbs, %FAbs221
  %v205 = fcmp fast ogt float %FAbs, %FAbs222
  %v206 = and i1 %v204, %v205
  br i1 %v206, label %".\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i_crit_edge", label %41

".\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i_crit_edge": ; preds = %40
  br label %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i"

; <label>:41                                      ; preds = %40
  %v207 = fcmp fast ogt float %FAbs221, %FAbs222
  br i1 %v207, label %".\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i_crit_edge.16", label %42

".\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i_crit_edge.16": ; preds = %41
  br label %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i"

; <label>:42                                      ; preds = %41
  br label %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i"

"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i": ; preds = %".\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i_crit_edge.16", %".\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i_crit_edge", %42
  %.0172 = phi i32 [ 2, %42 ], [ 0, %".\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i_crit_edge" ], [ 1, %".\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i_crit_edge.16" ]
  %v208 = add nuw nsw i32 %.0172, 1
  %v209 = urem i32 %v208, 3
  %v210 = add nuw nsw i32 %.0172, 2
  %v211 = urem i32 %v210, 3
  %v212 = getelementptr [3 x float], [3 x float]* %v22, i32 0, i32 %.0172
  %v213 = load float, float* %v212, align 4, !tbaa !231, !noalias !375
  %v214 = fcmp fast olt float %v213, 0.000000e+00
  %.5.0.i0 = select i1 %v214, i32 %v211, i32 %v209
  %.5.0.i1 = select i1 %v214, i32 %v209, i32 %v211
  %v215 = getelementptr [3 x float], [3 x float]* %v22, i32 0, i32 %.5.0.i0
  %v216 = load float, float* %v215, align 4, !tbaa !231, !noalias !375
  %v217 = fdiv float %v216, %v213
  %v218 = getelementptr [3 x float], [3 x float]* %v22, i32 0, i32 %.5.0.i1
  %v219 = load float, float* %v218, align 4, !tbaa !231, !noalias !375
  %v220 = fdiv float %v219, %v213
  %v221 = fdiv float 1.000000e+00, %v213
  store i32 1, i32* %v91, align 4, !tbaa !221
  br label %18
}

define i32 @Fallback_TraceRay.ss_1(%struct.RuntimeDataStruct* %runtimeData) #9 {
Fallback_TraceRay.BB1.from.Fallback_CallIndirect:
  %stackFrame.offset.remat = call i32 @stackFrameOffset(%struct.RuntimeDataStruct* %runtimeData)
  %v24.ptr = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %stackFrame.offset.remat, i32 4)
  %v24.int = load i32, i32* %v24.ptr
  br label %0

; <label>:0                                       ; preds = %Fallback_TraceRay.BB1.from.Fallback_CallIndirect
  %v444 = call i32 @fb_Fallback_SetPayloadOffset(%struct.RuntimeDataStruct* %runtimeData, i32 %v24.int)
  call void @stackFramePop(%struct.RuntimeDataStruct* %runtimeData, i32 24)
  call void @traceFramePop(%struct.RuntimeDataStruct* %runtimeData)
  %ret.stackFrame.offset = call i32 @stackFrameOffset(%struct.RuntimeDataStruct* %runtimeData)
  %ret.stateId.ptr = call i32* @stackIntPtr(%struct.RuntimeDataStruct* %runtimeData, i32 %ret.stackFrame.offset, i32 0)
  %ret.stateId = load i32, i32* %ret.stateId.ptr
  ret i32 %ret.stateId
}

declare i32 @dx.op.threadId.i32(i32, i32)

declare i32 @dx.op.flattenedThreadIdInGroup.i32(i32)

define i32 @dispatch(%struct.RuntimeDataStruct* %runtimeData, i32 %stateID) {
entry:
  switch i32 %stateID, label %badStateID [
    i32 1000, label %state_1000.RayGen.ss_0
    i32 1001, label %state_1001.RayGen.ss_1
    i32 1002, label %state_1002.Hit.ss_0
    i32 1003, label %state_1003.Hit.ss_1
    i32 1004, label %state_1004.Miss.ss_0
    i32 1005, label %state_1005.Fallback_TraceRay.ss_0
    i32 1006, label %state_1006.Fallback_TraceRay.ss_1
  ]

state_1000.RayGen.ss_0:                           ; preds = %entry
  %nextStateId = call i32 @RayGen.ss_0(%struct.RuntimeDataStruct* %runtimeData)
  ret i32 %nextStateId

state_1001.RayGen.ss_1:                           ; preds = %entry
  %nextStateId1 = call i32 @RayGen.ss_1(%struct.RuntimeDataStruct* %runtimeData)
  ret i32 %nextStateId1

state_1002.Hit.ss_0:                              ; preds = %entry
  %nextStateId2 = call i32 @Hit.ss_0(%struct.RuntimeDataStruct* %runtimeData)
  ret i32 %nextStateId2

state_1003.Hit.ss_1:                              ; preds = %entry
  %nextStateId3 = call i32 @Hit.ss_1(%struct.RuntimeDataStruct* %runtimeData)
  ret i32 %nextStateId3

state_1004.Miss.ss_0:                             ; preds = %entry
  %nextStateId4 = call i32 @Miss.ss_0(%struct.RuntimeDataStruct* %runtimeData)
  ret i32 %nextStateId4

state_1005.Fallback_TraceRay.ss_0:                ; preds = %entry
  %nextStateId5 = call i32 @Fallback_TraceRay.ss_0(%struct.RuntimeDataStruct* %runtimeData)
  ret i32 %nextStateId5

state_1006.Fallback_TraceRay.ss_1:                ; preds = %entry
  %nextStateId6 = call i32 @Fallback_TraceRay.ss_1(%struct.RuntimeDataStruct* %runtimeData)
  ret i32 %nextStateId6

badStateID:                                       ; preds = %entry
  ret i32 -3
}

attributes #0 = { nounwind readnone }
attributes #1 = { nounwind readonly }
attributes #2 = { alwaysinline "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="0" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }
attributes #4 = { alwaysinline nounwind readonly "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="0" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { alwaysinline nounwind "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="0" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #6 = { alwaysinline nounwind readnone "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="0" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #7 = { alwaysinline nounwind "disable-tail-calls"="false" "exp-shader"="internal" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="0" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #8 = { alwaysinline }
attributes #9 = { "state_function"="true" }

!llvm.ident = !{!0, !0, !0, !0, !0}
!llvm.module.flags = !{!1, !2}
!dx.version = !{!3}
!dx.valver = !{!4}
!dx.shaderModel = !{!5}
!dx.resources = !{!6}
!dx.typeAnnotations = !{!36, !120}
!dx.entryPoints = !{!212}
!dx.func.props = !{!213, !214, !215, !216}
!dx.func.signatures = !{!217}

!0 = !{!"clang version 3.7 (tags/RELEASE_370/final)"}
!1 = !{i32 2, !"Dwarf Version", i32 4}
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 1, i32 1}
!4 = !{i32 1, i32 2}
!5 = !{!"lib", i32 6, i32 1}
!6 = !{!7, !25, !28, !33}
!7 = !{!8, !10, !11, !12, !13, !14, !15, !16, !17, !18, !19, !20, !21, !23}
!8 = !{i32 0, [27 x %"class.Texture2D<vector<float, 4> >"]* @"\01?g_texNormal@@3PAV?$Texture2D@V?$vector@M$03@@@@A", !"g_texNormal", i32 1, i32 32, i32 27, i32 2, i32 0, !9}
!9 = !{i32 0, i32 9}
!10 = !{i32 1, [27 x %"class.Texture2D<vector<float, 4> >"]* @"\01?g_texDiffuse@@3PAV?$Texture2D@V?$vector@M$03@@@@A", !"g_texDiffuse", i32 1, i32 5, i32 27, i32 2, i32 0, !9}
!11 = !{i32 2, %struct.ByteAddressBuffer* @"\01?RayGenShaderTable@@3UByteAddressBuffer@@A", !"RayGenShaderTable", i32 214743647, i32 2, i32 1, i32 11, i32 0, null}
!12 = !{i32 3, %struct.ByteAddressBuffer* @"\01?g_indices@@3UByteAddressBuffer@@A", !"g_indices", i32 1, i32 1, i32 1, i32 11, i32 0, null}
!13 = !{i32 4, %"class.Texture2D<float>"* @"\01?texSSAO@@3V?$Texture2D@M@@A", !"texSSAO", i32 1, i32 4, i32 1, i32 2, i32 0, !9}
!14 = !{i32 5, %struct.ByteAddressBuffer* @"\01?g_attributes@@3UByteAddressBuffer@@A", !"g_attributes", i32 1, i32 2, i32 1, i32 11, i32 0, null}
!15 = !{i32 6, %struct.ByteAddressBuffer* @"\01?HitGroupShaderTable@@3UByteAddressBuffer@@A", !"\01?HitGroupShaderTable@@3UByteAddressBuffer@@A", i32 214743647, i32 0, i32 1, i32 11, i32 0, null}
!16 = !{i32 7, %struct.ByteAddressBuffer* @"\01?HitGroupShaderTable@@3UByteAddressBuffer@@A", !"HitGroupShaderTable", i32 214743647, i32 0, i32 1, i32 11, i32 0, null}
!17 = !{i32 8, %"class.Texture2D<float>"* @"\01?depth@@3V?$Texture2D@M@@A", !"depth", i32 0, i32 1, i32 1, i32 2, i32 0, !9}
!18 = !{i32 9, %"class.Texture2D<float>"* @"\01?texShadow@@3V?$Texture2D@M@@A", !"texShadow", i32 1, i32 3, i32 1, i32 2, i32 0, !9}
!19 = !{i32 10, %struct.ByteAddressBuffer* @"\01?MissShaderTable@@3UByteAddressBuffer@@A", !"MissShaderTable", i32 214743647, i32 1, i32 1, i32 11, i32 0, null}
!20 = !{i32 11, %"class.Texture2D<vector<float, 3> >"* @"\01?normals@@3V?$Texture2D@V?$vector@M$02@@@@A", !"normals", i32 0, i32 2, i32 1, i32 2, i32 0, !9}
!21 = !{i32 12, %struct.RaytracingAccelerationStructure* @"\01?g_accel@@3URaytracingAccelerationStructure@@A", !"g_accel", i32 0, i32 0, i32 1, i32 16, i32 0, !22}
!22 = !{i32 0, i32 4}
!23 = !{i32 13, %"class.StructuredBuffer<RayTraceMeshInfo>"* @"\01?g_meshInfo@@3V?$StructuredBuffer@URayTraceMeshInfo@@@@A", !"g_meshInfo", i32 1, i32 0, i32 1, i32 12, i32 0, !24}
!24 = !{i32 1, i32 28}
!25 = !{!26, !27}
!26 = !{i32 0, [0 x %struct.RWByteAddressBuffer]* @"\01?DescriptorHeapBufferTable@@3PAURWByteAddressBuffer@@A", !"DescriptorHeapBufferTable", i32 214743648, i32 0, i32 -1, i32 11, i1 false, i1 false, i1 false, null}
!27 = !{i32 1, %"class.RWTexture2D<vector<float, 4> >"* @"\01?g_screenOutput@@3V?$RWTexture2D@V?$vector@M$03@@@@A", !"g_screenOutput", i32 0, i32 2, i32 1, i32 2, i1 false, i1 false, i1 false, !9}
!28 = !{!29, !30, !31, !32}
!29 = !{i32 0, %b1* @b1, !"b1", i32 0, i32 1, i32 1, i32 88, null}
!30 = !{i32 1, %HitShaderConstants* @HitShaderConstants, !"HitShaderConstants", i32 0, i32 0, i32 1, i32 136, null}
!31 = !{i32 2, %AccelerationStructureList* @AccelerationStructureList, !"AccelerationStructureList", i32 214743647, i32 1, i32 1, i32 8, null}
!32 = !{i32 3, %Constants* @Constants, !"Constants", i32 214743647, i32 0, i32 1, i32 32, null}
!33 = !{!34, !35}
!34 = !{i32 0, %struct.SamplerComparisonState* @"\01?shadowSampler@@3USamplerComparisonState@@A", !"shadowSampler", i32 0, i32 1, i32 1, i32 1, null}
!35 = !{i32 1, %struct.SamplerState* @"\01?g_s0@@3USamplerState@@A", !"g_s0", i32 0, i32 -1, i32 1, i32 0, null}
!36 = !{i32 0, %struct.RaytracingInstanceDesc undef, !37, %struct.RWByteAddressBufferPointer undef, !42, %struct.RWByteAddressBuffer undef, !45, %struct.BoundingBox undef, !47, %struct.RayPayload undef, !50, %struct.BVHMetadata undef, !53, %struct.ByteAddressBuffer undef, !45, %struct.AABB undef, !56, %struct.TriangleMetaData undef, !59, %struct.BuiltInTriangleIntersectionAttributes undef, !62, %struct.RayData undef, !64, %"class.RWTexture2D<vector<float, 4> >" undef, !71, %AccelerationStructureList undef, !73, %Constants undef, !75, %"class.Texture2D<vector<float, 4> >" undef, !84, %"class.Texture2D<vector<float, 4> >::mips_type" undef, !86, %"class.Texture2D<float>" undef, !88, %"class.Texture2D<float>::mips_type" undef, !86, %struct.RaytracingAccelerationStructure undef, !45, %"class.StructuredBuffer<RayTraceMeshInfo>" undef, !90, %struct.RayTraceMeshInfo undef, !92, %HitShaderConstants undef, !100, %b1 undef, !109, %struct.DynamicCB undef, !111, %Material undef, !116, %"class.Texture2D<vector<float, 3> >" undef, !118, %"class.Texture2D<vector<float, 3> >::mips_type" undef, !86}
!37 = !{i32 64, !38, !39, !40, !41}
!38 = !{i32 6, !"Transform", i32 3, i32 0, i32 7, i32 9}
!39 = !{i32 6, !"InstanceIDAndMask", i32 3, i32 48, i32 7, i32 5}
!40 = !{i32 6, !"InstanceContributionToHitGroupIndexAndFlags", i32 3, i32 52, i32 7, i32 5}
!41 = !{i32 6, !"AccelerationStructure", i32 3, i32 56, i32 7, i32 5}
!42 = !{i32 8, !43, !44}
!43 = !{i32 6, !"buffer", i32 3, i32 0}
!44 = !{i32 6, !"offsetInBytes", i32 3, i32 4, i32 7, i32 5}
!45 = !{i32 4, !46}
!46 = !{i32 6, !"h", i32 3, i32 0, i32 7, i32 4}
!47 = !{i32 28, !48, !49}
!48 = !{i32 6, !"center", i32 3, i32 0, i32 7, i32 9}
!49 = !{i32 6, !"halfDim", i32 3, i32 16, i32 7, i32 9}
!50 = !{i32 8, !51, !52}
!51 = !{i32 6, !"SkipShading", i32 3, i32 0, i32 7, i32 1}
!52 = !{i32 6, !"RayHitT", i32 3, i32 4, i32 7, i32 9}
!53 = !{i32 112, !54, !55}
!54 = !{i32 6, !"instanceDesc", i32 3, i32 0}
!55 = !{i32 6, !"ObjectToWorld", i32 3, i32 64, i32 7, i32 9}
!56 = !{i32 28, !57, !58}
!57 = !{i32 6, !"min", i32 3, i32 0, i32 7, i32 9}
!58 = !{i32 6, !"max", i32 3, i32 16, i32 7, i32 9}
!59 = !{i32 8, !60, !61}
!60 = !{i32 6, !"GeometryContributionToHitGroupIndex", i32 3, i32 0, i32 7, i32 5}
!61 = !{i32 6, !"PrimitiveIndex", i32 3, i32 4, i32 7, i32 5}
!62 = !{i32 8, !63}
!63 = !{i32 6, !"barycentrics", i32 3, i32 0, i32 7, i32 9}
!64 = !{i32 92, !65, !66, !67, !68, !69, !70}
!65 = !{i32 6, !"Origin", i32 3, i32 0, i32 7, i32 9}
!66 = !{i32 6, !"Direction", i32 3, i32 16, i32 7, i32 9}
!67 = !{i32 6, !"InverseDirection", i32 3, i32 32, i32 7, i32 9}
!68 = !{i32 6, !"OriginTimesRayInverseDirection", i32 3, i32 48, i32 7, i32 9}
!69 = !{i32 6, !"Shear", i32 3, i32 64, i32 7, i32 9}
!70 = !{i32 6, !"SwizzledIndices", i32 3, i32 80, i32 7, i32 4}
!71 = !{i32 16, !72}
!72 = !{i32 6, !"h", i32 3, i32 0, i32 7, i32 9}
!73 = !{i32 8, !74}
!74 = !{i32 6, !"TopLevelAccelerationStructureGpuVA", i32 3, i32 0, i32 7, i32 5}
!75 = !{i32 32, !76, !77, !78, !79, !80, !81, !82, !83}
!76 = !{i32 6, !"RayDispatchDimensionsWidth", i32 3, i32 0, i32 7, i32 5}
!77 = !{i32 6, !"RayDispatchDimensionsHeight", i32 3, i32 4, i32 7, i32 5}
!78 = !{i32 6, !"HitGroupShaderRecordStride", i32 3, i32 8, i32 7, i32 5}
!79 = !{i32 6, !"MissShaderRecordStride", i32 3, i32 12, i32 7, i32 5}
!80 = !{i32 6, !"SamplerDescriptorHeapStartLo", i32 3, i32 16, i32 7, i32 5}
!81 = !{i32 6, !"SamplerDescriptorHeapStartHi", i32 3, i32 20, i32 7, i32 5}
!82 = !{i32 6, !"SrvCbvUavDescriptorHeapStartLo", i32 3, i32 24, i32 7, i32 5}
!83 = !{i32 6, !"SrvCbvUavDescriptorHeapStartHi", i32 3, i32 28, i32 7, i32 5}
!84 = !{i32 20, !72, !85}
!85 = !{i32 6, !"mips", i32 3, i32 16}
!86 = !{i32 4, !87}
!87 = !{i32 6, !"handle", i32 3, i32 0, i32 7, i32 5}
!88 = !{i32 8, !72, !89}
!89 = !{i32 6, !"mips", i32 3, i32 4}
!90 = !{i32 28, !91}
!91 = !{i32 6, !"h", i32 3, i32 0}
!92 = !{i32 28, !93, !94, !95, !96, !97, !98, !99}
!93 = !{i32 6, !"m_indexOffsetBytes", i32 3, i32 0, i32 7, i32 5}
!94 = !{i32 6, !"m_uvAttributeOffsetBytes", i32 3, i32 4, i32 7, i32 5}
!95 = !{i32 6, !"m_normalAttributeOffsetBytes", i32 3, i32 8, i32 7, i32 5}
!96 = !{i32 6, !"m_tangentAttributeOffsetBytes", i32 3, i32 12, i32 7, i32 5}
!97 = !{i32 6, !"m_positionAttributeOffsetBytes", i32 3, i32 16, i32 7, i32 5}
!98 = !{i32 6, !"m_attributeStrideBytes", i32 3, i32 20, i32 7, i32 5}
!99 = !{i32 6, !"m_materialInstanceId", i32 3, i32 24, i32 7, i32 5}
!100 = !{i32 136, !101, !102, !103, !104, !105, !107, !108}
!101 = !{i32 6, !"SunDirection", i32 3, i32 0, i32 7, i32 9}
!102 = !{i32 6, !"SunColor", i32 3, i32 16, i32 7, i32 9}
!103 = !{i32 6, !"AmbientColor", i32 3, i32 32, i32 7, i32 9}
!104 = !{i32 6, !"ShadowTexelSize", i32 3, i32 48, i32 7, i32 9}
!105 = !{i32 6, !"ModelToShadow", i32 2, !106, i32 3, i32 64, i32 7, i32 9}
!106 = !{i32 4, i32 4, i32 1}
!107 = !{i32 6, !"IsReflection", i32 3, i32 128, i32 7, i32 5}
!108 = !{i32 6, !"UseShadowRays", i32 3, i32 132, i32 7, i32 5}
!109 = !{i32 88, !110}
!110 = !{i32 6, !"g_dynamic", i32 3, i32 0}
!111 = !{i32 88, !112, !113, !114, !115}
!112 = !{i32 6, !"cameraToWorld", i32 2, !106, i32 3, i32 0, i32 7, i32 9}
!113 = !{i32 6, !"worldCameraPosition", i32 3, i32 64, i32 7, i32 9}
!114 = !{i32 6, !"padding", i32 3, i32 76, i32 7, i32 5}
!115 = !{i32 6, !"resolution", i32 3, i32 80, i32 7, i32 9}
!116 = !{i32 4, !117}
!117 = !{i32 6, !"MaterialID", i32 3, i32 0, i32 7, i32 5}
!118 = !{i32 16, !72, !119}
!119 = !{i32 6, !"mips", i32 3, i32 12}
!120 = !{i32 1, %class.matrix.float.3.4 ([3 x <4 x float>]*)* @"\01?CreateMatrix@@YA?AV?$matrix@M$02$03@@Y02V?$vector@M$03@@@Z", !121, i32 (%struct.RaytracingInstanceDesc*)* @"\01?GetInstanceContributionToHitGroupIndex@@YAIURaytracingInstanceDesc@@@Z", !128, void (%struct.RWByteAddressBufferPointer*, <3 x float>*, <3 x float>*, <3 x float>*, i32)* @"\01?BVHReadTriangle@@YAXURWByteAddressBufferPointer@@AIAV?$vector@M$02@@11I@Z", !132, void (%struct.RWByteAddressBuffer*, <2 x i32>)* @"\01?PointerGetBuffer@@YA?AURWByteAddressBuffer@@V?$vector@I$01@@@Z", !136, float (%class.matrix.float.3.4)* @"\01?Determinant@@YAMV?$matrix@M$02$03@@@Z", !137, void (<2 x float>, <2 x float>, <2 x float>, <3 x float>, <3 x float>, <3 x float>, <3 x float>*, <3 x float>*)* @"\01?CalculateTrianglePartialDerivatives@@YAXV?$vector@M$01@@00V?$vector@M$02@@11AIAV2@2@Z", !139, i32 (<2 x i32>)* @"\01?GetLeafIndexFromFlag@@YAHV?$vector@I$01@@@Z", !140, i32 (<2 x i32>)* @"\01?GetLeftNodeIndex@@YAIV?$vector@I$01@@@Z", !143, i1 (i1, i32)* @"\01?Cull@@YA_N_NI@Z", !144, i32 (%struct.RWByteAddressBufferPointer*)* @"\01?GetOffsetToVertices@@YAHURWByteAddressBufferPointer@@@Z", !148, i32 (%struct.RaytracingInstanceDesc*)* @"\01?GetInstanceMask@@YAIURaytracingInstanceDesc@@@Z", !128, void (%struct.BoundingBox*, %struct.RWByteAddressBuffer*, i32, i32)* @"\01?GetBoxFromBuffer@@YA?AUBoundingBox@@URWByteAddressBuffer@@II@Z", !149, void (%struct.RayPayload*)* @Miss, !150, void (%struct.BoundingBox*, %struct.RWByteAddressBufferPointer*, i32, <2 x i32>*)* @"\01?BVHReadBoundingBox@@YA?AUBoundingBox@@URWByteAddressBufferPointer@@HAIAV?$vector@I$01@@@Z", !153, void (i32, i32, i32, i32, i32, float, float, float, float, float, float, float, float, i32)* @Fallback_TraceRay, !155, void (%struct.BVHMetadata*, %struct.RWByteAddressBuffer*, i32)* @"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z", !156, <2 x i32> (<2 x i32>, i32)* @"\01?PointerAdd@@YA?AV?$vector@I$01@@V1@I@Z", !157, void (%struct.RaytracingInstanceDesc*, %struct.ByteAddressBuffer*, i32)* @"\01?LoadRaytracingInstanceDesc@@YA?AURaytracingInstanceDesc@@UByteAddressBuffer@@I@Z", !156, void ()* @"\01?Fallback_IgnoreHit@@YAXXZ", !158, void (%struct.RaytracingInstanceDesc*, <4 x i32>, <4 x i32>, <4 x i32>, <4 x i32>)* @"\01?RawDataToRaytracingInstanceDesc@@YA?AURaytracingInstanceDesc@@V?$vector@I$03@@000@Z", !159, void (%struct.AABB*, %struct.BoundingBox*)* @"\01?BoundingBoxToAABB@@YA?AUAABB@@UBoundingBox@@@Z", !160, <2 x i32> (i32, i32)* @"\01?CreateFlag@@YA?AV?$vector@I$01@@II@Z", !157, void (%struct.TriangleMetaData*, %struct.RWByteAddressBufferPointer*, i32)* @"\01?BVHReadTriangleMetadata@@YA?AUTriangleMetaData@@URWByteAddressBufferPointer@@H@Z", !161, i1 (%class.matrix.float.2.2, %class.matrix.float.2.2*)* @"\01?Inverse2x2@@YA_NV?$matrix@M$01$01@@AIAV1@@Z", !162, i32 (<3 x float>)* @"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z", !167, i1 (%struct.RWByteAddressBufferPointer*, <2 x i32>, i32, <3 x float>, <3 x float>, <3 x i32>, <3 x float>, <2 x float>*, float*, i32*)* @"\01?TestLeafNodeIntersections@@YA_NURWByteAddressBufferPointer@@V?$vector@I$01@@IV?$vector@M$02@@2V?$vector@H$02@@2AIAV?$vector@M$01@@AIAMAIAI@Z", !168, i1 (i32, i32, i32)* @"\01?Traverse@@YA_NIII@Z", !171, i32 (i32*, i32*, i32)* @"\01?StackPop@@YAIAIAHAIAII@Z", !172, i32 (<2 x i32>)* @"\01?GetRightNodeIndex@@YAIV?$vector@I$01@@@Z", !143, void (%struct.RWByteAddressBufferPointer*, %struct.RWByteAddressBuffer*, i32)* @"\01?CreateRWByteAddressBufferPointer@@YA?AURWByteAddressBufferPointer@@URWByteAddressBuffer@@I@Z", !156, void (i32*, i32, i32, i32)* @"\01?StackPush@@YAXAIAHIII@Z", !174, void (float)* @"\01?LogFloat@@YAXM@Z", !175, i32 (i32, i32)* @"\01?GetBoxAddress@@YAIII@Z", !157, void (i32*, i1, i32, i32, i32, i32)* @"\01?StackPush2@@YAXAIAH_NIIII@Z", !176, void ()* @"\01?LogTraceRayEnd@@YAXXZ", !158, void (<3 x float>)* @"\01?LogFloat3@@YAXV?$vector@M$02@@@Z", !175, i32 (%struct.RaytracingInstanceDesc*)* @"\01?GetInstanceID@@YAIURaytracingInstanceDesc@@@Z", !128, void (%struct.BoundingBox*, <2 x i32>)* @"\01?dump@@YAXUBoundingBox@@V?$vector@I$01@@@Z", !177, i32 (i32)* @"\01?InvokeAnyHit@@YAHH@Z", !178, float (i32, i32)* @"\01?ComputeCullFaceDir@@YAMII@Z", !179, void (%struct.BoundingBox*, <2 x i32>, <4 x i32>*, <4 x i32>*)* @"\01?CompressBox@@YAXUBoundingBox@@V?$vector@I$01@@AIAV?$vector@I$03@@2@Z", !180, void (%struct.RWByteAddressBufferPointer*, <2 x i32>)* @"\01?CreateRWByteAddressBufferPointerFromGpuVA@@YA?AURWByteAddressBufferPointer@@V?$vector@I$01@@@Z", !136, void (%struct.BoundingBox*, %struct.AABB*)* @"\01?AABBtoBoundingBox@@YA?AUBoundingBox@@UAABB@@@Z", !160, void (%struct.AABB*, <4 x i32>, <4 x i32>)* @"\01?RawDataToAABB@@YA?AUAABB@@V?$vector@H$03@@0@Z", !181, <3 x float> (%struct.BoundingBox*)* @"\01?GetMinCorner@@YA?AV?$vector@M$02@@UBoundingBox@@@Z", !182, i1 (i1, i32, i32)* @"\01?IsOpaque@@YA_N_NII@Z", !183, void ()* @main, !184, void (<3 x i32>)* @"\01?LogInt3@@YAXV?$vector@H$02@@@Z", !185, <4 x i32> (<2 x i32>)* @"\01?Load4@@YA?AV?$vector@I$03@@V?$vector@I$01@@@Z", !143, void (%struct.RaytracingInstanceDesc*, %struct.RWByteAddressBuffer*, i32)* @"\01?LoadRaytracingInstanceDesc@@YA?AURaytracingInstanceDesc@@URWByteAddressBuffer@@I@Z", !156, <3 x float> (%struct.BoundingBox*)* @"\01?GetMaxCorner@@YA?AV?$vector@M$02@@UBoundingBox@@@Z", !182, void (%struct.RayPayload*, %struct.BuiltInTriangleIntersectionAttributes*)* @Hit, !186, void (i32, i32*, float, i32*, float, float*)* @"\01?RecordClosestBox@@YAXIAIA_NM0MAIAM@Z", !189, void (%struct.BoundingBox*, <3 x float>, <3 x float>, <3 x float>, i32, <2 x i32>*)* @"\01?GetBoxDataFromTriangle@@YA?AUBoundingBox@@V?$vector@M$02@@00HAIAV?$vector@I$01@@@Z", !191, void (%struct.BoundingBox*, %struct.BoundingBox*, i32, %struct.BoundingBox*, i32, <2 x i32>*)* @"\01?GetBoxFromChildBoxes@@YA?AUBoundingBox@@U1@H0HAIAV?$vector@I$01@@@Z", !192, i32 (%struct.RWByteAddressBufferPointer*)* @"\01?GetOffsetToBoxes@@YAHURWByteAddressBufferPointer@@@Z", !148, i32 (%struct.RWByteAddressBufferPointer*)* @"\01?GetOffsetToTriangleMetadata@@YAHURWByteAddressBufferPointer@@@Z", !148, void (<4 x i32>)* @"\01?Log@@YAXV?$vector@I$03@@@Z", !193, <2 x float> (i32)* @"\01?GetUVAttribute@@YA?AV?$vector@M$01@@I@Z", !194, i32 (%struct.RaytracingInstanceDesc*)* @"\01?GetInstanceFlags@@YAIURaytracingInstanceDesc@@@Z", !128, void (i32)* @"\01?LogInt@@YAXH@Z", !185, <3 x float> (<3 x float>, <3 x float>, float, float, <3 x float>, <3 x float>, <3 x float>, <3 x float>)* @"\01?ApplyLightCommon@@YA?AV?$vector@M$02@@V1@0MM0000@Z", !195, <3 x float> (<3 x float>, <3 x i32>)* @"\01?Swizzle@@YA?AV?$vector@M$02@@V1@V?$vector@H$02@@@Z", !196, i32 (i32, i32, i32, i32)* @"\01?Traverse@@YAHIIII@Z", !197, void (i32)* @"\01?LogNoData@@YAXI@Z", !193, i1 (<2 x i32>)* @"\01?IsLeaf@@YA_NV?$vector@I$01@@@Z", !198, void (%struct.RWByteAddressBuffer*, i32, %struct.BVHMetadata*)* @"\01?StoreBVHMetadataToRawData@@YAXURWByteAddressBuffer@@IUBVHMetadata@@@Z", !199, <3 x float> (<3 x float>, <3 x float>, <3 x float>, <3 x float>)* @"\01?RayPlaneIntersection@@YA?AV?$vector@M$02@@V1@000@Z", !200, void (<3 x float>*, <3 x float>*, <3 x float>, <3 x float>)* @"\01?FSchlick@@YAXAIAV?$vector@M$02@@0V1@1@Z", !201, void (%struct.BoundingBox*, <4 x i32>, <4 x i32>, <2 x i32>*)* @"\01?RawDataToBoundingBox@@YA?AUBoundingBox@@V?$vector@H$03@@0AIAV?$vector@I$01@@@Z", !202, void (%struct.AABB*, %struct.AABB*, %class.matrix.float.3.4)* @"\01?TransformAABB@@YA?AUAABB@@U1@V?$matrix@M$02$03@@@Z", !203, i32 (<2 x i32>)* @"\01?PointerGetBufferStartOffset@@YAIV?$vector@I$01@@@Z", !143, void (<3 x float>*, float*)* @"\01?AntiAliasSpecular@@YAXAIAV?$vector@M$02@@AIAM@Z", !204, %class.matrix.float.3.4 (%class.matrix.float.3.4)* @"\01?InverseAffineTransform@@YA?AV?$matrix@M$02$03@@V1@@Z", !205, float (<3 x float>)* @"\01?GetShadow@@YAMV?$vector@M$02@@@Z", !206, void (%struct.RayData*, <3 x float>, <3 x float>)* @"\01?GetRayData@@YA?AURayData@@V?$vector@M$02@@0@Z", !207, <3 x i32> (i32)* @"\01?Load3x16BitIndices@@YA?AV?$vector@I$02@@I@Z", !143, void ()* @"\01?Fallback_AcceptHitAndEndSearch@@YAXXZ", !158, void ()* @RayGen, !158, void (i32*, i32*)* @"\01?swap@@YAXAIAH0@Z", !208, i32 (i32, i32)* @"\01?GetTriangleMetadataAddress@@YAIII@Z", !157, void (%struct.RWByteAddressBuffer*, i32, i32, <2 x i32>)* @"\01?WriteOnlyFlagToBuffer@@YAXURWByteAddressBuffer@@IIV?$vector@I$01@@@Z", !209, void (%struct.RWByteAddressBuffer*, i32, i32, %struct.BoundingBox*, <2 x i32>)* @"\01?WriteBoxToBuffer@@YAXURWByteAddressBuffer@@IIUBoundingBox@@V?$vector@I$01@@@Z", !210, void ()* @"\01?LogTraceRayStart@@YAXXZ", !158, void (<2 x i32>)* @"\01?LogInt2@@YAXV?$vector@H$01@@@Z", !185, void (<3 x float>, <3 x float>, <3 x float>, <3 x float>, <3 x float>, <3 x float>, <2 x float>*, <2 x float>*)* @"\01?CalculateUVDerivatives@@YAXV?$vector@M$02@@00000AIAV?$vector@M$01@@1@Z", !139, void (<3 x i32>, i32)* @"\01?main@@YAXV?$vector@I$02@@I@Z", !211}
!121 = !{!122, !126}
!122 = !{i32 1, !123, !125}
!123 = !{i32 2, !124, i32 7, i32 9}
!124 = !{i32 3, i32 4, i32 1}
!125 = !{}
!126 = !{i32 0, !127, !125}
!127 = !{i32 7, i32 9}
!128 = !{!129, !131}
!129 = !{i32 1, !130, !125}
!130 = !{i32 7, i32 5}
!131 = !{i32 0, !125, !125}
!132 = !{!133, !131, !134, !134, !134, !135}
!133 = !{i32 1, !125, !125}
!134 = !{i32 1, !127, !125}
!135 = !{i32 0, !130, !125}
!136 = !{!131, !133, !135}
!137 = !{!134, !138}
!138 = !{i32 0, !123, !125}
!139 = !{!133, !126, !126, !126, !126, !126, !126, !134, !134}
!140 = !{!141, !135}
!141 = !{i32 1, !142, !125}
!142 = !{i32 7, i32 4}
!143 = !{!129, !135}
!144 = !{!145, !147, !135}
!145 = !{i32 1, !146, !125}
!146 = !{i32 7, i32 1}
!147 = !{i32 0, !146, !125}
!148 = !{!141, !131}
!149 = !{!131, !133, !131, !135, !135}
!150 = !{!133, !151}
!151 = !{i32 2, !152, !125}
!152 = !{i32 4, !"SV_RayPayload"}
!153 = !{!131, !133, !131, !154, !129}
!154 = !{i32 0, !142, !125}
!155 = !{!133, !135, !135, !135, !135, !135, !126, !126, !126, !126, !126, !126, !126, !126, !135}
!156 = !{!131, !133, !131, !135}
!157 = !{!129, !135, !135}
!158 = !{!133}
!159 = !{!131, !133, !135, !135, !135, !135}
!160 = !{!131, !133, !131}
!161 = !{!131, !133, !131, !154}
!162 = !{!145, !163, !166}
!163 = !{i32 0, !164, !125}
!164 = !{i32 2, !165, i32 7, i32 9}
!165 = !{i32 2, i32 2, i32 1}
!166 = !{i32 1, !164, !125}
!167 = !{!141, !126}
!168 = !{!145, !131, !135, !135, !126, !126, !154, !126, !169, !169, !170}
!169 = !{i32 2, !127, !125}
!170 = !{i32 2, !130, !125}
!171 = !{!145, !135, !135, !135}
!172 = !{!129, !173, !129, !135}
!173 = !{i32 2, !142, !125}
!174 = !{!133, !173, !135, !135, !135}
!175 = !{!133, !126}
!176 = !{!133, !173, !147, !135, !135, !135, !135}
!177 = !{!133, !131, !135}
!178 = !{!141, !154}
!179 = !{!134, !135, !135}
!180 = !{!133, !131, !135, !129, !129}
!181 = !{!131, !133, !154, !154}
!182 = !{!134, !131}
!183 = !{!145, !147, !135, !135}
!184 = !{!131}
!185 = !{!133, !154}
!186 = !{!133, !151, !187}
!187 = !{i32 0, !188, !125}
!188 = !{i32 4, !"SV_IntersectionAttributes"}
!189 = !{!133, !135, !190, !126, !190, !126, !169}
!190 = !{i32 2, !146, !125}
!191 = !{!131, !133, !126, !126, !126, !154, !129}
!192 = !{!131, !133, !131, !154, !131, !154, !129}
!193 = !{!133, !135}
!194 = !{!134, !135}
!195 = !{!134, !126, !126, !126, !126, !126, !126, !126, !126}
!196 = !{!134, !126, !154}
!197 = !{!141, !135, !135, !135, !135}
!198 = !{!145, !135}
!199 = !{!133, !131, !135, !131}
!200 = !{!134, !126, !126, !126, !126}
!201 = !{!133, !169, !169, !126, !126}
!202 = !{!131, !133, !154, !154, !129}
!203 = !{!131, !133, !131, !138}
!204 = !{!133, !169, !169}
!205 = !{!122, !138}
!206 = !{!134, !126}
!207 = !{!131, !133, !126, !126}
!208 = !{!133, !173, !173}
!209 = !{!133, !131, !135, !135, !135}
!210 = !{!133, !131, !135, !135, !131, !135}
!211 = !{!133, !135, !135}
!212 = !{null, !"", null, !6, null}
!213 = !{void (%struct.RayPayload*, %struct.BuiltInTriangleIntersectionAttributes*)* @Hit, i32 10, i32 8, i32 8}
!214 = !{void (%struct.RayPayload*)* @Miss, i32 11, i32 8}
!215 = !{void ()* @RayGen, i32 7}
!216 = !{void ()* @main, i32 5, i32 8, i32 8, i32 1}
!217 = !{void ()* @main, null}
!218 = !{!219, !219, i64 0}
!219 = !{!"omnipotent char", !220, i64 0}
!220 = !{!"Simple C/C++ TBAA"}
!221 = !{!222, !222, i64 0}
!222 = !{!"int", !219, i64 0}
!223 = !{!224}
!224 = distinct !{!224, !225, !"\01?RawDataToRaytracingInstanceDesc@@YA?AURaytracingInstanceDesc@@V?$vector@I$03@@000@Z: %agg.result"}
!225 = distinct !{!225, !"\01?RawDataToRaytracingInstanceDesc@@YA?AURaytracingInstanceDesc@@V?$vector@I$03@@000@Z"}
!226 = !{!227, !229, !230}
!227 = distinct !{!227, !228, !"\01?BVHReadTriangle@@YAXURWByteAddressBufferPointer@@AIAV?$vector@M$02@@11I@Z: %v0"}
!228 = distinct !{!228, !"\01?BVHReadTriangle@@YAXURWByteAddressBufferPointer@@AIAV?$vector@M$02@@11I@Z"}
!229 = distinct !{!229, !228, !"\01?BVHReadTriangle@@YAXURWByteAddressBufferPointer@@AIAV?$vector@M$02@@11I@Z: %v1"}
!230 = distinct !{!230, !228, !"\01?BVHReadTriangle@@YAXURWByteAddressBufferPointer@@AIAV?$vector@M$02@@11I@Z: %v2"}
!231 = !{!232, !232, i64 0}
!232 = !{!"float", !219, i64 0}
!233 = !{!234, !236}
!234 = distinct !{!234, !235, !"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z: %hitT"}
!235 = distinct !{!235, !"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z"}
!236 = distinct !{!236, !235, !"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z: %bary"}
!237 = !{!238}
!238 = distinct !{!238, !239, !"\01?GetRayData@@YA?AURayData@@V?$vector@M$02@@0@Z: %agg.result"}
!239 = distinct !{!239, !"\01?GetRayData@@YA?AURayData@@V?$vector@M$02@@0@Z"}
!240 = !{i32 1}
!241 = !{!242}
!242 = distinct !{!242, !243, !"\01?StackPush@@YAXAIAHIII@Z: %stackTop"}
!243 = distinct !{!243, !"\01?StackPush@@YAXAIAHIII@Z"}
!244 = !{!245}
!245 = distinct !{!245, !246, !"\01?StackPop@@YAIAIAHAIAII@Z: %stackTop"}
!246 = distinct !{!246, !"\01?StackPop@@YAIAIAHAIAII@Z"}
!247 = !{!248, !250}
!248 = distinct !{!248, !249, !"\01?PointerGetBuffer@@YA?AURWByteAddressBuffer@@V?$vector@I$01@@@Z: %agg.result"}
!249 = distinct !{!249, !"\01?PointerGetBuffer@@YA?AURWByteAddressBuffer@@V?$vector@I$01@@@Z"}
!250 = distinct !{!250, !251, !"\01?CreateRWByteAddressBufferPointerFromGpuVA@@YA?AURWByteAddressBufferPointer@@V?$vector@I$01@@@Z: %agg.result"}
!251 = distinct !{!251, !"\01?CreateRWByteAddressBufferPointerFromGpuVA@@YA?AURWByteAddressBufferPointer@@V?$vector@I$01@@@Z"}
!252 = !{!253}
!253 = distinct !{!253, !254, !"\01?StackPush@@YAXAIAHIII@Z: %stackTop"}
!254 = distinct !{!254, !"\01?StackPush@@YAXAIAHIII@Z"}
!255 = !{!256}
!256 = distinct !{!256, !257, !"\01?GetRayData@@YA?AURayData@@V?$vector@M$02@@0@Z: %agg.result"}
!257 = distinct !{!257, !"\01?GetRayData@@YA?AURayData@@V?$vector@M$02@@0@Z"}
!258 = !{!259, !261, !262, !264, !265}
!259 = distinct !{!259, !260, !"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z: %hitT"}
!260 = distinct !{!260, !"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z"}
!261 = distinct !{!261, !260, !"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z: %bary"}
!262 = distinct !{!262, !263, !"\01?TestLeafNodeIntersections@@YA_NURWByteAddressBufferPointer@@V?$vector@I$01@@IV?$vector@M$02@@2V?$vector@H$02@@2AIAV?$vector@M$01@@AIAMAIAI@Z: %resultBary"}
!263 = distinct !{!263, !"\01?TestLeafNodeIntersections@@YA_NURWByteAddressBufferPointer@@V?$vector@I$01@@IV?$vector@M$02@@2V?$vector@H$02@@2AIAV?$vector@M$01@@AIAMAIAI@Z"}
!264 = distinct !{!264, !263, !"\01?TestLeafNodeIntersections@@YA_NURWByteAddressBufferPointer@@V?$vector@I$01@@IV?$vector@M$02@@2V?$vector@H$02@@2AIAV?$vector@M$01@@AIAMAIAI@Z: %resultT"}
!265 = distinct !{!265, !263, !"\01?TestLeafNodeIntersections@@YA_NURWByteAddressBufferPointer@@V?$vector@I$01@@IV?$vector@M$02@@2V?$vector@H$02@@2AIAV?$vector@M$01@@AIAMAIAI@Z: %resultTriId"}
!266 = !{!267, !269}
!267 = distinct !{!267, !268, !"\01?PointerGetBuffer@@YA?AURWByteAddressBuffer@@V?$vector@I$01@@@Z: %agg.result"}
!268 = distinct !{!268, !"\01?PointerGetBuffer@@YA?AURWByteAddressBuffer@@V?$vector@I$01@@@Z"}
!269 = distinct !{!269, !270, !"\01?CreateRWByteAddressBufferPointerFromGpuVA@@YA?AURWByteAddressBufferPointer@@V?$vector@I$01@@@Z: %agg.result"}
!270 = distinct !{!270, !"\01?CreateRWByteAddressBufferPointerFromGpuVA@@YA?AURWByteAddressBufferPointer@@V?$vector@I$01@@@Z"}
!271 = !{!272}
!272 = distinct !{!272, !273, !"\01?StackPush2@@YAXAIAH_NIIII@Z: %stackTop"}
!273 = distinct !{!273, !"\01?StackPush2@@YAXAIAH_NIIII@Z"}
!274 = !{!275}
!275 = distinct !{!275, !276, !"\01?StackPush@@YAXAIAHIII@Z: %stackTop"}
!276 = distinct !{!276, !"\01?StackPush@@YAXAIAHIII@Z"}
!277 = !{!278}
!278 = distinct !{!278, !279, !"\01?CreateRWByteAddressBufferPointer@@YA?AURWByteAddressBufferPointer@@URWByteAddressBuffer@@I@Z: %agg.result"}
!279 = distinct !{!279, !"\01?CreateRWByteAddressBufferPointer@@YA?AURWByteAddressBufferPointer@@URWByteAddressBuffer@@I@Z"}
!280 = !{!281}
!281 = distinct !{!281, !282, !"\01?BoundingBoxToAABB@@YA?AUAABB@@UBoundingBox@@@Z: %agg.result"}
!282 = distinct !{!282, !"\01?BoundingBoxToAABB@@YA?AUAABB@@UBoundingBox@@@Z"}
!283 = !{!284}
!284 = distinct !{!284, !285, !"\01?PointerGetBuffer@@YA?AURWByteAddressBuffer@@V?$vector@I$01@@@Z: %agg.result"}
!285 = distinct !{!285, !"\01?PointerGetBuffer@@YA?AURWByteAddressBuffer@@V?$vector@I$01@@@Z"}
!286 = !{!287}
!287 = distinct !{!287, !288, !"\01?RawDataToRaytracingInstanceDesc@@YA?AURaytracingInstanceDesc@@V?$vector@I$03@@000@Z: %agg.result"}
!288 = distinct !{!288, !"\01?RawDataToRaytracingInstanceDesc@@YA?AURaytracingInstanceDesc@@V?$vector@I$03@@000@Z"}
!289 = !{!290}
!290 = distinct !{!290, !291, !"\01?GetRayData@@YA?AURayData@@V?$vector@M$02@@0@Z: %agg.result"}
!291 = distinct !{!291, !"\01?GetRayData@@YA?AURayData@@V?$vector@M$02@@0@Z"}
!292 = !{!293}
!293 = distinct !{!293, !294, !"\01?StackPush@@YAXAIAHIII@Z: %stackTop"}
!294 = distinct !{!294, !"\01?StackPush@@YAXAIAHIII@Z"}
!295 = !{!296}
!296 = distinct !{!296, !297, !"\01?StackPop@@YAIAIAHAIAII@Z: %stackTop"}
!297 = distinct !{!297, !"\01?StackPop@@YAIAIAHAIAII@Z"}
!298 = !{!299, !301}
!299 = distinct !{!299, !300, !"\01?PointerGetBuffer@@YA?AURWByteAddressBuffer@@V?$vector@I$01@@@Z: %agg.result"}
!300 = distinct !{!300, !"\01?PointerGetBuffer@@YA?AURWByteAddressBuffer@@V?$vector@I$01@@@Z"}
!301 = distinct !{!301, !302, !"\01?CreateRWByteAddressBufferPointerFromGpuVA@@YA?AURWByteAddressBufferPointer@@V?$vector@I$01@@@Z: %agg.result"}
!302 = distinct !{!302, !"\01?CreateRWByteAddressBufferPointerFromGpuVA@@YA?AURWByteAddressBufferPointer@@V?$vector@I$01@@@Z"}
!303 = !{!304}
!304 = distinct !{!304, !305, !"\01?StackPush@@YAXAIAHIII@Z: %stackTop"}
!305 = distinct !{!305, !"\01?StackPush@@YAXAIAHIII@Z"}
!306 = !{!307}
!307 = distinct !{!307, !308, !"\01?GetRayData@@YA?AURayData@@V?$vector@M$02@@0@Z: %agg.result"}
!308 = distinct !{!308, !"\01?GetRayData@@YA?AURayData@@V?$vector@M$02@@0@Z"}
!309 = !{!310, !312, !313, !315, !316}
!310 = distinct !{!310, !311, !"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z: %hitT"}
!311 = distinct !{!311, !"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z"}
!312 = distinct !{!312, !311, !"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z: %bary"}
!313 = distinct !{!313, !314, !"\01?TestLeafNodeIntersections@@YA_NURWByteAddressBufferPointer@@V?$vector@I$01@@IV?$vector@M$02@@2V?$vector@H$02@@2AIAV?$vector@M$01@@AIAMAIAI@Z: %resultBary"}
!314 = distinct !{!314, !"\01?TestLeafNodeIntersections@@YA_NURWByteAddressBufferPointer@@V?$vector@I$01@@IV?$vector@M$02@@2V?$vector@H$02@@2AIAV?$vector@M$01@@AIAMAIAI@Z"}
!315 = distinct !{!315, !314, !"\01?TestLeafNodeIntersections@@YA_NURWByteAddressBufferPointer@@V?$vector@I$01@@IV?$vector@M$02@@2V?$vector@H$02@@2AIAV?$vector@M$01@@AIAMAIAI@Z: %resultT"}
!316 = distinct !{!316, !314, !"\01?TestLeafNodeIntersections@@YA_NURWByteAddressBufferPointer@@V?$vector@I$01@@IV?$vector@M$02@@2V?$vector@H$02@@2AIAV?$vector@M$01@@AIAMAIAI@Z: %resultTriId"}
!317 = !{!318, !320}
!318 = distinct !{!318, !319, !"\01?PointerGetBuffer@@YA?AURWByteAddressBuffer@@V?$vector@I$01@@@Z: %agg.result"}
!319 = distinct !{!319, !"\01?PointerGetBuffer@@YA?AURWByteAddressBuffer@@V?$vector@I$01@@@Z"}
!320 = distinct !{!320, !321, !"\01?CreateRWByteAddressBufferPointerFromGpuVA@@YA?AURWByteAddressBufferPointer@@V?$vector@I$01@@@Z: %agg.result"}
!321 = distinct !{!321, !"\01?CreateRWByteAddressBufferPointerFromGpuVA@@YA?AURWByteAddressBufferPointer@@V?$vector@I$01@@@Z"}
!322 = !{!323}
!323 = distinct !{!323, !324, !"\01?StackPush2@@YAXAIAH_NIIII@Z: %stackTop"}
!324 = distinct !{!324, !"\01?StackPush2@@YAXAIAH_NIIII@Z"}
!325 = !{!326}
!326 = distinct !{!326, !327, !"\01?StackPush@@YAXAIAHIII@Z: %stackTop"}
!327 = distinct !{!327, !"\01?StackPush@@YAXAIAHIII@Z"}
!328 = !{!329, !331}
!329 = distinct !{!329, !330, !"\01?CompressBox@@YAXUBoundingBox@@V?$vector@I$01@@AIAV?$vector@I$03@@2@Z: %data1"}
!330 = distinct !{!330, !"\01?CompressBox@@YAXUBoundingBox@@V?$vector@I$01@@AIAV?$vector@I$03@@2@Z"}
!331 = distinct !{!331, !330, !"\01?CompressBox@@YAXUBoundingBox@@V?$vector@I$01@@AIAV?$vector@I$03@@2@Z: %data2"}
!332 = !{!333, !333, i64 0}
!333 = !{!"bool", !219, i64 0}
!334 = !{i32 0, i32 2}
!335 = !{!336, !338}
!336 = distinct !{!336, !337, !"\01?CalculateUVDerivatives@@YAXV?$vector@M$02@@00000AIAV?$vector@M$01@@1@Z: %ddX"}
!337 = distinct !{!337, !"\01?CalculateUVDerivatives@@YAXV?$vector@M$02@@00000AIAV?$vector@M$01@@1@Z"}
!338 = distinct !{!338, !337, !"\01?CalculateUVDerivatives@@YAXV?$vector@M$02@@00000AIAV?$vector@M$01@@1@Z: %ddY"}
!339 = !{!340}
!340 = distinct !{!340, !341, !"\01?GetRayData@@YA?AURayData@@V?$vector@M$02@@0@Z: %agg.result"}
!341 = distinct !{!341, !"\01?GetRayData@@YA?AURayData@@V?$vector@M$02@@0@Z"}
!342 = !{!343}
!343 = distinct !{!343, !344, !"\01?StackPush@@YAXAIAHIII@Z: %stackTop"}
!344 = distinct !{!344, !"\01?StackPush@@YAXAIAHIII@Z"}
!345 = !{!346}
!346 = distinct !{!346, !347, !"\01?StackPop@@YAIAIAHAIAII@Z: %stackTop"}
!347 = distinct !{!347, !"\01?StackPop@@YAIAIAHAIAII@Z"}
!348 = !{!349, !351}
!349 = distinct !{!349, !350, !"\01?PointerGetBuffer@@YA?AURWByteAddressBuffer@@V?$vector@I$01@@@Z: %agg.result"}
!350 = distinct !{!350, !"\01?PointerGetBuffer@@YA?AURWByteAddressBuffer@@V?$vector@I$01@@@Z"}
!351 = distinct !{!351, !352, !"\01?CreateRWByteAddressBufferPointerFromGpuVA@@YA?AURWByteAddressBufferPointer@@V?$vector@I$01@@@Z: %agg.result"}
!352 = distinct !{!352, !"\01?CreateRWByteAddressBufferPointerFromGpuVA@@YA?AURWByteAddressBufferPointer@@V?$vector@I$01@@@Z"}
!353 = !{!354}
!354 = distinct !{!354, !355, !"\01?StackPush@@YAXAIAHIII@Z: %stackTop"}
!355 = distinct !{!355, !"\01?StackPush@@YAXAIAHIII@Z"}
!356 = !{!357}
!357 = distinct !{!357, !358, !"\01?StackPush2@@YAXAIAH_NIIII@Z: %stackTop"}
!358 = distinct !{!358, !"\01?StackPush2@@YAXAIAH_NIIII@Z"}
!359 = !{!360, !362, !363, !365, !366}
!360 = distinct !{!360, !361, !"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z: %hitT"}
!361 = distinct !{!361, !"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z"}
!362 = distinct !{!362, !361, !"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z: %bary"}
!363 = distinct !{!363, !364, !"\01?TestLeafNodeIntersections@@YA_NURWByteAddressBufferPointer@@V?$vector@I$01@@IV?$vector@M$02@@2V?$vector@H$02@@2AIAV?$vector@M$01@@AIAMAIAI@Z: %resultBary"}
!364 = distinct !{!364, !"\01?TestLeafNodeIntersections@@YA_NURWByteAddressBufferPointer@@V?$vector@I$01@@IV?$vector@M$02@@2V?$vector@H$02@@2AIAV?$vector@M$01@@AIAMAIAI@Z"}
!365 = distinct !{!365, !364, !"\01?TestLeafNodeIntersections@@YA_NURWByteAddressBufferPointer@@V?$vector@I$01@@IV?$vector@M$02@@2V?$vector@H$02@@2AIAV?$vector@M$01@@AIAMAIAI@Z: %resultT"}
!366 = distinct !{!366, !364, !"\01?TestLeafNodeIntersections@@YA_NURWByteAddressBufferPointer@@V?$vector@I$01@@IV?$vector@M$02@@2V?$vector@H$02@@2AIAV?$vector@M$01@@AIAMAIAI@Z: %resultTriId"}
!367 = !{!368, !370}
!368 = distinct !{!368, !369, !"\01?PointerGetBuffer@@YA?AURWByteAddressBuffer@@V?$vector@I$01@@@Z: %agg.result"}
!369 = distinct !{!369, !"\01?PointerGetBuffer@@YA?AURWByteAddressBuffer@@V?$vector@I$01@@@Z"}
!370 = distinct !{!370, !371, !"\01?CreateRWByteAddressBufferPointerFromGpuVA@@YA?AURWByteAddressBufferPointer@@V?$vector@I$01@@@Z: %agg.result"}
!371 = distinct !{!371, !"\01?CreateRWByteAddressBufferPointerFromGpuVA@@YA?AURWByteAddressBufferPointer@@V?$vector@I$01@@@Z"}
!372 = !{!373}
!373 = distinct !{!373, !374, !"\01?StackPush@@YAXAIAHIII@Z: %stackTop"}
!374 = distinct !{!374, !"\01?StackPush@@YAXAIAHIII@Z"}
!375 = !{!376}
!376 = distinct !{!376, !377, !"\01?GetRayData@@YA?AURayData@@V?$vector@M$02@@0@Z: %agg.result"}
!377 = distinct !{!377, !"\01?GetRayData@@YA?AURayData@@V?$vector@M$02@@0@Z"}
