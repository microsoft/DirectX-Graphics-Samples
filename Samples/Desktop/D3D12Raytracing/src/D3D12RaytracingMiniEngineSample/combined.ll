; ModuleID = 'merged_lib'
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
define void @"\01?Miss@@YAXURayPayload@@@Z"(%struct.RayPayload* noalias nocapture) #5 {
  %2 = getelementptr inbounds %struct.RayPayload, %struct.RayPayload* %0, i32 0, i32 0
  %3 = load i32, i32* %2, align 4
  %4 = getelementptr inbounds %struct.RayPayload, %struct.RayPayload* %0, i32 0, i32 1
  %5 = load float, float* %4, align 4
  %6 = icmp eq i32 %3, 0
  br i1 %6, label %7, label %10

; <label>:7                                       ; preds = %1
  %8 = load %"class.RWTexture2D<vector<float, 4> >", %"class.RWTexture2D<vector<float, 4> >"* @"\01?g_screenOutput@@3V?$RWTexture2D@V?$vector@M$03@@@@A", align 4
  %DispatchRaysIndex = call i32 @dx.op.dispatchRaysIndex.i32(i32 145, i8 0)
  %DispatchRaysIndex1 = call i32 @dx.op.dispatchRaysIndex.i32(i32 145, i8 1)
  %9 = call %dx.types.Handle @"dx.op.createHandleFromResourceStructForLib.class.RWTexture2D<vector<float, 4> >"(i32 160, %"class.RWTexture2D<vector<float, 4> >" %8)
  call void @dx.op.textureStore.f32(i32 67, %dx.types.Handle %9, i32 %DispatchRaysIndex, i32 %DispatchRaysIndex1, i32 undef, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 1.000000e+00, i8 15)
  br label %10

; <label>:10                                      ; preds = %7, %1
  store i32 %3, i32* %2, align 4
  store float %5, float* %4, align 4
  ret void
}

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
define void @"\01?Fallback_TraceRay@@YAXIIIIIMMMMMMMMI@Z"(i32, i32, i32, i32, i32, float, float, float, float, float, float, float, float, i32) #7 {
  %15 = load %struct.ByteAddressBuffer, %struct.ByteAddressBuffer* @"\01?MissShaderTable@@3UByteAddressBuffer@@A", align 4
  %16 = load %struct.ByteAddressBuffer, %struct.ByteAddressBuffer* @"\01?HitGroupShaderTable@@3UByteAddressBuffer@@A", align 4
  %17 = load %AccelerationStructureList, %AccelerationStructureList* @AccelerationStructureList, align 4
  %18 = load %Constants, %Constants* @Constants, align 4
  %AccelerationStructureList = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.AccelerationStructureList(i32 160, %AccelerationStructureList %17)
  %Constants295 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.Constants(i32 160, %Constants %18)
  %19 = alloca [3 x float], align 4
  %20 = alloca [3 x float], align 4
  %21 = alloca [3 x float], align 4
  %22 = alloca [3 x float], align 4
  %23 = alloca [3 x float], align 4
  %nodesToProcess.i.i = alloca [2 x i32], align 4
  %attr.i.i = alloca %struct.BuiltInTriangleIntersectionAttributes, align 4
  %24 = call i32 @"\01?Fallback_SetPayloadOffset@@YAII@Z"(i32 %13) #3
  call void @"\01?Fallback_SetRayFlags@@YAXI@Z"(i32 %0) #3
  %25 = insertelement <3 x float> undef, float %9, i64 0
  %26 = insertelement <3 x float> %25, float %10, i64 1
  %27 = insertelement <3 x float> %26, float %11, i64 2
  %28 = insertelement <3 x float> undef, float %5, i64 0
  %29 = insertelement <3 x float> %28, float %6, i64 1
  %30 = insertelement <3 x float> %29, float %7, i64 2
  call void @"\01?Fallback_TraceRayBegin@@YAXV?$vector@M$02@@M0M@Z"(<3 x float> %30, float %8, <3 x float> %27, float %12) #3
  %31 = call i32 @"\01?Fallback_GroupIndex@@YAIXZ"() #3
  %WorldRayDirection251 = call float @dx.op.worldRayDirection.f32(i32 148, i8 0)
  %WorldRayDirection252 = call float @dx.op.worldRayDirection.f32(i32 148, i8 1)
  %WorldRayDirection253 = call float @dx.op.worldRayDirection.f32(i32 148, i8 2)
  %WorldRayOrigin248 = call float @dx.op.worldRayOrigin.f32(i32 147, i8 0)
  %WorldRayOrigin249 = call float @dx.op.worldRayOrigin.f32(i32 147, i8 1)
  %WorldRayOrigin250 = call float @dx.op.worldRayOrigin.f32(i32 147, i8 2)
  %32 = getelementptr inbounds [3 x float], [3 x float]* %23, i32 0, i32 0
  store float %WorldRayDirection251, float* %32, align 4
  %33 = getelementptr inbounds [3 x float], [3 x float]* %23, i32 0, i32 1
  store float %WorldRayDirection252, float* %33, align 4
  %34 = getelementptr inbounds [3 x float], [3 x float]* %23, i32 0, i32 2
  store float %WorldRayDirection253, float* %34, align 4
  %.i0 = fdiv fast float 1.000000e+00, %WorldRayDirection251
  %.i1 = fdiv fast float 1.000000e+00, %WorldRayDirection252
  %.i2 = fdiv fast float 1.000000e+00, %WorldRayDirection253
  %.i0296 = fmul fast float %.i0, %WorldRayOrigin248
  %.i1297 = fmul fast float %.i1, %WorldRayOrigin249
  %.i2298 = fmul fast float %.i2, %WorldRayOrigin250
  %FAbs241 = call float @dx.op.unary.f32(i32 6, float %WorldRayDirection251)
  %FAbs242 = call float @dx.op.unary.f32(i32 6, float %WorldRayDirection252)
  %FAbs243 = call float @dx.op.unary.f32(i32 6, float %WorldRayDirection253)
  %35 = fcmp fast ogt float %FAbs241, %FAbs242
  %36 = fcmp fast ogt float %FAbs241, %FAbs243
  %37 = and i1 %35, %36
  br i1 %37, label %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i", label %38

; <label>:38                                      ; preds = %14
  %39 = fcmp fast ogt float %FAbs242, %FAbs243
  br i1 %39, label %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i", label %40

; <label>:40                                      ; preds = %38
  br label %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i"

"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i": ; preds = %40, %38, %14
  %.0171 = phi i32 [ 2, %40 ], [ 0, %14 ], [ 1, %38 ]
  %41 = add nuw nsw i32 %.0171, 1
  %42 = urem i32 %41, 3
  %43 = add nuw nsw i32 %.0171, 2
  %44 = urem i32 %43, 3
  %45 = getelementptr [3 x float], [3 x float]* %23, i32 0, i32 %.0171
  %46 = load float, float* %45, align 4, !tbaa !223, !noalias !225
  %47 = fcmp fast olt float %46, 0.000000e+00
  %worldRayData.i.i.5.0.i0 = select i1 %47, i32 %44, i32 %42
  %worldRayData.i.i.5.0.i1 = select i1 %47, i32 %42, i32 %44
  %48 = getelementptr [3 x float], [3 x float]* %23, i32 0, i32 %worldRayData.i.i.5.0.i0
  %49 = load float, float* %48, align 4, !tbaa !223, !noalias !225
  %50 = fdiv float %49, %46
  %51 = getelementptr [3 x float], [3 x float]* %23, i32 0, i32 %worldRayData.i.i.5.0.i1
  %52 = load float, float* %51, align 4, !tbaa !223, !noalias !225
  %53 = fdiv float %52, %46
  %54 = fdiv float 1.000000e+00, %46
  %55 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %AccelerationStructureList, i32 0)
  %56 = extractvalue %dx.types.CBufRet.i32 %55, 0
  %57 = extractvalue %dx.types.CBufRet.i32 %55, 1
  %58 = getelementptr inbounds [2 x i32], [2 x i32]* %nodesToProcess.i.i, i32 0, i32 0
  store i32 0, i32* %58, align 4, !tbaa !221
  %59 = getelementptr inbounds [0 x %struct.RWByteAddressBuffer], [0 x %struct.RWByteAddressBuffer]* @"\01?DescriptorHeapBufferTable@@3PAURWByteAddressBuffer@@A", i32 0, i32 %57, !dx.nonuniform !228
  %60 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %59, align 4, !noalias !125
  %61 = add i32 %56, 4
  %62 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %60)
  %63 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %62, i32 %61, i32 undef)
  %64 = extractvalue %dx.types.ResRet.i32 %63, 0
  %65 = add i32 %64, %56
  %66 = add i32 %56, 16
  %67 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %62, i32 %66, i32 undef)
  %68 = extractvalue %dx.types.ResRet.i32 %67, 0
  %69 = extractvalue %dx.types.ResRet.i32 %67, 1
  %70 = extractvalue %dx.types.ResRet.i32 %67, 2
  %71 = add i32 %56, 32
  %72 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %62, i32 %71, i32 undef)
  %73 = extractvalue %dx.types.ResRet.i32 %72, 0
  %74 = extractvalue %dx.types.ResRet.i32 %72, 1
  %75 = extractvalue %dx.types.ResRet.i32 %72, 2
  %76 = bitcast i32 %68 to float
  %77 = bitcast i32 %69 to float
  %78 = bitcast i32 %70 to float
  %79 = bitcast i32 %73 to float
  %80 = bitcast i32 %74 to float
  %81 = bitcast i32 %75 to float
  %RayTCurrent220 = call float @dx.op.rayTCurrent.f32(i32 154)
  %82 = fsub fast float %76, %WorldRayOrigin248
  %83 = fmul fast float %82, %.i0
  %84 = fsub fast float %77, %WorldRayOrigin249
  %85 = fmul fast float %84, %.i1
  %86 = fsub fast float %78, %WorldRayOrigin250
  %87 = fmul fast float %86, %.i2
  %FAbs238 = call float @dx.op.unary.f32(i32 6, float %.i0)
  %FAbs239 = call float @dx.op.unary.f32(i32 6, float %.i1)
  %FAbs240 = call float @dx.op.unary.f32(i32 6, float %.i2)
  %.i0305 = fmul fast float %79, %FAbs238
  %.i1306 = fmul fast float %FAbs239, %80
  %.i2307 = fmul fast float %FAbs240, %81
  %.i0308 = fadd fast float %.i0305, %83
  %.i1309 = fadd fast float %.i1306, %85
  %.i2310 = fadd fast float %.i2307, %87
  %.i0314 = fsub fast float %83, %.i0305
  %.i1315 = fsub fast float %85, %.i1306
  %.i2316 = fsub fast float %87, %.i2307
  %FMax277 = call float @dx.op.binary.f32(i32 35, float %.i0314, float %.i1315)
  %FMax276 = call float @dx.op.binary.f32(i32 35, float %FMax277, float %.i2316)
  %FMin275 = call float @dx.op.binary.f32(i32 36, float %.i0308, float %.i1309)
  %FMin274 = call float @dx.op.binary.f32(i32 36, float %FMin275, float %.i2310)
  %FMax273 = call float @dx.op.binary.f32(i32 35, float %FMax276, float 0.000000e+00)
  %FMin272 = call float @dx.op.binary.f32(i32 36, float %FMin274, float %RayTCurrent220)
  %88 = fcmp fast olt float %FMax273, %FMin272
  br i1 %88, label %.lr.ph.preheader.critedge, label %104

.lr.ph.preheader.critedge:                        ; preds = %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i"
  %89 = getelementptr [2048 x i32], [2048 x i32] addrspace(3)* @"\01?stack@@3PAIA", i32 0, i32 %31
  store i32 0, i32 addrspace(3)* %89, align 4, !tbaa !221, !noalias !229
  store i32 1, i32* %58, align 4, !tbaa !221
  call void @"\01?Fallback_SetInstanceIndex@@YAXI@Z"(i32 -1) #3
  %90 = getelementptr inbounds [3 x float], [3 x float]* %22, i32 0, i32 0
  %91 = getelementptr inbounds [3 x float], [3 x float]* %22, i32 0, i32 1
  %92 = getelementptr inbounds [3 x float], [3 x float]* %22, i32 0, i32 2
  %93 = getelementptr inbounds [2 x i32], [2 x i32]* %nodesToProcess.i.i, i32 0, i32 1
  %94 = getelementptr inbounds [3 x float], [3 x float]* %21, i32 0, i32 0
  %95 = getelementptr inbounds [3 x float], [3 x float]* %21, i32 0, i32 1
  %96 = getelementptr inbounds [3 x float], [3 x float]* %21, i32 0, i32 2
  %97 = getelementptr inbounds [3 x float], [3 x float]* %19, i32 0, i32 0
  %98 = getelementptr inbounds [3 x float], [3 x float]* %19, i32 0, i32 1
  %99 = getelementptr inbounds [3 x float], [3 x float]* %19, i32 0, i32 2
  %100 = getelementptr inbounds [3 x float], [3 x float]* %20, i32 0, i32 0
  %101 = getelementptr inbounds [3 x float], [3 x float]* %20, i32 0, i32 1
  %102 = getelementptr inbounds [3 x float], [3 x float]* %20, i32 0, i32 2
  %103 = getelementptr inbounds %struct.BuiltInTriangleIntersectionAttributes, %struct.BuiltInTriangleIntersectionAttributes* %attr.i.i, i32 0, i32 0
  br label %.lr.ph

; <label>:104                                     ; preds = %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i"
  call void @"\01?Fallback_SetInstanceIndex@@YAXI@Z"(i32 -1) #3
  br label %._crit_edge

.lr.ph:                                           ; preds = %447, %.lr.ph.preheader.critedge
  %resultTriId.i.i.0188 = phi i32 [ %resultTriId.i.i.2, %447 ], [ undef, %.lr.ph.preheader.critedge ]
  %resultBary.i.i.0187.i0 = phi float [ %resultBary.i.i.2.i0, %447 ], [ undef, %.lr.ph.preheader.critedge ]
  %resultBary.i.i.0187.i1 = phi float [ %resultBary.i.i.2.i1, %447 ], [ undef, %.lr.ph.preheader.critedge ]
  %stackPointer.i.i.1185 = phi i32 [ %stackPointer.i.i.3, %447 ], [ 1, %.lr.ph.preheader.critedge ]
  %instId.i.i.0184 = phi i32 [ %instId.i.i.2, %447 ], [ 0, %.lr.ph.preheader.critedge ]
  %instOffset.i.i.0183 = phi i32 [ %instOffset.i.i.2, %447 ], [ 0, %.lr.ph.preheader.critedge ]
  %instFlags.i.i.0182 = phi i32 [ %instFlags.i.i.2, %447 ], [ 0, %.lr.ph.preheader.critedge ]
  %instIdx.i.i.0181 = phi i32 [ %instIdx.i.i.2, %447 ], [ 0, %.lr.ph.preheader.critedge ]
  %currentGpuVA.i.i.0180.i0 = phi i32 [ %450, %447 ], [ %56, %.lr.ph.preheader.critedge ]
  %currentGpuVA.i.i.0180.i1 = phi i32 [ %451, %447 ], [ %57, %.lr.ph.preheader.critedge ]
  %currentBVHIndex.i.i.0179 = phi i32 [ %448, %447 ], [ 0, %.lr.ph.preheader.critedge ]
  %CurrentWorldToObject.i.i168.0178.i0 = phi float [ %CurrentWorldToObject.i.i168.2.i0, %447 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentWorldToObject.i.i168.0178.i1 = phi float [ %CurrentWorldToObject.i.i168.2.i1, %447 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentWorldToObject.i.i168.0178.i2 = phi float [ %CurrentWorldToObject.i.i168.2.i2, %447 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentWorldToObject.i.i168.0178.i3 = phi float [ %CurrentWorldToObject.i.i168.2.i3, %447 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentWorldToObject.i.i168.0178.i4 = phi float [ %CurrentWorldToObject.i.i168.2.i4, %447 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentWorldToObject.i.i168.0178.i5 = phi float [ %CurrentWorldToObject.i.i168.2.i5, %447 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentWorldToObject.i.i168.0178.i6 = phi float [ %CurrentWorldToObject.i.i168.2.i6, %447 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentWorldToObject.i.i168.0178.i7 = phi float [ %CurrentWorldToObject.i.i168.2.i7, %447 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentWorldToObject.i.i168.0178.i8 = phi float [ %CurrentWorldToObject.i.i168.2.i8, %447 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentWorldToObject.i.i168.0178.i9 = phi float [ %CurrentWorldToObject.i.i168.2.i9, %447 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentWorldToObject.i.i168.0178.i10 = phi float [ %CurrentWorldToObject.i.i168.2.i10, %447 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentWorldToObject.i.i168.0178.i11 = phi float [ %CurrentWorldToObject.i.i168.2.i11, %447 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentObjectToWorld.i.i167.0177.i0 = phi float [ %CurrentObjectToWorld.i.i167.2.i0, %447 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentObjectToWorld.i.i167.0177.i1 = phi float [ %CurrentObjectToWorld.i.i167.2.i1, %447 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentObjectToWorld.i.i167.0177.i2 = phi float [ %CurrentObjectToWorld.i.i167.2.i2, %447 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentObjectToWorld.i.i167.0177.i3 = phi float [ %CurrentObjectToWorld.i.i167.2.i3, %447 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentObjectToWorld.i.i167.0177.i4 = phi float [ %CurrentObjectToWorld.i.i167.2.i4, %447 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentObjectToWorld.i.i167.0177.i5 = phi float [ %CurrentObjectToWorld.i.i167.2.i5, %447 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentObjectToWorld.i.i167.0177.i6 = phi float [ %CurrentObjectToWorld.i.i167.2.i6, %447 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentObjectToWorld.i.i167.0177.i7 = phi float [ %CurrentObjectToWorld.i.i167.2.i7, %447 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentObjectToWorld.i.i167.0177.i8 = phi float [ %CurrentObjectToWorld.i.i167.2.i8, %447 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentObjectToWorld.i.i167.0177.i9 = phi float [ %CurrentObjectToWorld.i.i167.2.i9, %447 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentObjectToWorld.i.i167.0177.i10 = phi float [ %CurrentObjectToWorld.i.i167.2.i10, %447 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %CurrentObjectToWorld.i.i167.0177.i11 = phi float [ %CurrentObjectToWorld.i.i167.2.i11, %447 ], [ 0.000000e+00, %.lr.ph.preheader.critedge ]
  %.0174176.i0 = phi float [ %.3.i0, %447 ], [ undef, %.lr.ph.preheader.critedge ]
  %.0174176.i1 = phi float [ %.3.i1, %447 ], [ undef, %.lr.ph.preheader.critedge ]
  %.phi.trans.insert = getelementptr inbounds [2 x i32], [2 x i32]* %nodesToProcess.i.i, i32 0, i32 %currentBVHIndex.i.i.0179
  %.pre = load i32, i32* %.phi.trans.insert, align 4, !tbaa !221
  br label %105

; <label>:105                                     ; preds = %443, %.lr.ph
  %106 = phi i32 [ %.pre, %.lr.ph ], [ %445, %443 ]
  %.1.i0 = phi float [ %.0174176.i0, %.lr.ph ], [ %.3.i0, %443 ]
  %.1.i1 = phi float [ %.0174176.i1, %.lr.ph ], [ %.3.i1, %443 ]
  %currentRayData.i.i.0.1.i0 = phi float [ %WorldRayOrigin248, %.lr.ph ], [ %currentRayData.i.i.0.2.i0, %443 ]
  %currentRayData.i.i.0.1.i1 = phi float [ %WorldRayOrigin249, %.lr.ph ], [ %currentRayData.i.i.0.2.i1, %443 ]
  %currentRayData.i.i.0.1.i2 = phi float [ %WorldRayOrigin250, %.lr.ph ], [ %currentRayData.i.i.0.2.i2, %443 ]
  %currentRayData.i.i.1.1.i0 = phi float [ %WorldRayDirection251, %.lr.ph ], [ %currentRayData.i.i.1.2.i0, %443 ]
  %currentRayData.i.i.1.1.i1 = phi float [ %WorldRayDirection252, %.lr.ph ], [ %currentRayData.i.i.1.2.i1, %443 ]
  %currentRayData.i.i.1.1.i2 = phi float [ %WorldRayDirection253, %.lr.ph ], [ %currentRayData.i.i.1.2.i2, %443 ]
  %currentRayData.i.i.2.1.i0 = phi float [ %.i0, %.lr.ph ], [ %currentRayData.i.i.2.2.i0, %443 ]
  %currentRayData.i.i.2.1.i1 = phi float [ %.i1, %.lr.ph ], [ %currentRayData.i.i.2.2.i1, %443 ]
  %currentRayData.i.i.2.1.i2 = phi float [ %.i2, %.lr.ph ], [ %currentRayData.i.i.2.2.i2, %443 ]
  %currentRayData.i.i.3.1.i0 = phi float [ %.i0296, %.lr.ph ], [ %currentRayData.i.i.3.2.i0, %443 ]
  %currentRayData.i.i.3.1.i1 = phi float [ %.i1297, %.lr.ph ], [ %currentRayData.i.i.3.2.i1, %443 ]
  %currentRayData.i.i.3.1.i2 = phi float [ %.i2298, %.lr.ph ], [ %currentRayData.i.i.3.2.i2, %443 ]
  %currentRayData.i.i.4.1.i0 = phi float [ %50, %.lr.ph ], [ %currentRayData.i.i.4.2.i0, %443 ]
  %currentRayData.i.i.4.1.i1 = phi float [ %53, %.lr.ph ], [ %currentRayData.i.i.4.2.i1, %443 ]
  %currentRayData.i.i.4.1.i2 = phi float [ %54, %.lr.ph ], [ %currentRayData.i.i.4.2.i2, %443 ]
  %currentRayData.i.i.5.1.i0 = phi i32 [ %worldRayData.i.i.5.0.i0, %.lr.ph ], [ %currentRayData.i.i.5.2.i0, %443 ]
  %currentRayData.i.i.5.1.i1 = phi i32 [ %worldRayData.i.i.5.0.i1, %.lr.ph ], [ %currentRayData.i.i.5.2.i1, %443 ]
  %currentRayData.i.i.5.1.i2 = phi i32 [ %.0171, %.lr.ph ], [ %currentRayData.i.i.5.2.i2, %443 ]
  %CurrentObjectToWorld.i.i167.1.i0 = phi float [ %CurrentObjectToWorld.i.i167.0177.i0, %.lr.ph ], [ %CurrentObjectToWorld.i.i167.2.i0, %443 ]
  %CurrentObjectToWorld.i.i167.1.i1 = phi float [ %CurrentObjectToWorld.i.i167.0177.i1, %.lr.ph ], [ %CurrentObjectToWorld.i.i167.2.i1, %443 ]
  %CurrentObjectToWorld.i.i167.1.i2 = phi float [ %CurrentObjectToWorld.i.i167.0177.i2, %.lr.ph ], [ %CurrentObjectToWorld.i.i167.2.i2, %443 ]
  %CurrentObjectToWorld.i.i167.1.i3 = phi float [ %CurrentObjectToWorld.i.i167.0177.i3, %.lr.ph ], [ %CurrentObjectToWorld.i.i167.2.i3, %443 ]
  %CurrentObjectToWorld.i.i167.1.i4 = phi float [ %CurrentObjectToWorld.i.i167.0177.i4, %.lr.ph ], [ %CurrentObjectToWorld.i.i167.2.i4, %443 ]
  %CurrentObjectToWorld.i.i167.1.i5 = phi float [ %CurrentObjectToWorld.i.i167.0177.i5, %.lr.ph ], [ %CurrentObjectToWorld.i.i167.2.i5, %443 ]
  %CurrentObjectToWorld.i.i167.1.i6 = phi float [ %CurrentObjectToWorld.i.i167.0177.i6, %.lr.ph ], [ %CurrentObjectToWorld.i.i167.2.i6, %443 ]
  %CurrentObjectToWorld.i.i167.1.i7 = phi float [ %CurrentObjectToWorld.i.i167.0177.i7, %.lr.ph ], [ %CurrentObjectToWorld.i.i167.2.i7, %443 ]
  %CurrentObjectToWorld.i.i167.1.i8 = phi float [ %CurrentObjectToWorld.i.i167.0177.i8, %.lr.ph ], [ %CurrentObjectToWorld.i.i167.2.i8, %443 ]
  %CurrentObjectToWorld.i.i167.1.i9 = phi float [ %CurrentObjectToWorld.i.i167.0177.i9, %.lr.ph ], [ %CurrentObjectToWorld.i.i167.2.i9, %443 ]
  %CurrentObjectToWorld.i.i167.1.i10 = phi float [ %CurrentObjectToWorld.i.i167.0177.i10, %.lr.ph ], [ %CurrentObjectToWorld.i.i167.2.i10, %443 ]
  %CurrentObjectToWorld.i.i167.1.i11 = phi float [ %CurrentObjectToWorld.i.i167.0177.i11, %.lr.ph ], [ %CurrentObjectToWorld.i.i167.2.i11, %443 ]
  %CurrentWorldToObject.i.i168.1.i0 = phi float [ %CurrentWorldToObject.i.i168.0178.i0, %.lr.ph ], [ %CurrentWorldToObject.i.i168.2.i0, %443 ]
  %CurrentWorldToObject.i.i168.1.i1 = phi float [ %CurrentWorldToObject.i.i168.0178.i1, %.lr.ph ], [ %CurrentWorldToObject.i.i168.2.i1, %443 ]
  %CurrentWorldToObject.i.i168.1.i2 = phi float [ %CurrentWorldToObject.i.i168.0178.i2, %.lr.ph ], [ %CurrentWorldToObject.i.i168.2.i2, %443 ]
  %CurrentWorldToObject.i.i168.1.i3 = phi float [ %CurrentWorldToObject.i.i168.0178.i3, %.lr.ph ], [ %CurrentWorldToObject.i.i168.2.i3, %443 ]
  %CurrentWorldToObject.i.i168.1.i4 = phi float [ %CurrentWorldToObject.i.i168.0178.i4, %.lr.ph ], [ %CurrentWorldToObject.i.i168.2.i4, %443 ]
  %CurrentWorldToObject.i.i168.1.i5 = phi float [ %CurrentWorldToObject.i.i168.0178.i5, %.lr.ph ], [ %CurrentWorldToObject.i.i168.2.i5, %443 ]
  %CurrentWorldToObject.i.i168.1.i6 = phi float [ %CurrentWorldToObject.i.i168.0178.i6, %.lr.ph ], [ %CurrentWorldToObject.i.i168.2.i6, %443 ]
  %CurrentWorldToObject.i.i168.1.i7 = phi float [ %CurrentWorldToObject.i.i168.0178.i7, %.lr.ph ], [ %CurrentWorldToObject.i.i168.2.i7, %443 ]
  %CurrentWorldToObject.i.i168.1.i8 = phi float [ %CurrentWorldToObject.i.i168.0178.i8, %.lr.ph ], [ %CurrentWorldToObject.i.i168.2.i8, %443 ]
  %CurrentWorldToObject.i.i168.1.i9 = phi float [ %CurrentWorldToObject.i.i168.0178.i9, %.lr.ph ], [ %CurrentWorldToObject.i.i168.2.i9, %443 ]
  %CurrentWorldToObject.i.i168.1.i10 = phi float [ %CurrentWorldToObject.i.i168.0178.i10, %.lr.ph ], [ %CurrentWorldToObject.i.i168.2.i10, %443 ]
  %CurrentWorldToObject.i.i168.1.i11 = phi float [ %CurrentWorldToObject.i.i168.0178.i11, %.lr.ph ], [ %CurrentWorldToObject.i.i168.2.i11, %443 ]
  %ResetMatrices.i.i.1 = phi i32 [ 1, %.lr.ph ], [ %ResetMatrices.i.i.3, %443 ]
  %currentBVHIndex.i.i.1 = phi i32 [ %currentBVHIndex.i.i.0179, %.lr.ph ], [ %currentBVHIndex.i.i.2, %443 ]
  %currentGpuVA.i.i.1.i0 = phi i32 [ %currentGpuVA.i.i.0180.i0, %.lr.ph ], [ %currentGpuVA.i.i.2.i0, %443 ]
  %currentGpuVA.i.i.1.i1 = phi i32 [ %currentGpuVA.i.i.0180.i1, %.lr.ph ], [ %currentGpuVA.i.i.2.i1, %443 ]
  %instIdx.i.i.1 = phi i32 [ %instIdx.i.i.0181, %.lr.ph ], [ %instIdx.i.i.2, %443 ]
  %instFlags.i.i.1 = phi i32 [ %instFlags.i.i.0182, %.lr.ph ], [ %instFlags.i.i.2, %443 ]
  %instOffset.i.i.1 = phi i32 [ %instOffset.i.i.0183, %.lr.ph ], [ %instOffset.i.i.2, %443 ]
  %instId.i.i.1 = phi i32 [ %instId.i.i.0184, %.lr.ph ], [ %instId.i.i.2, %443 ]
  %stackPointer.i.i.2 = phi i32 [ %stackPointer.i.i.1185, %.lr.ph ], [ %stackPointer.i.i.3, %443 ]
  %resultBary.i.i.1.i0 = phi float [ %resultBary.i.i.0187.i0, %.lr.ph ], [ %resultBary.i.i.2.i0, %443 ]
  %resultBary.i.i.1.i1 = phi float [ %resultBary.i.i.0187.i1, %.lr.ph ], [ %resultBary.i.i.2.i1, %443 ]
  %resultTriId.i.i.1 = phi i32 [ %resultTriId.i.i.0188, %.lr.ph ], [ %resultTriId.i.i.2, %443 ]
  %currentRayData.i.i.1.1.upto0 = insertelement <3 x float> undef, float %currentRayData.i.i.1.1.i0, i32 0
  %currentRayData.i.i.1.1.upto1 = insertelement <3 x float> %currentRayData.i.i.1.1.upto0, float %currentRayData.i.i.1.1.i1, i32 1
  %currentRayData.i.i.1.1 = insertelement <3 x float> %currentRayData.i.i.1.1.upto1, float %currentRayData.i.i.1.1.i2, i32 2
  %currentRayData.i.i.0.1.upto0 = insertelement <3 x float> undef, float %currentRayData.i.i.0.1.i0, i32 0
  %currentRayData.i.i.0.1.upto1 = insertelement <3 x float> %currentRayData.i.i.0.1.upto0, float %currentRayData.i.i.0.1.i1, i32 1
  %currentRayData.i.i.0.1 = insertelement <3 x float> %currentRayData.i.i.0.1.upto1, float %currentRayData.i.i.0.1.i2, i32 2
  %107 = add nsw i32 %stackPointer.i.i.2, -1
  %108 = shl i32 %107, 6
  %109 = add i32 %108, %31
  %110 = getelementptr [2048 x i32], [2048 x i32] addrspace(3)* @"\01?stack@@3PAIA", i32 0, i32 %109
  %111 = load i32, i32 addrspace(3)* %110, align 4, !tbaa !221, !noalias !232
  %112 = getelementptr inbounds [2 x i32], [2 x i32]* %nodesToProcess.i.i, i32 0, i32 %currentBVHIndex.i.i.1
  %113 = add i32 %106, -1
  store i32 %113, i32* %112, align 4, !tbaa !221
  %114 = getelementptr inbounds [0 x %struct.RWByteAddressBuffer], [0 x %struct.RWByteAddressBuffer]* @"\01?DescriptorHeapBufferTable@@3PAURWByteAddressBuffer@@A", i32 0, i32 %currentGpuVA.i.i.1.i1, !dx.nonuniform !228
  %115 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %114, align 4, !noalias !235
  %116 = add i32 %currentGpuVA.i.i.1.i0, 16
  %117 = shl i32 %111, 5
  %118 = add i32 %116, %117
  %119 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %115)
  %120 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %119, i32 %118, i32 undef)
  %121 = extractvalue %dx.types.ResRet.i32 %120, 3
  %122 = add i32 %118, 16
  %123 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %119, i32 %122, i32 undef)
  %124 = extractvalue %dx.types.ResRet.i32 %123, 3
  %125 = icmp slt i32 %121, 0
  br i1 %125, label %126, label %391

; <label>:126                                     ; preds = %105
  %127 = icmp eq i32 %currentBVHIndex.i.i.1, 0
  br i1 %127, label %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189", label %"\01?IsOpaque@@YA_N_NII@Z.exit.i.i"

"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189": ; preds = %126
  %128 = and i32 %121, 2147483647
  %129 = mul i32 %128, 112
  %130 = add i32 %65, %129
  %131 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %60)
  %132 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %131, i32 %130, i32 undef)
  %133 = extractvalue %dx.types.ResRet.i32 %132, 0
  %134 = extractvalue %dx.types.ResRet.i32 %132, 1
  %135 = extractvalue %dx.types.ResRet.i32 %132, 2
  %136 = extractvalue %dx.types.ResRet.i32 %132, 3
  %137 = add i32 %130, 16
  %138 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %131, i32 %137, i32 undef)
  %139 = extractvalue %dx.types.ResRet.i32 %138, 0
  %140 = extractvalue %dx.types.ResRet.i32 %138, 1
  %141 = extractvalue %dx.types.ResRet.i32 %138, 2
  %142 = extractvalue %dx.types.ResRet.i32 %138, 3
  %143 = add i32 %130, 32
  %144 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %131, i32 %143, i32 undef)
  %145 = extractvalue %dx.types.ResRet.i32 %144, 0
  %146 = extractvalue %dx.types.ResRet.i32 %144, 1
  %147 = extractvalue %dx.types.ResRet.i32 %144, 2
  %148 = extractvalue %dx.types.ResRet.i32 %144, 3
  %149 = add i32 %130, 48
  %150 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %131, i32 %149, i32 undef)
  %151 = extractvalue %dx.types.ResRet.i32 %150, 0
  %152 = extractvalue %dx.types.ResRet.i32 %150, 1
  %153 = extractvalue %dx.types.ResRet.i32 %150, 2
  %154 = extractvalue %dx.types.ResRet.i32 %150, 3
  %155 = add i32 %130, 64
  %156 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %131, i32 %155, i32 undef)
  %157 = extractvalue %dx.types.ResRet.i32 %156, 0
  %158 = extractvalue %dx.types.ResRet.i32 %156, 1
  %159 = extractvalue %dx.types.ResRet.i32 %156, 2
  %160 = extractvalue %dx.types.ResRet.i32 %156, 3
  %161 = add i32 %130, 80
  %162 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %131, i32 %161, i32 undef)
  %163 = extractvalue %dx.types.ResRet.i32 %162, 0
  %164 = extractvalue %dx.types.ResRet.i32 %162, 1
  %165 = extractvalue %dx.types.ResRet.i32 %162, 2
  %166 = extractvalue %dx.types.ResRet.i32 %162, 3
  %167 = add i32 %130, 96
  %168 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %131, i32 %167, i32 undef)
  %169 = extractvalue %dx.types.ResRet.i32 %168, 0
  %170 = extractvalue %dx.types.ResRet.i32 %168, 1
  %171 = extractvalue %dx.types.ResRet.i32 %168, 2
  %172 = extractvalue %dx.types.ResRet.i32 %168, 3
  %173 = bitcast i32 %133 to float
  %174 = bitcast i32 %134 to float
  %175 = bitcast i32 %135 to float
  %176 = bitcast i32 %136 to float
  %177 = bitcast i32 %139 to float
  %178 = bitcast i32 %140 to float
  %179 = bitcast i32 %141 to float
  %180 = bitcast i32 %142 to float
  %181 = bitcast i32 %145 to float
  %182 = bitcast i32 %146 to float
  %183 = bitcast i32 %147 to float
  %184 = bitcast i32 %148 to float
  %185 = and i32 %152, 16777215
  %186 = and i32 %151, 16777215
  %187 = lshr i32 %151, 24
  %188 = and i32 %187, %1
  %189 = icmp eq i32 %188, 0
  %190 = bitcast i32 %157 to float
  %191 = bitcast i32 %158 to float
  %192 = bitcast i32 %159 to float
  %193 = bitcast i32 %160 to float
  %194 = bitcast i32 %163 to float
  %195 = bitcast i32 %164 to float
  %196 = bitcast i32 %165 to float
  %197 = bitcast i32 %166 to float
  %198 = bitcast i32 %169 to float
  %199 = bitcast i32 %170 to float
  %200 = bitcast i32 %171 to float
  %201 = bitcast i32 %172 to float
  br i1 %189, label %443, label %202

; <label>:202                                     ; preds = %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189"
  store i32 0, i32 addrspace(3)* %110, align 4, !tbaa !221, !noalias !240
  %203 = lshr i32 %152, 24
  %204 = fmul fast float %WorldRayDirection251, %173
  %FMad285 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayDirection252, float %174, float %204)
  %FMad284 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayDirection253, float %175, float %FMad285)
  %FMad283 = call float @dx.op.tertiary.f32(i32 46, float 0.000000e+00, float %176, float %FMad284)
  %205 = fmul fast float %WorldRayDirection251, %177
  %FMad282 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayDirection252, float %178, float %205)
  %FMad281 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayDirection253, float %179, float %FMad282)
  %FMad280 = call float @dx.op.tertiary.f32(i32 46, float 0.000000e+00, float %180, float %FMad281)
  %206 = fmul fast float %WorldRayDirection251, %181
  %FMad279 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayDirection252, float %182, float %206)
  %FMad278 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayDirection253, float %183, float %FMad279)
  %FMad = call float @dx.op.tertiary.f32(i32 46, float 0.000000e+00, float %184, float %FMad278)
  %207 = fmul fast float %WorldRayOrigin248, %173
  %FMad294 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayOrigin249, float %174, float %207)
  %FMad293 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayOrigin250, float %175, float %FMad294)
  %FMad292 = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %176, float %FMad293)
  %208 = fmul fast float %WorldRayOrigin248, %177
  %FMad291 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayOrigin249, float %178, float %208)
  %FMad290 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayOrigin250, float %179, float %FMad291)
  %FMad289 = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %180, float %FMad290)
  %209 = fmul fast float %WorldRayOrigin248, %181
  %FMad288 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayOrigin249, float %182, float %209)
  %FMad287 = call float @dx.op.tertiary.f32(i32 46, float %WorldRayOrigin250, float %183, float %FMad288)
  %FMad286 = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %184, float %FMad287)
  store float %FMad283, float* %90, align 4
  store float %FMad280, float* %91, align 4
  store float %FMad, float* %92, align 4
  %.i0340 = fdiv fast float 1.000000e+00, %FMad283
  %.i1341 = fdiv fast float 1.000000e+00, %FMad280
  %.i2342 = fdiv fast float 1.000000e+00, %FMad
  %.i0343 = fmul fast float %.i0340, %FMad292
  %.i1344 = fmul fast float %.i1341, %FMad289
  %.i2345 = fmul fast float %.i2342, %FMad286
  %FAbs = call float @dx.op.unary.f32(i32 6, float %FMad283)
  %FAbs221 = call float @dx.op.unary.f32(i32 6, float %FMad280)
  %FAbs222 = call float @dx.op.unary.f32(i32 6, float %FMad)
  %210 = fcmp fast ogt float %FAbs, %FAbs221
  %211 = fcmp fast ogt float %FAbs, %FAbs222
  %212 = and i1 %210, %211
  br i1 %212, label %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i", label %213

; <label>:213                                     ; preds = %202
  %214 = fcmp fast ogt float %FAbs221, %FAbs222
  br i1 %214, label %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i", label %215

; <label>:215                                     ; preds = %213
  br label %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i"

"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i": ; preds = %215, %213, %202
  %.0172 = phi i32 [ 2, %215 ], [ 0, %202 ], [ 1, %213 ]
  %216 = add nuw nsw i32 %.0172, 1
  %217 = urem i32 %216, 3
  %218 = add nuw nsw i32 %.0172, 2
  %219 = urem i32 %218, 3
  %220 = getelementptr [3 x float], [3 x float]* %22, i32 0, i32 %.0172
  %221 = load float, float* %220, align 4, !tbaa !223, !noalias !243
  %222 = fcmp fast olt float %221, 0.000000e+00
  %.5.0.i0 = select i1 %222, i32 %219, i32 %217
  %.5.0.i1 = select i1 %222, i32 %217, i32 %219
  %223 = getelementptr [3 x float], [3 x float]* %22, i32 0, i32 %.5.0.i0
  %224 = load float, float* %223, align 4, !tbaa !223, !noalias !243
  %225 = fdiv float %224, %221
  %226 = getelementptr [3 x float], [3 x float]* %22, i32 0, i32 %.5.0.i1
  %227 = load float, float* %226, align 4, !tbaa !223, !noalias !243
  %228 = fdiv float %227, %221
  %229 = fdiv float 1.000000e+00, %221
  store i32 1, i32* %93, align 4, !tbaa !221
  br label %443

"\01?IsOpaque@@YA_N_NII@Z.exit.i.i":              ; preds = %126
  %RayFlags256 = call i32 @dx.op.rayFlags.i32(i32 144)
  %230 = and i32 %RayFlags256, 64
  %RayTCurrent = call float @dx.op.rayTCurrent.f32(i32 154)
  %231 = icmp eq i32 %230, 0
  %232 = and i32 %121, 16777215
  %233 = add i32 %currentGpuVA.i.i.1.i0, 4
  %234 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %115)
  %235 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %234, i32 %233, i32 undef)
  %236 = extractvalue %dx.types.ResRet.i32 %235, 0
  %237 = mul nuw nsw i32 %232, 36
  %238 = add i32 %237, %currentGpuVA.i.i.1.i0
  %239 = add i32 %238, %236
  %240 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %234, i32 %239, i32 undef)
  %241 = extractvalue %dx.types.ResRet.i32 %240, 0
  %242 = extractvalue %dx.types.ResRet.i32 %240, 1
  %243 = extractvalue %dx.types.ResRet.i32 %240, 2
  %244 = extractvalue %dx.types.ResRet.i32 %240, 3
  %.i0346 = bitcast i32 %241 to float
  %.i1347 = bitcast i32 %242 to float
  %.i2348 = bitcast i32 %243 to float
  %.i3349 = bitcast i32 %244 to float
  %245 = add i32 %239, 16
  %246 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %234, i32 %245, i32 undef)
  %247 = extractvalue %dx.types.ResRet.i32 %246, 0
  %248 = extractvalue %dx.types.ResRet.i32 %246, 1
  %249 = extractvalue %dx.types.ResRet.i32 %246, 2
  %250 = extractvalue %dx.types.ResRet.i32 %246, 3
  %.i0350 = bitcast i32 %247 to float
  %.i1351 = bitcast i32 %248 to float
  %.i2352 = bitcast i32 %249 to float
  %.i3353 = bitcast i32 %250 to float
  %251 = add i32 %239, 32
  %252 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %234, i32 %251, i32 undef)
  %253 = extractvalue %dx.types.ResRet.i32 %252, 0
  %254 = bitcast i32 %253 to float
  %255 = and i32 %instFlags.i.i.1, 1
  %256 = icmp eq i32 %255, 0
  %257 = shl i32 %instFlags.i.i.1, 3
  %258 = and i32 %257, 16
  %259 = add nuw nsw i32 %258, 16
  %260 = xor i32 %258, 16
  %261 = add nuw nsw i32 %260, 16
  %262 = and i32 %RayFlags256, %259
  %263 = icmp ne i32 %262, 0
  %264 = and i1 %256, %263
  %265 = and i32 %RayFlags256, %261
  %266 = icmp ne i32 %265, 0
  %267 = and i1 %256, %266
  %.i0354 = fsub fast float %.i0346, %currentRayData.i.i.0.1.i0
  %.i1355 = fsub fast float %.i1347, %currentRayData.i.i.0.1.i1
  %.i2356 = fsub fast float %.i2348, %currentRayData.i.i.0.1.i2
  store float %.i0354, float* %94, align 4
  store float %.i1355, float* %95, align 4
  store float %.i2356, float* %96, align 4
  %268 = getelementptr [3 x float], [3 x float]* %21, i32 0, i32 %currentRayData.i.i.5.1.i0
  %269 = load float, float* %268, align 4, !tbaa !223, !noalias !246
  %270 = getelementptr [3 x float], [3 x float]* %21, i32 0, i32 %currentRayData.i.i.5.1.i1
  %271 = load float, float* %270, align 4, !tbaa !223, !noalias !246
  %272 = getelementptr [3 x float], [3 x float]* %21, i32 0, i32 %currentRayData.i.i.5.1.i2
  %273 = load float, float* %272, align 4, !tbaa !223, !noalias !246
  %.i0357 = fsub fast float %.i3349, %currentRayData.i.i.0.1.i0
  %.i1358 = fsub fast float %.i0350, %currentRayData.i.i.0.1.i1
  %.i2359 = fsub fast float %.i1351, %currentRayData.i.i.0.1.i2
  store float %.i0357, float* %97, align 4
  store float %.i1358, float* %98, align 4
  store float %.i2359, float* %99, align 4
  %274 = getelementptr [3 x float], [3 x float]* %19, i32 0, i32 %currentRayData.i.i.5.1.i0
  %275 = load float, float* %274, align 4, !tbaa !223, !noalias !246
  %276 = getelementptr [3 x float], [3 x float]* %19, i32 0, i32 %currentRayData.i.i.5.1.i1
  %277 = load float, float* %276, align 4, !tbaa !223, !noalias !246
  %278 = getelementptr [3 x float], [3 x float]* %19, i32 0, i32 %currentRayData.i.i.5.1.i2
  %279 = load float, float* %278, align 4, !tbaa !223, !noalias !246
  %.i0360 = fsub fast float %.i2352, %currentRayData.i.i.0.1.i0
  %.i1361 = fsub fast float %.i3353, %currentRayData.i.i.0.1.i1
  %.i2362 = fsub fast float %254, %currentRayData.i.i.0.1.i2
  store float %.i0360, float* %100, align 4
  store float %.i1361, float* %101, align 4
  store float %.i2362, float* %102, align 4
  %280 = getelementptr [3 x float], [3 x float]* %20, i32 0, i32 %currentRayData.i.i.5.1.i0
  %281 = load float, float* %280, align 4, !tbaa !223, !noalias !246
  %282 = getelementptr [3 x float], [3 x float]* %20, i32 0, i32 %currentRayData.i.i.5.1.i1
  %283 = load float, float* %282, align 4, !tbaa !223, !noalias !246
  %284 = getelementptr [3 x float], [3 x float]* %20, i32 0, i32 %currentRayData.i.i.5.1.i2
  %285 = load float, float* %284, align 4, !tbaa !223, !noalias !246
  %.i0363 = fmul float %currentRayData.i.i.4.1.i0, %273
  %.i1364 = fmul float %currentRayData.i.i.4.1.i1, %273
  %.i0366 = fsub float %269, %.i0363
  %.i1367 = fsub float %271, %.i1364
  %.i0368 = fmul float %currentRayData.i.i.4.1.i0, %279
  %.i1369 = fmul float %currentRayData.i.i.4.1.i1, %279
  %.i0371 = fsub float %275, %.i0368
  %.i1372 = fsub float %277, %.i1369
  %.i0373 = fmul float %currentRayData.i.i.4.1.i0, %285
  %.i1374 = fmul float %currentRayData.i.i.4.1.i1, %285
  %.i0376 = fsub float %281, %.i0373
  %.i1377 = fsub float %283, %.i1374
  %286 = fmul float %.i1372, %.i0376
  %287 = fmul float %.i0371, %.i1377
  %288 = fsub float %286, %287
  %289 = fmul float %.i0366, %.i1377
  %290 = fmul float %.i1367, %.i0376
  %291 = fsub float %289, %290
  %292 = fmul float %.i1367, %.i0371
  %293 = fmul float %.i0366, %.i1372
  %294 = fsub float %292, %293
  %295 = fadd fast float %291, %294
  %296 = fadd fast float %295, %288
  br i1 %267, label %297, label %303

; <label>:297                                     ; preds = %"\01?IsOpaque@@YA_N_NII@Z.exit.i.i"
  %298 = fcmp fast ogt float %288, 0.000000e+00
  %299 = fcmp fast ogt float %291, 0.000000e+00
  %300 = or i1 %298, %299
  %301 = fcmp fast ogt float %294, 0.000000e+00
  %302 = or i1 %301, %300
  %.old.old = fcmp fast oeq float %296, 0.000000e+00
  %or.cond195 = or i1 %302, %.old.old
  br i1 %or.cond195, label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i", label %323

; <label>:303                                     ; preds = %"\01?IsOpaque@@YA_N_NII@Z.exit.i.i"
  br i1 %264, label %304, label %310

; <label>:304                                     ; preds = %303
  %305 = fcmp fast olt float %288, 0.000000e+00
  %306 = fcmp fast olt float %291, 0.000000e+00
  %307 = or i1 %305, %306
  %308 = fcmp fast olt float %294, 0.000000e+00
  %309 = or i1 %308, %307
  %.old = fcmp fast oeq float %296, 0.000000e+00
  %or.cond194 = or i1 %309, %.old
  br i1 %or.cond194, label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i", label %323

; <label>:310                                     ; preds = %303
  %311 = fcmp fast olt float %288, 0.000000e+00
  %312 = fcmp fast olt float %291, 0.000000e+00
  %313 = or i1 %311, %312
  %314 = fcmp fast olt float %294, 0.000000e+00
  %315 = or i1 %314, %313
  %316 = fcmp fast ogt float %288, 0.000000e+00
  %317 = fcmp fast ogt float %291, 0.000000e+00
  %318 = or i1 %316, %317
  %319 = fcmp fast ogt float %294, 0.000000e+00
  %320 = or i1 %319, %318
  %321 = and i1 %315, %320
  %322 = fcmp fast oeq float %296, 0.000000e+00
  %or.cond193 = or i1 %322, %321
  br i1 %or.cond193, label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i", label %323

; <label>:323                                     ; preds = %310, %304, %297
  %324 = fmul fast float %288, %273
  %325 = fmul fast float %291, %279
  %326 = fmul fast float %294, %285
  %tmp = fadd fast float %325, %326
  %tmp517 = fadd fast float %tmp, %324
  %tmp518 = fmul fast float %tmp517, %currentRayData.i.i.4.1.i2
  br i1 %267, label %327, label %332

; <label>:327                                     ; preds = %323
  %328 = fcmp fast ogt float %tmp518, 0.000000e+00
  %329 = fmul fast float %296, %RayTCurrent
  %330 = fcmp fast olt float %tmp518, %329
  %331 = or i1 %328, %330
  br i1 %331, label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i", label %349

; <label>:332                                     ; preds = %323
  br i1 %264, label %333, label %338

; <label>:333                                     ; preds = %332
  %334 = fcmp fast olt float %tmp518, 0.000000e+00
  %335 = fmul fast float %296, %RayTCurrent
  %336 = fcmp fast ogt float %tmp518, %335
  %337 = or i1 %334, %336
  br i1 %337, label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i", label %349

; <label>:338                                     ; preds = %332
  %339 = fcmp fast ogt float %296, 0.000000e+00
  %340 = select i1 %339, i32 1, i32 -1
  %341 = bitcast float %tmp518 to i32
  %342 = xor i32 %341, %340
  %343 = uitofp i32 %342 to float
  %344 = bitcast float %296 to i32
  %345 = xor i32 %340, %344
  %346 = uitofp i32 %345 to float
  %347 = fmul fast float %346, %RayTCurrent
  %348 = fcmp fast ogt float %343, %347
  br i1 %348, label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i", label %349

; <label>:349                                     ; preds = %338, %333, %327
  %350 = fdiv fast float 1.000000e+00, %296
  %351 = fmul fast float %350, %291
  %352 = fmul fast float %350, %294
  %353 = fmul fast float %350, %tmp518
  br label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i"

"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i": ; preds = %349, %338, %333, %327, %310, %304, %297
  %.2.i0 = phi float [ %.1.i0, %297 ], [ %.1.i0, %327 ], [ %351, %349 ], [ %.1.i0, %333 ], [ %.1.i0, %338 ], [ %.1.i0, %304 ], [ %.1.i0, %310 ]
  %.2.i1 = phi float [ %.1.i1, %297 ], [ %.1.i1, %327 ], [ %352, %349 ], [ %.1.i1, %333 ], [ %.1.i1, %338 ], [ %.1.i1, %304 ], [ %.1.i1, %310 ]
  %.0173 = phi float [ %RayTCurrent, %297 ], [ %RayTCurrent, %327 ], [ %353, %349 ], [ %RayTCurrent, %333 ], [ %RayTCurrent, %338 ], [ %RayTCurrent, %304 ], [ %RayTCurrent, %310 ]
  %354 = fcmp fast olt float %.0173, %RayTCurrent
  %RayTMin = call float @dx.op.rayTMin.f32(i32 153)
  %355 = fcmp fast ogt float %.0173, %RayTMin
  %356 = and i1 %354, %355
  %. = select i1 %356, i1 true, i1 false
  %.resultTriId.i.i.1 = select i1 %356, i32 %232, i32 %resultTriId.i.i.1
  %.0173.RayTCurrent = select i1 %356, float %.0173, float %RayTCurrent
  %.2.i0.resultBary.i.i.1.i0 = select i1 %356, float %.2.i0, float %resultBary.i.i.1.i0
  %.2.i1.resultBary.i.i.1.i1 = select i1 %356, float %.2.i1, float %resultBary.i.i.1.i1
  %357 = and i1 %231, %.
  br i1 %357, label %358, label %443

; <label>:358                                     ; preds = %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i"
  %.0.upto0 = insertelement <2 x float> undef, float %.2.i0.resultBary.i.i.1.i0, i32 0
  %.0 = insertelement <2 x float> %.0.upto0, float %.2.i1.resultBary.i.i.1.i1, i32 1
  %359 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %114, align 4, !noalias !254
  %360 = add i32 %currentGpuVA.i.i.1.i0, 8
  %361 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %359)
  %362 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %361, i32 %360, i32 undef)
  %363 = extractvalue %dx.types.ResRet.i32 %362, 0
  %364 = shl i32 %.resultTriId.i.i.1, 3
  %365 = add i32 %364, %currentGpuVA.i.i.1.i0
  %366 = add i32 %365, %363
  %367 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %361, i32 %366, i32 undef)
  %368 = extractvalue %dx.types.ResRet.i32 %367, 0
  %369 = extractvalue %dx.types.ResRet.i32 %367, 1
  %370 = mul i32 %368, %3
  %371 = add i32 %instOffset.i.i.1, %2
  %372 = add i32 %371, %370
  store <2 x float> %.0, <2 x float>* %103, align 4, !tbaa !218
  call void @"\01?Fallback_SetPendingAttr@@YAXUBuiltInTriangleIntersectionAttributes@@@Z"(%struct.BuiltInTriangleIntersectionAttributes* nonnull %attr.i.i) #3
  call void @"\01?Fallback_SetPendingTriVals@@YAXMIIIII@Z"(float %.0173.RayTCurrent, i32 %369, i32 %372, i32 %instIdx.i.i.1, i32 %instId.i.i.1, i32 254) #3
  %373 = icmp eq i32 %ResetMatrices.i.i.1, 0
  br i1 %373, label %387, label %374

; <label>:374                                     ; preds = %358
  call void @"\01?Fallback_SetObjectRayOrigin@@YAXV?$vector@M$02@@@Z"(<3 x float> %currentRayData.i.i.0.1) #3
  call void @"\01?Fallback_SetObjectRayDirection@@YAXV?$vector@M$02@@@Z"(<3 x float> %currentRayData.i.i.1.1) #3
  %.upto0 = insertelement <4 x float> undef, float %CurrentWorldToObject.i.i168.1.i0, i32 0
  %.upto1 = insertelement <4 x float> %.upto0, float %CurrentWorldToObject.i.i168.1.i1, i32 1
  %.upto2 = insertelement <4 x float> %.upto1, float %CurrentWorldToObject.i.i168.1.i2, i32 2
  %375 = insertelement <4 x float> %.upto2, float %CurrentWorldToObject.i.i168.1.i3, i32 3
  %376 = insertvalue %class.matrix.float.3.4 undef, <4 x float> %375, 0, 0
  %.upto01 = insertelement <4 x float> undef, float %CurrentWorldToObject.i.i168.1.i4, i32 0
  %.upto12 = insertelement <4 x float> %.upto01, float %CurrentWorldToObject.i.i168.1.i5, i32 1
  %.upto23 = insertelement <4 x float> %.upto12, float %CurrentWorldToObject.i.i168.1.i6, i32 2
  %377 = insertelement <4 x float> %.upto23, float %CurrentWorldToObject.i.i168.1.i7, i32 3
  %378 = insertvalue %class.matrix.float.3.4 %376, <4 x float> %377, 0, 1
  %.upto04 = insertelement <4 x float> undef, float %CurrentWorldToObject.i.i168.1.i8, i32 0
  %.upto15 = insertelement <4 x float> %.upto04, float %CurrentWorldToObject.i.i168.1.i9, i32 1
  %.upto26 = insertelement <4 x float> %.upto15, float %CurrentWorldToObject.i.i168.1.i10, i32 2
  %379 = insertelement <4 x float> %.upto26, float %CurrentWorldToObject.i.i168.1.i11, i32 3
  %380 = insertvalue %class.matrix.float.3.4 %378, <4 x float> %379, 0, 2
  call void @"\01?Fallback_SetWorldToObject@@YAXV?$matrix@M$02$03@@@Z"(%class.matrix.float.3.4 %380) #3
  %.upto07 = insertelement <4 x float> undef, float %CurrentObjectToWorld.i.i167.1.i0, i32 0
  %.upto18 = insertelement <4 x float> %.upto07, float %CurrentObjectToWorld.i.i167.1.i1, i32 1
  %.upto29 = insertelement <4 x float> %.upto18, float %CurrentObjectToWorld.i.i167.1.i2, i32 2
  %381 = insertelement <4 x float> %.upto29, float %CurrentObjectToWorld.i.i167.1.i3, i32 3
  %382 = insertvalue %class.matrix.float.3.4 undef, <4 x float> %381, 0, 0
  %.upto010 = insertelement <4 x float> undef, float %CurrentObjectToWorld.i.i167.1.i4, i32 0
  %.upto111 = insertelement <4 x float> %.upto010, float %CurrentObjectToWorld.i.i167.1.i5, i32 1
  %.upto212 = insertelement <4 x float> %.upto111, float %CurrentObjectToWorld.i.i167.1.i6, i32 2
  %383 = insertelement <4 x float> %.upto212, float %CurrentObjectToWorld.i.i167.1.i7, i32 3
  %384 = insertvalue %class.matrix.float.3.4 %382, <4 x float> %383, 0, 1
  %.upto013 = insertelement <4 x float> undef, float %CurrentObjectToWorld.i.i167.1.i8, i32 0
  %.upto114 = insertelement <4 x float> %.upto013, float %CurrentObjectToWorld.i.i167.1.i9, i32 1
  %.upto215 = insertelement <4 x float> %.upto114, float %CurrentObjectToWorld.i.i167.1.i10, i32 2
  %385 = insertelement <4 x float> %.upto215, float %CurrentObjectToWorld.i.i167.1.i11, i32 3
  %386 = insertvalue %class.matrix.float.3.4 %384, <4 x float> %385, 0, 2
  call void @"\01?Fallback_SetObjectToWorld@@YAXV?$matrix@M$02$03@@@Z"(%class.matrix.float.3.4 %386) #3
  br label %387

; <label>:387                                     ; preds = %374, %358
  call void @"\01?Fallback_CommitHit@@YAXXZ"() #3
  %388 = and i32 %RayFlags256, 4
  %389 = icmp eq i32 %388, 0
  br i1 %389, label %443, label %390

; <label>:390                                     ; preds = %387
  store i32 0, i32* %93, align 4, !tbaa !221
  store i32 0, i32* %58, align 4, !tbaa !221
  br label %443

; <label>:391                                     ; preds = %105
  %392 = and i32 %121, 16777215
  %RayTCurrent219 = call float @dx.op.rayTCurrent.f32(i32 154)
  %393 = shl nuw nsw i32 %392, 5
  %394 = add i32 %116, %393
  %395 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %115)
  %396 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %395, i32 %394, i32 undef)
  %397 = extractvalue %dx.types.ResRet.i32 %396, 0
  %398 = extractvalue %dx.types.ResRet.i32 %396, 1
  %399 = extractvalue %dx.types.ResRet.i32 %396, 2
  %400 = add i32 %394, 16
  %401 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %395, i32 %400, i32 undef)
  %402 = extractvalue %dx.types.ResRet.i32 %401, 0
  %403 = extractvalue %dx.types.ResRet.i32 %401, 1
  %404 = extractvalue %dx.types.ResRet.i32 %401, 2
  %405 = bitcast i32 %397 to float
  %406 = bitcast i32 %398 to float
  %407 = bitcast i32 %399 to float
  %408 = bitcast i32 %402 to float
  %409 = bitcast i32 %403 to float
  %410 = bitcast i32 %404 to float
  %411 = shl i32 %124, 5
  %412 = add i32 %116, %411
  %413 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %395, i32 %412, i32 undef)
  %414 = extractvalue %dx.types.ResRet.i32 %413, 0
  %415 = extractvalue %dx.types.ResRet.i32 %413, 1
  %416 = extractvalue %dx.types.ResRet.i32 %413, 2
  %417 = add i32 %412, 16
  %418 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %395, i32 %417, i32 undef)
  %419 = extractvalue %dx.types.ResRet.i32 %418, 0
  %420 = extractvalue %dx.types.ResRet.i32 %418, 1
  %421 = extractvalue %dx.types.ResRet.i32 %418, 2
  %422 = bitcast i32 %414 to float
  %423 = bitcast i32 %415 to float
  %424 = bitcast i32 %416 to float
  %425 = bitcast i32 %419 to float
  %426 = bitcast i32 %420 to float
  %427 = bitcast i32 %421 to float
  %.i0378 = fmul fast float %405, %currentRayData.i.i.2.1.i0
  %.i1379 = fmul fast float %406, %currentRayData.i.i.2.1.i1
  %.i2380 = fmul fast float %407, %currentRayData.i.i.2.1.i2
  %.i0381 = fsub fast float %.i0378, %currentRayData.i.i.3.1.i0
  %.i1382 = fsub fast float %.i1379, %currentRayData.i.i.3.1.i1
  %.i2383 = fsub fast float %.i2380, %currentRayData.i.i.3.1.i2
  %FAbs232 = call float @dx.op.unary.f32(i32 6, float %currentRayData.i.i.2.1.i0)
  %FAbs233 = call float @dx.op.unary.f32(i32 6, float %currentRayData.i.i.2.1.i1)
  %FAbs234 = call float @dx.op.unary.f32(i32 6, float %currentRayData.i.i.2.1.i2)
  %.i0384 = fmul fast float %FAbs232, %408
  %.i1385 = fmul fast float %FAbs233, %409
  %.i2386 = fmul fast float %FAbs234, %410
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
  %428 = fcmp fast olt float %FMax266, %FMin265
  %.i0396 = fmul fast float %422, %currentRayData.i.i.2.1.i0
  %.i1397 = fmul fast float %423, %currentRayData.i.i.2.1.i1
  %.i2398 = fmul fast float %424, %currentRayData.i.i.2.1.i2
  %.i0399 = fsub fast float %.i0396, %currentRayData.i.i.3.1.i0
  %.i1400 = fsub fast float %.i1397, %currentRayData.i.i.3.1.i1
  %.i2401 = fsub fast float %.i2398, %currentRayData.i.i.3.1.i2
  %.i0402 = fmul fast float %FAbs232, %425
  %.i1403 = fmul fast float %FAbs233, %426
  %.i2404 = fmul fast float %FAbs234, %427
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
  %429 = fcmp fast olt float %FMax, %FMin259
  %430 = and i1 %428, %429
  br i1 %430, label %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i", label %439

"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i":       ; preds = %391
  %431 = fcmp fast olt float %FMax, %FMax266
  %432 = select i1 %431, i32 %392, i32 %124
  %433 = select i1 %431, i32 %124, i32 %392
  %434 = shl i32 %stackPointer.i.i.2, 6
  %435 = add i32 %434, %31
  store i32 %432, i32 addrspace(3)* %110, align 4, !tbaa !221, !noalias !259
  %436 = getelementptr [2048 x i32], [2048 x i32] addrspace(3)* @"\01?stack@@3PAIA", i32 0, i32 %435
  store i32 %433, i32 addrspace(3)* %436, align 4, !tbaa !221, !noalias !259
  %437 = add nsw i32 %stackPointer.i.i.2, 1
  %438 = add i32 %106, 1
  store i32 %438, i32* %112, align 4, !tbaa !221
  br label %443

; <label>:439                                     ; preds = %391
  %440 = or i1 %428, %429
  br i1 %440, label %441, label %443

; <label>:441                                     ; preds = %439
  %442 = select i1 %429, i32 %124, i32 %392
  store i32 %442, i32 addrspace(3)* %110, align 4, !tbaa !221, !noalias !262
  store i32 %106, i32* %112, align 4, !tbaa !221
  br label %443

; <label>:443                                     ; preds = %441, %439, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i", %390, %387, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i", %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i", %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189"
  %.3.i0 = phi float [ %.1.i0, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %.1.i0, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189" ], [ %.2.i0, %390 ], [ %.2.i0, %387 ], [ %.2.i0, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i" ], [ %.1.i0, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %.1.i0, %441 ], [ %.1.i0, %439 ]
  %.3.i1 = phi float [ %.1.i1, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %.1.i1, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189" ], [ %.2.i1, %390 ], [ %.2.i1, %387 ], [ %.2.i1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i" ], [ %.1.i1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %.1.i1, %441 ], [ %.1.i1, %439 ]
  %currentRayData.i.i.0.2.i0 = phi float [ %FMad292, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %currentRayData.i.i.0.1.i0, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189" ], [ %currentRayData.i.i.0.1.i0, %390 ], [ %currentRayData.i.i.0.1.i0, %387 ], [ %currentRayData.i.i.0.1.i0, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i" ], [ %currentRayData.i.i.0.1.i0, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %currentRayData.i.i.0.1.i0, %441 ], [ %currentRayData.i.i.0.1.i0, %439 ]
  %currentRayData.i.i.0.2.i1 = phi float [ %FMad289, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %currentRayData.i.i.0.1.i1, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189" ], [ %currentRayData.i.i.0.1.i1, %390 ], [ %currentRayData.i.i.0.1.i1, %387 ], [ %currentRayData.i.i.0.1.i1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i" ], [ %currentRayData.i.i.0.1.i1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %currentRayData.i.i.0.1.i1, %441 ], [ %currentRayData.i.i.0.1.i1, %439 ]
  %currentRayData.i.i.0.2.i2 = phi float [ %FMad286, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %currentRayData.i.i.0.1.i2, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189" ], [ %currentRayData.i.i.0.1.i2, %390 ], [ %currentRayData.i.i.0.1.i2, %387 ], [ %currentRayData.i.i.0.1.i2, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i" ], [ %currentRayData.i.i.0.1.i2, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %currentRayData.i.i.0.1.i2, %441 ], [ %currentRayData.i.i.0.1.i2, %439 ]
  %currentRayData.i.i.1.2.i0 = phi float [ %FMad283, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %currentRayData.i.i.1.1.i0, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189" ], [ %currentRayData.i.i.1.1.i0, %390 ], [ %currentRayData.i.i.1.1.i0, %387 ], [ %currentRayData.i.i.1.1.i0, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i" ], [ %currentRayData.i.i.1.1.i0, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %currentRayData.i.i.1.1.i0, %441 ], [ %currentRayData.i.i.1.1.i0, %439 ]
  %currentRayData.i.i.1.2.i1 = phi float [ %FMad280, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %currentRayData.i.i.1.1.i1, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189" ], [ %currentRayData.i.i.1.1.i1, %390 ], [ %currentRayData.i.i.1.1.i1, %387 ], [ %currentRayData.i.i.1.1.i1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i" ], [ %currentRayData.i.i.1.1.i1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %currentRayData.i.i.1.1.i1, %441 ], [ %currentRayData.i.i.1.1.i1, %439 ]
  %currentRayData.i.i.1.2.i2 = phi float [ %FMad, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %currentRayData.i.i.1.1.i2, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189" ], [ %currentRayData.i.i.1.1.i2, %390 ], [ %currentRayData.i.i.1.1.i2, %387 ], [ %currentRayData.i.i.1.1.i2, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i" ], [ %currentRayData.i.i.1.1.i2, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %currentRayData.i.i.1.1.i2, %441 ], [ %currentRayData.i.i.1.1.i2, %439 ]
  %currentRayData.i.i.2.2.i0 = phi float [ %.i0340, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %currentRayData.i.i.2.1.i0, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189" ], [ %currentRayData.i.i.2.1.i0, %390 ], [ %currentRayData.i.i.2.1.i0, %387 ], [ %currentRayData.i.i.2.1.i0, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i" ], [ %currentRayData.i.i.2.1.i0, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %currentRayData.i.i.2.1.i0, %441 ], [ %currentRayData.i.i.2.1.i0, %439 ]
  %currentRayData.i.i.2.2.i1 = phi float [ %.i1341, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %currentRayData.i.i.2.1.i1, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189" ], [ %currentRayData.i.i.2.1.i1, %390 ], [ %currentRayData.i.i.2.1.i1, %387 ], [ %currentRayData.i.i.2.1.i1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i" ], [ %currentRayData.i.i.2.1.i1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %currentRayData.i.i.2.1.i1, %441 ], [ %currentRayData.i.i.2.1.i1, %439 ]
  %currentRayData.i.i.2.2.i2 = phi float [ %.i2342, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %currentRayData.i.i.2.1.i2, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189" ], [ %currentRayData.i.i.2.1.i2, %390 ], [ %currentRayData.i.i.2.1.i2, %387 ], [ %currentRayData.i.i.2.1.i2, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i" ], [ %currentRayData.i.i.2.1.i2, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %currentRayData.i.i.2.1.i2, %441 ], [ %currentRayData.i.i.2.1.i2, %439 ]
  %currentRayData.i.i.3.2.i0 = phi float [ %.i0343, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %currentRayData.i.i.3.1.i0, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189" ], [ %currentRayData.i.i.3.1.i0, %390 ], [ %currentRayData.i.i.3.1.i0, %387 ], [ %currentRayData.i.i.3.1.i0, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i" ], [ %currentRayData.i.i.3.1.i0, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %currentRayData.i.i.3.1.i0, %441 ], [ %currentRayData.i.i.3.1.i0, %439 ]
  %currentRayData.i.i.3.2.i1 = phi float [ %.i1344, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %currentRayData.i.i.3.1.i1, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189" ], [ %currentRayData.i.i.3.1.i1, %390 ], [ %currentRayData.i.i.3.1.i1, %387 ], [ %currentRayData.i.i.3.1.i1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i" ], [ %currentRayData.i.i.3.1.i1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %currentRayData.i.i.3.1.i1, %441 ], [ %currentRayData.i.i.3.1.i1, %439 ]
  %currentRayData.i.i.3.2.i2 = phi float [ %.i2345, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %currentRayData.i.i.3.1.i2, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189" ], [ %currentRayData.i.i.3.1.i2, %390 ], [ %currentRayData.i.i.3.1.i2, %387 ], [ %currentRayData.i.i.3.1.i2, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i" ], [ %currentRayData.i.i.3.1.i2, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %currentRayData.i.i.3.1.i2, %441 ], [ %currentRayData.i.i.3.1.i2, %439 ]
  %currentRayData.i.i.4.2.i0 = phi float [ %225, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %currentRayData.i.i.4.1.i0, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189" ], [ %currentRayData.i.i.4.1.i0, %390 ], [ %currentRayData.i.i.4.1.i0, %387 ], [ %currentRayData.i.i.4.1.i0, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i" ], [ %currentRayData.i.i.4.1.i0, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %currentRayData.i.i.4.1.i0, %441 ], [ %currentRayData.i.i.4.1.i0, %439 ]
  %currentRayData.i.i.4.2.i1 = phi float [ %228, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %currentRayData.i.i.4.1.i1, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189" ], [ %currentRayData.i.i.4.1.i1, %390 ], [ %currentRayData.i.i.4.1.i1, %387 ], [ %currentRayData.i.i.4.1.i1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i" ], [ %currentRayData.i.i.4.1.i1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %currentRayData.i.i.4.1.i1, %441 ], [ %currentRayData.i.i.4.1.i1, %439 ]
  %currentRayData.i.i.4.2.i2 = phi float [ %229, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %currentRayData.i.i.4.1.i2, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189" ], [ %currentRayData.i.i.4.1.i2, %390 ], [ %currentRayData.i.i.4.1.i2, %387 ], [ %currentRayData.i.i.4.1.i2, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i" ], [ %currentRayData.i.i.4.1.i2, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %currentRayData.i.i.4.1.i2, %441 ], [ %currentRayData.i.i.4.1.i2, %439 ]
  %currentRayData.i.i.5.2.i0 = phi i32 [ %.5.0.i0, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %currentRayData.i.i.5.1.i0, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189" ], [ %currentRayData.i.i.5.1.i0, %390 ], [ %currentRayData.i.i.5.1.i0, %387 ], [ %currentRayData.i.i.5.1.i0, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i" ], [ %currentRayData.i.i.5.1.i0, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %currentRayData.i.i.5.1.i0, %441 ], [ %currentRayData.i.i.5.1.i0, %439 ]
  %currentRayData.i.i.5.2.i1 = phi i32 [ %.5.0.i1, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %currentRayData.i.i.5.1.i1, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189" ], [ %currentRayData.i.i.5.1.i1, %390 ], [ %currentRayData.i.i.5.1.i1, %387 ], [ %currentRayData.i.i.5.1.i1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i" ], [ %currentRayData.i.i.5.1.i1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %currentRayData.i.i.5.1.i1, %441 ], [ %currentRayData.i.i.5.1.i1, %439 ]
  %currentRayData.i.i.5.2.i2 = phi i32 [ %.0172, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %currentRayData.i.i.5.1.i2, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189" ], [ %currentRayData.i.i.5.1.i2, %390 ], [ %currentRayData.i.i.5.1.i2, %387 ], [ %currentRayData.i.i.5.1.i2, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i" ], [ %currentRayData.i.i.5.1.i2, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %currentRayData.i.i.5.1.i2, %441 ], [ %currentRayData.i.i.5.1.i2, %439 ]
  %CurrentObjectToWorld.i.i167.2.i0 = phi float [ %190, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i0, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189" ], [ %CurrentObjectToWorld.i.i167.1.i0, %390 ], [ %CurrentObjectToWorld.i.i167.1.i0, %387 ], [ %CurrentObjectToWorld.i.i167.1.i0, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i0, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i0, %441 ], [ %CurrentObjectToWorld.i.i167.1.i0, %439 ]
  %CurrentObjectToWorld.i.i167.2.i1 = phi float [ %191, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i1, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189" ], [ %CurrentObjectToWorld.i.i167.1.i1, %390 ], [ %CurrentObjectToWorld.i.i167.1.i1, %387 ], [ %CurrentObjectToWorld.i.i167.1.i1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i1, %441 ], [ %CurrentObjectToWorld.i.i167.1.i1, %439 ]
  %CurrentObjectToWorld.i.i167.2.i2 = phi float [ %192, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i2, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189" ], [ %CurrentObjectToWorld.i.i167.1.i2, %390 ], [ %CurrentObjectToWorld.i.i167.1.i2, %387 ], [ %CurrentObjectToWorld.i.i167.1.i2, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i2, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i2, %441 ], [ %CurrentObjectToWorld.i.i167.1.i2, %439 ]
  %CurrentObjectToWorld.i.i167.2.i3 = phi float [ %193, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i3, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189" ], [ %CurrentObjectToWorld.i.i167.1.i3, %390 ], [ %CurrentObjectToWorld.i.i167.1.i3, %387 ], [ %CurrentObjectToWorld.i.i167.1.i3, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i3, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i3, %441 ], [ %CurrentObjectToWorld.i.i167.1.i3, %439 ]
  %CurrentObjectToWorld.i.i167.2.i4 = phi float [ %194, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i4, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189" ], [ %CurrentObjectToWorld.i.i167.1.i4, %390 ], [ %CurrentObjectToWorld.i.i167.1.i4, %387 ], [ %CurrentObjectToWorld.i.i167.1.i4, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i4, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i4, %441 ], [ %CurrentObjectToWorld.i.i167.1.i4, %439 ]
  %CurrentObjectToWorld.i.i167.2.i5 = phi float [ %195, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i5, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189" ], [ %CurrentObjectToWorld.i.i167.1.i5, %390 ], [ %CurrentObjectToWorld.i.i167.1.i5, %387 ], [ %CurrentObjectToWorld.i.i167.1.i5, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i5, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i5, %441 ], [ %CurrentObjectToWorld.i.i167.1.i5, %439 ]
  %CurrentObjectToWorld.i.i167.2.i6 = phi float [ %196, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i6, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189" ], [ %CurrentObjectToWorld.i.i167.1.i6, %390 ], [ %CurrentObjectToWorld.i.i167.1.i6, %387 ], [ %CurrentObjectToWorld.i.i167.1.i6, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i6, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i6, %441 ], [ %CurrentObjectToWorld.i.i167.1.i6, %439 ]
  %CurrentObjectToWorld.i.i167.2.i7 = phi float [ %197, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i7, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189" ], [ %CurrentObjectToWorld.i.i167.1.i7, %390 ], [ %CurrentObjectToWorld.i.i167.1.i7, %387 ], [ %CurrentObjectToWorld.i.i167.1.i7, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i7, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i7, %441 ], [ %CurrentObjectToWorld.i.i167.1.i7, %439 ]
  %CurrentObjectToWorld.i.i167.2.i8 = phi float [ %198, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i8, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189" ], [ %CurrentObjectToWorld.i.i167.1.i8, %390 ], [ %CurrentObjectToWorld.i.i167.1.i8, %387 ], [ %CurrentObjectToWorld.i.i167.1.i8, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i8, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i8, %441 ], [ %CurrentObjectToWorld.i.i167.1.i8, %439 ]
  %CurrentObjectToWorld.i.i167.2.i9 = phi float [ %199, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i9, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189" ], [ %CurrentObjectToWorld.i.i167.1.i9, %390 ], [ %CurrentObjectToWorld.i.i167.1.i9, %387 ], [ %CurrentObjectToWorld.i.i167.1.i9, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i9, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i9, %441 ], [ %CurrentObjectToWorld.i.i167.1.i9, %439 ]
  %CurrentObjectToWorld.i.i167.2.i10 = phi float [ %200, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i10, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189" ], [ %CurrentObjectToWorld.i.i167.1.i10, %390 ], [ %CurrentObjectToWorld.i.i167.1.i10, %387 ], [ %CurrentObjectToWorld.i.i167.1.i10, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i10, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i10, %441 ], [ %CurrentObjectToWorld.i.i167.1.i10, %439 ]
  %CurrentObjectToWorld.i.i167.2.i11 = phi float [ %201, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i11, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189" ], [ %CurrentObjectToWorld.i.i167.1.i11, %390 ], [ %CurrentObjectToWorld.i.i167.1.i11, %387 ], [ %CurrentObjectToWorld.i.i167.1.i11, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i11, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i11, %441 ], [ %CurrentObjectToWorld.i.i167.1.i11, %439 ]
  %CurrentWorldToObject.i.i168.2.i0 = phi float [ %173, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %CurrentWorldToObject.i.i168.1.i0, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189" ], [ %CurrentWorldToObject.i.i168.1.i0, %390 ], [ %CurrentWorldToObject.i.i168.1.i0, %387 ], [ %CurrentWorldToObject.i.i168.1.i0, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i0, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %CurrentWorldToObject.i.i168.1.i0, %441 ], [ %CurrentWorldToObject.i.i168.1.i0, %439 ]
  %CurrentWorldToObject.i.i168.2.i1 = phi float [ %174, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %CurrentWorldToObject.i.i168.1.i1, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189" ], [ %CurrentWorldToObject.i.i168.1.i1, %390 ], [ %CurrentWorldToObject.i.i168.1.i1, %387 ], [ %CurrentWorldToObject.i.i168.1.i1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %CurrentWorldToObject.i.i168.1.i1, %441 ], [ %CurrentWorldToObject.i.i168.1.i1, %439 ]
  %CurrentWorldToObject.i.i168.2.i2 = phi float [ %175, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %CurrentWorldToObject.i.i168.1.i2, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189" ], [ %CurrentWorldToObject.i.i168.1.i2, %390 ], [ %CurrentWorldToObject.i.i168.1.i2, %387 ], [ %CurrentWorldToObject.i.i168.1.i2, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i2, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %CurrentWorldToObject.i.i168.1.i2, %441 ], [ %CurrentWorldToObject.i.i168.1.i2, %439 ]
  %CurrentWorldToObject.i.i168.2.i3 = phi float [ %176, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %CurrentWorldToObject.i.i168.1.i3, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189" ], [ %CurrentWorldToObject.i.i168.1.i3, %390 ], [ %CurrentWorldToObject.i.i168.1.i3, %387 ], [ %CurrentWorldToObject.i.i168.1.i3, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i3, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %CurrentWorldToObject.i.i168.1.i3, %441 ], [ %CurrentWorldToObject.i.i168.1.i3, %439 ]
  %CurrentWorldToObject.i.i168.2.i4 = phi float [ %177, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %CurrentWorldToObject.i.i168.1.i4, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189" ], [ %CurrentWorldToObject.i.i168.1.i4, %390 ], [ %CurrentWorldToObject.i.i168.1.i4, %387 ], [ %CurrentWorldToObject.i.i168.1.i4, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i4, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %CurrentWorldToObject.i.i168.1.i4, %441 ], [ %CurrentWorldToObject.i.i168.1.i4, %439 ]
  %CurrentWorldToObject.i.i168.2.i5 = phi float [ %178, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %CurrentWorldToObject.i.i168.1.i5, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189" ], [ %CurrentWorldToObject.i.i168.1.i5, %390 ], [ %CurrentWorldToObject.i.i168.1.i5, %387 ], [ %CurrentWorldToObject.i.i168.1.i5, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i5, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %CurrentWorldToObject.i.i168.1.i5, %441 ], [ %CurrentWorldToObject.i.i168.1.i5, %439 ]
  %CurrentWorldToObject.i.i168.2.i6 = phi float [ %179, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %CurrentWorldToObject.i.i168.1.i6, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189" ], [ %CurrentWorldToObject.i.i168.1.i6, %390 ], [ %CurrentWorldToObject.i.i168.1.i6, %387 ], [ %CurrentWorldToObject.i.i168.1.i6, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i6, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %CurrentWorldToObject.i.i168.1.i6, %441 ], [ %CurrentWorldToObject.i.i168.1.i6, %439 ]
  %CurrentWorldToObject.i.i168.2.i7 = phi float [ %180, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %CurrentWorldToObject.i.i168.1.i7, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189" ], [ %CurrentWorldToObject.i.i168.1.i7, %390 ], [ %CurrentWorldToObject.i.i168.1.i7, %387 ], [ %CurrentWorldToObject.i.i168.1.i7, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i7, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %CurrentWorldToObject.i.i168.1.i7, %441 ], [ %CurrentWorldToObject.i.i168.1.i7, %439 ]
  %CurrentWorldToObject.i.i168.2.i8 = phi float [ %181, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %CurrentWorldToObject.i.i168.1.i8, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189" ], [ %CurrentWorldToObject.i.i168.1.i8, %390 ], [ %CurrentWorldToObject.i.i168.1.i8, %387 ], [ %CurrentWorldToObject.i.i168.1.i8, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i8, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %CurrentWorldToObject.i.i168.1.i8, %441 ], [ %CurrentWorldToObject.i.i168.1.i8, %439 ]
  %CurrentWorldToObject.i.i168.2.i9 = phi float [ %182, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %CurrentWorldToObject.i.i168.1.i9, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189" ], [ %CurrentWorldToObject.i.i168.1.i9, %390 ], [ %CurrentWorldToObject.i.i168.1.i9, %387 ], [ %CurrentWorldToObject.i.i168.1.i9, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i9, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %CurrentWorldToObject.i.i168.1.i9, %441 ], [ %CurrentWorldToObject.i.i168.1.i9, %439 ]
  %CurrentWorldToObject.i.i168.2.i10 = phi float [ %183, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %CurrentWorldToObject.i.i168.1.i10, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189" ], [ %CurrentWorldToObject.i.i168.1.i10, %390 ], [ %CurrentWorldToObject.i.i168.1.i10, %387 ], [ %CurrentWorldToObject.i.i168.1.i10, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i10, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %CurrentWorldToObject.i.i168.1.i10, %441 ], [ %CurrentWorldToObject.i.i168.1.i10, %439 ]
  %CurrentWorldToObject.i.i168.2.i11 = phi float [ %184, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %CurrentWorldToObject.i.i168.1.i11, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189" ], [ %CurrentWorldToObject.i.i168.1.i11, %390 ], [ %CurrentWorldToObject.i.i168.1.i11, %387 ], [ %CurrentWorldToObject.i.i168.1.i11, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i11, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %CurrentWorldToObject.i.i168.1.i11, %441 ], [ %CurrentWorldToObject.i.i168.1.i11, %439 ]
  %ResetMatrices.i.i.3 = phi i32 [ %ResetMatrices.i.i.1, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %ResetMatrices.i.i.1, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189" ], [ 0, %390 ], [ 0, %387 ], [ %ResetMatrices.i.i.1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i" ], [ %ResetMatrices.i.i.1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %ResetMatrices.i.i.1, %441 ], [ %ResetMatrices.i.i.1, %439 ]
  %currentBVHIndex.i.i.2 = phi i32 [ 1, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ 0, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189" ], [ %currentBVHIndex.i.i.1, %390 ], [ %currentBVHIndex.i.i.1, %387 ], [ %currentBVHIndex.i.i.1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i" ], [ %currentBVHIndex.i.i.1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %currentBVHIndex.i.i.1, %441 ], [ %currentBVHIndex.i.i.1, %439 ]
  %currentGpuVA.i.i.2.i0 = phi i32 [ %153, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %currentGpuVA.i.i.1.i0, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189" ], [ %currentGpuVA.i.i.1.i0, %390 ], [ %currentGpuVA.i.i.1.i0, %387 ], [ %currentGpuVA.i.i.1.i0, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i" ], [ %currentGpuVA.i.i.1.i0, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %currentGpuVA.i.i.1.i0, %441 ], [ %currentGpuVA.i.i.1.i0, %439 ]
  %currentGpuVA.i.i.2.i1 = phi i32 [ %154, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %currentGpuVA.i.i.1.i1, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189" ], [ %currentGpuVA.i.i.1.i1, %390 ], [ %currentGpuVA.i.i.1.i1, %387 ], [ %currentGpuVA.i.i.1.i1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i" ], [ %currentGpuVA.i.i.1.i1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %currentGpuVA.i.i.1.i1, %441 ], [ %currentGpuVA.i.i.1.i1, %439 ]
  %instIdx.i.i.2 = phi i32 [ %128, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %128, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189" ], [ %instIdx.i.i.1, %390 ], [ %instIdx.i.i.1, %387 ], [ %instIdx.i.i.1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i" ], [ %instIdx.i.i.1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %instIdx.i.i.1, %441 ], [ %instIdx.i.i.1, %439 ]
  %instFlags.i.i.2 = phi i32 [ %203, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %instFlags.i.i.1, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189" ], [ %instFlags.i.i.1, %390 ], [ %instFlags.i.i.1, %387 ], [ %instFlags.i.i.1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i" ], [ %instFlags.i.i.1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %instFlags.i.i.1, %441 ], [ %instFlags.i.i.1, %439 ]
  %instOffset.i.i.2 = phi i32 [ %185, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %185, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189" ], [ %instOffset.i.i.1, %390 ], [ %instOffset.i.i.1, %387 ], [ %instOffset.i.i.1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i" ], [ %instOffset.i.i.1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %instOffset.i.i.1, %441 ], [ %instOffset.i.i.1, %439 ]
  %instId.i.i.2 = phi i32 [ %186, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %186, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189" ], [ %instId.i.i.1, %390 ], [ %instId.i.i.1, %387 ], [ %instId.i.i.1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i" ], [ %instId.i.i.1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %instId.i.i.1, %441 ], [ %instId.i.i.1, %439 ]
  %stackPointer.i.i.3 = phi i32 [ %stackPointer.i.i.2, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %107, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189" ], [ %107, %390 ], [ %107, %387 ], [ %107, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i" ], [ %437, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %stackPointer.i.i.2, %441 ], [ %107, %439 ]
  %resultBary.i.i.2.i0 = phi float [ %resultBary.i.i.1.i0, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %resultBary.i.i.1.i0, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189" ], [ %.2.i0.resultBary.i.i.1.i0, %390 ], [ %.2.i0.resultBary.i.i.1.i0, %387 ], [ %.2.i0.resultBary.i.i.1.i0, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i" ], [ %resultBary.i.i.1.i0, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %resultBary.i.i.1.i0, %441 ], [ %resultBary.i.i.1.i0, %439 ]
  %resultBary.i.i.2.i1 = phi float [ %resultBary.i.i.1.i1, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %resultBary.i.i.1.i1, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189" ], [ %.2.i1.resultBary.i.i.1.i1, %390 ], [ %.2.i1.resultBary.i.i.1.i1, %387 ], [ %.2.i1.resultBary.i.i.1.i1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i" ], [ %resultBary.i.i.1.i1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %resultBary.i.i.1.i1, %441 ], [ %resultBary.i.i.1.i1, %439 ]
  %resultTriId.i.i.2 = phi i32 [ %resultTriId.i.i.1, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i" ], [ %resultTriId.i.i.1, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189" ], [ %.resultTriId.i.i.1, %390 ], [ %.resultTriId.i.i.1, %387 ], [ %.resultTriId.i.i.1, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i" ], [ %resultTriId.i.i.1, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i" ], [ %resultTriId.i.i.1, %441 ], [ %resultTriId.i.i.1, %439 ]
  %444 = getelementptr inbounds [2 x i32], [2 x i32]* %nodesToProcess.i.i, i32 0, i32 %currentBVHIndex.i.i.2
  %445 = load i32, i32* %444, align 4, !tbaa !221
  %446 = icmp eq i32 %445, 0
  br i1 %446, label %447, label %105

; <label>:447                                     ; preds = %443
  %448 = add i32 %currentBVHIndex.i.i.2, -1
  %449 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %AccelerationStructureList, i32 0)
  %450 = extractvalue %dx.types.CBufRet.i32 %449, 0
  %451 = extractvalue %dx.types.CBufRet.i32 %449, 1
  %452 = load i32, i32* %58, align 4, !tbaa !221
  %453 = icmp eq i32 %452, 0
  br i1 %453, label %._crit_edge, label %.lr.ph

._crit_edge:                                      ; preds = %447, %104
  %InstanceIndex = call i32 @dx.op.instanceIndex.i32(i32 142)
  %454 = icmp eq i32 %InstanceIndex, -1
  br i1 %454, label %466, label %455

; <label>:455                                     ; preds = %._crit_edge
  %RayFlags258 = call i32 @dx.op.rayFlags.i32(i32 144)
  %456 = and i32 %RayFlags258, 8
  %457 = icmp eq i32 %456, 0
  br i1 %457, label %458, label %"\01?Traverse@@YAHIIII@Z.exit"

; <label>:458                                     ; preds = %455
  %459 = call i32 @"\01?Fallback_GeometryIndex@@YAIXZ"() #3
  %460 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %Constants295, i32 0)
  %461 = extractvalue %dx.types.CBufRet.i32 %460, 2
  %462 = mul i32 %461, %459
  %463 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.ByteAddressBuffer(i32 160, %struct.ByteAddressBuffer %16)
  %464 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %463, i32 %462, i32 undef)
  %465 = extractvalue %dx.types.ResRet.i32 %464, 0
  br label %"\01?Traverse@@YAHIIII@Z.exit"

; <label>:466                                     ; preds = %._crit_edge
  %467 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %Constants295, i32 0)
  %468 = extractvalue %dx.types.CBufRet.i32 %467, 3
  %469 = mul i32 %468, %4
  %470 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.ByteAddressBuffer(i32 160, %struct.ByteAddressBuffer %15)
  %471 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %470, i32 %469, i32 undef)
  %472 = extractvalue %dx.types.ResRet.i32 %471, 0
  br label %"\01?Traverse@@YAHIIII@Z.exit"

"\01?Traverse@@YAHIIII@Z.exit":                   ; preds = %466, %458, %455
  %stateID.i.0 = phi i32 [ %465, %458 ], [ %472, %466 ], [ 0, %455 ]
  %473 = icmp eq i32 %stateID.i.0, 0
  br i1 %473, label %475, label %474

; <label>:474                                     ; preds = %"\01?Traverse@@YAHIIII@Z.exit"
  call void @"\01?Fallback_CallIndirect@@YAXH@Z"(i32 %stateID.i.0) #3
  br label %475

; <label>:475                                     ; preds = %474, %"\01?Traverse@@YAHIIII@Z.exit"
  %476 = call i32 @"\01?Fallback_SetPayloadOffset@@YAII@Z"(i32 %24) #3
  ret void
}

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
  store <4 x float> %29, <4 x float>* %30, align 4, !tbaa !218, !alias.scope !265
  %.i035 = bitcast i32 %13 to float
  %.i136 = bitcast i32 %14 to float
  %.i237 = bitcast i32 %15 to float
  %.i338 = bitcast i32 %16 to float
  %.upto043 = insertelement <4 x float> undef, float %.i035, i32 0
  %.upto144 = insertelement <4 x float> %.upto043, float %.i136, i32 1
  %.upto245 = insertelement <4 x float> %.upto144, float %.i237, i32 2
  %31 = insertelement <4 x float> %.upto245, float %.i338, i32 3
  %32 = getelementptr inbounds %struct.RaytracingInstanceDesc, %struct.RaytracingInstanceDesc* %0, i32 0, i32 0, i32 1
  store <4 x float> %31, <4 x float>* %32, align 4, !tbaa !218, !alias.scope !265
  %.i039 = bitcast i32 %19 to float
  %.i140 = bitcast i32 %20 to float
  %.i241 = bitcast i32 %21 to float
  %.i342 = bitcast i32 %22 to float
  %.upto046 = insertelement <4 x float> undef, float %.i039, i32 0
  %.upto147 = insertelement <4 x float> %.upto046, float %.i140, i32 1
  %.upto248 = insertelement <4 x float> %.upto147, float %.i241, i32 2
  %33 = insertelement <4 x float> %.upto248, float %.i342, i32 3
  %34 = getelementptr inbounds %struct.RaytracingInstanceDesc, %struct.RaytracingInstanceDesc* %0, i32 0, i32 0, i32 2
  store <4 x float> %33, <4 x float>* %34, align 4, !tbaa !218, !alias.scope !265
  %35 = getelementptr inbounds %struct.RaytracingInstanceDesc, %struct.RaytracingInstanceDesc* %0, i32 0, i32 1
  store i32 %25, i32* %35, align 4, !tbaa !221, !alias.scope !265
  %36 = getelementptr inbounds %struct.RaytracingInstanceDesc, %struct.RaytracingInstanceDesc* %0, i32 0, i32 2
  store i32 %26, i32* %36, align 4, !tbaa !221, !alias.scope !265
  %.upto049 = insertelement <2 x i32> undef, i32 %27, i32 0
  %37 = insertelement <2 x i32> %.upto049, i32 %28, i32 1
  %38 = getelementptr inbounds %struct.RaytracingInstanceDesc, %struct.RaytracingInstanceDesc* %0, i32 0, i32 3
  store <2 x i32> %37, <2 x i32>* %38, align 4, !tbaa !218, !alias.scope !265
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
  %19 = load i32, i32* %18, align 4, !tbaa !221, !noalias !268
  %20 = add i32 %19, 4
  %21 = getelementptr inbounds %struct.RWByteAddressBufferPointer, %struct.RWByteAddressBufferPointer* %0, i32 0, i32 0
  %22 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %21, align 4, !noalias !268
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
  %44 = load float, float* %8, align 4, !tbaa !223
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
  %63 = load float, float* %62, align 4, !tbaa !223, !noalias !273
  %64 = extractelement <3 x i32> %5, i32 1
  %65 = getelementptr [3 x float], [3 x float]* %15, i32 0, i32 %64
  %66 = load float, float* %65, align 4, !tbaa !223, !noalias !273
  %67 = extractelement <3 x i32> %5, i32 2
  %68 = getelementptr [3 x float], [3 x float]* %15, i32 0, i32 %67
  %69 = load float, float* %68, align 4, !tbaa !223, !noalias !273
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
  %74 = load float, float* %73, align 4, !tbaa !223, !noalias !273
  %75 = getelementptr [3 x float], [3 x float]* %13, i32 0, i32 %64
  %76 = load float, float* %75, align 4, !tbaa !223, !noalias !273
  %77 = getelementptr [3 x float], [3 x float]* %13, i32 0, i32 %67
  %78 = load float, float* %77, align 4, !tbaa !223, !noalias !273
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
  %83 = load float, float* %82, align 4, !tbaa !223, !noalias !273
  %84 = getelementptr [3 x float], [3 x float]* %14, i32 0, i32 %64
  %85 = load float, float* %84, align 4, !tbaa !223, !noalias !273
  %86 = getelementptr [3 x float], [3 x float]* %14, i32 0, i32 %67
  %87 = load float, float* %86, align 4, !tbaa !223, !noalias !273
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
  store float %.02, float* %8, align 4, !tbaa !223
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
  %25 = load float, float* %24, align 4, !tbaa !223, !noalias !277
  %26 = fcmp fast olt float %25, 0.000000e+00
  %worldRayData.5.0.i0 = select i1 %26, i32 %23, i32 %21
  %worldRayData.5.0.i1 = select i1 %26, i32 %21, i32 %23
  %27 = getelementptr [3 x float], [3 x float]* %9, i32 0, i32 %worldRayData.5.0.i0
  %28 = load float, float* %27, align 4, !tbaa !223, !noalias !277
  %29 = fdiv float %28, %25
  %30 = getelementptr [3 x float], [3 x float]* %9, i32 0, i32 %worldRayData.5.0.i1
  %31 = load float, float* %30, align 4, !tbaa !223, !noalias !277
  %32 = fdiv float %31, %25
  %33 = fdiv float 1.000000e+00, %25
  %34 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %AccelerationStructureList, i32 0)
  %35 = extractvalue %dx.types.CBufRet.i32 %34, 0
  %36 = extractvalue %dx.types.CBufRet.i32 %34, 1
  %37 = getelementptr inbounds [2 x i32], [2 x i32]* %nodesToProcess, i32 0, i32 0
  store i32 0, i32* %37, align 4, !tbaa !221
  %38 = getelementptr inbounds [0 x %struct.RWByteAddressBuffer], [0 x %struct.RWByteAddressBuffer]* @"\01?DescriptorHeapBufferTable@@3PAURWByteAddressBuffer@@A", i32 0, i32 %36, !dx.nonuniform !228
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
  store i32 0, i32 addrspace(3)* %68, align 4, !tbaa !221, !noalias !280
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
  %90 = load i32, i32 addrspace(3)* %89, align 4, !tbaa !221, !noalias !283
  %91 = getelementptr inbounds [2 x i32], [2 x i32]* %nodesToProcess, i32 0, i32 %currentBVHIndex.1
  %92 = add i32 %85, -1
  store i32 %92, i32* %91, align 4, !tbaa !221
  %93 = getelementptr inbounds [0 x %struct.RWByteAddressBuffer], [0 x %struct.RWByteAddressBuffer]* @"\01?DescriptorHeapBufferTable@@3PAURWByteAddressBuffer@@A", i32 0, i32 %currentGpuVA.1.i1, !dx.nonuniform !228
  %94 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %93, align 4, !noalias !286
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
  store i32 0, i32 addrspace(3)* %89, align 4, !tbaa !221, !noalias !291
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
  %200 = load float, float* %199, align 4, !tbaa !223, !noalias !294
  %201 = fcmp fast olt float %200, 0.000000e+00
  %.5.0.i0 = select i1 %201, i32 %198, i32 %196
  %.5.0.i1 = select i1 %201, i32 %196, i32 %198
  %202 = getelementptr [3 x float], [3 x float]* %8, i32 0, i32 %.5.0.i0
  %203 = load float, float* %202, align 4, !tbaa !223, !noalias !294
  %204 = fdiv float %203, %200
  %205 = getelementptr [3 x float], [3 x float]* %8, i32 0, i32 %.5.0.i1
  %206 = load float, float* %205, align 4, !tbaa !223, !noalias !294
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
  %248 = load float, float* %247, align 4, !tbaa !223, !noalias !297
  %249 = getelementptr [3 x float], [3 x float]* %7, i32 0, i32 %currentRayData.5.1.i1
  %250 = load float, float* %249, align 4, !tbaa !223, !noalias !297
  %251 = getelementptr [3 x float], [3 x float]* %7, i32 0, i32 %currentRayData.5.1.i2
  %252 = load float, float* %251, align 4, !tbaa !223, !noalias !297
  %.i0386 = fsub fast float %.i3378, %currentRayData.0.1.i0
  %.i1387 = fsub fast float %.i0379, %currentRayData.0.1.i1
  %.i2388 = fsub fast float %.i1380, %currentRayData.0.1.i2
  store float %.i0386, float* %76, align 4
  store float %.i1387, float* %77, align 4
  store float %.i2388, float* %78, align 4
  %253 = getelementptr [3 x float], [3 x float]* %5, i32 0, i32 %currentRayData.5.1.i0
  %254 = load float, float* %253, align 4, !tbaa !223, !noalias !297
  %255 = getelementptr [3 x float], [3 x float]* %5, i32 0, i32 %currentRayData.5.1.i1
  %256 = load float, float* %255, align 4, !tbaa !223, !noalias !297
  %257 = getelementptr [3 x float], [3 x float]* %5, i32 0, i32 %currentRayData.5.1.i2
  %258 = load float, float* %257, align 4, !tbaa !223, !noalias !297
  %.i0389 = fsub fast float %.i2381, %currentRayData.0.1.i0
  %.i1390 = fsub fast float %.i3382, %currentRayData.0.1.i1
  %.i2391 = fsub fast float %233, %currentRayData.0.1.i2
  store float %.i0389, float* %79, align 4
  store float %.i1390, float* %80, align 4
  store float %.i2391, float* %81, align 4
  %259 = getelementptr [3 x float], [3 x float]* %6, i32 0, i32 %currentRayData.5.1.i0
  %260 = load float, float* %259, align 4, !tbaa !223, !noalias !297
  %261 = getelementptr [3 x float], [3 x float]* %6, i32 0, i32 %currentRayData.5.1.i1
  %262 = load float, float* %261, align 4, !tbaa !223, !noalias !297
  %263 = getelementptr [3 x float], [3 x float]* %6, i32 0, i32 %currentRayData.5.1.i2
  %264 = load float, float* %263, align 4, !tbaa !223, !noalias !297
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
  %338 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %93, align 4, !noalias !305
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
  store i32 %411, i32 addrspace(3)* %89, align 4, !tbaa !221, !noalias !310
  %415 = getelementptr [2048 x i32], [2048 x i32] addrspace(3)* @"\01?stack@@3PAIA", i32 0, i32 %414
  store i32 %412, i32 addrspace(3)* %415, align 4, !tbaa !221, !noalias !310
  %416 = add nsw i32 %stackPointer.2, 1
  %417 = add i32 %85, 1
  store i32 %417, i32* %91, align 4, !tbaa !221
  br label %422

; <label>:418                                     ; preds = %370
  %419 = or i1 %407, %408
  br i1 %419, label %420, label %422

; <label>:420                                     ; preds = %418
  %421 = select i1 %408, i32 %103, i32 %371
  store i32 %421, i32 addrspace(3)* %89, align 4, !tbaa !221, !noalias !313
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
  store i32 %3, i32* %4, align 4, !tbaa !221, !alias.scope !316
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
  store <3 x float> %16, <3 x float>* %17, align 4, !tbaa !218, !alias.scope !319
  %.i03 = fadd fast float %11, %5
  %.i14 = fadd fast float %13, %7
  %.i25 = fadd fast float %15, %9
  %.upto06 = insertelement <3 x float> undef, float %.i03, i32 0
  %.upto17 = insertelement <3 x float> %.upto06, float %.i14, i32 1
  %18 = insertelement <3 x float> %.upto17, float %.i25, i32 2
  %19 = getelementptr inbounds %struct.AABB, %struct.AABB* %0, i32 0, i32 1
  store <3 x float> %18, <3 x float>* %19, align 4, !tbaa !218, !alias.scope !319
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
  call void @"\01?Fallback_Scheduler@@YAXHII@Z"(i32 %5, i32 %8, i32 %7) #3
  ret void
}

; Function Attrs: alwaysinline nounwind readnone
define void @"\01?LogInt3@@YAXV?$vector@H$02@@@Z"(<3 x i32>) #6 {
  ret void
}

; Function Attrs: alwaysinline nounwind readonly
define <4 x i32> @"\01?Load4@@YA?AV?$vector@I$03@@V?$vector@I$01@@@Z"(<2 x i32>) #4 {
  %2 = extractelement <2 x i32> %0, i32 1
  %3 = getelementptr inbounds [0 x %struct.RWByteAddressBuffer], [0 x %struct.RWByteAddressBuffer]* @"\01?DescriptorHeapBufferTable@@3PAURWByteAddressBuffer@@A", i32 0, i32 %2, !dx.nonuniform !228
  %4 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %3, align 4, !noalias !322
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
  store <4 x float> %29, <4 x float>* %30, align 4, !tbaa !218, !alias.scope !325
  %.i035 = bitcast i32 %13 to float
  %.i136 = bitcast i32 %14 to float
  %.i237 = bitcast i32 %15 to float
  %.i338 = bitcast i32 %16 to float
  %.upto043 = insertelement <4 x float> undef, float %.i035, i32 0
  %.upto144 = insertelement <4 x float> %.upto043, float %.i136, i32 1
  %.upto245 = insertelement <4 x float> %.upto144, float %.i237, i32 2
  %31 = insertelement <4 x float> %.upto245, float %.i338, i32 3
  %32 = getelementptr inbounds %struct.RaytracingInstanceDesc, %struct.RaytracingInstanceDesc* %0, i32 0, i32 0, i32 1
  store <4 x float> %31, <4 x float>* %32, align 4, !tbaa !218, !alias.scope !325
  %.i039 = bitcast i32 %19 to float
  %.i140 = bitcast i32 %20 to float
  %.i241 = bitcast i32 %21 to float
  %.i342 = bitcast i32 %22 to float
  %.upto046 = insertelement <4 x float> undef, float %.i039, i32 0
  %.upto147 = insertelement <4 x float> %.upto046, float %.i140, i32 1
  %.upto248 = insertelement <4 x float> %.upto147, float %.i241, i32 2
  %33 = insertelement <4 x float> %.upto248, float %.i342, i32 3
  %34 = getelementptr inbounds %struct.RaytracingInstanceDesc, %struct.RaytracingInstanceDesc* %0, i32 0, i32 0, i32 2
  store <4 x float> %33, <4 x float>* %34, align 4, !tbaa !218, !alias.scope !325
  %35 = getelementptr inbounds %struct.RaytracingInstanceDesc, %struct.RaytracingInstanceDesc* %0, i32 0, i32 1
  store i32 %25, i32* %35, align 4, !tbaa !221, !alias.scope !325
  %36 = getelementptr inbounds %struct.RaytracingInstanceDesc, %struct.RaytracingInstanceDesc* %0, i32 0, i32 2
  store i32 %26, i32* %36, align 4, !tbaa !221, !alias.scope !325
  %.upto049 = insertelement <2 x i32> undef, i32 %27, i32 0
  %37 = insertelement <2 x i32> %.upto049, i32 %28, i32 1
  %38 = getelementptr inbounds %struct.RaytracingInstanceDesc, %struct.RaytracingInstanceDesc* %0, i32 0, i32 3
  store <2 x i32> %37, <2 x i32>* %38, align 4, !tbaa !218, !alias.scope !325
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
define void @"\01?Hit@@YAXURayPayload@@UBuiltInTriangleIntersectionAttributes@@@Z"(%struct.RayPayload* noalias nocapture, %struct.BuiltInTriangleIntersectionAttributes* nocapture readonly) #5 {
  %LoadShaderTableHandle = load %struct.ByteAddressBuffer, %struct.ByteAddressBuffer* @"\01?HitGroupShaderTable@@3UByteAddressBuffer@@A"
  %ShaderTableHandle = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.ByteAddressBuffer(i32 160, %struct.ByteAddressBuffer %LoadShaderTableHandle)
  %DispatchRaysConstants = load %Constants, %Constants* @Constants
  %Constants = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.Constants(i32 160, %Constants %DispatchRaysConstants)
  %geometry.index = call i32 @"\01?Fallback_GeometryIndex@@YAIXZ"()
  %ConstantBuffer = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %Constants, i32 0)
  %ShaderTableStride = extractvalue %dx.types.CBufRet.i32 %ConstantBuffer, 2
  %baseShaderRecordOffset = mul i32 %geometry.index, %ShaderTableStride
  %3 = load %struct.SamplerComparisonState, %struct.SamplerComparisonState* @"\01?shadowSampler@@3USamplerComparisonState@@A", align 4
  %4 = load %struct.SamplerState, %struct.SamplerState* @"\01?g_s0@@3USamplerState@@A", align 4
  %5 = load %"class.Texture2D<float>", %"class.Texture2D<float>"* @"\01?texSSAO@@3V?$Texture2D@M@@A", align 4
  %6 = load %"class.Texture2D<float>", %"class.Texture2D<float>"* @"\01?texShadow@@3V?$Texture2D@M@@A", align 4
  %7 = load %struct.ByteAddressBuffer, %struct.ByteAddressBuffer* @"\01?g_attributes@@3UByteAddressBuffer@@A", align 4
  %8 = load %struct.ByteAddressBuffer, %struct.ByteAddressBuffer* @"\01?g_indices@@3UByteAddressBuffer@@A", align 4
  %9 = load %"class.StructuredBuffer<RayTraceMeshInfo>", %"class.StructuredBuffer<RayTraceMeshInfo>"* @"\01?g_meshInfo@@3V?$StructuredBuffer@URayTraceMeshInfo@@@@A", align 4
  %10 = load %struct.RaytracingAccelerationStructure, %struct.RaytracingAccelerationStructure* @"\01?g_accel@@3URaytracingAccelerationStructure@@A", align 4
  %11 = load %"class.RWTexture2D<vector<float, 4> >", %"class.RWTexture2D<vector<float, 4> >"* @"\01?g_screenOutput@@3V?$RWTexture2D@V?$vector@M$03@@@@A", align 4
  %12 = load %b1, %b1* @b1, align 4
  %13 = load %HitShaderConstants, %HitShaderConstants* @HitShaderConstants, align 4
  %b1115 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.b1(i32 160, %b1 %12)
  %HitShaderConstants114 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.HitShaderConstants(i32 160, %HitShaderConstants %13)
  %14 = alloca [3 x float], align 4
  %15 = alloca [3 x float], align 4
  %16 = alloca [3 x float], align 4
  %17 = alloca [3 x float], align 4
  %18 = alloca [3 x float], align 4
  %19 = alloca %struct.RayPayload, align 8
  %RayTCurrent = call float @dx.op.rayTCurrent.f32(i32 154)
  %20 = getelementptr inbounds %struct.RayPayload, %struct.RayPayload* %0, i32 0, i32 1
  store float %RayTCurrent, float* %20, align 4, !tbaa !223
  %21 = getelementptr inbounds %struct.RayPayload, %struct.RayPayload* %0, i32 0, i32 0
  %22 = load i32, i32* %21, align 1, !tbaa !328, !range !330
  %23 = icmp eq i32 %22, 0
  br i1 %23, label %"\01?Load3x16BitIndices@@YA?AV?$vector@I$02@@I@Z.exit", label %403

"\01?Load3x16BitIndices@@YA?AV?$vector@I$02@@I@Z.exit": ; preds = %2
  %24 = add i32 8, %baseShaderRecordOffset
  %ShaderRecordBuffer = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %ShaderTableHandle, i32 %24, i32 undef)
  %25 = extractvalue %dx.types.ResRet.i32 %ShaderRecordBuffer, 0
  %26 = call %dx.types.Handle @"dx.op.createHandleFromResourceStructForLib.class.StructuredBuffer<RayTraceMeshInfo>"(i32 160, %"class.StructuredBuffer<RayTraceMeshInfo>" %9)
  %27 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %26, i32 %25, i32 0)
  %28 = extractvalue %dx.types.ResRet.i32 %27, 0
  %29 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %26, i32 %25, i32 4)
  %30 = extractvalue %dx.types.ResRet.i32 %29, 0
  %31 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %26, i32 %25, i32 8)
  %32 = extractvalue %dx.types.ResRet.i32 %31, 0
  %33 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %26, i32 %25, i32 12)
  %34 = extractvalue %dx.types.ResRet.i32 %33, 0
  %35 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %26, i32 %25, i32 16)
  %36 = extractvalue %dx.types.ResRet.i32 %35, 0
  %37 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %26, i32 %25, i32 20)
  %38 = extractvalue %dx.types.ResRet.i32 %37, 0
  %39 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %26, i32 %25, i32 24)
  %40 = extractvalue %dx.types.ResRet.i32 %39, 0
  %PrimitiveID82 = call i32 @dx.op.primitiveID.i32(i32 108)
  %41 = mul i32 %PrimitiveID82, 6
  %42 = add i32 %41, %28
  %43 = and i32 %42, -4
  %44 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.ByteAddressBuffer(i32 160, %struct.ByteAddressBuffer %8)
  %45 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %44, i32 %43, i32 undef)
  %46 = extractvalue %dx.types.ResRet.i32 %45, 0
  %47 = extractvalue %dx.types.ResRet.i32 %45, 1
  %48 = icmp eq i32 %43, %42
  %49 = and i32 %46, 65535
  %50 = lshr i32 %46, 16
  %51 = lshr i32 %46, 16
  %52 = lshr i32 %47, 16
  %.sink = select i1 %48, i32 %47, i32 %52
  %.0.i0 = select i1 %48, i32 %49, i32 %51
  %.0.i1.in = select i1 %48, i32 %50, i32 %47
  %.0.i1 = and i32 %.0.i1.in, 65535
  %53 = and i32 %.sink, 65535
  %54 = mul i32 %.0.i0, %38
  %55 = add i32 %54, %30
  %56 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.ByteAddressBuffer(i32 160, %struct.ByteAddressBuffer %7)
  %57 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %56, i32 %55, i32 undef)
  %58 = extractvalue %dx.types.ResRet.i32 %57, 0
  %59 = extractvalue %dx.types.ResRet.i32 %57, 1
  %.i0 = bitcast i32 %58 to float
  %.i1 = bitcast i32 %59 to float
  %60 = mul i32 %.0.i1, %38
  %61 = add i32 %60, %30
  %62 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %56, i32 %61, i32 undef)
  %63 = extractvalue %dx.types.ResRet.i32 %62, 0
  %64 = extractvalue %dx.types.ResRet.i32 %62, 1
  %.i0116 = bitcast i32 %63 to float
  %.i1117 = bitcast i32 %64 to float
  %65 = mul i32 %53, %38
  %66 = add i32 %65, %30
  %67 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %56, i32 %66, i32 undef)
  %68 = extractvalue %dx.types.ResRet.i32 %67, 0
  %69 = extractvalue %dx.types.ResRet.i32 %67, 1
  %.i0118 = bitcast i32 %68 to float
  %.i1119 = bitcast i32 %69 to float
  %70 = getelementptr inbounds %struct.BuiltInTriangleIntersectionAttributes, %struct.BuiltInTriangleIntersectionAttributes* %1, i32 0, i32 0
  %71 = load <2 x float>, <2 x float>* %70, align 4
  %72 = extractelement <2 x float> %71, i32 0
  %73 = fsub fast float 1.000000e+00, %72
  %74 = extractelement <2 x float> %71, i32 1
  %75 = fsub fast float %73, %74
  %.i0120 = fmul fast float %75, %.i0
  %.i1121 = fmul fast float %75, %.i1
  %.i0122 = fmul fast float %72, %.i0116
  %.i1123 = fmul fast float %72, %.i1117
  %.i0126 = fmul fast float %74, %.i0118
  %.i1127 = fmul fast float %74, %.i1119
  %.i0124 = fadd fast float %.i0126, %.i0122
  %.i0128 = fadd fast float %.i0124, %.i0120
  %.i1125 = fadd fast float %.i1127, %.i1123
  %.i1129 = fadd fast float %.i1125, %.i1121
  %76 = add i32 %54, %32
  %77 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.ByteAddressBuffer(i32 160, %struct.ByteAddressBuffer %7)
  %78 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %77, i32 %76, i32 undef)
  %79 = extractvalue %dx.types.ResRet.i32 %78, 0
  %80 = extractvalue %dx.types.ResRet.i32 %78, 1
  %81 = extractvalue %dx.types.ResRet.i32 %78, 2
  %.i0130 = bitcast i32 %79 to float
  %.i1131 = bitcast i32 %80 to float
  %.i2 = bitcast i32 %81 to float
  %82 = add i32 %60, %32
  %83 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %77, i32 %82, i32 undef)
  %84 = extractvalue %dx.types.ResRet.i32 %83, 0
  %85 = extractvalue %dx.types.ResRet.i32 %83, 1
  %86 = extractvalue %dx.types.ResRet.i32 %83, 2
  %.i0132 = bitcast i32 %84 to float
  %.i1133 = bitcast i32 %85 to float
  %.i2134 = bitcast i32 %86 to float
  %87 = add i32 %65, %32
  %88 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %77, i32 %87, i32 undef)
  %89 = extractvalue %dx.types.ResRet.i32 %88, 0
  %90 = extractvalue %dx.types.ResRet.i32 %88, 1
  %91 = extractvalue %dx.types.ResRet.i32 %88, 2
  %.i0135 = bitcast i32 %89 to float
  %.i1136 = bitcast i32 %90 to float
  %.i2137 = bitcast i32 %91 to float
  %.i0138 = fmul fast float %.i0130, %75
  %.i1139 = fmul fast float %.i1131, %75
  %.i2140 = fmul fast float %.i2, %75
  %.i0141 = fmul fast float %.i0132, %72
  %.i1142 = fmul fast float %.i1133, %72
  %.i2143 = fmul fast float %.i2134, %72
  %.i0144 = fadd fast float %.i0141, %.i0138
  %.i1145 = fadd fast float %.i1142, %.i1139
  %.i2146 = fadd fast float %.i2143, %.i2140
  %.i0147 = fmul fast float %.i0135, %74
  %.i1148 = fmul fast float %.i1136, %74
  %.i2149 = fmul fast float %.i2137, %74
  %.i0150 = fadd fast float %.i0144, %.i0147
  %.i1151 = fadd fast float %.i1145, %.i1148
  %.i2152 = fadd fast float %.i2146, %.i2149
  %92 = fmul fast float %.i0150, %.i0150
  %93 = fmul fast float %.i1151, %.i1151
  %94 = fadd fast float %92, %93
  %95 = fmul fast float %.i2152, %.i2152
  %96 = fadd fast float %94, %95
  %Sqrt72 = call float @dx.op.unary.f32(i32 24, float %96)
  %.i0153 = fdiv fast float %.i0150, %Sqrt72
  %.i1154 = fdiv fast float %.i1151, %Sqrt72
  %.i2155 = fdiv fast float %.i2152, %Sqrt72
  %97 = add i32 %54, %34
  %98 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %77, i32 %97, i32 undef)
  %99 = extractvalue %dx.types.ResRet.i32 %98, 0
  %100 = extractvalue %dx.types.ResRet.i32 %98, 1
  %101 = extractvalue %dx.types.ResRet.i32 %98, 2
  %.i0156 = bitcast i32 %99 to float
  %.i1157 = bitcast i32 %100 to float
  %.i2158 = bitcast i32 %101 to float
  %102 = add i32 %60, %34
  %103 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %77, i32 %102, i32 undef)
  %104 = extractvalue %dx.types.ResRet.i32 %103, 0
  %105 = extractvalue %dx.types.ResRet.i32 %103, 1
  %106 = extractvalue %dx.types.ResRet.i32 %103, 2
  %.i0159 = bitcast i32 %104 to float
  %.i1160 = bitcast i32 %105 to float
  %.i2161 = bitcast i32 %106 to float
  %107 = add i32 %65, %34
  %108 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %77, i32 %107, i32 undef)
  %109 = extractvalue %dx.types.ResRet.i32 %108, 0
  %110 = extractvalue %dx.types.ResRet.i32 %108, 1
  %111 = extractvalue %dx.types.ResRet.i32 %108, 2
  %.i0162 = bitcast i32 %109 to float
  %.i1163 = bitcast i32 %110 to float
  %.i2164 = bitcast i32 %111 to float
  %.i0165 = fmul fast float %.i0156, %75
  %.i1166 = fmul fast float %.i1157, %75
  %.i2167 = fmul fast float %.i2158, %75
  %.i0168 = fmul fast float %.i0159, %72
  %.i1169 = fmul fast float %.i1160, %72
  %.i2170 = fmul fast float %.i2161, %72
  %.i0171 = fadd fast float %.i0168, %.i0165
  %.i1172 = fadd fast float %.i1169, %.i1166
  %.i2173 = fadd fast float %.i2170, %.i2167
  %.i0174 = fmul fast float %.i0162, %74
  %.i1175 = fmul fast float %.i1163, %74
  %.i2176 = fmul fast float %.i2164, %74
  %.i0177 = fadd fast float %.i0171, %.i0174
  %.i1178 = fadd fast float %.i1172, %.i1175
  %.i2179 = fadd fast float %.i2173, %.i2176
  %112 = fmul fast float %.i0177, %.i0177
  %113 = fmul fast float %.i1178, %.i1178
  %114 = fadd fast float %112, %113
  %115 = fmul fast float %.i2179, %.i2179
  %116 = fadd fast float %114, %115
  %Sqrt73 = call float @dx.op.unary.f32(i32 24, float %116)
  %.i0180 = fdiv fast float %.i0177, %Sqrt73
  %.i1181 = fdiv fast float %.i1178, %Sqrt73
  %.i2182 = fdiv fast float %.i2179, %Sqrt73
  %117 = add i32 %54, %36
  %118 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %77, i32 %117, i32 undef)
  %119 = extractvalue %dx.types.ResRet.i32 %118, 0
  %120 = extractvalue %dx.types.ResRet.i32 %118, 1
  %121 = extractvalue %dx.types.ResRet.i32 %118, 2
  %.i0183 = bitcast i32 %119 to float
  %.i1184 = bitcast i32 %120 to float
  %.i2185 = bitcast i32 %121 to float
  %122 = add i32 %60, %36
  %123 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.ByteAddressBuffer(i32 160, %struct.ByteAddressBuffer %7)
  %124 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %123, i32 %122, i32 undef)
  %125 = extractvalue %dx.types.ResRet.i32 %124, 0
  %126 = extractvalue %dx.types.ResRet.i32 %124, 1
  %127 = extractvalue %dx.types.ResRet.i32 %124, 2
  %.i0186 = bitcast i32 %125 to float
  %.i1187 = bitcast i32 %126 to float
  %.i2188 = bitcast i32 %127 to float
  %128 = add i32 %65, %36
  %129 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %123, i32 %128, i32 undef)
  %130 = extractvalue %dx.types.ResRet.i32 %129, 0
  %131 = extractvalue %dx.types.ResRet.i32 %129, 1
  %132 = extractvalue %dx.types.ResRet.i32 %129, 2
  %.i0189 = bitcast i32 %130 to float
  %.i1190 = bitcast i32 %131 to float
  %.i2191 = bitcast i32 %132 to float
  %WorldRayOrigin = call float @dx.op.worldRayOrigin.f32(i32 147, i8 0)
  %WorldRayOrigin104 = call float @dx.op.worldRayOrigin.f32(i32 147, i8 1)
  %WorldRayOrigin105 = call float @dx.op.worldRayOrigin.f32(i32 147, i8 2)
  %WorldRayDirection = call float @dx.op.worldRayDirection.f32(i32 148, i8 0)
  %WorldRayDirection99 = call float @dx.op.worldRayDirection.f32(i32 148, i8 1)
  %WorldRayDirection100 = call float @dx.op.worldRayDirection.f32(i32 148, i8 2)
  %RayTCurrent81 = call float @dx.op.rayTCurrent.f32(i32 154)
  %.i0192 = fmul fast float %RayTCurrent81, %WorldRayDirection
  %.i1193 = fmul fast float %RayTCurrent81, %WorldRayDirection99
  %.i2194 = fmul fast float %RayTCurrent81, %WorldRayDirection100
  %.i0195 = fadd fast float %.i0192, %WorldRayOrigin
  %.i1196 = fadd fast float %.i1193, %WorldRayOrigin104
  %.i2197 = fadd fast float %.i2194, %WorldRayOrigin105
  %DispatchRaysIndex = call i32 @dx.op.dispatchRaysIndex.i32(i32 145, i8 0)
  %DispatchRaysIndex106 = call i32 @dx.op.dispatchRaysIndex.i32(i32 145, i8 1)
  %133 = add i32 %DispatchRaysIndex, 1
  %.i0198 = uitofp i32 %133 to float
  %.i1199 = uitofp i32 %DispatchRaysIndex106 to float
  %.i0200 = fadd fast float %.i0198, 5.000000e-01
  %.i1201 = fadd fast float %.i1199, 5.000000e-01
  %134 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %b1115, i32 5)
  %135 = extractvalue %dx.types.CBufRet.f32 %134, 0
  %136 = extractvalue %dx.types.CBufRet.f32 %134, 1
  %.i0202 = fdiv fast float %.i0200, %135
  %.i1203 = fdiv fast float %.i1201, %136
  %.i0204 = fmul fast float %.i0202, 2.000000e+00
  %.i1205 = fmul fast float %.i1203, 2.000000e+00
  %.i0206 = fadd fast float %.i0204, -1.000000e+00
  %.i1207 = fadd fast float %.i1205, -1.000000e+00
  %137 = fsub fast float -0.000000e+00, %.i1207
  %138 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %b1115, i32 0)
  %139 = extractvalue %dx.types.CBufRet.f32 %138, 0
  %140 = extractvalue %dx.types.CBufRet.f32 %138, 1
  %141 = extractvalue %dx.types.CBufRet.f32 %138, 2
  %142 = extractvalue %dx.types.CBufRet.f32 %138, 3
  %143 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %b1115, i32 1)
  %144 = extractvalue %dx.types.CBufRet.f32 %143, 0
  %145 = extractvalue %dx.types.CBufRet.f32 %143, 1
  %146 = extractvalue %dx.types.CBufRet.f32 %143, 2
  %147 = extractvalue %dx.types.CBufRet.f32 %143, 3
  %148 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %b1115, i32 2)
  %149 = extractvalue %dx.types.CBufRet.f32 %148, 0
  %150 = extractvalue %dx.types.CBufRet.f32 %148, 1
  %151 = extractvalue %dx.types.CBufRet.f32 %148, 2
  %152 = extractvalue %dx.types.CBufRet.f32 %148, 3
  %153 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %b1115, i32 3)
  %154 = extractvalue %dx.types.CBufRet.f32 %153, 0
  %155 = extractvalue %dx.types.CBufRet.f32 %153, 1
  %156 = extractvalue %dx.types.CBufRet.f32 %153, 2
  %157 = extractvalue %dx.types.CBufRet.f32 %153, 3
  %158 = fmul fast float %139, %.i0206
  %FMad33 = call float @dx.op.tertiary.f32(i32 46, float %137, float %140, float %158)
  %FMad32 = call float @dx.op.tertiary.f32(i32 46, float 0.000000e+00, float %141, float %FMad33)
  %FMad31 = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %142, float %FMad32)
  %159 = fmul fast float %144, %.i0206
  %FMad30 = call float @dx.op.tertiary.f32(i32 46, float %137, float %145, float %159)
  %FMad29 = call float @dx.op.tertiary.f32(i32 46, float 0.000000e+00, float %146, float %FMad30)
  %FMad28 = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %147, float %FMad29)
  %160 = fmul fast float %149, %.i0206
  %FMad27 = call float @dx.op.tertiary.f32(i32 46, float %137, float %150, float %160)
  %FMad26 = call float @dx.op.tertiary.f32(i32 46, float 0.000000e+00, float %151, float %FMad27)
  %FMad25 = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %152, float %FMad26)
  %161 = fmul fast float %154, %.i0206
  %FMad24 = call float @dx.op.tertiary.f32(i32 46, float %137, float %155, float %161)
  %FMad23 = call float @dx.op.tertiary.f32(i32 46, float 0.000000e+00, float %156, float %FMad24)
  %FMad22 = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %157, float %FMad23)
  %.i0208 = fdiv fast float %FMad31, %FMad22
  %.i1209 = fdiv fast float %FMad28, %FMad22
  %.i2210 = fdiv fast float %FMad25, %FMad22
  %162 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %b1115, i32 4)
  %163 = extractvalue %dx.types.CBufRet.f32 %162, 0
  %164 = extractvalue %dx.types.CBufRet.f32 %162, 1
  %165 = extractvalue %dx.types.CBufRet.f32 %162, 2
  %.i0211 = fsub fast float %.i0208, %163
  %.i1212 = fsub fast float %.i1209, %164
  %.i2213 = fsub fast float %.i2210, %165
  %166 = fmul fast float %.i0211, %.i0211
  %167 = fmul fast float %.i1212, %.i1212
  %168 = fadd fast float %166, %167
  %169 = fmul fast float %.i2213, %.i2213
  %170 = fadd fast float %168, %169
  %Sqrt71 = call float @dx.op.unary.f32(i32 24, float %170)
  %.i0214 = fdiv fast float %.i0211, %Sqrt71
  %.i1215 = fdiv fast float %.i1212, %Sqrt71
  %.i2216 = fdiv fast float %.i2213, %Sqrt71
  %171 = add i32 %DispatchRaysIndex106, 1
  %.i0217 = uitofp i32 %DispatchRaysIndex to float
  %.i1218 = uitofp i32 %171 to float
  %.i0219 = fadd fast float %.i0217, 5.000000e-01
  %.i1220 = fadd fast float %.i1218, 5.000000e-01
  %.i0221 = fdiv fast float %.i0219, %135
  %.i1222 = fdiv fast float %.i1220, %136
  %.i0223 = fmul fast float %.i0221, 2.000000e+00
  %.i1224 = fmul fast float %.i1222, 2.000000e+00
  %.i0225 = fadd fast float %.i0223, -1.000000e+00
  %.i1226 = fadd fast float %.i1224, -1.000000e+00
  %172 = fsub fast float -0.000000e+00, %.i1226
  %173 = fmul fast float %139, %.i0225
  %FMad65 = call float @dx.op.tertiary.f32(i32 46, float %172, float %140, float %173)
  %FMad64 = call float @dx.op.tertiary.f32(i32 46, float 0.000000e+00, float %141, float %FMad65)
  %FMad63 = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %142, float %FMad64)
  %174 = fmul fast float %144, %.i0225
  %FMad62 = call float @dx.op.tertiary.f32(i32 46, float %172, float %145, float %174)
  %FMad61 = call float @dx.op.tertiary.f32(i32 46, float 0.000000e+00, float %146, float %FMad62)
  %FMad60 = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %147, float %FMad61)
  %175 = fmul fast float %149, %.i0225
  %FMad59 = call float @dx.op.tertiary.f32(i32 46, float %172, float %150, float %175)
  %FMad58 = call float @dx.op.tertiary.f32(i32 46, float 0.000000e+00, float %151, float %FMad59)
  %FMad57 = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %152, float %FMad58)
  %176 = fmul fast float %154, %.i0225
  %FMad56 = call float @dx.op.tertiary.f32(i32 46, float %172, float %155, float %176)
  %FMad55 = call float @dx.op.tertiary.f32(i32 46, float 0.000000e+00, float %156, float %FMad56)
  %FMad54 = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %157, float %FMad55)
  %.i0227 = fdiv fast float %FMad63, %FMad54
  %.i1228 = fdiv fast float %FMad60, %FMad54
  %.i2229 = fdiv fast float %FMad57, %FMad54
  %.i0230 = fsub fast float %.i0227, %163
  %.i1231 = fsub fast float %.i1228, %164
  %.i2232 = fsub fast float %.i2229, %165
  %177 = fmul fast float %.i0230, %.i0230
  %178 = fmul fast float %.i1231, %.i1231
  %179 = fadd fast float %177, %178
  %180 = fmul fast float %.i2232, %.i2232
  %181 = fadd fast float %179, %180
  %Sqrt70 = call float @dx.op.unary.f32(i32 24, float %181)
  %.i0233 = fdiv fast float %.i0230, %Sqrt70
  %.i1234 = fdiv fast float %.i1231, %Sqrt70
  %.i2235 = fdiv fast float %.i2232, %Sqrt70
  %.i0236 = fsub fast float %.i0186, %.i0183
  %.i1237 = fsub fast float %.i1187, %.i1184
  %.i2238 = fsub fast float %.i2188, %.i2185
  %.i0239 = fsub fast float %.i0189, %.i0183
  %.i1240 = fsub fast float %.i1190, %.i1184
  %.i2241 = fsub fast float %.i2191, %.i2185
  %182 = fmul fast float %.i1240, %.i2238
  %183 = fmul fast float %.i2241, %.i1237
  %184 = fsub fast float %182, %183
  %185 = fmul fast float %.i2241, %.i0236
  %186 = fmul fast float %.i0239, %.i2238
  %187 = fsub fast float %185, %186
  %188 = fmul fast float %.i0239, %.i1237
  %189 = fmul fast float %.i1240, %.i0236
  %190 = fsub fast float %188, %189
  %191 = fmul fast float %184, %184
  %192 = fmul fast float %187, %187
  %193 = fadd fast float %191, %192
  %194 = fmul fast float %190, %190
  %195 = fadd fast float %193, %194
  %Sqrt74 = call float @dx.op.unary.f32(i32 24, float %195)
  %.i0242 = fdiv fast float %184, %Sqrt74
  %.i1243 = fdiv fast float %187, %Sqrt74
  %.i2244 = fdiv fast float %190, %Sqrt74
  %.i0245 = fsub fast float %163, %.i0195
  %.i1246 = fsub fast float %164, %.i1196
  %.i2247 = fsub fast float %165, %.i2197
  %.i0248 = fsub fast float -0.000000e+00, %.i0242
  %.i1249 = fsub fast float -0.000000e+00, %.i1243
  %.i2250 = fsub fast float -0.000000e+00, %.i2244
  %196 = call float @dx.op.dot3.f32(i32 55, float %.i0248, float %.i1249, float %.i2250, float %.i0245, float %.i1246, float %.i2247)
  %197 = call float @dx.op.dot3.f32(i32 55, float %.i0242, float %.i1243, float %.i2244, float %.i0214, float %.i1215, float %.i2216)
  %198 = fdiv fast float %196, %197
  %.i0251 = fmul fast float %198, %.i0214
  %.i1252 = fmul fast float %198, %.i1215
  %.i2253 = fmul fast float %198, %.i2216
  %.i0254 = fadd fast float %.i0251, %163
  %.i1255 = fadd fast float %.i1252, %164
  %.i2256 = fadd fast float %.i2253, %165
  %199 = call float @dx.op.dot3.f32(i32 55, float %.i0242, float %.i1243, float %.i2244, float %.i0233, float %.i1234, float %.i2235)
  %200 = fdiv fast float %196, %199
  %.i0263 = fmul fast float %200, %.i0233
  %.i1264 = fmul fast float %200, %.i1234
  %.i2265 = fmul fast float %200, %.i2235
  %.i0266 = fadd fast float %.i0263, %163
  %.i1267 = fadd fast float %.i1264, %164
  %.i2268 = fadd fast float %.i2265, %165
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
  %201 = fmul fast float %.i0269, %.i1272
  %202 = fmul fast float %.i0271, %.i1270
  %203 = fsub fast float %201, %202
  %204 = fdiv fast float 1.000000e+00, %203
  %.i0279 = fmul fast float %204, %.i1272
  %205 = fmul fast float %.i0271, %204
  %.i1280 = fsub fast float -0.000000e+00, %205
  %206 = fmul fast float %.i1270, %204
  %.i2281 = fsub fast float -0.000000e+00, %206
  %.i3 = fmul fast float %204, %.i0269
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
  %207 = getelementptr inbounds [3 x float], [3 x float]* %14, i32 0, i32 0
  store float %.i0266, float* %207, align 4
  %208 = getelementptr inbounds [3 x float], [3 x float]* %14, i32 0, i32 1
  store float %.i1267, float* %208, align 4
  %209 = getelementptr inbounds [3 x float], [3 x float]* %14, i32 0, i32 2
  store float %.i2268, float* %209, align 4
  %210 = getelementptr inbounds [3 x float], [3 x float]* %15, i32 0, i32 0
  store float %.i0254, float* %210, align 4
  %211 = getelementptr inbounds [3 x float], [3 x float]* %15, i32 0, i32 1
  store float %.i1255, float* %211, align 4
  %212 = getelementptr inbounds [3 x float], [3 x float]* %15, i32 0, i32 2
  store float %.i2256, float* %212, align 4
  %213 = getelementptr inbounds [3 x float], [3 x float]* %16, i32 0, i32 0
  store float %.i0195, float* %213, align 4
  %214 = getelementptr inbounds [3 x float], [3 x float]* %16, i32 0, i32 1
  store float %.i1196, float* %214, align 4
  %215 = getelementptr inbounds [3 x float], [3 x float]* %16, i32 0, i32 2
  store float %.i2197, float* %215, align 4
  %216 = getelementptr inbounds [3 x float], [3 x float]* %17, i32 0, i32 0
  store float %.i0297, float* %216, align 4
  %217 = getelementptr inbounds [3 x float], [3 x float]* %17, i32 0, i32 1
  store float %.i1298, float* %217, align 4
  %218 = getelementptr inbounds [3 x float], [3 x float]* %17, i32 0, i32 2
  store float %.i2299, float* %218, align 4
  %219 = getelementptr inbounds [3 x float], [3 x float]* %18, i32 0, i32 0
  store float %.i0288, float* %219, align 4
  %220 = getelementptr inbounds [3 x float], [3 x float]* %18, i32 0, i32 1
  store float %.i1289, float* %220, align 4
  %221 = getelementptr inbounds [3 x float], [3 x float]* %18, i32 0, i32 2
  store float %.i2290, float* %221, align 4
  %FAbs = call float @dx.op.unary.f32(i32 6, float %.i0242)
  %FAbs68 = call float @dx.op.unary.f32(i32 6, float %.i1243)
  %FAbs69 = call float @dx.op.unary.f32(i32 6, float %.i2244)
  %222 = fcmp fast ogt float %FAbs, %FAbs68
  %223 = fcmp fast ogt float %FAbs, %FAbs69
  %224 = and i1 %222, %223
  br i1 %224, label %"\01?CalculateUVDerivatives@@YAXV?$vector@M$02@@00000AIAV?$vector@M$01@@1@Z.exit", label %225

; <label>:225                                     ; preds = %"\01?Load3x16BitIndices@@YA?AV?$vector@I$02@@I@Z.exit"
  %226 = fcmp fast ogt float %FAbs68, %FAbs69
  br i1 %226, label %"\01?CalculateUVDerivatives@@YAXV?$vector@M$02@@00000AIAV?$vector@M$01@@1@Z.exit", label %227

; <label>:227                                     ; preds = %225
  br label %"\01?CalculateUVDerivatives@@YAXV?$vector@M$02@@00000AIAV?$vector@M$01@@1@Z.exit"

"\01?CalculateUVDerivatives@@YAXV?$vector@M$02@@00000AIAV?$vector@M$01@@1@Z.exit": ; preds = %227, %225, %"\01?Load3x16BitIndices@@YA?AV?$vector@I$02@@I@Z.exit"
  %228 = phi float [ %.i1267, %227 ], [ %.i2268, %"\01?Load3x16BitIndices@@YA?AV?$vector@I$02@@I@Z.exit" ], [ %.i2268, %225 ]
  %229 = phi float [ %.i1196, %227 ], [ %.i2197, %"\01?Load3x16BitIndices@@YA?AV?$vector@I$02@@I@Z.exit" ], [ %.i2197, %225 ]
  %230 = phi float [ %.i1255, %227 ], [ %.i2256, %"\01?Load3x16BitIndices@@YA?AV?$vector@I$02@@I@Z.exit" ], [ %.i2256, %225 ]
  %231 = phi float [ %.i1298, %227 ], [ %.i2299, %"\01?Load3x16BitIndices@@YA?AV?$vector@I$02@@I@Z.exit" ], [ %.i2299, %225 ]
  %232 = phi float [ %.i1289, %227 ], [ %.i2290, %"\01?Load3x16BitIndices@@YA?AV?$vector@I$02@@I@Z.exit" ], [ %.i2290, %225 ]
  %indices.i.0.i0 = phi i32 [ 0, %227 ], [ 1, %"\01?Load3x16BitIndices@@YA?AV?$vector@I$02@@I@Z.exit" ], [ 0, %225 ]
  %233 = getelementptr [3 x float], [3 x float]* %18, i32 0, i32 %indices.i.0.i0
  %234 = load float, float* %233, align 4, !tbaa !223, !noalias !331
  %235 = getelementptr [3 x float], [3 x float]* %17, i32 0, i32 %indices.i.0.i0
  %236 = load float, float* %235, align 4, !tbaa !223, !noalias !331
  %237 = fmul fast float %231, %234
  %238 = fmul fast float %232, %236
  %239 = fsub fast float %237, %238
  %240 = fdiv fast float 1.000000e+00, %239
  %.i0300 = fmul fast float %240, %231
  %241 = fmul fast float %232, %240
  %.i1301 = fsub fast float -0.000000e+00, %241
  %242 = fmul fast float %236, %240
  %.i2302 = fsub fast float -0.000000e+00, %242
  %.i3303 = fmul fast float %240, %234
  %243 = getelementptr [3 x float], [3 x float]* %15, i32 0, i32 %indices.i.0.i0
  %244 = load float, float* %243, align 4, !tbaa !223, !noalias !331
  %245 = getelementptr [3 x float], [3 x float]* %16, i32 0, i32 %indices.i.0.i0
  %246 = load float, float* %245, align 4, !tbaa !223, !noalias !331
  %247 = fsub fast float %244, %246
  %248 = fsub fast float %230, %229
  %249 = fmul fast float %247, %.i0300
  %FMad21 = call float @dx.op.tertiary.f32(i32 46, float %248, float %.i1301, float %249)
  %250 = fmul fast float %247, %.i2302
  %FMad = call float @dx.op.tertiary.f32(i32 46, float %248, float %.i3303, float %250)
  %FAbs79 = call float @dx.op.unary.f32(i32 6, float %FMad21)
  %FAbs80 = call float @dx.op.unary.f32(i32 6, float %FMad)
  %251 = getelementptr [3 x float], [3 x float]* %14, i32 0, i32 %indices.i.0.i0
  %252 = load float, float* %251, align 4, !tbaa !223, !noalias !331
  %253 = fsub fast float %252, %246
  %254 = fsub fast float %228, %229
  %255 = fmul fast float %253, %.i0300
  %FMad53 = call float @dx.op.tertiary.f32(i32 46, float %254, float %.i1301, float %255)
  %256 = fmul fast float %253, %.i2302
  %FMad52 = call float @dx.op.tertiary.f32(i32 46, float %254, float %.i3303, float %256)
  %FAbs77 = call float @dx.op.unary.f32(i32 6, float %FMad53)
  %FAbs78 = call float @dx.op.unary.f32(i32 6, float %FMad52)
  %.i0304 = fsub fast float -0.000000e+00, %WorldRayDirection
  %.i1305 = fsub fast float -0.000000e+00, %WorldRayDirection99
  %.i2306 = fsub fast float -0.000000e+00, %WorldRayDirection100
  %257 = fmul fast float %WorldRayDirection, %WorldRayDirection
  %258 = fmul fast float %WorldRayDirection99, %WorldRayDirection99
  %259 = fadd fast float %258, %257
  %260 = fmul fast float %WorldRayDirection100, %WorldRayDirection100
  %261 = fadd fast float %259, %260
  %Sqrt75 = call float @dx.op.unary.f32(i32 24, float %261)
  %.i0307 = fdiv fast float %.i0304, %Sqrt75
  %.i1308 = fdiv fast float %.i1305, %Sqrt75
  %.i2309 = fdiv fast float %.i2306, %Sqrt75
  %262 = getelementptr inbounds [27 x %"class.Texture2D<vector<float, 4> >"], [27 x %"class.Texture2D<vector<float, 4> >"]* @"\01?g_texDiffuse@@3PAV?$Texture2D@V?$vector@M$03@@@@A", i32 0, i32 %40, !dx.nonuniform !228
  %263 = load %"class.Texture2D<vector<float, 4> >", %"class.Texture2D<vector<float, 4> >"* %262, align 4
  %264 = call %dx.types.Handle @"dx.op.createHandleFromResourceStructForLib.class.Texture2D<vector<float, 4> >"(i32 160, %"class.Texture2D<vector<float, 4> >" %263)
  %265 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.SamplerState(i32 160, %struct.SamplerState %4)
  %266 = call %dx.types.ResRet.f32 @dx.op.sampleGrad.f32(i32 63, %dx.types.Handle %264, %dx.types.Handle %265, float %.i0128, float %.i1129, float undef, float undef, i32 undef, i32 undef, i32 undef, float %FAbs79, float %FAbs80, float undef, float %FAbs77, float %FAbs78, float undef, float undef)
  %267 = extractvalue %dx.types.ResRet.f32 %266, 0
  %268 = extractvalue %dx.types.ResRet.f32 %266, 1
  %269 = extractvalue %dx.types.ResRet.f32 %266, 2
  %270 = getelementptr inbounds [27 x %"class.Texture2D<vector<float, 4> >"], [27 x %"class.Texture2D<vector<float, 4> >"]* @"\01?g_texNormal@@3PAV?$Texture2D@V?$vector@M$03@@@@A", i32 0, i32 %40, !dx.nonuniform !228
  %271 = load %"class.Texture2D<vector<float, 4> >", %"class.Texture2D<vector<float, 4> >"* %270, align 4
  %272 = call %dx.types.Handle @"dx.op.createHandleFromResourceStructForLib.class.Texture2D<vector<float, 4> >"(i32 160, %"class.Texture2D<vector<float, 4> >" %271)
  %273 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.SamplerState(i32 160, %struct.SamplerState %4)
  %274 = call %dx.types.ResRet.f32 @dx.op.sampleGrad.f32(i32 63, %dx.types.Handle %272, %dx.types.Handle %273, float %.i0128, float %.i1129, float undef, float undef, i32 undef, i32 undef, i32 undef, float %FAbs79, float %FAbs80, float undef, float %FAbs77, float %FAbs78, float undef, float undef)
  %275 = extractvalue %dx.types.ResRet.f32 %274, 0
  %276 = extractvalue %dx.types.ResRet.f32 %274, 1
  %277 = extractvalue %dx.types.ResRet.f32 %274, 2
  %.i0310 = fmul fast float %275, 2.000000e+00
  %.i1311 = fmul fast float %276, 2.000000e+00
  %.i2312 = fmul fast float %277, 2.000000e+00
  %.i0313 = fadd fast float %.i0310, -1.000000e+00
  %.i1314 = fadd fast float %.i1311, -1.000000e+00
  %.i2315 = fadd fast float %.i2312, -1.000000e+00
  %278 = call float @dx.op.dot3.f32(i32 55, float %.i0313, float %.i1314, float %.i2315, float %.i0313, float %.i1314, float %.i2315)
  %Rsqrt = call float @dx.op.unary.f32(i32 25, float %278)
  %.i0316 = fmul fast float %.i0313, %Rsqrt
  %.i1317 = fmul fast float %.i1314, %Rsqrt
  %.i2318 = fmul fast float %.i2315, %Rsqrt
  %279 = fmul fast float %.i2182, %.i1154
  %280 = fmul fast float %.i1181, %.i2155
  %281 = fsub fast float %279, %280
  %282 = fmul fast float %.i0180, %.i2155
  %283 = fmul fast float %.i2182, %.i0153
  %284 = fsub fast float %282, %283
  %285 = fmul fast float %.i1181, %.i0153
  %286 = fmul fast float %.i0180, %.i1154
  %287 = fsub fast float %285, %286
  %288 = fmul fast float %.i0316, %.i0180
  %FMad51 = call float @dx.op.tertiary.f32(i32 46, float %.i1317, float %281, float %288)
  %FMad50 = call float @dx.op.tertiary.f32(i32 46, float %.i2318, float %.i0153, float %FMad51)
  %289 = fmul fast float %.i0316, %.i1181
  %FMad49 = call float @dx.op.tertiary.f32(i32 46, float %.i1317, float %284, float %289)
  %FMad48 = call float @dx.op.tertiary.f32(i32 46, float %.i2318, float %.i1154, float %FMad49)
  %290 = fmul fast float %.i0316, %.i2182
  %FMad47 = call float @dx.op.tertiary.f32(i32 46, float %.i1317, float %287, float %290)
  %FMad46 = call float @dx.op.tertiary.f32(i32 46, float %.i2318, float %.i2155, float %FMad47)
  %291 = fmul fast float %FMad50, %FMad50
  %292 = fmul fast float %FMad48, %FMad48
  %293 = fadd fast float %292, %291
  %294 = fmul fast float %FMad46, %FMad46
  %295 = fadd fast float %293, %294
  %Sqrt76 = call float @dx.op.unary.f32(i32 24, float %295)
  %.i0319 = fdiv fast float %FMad50, %Sqrt76
  %.i1320 = fdiv fast float %FMad48, %Sqrt76
  %.i2321 = fdiv fast float %FMad46, %Sqrt76
  %296 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %HitShaderConstants114, i32 2)
  %297 = extractvalue %dx.types.CBufRet.f32 %296, 0
  %298 = extractvalue %dx.types.CBufRet.f32 %296, 1
  %299 = extractvalue %dx.types.CBufRet.f32 %296, 2
  %.i0322 = fmul fast float %297, %267
  %.i1323 = fmul fast float %298, %268
  %.i2324 = fmul fast float %299, %269
  %300 = call %dx.types.Handle @"dx.op.createHandleFromResourceStructForLib.class.Texture2D<float>"(i32 160, %"class.Texture2D<float>" %5)
  %TextureLoad = call %dx.types.ResRet.f32 @dx.op.textureLoad.f32(i32 66, %dx.types.Handle %300, i32 0, i32 %DispatchRaysIndex, i32 %DispatchRaysIndex106, i32 undef, i32 undef, i32 undef, i32 undef)
  %301 = extractvalue %dx.types.ResRet.f32 %TextureLoad, 0
  %.i0325 = fmul fast float %.i0322, %301
  %.i1326 = fmul fast float %.i1323, %301
  %.i2327 = fmul fast float %.i2324, %301
  %302 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %HitShaderConstants114, i32 8)
  %303 = extractvalue %dx.types.CBufRet.i32 %302, 1
  %304 = icmp eq i32 %303, 0
  br i1 %304, label %316, label %305

; <label>:305                                     ; preds = %"\01?CalculateUVDerivatives@@YAXV?$vector@M$02@@00000AIAV?$vector@M$01@@1@Z.exit"
  %306 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %HitShaderConstants114, i32 0)
  %307 = extractvalue %dx.types.CBufRet.f32 %306, 0
  %308 = extractvalue %dx.types.CBufRet.f32 %306, 1
  %309 = extractvalue %dx.types.CBufRet.f32 %306, 2
  %310 = getelementptr inbounds %struct.RayPayload, %struct.RayPayload* %19, i32 0, i32 0
  store i32 1, i32* %310, align 8
  %311 = getelementptr inbounds %struct.RayPayload, %struct.RayPayload* %19, i32 0, i32 1
  store float 0x47EFFFFFE0000000, float* %311, align 4
  %312 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RaytracingAccelerationStructure(i32 160, %struct.RaytracingAccelerationStructure %10)
  call void @dx.op.traceRay.struct.RayPayload(i32 157, %dx.types.Handle %312, i32 4, i32 -1, i32 0, i32 1, i32 0, float %.i0195, float %.i1196, float %.i2197, float 0x3FB99999A0000000, float %307, float %308, float %309, float 0x47EFFFFFE0000000, %struct.RayPayload* nonnull %19)
  %313 = load float, float* %311, align 4
  %314 = fcmp fast olt float %313, 0x47EFFFFFE0000000
  br i1 %314, label %315, label %372

; <label>:315                                     ; preds = %305
  br label %372

; <label>:316                                     ; preds = %"\01?CalculateUVDerivatives@@YAXV?$vector@M$02@@00000AIAV?$vector@M$01@@1@Z.exit"
  %317 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %HitShaderConstants114, i32 4)
  %318 = extractvalue %dx.types.CBufRet.f32 %317, 0
  %319 = extractvalue %dx.types.CBufRet.f32 %317, 1
  %320 = extractvalue %dx.types.CBufRet.f32 %317, 2
  %321 = extractvalue %dx.types.CBufRet.f32 %317, 3
  %322 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %HitShaderConstants114, i32 5)
  %323 = extractvalue %dx.types.CBufRet.f32 %322, 0
  %324 = extractvalue %dx.types.CBufRet.f32 %322, 1
  %325 = extractvalue %dx.types.CBufRet.f32 %322, 2
  %326 = extractvalue %dx.types.CBufRet.f32 %322, 3
  %327 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %HitShaderConstants114, i32 6)
  %328 = extractvalue %dx.types.CBufRet.f32 %327, 0
  %329 = extractvalue %dx.types.CBufRet.f32 %327, 1
  %330 = extractvalue %dx.types.CBufRet.f32 %327, 2
  %331 = extractvalue %dx.types.CBufRet.f32 %327, 3
  %332 = fmul fast float %318, %.i0195
  %FMad45 = call float @dx.op.tertiary.f32(i32 46, float %.i1196, float %319, float %332)
  %FMad44 = call float @dx.op.tertiary.f32(i32 46, float %.i2197, float %320, float %FMad45)
  %FMad43 = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %321, float %FMad44)
  %333 = fmul fast float %323, %.i0195
  %FMad42 = call float @dx.op.tertiary.f32(i32 46, float %.i1196, float %324, float %333)
  %FMad41 = call float @dx.op.tertiary.f32(i32 46, float %.i2197, float %325, float %FMad42)
  %FMad40 = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %326, float %FMad41)
  %334 = fmul fast float %328, %.i0195
  %FMad39 = call float @dx.op.tertiary.f32(i32 46, float %.i1196, float %329, float %334)
  %FMad38 = call float @dx.op.tertiary.f32(i32 46, float %.i2197, float %330, float %FMad39)
  %FMad37 = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %331, float %FMad38)
  %335 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %HitShaderConstants114, i32 3)
  %336 = extractvalue %dx.types.CBufRet.f32 %335, 0
  %337 = fmul fast float %336, 2.500000e-01
  %338 = fmul fast float %336, 1.750000e+00
  %339 = fmul fast float %336, 1.250000e+00
  %340 = fmul fast float %336, 7.500000e-01
  %341 = call %dx.types.Handle @"dx.op.createHandleFromResourceStructForLib.class.Texture2D<float>"(i32 160, %"class.Texture2D<float>" %6)
  %342 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.SamplerComparisonState(i32 160, %struct.SamplerComparisonState %3)
  %343 = call %dx.types.ResRet.f32 @dx.op.sampleCmpLevelZero.f32(i32 65, %dx.types.Handle %341, %dx.types.Handle %342, float %FMad43, float %FMad40, float undef, float undef, i32 undef, i32 undef, i32 undef, float %FMad37)
  %344 = extractvalue %dx.types.ResRet.f32 %343, 0
  %345 = fmul fast float %344, 2.000000e+00
  %.i0328 = fsub fast float %FMad43, %338
  %.i1329 = fadd fast float %337, %FMad40
  %346 = call %dx.types.ResRet.f32 @dx.op.sampleCmpLevelZero.f32(i32 65, %dx.types.Handle %341, %dx.types.Handle %342, float %.i0328, float %.i1329, float undef, float undef, i32 undef, i32 undef, i32 undef, float %FMad37)
  %347 = extractvalue %dx.types.ResRet.f32 %346, 0
  %348 = fadd fast float %347, %345
  %.i0330 = fsub fast float %FMad43, %337
  %.i1331 = fsub fast float %FMad40, %338
  %349 = call %dx.types.ResRet.f32 @dx.op.sampleCmpLevelZero.f32(i32 65, %dx.types.Handle %341, %dx.types.Handle %342, float %.i0330, float %.i1331, float undef, float undef, i32 undef, i32 undef, i32 undef, float %FMad37)
  %350 = extractvalue %dx.types.ResRet.f32 %349, 0
  %351 = fadd fast float %348, %350
  %.i0332 = fadd fast float %338, %FMad43
  %.i1333 = fsub fast float %FMad40, %337
  %352 = call %dx.types.ResRet.f32 @dx.op.sampleCmpLevelZero.f32(i32 65, %dx.types.Handle %341, %dx.types.Handle %342, float %.i0332, float %.i1333, float undef, float undef, i32 undef, i32 undef, i32 undef, float %FMad37)
  %353 = extractvalue %dx.types.ResRet.f32 %352, 0
  %354 = fadd fast float %351, %353
  %.i0334 = fadd fast float %337, %FMad43
  %.i1335 = fadd fast float %338, %FMad40
  %355 = call %dx.types.ResRet.f32 @dx.op.sampleCmpLevelZero.f32(i32 65, %dx.types.Handle %341, %dx.types.Handle %342, float %.i0334, float %.i1335, float undef, float undef, i32 undef, i32 undef, i32 undef, float %FMad37)
  %356 = extractvalue %dx.types.ResRet.f32 %355, 0
  %357 = fadd fast float %354, %356
  %.i0336 = fsub fast float %FMad43, %340
  %.i1337 = fadd fast float %339, %FMad40
  %358 = call %dx.types.ResRet.f32 @dx.op.sampleCmpLevelZero.f32(i32 65, %dx.types.Handle %341, %dx.types.Handle %342, float %.i0336, float %.i1337, float undef, float undef, i32 undef, i32 undef, i32 undef, float %FMad37)
  %359 = extractvalue %dx.types.ResRet.f32 %358, 0
  %360 = fadd fast float %357, %359
  %.i0338 = fsub fast float %FMad43, %339
  %.i1339 = fsub fast float %FMad40, %340
  %361 = call %dx.types.ResRet.f32 @dx.op.sampleCmpLevelZero.f32(i32 65, %dx.types.Handle %341, %dx.types.Handle %342, float %.i0338, float %.i1339, float undef, float undef, i32 undef, i32 undef, i32 undef, float %FMad37)
  %362 = extractvalue %dx.types.ResRet.f32 %361, 0
  %363 = fadd fast float %360, %362
  %.i0340 = fadd fast float %340, %FMad43
  %.i1341 = fsub fast float %FMad40, %339
  %364 = call %dx.types.ResRet.f32 @dx.op.sampleCmpLevelZero.f32(i32 65, %dx.types.Handle %341, %dx.types.Handle %342, float %.i0340, float %.i1341, float undef, float undef, i32 undef, i32 undef, i32 undef, float %FMad37)
  %365 = extractvalue %dx.types.ResRet.f32 %364, 0
  %366 = fadd fast float %363, %365
  %.i0342 = fadd fast float %339, %FMad43
  %.i1343 = fadd fast float %340, %FMad40
  %367 = call %dx.types.ResRet.f32 @dx.op.sampleCmpLevelZero.f32(i32 65, %dx.types.Handle %341, %dx.types.Handle %342, float %.i0342, float %.i1343, float undef, float undef, i32 undef, i32 undef, i32 undef, float %FMad37)
  %368 = extractvalue %dx.types.ResRet.f32 %367, 0
  %369 = fadd fast float %366, %368
  %370 = fmul fast float %369, 0x3FB99999A0000000
  %371 = fmul fast float %370, %370
  br label %372

; <label>:372                                     ; preds = %316, %315, %305
  %shadow.0 = phi float [ 0.000000e+00, %315 ], [ 1.000000e+00, %305 ], [ %371, %316 ]
  %373 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %HitShaderConstants114, i32 1)
  %374 = extractvalue %dx.types.CBufRet.f32 %373, 0
  %375 = extractvalue %dx.types.CBufRet.f32 %373, 1
  %376 = extractvalue %dx.types.CBufRet.f32 %373, 2
  %377 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %HitShaderConstants114, i32 0)
  %378 = extractvalue %dx.types.CBufRet.f32 %377, 0
  %379 = extractvalue %dx.types.CBufRet.f32 %377, 1
  %380 = extractvalue %dx.types.CBufRet.f32 %377, 2
  %.i0344 = fsub fast float %378, %.i0307
  %.i1345 = fsub fast float %379, %.i1308
  %.i2346 = fsub fast float %380, %.i2309
  %381 = fmul fast float %.i0344, %.i0344
  %382 = fmul fast float %.i1345, %.i1345
  %383 = fadd fast float %381, %382
  %384 = fmul fast float %.i2346, %.i2346
  %385 = fadd fast float %383, %384
  %Sqrt = call float @dx.op.unary.f32(i32 24, float %385)
  %.i0347 = fdiv fast float %.i0344, %Sqrt
  %.i1348 = fdiv fast float %.i1345, %Sqrt
  %.i2349 = fdiv fast float %.i2346, %Sqrt
  %386 = call float @dx.op.dot3.f32(i32 55, float %378, float %379, float %380, float %.i0347, float %.i1348, float %.i2349)
  %Saturate19 = call float @dx.op.unary.f32(i32 7, float %386)
  %387 = fsub fast float 1.000000e+00, %Saturate19
  %Log66 = call float @dx.op.unary.f32(i32 23, float %387)
  %388 = fmul fast float %Log66, 5.000000e+00
  %Exp67 = call float @dx.op.unary.f32(i32 21, float %388)
  %389 = fmul fast float %Exp67, %267
  %390 = fmul fast float %Exp67, %268
  %391 = fmul fast float %Exp67, %269
  %.i0362 = fsub fast float %267, %389
  %.i1363 = fsub fast float %268, %390
  %.i2364 = fsub fast float %269, %391
  %392 = call float @dx.op.dot3.f32(i32 55, float %.i0319, float %.i1320, float %.i2321, float %378, float %379, float %380)
  %Saturate = call float @dx.op.unary.f32(i32 7, float %392)
  %.i0365 = fmul fast float %374, %shadow.0
  %.i0368 = fmul fast float %.i0365, %Saturate
  %.i0371 = fmul fast float %.i0368, %.i0362
  %.i1366 = fmul fast float %375, %shadow.0
  %.i1369 = fmul fast float %.i1366, %Saturate
  %.i1372 = fmul fast float %.i1369, %.i1363
  %.i2367 = fmul fast float %376, %shadow.0
  %.i2370 = fmul fast float %.i2367, %Saturate
  %.i2373 = fmul fast float %.i2370, %.i2364
  %.i0374 = fadd fast float %.i0371, %.i0325
  %.i1375 = fadd fast float %.i1372, %.i1326
  %.i2376 = fadd fast float %.i2373, %.i2327
  %393 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %HitShaderConstants114, i32 8)
  %394 = extractvalue %dx.types.CBufRet.i32 %393, 0
  %395 = icmp eq i32 %394, 0
  br i1 %395, label %401, label %396

; <label>:396                                     ; preds = %372
  %.i0377 = fmul fast float %.i0374, 0x3FD3333340000000
  %.i1378 = fmul fast float %.i1375, 0x3FD3333340000000
  %.i2379 = fmul fast float %.i2376, 0x3FD3333340000000
  %397 = call %dx.types.Handle @"dx.op.createHandleFromResourceStructForLib.class.RWTexture2D<vector<float, 4> >"(i32 160, %"class.RWTexture2D<vector<float, 4> >" %11)
  %TextureLoad113 = call %dx.types.ResRet.f32 @dx.op.textureLoad.f32(i32 66, %dx.types.Handle %397, i32 undef, i32 %DispatchRaysIndex, i32 %DispatchRaysIndex106, i32 undef, i32 undef, i32 undef, i32 undef)
  %398 = extractvalue %dx.types.ResRet.f32 %TextureLoad113, 0
  %399 = extractvalue %dx.types.ResRet.f32 %TextureLoad113, 1
  %400 = extractvalue %dx.types.ResRet.f32 %TextureLoad113, 2
  %.i0380 = fadd fast float %398, %.i0377
  %.i1381 = fadd fast float %399, %.i1378
  %.i2382 = fadd fast float %400, %.i2379
  br label %401

; <label>:401                                     ; preds = %396, %372
  %outputColor.0.i0 = phi float [ %.i0380, %396 ], [ %.i0374, %372 ]
  %outputColor.0.i1 = phi float [ %.i1381, %396 ], [ %.i1375, %372 ]
  %outputColor.0.i2 = phi float [ %.i2382, %396 ], [ %.i2376, %372 ]
  %402 = call %dx.types.Handle @"dx.op.createHandleFromResourceStructForLib.class.RWTexture2D<vector<float, 4> >"(i32 160, %"class.RWTexture2D<vector<float, 4> >" %11)
  call void @dx.op.textureStore.f32(i32 67, %dx.types.Handle %402, i32 %DispatchRaysIndex, i32 %DispatchRaysIndex106, i32 undef, float %outputColor.0.i0, float %outputColor.0.i1, float %outputColor.0.i2, float 1.000000e+00, i8 15)
  br label %403

; <label>:403                                     ; preds = %401, %2
  ret void
}

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
  %12 = load float, float* %11, align 4, !tbaa !223
  %13 = extractelement <3 x i32> %1, i32 1
  %14 = getelementptr [3 x float], [3 x float]* %3, i32 0, i32 %13
  %15 = load float, float* %14, align 4, !tbaa !223
  %16 = extractelement <3 x i32> %1, i32 2
  %17 = getelementptr [3 x float], [3 x float]* %3, i32 0, i32 %16
  %18 = load float, float* %17, align 4, !tbaa !223
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
  %29 = load float, float* %28, align 4, !tbaa !223, !noalias !335
  %30 = fcmp fast olt float %29, 0.000000e+00
  %worldRayData.i.5.0.i0 = select i1 %30, i32 %27, i32 %25
  %worldRayData.i.5.0.i1 = select i1 %30, i32 %25, i32 %27
  %31 = getelementptr [3 x float], [3 x float]* %13, i32 0, i32 %worldRayData.i.5.0.i0
  %32 = load float, float* %31, align 4, !tbaa !223, !noalias !335
  %33 = fdiv float %32, %29
  %34 = getelementptr [3 x float], [3 x float]* %13, i32 0, i32 %worldRayData.i.5.0.i1
  %35 = load float, float* %34, align 4, !tbaa !223, !noalias !335
  %36 = fdiv float %35, %29
  %37 = fdiv float 1.000000e+00, %29
  %38 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %AccelerationStructureList, i32 0)
  %39 = extractvalue %dx.types.CBufRet.i32 %38, 0
  %40 = extractvalue %dx.types.CBufRet.i32 %38, 1
  %41 = getelementptr inbounds [2 x i32], [2 x i32]* %nodesToProcess.i, i32 0, i32 0
  store i32 0, i32* %41, align 4, !tbaa !221
  %42 = getelementptr inbounds [0 x %struct.RWByteAddressBuffer], [0 x %struct.RWByteAddressBuffer]* @"\01?DescriptorHeapBufferTable@@3PAURWByteAddressBuffer@@A", i32 0, i32 %40, !dx.nonuniform !228
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
  store i32 0, i32 addrspace(3)* %72, align 4, !tbaa !221, !noalias !338
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
  %94 = load i32, i32 addrspace(3)* %93, align 4, !tbaa !221, !noalias !341
  %95 = getelementptr inbounds [2 x i32], [2 x i32]* %nodesToProcess.i, i32 0, i32 %currentBVHIndex.i.1
  %96 = add i32 %89, -1
  store i32 %96, i32* %95, align 4, !tbaa !221
  %97 = getelementptr inbounds [0 x %struct.RWByteAddressBuffer], [0 x %struct.RWByteAddressBuffer]* @"\01?DescriptorHeapBufferTable@@3PAURWByteAddressBuffer@@A", i32 0, i32 %currentGpuVA.i.1.i1, !dx.nonuniform !228
  %98 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %97, align 4, !noalias !344
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
  store i32 0, i32 addrspace(3)* %93, align 4, !tbaa !221, !noalias !349
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
  %204 = load float, float* %203, align 4, !tbaa !223, !noalias !352
  %205 = fcmp fast olt float %204, 0.000000e+00
  %.5.0.i0 = select i1 %205, i32 %202, i32 %200
  %.5.0.i1 = select i1 %205, i32 %200, i32 %202
  %206 = getelementptr [3 x float], [3 x float]* %12, i32 0, i32 %.5.0.i0
  %207 = load float, float* %206, align 4, !tbaa !223, !noalias !352
  %208 = fdiv float %207, %204
  %209 = getelementptr [3 x float], [3 x float]* %12, i32 0, i32 %.5.0.i1
  %210 = load float, float* %209, align 4, !tbaa !223, !noalias !352
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
  %252 = load float, float* %251, align 4, !tbaa !223, !noalias !355
  %253 = getelementptr [3 x float], [3 x float]* %11, i32 0, i32 %currentRayData.i.5.1.i1
  %254 = load float, float* %253, align 4, !tbaa !223, !noalias !355
  %255 = getelementptr [3 x float], [3 x float]* %11, i32 0, i32 %currentRayData.i.5.1.i2
  %256 = load float, float* %255, align 4, !tbaa !223, !noalias !355
  %.i0357 = fsub fast float %.i3349, %currentRayData.i.0.1.i0
  %.i1358 = fsub fast float %.i0350, %currentRayData.i.0.1.i1
  %.i2359 = fsub fast float %.i1351, %currentRayData.i.0.1.i2
  store float %.i0357, float* %80, align 4
  store float %.i1358, float* %81, align 4
  store float %.i2359, float* %82, align 4
  %257 = getelementptr [3 x float], [3 x float]* %9, i32 0, i32 %currentRayData.i.5.1.i0
  %258 = load float, float* %257, align 4, !tbaa !223, !noalias !355
  %259 = getelementptr [3 x float], [3 x float]* %9, i32 0, i32 %currentRayData.i.5.1.i1
  %260 = load float, float* %259, align 4, !tbaa !223, !noalias !355
  %261 = getelementptr [3 x float], [3 x float]* %9, i32 0, i32 %currentRayData.i.5.1.i2
  %262 = load float, float* %261, align 4, !tbaa !223, !noalias !355
  %.i0360 = fsub fast float %.i2352, %currentRayData.i.0.1.i0
  %.i1361 = fsub fast float %.i3353, %currentRayData.i.0.1.i1
  %.i2362 = fsub fast float %237, %currentRayData.i.0.1.i2
  store float %.i0360, float* %83, align 4
  store float %.i1361, float* %84, align 4
  store float %.i2362, float* %85, align 4
  %263 = getelementptr [3 x float], [3 x float]* %10, i32 0, i32 %currentRayData.i.5.1.i0
  %264 = load float, float* %263, align 4, !tbaa !223, !noalias !355
  %265 = getelementptr [3 x float], [3 x float]* %10, i32 0, i32 %currentRayData.i.5.1.i1
  %266 = load float, float* %265, align 4, !tbaa !223, !noalias !355
  %267 = getelementptr [3 x float], [3 x float]* %10, i32 0, i32 %currentRayData.i.5.1.i2
  %268 = load float, float* %267, align 4, !tbaa !223, !noalias !355
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
  %342 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %97, align 4, !noalias !363
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
  store i32 %415, i32 addrspace(3)* %93, align 4, !tbaa !221, !noalias !368
  %419 = getelementptr [2048 x i32], [2048 x i32] addrspace(3)* @"\01?stack@@3PAIA", i32 0, i32 %418
  store i32 %416, i32 addrspace(3)* %419, align 4, !tbaa !221, !noalias !368
  %420 = add nsw i32 %stackPointer.i.2, 1
  %421 = add i32 %89, 1
  store i32 %421, i32* %95, align 4, !tbaa !221
  br label %426

; <label>:422                                     ; preds = %374
  %423 = or i1 %411, %412
  br i1 %423, label %424, label %426

; <label>:424                                     ; preds = %422
  %425 = select i1 %412, i32 %107, i32 %375
  store i32 %425, i32 addrspace(3)* %93, align 4, !tbaa !221, !noalias !371
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
  %10 = load float, float* %1, align 4, !tbaa !223
  %11 = fadd fast float %10, -1.000000e+00
  %12 = fmul fast float %11, %9
  %13 = fadd fast float %12, 1.000000e+00
  store float %13, float* %1, align 4, !tbaa !223
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
  %24 = load float, float* %23, align 4, !tbaa !223
  %25 = fcmp fast olt float %24, 0.000000e+00
  %.5.0.i0 = select i1 %25, i32 %22, i32 %20
  %.5.0.i1 = select i1 %25, i32 %20, i32 %22
  %.5.0.upto0 = insertelement <3 x i32> undef, i32 %.5.0.i0, i32 0
  %.5.0.upto1 = insertelement <3 x i32> %.5.0.upto0, i32 %.5.0.i1, i32 1
  %.5.0 = insertelement <3 x i32> %.5.0.upto1, i32 %.0, i32 2
  %26 = getelementptr [3 x float], [3 x float]* %4, i32 0, i32 %.5.0.i0
  %27 = load float, float* %26, align 4, !tbaa !223
  %28 = fdiv fast float %27, %24
  %29 = getelementptr [3 x float], [3 x float]* %4, i32 0, i32 %.5.0.i1
  %30 = load float, float* %29, align 4, !tbaa !223
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
define void @"\01?RayGen@@YAXXZ"() #5 {
  %1 = load %"class.Texture2D<vector<float, 3> >", %"class.Texture2D<vector<float, 3> >"* @"\01?normals@@3V?$Texture2D@V?$vector@M$02@@@@A", align 4
  %2 = load %"class.Texture2D<float>", %"class.Texture2D<float>"* @"\01?depth@@3V?$Texture2D@M@@A", align 4
  %3 = load %b1, %b1* @b1, align 4
  %b1 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.b1(i32 160, %b1 %3)
  %4 = alloca %struct.RayPayload, align 8
  %DispatchRaysIndex = call i32 @dx.op.dispatchRaysIndex.i32(i32 145, i8 0)
  %DispatchRaysIndex1 = call i32 @dx.op.dispatchRaysIndex.i32(i32 145, i8 1)
  %.i0 = uitofp i32 %DispatchRaysIndex to float
  %.i1 = uitofp i32 %DispatchRaysIndex1 to float
  %.i017 = fadd fast float %.i0, 5.000000e-01
  %.i118 = fadd fast float %.i1, 5.000000e-01
  %5 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %b1, i32 5)
  %6 = extractvalue %dx.types.CBufRet.f32 %5, 0
  %7 = extractvalue %dx.types.CBufRet.f32 %5, 1
  %.i019 = fdiv fast float %.i017, %6
  %.i120 = fdiv fast float %.i118, %7
  %.i021 = fmul fast float %.i019, 2.000000e+00
  %.i122 = fmul fast float %.i120, 2.000000e+00
  %.i023 = fadd fast float %.i021, -1.000000e+00
  %.i124 = fadd fast float %.i122, -1.000000e+00
  %8 = fsub fast float -0.000000e+00, %.i124
  %9 = fptosi float %.i017 to i32
  %10 = fptosi float %.i118 to i32
  %11 = call %dx.types.Handle @"dx.op.createHandleFromResourceStructForLib.class.Texture2D<float>"(i32 160, %"class.Texture2D<float>" %2)
  %TextureLoad = call %dx.types.ResRet.f32 @dx.op.textureLoad.f32(i32 66, %dx.types.Handle %11, i32 0, i32 %9, i32 %10, i32 undef, i32 undef, i32 undef, i32 undef)
  %12 = extractvalue %dx.types.ResRet.f32 %TextureLoad, 0
  %13 = call %dx.types.Handle @"dx.op.createHandleFromResourceStructForLib.class.Texture2D<vector<float, 3> >"(i32 160, %"class.Texture2D<vector<float, 3> >" %1)
  %TextureLoad2 = call %dx.types.ResRet.f32 @dx.op.textureLoad.f32(i32 66, %dx.types.Handle %13, i32 0, i32 %9, i32 %10, i32 undef, i32 undef, i32 undef, i32 undef)
  %14 = extractvalue %dx.types.ResRet.f32 %TextureLoad2, 0
  %15 = extractvalue %dx.types.ResRet.f32 %TextureLoad2, 1
  %16 = extractvalue %dx.types.ResRet.f32 %TextureLoad2, 2
  %17 = fmul fast float %14, %14
  %18 = fmul fast float %15, %15
  %19 = fadd fast float %17, %18
  %20 = fmul fast float %16, %16
  %21 = fadd fast float %19, %20
  %Sqrt = call float @dx.op.unary.f32(i32 24, float %21)
  %22 = fcmp fast olt float %Sqrt, 0x3FB99999A0000000
  br i1 %22, label %68, label %23

; <label>:23                                      ; preds = %0
  %24 = load %struct.RaytracingAccelerationStructure, %struct.RaytracingAccelerationStructure* @"\01?g_accel@@3URaytracingAccelerationStructure@@A", align 4
  %.i025 = fdiv fast float %14, %Sqrt
  %.i126 = fdiv fast float %15, %Sqrt
  %.i2 = fdiv fast float %16, %Sqrt
  %25 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %b1, i32 0)
  %26 = extractvalue %dx.types.CBufRet.f32 %25, 0
  %27 = extractvalue %dx.types.CBufRet.f32 %25, 1
  %28 = extractvalue %dx.types.CBufRet.f32 %25, 2
  %29 = extractvalue %dx.types.CBufRet.f32 %25, 3
  %30 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %b1, i32 1)
  %31 = extractvalue %dx.types.CBufRet.f32 %30, 0
  %32 = extractvalue %dx.types.CBufRet.f32 %30, 1
  %33 = extractvalue %dx.types.CBufRet.f32 %30, 2
  %34 = extractvalue %dx.types.CBufRet.f32 %30, 3
  %35 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %b1, i32 2)
  %36 = extractvalue %dx.types.CBufRet.f32 %35, 0
  %37 = extractvalue %dx.types.CBufRet.f32 %35, 1
  %38 = extractvalue %dx.types.CBufRet.f32 %35, 2
  %39 = extractvalue %dx.types.CBufRet.f32 %35, 3
  %40 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %b1, i32 3)
  %41 = extractvalue %dx.types.CBufRet.f32 %40, 0
  %42 = extractvalue %dx.types.CBufRet.f32 %40, 1
  %43 = extractvalue %dx.types.CBufRet.f32 %40, 2
  %44 = extractvalue %dx.types.CBufRet.f32 %40, 3
  %45 = fmul fast float %26, %.i023
  %FMad16 = call float @dx.op.tertiary.f32(i32 46, float %8, float %27, float %45)
  %FMad15 = call float @dx.op.tertiary.f32(i32 46, float %12, float %28, float %FMad16)
  %FMad14 = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %29, float %FMad15)
  %46 = fmul fast float %31, %.i023
  %FMad13 = call float @dx.op.tertiary.f32(i32 46, float %8, float %32, float %46)
  %FMad12 = call float @dx.op.tertiary.f32(i32 46, float %12, float %33, float %FMad13)
  %FMad11 = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %34, float %FMad12)
  %47 = fmul fast float %36, %.i023
  %FMad10 = call float @dx.op.tertiary.f32(i32 46, float %8, float %37, float %47)
  %FMad9 = call float @dx.op.tertiary.f32(i32 46, float %12, float %38, float %FMad10)
  %FMad8 = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %39, float %FMad9)
  %48 = fmul fast float %41, %.i023
  %FMad7 = call float @dx.op.tertiary.f32(i32 46, float %8, float %42, float %48)
  %FMad6 = call float @dx.op.tertiary.f32(i32 46, float %12, float %43, float %FMad7)
  %FMad = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %44, float %FMad6)
  %.i027 = fdiv fast float %FMad14, %FMad
  %.i128 = fdiv fast float %FMad11, %FMad
  %.i229 = fdiv fast float %FMad8, %FMad
  %49 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %b1, i32 4)
  %50 = extractvalue %dx.types.CBufRet.f32 %49, 0
  %51 = extractvalue %dx.types.CBufRet.f32 %49, 1
  %52 = extractvalue %dx.types.CBufRet.f32 %49, 2
  %.i030 = fsub fast float %50, %.i027
  %.i131 = fsub fast float %51, %.i128
  %.i232 = fsub fast float %52, %.i229
  %53 = fmul fast float %.i030, %.i030
  %54 = fmul fast float %.i131, %.i131
  %55 = fadd fast float %53, %54
  %56 = fmul fast float %.i232, %.i232
  %57 = fadd fast float %55, %56
  %Sqrt4 = call float @dx.op.unary.f32(i32 24, float %57)
  %.i033 = fdiv fast float %.i030, %Sqrt4
  %.i134 = fdiv fast float %.i131, %Sqrt4
  %.i235 = fdiv fast float %.i232, %Sqrt4
  %.i036 = fsub fast float -0.000000e+00, %.i033
  %.i137 = fsub fast float -0.000000e+00, %.i134
  %.i238 = fsub fast float -0.000000e+00, %.i235
  %58 = call float @dx.op.dot3.f32(i32 55, float %.i036, float %.i137, float %.i238, float %.i025, float %.i126, float %.i2)
  %59 = fmul fast float %58, 2.000000e+00
  %.i042 = fmul fast float %59, %.i025
  %.i143 = fmul fast float %59, %.i126
  %.i244 = fmul fast float %59, %.i2
  %.i045 = fsub fast float %.i036, %.i042
  %.i146 = fsub fast float %.i137, %.i143
  %.i247 = fsub fast float %.i238, %.i244
  %60 = fmul fast float %.i045, %.i045
  %61 = fmul fast float %.i146, %.i146
  %62 = fadd fast float %60, %61
  %63 = fmul fast float %.i247, %.i247
  %64 = fadd fast float %62, %63
  %Sqrt5 = call float @dx.op.unary.f32(i32 24, float %64)
  %.i048 = fdiv fast float %.i045, %Sqrt5
  %.i149 = fdiv fast float %.i146, %Sqrt5
  %.i250 = fdiv fast float %.i247, %Sqrt5
  %.i051 = fmul fast float %.i033, 0x3FB99999A0000000
  %.i152 = fmul fast float %.i134, 0x3FB99999A0000000
  %.i253 = fmul fast float %.i235, 0x3FB99999A0000000
  %.i054 = fsub fast float %.i027, %.i051
  %.i155 = fsub fast float %.i128, %.i152
  %.i256 = fsub fast float %.i229, %.i253
  %65 = getelementptr inbounds %struct.RayPayload, %struct.RayPayload* %4, i32 0, i32 0
  store i32 0, i32* %65, align 8
  %66 = getelementptr inbounds %struct.RayPayload, %struct.RayPayload* %4, i32 0, i32 1
  store float 0x47EFFFFFE0000000, float* %66, align 4
  %67 = call %dx.types.Handle @dx.op.createHandleFromResourceStructForLib.struct.RaytracingAccelerationStructure(i32 160, %struct.RaytracingAccelerationStructure %24)
  call void @dx.op.traceRay.struct.RayPayload(i32 157, %dx.types.Handle %67, i32 16, i32 -1, i32 0, i32 1, i32 0, float %.i054, float %.i155, float %.i256, float 0.000000e+00, float %.i048, float %.i149, float %.i250, float 0x47EFFFFFE0000000, %struct.RayPayload* nonnull %4)
  br label %68

; <label>:68                                      ; preds = %23, %0
  ret void
}

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
  %9 = load <3 x float>, <3 x float>* %8, align 4, !noalias !374
  %10 = extractelement <3 x float> %9, i32 0
  %11 = bitcast float %10 to i32
  %12 = extractelement <3 x float> %9, i32 1
  %13 = bitcast float %12 to i32
  %14 = extractelement <3 x float> %9, i32 2
  %15 = bitcast float %14 to i32
  %16 = extractelement <2 x i32> %4, i32 0
  %17 = getelementptr inbounds %struct.BoundingBox, %struct.BoundingBox* %3, i32 0, i32 1
  %18 = load <3 x float>, <3 x float>* %17, align 4, !noalias !374
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
  %60 = load float, float* %59, align 4, !tbaa !223
  %61 = getelementptr [3 x float], [3 x float]* %12, i32 0, i32 %indices.0.i0
  %62 = load float, float* %61, align 4, !tbaa !223
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
  %70 = load float, float* %69, align 4, !tbaa !223
  %71 = getelementptr [3 x float], [3 x float]* %11, i32 0, i32 %indices.0.i0
  %72 = load float, float* %71, align 4, !tbaa !223
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
  %80 = load float, float* %79, align 4, !tbaa !223
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
  call void @"\01?Fallback_Scheduler@@YAXHII@Z"(i32 %7, i32 %10, i32 %9) #3
  ret void
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
!120 = !{i32 1, %class.matrix.float.3.4 ([3 x <4 x float>]*)* @"\01?CreateMatrix@@YA?AV?$matrix@M$02$03@@Y02V?$vector@M$03@@@Z", !121, i32 (%struct.RaytracingInstanceDesc*)* @"\01?GetInstanceContributionToHitGroupIndex@@YAIURaytracingInstanceDesc@@@Z", !128, void (%struct.RWByteAddressBufferPointer*, <3 x float>*, <3 x float>*, <3 x float>*, i32)* @"\01?BVHReadTriangle@@YAXURWByteAddressBufferPointer@@AIAV?$vector@M$02@@11I@Z", !132, void (%struct.RWByteAddressBuffer*, <2 x i32>)* @"\01?PointerGetBuffer@@YA?AURWByteAddressBuffer@@V?$vector@I$01@@@Z", !136, float (%class.matrix.float.3.4)* @"\01?Determinant@@YAMV?$matrix@M$02$03@@@Z", !137, void (<2 x float>, <2 x float>, <2 x float>, <3 x float>, <3 x float>, <3 x float>, <3 x float>*, <3 x float>*)* @"\01?CalculateTrianglePartialDerivatives@@YAXV?$vector@M$01@@00V?$vector@M$02@@11AIAV2@2@Z", !139, i32 (<2 x i32>)* @"\01?GetLeafIndexFromFlag@@YAHV?$vector@I$01@@@Z", !140, i32 (<2 x i32>)* @"\01?GetLeftNodeIndex@@YAIV?$vector@I$01@@@Z", !143, i1 (i1, i32)* @"\01?Cull@@YA_N_NI@Z", !144, i32 (%struct.RWByteAddressBufferPointer*)* @"\01?GetOffsetToVertices@@YAHURWByteAddressBufferPointer@@@Z", !148, i32 (%struct.RaytracingInstanceDesc*)* @"\01?GetInstanceMask@@YAIURaytracingInstanceDesc@@@Z", !128, void (%struct.BoundingBox*, %struct.RWByteAddressBuffer*, i32, i32)* @"\01?GetBoxFromBuffer@@YA?AUBoundingBox@@URWByteAddressBuffer@@II@Z", !149, void (%struct.RayPayload*)* @"\01?Miss@@YAXURayPayload@@@Z", !150, void (%struct.BoundingBox*, %struct.RWByteAddressBufferPointer*, i32, <2 x i32>*)* @"\01?BVHReadBoundingBox@@YA?AUBoundingBox@@URWByteAddressBufferPointer@@HAIAV?$vector@I$01@@@Z", !153, void (i32, i32, i32, i32, i32, float, float, float, float, float, float, float, float, i32)* @"\01?Fallback_TraceRay@@YAXIIIIIMMMMMMMMI@Z", !155, void (%struct.BVHMetadata*, %struct.RWByteAddressBuffer*, i32)* @"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z", !156, <2 x i32> (<2 x i32>, i32)* @"\01?PointerAdd@@YA?AV?$vector@I$01@@V1@I@Z", !157, void (%struct.RaytracingInstanceDesc*, %struct.ByteAddressBuffer*, i32)* @"\01?LoadRaytracingInstanceDesc@@YA?AURaytracingInstanceDesc@@UByteAddressBuffer@@I@Z", !156, void ()* @"\01?Fallback_IgnoreHit@@YAXXZ", !158, void (%struct.RaytracingInstanceDesc*, <4 x i32>, <4 x i32>, <4 x i32>, <4 x i32>)* @"\01?RawDataToRaytracingInstanceDesc@@YA?AURaytracingInstanceDesc@@V?$vector@I$03@@000@Z", !159, void (%struct.AABB*, %struct.BoundingBox*)* @"\01?BoundingBoxToAABB@@YA?AUAABB@@UBoundingBox@@@Z", !160, <2 x i32> (i32, i32)* @"\01?CreateFlag@@YA?AV?$vector@I$01@@II@Z", !157, void (%struct.TriangleMetaData*, %struct.RWByteAddressBufferPointer*, i32)* @"\01?BVHReadTriangleMetadata@@YA?AUTriangleMetaData@@URWByteAddressBufferPointer@@H@Z", !161, i1 (%class.matrix.float.2.2, %class.matrix.float.2.2*)* @"\01?Inverse2x2@@YA_NV?$matrix@M$01$01@@AIAV1@@Z", !162, i32 (<3 x float>)* @"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z", !167, i1 (%struct.RWByteAddressBufferPointer*, <2 x i32>, i32, <3 x float>, <3 x float>, <3 x i32>, <3 x float>, <2 x float>*, float*, i32*)* @"\01?TestLeafNodeIntersections@@YA_NURWByteAddressBufferPointer@@V?$vector@I$01@@IV?$vector@M$02@@2V?$vector@H$02@@2AIAV?$vector@M$01@@AIAMAIAI@Z", !168, i1 (i32, i32, i32)* @"\01?Traverse@@YA_NIII@Z", !171, i32 (i32*, i32*, i32)* @"\01?StackPop@@YAIAIAHAIAII@Z", !172, i32 (<2 x i32>)* @"\01?GetRightNodeIndex@@YAIV?$vector@I$01@@@Z", !143, void (%struct.RWByteAddressBufferPointer*, %struct.RWByteAddressBuffer*, i32)* @"\01?CreateRWByteAddressBufferPointer@@YA?AURWByteAddressBufferPointer@@URWByteAddressBuffer@@I@Z", !156, void (i32*, i32, i32, i32)* @"\01?StackPush@@YAXAIAHIII@Z", !174, void (float)* @"\01?LogFloat@@YAXM@Z", !175, i32 (i32, i32)* @"\01?GetBoxAddress@@YAIII@Z", !157, void (i32*, i1, i32, i32, i32, i32)* @"\01?StackPush2@@YAXAIAH_NIIII@Z", !176, void ()* @"\01?LogTraceRayEnd@@YAXXZ", !158, void (<3 x float>)* @"\01?LogFloat3@@YAXV?$vector@M$02@@@Z", !175, i32 (%struct.RaytracingInstanceDesc*)* @"\01?GetInstanceID@@YAIURaytracingInstanceDesc@@@Z", !128, void (%struct.BoundingBox*, <2 x i32>)* @"\01?dump@@YAXUBoundingBox@@V?$vector@I$01@@@Z", !177, i32 (i32)* @"\01?InvokeAnyHit@@YAHH@Z", !178, float (i32, i32)* @"\01?ComputeCullFaceDir@@YAMII@Z", !179, void (%struct.BoundingBox*, <2 x i32>, <4 x i32>*, <4 x i32>*)* @"\01?CompressBox@@YAXUBoundingBox@@V?$vector@I$01@@AIAV?$vector@I$03@@2@Z", !180, void (%struct.RWByteAddressBufferPointer*, <2 x i32>)* @"\01?CreateRWByteAddressBufferPointerFromGpuVA@@YA?AURWByteAddressBufferPointer@@V?$vector@I$01@@@Z", !136, void (%struct.BoundingBox*, %struct.AABB*)* @"\01?AABBtoBoundingBox@@YA?AUBoundingBox@@UAABB@@@Z", !160, void (%struct.AABB*, <4 x i32>, <4 x i32>)* @"\01?RawDataToAABB@@YA?AUAABB@@V?$vector@H$03@@0@Z", !181, <3 x float> (%struct.BoundingBox*)* @"\01?GetMinCorner@@YA?AV?$vector@M$02@@UBoundingBox@@@Z", !182, i1 (i1, i32, i32)* @"\01?IsOpaque@@YA_N_NII@Z", !183, void ()* @main, !184, void (<3 x i32>)* @"\01?LogInt3@@YAXV?$vector@H$02@@@Z", !185, <4 x i32> (<2 x i32>)* @"\01?Load4@@YA?AV?$vector@I$03@@V?$vector@I$01@@@Z", !143, void (%struct.RaytracingInstanceDesc*, %struct.RWByteAddressBuffer*, i32)* @"\01?LoadRaytracingInstanceDesc@@YA?AURaytracingInstanceDesc@@URWByteAddressBuffer@@I@Z", !156, <3 x float> (%struct.BoundingBox*)* @"\01?GetMaxCorner@@YA?AV?$vector@M$02@@UBoundingBox@@@Z", !182, void (%struct.RayPayload*, %struct.BuiltInTriangleIntersectionAttributes*)* @"\01?Hit@@YAXURayPayload@@UBuiltInTriangleIntersectionAttributes@@@Z", !186, void (i32, i32*, float, i32*, float, float*)* @"\01?RecordClosestBox@@YAXIAIA_NM0MAIAM@Z", !189, void (%struct.BoundingBox*, <3 x float>, <3 x float>, <3 x float>, i32, <2 x i32>*)* @"\01?GetBoxDataFromTriangle@@YA?AUBoundingBox@@V?$vector@M$02@@00HAIAV?$vector@I$01@@@Z", !191, void (%struct.BoundingBox*, %struct.BoundingBox*, i32, %struct.BoundingBox*, i32, <2 x i32>*)* @"\01?GetBoxFromChildBoxes@@YA?AUBoundingBox@@U1@H0HAIAV?$vector@I$01@@@Z", !192, i32 (%struct.RWByteAddressBufferPointer*)* @"\01?GetOffsetToBoxes@@YAHURWByteAddressBufferPointer@@@Z", !148, i32 (%struct.RWByteAddressBufferPointer*)* @"\01?GetOffsetToTriangleMetadata@@YAHURWByteAddressBufferPointer@@@Z", !148, void (<4 x i32>)* @"\01?Log@@YAXV?$vector@I$03@@@Z", !193, <2 x float> (i32)* @"\01?GetUVAttribute@@YA?AV?$vector@M$01@@I@Z", !194, i32 (%struct.RaytracingInstanceDesc*)* @"\01?GetInstanceFlags@@YAIURaytracingInstanceDesc@@@Z", !128, void (i32)* @"\01?LogInt@@YAXH@Z", !185, <3 x float> (<3 x float>, <3 x float>, float, float, <3 x float>, <3 x float>, <3 x float>, <3 x float>)* @"\01?ApplyLightCommon@@YA?AV?$vector@M$02@@V1@0MM0000@Z", !195, <3 x float> (<3 x float>, <3 x i32>)* @"\01?Swizzle@@YA?AV?$vector@M$02@@V1@V?$vector@H$02@@@Z", !196, i32 (i32, i32, i32, i32)* @"\01?Traverse@@YAHIIII@Z", !197, void (i32)* @"\01?LogNoData@@YAXI@Z", !193, i1 (<2 x i32>)* @"\01?IsLeaf@@YA_NV?$vector@I$01@@@Z", !198, void (%struct.RWByteAddressBuffer*, i32, %struct.BVHMetadata*)* @"\01?StoreBVHMetadataToRawData@@YAXURWByteAddressBuffer@@IUBVHMetadata@@@Z", !199, <3 x float> (<3 x float>, <3 x float>, <3 x float>, <3 x float>)* @"\01?RayPlaneIntersection@@YA?AV?$vector@M$02@@V1@000@Z", !200, void (<3 x float>*, <3 x float>*, <3 x float>, <3 x float>)* @"\01?FSchlick@@YAXAIAV?$vector@M$02@@0V1@1@Z", !201, void (%struct.BoundingBox*, <4 x i32>, <4 x i32>, <2 x i32>*)* @"\01?RawDataToBoundingBox@@YA?AUBoundingBox@@V?$vector@H$03@@0AIAV?$vector@I$01@@@Z", !202, void (%struct.AABB*, %struct.AABB*, %class.matrix.float.3.4)* @"\01?TransformAABB@@YA?AUAABB@@U1@V?$matrix@M$02$03@@@Z", !203, i32 (<2 x i32>)* @"\01?PointerGetBufferStartOffset@@YAIV?$vector@I$01@@@Z", !143, void (<3 x float>*, float*)* @"\01?AntiAliasSpecular@@YAXAIAV?$vector@M$02@@AIAM@Z", !204, %class.matrix.float.3.4 (%class.matrix.float.3.4)* @"\01?InverseAffineTransform@@YA?AV?$matrix@M$02$03@@V1@@Z", !205, float (<3 x float>)* @"\01?GetShadow@@YAMV?$vector@M$02@@@Z", !206, void (%struct.RayData*, <3 x float>, <3 x float>)* @"\01?GetRayData@@YA?AURayData@@V?$vector@M$02@@0@Z", !207, <3 x i32> (i32)* @"\01?Load3x16BitIndices@@YA?AV?$vector@I$02@@I@Z", !143, void ()* @"\01?Fallback_AcceptHitAndEndSearch@@YAXXZ", !158, void ()* @"\01?RayGen@@YAXXZ", !158, void (i32*, i32*)* @"\01?swap@@YAXAIAH0@Z", !208, i32 (i32, i32)* @"\01?GetTriangleMetadataAddress@@YAIII@Z", !157, void (%struct.RWByteAddressBuffer*, i32, i32, <2 x i32>)* @"\01?WriteOnlyFlagToBuffer@@YAXURWByteAddressBuffer@@IIV?$vector@I$01@@@Z", !209, void (%struct.RWByteAddressBuffer*, i32, i32, %struct.BoundingBox*, <2 x i32>)* @"\01?WriteBoxToBuffer@@YAXURWByteAddressBuffer@@IIUBoundingBox@@V?$vector@I$01@@@Z", !210, void ()* @"\01?LogTraceRayStart@@YAXXZ", !158, void (<2 x i32>)* @"\01?LogInt2@@YAXV?$vector@H$01@@@Z", !185, void (<3 x float>, <3 x float>, <3 x float>, <3 x float>, <3 x float>, <3 x float>, <2 x float>*, <2 x float>*)* @"\01?CalculateUVDerivatives@@YAXV?$vector@M$02@@00000AIAV?$vector@M$01@@1@Z", !139, void (<3 x i32>, i32)* @"\01?main@@YAXV?$vector@I$02@@I@Z", !211}
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
!213 = !{void (%struct.RayPayload*, %struct.BuiltInTriangleIntersectionAttributes*)* @"\01?Hit@@YAXURayPayload@@UBuiltInTriangleIntersectionAttributes@@@Z", i32 10, i32 8, i32 8}
!214 = !{void (%struct.RayPayload*)* @"\01?Miss@@YAXURayPayload@@@Z", i32 11, i32 8}
!215 = !{void ()* @"\01?RayGen@@YAXXZ", i32 7}
!216 = !{void ()* @main, i32 5, i32 8, i32 8, i32 1}
!217 = !{void ()* @main, null}
!218 = !{!219, !219, i64 0}
!219 = !{!"omnipotent char", !220, i64 0}
!220 = !{!"Simple C/C++ TBAA"}
!221 = !{!222, !222, i64 0}
!222 = !{!"int", !219, i64 0}
!223 = !{!224, !224, i64 0}
!224 = !{!"float", !219, i64 0}
!225 = !{!226}
!226 = distinct !{!226, !227, !"\01?GetRayData@@YA?AURayData@@V?$vector@M$02@@0@Z: %agg.result"}
!227 = distinct !{!227, !"\01?GetRayData@@YA?AURayData@@V?$vector@M$02@@0@Z"}
!228 = !{i32 1}
!229 = !{!230}
!230 = distinct !{!230, !231, !"\01?StackPush@@YAXAIAHIII@Z: %stackTop"}
!231 = distinct !{!231, !"\01?StackPush@@YAXAIAHIII@Z"}
!232 = !{!233}
!233 = distinct !{!233, !234, !"\01?StackPop@@YAIAIAHAIAII@Z: %stackTop"}
!234 = distinct !{!234, !"\01?StackPop@@YAIAIAHAIAII@Z"}
!235 = !{!236, !238}
!236 = distinct !{!236, !237, !"\01?PointerGetBuffer@@YA?AURWByteAddressBuffer@@V?$vector@I$01@@@Z: %agg.result"}
!237 = distinct !{!237, !"\01?PointerGetBuffer@@YA?AURWByteAddressBuffer@@V?$vector@I$01@@@Z"}
!238 = distinct !{!238, !239, !"\01?CreateRWByteAddressBufferPointerFromGpuVA@@YA?AURWByteAddressBufferPointer@@V?$vector@I$01@@@Z: %agg.result"}
!239 = distinct !{!239, !"\01?CreateRWByteAddressBufferPointerFromGpuVA@@YA?AURWByteAddressBufferPointer@@V?$vector@I$01@@@Z"}
!240 = !{!241}
!241 = distinct !{!241, !242, !"\01?StackPush@@YAXAIAHIII@Z: %stackTop"}
!242 = distinct !{!242, !"\01?StackPush@@YAXAIAHIII@Z"}
!243 = !{!244}
!244 = distinct !{!244, !245, !"\01?GetRayData@@YA?AURayData@@V?$vector@M$02@@0@Z: %agg.result"}
!245 = distinct !{!245, !"\01?GetRayData@@YA?AURayData@@V?$vector@M$02@@0@Z"}
!246 = !{!247, !249, !250, !252, !253}
!247 = distinct !{!247, !248, !"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z: %hitT"}
!248 = distinct !{!248, !"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z"}
!249 = distinct !{!249, !248, !"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z: %bary"}
!250 = distinct !{!250, !251, !"\01?TestLeafNodeIntersections@@YA_NURWByteAddressBufferPointer@@V?$vector@I$01@@IV?$vector@M$02@@2V?$vector@H$02@@2AIAV?$vector@M$01@@AIAMAIAI@Z: %resultBary"}
!251 = distinct !{!251, !"\01?TestLeafNodeIntersections@@YA_NURWByteAddressBufferPointer@@V?$vector@I$01@@IV?$vector@M$02@@2V?$vector@H$02@@2AIAV?$vector@M$01@@AIAMAIAI@Z"}
!252 = distinct !{!252, !251, !"\01?TestLeafNodeIntersections@@YA_NURWByteAddressBufferPointer@@V?$vector@I$01@@IV?$vector@M$02@@2V?$vector@H$02@@2AIAV?$vector@M$01@@AIAMAIAI@Z: %resultT"}
!253 = distinct !{!253, !251, !"\01?TestLeafNodeIntersections@@YA_NURWByteAddressBufferPointer@@V?$vector@I$01@@IV?$vector@M$02@@2V?$vector@H$02@@2AIAV?$vector@M$01@@AIAMAIAI@Z: %resultTriId"}
!254 = !{!255, !257}
!255 = distinct !{!255, !256, !"\01?PointerGetBuffer@@YA?AURWByteAddressBuffer@@V?$vector@I$01@@@Z: %agg.result"}
!256 = distinct !{!256, !"\01?PointerGetBuffer@@YA?AURWByteAddressBuffer@@V?$vector@I$01@@@Z"}
!257 = distinct !{!257, !258, !"\01?CreateRWByteAddressBufferPointerFromGpuVA@@YA?AURWByteAddressBufferPointer@@V?$vector@I$01@@@Z: %agg.result"}
!258 = distinct !{!258, !"\01?CreateRWByteAddressBufferPointerFromGpuVA@@YA?AURWByteAddressBufferPointer@@V?$vector@I$01@@@Z"}
!259 = !{!260}
!260 = distinct !{!260, !261, !"\01?StackPush2@@YAXAIAH_NIIII@Z: %stackTop"}
!261 = distinct !{!261, !"\01?StackPush2@@YAXAIAH_NIIII@Z"}
!262 = !{!263}
!263 = distinct !{!263, !264, !"\01?StackPush@@YAXAIAHIII@Z: %stackTop"}
!264 = distinct !{!264, !"\01?StackPush@@YAXAIAHIII@Z"}
!265 = !{!266}
!266 = distinct !{!266, !267, !"\01?RawDataToRaytracingInstanceDesc@@YA?AURaytracingInstanceDesc@@V?$vector@I$03@@000@Z: %agg.result"}
!267 = distinct !{!267, !"\01?RawDataToRaytracingInstanceDesc@@YA?AURaytracingInstanceDesc@@V?$vector@I$03@@000@Z"}
!268 = !{!269, !271, !272}
!269 = distinct !{!269, !270, !"\01?BVHReadTriangle@@YAXURWByteAddressBufferPointer@@AIAV?$vector@M$02@@11I@Z: %v0"}
!270 = distinct !{!270, !"\01?BVHReadTriangle@@YAXURWByteAddressBufferPointer@@AIAV?$vector@M$02@@11I@Z"}
!271 = distinct !{!271, !270, !"\01?BVHReadTriangle@@YAXURWByteAddressBufferPointer@@AIAV?$vector@M$02@@11I@Z: %v1"}
!272 = distinct !{!272, !270, !"\01?BVHReadTriangle@@YAXURWByteAddressBufferPointer@@AIAV?$vector@M$02@@11I@Z: %v2"}
!273 = !{!274, !276}
!274 = distinct !{!274, !275, !"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z: %hitT"}
!275 = distinct !{!275, !"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z"}
!276 = distinct !{!276, !275, !"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z: %bary"}
!277 = !{!278}
!278 = distinct !{!278, !279, !"\01?GetRayData@@YA?AURayData@@V?$vector@M$02@@0@Z: %agg.result"}
!279 = distinct !{!279, !"\01?GetRayData@@YA?AURayData@@V?$vector@M$02@@0@Z"}
!280 = !{!281}
!281 = distinct !{!281, !282, !"\01?StackPush@@YAXAIAHIII@Z: %stackTop"}
!282 = distinct !{!282, !"\01?StackPush@@YAXAIAHIII@Z"}
!283 = !{!284}
!284 = distinct !{!284, !285, !"\01?StackPop@@YAIAIAHAIAII@Z: %stackTop"}
!285 = distinct !{!285, !"\01?StackPop@@YAIAIAHAIAII@Z"}
!286 = !{!287, !289}
!287 = distinct !{!287, !288, !"\01?PointerGetBuffer@@YA?AURWByteAddressBuffer@@V?$vector@I$01@@@Z: %agg.result"}
!288 = distinct !{!288, !"\01?PointerGetBuffer@@YA?AURWByteAddressBuffer@@V?$vector@I$01@@@Z"}
!289 = distinct !{!289, !290, !"\01?CreateRWByteAddressBufferPointerFromGpuVA@@YA?AURWByteAddressBufferPointer@@V?$vector@I$01@@@Z: %agg.result"}
!290 = distinct !{!290, !"\01?CreateRWByteAddressBufferPointerFromGpuVA@@YA?AURWByteAddressBufferPointer@@V?$vector@I$01@@@Z"}
!291 = !{!292}
!292 = distinct !{!292, !293, !"\01?StackPush@@YAXAIAHIII@Z: %stackTop"}
!293 = distinct !{!293, !"\01?StackPush@@YAXAIAHIII@Z"}
!294 = !{!295}
!295 = distinct !{!295, !296, !"\01?GetRayData@@YA?AURayData@@V?$vector@M$02@@0@Z: %agg.result"}
!296 = distinct !{!296, !"\01?GetRayData@@YA?AURayData@@V?$vector@M$02@@0@Z"}
!297 = !{!298, !300, !301, !303, !304}
!298 = distinct !{!298, !299, !"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z: %hitT"}
!299 = distinct !{!299, !"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z"}
!300 = distinct !{!300, !299, !"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z: %bary"}
!301 = distinct !{!301, !302, !"\01?TestLeafNodeIntersections@@YA_NURWByteAddressBufferPointer@@V?$vector@I$01@@IV?$vector@M$02@@2V?$vector@H$02@@2AIAV?$vector@M$01@@AIAMAIAI@Z: %resultBary"}
!302 = distinct !{!302, !"\01?TestLeafNodeIntersections@@YA_NURWByteAddressBufferPointer@@V?$vector@I$01@@IV?$vector@M$02@@2V?$vector@H$02@@2AIAV?$vector@M$01@@AIAMAIAI@Z"}
!303 = distinct !{!303, !302, !"\01?TestLeafNodeIntersections@@YA_NURWByteAddressBufferPointer@@V?$vector@I$01@@IV?$vector@M$02@@2V?$vector@H$02@@2AIAV?$vector@M$01@@AIAMAIAI@Z: %resultT"}
!304 = distinct !{!304, !302, !"\01?TestLeafNodeIntersections@@YA_NURWByteAddressBufferPointer@@V?$vector@I$01@@IV?$vector@M$02@@2V?$vector@H$02@@2AIAV?$vector@M$01@@AIAMAIAI@Z: %resultTriId"}
!305 = !{!306, !308}
!306 = distinct !{!306, !307, !"\01?PointerGetBuffer@@YA?AURWByteAddressBuffer@@V?$vector@I$01@@@Z: %agg.result"}
!307 = distinct !{!307, !"\01?PointerGetBuffer@@YA?AURWByteAddressBuffer@@V?$vector@I$01@@@Z"}
!308 = distinct !{!308, !309, !"\01?CreateRWByteAddressBufferPointerFromGpuVA@@YA?AURWByteAddressBufferPointer@@V?$vector@I$01@@@Z: %agg.result"}
!309 = distinct !{!309, !"\01?CreateRWByteAddressBufferPointerFromGpuVA@@YA?AURWByteAddressBufferPointer@@V?$vector@I$01@@@Z"}
!310 = !{!311}
!311 = distinct !{!311, !312, !"\01?StackPush2@@YAXAIAH_NIIII@Z: %stackTop"}
!312 = distinct !{!312, !"\01?StackPush2@@YAXAIAH_NIIII@Z"}
!313 = !{!314}
!314 = distinct !{!314, !315, !"\01?StackPush@@YAXAIAHIII@Z: %stackTop"}
!315 = distinct !{!315, !"\01?StackPush@@YAXAIAHIII@Z"}
!316 = !{!317}
!317 = distinct !{!317, !318, !"\01?CreateRWByteAddressBufferPointer@@YA?AURWByteAddressBufferPointer@@URWByteAddressBuffer@@I@Z: %agg.result"}
!318 = distinct !{!318, !"\01?CreateRWByteAddressBufferPointer@@YA?AURWByteAddressBufferPointer@@URWByteAddressBuffer@@I@Z"}
!319 = !{!320}
!320 = distinct !{!320, !321, !"\01?BoundingBoxToAABB@@YA?AUAABB@@UBoundingBox@@@Z: %agg.result"}
!321 = distinct !{!321, !"\01?BoundingBoxToAABB@@YA?AUAABB@@UBoundingBox@@@Z"}
!322 = !{!323}
!323 = distinct !{!323, !324, !"\01?PointerGetBuffer@@YA?AURWByteAddressBuffer@@V?$vector@I$01@@@Z: %agg.result"}
!324 = distinct !{!324, !"\01?PointerGetBuffer@@YA?AURWByteAddressBuffer@@V?$vector@I$01@@@Z"}
!325 = !{!326}
!326 = distinct !{!326, !327, !"\01?RawDataToRaytracingInstanceDesc@@YA?AURaytracingInstanceDesc@@V?$vector@I$03@@000@Z: %agg.result"}
!327 = distinct !{!327, !"\01?RawDataToRaytracingInstanceDesc@@YA?AURaytracingInstanceDesc@@V?$vector@I$03@@000@Z"}
!328 = !{!329, !329, i64 0}
!329 = !{!"bool", !219, i64 0}
!330 = !{i32 0, i32 2}
!331 = !{!332, !334}
!332 = distinct !{!332, !333, !"\01?CalculateUVDerivatives@@YAXV?$vector@M$02@@00000AIAV?$vector@M$01@@1@Z: %ddX"}
!333 = distinct !{!333, !"\01?CalculateUVDerivatives@@YAXV?$vector@M$02@@00000AIAV?$vector@M$01@@1@Z"}
!334 = distinct !{!334, !333, !"\01?CalculateUVDerivatives@@YAXV?$vector@M$02@@00000AIAV?$vector@M$01@@1@Z: %ddY"}
!335 = !{!336}
!336 = distinct !{!336, !337, !"\01?GetRayData@@YA?AURayData@@V?$vector@M$02@@0@Z: %agg.result"}
!337 = distinct !{!337, !"\01?GetRayData@@YA?AURayData@@V?$vector@M$02@@0@Z"}
!338 = !{!339}
!339 = distinct !{!339, !340, !"\01?StackPush@@YAXAIAHIII@Z: %stackTop"}
!340 = distinct !{!340, !"\01?StackPush@@YAXAIAHIII@Z"}
!341 = !{!342}
!342 = distinct !{!342, !343, !"\01?StackPop@@YAIAIAHAIAII@Z: %stackTop"}
!343 = distinct !{!343, !"\01?StackPop@@YAIAIAHAIAII@Z"}
!344 = !{!345, !347}
!345 = distinct !{!345, !346, !"\01?PointerGetBuffer@@YA?AURWByteAddressBuffer@@V?$vector@I$01@@@Z: %agg.result"}
!346 = distinct !{!346, !"\01?PointerGetBuffer@@YA?AURWByteAddressBuffer@@V?$vector@I$01@@@Z"}
!347 = distinct !{!347, !348, !"\01?CreateRWByteAddressBufferPointerFromGpuVA@@YA?AURWByteAddressBufferPointer@@V?$vector@I$01@@@Z: %agg.result"}
!348 = distinct !{!348, !"\01?CreateRWByteAddressBufferPointerFromGpuVA@@YA?AURWByteAddressBufferPointer@@V?$vector@I$01@@@Z"}
!349 = !{!350}
!350 = distinct !{!350, !351, !"\01?StackPush@@YAXAIAHIII@Z: %stackTop"}
!351 = distinct !{!351, !"\01?StackPush@@YAXAIAHIII@Z"}
!352 = !{!353}
!353 = distinct !{!353, !354, !"\01?GetRayData@@YA?AURayData@@V?$vector@M$02@@0@Z: %agg.result"}
!354 = distinct !{!354, !"\01?GetRayData@@YA?AURayData@@V?$vector@M$02@@0@Z"}
!355 = !{!356, !358, !359, !361, !362}
!356 = distinct !{!356, !357, !"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z: %hitT"}
!357 = distinct !{!357, !"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z"}
!358 = distinct !{!358, !357, !"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z: %bary"}
!359 = distinct !{!359, !360, !"\01?TestLeafNodeIntersections@@YA_NURWByteAddressBufferPointer@@V?$vector@I$01@@IV?$vector@M$02@@2V?$vector@H$02@@2AIAV?$vector@M$01@@AIAMAIAI@Z: %resultBary"}
!360 = distinct !{!360, !"\01?TestLeafNodeIntersections@@YA_NURWByteAddressBufferPointer@@V?$vector@I$01@@IV?$vector@M$02@@2V?$vector@H$02@@2AIAV?$vector@M$01@@AIAMAIAI@Z"}
!361 = distinct !{!361, !360, !"\01?TestLeafNodeIntersections@@YA_NURWByteAddressBufferPointer@@V?$vector@I$01@@IV?$vector@M$02@@2V?$vector@H$02@@2AIAV?$vector@M$01@@AIAMAIAI@Z: %resultT"}
!362 = distinct !{!362, !360, !"\01?TestLeafNodeIntersections@@YA_NURWByteAddressBufferPointer@@V?$vector@I$01@@IV?$vector@M$02@@2V?$vector@H$02@@2AIAV?$vector@M$01@@AIAMAIAI@Z: %resultTriId"}
!363 = !{!364, !366}
!364 = distinct !{!364, !365, !"\01?PointerGetBuffer@@YA?AURWByteAddressBuffer@@V?$vector@I$01@@@Z: %agg.result"}
!365 = distinct !{!365, !"\01?PointerGetBuffer@@YA?AURWByteAddressBuffer@@V?$vector@I$01@@@Z"}
!366 = distinct !{!366, !367, !"\01?CreateRWByteAddressBufferPointerFromGpuVA@@YA?AURWByteAddressBufferPointer@@V?$vector@I$01@@@Z: %agg.result"}
!367 = distinct !{!367, !"\01?CreateRWByteAddressBufferPointerFromGpuVA@@YA?AURWByteAddressBufferPointer@@V?$vector@I$01@@@Z"}
!368 = !{!369}
!369 = distinct !{!369, !370, !"\01?StackPush2@@YAXAIAH_NIIII@Z: %stackTop"}
!370 = distinct !{!370, !"\01?StackPush2@@YAXAIAH_NIIII@Z"}
!371 = !{!372}
!372 = distinct !{!372, !373, !"\01?StackPush@@YAXAIAHIII@Z: %stackTop"}
!373 = distinct !{!373, !"\01?StackPush@@YAXAIAHIII@Z"}
!374 = !{!375, !377}
!375 = distinct !{!375, !376, !"\01?CompressBox@@YAXUBoundingBox@@V?$vector@I$01@@AIAV?$vector@I$03@@2@Z: %data1"}
!376 = distinct !{!376, !"\01?CompressBox@@YAXUBoundingBox@@V?$vector@I$01@@AIAV?$vector@I$03@@2@Z"}
!377 = distinct !{!377, !376, !"\01?CompressBox@@YAXUBoundingBox@@V?$vector@I$01@@AIAV?$vector@I$03@@2@Z: %data2"}
