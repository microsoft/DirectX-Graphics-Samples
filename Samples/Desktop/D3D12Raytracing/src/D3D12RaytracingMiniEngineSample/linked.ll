; ModuleID = 'main'
target triple = "dxil-ms-dx"

%Constants = type { i32, i32, i32, i32, i32, i32, i32, i32 }
%struct.ByteAddressBuffer = type { i32 }
%"class.Texture2D<float>" = type { float, %"class.Texture2D<float>::mips_type" }
%"class.Texture2D<float>::mips_type" = type { i32 }
%"class.Texture2D<vector<float, 3> >" = type { <3 x float>, %"class.Texture2D<vector<float, 3> >::mips_type" }
%"class.Texture2D<vector<float, 3> >::mips_type" = type { i32 }
%AccelerationStructureList = type { <2 x i32> }
%struct.RWByteAddressBuffer = type { i32 }
%"class.RWTexture2D<vector<float, 4> >" = type { <4 x float> }
%"class.Texture2D<vector<float, 4> >" = type { <4 x float>, %"class.Texture2D<vector<float, 4> >::mips_type" }
%"class.Texture2D<vector<float, 4> >::mips_type" = type { i32 }
%"class.StructuredBuffer<RayTraceMeshInfo>" = type { %struct.RayTraceMeshInfo }
%struct.RayTraceMeshInfo = type { i32, i32, i32, i32, i32, i32, i32 }
%struct.SamplerState = type { i32 }
%struct.SamplerComparisonState = type { i32 }
%dx.alignment.legacy.b1 = type { %dx.alignment.legacy.struct.DynamicCB }
%dx.alignment.legacy.struct.DynamicCB = type { [4 x <4 x float>], <3 x float>, i32, <2 x float> }
%dx.alignment.legacy.HitShaderConstants = type { <3 x float>, <3 x float>, <3 x float>, <4 x float>, [4 x <4 x float>], i32, i32 }
%dx.types.CBufRet.f32 = type { float, float, float, float }
%dx.types.Handle = type { i8* }
%dx.types.CBufRet.i32 = type { i32, i32, i32, i32 }
%dx.types.ResRet.f32 = type { float, float, float, float, i32 }
%dx.types.ResRet.i32 = type { i32, i32, i32, i32, i32 }
%struct.RaytracingAccelerationStructure = type { i32 }
%b1 = type { %struct.DynamicCB }
%struct.DynamicCB = type { %class.matrix.float.4.4, <3 x float>, i32, <2 x float> }
%class.matrix.float.4.4 = type { [4 x <4 x float>] }
%HitShaderConstants = type { <3 x float>, <3 x float>, <3 x float>, <4 x float>, %class.matrix.float.4.4, i32, i32 }

@Constants = external constant %Constants
@"\01?RayGenShaderTable@@3UByteAddressBuffer@@A" = external global %struct.ByteAddressBuffer
@"\01?depth@@3V?$Texture2D@M@@A" = external global %"class.Texture2D<float>"
@"\01?normals@@3V?$Texture2D@V?$vector@M$02@@@@A" = external global %"class.Texture2D<vector<float, 3> >"
@AccelerationStructureList = external constant %AccelerationStructureList
@"\01?MissShaderTable@@3UByteAddressBuffer@@A" = external global %struct.ByteAddressBuffer
@"\01?stack@@3PAIA" = addrspace(3) global [2048 x i32] zeroinitializer
@"\01?HitGroupShaderTable@@3UByteAddressBuffer@@A" = external global %struct.ByteAddressBuffer
@"\01?DescriptorHeapBufferTable@@3PAURWByteAddressBuffer@@A" = external global [0 x %struct.RWByteAddressBuffer]
@"\01?g_screenOutput@@3V?$RWTexture2D@V?$vector@M$03@@@@A" = external global %"class.RWTexture2D<vector<float, 4> >"
@"\01?g_attributes@@3UByteAddressBuffer@@A" = external global %struct.ByteAddressBuffer
@"\01?g_texNormal@@3PAV?$Texture2D@V?$vector@M$03@@@@A" = external global [27 x %"class.Texture2D<vector<float, 4> >"]
@"\01?texSSAO@@3V?$Texture2D@M@@A" = external global %"class.Texture2D<float>"
@"\01?texShadow@@3V?$Texture2D@M@@A" = external global %"class.Texture2D<float>"
@"\01?g_meshInfo@@3V?$StructuredBuffer@URayTraceMeshInfo@@@@A" = external global %"class.StructuredBuffer<RayTraceMeshInfo>"
@"\01?g_indices@@3UByteAddressBuffer@@A" = external global %struct.ByteAddressBuffer
@"\01?g_texDiffuse@@3PAV?$Texture2D@V?$vector@M$03@@@@A" = external global [27 x %"class.Texture2D<vector<float, 4> >"]
@"\01?g_s0@@3USamplerState@@A" = external global %struct.SamplerState
@"\01?shadowSampler@@3USamplerComparisonState@@A" = external global %struct.SamplerComparisonState
@b1_legacy = external global %dx.alignment.legacy.b1
@HitShaderConstants_legacy = external global %dx.alignment.legacy.HitShaderConstants
@llvm.used = appending global [20 x i8*] [i8* bitcast (%dx.alignment.legacy.b1* @b1_legacy to i8*), i8* bitcast (%AccelerationStructureList* @AccelerationStructureList to i8*), i8* bitcast (%dx.alignment.legacy.HitShaderConstants* @HitShaderConstants_legacy to i8*), i8* bitcast (%Constants* @Constants to i8*), i8* bitcast ([27 x %"class.Texture2D<vector<float, 4> >"]* @"\01?g_texNormal@@3PAV?$Texture2D@V?$vector@M$03@@@@A" to i8*), i8* bitcast (%"class.StructuredBuffer<RayTraceMeshInfo>"* @"\01?g_meshInfo@@3V?$StructuredBuffer@URayTraceMeshInfo@@@@A" to i8*), i8* bitcast ([27 x %"class.Texture2D<vector<float, 4> >"]* @"\01?g_texDiffuse@@3PAV?$Texture2D@V?$vector@M$03@@@@A" to i8*), i8* bitcast (%struct.ByteAddressBuffer* @"\01?RayGenShaderTable@@3UByteAddressBuffer@@A" to i8*), i8* bitcast (%struct.ByteAddressBuffer* @"\01?g_indices@@3UByteAddressBuffer@@A" to i8*), i8* bitcast (%"class.Texture2D<float>"* @"\01?texSSAO@@3V?$Texture2D@M@@A" to i8*), i8* bitcast (%struct.ByteAddressBuffer* @"\01?g_attributes@@3UByteAddressBuffer@@A" to i8*), i8* bitcast (%struct.ByteAddressBuffer* @"\01?HitGroupShaderTable@@3UByteAddressBuffer@@A" to i8*), i8* bitcast (%"class.Texture2D<float>"* @"\01?depth@@3V?$Texture2D@M@@A" to i8*), i8* bitcast (%"class.Texture2D<float>"* @"\01?texShadow@@3V?$Texture2D@M@@A" to i8*), i8* bitcast (%struct.ByteAddressBuffer* @"\01?MissShaderTable@@3UByteAddressBuffer@@A" to i8*), i8* bitcast (%"class.Texture2D<vector<float, 3> >"* @"\01?normals@@3V?$Texture2D@V?$vector@M$02@@@@A" to i8*), i8* bitcast (%"class.RWTexture2D<vector<float, 4> >"* @"\01?g_screenOutput@@3V?$RWTexture2D@V?$vector@M$03@@@@A" to i8*), i8* bitcast ([0 x %struct.RWByteAddressBuffer]* @"\01?DescriptorHeapBufferTable@@3PAURWByteAddressBuffer@@A" to i8*), i8* bitcast (%struct.SamplerComparisonState* @"\01?shadowSampler@@3USamplerComparisonState@@A" to i8*), i8* bitcast (%struct.SamplerState* @"\01?g_s0@@3USamplerState@@A" to i8*)], section "llvm.metadata"

; Function Attrs: nounwind readonly
declare %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32, %dx.types.Handle, i32) #0

; Function Attrs: nounwind readnone
declare float @dx.op.tertiary.f32(i32, float, float, float) #1

; Function Attrs: nounwind readonly
declare %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32, %dx.types.Handle, i32) #0

; Function Attrs: nounwind readonly
declare %dx.types.ResRet.f32 @dx.op.sampleCmpLevelZero.f32(i32, %dx.types.Handle, %dx.types.Handle, float, float, float, float, i32, i32, i32, float) #0

; Function Attrs: nounwind
declare void @dx.op.textureStore.f32(i32, %dx.types.Handle, i32, i32, i32, float, float, float, float, i8) #2

declare i32 @dx.op.flattenedThreadIdInGroup.i32(i32)

declare i32 @dx.op.threadId.i32(i32, i32)

; Function Attrs: nounwind readonly
declare %dx.types.ResRet.f32 @dx.op.textureLoad.f32(i32, %dx.types.Handle, i32, i32, i32, i32, i32, i32, i32) #0

; Function Attrs: nounwind readnone
declare float @dx.op.dot3.f32(i32, float, float, float, float, float, float) #1

; Function Attrs: nounwind readonly
declare %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32, %dx.types.Handle, i32, i32) #0

; Function Attrs: nounwind readonly
declare %dx.types.ResRet.f32 @dx.op.sampleGrad.f32(i32, %dx.types.Handle, %dx.types.Handle, float, float, float, float, i32, i32, i32, float, float, float, float, float, float, float) #0

; Function Attrs: nounwind readnone
declare float @dx.op.binary.f32(i32, float, float) #1

; Function Attrs: nounwind readnone
declare float @dx.op.unary.f32(i32, float) #1

define void @main() {
  %g_screenOutput_UAV_2d = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 1, i32 0, i32 2, i1 false)
  %normals_texture_2d = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 0, i32 11, i32 2, i1 false)
  %MissShaderTable_texture_rawbuf = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 0, i32 10, i32 1, i1 false)
  %texShadow_texture_2d = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 0, i32 9, i32 3, i1 false)
  %depth_texture_2d = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 0, i32 8, i32 1, i1 false)
  %HitGroupShaderTable_texture_rawbuf = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 0, i32 7, i32 0, i1 false)
  %g_attributes_texture_rawbuf = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 0, i32 6, i32 2, i1 false)
  %texSSAO_texture_2d = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 0, i32 5, i32 4, i1 false)
  %g_indices_texture_rawbuf = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 0, i32 4, i32 1, i1 false)
  %RayGenShaderTable_texture_rawbuf = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 0, i32 3, i32 2, i1 false)
  %g_meshInfo_texture_structbuf = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 0, i32 1, i32 0, i1 false)
  %g_s0_sampler = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 3, i32 1, i32 0, i1 false)
  %shadowSampler_sampler = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 3, i32 0, i32 1, i1 false)
  %Constants_cbuffer = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 2, i32 3, i32 0, i1 false)
  %HitShaderConstants_cbuffer = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 2, i32 2, i32 0, i1 false)
  %AccelerationStructureList_cbuffer = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 2, i32 1, i32 1, i1 false)
  %b1_cbuffer = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 2, i32 0, i32 1, i1 false)
  %nodesToProcess.i.i.i.i.i = alloca [2 x i32], align 4
  %v11.i.28.i.i = alloca [3 x float], align 4
  %v12.i.29.i.i = alloca [3 x float], align 4
  %v13.i.30.i.i = alloca [3 x float], align 4
  %v14.i.31.i.i = alloca [3 x float], align 4
  %v15.i.32.i.i = alloca [3 x float], align 4
  %theStack.i = alloca [256 x i32], align 4
  %1 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %RayGenShaderTable_texture_rawbuf, i32 0, i32 undef)
  %2 = extractvalue %dx.types.ResRet.i32 %1, 0
  %3 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %Constants_cbuffer, i32 0)
  %4 = extractvalue %dx.types.CBufRet.i32 %3, 1
  %5 = extractvalue %dx.types.CBufRet.i32 %3, 0
  %DTidx.i = call i32 @dx.op.threadId.i32(i32 93, i32 0) #2
  %DTidy.i = call i32 @dx.op.threadId.i32(i32 93, i32 1) #2
  %groupIndex.i = call i32 @dx.op.flattenedThreadIdInGroup.i32(i32 96) #2
  %cmp.i = icmp sge i32 %DTidx.i, %5
  %cmp7.i = icmp sge i32 %DTidy.i, %4
  %or.cond = or i1 %cmp.i, %cmp7.i
  br i1 %or.cond, label %fb_Fallback_Scheduler.exit, label %if.end.i

if.end.i:                                         ; preds = %0
  %arrayidx.i.4.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 255
  store i32 -1, i32* %arrayidx.i.4.i, align 4
  br label %while.cond.i

while.cond.i:                                     ; preds = %state_1001.RayGen.ss_1.i.i, %Hit.ss_1.exit.i.i, %Miss.ss_0.exit.i.i, %state_1006.Fallback_TraceRay.ss_1.i.i, %25, %remat_begin.i.i.i, %remat_begin.i.101.i.i, %Hit.BB0._crit_edge.i.i.i, %82, %remat_begin.i.290.i.i, %while.body.i, %if.end.i
  %.i0 = phi float [ undef, %if.end.i ], [ %.i0, %state_1001.RayGen.ss_1.i.i ], [ %.i0, %Hit.ss_1.exit.i.i ], [ %.i0, %Miss.ss_0.exit.i.i ], [ %.i0, %state_1006.Fallback_TraceRay.ss_1.i.i ], [ %.i0, %25 ], [ %.i0, %remat_begin.i.i.i ], [ %.i0, %remat_begin.i.101.i.i ], [ %.i0, %Hit.BB0._crit_edge.i.i.i ], [ %.i037, %82 ], [ %.i037, %remat_begin.i.290.i.i ], [ %.i0, %while.body.i ]
  %.i1 = phi float [ undef, %if.end.i ], [ %.i1, %state_1001.RayGen.ss_1.i.i ], [ %.i1, %Hit.ss_1.exit.i.i ], [ %.i1, %Miss.ss_0.exit.i.i ], [ %.i1, %state_1006.Fallback_TraceRay.ss_1.i.i ], [ %.i1, %25 ], [ %.i1, %remat_begin.i.i.i ], [ %.i1, %remat_begin.i.101.i.i ], [ %.i1, %Hit.BB0._crit_edge.i.i.i ], [ %.i138, %82 ], [ %.i138, %remat_begin.i.290.i.i ], [ %.i1, %while.body.i ]
  %.i2 = phi float [ undef, %if.end.i ], [ %.i2, %state_1001.RayGen.ss_1.i.i ], [ %.i2, %Hit.ss_1.exit.i.i ], [ %.i2, %Miss.ss_0.exit.i.i ], [ %.i2, %state_1006.Fallback_TraceRay.ss_1.i.i ], [ %.i2, %25 ], [ %.i2, %remat_begin.i.i.i ], [ %.i2, %remat_begin.i.101.i.i ], [ %.i2, %Hit.BB0._crit_edge.i.i.i ], [ %.i239, %82 ], [ %.i239, %remat_begin.i.290.i.i ], [ %.i2, %while.body.i ]
  %.i3 = phi float [ undef, %if.end.i ], [ %.i3, %state_1001.RayGen.ss_1.i.i ], [ %.i3, %Hit.ss_1.exit.i.i ], [ %.i3, %Miss.ss_0.exit.i.i ], [ %.i3, %state_1006.Fallback_TraceRay.ss_1.i.i ], [ %.i3, %25 ], [ %.i3, %remat_begin.i.i.i ], [ %.i3, %remat_begin.i.101.i.i ], [ %.i3, %Hit.BB0._crit_edge.i.i.i ], [ %.i340, %82 ], [ %.i340, %remat_begin.i.290.i.i ], [ %.i3, %while.body.i ]
  %.i4 = phi float [ undef, %if.end.i ], [ %.i4, %state_1001.RayGen.ss_1.i.i ], [ %.i4, %Hit.ss_1.exit.i.i ], [ %.i4, %Miss.ss_0.exit.i.i ], [ %.i4, %state_1006.Fallback_TraceRay.ss_1.i.i ], [ %.i4, %25 ], [ %.i4, %remat_begin.i.i.i ], [ %.i4, %remat_begin.i.101.i.i ], [ %.i4, %Hit.BB0._crit_edge.i.i.i ], [ %.i441, %82 ], [ %.i441, %remat_begin.i.290.i.i ], [ %.i4, %while.body.i ]
  %.i5 = phi float [ undef, %if.end.i ], [ %.i5, %state_1001.RayGen.ss_1.i.i ], [ %.i5, %Hit.ss_1.exit.i.i ], [ %.i5, %Miss.ss_0.exit.i.i ], [ %.i5, %state_1006.Fallback_TraceRay.ss_1.i.i ], [ %.i5, %25 ], [ %.i5, %remat_begin.i.i.i ], [ %.i5, %remat_begin.i.101.i.i ], [ %.i5, %Hit.BB0._crit_edge.i.i.i ], [ %.i542, %82 ], [ %.i542, %remat_begin.i.290.i.i ], [ %.i5, %while.body.i ]
  %.i6 = phi float [ undef, %if.end.i ], [ %.i6, %state_1001.RayGen.ss_1.i.i ], [ %.i6, %Hit.ss_1.exit.i.i ], [ %.i6, %Miss.ss_0.exit.i.i ], [ %.i6, %state_1006.Fallback_TraceRay.ss_1.i.i ], [ %.i6, %25 ], [ %.i6, %remat_begin.i.i.i ], [ %.i6, %remat_begin.i.101.i.i ], [ %.i6, %Hit.BB0._crit_edge.i.i.i ], [ %.i643, %82 ], [ %.i643, %remat_begin.i.290.i.i ], [ %.i6, %while.body.i ]
  %.i7 = phi float [ undef, %if.end.i ], [ %.i7, %state_1001.RayGen.ss_1.i.i ], [ %.i7, %Hit.ss_1.exit.i.i ], [ %.i7, %Miss.ss_0.exit.i.i ], [ %.i7, %state_1006.Fallback_TraceRay.ss_1.i.i ], [ %.i7, %25 ], [ %.i7, %remat_begin.i.i.i ], [ %.i7, %remat_begin.i.101.i.i ], [ %.i7, %Hit.BB0._crit_edge.i.i.i ], [ %.i744, %82 ], [ %.i744, %remat_begin.i.290.i.i ], [ %.i7, %while.body.i ]
  %.i8 = phi float [ undef, %if.end.i ], [ %.i8, %state_1001.RayGen.ss_1.i.i ], [ %.i8, %Hit.ss_1.exit.i.i ], [ %.i8, %Miss.ss_0.exit.i.i ], [ %.i8, %state_1006.Fallback_TraceRay.ss_1.i.i ], [ %.i8, %25 ], [ %.i8, %remat_begin.i.i.i ], [ %.i8, %remat_begin.i.101.i.i ], [ %.i8, %Hit.BB0._crit_edge.i.i.i ], [ %.i845, %82 ], [ %.i845, %remat_begin.i.290.i.i ], [ %.i8, %while.body.i ]
  %.i9 = phi float [ undef, %if.end.i ], [ %.i9, %state_1001.RayGen.ss_1.i.i ], [ %.i9, %Hit.ss_1.exit.i.i ], [ %.i9, %Miss.ss_0.exit.i.i ], [ %.i9, %state_1006.Fallback_TraceRay.ss_1.i.i ], [ %.i9, %25 ], [ %.i9, %remat_begin.i.i.i ], [ %.i9, %remat_begin.i.101.i.i ], [ %.i9, %Hit.BB0._crit_edge.i.i.i ], [ %.i946, %82 ], [ %.i946, %remat_begin.i.290.i.i ], [ %.i9, %while.body.i ]
  %.i10 = phi float [ undef, %if.end.i ], [ %.i10, %state_1001.RayGen.ss_1.i.i ], [ %.i10, %Hit.ss_1.exit.i.i ], [ %.i10, %Miss.ss_0.exit.i.i ], [ %.i10, %state_1006.Fallback_TraceRay.ss_1.i.i ], [ %.i10, %25 ], [ %.i10, %remat_begin.i.i.i ], [ %.i10, %remat_begin.i.101.i.i ], [ %.i10, %Hit.BB0._crit_edge.i.i.i ], [ %.i1047, %82 ], [ %.i1047, %remat_begin.i.290.i.i ], [ %.i10, %while.body.i ]
  %.i11 = phi float [ undef, %if.end.i ], [ %.i11, %state_1001.RayGen.ss_1.i.i ], [ %.i11, %Hit.ss_1.exit.i.i ], [ %.i11, %Miss.ss_0.exit.i.i ], [ %.i11, %state_1006.Fallback_TraceRay.ss_1.i.i ], [ %.i11, %25 ], [ %.i11, %remat_begin.i.i.i ], [ %.i11, %remat_begin.i.101.i.i ], [ %.i11, %Hit.BB0._crit_edge.i.i.i ], [ %.i1148, %82 ], [ %.i1148, %remat_begin.i.290.i.i ], [ %.i11, %while.body.i ]
  %.i013 = phi float [ undef, %if.end.i ], [ %.i013, %state_1001.RayGen.ss_1.i.i ], [ %.i013, %Hit.ss_1.exit.i.i ], [ %.i013, %Miss.ss_0.exit.i.i ], [ %.i013, %state_1006.Fallback_TraceRay.ss_1.i.i ], [ %.i013, %25 ], [ %.i013, %remat_begin.i.i.i ], [ %.i013, %remat_begin.i.101.i.i ], [ %.i013, %Hit.BB0._crit_edge.i.i.i ], [ %.i061, %82 ], [ %.i061, %remat_begin.i.290.i.i ], [ %.i013, %while.body.i ]
  %.i114 = phi float [ undef, %if.end.i ], [ %.i114, %state_1001.RayGen.ss_1.i.i ], [ %.i114, %Hit.ss_1.exit.i.i ], [ %.i114, %Miss.ss_0.exit.i.i ], [ %.i114, %state_1006.Fallback_TraceRay.ss_1.i.i ], [ %.i114, %25 ], [ %.i114, %remat_begin.i.i.i ], [ %.i114, %remat_begin.i.101.i.i ], [ %.i114, %Hit.BB0._crit_edge.i.i.i ], [ %.i162, %82 ], [ %.i162, %remat_begin.i.290.i.i ], [ %.i114, %while.body.i ]
  %.i215 = phi float [ undef, %if.end.i ], [ %.i215, %state_1001.RayGen.ss_1.i.i ], [ %.i215, %Hit.ss_1.exit.i.i ], [ %.i215, %Miss.ss_0.exit.i.i ], [ %.i215, %state_1006.Fallback_TraceRay.ss_1.i.i ], [ %.i215, %25 ], [ %.i215, %remat_begin.i.i.i ], [ %.i215, %remat_begin.i.101.i.i ], [ %.i215, %Hit.BB0._crit_edge.i.i.i ], [ %.i263, %82 ], [ %.i263, %remat_begin.i.290.i.i ], [ %.i215, %while.body.i ]
  %.i316 = phi float [ undef, %if.end.i ], [ %.i316, %state_1001.RayGen.ss_1.i.i ], [ %.i316, %Hit.ss_1.exit.i.i ], [ %.i316, %Miss.ss_0.exit.i.i ], [ %.i316, %state_1006.Fallback_TraceRay.ss_1.i.i ], [ %.i316, %25 ], [ %.i316, %remat_begin.i.i.i ], [ %.i316, %remat_begin.i.101.i.i ], [ %.i316, %Hit.BB0._crit_edge.i.i.i ], [ %.i364, %82 ], [ %.i364, %remat_begin.i.290.i.i ], [ %.i316, %while.body.i ]
  %.i417 = phi float [ undef, %if.end.i ], [ %.i417, %state_1001.RayGen.ss_1.i.i ], [ %.i417, %Hit.ss_1.exit.i.i ], [ %.i417, %Miss.ss_0.exit.i.i ], [ %.i417, %state_1006.Fallback_TraceRay.ss_1.i.i ], [ %.i417, %25 ], [ %.i417, %remat_begin.i.i.i ], [ %.i417, %remat_begin.i.101.i.i ], [ %.i417, %Hit.BB0._crit_edge.i.i.i ], [ %.i465, %82 ], [ %.i465, %remat_begin.i.290.i.i ], [ %.i417, %while.body.i ]
  %.i518 = phi float [ undef, %if.end.i ], [ %.i518, %state_1001.RayGen.ss_1.i.i ], [ %.i518, %Hit.ss_1.exit.i.i ], [ %.i518, %Miss.ss_0.exit.i.i ], [ %.i518, %state_1006.Fallback_TraceRay.ss_1.i.i ], [ %.i518, %25 ], [ %.i518, %remat_begin.i.i.i ], [ %.i518, %remat_begin.i.101.i.i ], [ %.i518, %Hit.BB0._crit_edge.i.i.i ], [ %.i566, %82 ], [ %.i566, %remat_begin.i.290.i.i ], [ %.i518, %while.body.i ]
  %.i619 = phi float [ undef, %if.end.i ], [ %.i619, %state_1001.RayGen.ss_1.i.i ], [ %.i619, %Hit.ss_1.exit.i.i ], [ %.i619, %Miss.ss_0.exit.i.i ], [ %.i619, %state_1006.Fallback_TraceRay.ss_1.i.i ], [ %.i619, %25 ], [ %.i619, %remat_begin.i.i.i ], [ %.i619, %remat_begin.i.101.i.i ], [ %.i619, %Hit.BB0._crit_edge.i.i.i ], [ %.i667, %82 ], [ %.i667, %remat_begin.i.290.i.i ], [ %.i619, %while.body.i ]
  %.i720 = phi float [ undef, %if.end.i ], [ %.i720, %state_1001.RayGen.ss_1.i.i ], [ %.i720, %Hit.ss_1.exit.i.i ], [ %.i720, %Miss.ss_0.exit.i.i ], [ %.i720, %state_1006.Fallback_TraceRay.ss_1.i.i ], [ %.i720, %25 ], [ %.i720, %remat_begin.i.i.i ], [ %.i720, %remat_begin.i.101.i.i ], [ %.i720, %Hit.BB0._crit_edge.i.i.i ], [ %.i768, %82 ], [ %.i768, %remat_begin.i.290.i.i ], [ %.i720, %while.body.i ]
  %.i821 = phi float [ undef, %if.end.i ], [ %.i821, %state_1001.RayGen.ss_1.i.i ], [ %.i821, %Hit.ss_1.exit.i.i ], [ %.i821, %Miss.ss_0.exit.i.i ], [ %.i821, %state_1006.Fallback_TraceRay.ss_1.i.i ], [ %.i821, %25 ], [ %.i821, %remat_begin.i.i.i ], [ %.i821, %remat_begin.i.101.i.i ], [ %.i821, %Hit.BB0._crit_edge.i.i.i ], [ %.i869, %82 ], [ %.i869, %remat_begin.i.290.i.i ], [ %.i821, %while.body.i ]
  %.i922 = phi float [ undef, %if.end.i ], [ %.i922, %state_1001.RayGen.ss_1.i.i ], [ %.i922, %Hit.ss_1.exit.i.i ], [ %.i922, %Miss.ss_0.exit.i.i ], [ %.i922, %state_1006.Fallback_TraceRay.ss_1.i.i ], [ %.i922, %25 ], [ %.i922, %remat_begin.i.i.i ], [ %.i922, %remat_begin.i.101.i.i ], [ %.i922, %Hit.BB0._crit_edge.i.i.i ], [ %.i970, %82 ], [ %.i970, %remat_begin.i.290.i.i ], [ %.i922, %while.body.i ]
  %.i1023 = phi float [ undef, %if.end.i ], [ %.i1023, %state_1001.RayGen.ss_1.i.i ], [ %.i1023, %Hit.ss_1.exit.i.i ], [ %.i1023, %Miss.ss_0.exit.i.i ], [ %.i1023, %state_1006.Fallback_TraceRay.ss_1.i.i ], [ %.i1023, %25 ], [ %.i1023, %remat_begin.i.i.i ], [ %.i1023, %remat_begin.i.101.i.i ], [ %.i1023, %Hit.BB0._crit_edge.i.i.i ], [ %.i1071, %82 ], [ %.i1071, %remat_begin.i.290.i.i ], [ %.i1023, %while.body.i ]
  %.i1124 = phi float [ undef, %if.end.i ], [ %.i1124, %state_1001.RayGen.ss_1.i.i ], [ %.i1124, %Hit.ss_1.exit.i.i ], [ %.i1124, %Miss.ss_0.exit.i.i ], [ %.i1124, %state_1006.Fallback_TraceRay.ss_1.i.i ], [ %.i1124, %25 ], [ %.i1124, %remat_begin.i.i.i ], [ %.i1124, %remat_begin.i.101.i.i ], [ %.i1124, %Hit.BB0._crit_edge.i.i.i ], [ %.i1172, %82 ], [ %.i1172, %remat_begin.i.290.i.i ], [ %.i1124, %while.body.i ]
  %6 = phi i32 [ 255, %if.end.i ], [ %add.i.i.9.i.i, %state_1001.RayGen.ss_1.i.i ], [ %add.i.i.173.i.i, %Hit.ss_1.exit.i.i ], [ %6, %Miss.ss_0.exit.i.i ], [ %add.i.6.i.628.i.i, %state_1006.Fallback_TraceRay.ss_1.i.i ], [ %sub.i.i.i.i, %remat_begin.i.i.i ], [ %6, %25 ], [ %6, %Hit.BB0._crit_edge.i.i.i ], [ %sub.i.i.17.i.i, %remat_begin.i.101.i.i ], [ %sub.i.137.i.i.i, %remat_begin.i.290.i.i ], [ %6, %82 ], [ %6, %while.body.i ]
  %7 = phi i32 [ 3333, %if.end.i ], [ %7, %state_1001.RayGen.ss_1.i.i ], [ %7, %Hit.ss_1.exit.i.i ], [ %7, %Miss.ss_0.exit.i.i ], [ %126, %state_1006.Fallback_TraceRay.ss_1.i.i ], [ %7, %25 ], [ %7, %remat_begin.i.i.i ], [ %7, %remat_begin.i.101.i.i ], [ %7, %Hit.BB0._crit_edge.i.i.i ], [ %75, %remat_begin.i.290.i.i ], [ %84, %82 ], [ %7, %while.body.i ]
  %8 = phi i32 [ 2222, %if.end.i ], [ %8, %state_1001.RayGen.ss_1.i.i ], [ %8, %Hit.ss_1.exit.i.i ], [ %8, %Miss.ss_0.exit.i.i ], [ %125, %state_1006.Fallback_TraceRay.ss_1.i.i ], [ %8, %25 ], [ %8, %remat_begin.i.i.i ], [ %8, %remat_begin.i.101.i.i ], [ %8, %Hit.BB0._crit_edge.i.i.i ], [ %76, %remat_begin.i.290.i.i ], [ %83, %82 ], [ %8, %while.body.i ]
  %9 = phi i32 [ 1111, %if.end.i ], [ %9, %state_1001.RayGen.ss_1.i.i ], [ %9, %Hit.ss_1.exit.i.i ], [ %9, %Miss.ss_0.exit.i.i ], [ %v24.int.i.i.i, %state_1006.Fallback_TraceRay.ss_1.i.i ], [ %9, %25 ], [ %9, %remat_begin.i.i.i ], [ %9, %remat_begin.i.101.i.i ], [ %9, %Hit.BB0._crit_edge.i.i.i ], [ %arg13.int.i.i.i, %remat_begin.i.290.i.i ], [ %9, %82 ], [ %9, %while.body.i ]
  %10 = phi i32 [ undef, %if.end.i ], [ %10, %state_1001.RayGen.ss_1.i.i ], [ %10, %Hit.ss_1.exit.i.i ], [ %10, %Miss.ss_0.exit.i.i ], [ %10, %state_1006.Fallback_TraceRay.ss_1.i.i ], [ %10, %25 ], [ %10, %remat_begin.i.i.i ], [ %10, %remat_begin.i.101.i.i ], [ %10, %Hit.BB0._crit_edge.i.i.i ], [ %78, %82 ], [ %78, %remat_begin.i.290.i.i ], [ %10, %while.body.i ]
  %11 = phi i32 [ undef, %if.end.i ], [ %11, %state_1001.RayGen.ss_1.i.i ], [ %11, %Hit.ss_1.exit.i.i ], [ %11, %Miss.ss_0.exit.i.i ], [ %11, %state_1006.Fallback_TraceRay.ss_1.i.i ], [ %11, %25 ], [ %11, %remat_begin.i.i.i ], [ %11, %remat_begin.i.101.i.i ], [ %11, %Hit.BB0._crit_edge.i.i.i ], [ %79, %82 ], [ %79, %remat_begin.i.290.i.i ], [ %11, %while.body.i ]
  %12 = phi float [ undef, %if.end.i ], [ %12, %state_1001.RayGen.ss_1.i.i ], [ %12, %Hit.ss_1.exit.i.i ], [ %12, %Miss.ss_0.exit.i.i ], [ %12, %state_1006.Fallback_TraceRay.ss_1.i.i ], [ %12, %25 ], [ %12, %remat_begin.i.i.i ], [ %12, %remat_begin.i.101.i.i ], [ %12, %Hit.BB0._crit_edge.i.i.i ], [ %70, %82 ], [ %70, %remat_begin.i.290.i.i ], [ %12, %while.body.i ]
  %13 = phi float [ undef, %if.end.i ], [ %13, %state_1001.RayGen.ss_1.i.i ], [ %13, %Hit.ss_1.exit.i.i ], [ %13, %Miss.ss_0.exit.i.i ], [ %13, %state_1006.Fallback_TraceRay.ss_1.i.i ], [ %13, %25 ], [ %13, %remat_begin.i.i.i ], [ %13, %remat_begin.i.101.i.i ], [ %13, %Hit.BB0._crit_edge.i.i.i ], [ %69, %82 ], [ %69, %remat_begin.i.290.i.i ], [ %13, %while.body.i ]
  %14 = phi float [ undef, %if.end.i ], [ %14, %state_1001.RayGen.ss_1.i.i ], [ %14, %Hit.ss_1.exit.i.i ], [ %14, %Miss.ss_0.exit.i.i ], [ %14, %state_1006.Fallback_TraceRay.ss_1.i.i ], [ %14, %25 ], [ %14, %remat_begin.i.i.i ], [ %14, %remat_begin.i.101.i.i ], [ %14, %Hit.BB0._crit_edge.i.i.i ], [ %68, %82 ], [ %68, %remat_begin.i.290.i.i ], [ %14, %while.body.i ]
  %15 = phi float [ undef, %if.end.i ], [ %15, %state_1001.RayGen.ss_1.i.i ], [ %15, %Hit.ss_1.exit.i.i ], [ %15, %Miss.ss_0.exit.i.i ], [ %15, %state_1006.Fallback_TraceRay.ss_1.i.i ], [ %15, %25 ], [ %15, %remat_begin.i.i.i ], [ %15, %remat_begin.i.101.i.i ], [ %15, %Hit.BB0._crit_edge.i.i.i ], [ %66, %82 ], [ %66, %remat_begin.i.290.i.i ], [ %15, %while.body.i ]
  %16 = phi float [ undef, %if.end.i ], [ %16, %state_1001.RayGen.ss_1.i.i ], [ %16, %Hit.ss_1.exit.i.i ], [ %16, %Miss.ss_0.exit.i.i ], [ %16, %state_1006.Fallback_TraceRay.ss_1.i.i ], [ %16, %25 ], [ %16, %remat_begin.i.i.i ], [ %16, %remat_begin.i.101.i.i ], [ %16, %Hit.BB0._crit_edge.i.i.i ], [ %65, %82 ], [ %65, %remat_begin.i.290.i.i ], [ %16, %while.body.i ]
  %17 = phi float [ undef, %if.end.i ], [ %17, %state_1001.RayGen.ss_1.i.i ], [ %17, %Hit.ss_1.exit.i.i ], [ %17, %Miss.ss_0.exit.i.i ], [ %17, %state_1006.Fallback_TraceRay.ss_1.i.i ], [ %17, %25 ], [ %17, %remat_begin.i.i.i ], [ %17, %remat_begin.i.101.i.i ], [ %17, %Hit.BB0._crit_edge.i.i.i ], [ %64, %82 ], [ %64, %remat_begin.i.290.i.i ], [ %17, %while.body.i ]
  %18 = phi float [ undef, %if.end.i ], [ %18, %state_1001.RayGen.ss_1.i.i ], [ %18, %Hit.ss_1.exit.i.i ], [ %18, %Miss.ss_0.exit.i.i ], [ %18, %state_1006.Fallback_TraceRay.ss_1.i.i ], [ %18, %25 ], [ %18, %remat_begin.i.i.i ], [ %18, %remat_begin.i.101.i.i ], [ %18, %Hit.BB0._crit_edge.i.i.i ], [ %80, %82 ], [ %80, %remat_begin.i.290.i.i ], [ %18, %while.body.i ]
  %stateId.0.i = phi i32 [ %2, %if.end.i ], [ %ret.stateId.i.13.i.i, %state_1001.RayGen.ss_1.i.i ], [ %ret.stateId.i.177.i.i, %Hit.ss_1.exit.i.i ], [ %ret.stateId.i.193.i.i, %Miss.ss_0.exit.i.i ], [ %ret.stateId.i.642.i.i, %state_1006.Fallback_TraceRay.ss_1.i.i ], [ 1005, %remat_begin.i.i.i ], [ %ret.stateId.i.i.i, %25 ], [ %ret.stateId.i.37.i.i, %Hit.BB0._crit_edge.i.i.i ], [ 1005, %remat_begin.i.101.i.i ], [ %stateID.i.0.i.i.i, %remat_begin.i.290.i.i ], [ %ret.stateId.i.295.i.i, %82 ], [ -3, %while.body.i ]
  %cmp11.i = icmp sge i32 %stateId.0.i, 0
  br i1 %cmp11.i, label %while.body.i, label %fb_Fallback_Scheduler.exit

while.body.i:                                     ; preds = %while.cond.i
  switch i32 %stateId.0.i, label %while.cond.i [
    i32 1000, label %state_1000.RayGen.ss_0.i.i
    i32 1001, label %state_1001.RayGen.ss_1.i.i
    i32 1002, label %state_1002.Hit.ss_0.i.i
    i32 1003, label %state_1003.Hit.ss_1.i.i
    i32 1004, label %state_1004.Miss.ss_0.i.i
    i32 1005, label %state_1005.Fallback_TraceRay.ss_0.i.i
    i32 1006, label %state_1006.Fallback_TraceRay.ss_1.i.i
  ]

state_1000.RayGen.ss_0.i.i:                       ; preds = %while.body.i
  %sub.i.i.i.i = sub nsw i32 %6, 20
  %.i0.i.i.i = uitofp i32 %DTidx.i to float
  %.i1.i.i.i = uitofp i32 %DTidy.i to float
  %.i017.i.i.i = fadd fast float %.i0.i.i.i, 5.000000e-01
  %.i118.i.i.i = fadd fast float %.i1.i.i.i, 5.000000e-01
  %v4.i.i.i = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %b1_cbuffer, i32 5) #2
  %v5.i.i.i = extractvalue %dx.types.CBufRet.f32 %v4.i.i.i, 0
  %v6.i.i.i = extractvalue %dx.types.CBufRet.f32 %v4.i.i.i, 1
  %.i019.i.i.i = fdiv fast float %.i017.i.i.i, %v5.i.i.i
  %.i120.i.i.i = fdiv fast float %.i118.i.i.i, %v6.i.i.i
  %.i021.i.i.i = fmul fast float %.i019.i.i.i, 2.000000e+00
  %.i122.i.i.i = fmul fast float %.i120.i.i.i, 2.000000e+00
  %.i023.i.i.i = fadd fast float %.i021.i.i.i, -1.000000e+00
  %.i124.i.i.i = fadd fast float %.i122.i.i.i, -1.000000e+00
  %v7.i.i.i = fsub fast float -0.000000e+00, %.i124.i.i.i
  %v8.i.i.i = fptosi float %.i017.i.i.i to i32
  %v9.i.i.i = fptosi float %.i118.i.i.i to i32
  %TextureLoad.i.i.i = call %dx.types.ResRet.f32 @dx.op.textureLoad.f32(i32 66, %dx.types.Handle %depth_texture_2d, i32 0, i32 %v8.i.i.i, i32 %v9.i.i.i, i32 undef, i32 undef, i32 undef, i32 undef) #2
  %v11.i.i.i = extractvalue %dx.types.ResRet.f32 %TextureLoad.i.i.i, 0
  %TextureLoad2.i.i.i = call %dx.types.ResRet.f32 @dx.op.textureLoad.f32(i32 66, %dx.types.Handle %normals_texture_2d, i32 0, i32 %v8.i.i.i, i32 %v9.i.i.i, i32 undef, i32 undef, i32 undef, i32 undef) #2
  %v13.i.i.i = extractvalue %dx.types.ResRet.f32 %TextureLoad2.i.i.i, 0
  %v14.i.i.i = extractvalue %dx.types.ResRet.f32 %TextureLoad2.i.i.i, 1
  %v15.i.i.i = extractvalue %dx.types.ResRet.f32 %TextureLoad2.i.i.i, 2
  %v16.i.i.i = fmul fast float %v13.i.i.i, %v13.i.i.i
  %v17.i.i.i = fmul fast float %v14.i.i.i, %v14.i.i.i
  %v18.i.i.i = fadd fast float %v16.i.i.i, %v17.i.i.i
  %v19.i.i.i = fmul fast float %v15.i.i.i, %v15.i.i.i
  %v20.i.i.i = fadd fast float %v18.i.i.i, %v19.i.i.i
  %Sqrt.i.i.i = call float @dx.op.unary.f32(i32 24, float %v20.i.i.i) #2
  %v21.i.i.i = fcmp fast olt float %Sqrt.i.i.i, 0x3FB99999A0000000
  br i1 %v21.i.i.i, label %25, label %remat_begin.i.i.i

remat_begin.i.i.i:                                ; preds = %state_1000.RayGen.ss_0.i.i
  %.i025.i.i.i = fdiv fast float %v13.i.i.i, %Sqrt.i.i.i
  %.i126.i.i.i = fdiv fast float %v14.i.i.i, %Sqrt.i.i.i
  %.i2.i.i.i = fdiv fast float %v15.i.i.i, %Sqrt.i.i.i
  %v23.i.i.i = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %b1_cbuffer, i32 0) #2
  %v24.i.i.i = extractvalue %dx.types.CBufRet.f32 %v23.i.i.i, 0
  %v25.i.i.i = extractvalue %dx.types.CBufRet.f32 %v23.i.i.i, 1
  %v26.i.i.i = extractvalue %dx.types.CBufRet.f32 %v23.i.i.i, 2
  %v27.i.i.i = extractvalue %dx.types.CBufRet.f32 %v23.i.i.i, 3
  %v28.i.i.i = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %b1_cbuffer, i32 1) #2
  %v29.i.i.i = extractvalue %dx.types.CBufRet.f32 %v28.i.i.i, 0
  %v30.i.i.i = extractvalue %dx.types.CBufRet.f32 %v28.i.i.i, 1
  %v31.i.i.i = extractvalue %dx.types.CBufRet.f32 %v28.i.i.i, 2
  %v32.i.i.i = extractvalue %dx.types.CBufRet.f32 %v28.i.i.i, 3
  %v33.i.i.i = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %b1_cbuffer, i32 2) #2
  %v34.i.i.i = extractvalue %dx.types.CBufRet.f32 %v33.i.i.i, 0
  %v35.i.i.i = extractvalue %dx.types.CBufRet.f32 %v33.i.i.i, 1
  %v36.i.i.i = extractvalue %dx.types.CBufRet.f32 %v33.i.i.i, 2
  %v37.i.i.i = extractvalue %dx.types.CBufRet.f32 %v33.i.i.i, 3
  %v38.i.i.i = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %b1_cbuffer, i32 3) #2
  %v39.i.i.i = extractvalue %dx.types.CBufRet.f32 %v38.i.i.i, 0
  %v40.i.i.i = extractvalue %dx.types.CBufRet.f32 %v38.i.i.i, 1
  %v41.i.i.i = extractvalue %dx.types.CBufRet.f32 %v38.i.i.i, 2
  %v42.i.i.i = extractvalue %dx.types.CBufRet.f32 %v38.i.i.i, 3
  %v43.i.i.i = fmul fast float %v24.i.i.i, %.i023.i.i.i
  %FMad16.i.i.i = call float @dx.op.tertiary.f32(i32 46, float %v7.i.i.i, float %v25.i.i.i, float %v43.i.i.i) #2
  %FMad15.i.i.i = call float @dx.op.tertiary.f32(i32 46, float %v11.i.i.i, float %v26.i.i.i, float %FMad16.i.i.i) #2
  %FMad14.i.i.i = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %v27.i.i.i, float %FMad15.i.i.i) #2
  %v44.i.i.i = fmul fast float %v29.i.i.i, %.i023.i.i.i
  %FMad13.i.i.i = call float @dx.op.tertiary.f32(i32 46, float %v7.i.i.i, float %v30.i.i.i, float %v44.i.i.i) #2
  %FMad12.i.i.i = call float @dx.op.tertiary.f32(i32 46, float %v11.i.i.i, float %v31.i.i.i, float %FMad13.i.i.i) #2
  %FMad11.i.i.i = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %v32.i.i.i, float %FMad12.i.i.i) #2
  %v45.i.i.i = fmul fast float %v34.i.i.i, %.i023.i.i.i
  %FMad10.i.i.i = call float @dx.op.tertiary.f32(i32 46, float %v7.i.i.i, float %v35.i.i.i, float %v45.i.i.i) #2
  %FMad9.i.i.i = call float @dx.op.tertiary.f32(i32 46, float %v11.i.i.i, float %v36.i.i.i, float %FMad10.i.i.i) #2
  %FMad8.i.i.i = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %v37.i.i.i, float %FMad9.i.i.i) #2
  %v46.i.i.i = fmul fast float %v39.i.i.i, %.i023.i.i.i
  %FMad7.i.i.i = call float @dx.op.tertiary.f32(i32 46, float %v7.i.i.i, float %v40.i.i.i, float %v46.i.i.i) #2
  %FMad6.i.i.i = call float @dx.op.tertiary.f32(i32 46, float %v11.i.i.i, float %v41.i.i.i, float %FMad7.i.i.i) #2
  %FMad.i.i.i = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %v42.i.i.i, float %FMad6.i.i.i) #2
  %.i027.i.i.i = fdiv fast float %FMad14.i.i.i, %FMad.i.i.i
  %.i128.i.i.i = fdiv fast float %FMad11.i.i.i, %FMad.i.i.i
  %.i229.i.i.i = fdiv fast float %FMad8.i.i.i, %FMad.i.i.i
  %v47.i.i.i = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %b1_cbuffer, i32 4) #2
  %v48.i.i.i = extractvalue %dx.types.CBufRet.f32 %v47.i.i.i, 0
  %v49.i.i.i = extractvalue %dx.types.CBufRet.f32 %v47.i.i.i, 1
  %v50.i.i.i = extractvalue %dx.types.CBufRet.f32 %v47.i.i.i, 2
  %.i030.i.i.i = fsub fast float %v48.i.i.i, %.i027.i.i.i
  %.i131.i.i.i = fsub fast float %v49.i.i.i, %.i128.i.i.i
  %.i232.i.i.i = fsub fast float %v50.i.i.i, %.i229.i.i.i
  %v51.i.i.i = fmul fast float %.i030.i.i.i, %.i030.i.i.i
  %v52.i.i.i = fmul fast float %.i131.i.i.i, %.i131.i.i.i
  %v53.i.i.i = fadd fast float %v51.i.i.i, %v52.i.i.i
  %v54.i.i.i = fmul fast float %.i232.i.i.i, %.i232.i.i.i
  %v55.i.i.i = fadd fast float %v53.i.i.i, %v54.i.i.i
  %Sqrt4.i.i.i = call float @dx.op.unary.f32(i32 24, float %v55.i.i.i) #2
  %.i033.i.i.i = fdiv fast float %.i030.i.i.i, %Sqrt4.i.i.i
  %.i134.i.i.i = fdiv fast float %.i131.i.i.i, %Sqrt4.i.i.i
  %.i235.i.i.i = fdiv fast float %.i232.i.i.i, %Sqrt4.i.i.i
  %.i036.i.i.i = fsub fast float -0.000000e+00, %.i033.i.i.i
  %.i137.i.i.i = fsub fast float -0.000000e+00, %.i134.i.i.i
  %.i238.i.i.i = fsub fast float -0.000000e+00, %.i235.i.i.i
  %v56.i.i.i = call float @dx.op.dot3.f32(i32 55, float %.i036.i.i.i, float %.i137.i.i.i, float %.i238.i.i.i, float %.i025.i.i.i, float %.i126.i.i.i, float %.i2.i.i.i) #2
  %v57.i.i.i = fmul fast float %v56.i.i.i, 2.000000e+00
  %.i042.i.i.i = fmul fast float %v57.i.i.i, %.i025.i.i.i
  %.i143.i.i.i = fmul fast float %v57.i.i.i, %.i126.i.i.i
  %.i244.i.i.i = fmul fast float %v57.i.i.i, %.i2.i.i.i
  %.i045.i.i.i = fsub fast float %.i036.i.i.i, %.i042.i.i.i
  %.i146.i.i.i = fsub fast float %.i137.i.i.i, %.i143.i.i.i
  %.i247.i.i.i = fsub fast float %.i238.i.i.i, %.i244.i.i.i
  %v58.i.i.i = fmul fast float %.i045.i.i.i, %.i045.i.i.i
  %v59.i.i.i = fmul fast float %.i146.i.i.i, %.i146.i.i.i
  %v60.i.i.i = fadd fast float %v58.i.i.i, %v59.i.i.i
  %v61.i.i.i = fmul fast float %.i247.i.i.i, %.i247.i.i.i
  %v62.i.i.i = fadd fast float %v60.i.i.i, %v61.i.i.i
  %Sqrt5.i.i.i = call float @dx.op.unary.f32(i32 24, float %v62.i.i.i) #2
  %.i048.i.i.i = fdiv fast float %.i045.i.i.i, %Sqrt5.i.i.i
  %.i149.i.i.i = fdiv fast float %.i146.i.i.i, %Sqrt5.i.i.i
  %.i250.i.i.i = fdiv fast float %.i247.i.i.i, %Sqrt5.i.i.i
  %.i051.i.i.i = fmul fast float %.i033.i.i.i, 0x3FB99999A0000000
  %.i152.i.i.i = fmul fast float %.i134.i.i.i, 0x3FB99999A0000000
  %.i253.i.i.i = fmul fast float %.i235.i.i.i, 0x3FB99999A0000000
  %.i054.i.i.i = fsub fast float %.i027.i.i.i, %.i051.i.i.i
  %.i155.i.i.i = fsub fast float %.i128.i.i.i, %.i152.i.i.i
  %.i256.i.i.i = fsub fast float %.i229.i.i.i, %.i253.i.i.i
  %add.i.49.i.i.i = add nsw i32 %sub.i.i.i.i, 16
  %arrayidx.i.51.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.49.i.i.i
  store i32 0, i32* %arrayidx.i.51.i.i.i, align 8
  %add.i.46.i.i.i = add nsw i32 %sub.i.i.i.i, 17
  %arrayidx.i.48.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.46.i.i.i
  %v64.i.i.i = bitcast i32* %arrayidx.i.48.i.i.i to float*
  store float 0x47EFFFFFE0000000, float* %v64.i.i.i, align 4
  %new.payload.offset.i.i.i = add i32 %sub.i.i.i.i, 16
  %arrayidx.i.45.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %sub.i.i.i.i
  store i32 1001, i32* %arrayidx.i.45.i.i.i
  %add.i.41.i.i.i = add nsw i32 %sub.i.i.i.i, 1
  %arrayidx.i.43.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.41.i.i.i
  store i32 16, i32* %arrayidx.i.43.i.i.i
  %add.i.38.i.i.i = add nsw i32 %sub.i.i.i.i, 2
  %arrayidx.i.40.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.38.i.i.i
  store i32 -1, i32* %arrayidx.i.40.i.i.i
  %add.i.35.i.i.i = add nsw i32 %sub.i.i.i.i, 3
  %arrayidx.i.37.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.35.i.i.i
  store i32 0, i32* %arrayidx.i.37.i.i.i
  %add.i.32.i.i.i = add nsw i32 %sub.i.i.i.i, 4
  %arrayidx.i.34.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.32.i.i.i
  store i32 1, i32* %arrayidx.i.34.i.i.i
  %add.i.29.i.i.i = add nsw i32 %sub.i.i.i.i, 5
  %arrayidx.i.31.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.29.i.i.i
  store i32 0, i32* %arrayidx.i.31.i.i.i
  %19 = bitcast float %.i054.i.i.i to i32
  %add.i.26.i.i.i = add nsw i32 %sub.i.i.i.i, 6
  %arrayidx.i.28.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.26.i.i.i
  store i32 %19, i32* %arrayidx.i.28.i.i.i
  %20 = bitcast float %.i155.i.i.i to i32
  %add.i.23.i.i.i = add nsw i32 %sub.i.i.i.i, 7
  %arrayidx.i.25.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.23.i.i.i
  store i32 %20, i32* %arrayidx.i.25.i.i.i
  %21 = bitcast float %.i256.i.i.i to i32
  %add.i.20.i.i.i = add nsw i32 %sub.i.i.i.i, 8
  %arrayidx.i.22.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.20.i.i.i
  store i32 %21, i32* %arrayidx.i.22.i.i.i
  %add.i.17.i.i.i = add nsw i32 %sub.i.i.i.i, 9
  %arrayidx.i.19.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.17.i.i.i
  store i32 0, i32* %arrayidx.i.19.i.i.i
  %22 = bitcast float %.i048.i.i.i to i32
  %add.i.14.i.i.i = add nsw i32 %sub.i.i.i.i, 10
  %arrayidx.i.16.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.14.i.i.i
  store i32 %22, i32* %arrayidx.i.16.i.i.i
  %23 = bitcast float %.i149.i.i.i to i32
  %add.i.11.i.i.i = add nsw i32 %sub.i.i.i.i, 11
  %arrayidx.i.13.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.11.i.i.i
  store i32 %23, i32* %arrayidx.i.13.i.i.i
  %24 = bitcast float %.i250.i.i.i to i32
  %add.i.8.i.i.i = add nsw i32 %sub.i.i.i.i, 12
  %arrayidx.i.10.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.8.i.i.i
  store i32 %24, i32* %arrayidx.i.10.i.i.i
  %add.i.5.i.i.i = add nsw i32 %sub.i.i.i.i, 13
  %arrayidx.i.7.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.5.i.i.i
  store i32 2139095039, i32* %arrayidx.i.7.i.i.i
  %add.i.2.i.i.i = add nsw i32 %sub.i.i.i.i, 14
  %arrayidx.i.4.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.2.i.i.i
  store i32 %new.payload.offset.i.i.i, i32* %arrayidx.i.4.i.i.i
  br label %while.cond.i

; <label>:25                                      ; preds = %state_1000.RayGen.ss_0.i.i
  %arrayidx.i.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %6
  %ret.stateId.i.i.i = load i32, i32* %arrayidx.i.i.i.i
  br label %while.cond.i

state_1001.RayGen.ss_1.i.i:                       ; preds = %while.body.i
  %add.i.i.9.i.i = add nsw i32 %6, 20
  %arrayidx.i.i.12.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.i.9.i.i
  %ret.stateId.i.13.i.i = load i32, i32* %arrayidx.i.i.12.i.i
  br label %while.cond.i

state_1002.Hit.ss_0.i.i:                          ; preds = %while.body.i
  %sub.i.i.17.i.i = sub nsw i32 %6, 32
  %ConstantBuffer.i.i.i = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %Constants_cbuffer, i32 0) #2
  %ShaderTableStride.i.i.i = extractvalue %dx.types.CBufRet.i32 %ConstantBuffer.i.i.i, 2
  %baseShaderRecordOffset.i.i.i = mul i32 %10, %ShaderTableStride.i.i.i
  %add.i.114.i.i.i = add nsw i32 %9, 1
  %arrayidx.i.116.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.114.i.i.i
  %v17.i.33.i.i = bitcast i32* %arrayidx.i.116.i.i.i to float*
  store float %18, float* %v17.i.33.i.i, align 4, !tbaa !91
  %arrayidx.i.113.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %9
  %v19.i.34.i.i = load i32, i32* %arrayidx.i.113.i.i.i, align 1, !tbaa !95, !range !97
  %v20.i.35.i.i = icmp eq i32 %v19.i.34.i.i, 0
  br i1 %v20.i.35.i.i, label %"\01?Load3x16BitIndices@@YA?AV?$vector@I$02@@I@Z.exit.i.i.i", label %Hit.BB0._crit_edge.i.i.i

Hit.BB0._crit_edge.i.i.i:                         ; preds = %state_1002.Hit.ss_0.i.i, %._crit_edge.i.i.i
  %arrayidx.i.109.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %6
  %ret.stateId.i.37.i.i = load i32, i32* %arrayidx.i.109.i.i.i
  br label %while.cond.i

"\01?Load3x16BitIndices@@YA?AV?$vector@I$02@@I@Z.exit.i.i.i": ; preds = %state_1002.Hit.ss_0.i.i
  %v21.i.38.i.i = add i32 8, %baseShaderRecordOffset.i.i.i
  %ShaderRecordBuffer.i.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %HitGroupShaderTable_texture_rawbuf, i32 %v21.i.38.i.i, i32 undef) #2
  %v22.i.39.i.i = extractvalue %dx.types.ResRet.i32 %ShaderRecordBuffer.i.i.i, 0
  %v24.i.41.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %g_meshInfo_texture_structbuf, i32 %v22.i.39.i.i, i32 0) #2
  %v25.i.42.i.i = extractvalue %dx.types.ResRet.i32 %v24.i.41.i.i, 0
  %v26.i.43.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %g_meshInfo_texture_structbuf, i32 %v22.i.39.i.i, i32 4) #2
  %v27.i.44.i.i = extractvalue %dx.types.ResRet.i32 %v26.i.43.i.i, 0
  %v28.i.45.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %g_meshInfo_texture_structbuf, i32 %v22.i.39.i.i, i32 8) #2
  %v29.i.46.i.i = extractvalue %dx.types.ResRet.i32 %v28.i.45.i.i, 0
  %v30.i.47.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %g_meshInfo_texture_structbuf, i32 %v22.i.39.i.i, i32 12) #2
  %v31.i.48.i.i = extractvalue %dx.types.ResRet.i32 %v30.i.47.i.i, 0
  %v32.i.49.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %g_meshInfo_texture_structbuf, i32 %v22.i.39.i.i, i32 16) #2
  %v33.i.50.i.i = extractvalue %dx.types.ResRet.i32 %v32.i.49.i.i, 0
  %v34.i.51.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %g_meshInfo_texture_structbuf, i32 %v22.i.39.i.i, i32 20) #2
  %v35.i.52.i.i = extractvalue %dx.types.ResRet.i32 %v34.i.51.i.i, 0
  %v36.i.53.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %g_meshInfo_texture_structbuf, i32 %v22.i.39.i.i, i32 24) #2
  %v37.i.54.i.i = extractvalue %dx.types.ResRet.i32 %v36.i.53.i.i, 0
  %v38.i.55.i.i = mul i32 %11, 6
  %v39.i.56.i.i = add i32 %v38.i.55.i.i, %v25.i.42.i.i
  %v40.i.57.i.i = and i32 %v39.i.56.i.i, -4
  %v42.i.59.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %g_indices_texture_rawbuf, i32 %v40.i.57.i.i, i32 undef) #2
  %v43.i.60.i.i = extractvalue %dx.types.ResRet.i32 %v42.i.59.i.i, 0
  %v44.i.61.i.i = extractvalue %dx.types.ResRet.i32 %v42.i.59.i.i, 1
  %v45.i.62.i.i = icmp eq i32 %v40.i.57.i.i, %v39.i.56.i.i
  %v46.i.63.i.i = and i32 %v43.i.60.i.i, 65535
  %v47.i.64.i.i = lshr i32 %v43.i.60.i.i, 16
  %v48.i.65.i.i = lshr i32 %v43.i.60.i.i, 16
  %v49.i.66.i.i = lshr i32 %v44.i.61.i.i, 16
  %.sink.i.i.i = select i1 %v45.i.62.i.i, i32 %v44.i.61.i.i, i32 %v49.i.66.i.i
  %.0.i0.i.i.i = select i1 %v45.i.62.i.i, i32 %v46.i.63.i.i, i32 %v48.i.65.i.i
  %.0.i1.in.i.i.i = select i1 %v45.i.62.i.i, i32 %v47.i.64.i.i, i32 %v44.i.61.i.i
  %.0.i1.i.i.i = and i32 %.0.i1.in.i.i.i, 65535
  %v50.i.67.i.i = and i32 %.sink.i.i.i, 65535
  %v51.i.68.i.i = mul i32 %.0.i0.i.i.i, %v35.i.52.i.i
  %v52.i.69.i.i = add i32 %v51.i.68.i.i, %v27.i.44.i.i
  %v54.i.71.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %g_attributes_texture_rawbuf, i32 %v52.i.69.i.i, i32 undef) #2
  %v55.i.72.i.i = extractvalue %dx.types.ResRet.i32 %v54.i.71.i.i, 0
  %v56.i.73.i.i = extractvalue %dx.types.ResRet.i32 %v54.i.71.i.i, 1
  %.i0.i.74.i.i = bitcast i32 %v55.i.72.i.i to float
  %.i1.i.75.i.i = bitcast i32 %v56.i.73.i.i to float
  %v57.i.76.i.i = mul i32 %.0.i1.i.i.i, %v35.i.52.i.i
  %v58.i.77.i.i = add i32 %v57.i.76.i.i, %v27.i.44.i.i
  %v59.i.78.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %g_attributes_texture_rawbuf, i32 %v58.i.77.i.i, i32 undef) #2
  %v60.i.79.i.i = extractvalue %dx.types.ResRet.i32 %v59.i.78.i.i, 0
  %v61.i.80.i.i = extractvalue %dx.types.ResRet.i32 %v59.i.78.i.i, 1
  %.i0116.i.i.i = bitcast i32 %v60.i.79.i.i to float
  %.i1117.i.i.i = bitcast i32 %v61.i.80.i.i to float
  %v62.i.81.i.i = mul i32 %v50.i.67.i.i, %v35.i.52.i.i
  %v63.i.i.i = add i32 %v62.i.81.i.i, %v27.i.44.i.i
  %v64.i.82.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %g_attributes_texture_rawbuf, i32 %v63.i.i.i, i32 undef) #2
  %v65.i.83.i.i = extractvalue %dx.types.ResRet.i32 %v64.i.82.i.i, 0
  %v66.i.i.i = extractvalue %dx.types.ResRet.i32 %v64.i.82.i.i, 1
  %.i0118.i.i.i = bitcast i32 %v65.i.83.i.i to float
  %.i1119.i.i.i = bitcast i32 %v66.i.i.i to float
  %arrayidx.i.107.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %8
  %v67.v.i.i.i = bitcast i32* %arrayidx.i.107.i.i.i to float*
  %add.i.103.i.i.i = add nsw i32 %8, 1
  %arrayidx.i.105.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.103.i.i.i
  %v67.v34.i.i.i = bitcast i32* %arrayidx.i.105.i.i.i to float*
  %el.i.i.i = load float, float* %v67.v.i.i.i
  %el35.i.i.i = load float, float* %v67.v34.i.i.i
  %v70.i.i.i = fsub fast float 1.000000e+00, %el.i.i.i
  %v72.i.i.i = fsub fast float %v70.i.i.i, %el35.i.i.i
  %.i0120.i.i.i = fmul fast float %v72.i.i.i, %.i0.i.74.i.i
  %.i1121.i.i.i = fmul fast float %v72.i.i.i, %.i1.i.75.i.i
  %.i0122.i.i.i = fmul fast float %el.i.i.i, %.i0116.i.i.i
  %.i1123.i.i.i = fmul fast float %el.i.i.i, %.i1117.i.i.i
  %.i0126.i.i.i = fmul fast float %el35.i.i.i, %.i0118.i.i.i
  %.i1127.i.i.i = fmul fast float %el35.i.i.i, %.i1119.i.i.i
  %.i0124.i.i.i = fadd fast float %.i0126.i.i.i, %.i0122.i.i.i
  %.i0128.i.i.i = fadd fast float %.i0124.i.i.i, %.i0120.i.i.i
  %.i1125.i.i.i = fadd fast float %.i1127.i.i.i, %.i1123.i.i.i
  %.i1129.i.i.i = fadd fast float %.i1125.i.i.i, %.i1121.i.i.i
  %v73.i.i.i = add i32 %v51.i.68.i.i, %v29.i.46.i.i
  %v75.i.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %g_attributes_texture_rawbuf, i32 %v73.i.i.i, i32 undef) #2
  %v76.i.i.i = extractvalue %dx.types.ResRet.i32 %v75.i.i.i, 0
  %v77.i.i.i = extractvalue %dx.types.ResRet.i32 %v75.i.i.i, 1
  %v78.i.i.i = extractvalue %dx.types.ResRet.i32 %v75.i.i.i, 2
  %.i0130.i.i.i = bitcast i32 %v76.i.i.i to float
  %.i1131.i.i.i = bitcast i32 %v77.i.i.i to float
  %.i2.i.84.i.i = bitcast i32 %v78.i.i.i to float
  %v79.i.i.i = add i32 %v57.i.76.i.i, %v29.i.46.i.i
  %v80.i.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %g_attributes_texture_rawbuf, i32 %v79.i.i.i, i32 undef) #2
  %v81.i.i.i = extractvalue %dx.types.ResRet.i32 %v80.i.i.i, 0
  %v82.i.i.i = extractvalue %dx.types.ResRet.i32 %v80.i.i.i, 1
  %v83.i.i.i = extractvalue %dx.types.ResRet.i32 %v80.i.i.i, 2
  %.i0132.i.i.i = bitcast i32 %v81.i.i.i to float
  %.i1133.i.i.i = bitcast i32 %v82.i.i.i to float
  %.i2134.i.i.i = bitcast i32 %v83.i.i.i to float
  %v84.i.i.i = add i32 %v62.i.81.i.i, %v29.i.46.i.i
  %v85.i.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %g_attributes_texture_rawbuf, i32 %v84.i.i.i, i32 undef) #2
  %v86.i.i.i = extractvalue %dx.types.ResRet.i32 %v85.i.i.i, 0
  %v87.i.i.i = extractvalue %dx.types.ResRet.i32 %v85.i.i.i, 1
  %v88.i.i.i = extractvalue %dx.types.ResRet.i32 %v85.i.i.i, 2
  %.i0135.i.i.i = bitcast i32 %v86.i.i.i to float
  %.i1136.i.i.i = bitcast i32 %v87.i.i.i to float
  %.i2137.i.i.i = bitcast i32 %v88.i.i.i to float
  %.i0138.i.i.i = fmul fast float %.i0130.i.i.i, %v72.i.i.i
  %.i1139.i.i.i = fmul fast float %.i1131.i.i.i, %v72.i.i.i
  %.i2140.i.i.i = fmul fast float %.i2.i.84.i.i, %v72.i.i.i
  %.i0141.i.i.i = fmul fast float %.i0132.i.i.i, %el.i.i.i
  %.i1142.i.i.i = fmul fast float %.i1133.i.i.i, %el.i.i.i
  %.i2143.i.i.i = fmul fast float %.i2134.i.i.i, %el.i.i.i
  %.i0144.i.i.i = fadd fast float %.i0141.i.i.i, %.i0138.i.i.i
  %.i1145.i.i.i = fadd fast float %.i1142.i.i.i, %.i1139.i.i.i
  %.i2146.i.i.i = fadd fast float %.i2143.i.i.i, %.i2140.i.i.i
  %.i0147.i.i.i = fmul fast float %.i0135.i.i.i, %el35.i.i.i
  %.i1148.i.i.i = fmul fast float %.i1136.i.i.i, %el35.i.i.i
  %.i2149.i.i.i = fmul fast float %.i2137.i.i.i, %el35.i.i.i
  %.i0150.i.i.i = fadd fast float %.i0144.i.i.i, %.i0147.i.i.i
  %.i1151.i.i.i = fadd fast float %.i1145.i.i.i, %.i1148.i.i.i
  %.i2152.i.i.i = fadd fast float %.i2146.i.i.i, %.i2149.i.i.i
  %v89.i.i.i = fmul fast float %.i0150.i.i.i, %.i0150.i.i.i
  %v90.i.i.i = fmul fast float %.i1151.i.i.i, %.i1151.i.i.i
  %v91.i.i.i = fadd fast float %v89.i.i.i, %v90.i.i.i
  %v92.i.i.i = fmul fast float %.i2152.i.i.i, %.i2152.i.i.i
  %v93.i.i.i = fadd fast float %v91.i.i.i, %v92.i.i.i
  %Sqrt72.i.i.i = call float @dx.op.unary.f32(i32 24, float %v93.i.i.i) #2
  %.i0153.i.i.i = fdiv fast float %.i0150.i.i.i, %Sqrt72.i.i.i
  %.i1154.i.i.i = fdiv fast float %.i1151.i.i.i, %Sqrt72.i.i.i
  %.i2155.i.i.i = fdiv fast float %.i2152.i.i.i, %Sqrt72.i.i.i
  %v94.i.i.i = add i32 %v51.i.68.i.i, %v31.i.48.i.i
  %v95.i.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %g_attributes_texture_rawbuf, i32 %v94.i.i.i, i32 undef) #2
  %v96.i.i.i = extractvalue %dx.types.ResRet.i32 %v95.i.i.i, 0
  %v97.i.i.i = extractvalue %dx.types.ResRet.i32 %v95.i.i.i, 1
  %v98.i.i.i = extractvalue %dx.types.ResRet.i32 %v95.i.i.i, 2
  %.i0156.i.i.i = bitcast i32 %v96.i.i.i to float
  %.i1157.i.i.i = bitcast i32 %v97.i.i.i to float
  %.i2158.i.i.i = bitcast i32 %v98.i.i.i to float
  %v99.i.i.i = add i32 %v57.i.76.i.i, %v31.i.48.i.i
  %v100.i.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %g_attributes_texture_rawbuf, i32 %v99.i.i.i, i32 undef) #2
  %v101.i.i.i = extractvalue %dx.types.ResRet.i32 %v100.i.i.i, 0
  %v102.i.i.i = extractvalue %dx.types.ResRet.i32 %v100.i.i.i, 1
  %v103.i.i.i = extractvalue %dx.types.ResRet.i32 %v100.i.i.i, 2
  %.i0159.i.i.i = bitcast i32 %v101.i.i.i to float
  %.i1160.i.i.i = bitcast i32 %v102.i.i.i to float
  %.i2161.i.i.i = bitcast i32 %v103.i.i.i to float
  %v104.i.i.i = add i32 %v62.i.81.i.i, %v31.i.48.i.i
  %v105.i.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %g_attributes_texture_rawbuf, i32 %v104.i.i.i, i32 undef) #2
  %v106.i.i.i = extractvalue %dx.types.ResRet.i32 %v105.i.i.i, 0
  %v107.i.i.i = extractvalue %dx.types.ResRet.i32 %v105.i.i.i, 1
  %v108.i.i.i = extractvalue %dx.types.ResRet.i32 %v105.i.i.i, 2
  %.i0162.i.i.i = bitcast i32 %v106.i.i.i to float
  %.i1163.i.i.i = bitcast i32 %v107.i.i.i to float
  %.i2164.i.i.i = bitcast i32 %v108.i.i.i to float
  %.i0165.i.i.i = fmul fast float %.i0156.i.i.i, %v72.i.i.i
  %.i1166.i.i.i = fmul fast float %.i1157.i.i.i, %v72.i.i.i
  %.i2167.i.i.i = fmul fast float %.i2158.i.i.i, %v72.i.i.i
  %.i0168.i.i.i = fmul fast float %.i0159.i.i.i, %el.i.i.i
  %.i1169.i.i.i = fmul fast float %.i1160.i.i.i, %el.i.i.i
  %.i2170.i.i.i = fmul fast float %.i2161.i.i.i, %el.i.i.i
  %.i0171.i.i.i = fadd fast float %.i0168.i.i.i, %.i0165.i.i.i
  %.i1172.i.i.i = fadd fast float %.i1169.i.i.i, %.i1166.i.i.i
  %.i2173.i.i.i = fadd fast float %.i2170.i.i.i, %.i2167.i.i.i
  %.i0174.i.i.i = fmul fast float %.i0162.i.i.i, %el35.i.i.i
  %.i1175.i.i.i = fmul fast float %.i1163.i.i.i, %el35.i.i.i
  %.i2176.i.i.i = fmul fast float %.i2164.i.i.i, %el35.i.i.i
  %.i0177.i.i.i = fadd fast float %.i0171.i.i.i, %.i0174.i.i.i
  %.i1178.i.i.i = fadd fast float %.i1172.i.i.i, %.i1175.i.i.i
  %.i2179.i.i.i = fadd fast float %.i2173.i.i.i, %.i2176.i.i.i
  %v109.i.i.i = fmul fast float %.i0177.i.i.i, %.i0177.i.i.i
  %v110.i.i.i = fmul fast float %.i1178.i.i.i, %.i1178.i.i.i
  %v111.i.i.i = fadd fast float %v109.i.i.i, %v110.i.i.i
  %v112.i.i.i = fmul fast float %.i2179.i.i.i, %.i2179.i.i.i
  %v113.i.i.i = fadd fast float %v111.i.i.i, %v112.i.i.i
  %Sqrt73.i.i.i = call float @dx.op.unary.f32(i32 24, float %v113.i.i.i) #2
  %.i0180.i.i.i = fdiv fast float %.i0177.i.i.i, %Sqrt73.i.i.i
  %.i1181.i.i.i = fdiv fast float %.i1178.i.i.i, %Sqrt73.i.i.i
  %.i2182.i.i.i = fdiv fast float %.i2179.i.i.i, %Sqrt73.i.i.i
  %v114.i.i.i = add i32 %v51.i.68.i.i, %v33.i.50.i.i
  %v115.i.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %g_attributes_texture_rawbuf, i32 %v114.i.i.i, i32 undef) #2
  %v116.i.i.i = extractvalue %dx.types.ResRet.i32 %v115.i.i.i, 0
  %v117.i.i.i = extractvalue %dx.types.ResRet.i32 %v115.i.i.i, 1
  %v118.i.i.i = extractvalue %dx.types.ResRet.i32 %v115.i.i.i, 2
  %.i0183.i.i.i = bitcast i32 %v116.i.i.i to float
  %.i1184.i.i.i = bitcast i32 %v117.i.i.i to float
  %.i2185.i.i.i = bitcast i32 %v118.i.i.i to float
  %v119.i.i.i = add i32 %v57.i.76.i.i, %v33.i.50.i.i
  %v121.i.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %g_attributes_texture_rawbuf, i32 %v119.i.i.i, i32 undef) #2
  %v122.i.i.i = extractvalue %dx.types.ResRet.i32 %v121.i.i.i, 0
  %v123.i.i.i = extractvalue %dx.types.ResRet.i32 %v121.i.i.i, 1
  %v124.i.i.i = extractvalue %dx.types.ResRet.i32 %v121.i.i.i, 2
  %.i0186.i.i.i = bitcast i32 %v122.i.i.i to float
  %.i1187.i.i.i = bitcast i32 %v123.i.i.i to float
  %.i2188.i.i.i = bitcast i32 %v124.i.i.i to float
  %v125.i.i.i = add i32 %v62.i.81.i.i, %v33.i.50.i.i
  %v126.i.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %g_attributes_texture_rawbuf, i32 %v125.i.i.i, i32 undef) #2
  %v127.i.i.i = extractvalue %dx.types.ResRet.i32 %v126.i.i.i, 0
  %v128.i.i.i = extractvalue %dx.types.ResRet.i32 %v126.i.i.i, 1
  %v129.i.i.i = extractvalue %dx.types.ResRet.i32 %v126.i.i.i, 2
  %.i0189.i.i.i = bitcast i32 %v127.i.i.i to float
  %.i1190.i.i.i = bitcast i32 %v128.i.i.i to float
  %.i2191.i.i.i = bitcast i32 %v129.i.i.i to float
  %.i0192.i.i.i = fmul fast float %18, %14
  %.i1193.i.i.i = fmul fast float %18, %13
  %.i2194.i.i.i = fmul fast float %18, %12
  %.i0195.i.i.i = fadd fast float %.i0192.i.i.i, %17
  %.i1196.i.i.i = fadd fast float %.i1193.i.i.i, %16
  %.i2197.i.i.i = fadd fast float %.i2194.i.i.i, %15
  %v130.i.i.i = add i32 %DTidx.i, 1
  %.i0198.i.i.i = uitofp i32 %v130.i.i.i to float
  %.i1199.i.i.i = uitofp i32 %DTidy.i to float
  %.i0200.i.i.i = fadd fast float %.i0198.i.i.i, 5.000000e-01
  %.i1201.i.i.i = fadd fast float %.i1199.i.i.i, 5.000000e-01
  %v131.i.i.i = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %b1_cbuffer, i32 5) #2
  %v132.i.i.i = extractvalue %dx.types.CBufRet.f32 %v131.i.i.i, 0
  %v133.i.i.i = extractvalue %dx.types.CBufRet.f32 %v131.i.i.i, 1
  %.i0202.i.i.i = fdiv fast float %.i0200.i.i.i, %v132.i.i.i
  %.i1203.i.i.i = fdiv fast float %.i1201.i.i.i, %v133.i.i.i
  %.i0204.i.i.i = fmul fast float %.i0202.i.i.i, 2.000000e+00
  %.i1205.i.i.i = fmul fast float %.i1203.i.i.i, 2.000000e+00
  %.i0206.i.i.i = fadd fast float %.i0204.i.i.i, -1.000000e+00
  %.i1207.i.i.i = fadd fast float %.i1205.i.i.i, -1.000000e+00
  %v134.i.i.i = fsub fast float -0.000000e+00, %.i1207.i.i.i
  %v135.i.i.i = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %b1_cbuffer, i32 0) #2
  %v136.i.i.i = extractvalue %dx.types.CBufRet.f32 %v135.i.i.i, 0
  %v137.i.i.i = extractvalue %dx.types.CBufRet.f32 %v135.i.i.i, 1
  %v138.i.i.i = extractvalue %dx.types.CBufRet.f32 %v135.i.i.i, 2
  %v139.i.i.i = extractvalue %dx.types.CBufRet.f32 %v135.i.i.i, 3
  %v140.i.i.i = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %b1_cbuffer, i32 1) #2
  %v141.i.i.i = extractvalue %dx.types.CBufRet.f32 %v140.i.i.i, 0
  %v142.i.i.i = extractvalue %dx.types.CBufRet.f32 %v140.i.i.i, 1
  %v143.i.i.i = extractvalue %dx.types.CBufRet.f32 %v140.i.i.i, 2
  %v144.i.i.i = extractvalue %dx.types.CBufRet.f32 %v140.i.i.i, 3
  %v145.i.i.i = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %b1_cbuffer, i32 2) #2
  %v146.i.i.i = extractvalue %dx.types.CBufRet.f32 %v145.i.i.i, 0
  %v147.i.i.i = extractvalue %dx.types.CBufRet.f32 %v145.i.i.i, 1
  %v148.i.i.i = extractvalue %dx.types.CBufRet.f32 %v145.i.i.i, 2
  %v149.i.i.i = extractvalue %dx.types.CBufRet.f32 %v145.i.i.i, 3
  %v150.i.i.i = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %b1_cbuffer, i32 3) #2
  %v151.i.i.i = extractvalue %dx.types.CBufRet.f32 %v150.i.i.i, 0
  %v152.i.i.i = extractvalue %dx.types.CBufRet.f32 %v150.i.i.i, 1
  %v153.i.i.i = extractvalue %dx.types.CBufRet.f32 %v150.i.i.i, 2
  %v154.i.i.i = extractvalue %dx.types.CBufRet.f32 %v150.i.i.i, 3
  %v155.i.i.i = fmul fast float %v136.i.i.i, %.i0206.i.i.i
  %FMad33.i.i.i = call float @dx.op.tertiary.f32(i32 46, float %v134.i.i.i, float %v137.i.i.i, float %v155.i.i.i) #2
  %FMad32.i.i.i = call float @dx.op.tertiary.f32(i32 46, float 0.000000e+00, float %v138.i.i.i, float %FMad33.i.i.i) #2
  %FMad31.i.i.i = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %v139.i.i.i, float %FMad32.i.i.i) #2
  %v156.i.i.i = fmul fast float %v141.i.i.i, %.i0206.i.i.i
  %FMad30.i.i.i = call float @dx.op.tertiary.f32(i32 46, float %v134.i.i.i, float %v142.i.i.i, float %v156.i.i.i) #2
  %FMad29.i.i.i = call float @dx.op.tertiary.f32(i32 46, float 0.000000e+00, float %v143.i.i.i, float %FMad30.i.i.i) #2
  %FMad28.i.i.i = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %v144.i.i.i, float %FMad29.i.i.i) #2
  %v157.i.i.i = fmul fast float %v146.i.i.i, %.i0206.i.i.i
  %FMad27.i.i.i = call float @dx.op.tertiary.f32(i32 46, float %v134.i.i.i, float %v147.i.i.i, float %v157.i.i.i) #2
  %FMad26.i.i.i = call float @dx.op.tertiary.f32(i32 46, float 0.000000e+00, float %v148.i.i.i, float %FMad27.i.i.i) #2
  %FMad25.i.i.i = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %v149.i.i.i, float %FMad26.i.i.i) #2
  %v158.i.i.i = fmul fast float %v151.i.i.i, %.i0206.i.i.i
  %FMad24.i.i.i = call float @dx.op.tertiary.f32(i32 46, float %v134.i.i.i, float %v152.i.i.i, float %v158.i.i.i) #2
  %FMad23.i.i.i = call float @dx.op.tertiary.f32(i32 46, float 0.000000e+00, float %v153.i.i.i, float %FMad24.i.i.i) #2
  %FMad22.i.i.i = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %v154.i.i.i, float %FMad23.i.i.i) #2
  %.i0208.i.i.i = fdiv fast float %FMad31.i.i.i, %FMad22.i.i.i
  %.i1209.i.i.i = fdiv fast float %FMad28.i.i.i, %FMad22.i.i.i
  %.i2210.i.i.i = fdiv fast float %FMad25.i.i.i, %FMad22.i.i.i
  %v159.i.i.i = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %b1_cbuffer, i32 4) #2
  %v160.i.i.i = extractvalue %dx.types.CBufRet.f32 %v159.i.i.i, 0
  %v161.i.i.i = extractvalue %dx.types.CBufRet.f32 %v159.i.i.i, 1
  %v162.i.i.i = extractvalue %dx.types.CBufRet.f32 %v159.i.i.i, 2
  %.i0211.i.i.i = fsub fast float %.i0208.i.i.i, %v160.i.i.i
  %.i1212.i.i.i = fsub fast float %.i1209.i.i.i, %v161.i.i.i
  %.i2213.i.i.i = fsub fast float %.i2210.i.i.i, %v162.i.i.i
  %v163.i.i.i = fmul fast float %.i0211.i.i.i, %.i0211.i.i.i
  %v164.i.i.i = fmul fast float %.i1212.i.i.i, %.i1212.i.i.i
  %v165.i.i.i = fadd fast float %v163.i.i.i, %v164.i.i.i
  %v166.i.i.i = fmul fast float %.i2213.i.i.i, %.i2213.i.i.i
  %v167.i.i.i = fadd fast float %v165.i.i.i, %v166.i.i.i
  %Sqrt71.i.i.i = call float @dx.op.unary.f32(i32 24, float %v167.i.i.i) #2
  %.i0214.i.i.i = fdiv fast float %.i0211.i.i.i, %Sqrt71.i.i.i
  %.i1215.i.i.i = fdiv fast float %.i1212.i.i.i, %Sqrt71.i.i.i
  %.i2216.i.i.i = fdiv fast float %.i2213.i.i.i, %Sqrt71.i.i.i
  %v168.i.i.i = add i32 %DTidy.i, 1
  %.i0217.i.i.i = uitofp i32 %DTidx.i to float
  %.i1218.i.i.i = uitofp i32 %v168.i.i.i to float
  %.i0219.i.i.i = fadd fast float %.i0217.i.i.i, 5.000000e-01
  %.i1220.i.i.i = fadd fast float %.i1218.i.i.i, 5.000000e-01
  %.i0221.i.i.i = fdiv fast float %.i0219.i.i.i, %v132.i.i.i
  %.i1222.i.i.i = fdiv fast float %.i1220.i.i.i, %v133.i.i.i
  %.i0223.i.i.i = fmul fast float %.i0221.i.i.i, 2.000000e+00
  %.i1224.i.i.i = fmul fast float %.i1222.i.i.i, 2.000000e+00
  %.i0225.i.i.i = fadd fast float %.i0223.i.i.i, -1.000000e+00
  %.i1226.i.i.i = fadd fast float %.i1224.i.i.i, -1.000000e+00
  %v169.i.i.i = fsub fast float -0.000000e+00, %.i1226.i.i.i
  %v170.i.i.i = fmul fast float %v136.i.i.i, %.i0225.i.i.i
  %FMad65.i.i.i = call float @dx.op.tertiary.f32(i32 46, float %v169.i.i.i, float %v137.i.i.i, float %v170.i.i.i) #2
  %FMad64.i.i.i = call float @dx.op.tertiary.f32(i32 46, float 0.000000e+00, float %v138.i.i.i, float %FMad65.i.i.i) #2
  %FMad63.i.i.i = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %v139.i.i.i, float %FMad64.i.i.i) #2
  %v171.i.i.i = fmul fast float %v141.i.i.i, %.i0225.i.i.i
  %FMad62.i.i.i = call float @dx.op.tertiary.f32(i32 46, float %v169.i.i.i, float %v142.i.i.i, float %v171.i.i.i) #2
  %FMad61.i.i.i = call float @dx.op.tertiary.f32(i32 46, float 0.000000e+00, float %v143.i.i.i, float %FMad62.i.i.i) #2
  %FMad60.i.i.i = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %v144.i.i.i, float %FMad61.i.i.i) #2
  %v172.i.i.i = fmul fast float %v146.i.i.i, %.i0225.i.i.i
  %FMad59.i.i.i = call float @dx.op.tertiary.f32(i32 46, float %v169.i.i.i, float %v147.i.i.i, float %v172.i.i.i) #2
  %FMad58.i.i.i = call float @dx.op.tertiary.f32(i32 46, float 0.000000e+00, float %v148.i.i.i, float %FMad59.i.i.i) #2
  %FMad57.i.i.i = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %v149.i.i.i, float %FMad58.i.i.i) #2
  %v173.i.i.i = fmul fast float %v151.i.i.i, %.i0225.i.i.i
  %FMad56.i.i.i = call float @dx.op.tertiary.f32(i32 46, float %v169.i.i.i, float %v152.i.i.i, float %v173.i.i.i) #2
  %FMad55.i.i.i = call float @dx.op.tertiary.f32(i32 46, float 0.000000e+00, float %v153.i.i.i, float %FMad56.i.i.i) #2
  %FMad54.i.i.i = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %v154.i.i.i, float %FMad55.i.i.i) #2
  %.i0227.i.i.i = fdiv fast float %FMad63.i.i.i, %FMad54.i.i.i
  %.i1228.i.i.i = fdiv fast float %FMad60.i.i.i, %FMad54.i.i.i
  %.i2229.i.i.i = fdiv fast float %FMad57.i.i.i, %FMad54.i.i.i
  %.i0230.i.i.i = fsub fast float %.i0227.i.i.i, %v160.i.i.i
  %.i1231.i.i.i = fsub fast float %.i1228.i.i.i, %v161.i.i.i
  %.i2232.i.i.i = fsub fast float %.i2229.i.i.i, %v162.i.i.i
  %v174.i.i.i = fmul fast float %.i0230.i.i.i, %.i0230.i.i.i
  %v175.i.i.i = fmul fast float %.i1231.i.i.i, %.i1231.i.i.i
  %v176.i.i.i = fadd fast float %v174.i.i.i, %v175.i.i.i
  %v177.i.i.i = fmul fast float %.i2232.i.i.i, %.i2232.i.i.i
  %v178.i.i.i = fadd fast float %v176.i.i.i, %v177.i.i.i
  %Sqrt70.i.i.i = call float @dx.op.unary.f32(i32 24, float %v178.i.i.i) #2
  %.i0233.i.i.i = fdiv fast float %.i0230.i.i.i, %Sqrt70.i.i.i
  %.i1234.i.i.i = fdiv fast float %.i1231.i.i.i, %Sqrt70.i.i.i
  %.i2235.i.i.i = fdiv fast float %.i2232.i.i.i, %Sqrt70.i.i.i
  %.i0236.i.i.i = fsub fast float %.i0186.i.i.i, %.i0183.i.i.i
  %.i1237.i.i.i = fsub fast float %.i1187.i.i.i, %.i1184.i.i.i
  %.i2238.i.i.i = fsub fast float %.i2188.i.i.i, %.i2185.i.i.i
  %.i0239.i.i.i = fsub fast float %.i0189.i.i.i, %.i0183.i.i.i
  %.i1240.i.i.i = fsub fast float %.i1190.i.i.i, %.i1184.i.i.i
  %.i2241.i.i.i = fsub fast float %.i2191.i.i.i, %.i2185.i.i.i
  %v179.i.i.i = fmul fast float %.i1240.i.i.i, %.i2238.i.i.i
  %v180.i.i.i = fmul fast float %.i2241.i.i.i, %.i1237.i.i.i
  %v181.i.i.i = fsub fast float %v179.i.i.i, %v180.i.i.i
  %v182.i.i.i = fmul fast float %.i2241.i.i.i, %.i0236.i.i.i
  %v183.i.i.i = fmul fast float %.i0239.i.i.i, %.i2238.i.i.i
  %v184.i.i.i = fsub fast float %v182.i.i.i, %v183.i.i.i
  %v185.i.i.i = fmul fast float %.i0239.i.i.i, %.i1237.i.i.i
  %v186.i.i.i = fmul fast float %.i1240.i.i.i, %.i0236.i.i.i
  %v187.i.i.i = fsub fast float %v185.i.i.i, %v186.i.i.i
  %v188.i.i.i = fmul fast float %v181.i.i.i, %v181.i.i.i
  %v189.i.i.i = fmul fast float %v184.i.i.i, %v184.i.i.i
  %v190.i.i.i = fadd fast float %v188.i.i.i, %v189.i.i.i
  %v191.i.i.i = fmul fast float %v187.i.i.i, %v187.i.i.i
  %v192.i.i.i = fadd fast float %v190.i.i.i, %v191.i.i.i
  %Sqrt74.i.i.i = call float @dx.op.unary.f32(i32 24, float %v192.i.i.i) #2
  %.i0242.i.i.i = fdiv fast float %v181.i.i.i, %Sqrt74.i.i.i
  %.i1243.i.i.i = fdiv fast float %v184.i.i.i, %Sqrt74.i.i.i
  %.i2244.i.i.i = fdiv fast float %v187.i.i.i, %Sqrt74.i.i.i
  %.i0245.i.i.i = fsub fast float %v160.i.i.i, %.i0195.i.i.i
  %.i1246.i.i.i = fsub fast float %v161.i.i.i, %.i1196.i.i.i
  %.i2247.i.i.i = fsub fast float %v162.i.i.i, %.i2197.i.i.i
  %.i0248.i.i.i = fsub fast float -0.000000e+00, %.i0242.i.i.i
  %.i1249.i.i.i = fsub fast float -0.000000e+00, %.i1243.i.i.i
  %.i2250.i.i.i = fsub fast float -0.000000e+00, %.i2244.i.i.i
  %v193.i.i.i = call float @dx.op.dot3.f32(i32 55, float %.i0248.i.i.i, float %.i1249.i.i.i, float %.i2250.i.i.i, float %.i0245.i.i.i, float %.i1246.i.i.i, float %.i2247.i.i.i) #2
  %v194.i.i.i = call float @dx.op.dot3.f32(i32 55, float %.i0242.i.i.i, float %.i1243.i.i.i, float %.i2244.i.i.i, float %.i0214.i.i.i, float %.i1215.i.i.i, float %.i2216.i.i.i) #2
  %v195.i.i.i = fdiv fast float %v193.i.i.i, %v194.i.i.i
  %.i0251.i.i.i = fmul fast float %v195.i.i.i, %.i0214.i.i.i
  %.i1252.i.i.i = fmul fast float %v195.i.i.i, %.i1215.i.i.i
  %.i2253.i.i.i = fmul fast float %v195.i.i.i, %.i2216.i.i.i
  %.i0254.i.i.i = fadd fast float %.i0251.i.i.i, %v160.i.i.i
  %.i1255.i.i.i = fadd fast float %.i1252.i.i.i, %v161.i.i.i
  %.i2256.i.i.i = fadd fast float %.i2253.i.i.i, %v162.i.i.i
  %v196.i.i.i = call float @dx.op.dot3.f32(i32 55, float %.i0242.i.i.i, float %.i1243.i.i.i, float %.i2244.i.i.i, float %.i0233.i.i.i, float %.i1234.i.i.i, float %.i2235.i.i.i) #2
  %v197.i.i.i = fdiv fast float %v193.i.i.i, %v196.i.i.i
  %.i0263.i.i.i = fmul fast float %v197.i.i.i, %.i0233.i.i.i
  %.i1264.i.i.i = fmul fast float %v197.i.i.i, %.i1234.i.i.i
  %.i2265.i.i.i = fmul fast float %v197.i.i.i, %.i2235.i.i.i
  %.i0266.i.i.i = fadd fast float %.i0263.i.i.i, %v160.i.i.i
  %.i1267.i.i.i = fadd fast float %.i1264.i.i.i, %v161.i.i.i
  %.i2268.i.i.i = fadd fast float %.i2265.i.i.i, %v162.i.i.i
  %.i0269.i.i.i = fsub fast float %.i0.i.74.i.i, %.i0118.i.i.i
  %.i1270.i.i.i = fsub fast float %.i1.i.75.i.i, %.i1119.i.i.i
  %.i0271.i.i.i = fsub fast float %.i0116.i.i.i, %.i0118.i.i.i
  %.i1272.i.i.i = fsub fast float %.i1117.i.i.i, %.i1119.i.i.i
  %.i0273.i.i.i = fsub fast float %.i0183.i.i.i, %.i0189.i.i.i
  %.i1274.i.i.i = fsub fast float %.i1184.i.i.i, %.i1190.i.i.i
  %.i2275.i.i.i = fsub fast float %.i2185.i.i.i, %.i2191.i.i.i
  %.i0276.i.i.i = fsub fast float %.i0186.i.i.i, %.i0189.i.i.i
  %.i1277.i.i.i = fsub fast float %.i1187.i.i.i, %.i1190.i.i.i
  %.i2278.i.i.i = fsub fast float %.i2188.i.i.i, %.i2191.i.i.i
  %v198.i.i.i = fmul fast float %.i0269.i.i.i, %.i1272.i.i.i
  %v199.i.i.i = fmul fast float %.i0271.i.i.i, %.i1270.i.i.i
  %v200.i.i.i = fsub fast float %v198.i.i.i, %v199.i.i.i
  %v201.i.i.i = fdiv fast float 1.000000e+00, %v200.i.i.i
  %.i0279.i.i.i = fmul fast float %v201.i.i.i, %.i1272.i.i.i
  %v202.i.i.i = fmul fast float %.i0271.i.i.i, %v201.i.i.i
  %.i1280.i.i.i = fsub fast float -0.000000e+00, %v202.i.i.i
  %v203.i.i.i = fmul fast float %.i1270.i.i.i, %v201.i.i.i
  %.i2281.i.i.i = fsub fast float -0.000000e+00, %v203.i.i.i
  %.i3.i.i.i = fmul fast float %v201.i.i.i, %.i0269.i.i.i
  %.i0282.i.i.i = fmul fast float %.i0279.i.i.i, %.i0273.i.i.i
  %.i1283.i.i.i = fmul fast float %.i0279.i.i.i, %.i1274.i.i.i
  %.i2284.i.i.i = fmul fast float %.i0279.i.i.i, %.i2275.i.i.i
  %.i0285.i.i.i = fmul fast float %.i0276.i.i.i, %.i1280.i.i.i
  %.i1286.i.i.i = fmul fast float %.i1277.i.i.i, %.i1280.i.i.i
  %.i2287.i.i.i = fmul fast float %.i2278.i.i.i, %.i1280.i.i.i
  %.i0288.i.i.i = fadd fast float %.i0282.i.i.i, %.i0285.i.i.i
  %.i1289.i.i.i = fadd fast float %.i1283.i.i.i, %.i1286.i.i.i
  %.i2290.i.i.i = fadd fast float %.i2284.i.i.i, %.i2287.i.i.i
  %.i0291.i.i.i = fmul fast float %.i0273.i.i.i, %.i2281.i.i.i
  %.i1292.i.i.i = fmul fast float %.i1274.i.i.i, %.i2281.i.i.i
  %.i2293.i.i.i = fmul fast float %.i2275.i.i.i, %.i2281.i.i.i
  %.i0294.i.i.i = fmul fast float %.i3.i.i.i, %.i0276.i.i.i
  %.i1295.i.i.i = fmul fast float %.i3.i.i.i, %.i1277.i.i.i
  %.i2296.i.i.i = fmul fast float %.i3.i.i.i, %.i2278.i.i.i
  %.i0297.i.i.i = fadd fast float %.i0291.i.i.i, %.i0294.i.i.i
  %.i1298.i.i.i = fadd fast float %.i1292.i.i.i, %.i1295.i.i.i
  %.i2299.i.i.i = fadd fast float %.i2293.i.i.i, %.i2296.i.i.i
  %v204.i.i.i = getelementptr inbounds [3 x float], [3 x float]* %v11.i.28.i.i, i32 0, i32 0
  store float %.i0266.i.i.i, float* %v204.i.i.i, align 4
  %v205.i.i.i = getelementptr inbounds [3 x float], [3 x float]* %v11.i.28.i.i, i32 0, i32 1
  store float %.i1267.i.i.i, float* %v205.i.i.i, align 4
  %v206.i.i.i = getelementptr inbounds [3 x float], [3 x float]* %v11.i.28.i.i, i32 0, i32 2
  store float %.i2268.i.i.i, float* %v206.i.i.i, align 4
  %v207.i.i.i = getelementptr inbounds [3 x float], [3 x float]* %v12.i.29.i.i, i32 0, i32 0
  store float %.i0254.i.i.i, float* %v207.i.i.i, align 4
  %v208.i.i.i = getelementptr inbounds [3 x float], [3 x float]* %v12.i.29.i.i, i32 0, i32 1
  store float %.i1255.i.i.i, float* %v208.i.i.i, align 4
  %v209.i.i.i = getelementptr inbounds [3 x float], [3 x float]* %v12.i.29.i.i, i32 0, i32 2
  store float %.i2256.i.i.i, float* %v209.i.i.i, align 4
  %v210.i.i.i = getelementptr inbounds [3 x float], [3 x float]* %v13.i.30.i.i, i32 0, i32 0
  store float %.i0195.i.i.i, float* %v210.i.i.i, align 4
  %v211.i.i.i = getelementptr inbounds [3 x float], [3 x float]* %v13.i.30.i.i, i32 0, i32 1
  store float %.i1196.i.i.i, float* %v211.i.i.i, align 4
  %v212.i.i.i = getelementptr inbounds [3 x float], [3 x float]* %v13.i.30.i.i, i32 0, i32 2
  store float %.i2197.i.i.i, float* %v212.i.i.i, align 4
  %v213.i.i.i = getelementptr inbounds [3 x float], [3 x float]* %v14.i.31.i.i, i32 0, i32 0
  store float %.i0297.i.i.i, float* %v213.i.i.i, align 4
  %v214.i.i.i = getelementptr inbounds [3 x float], [3 x float]* %v14.i.31.i.i, i32 0, i32 1
  store float %.i1298.i.i.i, float* %v214.i.i.i, align 4
  %v215.i.i.i = getelementptr inbounds [3 x float], [3 x float]* %v14.i.31.i.i, i32 0, i32 2
  store float %.i2299.i.i.i, float* %v215.i.i.i, align 4
  %v216.i.i.i = getelementptr inbounds [3 x float], [3 x float]* %v15.i.32.i.i, i32 0, i32 0
  store float %.i0288.i.i.i, float* %v216.i.i.i, align 4
  %v217.i.i.i = getelementptr inbounds [3 x float], [3 x float]* %v15.i.32.i.i, i32 0, i32 1
  store float %.i1289.i.i.i, float* %v217.i.i.i, align 4
  %v218.i.i.i = getelementptr inbounds [3 x float], [3 x float]* %v15.i.32.i.i, i32 0, i32 2
  store float %.i2290.i.i.i, float* %v218.i.i.i, align 4
  %FAbs.i.i.i = call float @dx.op.unary.f32(i32 6, float %.i0242.i.i.i) #2
  %FAbs68.i.i.i = call float @dx.op.unary.f32(i32 6, float %.i1243.i.i.i) #2
  %FAbs69.i.i.i = call float @dx.op.unary.f32(i32 6, float %.i2244.i.i.i) #2
  %v219.i.i.i = fcmp fast ogt float %FAbs.i.i.i, %FAbs68.i.i.i
  %v220.i.i.i = fcmp fast ogt float %FAbs.i.i.i, %FAbs69.i.i.i
  %v221.i.i.i = and i1 %v219.i.i.i, %v220.i.i.i
  br i1 %v221.i.i.i, label %"\01?CalculateUVDerivatives@@YAXV?$vector@M$02@@00000AIAV?$vector@M$01@@1@Z.exit.i.i.i", label %26

; <label>:26                                      ; preds = %"\01?Load3x16BitIndices@@YA?AV?$vector@I$02@@I@Z.exit.i.i.i"
  %v222.i.i.i = fcmp fast ogt float %FAbs68.i.i.i, %FAbs69.i.i.i
  br i1 %v222.i.i.i, label %"\01?CalculateUVDerivatives@@YAXV?$vector@M$02@@00000AIAV?$vector@M$01@@1@Z.exit.i.i.i", label %27

; <label>:27                                      ; preds = %26
  br label %"\01?CalculateUVDerivatives@@YAXV?$vector@M$02@@00000AIAV?$vector@M$01@@1@Z.exit.i.i.i"

"\01?CalculateUVDerivatives@@YAXV?$vector@M$02@@00000AIAV?$vector@M$01@@1@Z.exit.i.i.i": ; preds = %26, %"\01?Load3x16BitIndices@@YA?AV?$vector@I$02@@I@Z.exit.i.i.i", %27
  %v223.i.i.i = phi float [ %.i1267.i.i.i, %27 ], [ %.i2268.i.i.i, %"\01?Load3x16BitIndices@@YA?AV?$vector@I$02@@I@Z.exit.i.i.i" ], [ %.i2268.i.i.i, %26 ]
  %v224.i.i.i = phi float [ %.i1196.i.i.i, %27 ], [ %.i2197.i.i.i, %"\01?Load3x16BitIndices@@YA?AV?$vector@I$02@@I@Z.exit.i.i.i" ], [ %.i2197.i.i.i, %26 ]
  %v225.i.i.i = phi float [ %.i1255.i.i.i, %27 ], [ %.i2256.i.i.i, %"\01?Load3x16BitIndices@@YA?AV?$vector@I$02@@I@Z.exit.i.i.i" ], [ %.i2256.i.i.i, %26 ]
  %v226.i.i.i = phi float [ %.i1298.i.i.i, %27 ], [ %.i2299.i.i.i, %"\01?Load3x16BitIndices@@YA?AV?$vector@I$02@@I@Z.exit.i.i.i" ], [ %.i2299.i.i.i, %26 ]
  %v227.i.i.i = phi float [ %.i1289.i.i.i, %27 ], [ %.i2290.i.i.i, %"\01?Load3x16BitIndices@@YA?AV?$vector@I$02@@I@Z.exit.i.i.i" ], [ %.i2290.i.i.i, %26 ]
  %indices.i.0.i0.i.i.i = phi i32 [ 0, %27 ], [ 1, %"\01?Load3x16BitIndices@@YA?AV?$vector@I$02@@I@Z.exit.i.i.i" ], [ 0, %26 ]
  %v228.i.i.i = getelementptr [3 x float], [3 x float]* %v15.i.32.i.i, i32 0, i32 %indices.i.0.i0.i.i.i
  %v229.i.i.i = load float, float* %v228.i.i.i, align 4, !tbaa !91, !noalias !98
  %v230.i.i.i = getelementptr [3 x float], [3 x float]* %v14.i.31.i.i, i32 0, i32 %indices.i.0.i0.i.i.i
  %v231.i.i.i = load float, float* %v230.i.i.i, align 4, !tbaa !91, !noalias !98
  %v232.i.i.i = fmul fast float %v226.i.i.i, %v229.i.i.i
  %v233.i.i.i = fmul fast float %v227.i.i.i, %v231.i.i.i
  %v234.i.i.i = fsub fast float %v232.i.i.i, %v233.i.i.i
  %v235.i.i.i = fdiv fast float 1.000000e+00, %v234.i.i.i
  %.i0300.i.i.i = fmul fast float %v235.i.i.i, %v226.i.i.i
  %v236.i.i.i = fmul fast float %v227.i.i.i, %v235.i.i.i
  %.i1301.i.i.i = fsub fast float -0.000000e+00, %v236.i.i.i
  %v237.i.i.i = fmul fast float %v231.i.i.i, %v235.i.i.i
  %.i2302.i.i.i = fsub fast float -0.000000e+00, %v237.i.i.i
  %.i3303.i.i.i = fmul fast float %v235.i.i.i, %v229.i.i.i
  %v238.i.i.i = getelementptr [3 x float], [3 x float]* %v12.i.29.i.i, i32 0, i32 %indices.i.0.i0.i.i.i
  %v239.i.i.i = load float, float* %v238.i.i.i, align 4, !tbaa !91, !noalias !98
  %v240.i.i.i = getelementptr [3 x float], [3 x float]* %v13.i.30.i.i, i32 0, i32 %indices.i.0.i0.i.i.i
  %v241.i.i.i = load float, float* %v240.i.i.i, align 4, !tbaa !91, !noalias !98
  %v242.i.i.i = fsub fast float %v239.i.i.i, %v241.i.i.i
  %v243.i.i.i = fsub fast float %v225.i.i.i, %v224.i.i.i
  %v244.i.i.i = fmul fast float %v242.i.i.i, %.i0300.i.i.i
  %FMad21.i.i.i = call float @dx.op.tertiary.f32(i32 46, float %v243.i.i.i, float %.i1301.i.i.i, float %v244.i.i.i) #2
  %v245.i.i.i = fmul fast float %v242.i.i.i, %.i2302.i.i.i
  %FMad.i.86.i.i = call float @dx.op.tertiary.f32(i32 46, float %v243.i.i.i, float %.i3303.i.i.i, float %v245.i.i.i) #2
  %FAbs79.i.i.i = call float @dx.op.unary.f32(i32 6, float %FMad21.i.i.i) #2
  %FAbs80.i.i.i = call float @dx.op.unary.f32(i32 6, float %FMad.i.86.i.i) #2
  %v246.i.i.i = getelementptr [3 x float], [3 x float]* %v11.i.28.i.i, i32 0, i32 %indices.i.0.i0.i.i.i
  %v247.i.i.i = load float, float* %v246.i.i.i, align 4, !tbaa !91, !noalias !98
  %v248.i.i.i = fsub fast float %v247.i.i.i, %v241.i.i.i
  %v249.i.i.i = fsub fast float %v223.i.i.i, %v224.i.i.i
  %v250.i.i.i = fmul fast float %v248.i.i.i, %.i0300.i.i.i
  %FMad53.i.i.i = call float @dx.op.tertiary.f32(i32 46, float %v249.i.i.i, float %.i1301.i.i.i, float %v250.i.i.i) #2
  %v251.i.i.i = fmul fast float %v248.i.i.i, %.i2302.i.i.i
  %FMad52.i.i.i = call float @dx.op.tertiary.f32(i32 46, float %v249.i.i.i, float %.i3303.i.i.i, float %v251.i.i.i) #2
  %FAbs77.i.i.i = call float @dx.op.unary.f32(i32 6, float %FMad53.i.i.i) #2
  %FAbs78.i.i.i = call float @dx.op.unary.f32(i32 6, float %FMad52.i.i.i) #2
  %.i0304.i.i.i = fsub fast float -0.000000e+00, %14
  %.i1305.i.i.i = fsub fast float -0.000000e+00, %13
  %.i2306.i.i.i = fsub fast float -0.000000e+00, %12
  %v252.i.i.i = fmul fast float %14, %14
  %v253.i.i.i = fmul fast float %13, %13
  %v254.i.i.i = fadd fast float %v253.i.i.i, %v252.i.i.i
  %v255.i.i.i = fmul fast float %12, %12
  %v256.i.i.i = fadd fast float %v254.i.i.i, %v255.i.i.i
  %Sqrt75.i.i.i = call float @dx.op.unary.f32(i32 24, float %v256.i.i.i) #2
  %.i0307.i.i.i = fdiv fast float %.i0304.i.i.i, %Sqrt75.i.i.i
  %.i1308.i.i.i = fdiv fast float %.i1305.i.i.i, %Sqrt75.i.i.i
  %.i2309.i.i.i = fdiv fast float %.i2306.i.i.i, %Sqrt75.i.i.i
  %28 = add i32 %v37.i.54.i.i, 5
  %g_texDiffuse_texture_2d = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 0, i32 2, i32 %28, i1 true)
  %v261.i.i.i = call %dx.types.ResRet.f32 @dx.op.sampleGrad.f32(i32 63, %dx.types.Handle %g_texDiffuse_texture_2d, %dx.types.Handle %g_s0_sampler, float %.i0128.i.i.i, float %.i1129.i.i.i, float undef, float undef, i32 undef, i32 undef, i32 undef, float %FAbs79.i.i.i, float %FAbs80.i.i.i, float undef, float %FAbs77.i.i.i, float %FAbs78.i.i.i, float undef, float undef) #2
  %v262.i.i.i = extractvalue %dx.types.ResRet.f32 %v261.i.i.i, 0
  %v263.i.i.i = extractvalue %dx.types.ResRet.f32 %v261.i.i.i, 1
  %v264.i.i.i = extractvalue %dx.types.ResRet.f32 %v261.i.i.i, 2
  %29 = add i32 %v37.i.54.i.i, 32
  %g_texNormal_texture_2d = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 0, i32 0, i32 %29, i1 true)
  %v269.i.i.i = call %dx.types.ResRet.f32 @dx.op.sampleGrad.f32(i32 63, %dx.types.Handle %g_texNormal_texture_2d, %dx.types.Handle %g_s0_sampler, float %.i0128.i.i.i, float %.i1129.i.i.i, float undef, float undef, i32 undef, i32 undef, i32 undef, float %FAbs79.i.i.i, float %FAbs80.i.i.i, float undef, float %FAbs77.i.i.i, float %FAbs78.i.i.i, float undef, float undef) #2
  %v270.i.i.i = extractvalue %dx.types.ResRet.f32 %v269.i.i.i, 0
  %v271.i.i.i = extractvalue %dx.types.ResRet.f32 %v269.i.i.i, 1
  %v272.i.i.i = extractvalue %dx.types.ResRet.f32 %v269.i.i.i, 2
  %.i0310.i.i.i = fmul fast float %v270.i.i.i, 2.000000e+00
  %.i1311.i.i.i = fmul fast float %v271.i.i.i, 2.000000e+00
  %.i2312.i.i.i = fmul fast float %v272.i.i.i, 2.000000e+00
  %.i0313.i.i.i = fadd fast float %.i0310.i.i.i, -1.000000e+00
  %.i1314.i.i.i = fadd fast float %.i1311.i.i.i, -1.000000e+00
  %.i2315.i.i.i = fadd fast float %.i2312.i.i.i, -1.000000e+00
  %v273.i.i.i = call float @dx.op.dot3.f32(i32 55, float %.i0313.i.i.i, float %.i1314.i.i.i, float %.i2315.i.i.i, float %.i0313.i.i.i, float %.i1314.i.i.i, float %.i2315.i.i.i) #2
  %Rsqrt.i.i.i = call float @dx.op.unary.f32(i32 25, float %v273.i.i.i) #2
  %.i0316.i.i.i = fmul fast float %.i0313.i.i.i, %Rsqrt.i.i.i
  %.i1317.i.i.i = fmul fast float %.i1314.i.i.i, %Rsqrt.i.i.i
  %.i2318.i.i.i = fmul fast float %.i2315.i.i.i, %Rsqrt.i.i.i
  %v274.i.i.i = fmul fast float %.i2182.i.i.i, %.i1154.i.i.i
  %v275.i.i.i = fmul fast float %.i1181.i.i.i, %.i2155.i.i.i
  %v276.i.i.i = fsub fast float %v274.i.i.i, %v275.i.i.i
  %v277.i.i.i = fmul fast float %.i0180.i.i.i, %.i2155.i.i.i
  %v278.i.i.i = fmul fast float %.i2182.i.i.i, %.i0153.i.i.i
  %v279.i.i.i = fsub fast float %v277.i.i.i, %v278.i.i.i
  %v280.i.i.i = fmul fast float %.i1181.i.i.i, %.i0153.i.i.i
  %v281.i.i.i = fmul fast float %.i0180.i.i.i, %.i1154.i.i.i
  %v282.i.i.i = fsub fast float %v280.i.i.i, %v281.i.i.i
  %v283.i.i.i = fmul fast float %.i0316.i.i.i, %.i0180.i.i.i
  %FMad51.i.i.i = call float @dx.op.tertiary.f32(i32 46, float %.i1317.i.i.i, float %v276.i.i.i, float %v283.i.i.i) #2
  %FMad50.i.i.i = call float @dx.op.tertiary.f32(i32 46, float %.i2318.i.i.i, float %.i0153.i.i.i, float %FMad51.i.i.i) #2
  %v284.i.i.i = fmul fast float %.i0316.i.i.i, %.i1181.i.i.i
  %FMad49.i.i.i = call float @dx.op.tertiary.f32(i32 46, float %.i1317.i.i.i, float %v279.i.i.i, float %v284.i.i.i) #2
  %FMad48.i.i.i = call float @dx.op.tertiary.f32(i32 46, float %.i2318.i.i.i, float %.i1154.i.i.i, float %FMad49.i.i.i) #2
  %v285.i.i.i = fmul fast float %.i0316.i.i.i, %.i2182.i.i.i
  %FMad47.i.i.i = call float @dx.op.tertiary.f32(i32 46, float %.i1317.i.i.i, float %v282.i.i.i, float %v285.i.i.i) #2
  %FMad46.i.i.i = call float @dx.op.tertiary.f32(i32 46, float %.i2318.i.i.i, float %.i2155.i.i.i, float %FMad47.i.i.i) #2
  %v286.i.i.i = fmul fast float %FMad50.i.i.i, %FMad50.i.i.i
  %v287.i.i.i = fmul fast float %FMad48.i.i.i, %FMad48.i.i.i
  %v288.i.i.i = fadd fast float %v287.i.i.i, %v286.i.i.i
  %v289.i.i.i = fmul fast float %FMad46.i.i.i, %FMad46.i.i.i
  %v290.i.i.i = fadd fast float %v288.i.i.i, %v289.i.i.i
  %Sqrt76.i.i.i = call float @dx.op.unary.f32(i32 24, float %v290.i.i.i) #2
  %.i0319.i.i.i = fdiv fast float %FMad50.i.i.i, %Sqrt76.i.i.i
  %.i1320.i.i.i = fdiv fast float %FMad48.i.i.i, %Sqrt76.i.i.i
  %.i2321.i.i.i = fdiv fast float %FMad46.i.i.i, %Sqrt76.i.i.i
  %v291.i.i.i = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %HitShaderConstants_cbuffer, i32 2) #2
  %v292.i.i.i = extractvalue %dx.types.CBufRet.f32 %v291.i.i.i, 0
  %v293.i.i.i = extractvalue %dx.types.CBufRet.f32 %v291.i.i.i, 1
  %v294.i.i.i = extractvalue %dx.types.CBufRet.f32 %v291.i.i.i, 2
  %.i0322.i.i.i = fmul fast float %v292.i.i.i, %v262.i.i.i
  %.i1323.i.i.i = fmul fast float %v293.i.i.i, %v263.i.i.i
  %.i2324.i.i.i = fmul fast float %v294.i.i.i, %v264.i.i.i
  %TextureLoad.i.87.i.i = call %dx.types.ResRet.f32 @dx.op.textureLoad.f32(i32 66, %dx.types.Handle %texSSAO_texture_2d, i32 0, i32 %DTidx.i, i32 %DTidy.i, i32 undef, i32 undef, i32 undef, i32 undef) #2
  %v296.i.i.i = extractvalue %dx.types.ResRet.f32 %TextureLoad.i.87.i.i, 0
  %.i0325.i.i.i = fmul fast float %.i0322.i.i.i, %v296.i.i.i
  %.i1326.i.i.i = fmul fast float %.i1323.i.i.i, %v296.i.i.i
  %.i2327.i.i.i = fmul fast float %.i2324.i.i.i, %v296.i.i.i
  %v297.i.i.i = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %HitShaderConstants_cbuffer, i32 8) #2
  %v298.i.i.i = extractvalue %dx.types.CBufRet.i32 %v297.i.i.i, 1
  %v299.i.i.i = icmp eq i32 %v298.i.i.i, 0
  br i1 %v299.i.i.i, label %48, label %remat_begin.i.101.i.i

remat_begin.i.101.i.i:                            ; preds = %"\01?CalculateUVDerivatives@@YAXV?$vector@M$02@@00000AIAV?$vector@M$01@@1@Z.exit.i.i.i"
  %v300.i.i.i = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %HitShaderConstants_cbuffer, i32 0) #2
  %v301.i.i.i = extractvalue %dx.types.CBufRet.f32 %v300.i.i.i, 0
  %v302.i.i.i = extractvalue %dx.types.CBufRet.f32 %v300.i.i.i, 1
  %v303.i.i.i = extractvalue %dx.types.CBufRet.f32 %v300.i.i.i, 2
  %add.i.87.i.i.i = add nsw i32 %sub.i.i.17.i.i, 16
  %arrayidx.i.89.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.87.i.i.i
  store i32 1, i32* %arrayidx.i.89.i.i.i, align 8
  %add.i.84.i.i.i = add nsw i32 %sub.i.i.17.i.i, 17
  %arrayidx.i.86.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.84.i.i.i
  %v305.i.i.i = bitcast i32* %arrayidx.i.86.i.i.i to float*
  store float 0x47EFFFFFE0000000, float* %v305.i.i.i, align 4
  %new.payload.offset.i.88.i.i = add i32 %sub.i.i.17.i.i, 16
  %arrayidx.i.83.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %sub.i.i.17.i.i
  store i32 1003, i32* %arrayidx.i.83.i.i.i
  %add.i.79.i.i.i = add nsw i32 %sub.i.i.17.i.i, 1
  %arrayidx.i.81.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.79.i.i.i
  store i32 4, i32* %arrayidx.i.81.i.i.i
  %add.i.76.i.i.i = add nsw i32 %sub.i.i.17.i.i, 2
  %arrayidx.i.78.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.76.i.i.i
  store i32 -1, i32* %arrayidx.i.78.i.i.i
  %add.i.73.i.i.i = add nsw i32 %sub.i.i.17.i.i, 3
  %arrayidx.i.75.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.73.i.i.i
  store i32 0, i32* %arrayidx.i.75.i.i.i
  %add.i.70.i.i.i = add nsw i32 %sub.i.i.17.i.i, 4
  %arrayidx.i.72.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.70.i.i.i
  store i32 1, i32* %arrayidx.i.72.i.i.i
  %add.i.67.i.i.i = add nsw i32 %sub.i.i.17.i.i, 5
  %arrayidx.i.69.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.67.i.i.i
  store i32 0, i32* %arrayidx.i.69.i.i.i
  %30 = bitcast float %.i0195.i.i.i to i32
  %add.i.64.i.i.i = add nsw i32 %sub.i.i.17.i.i, 6
  %arrayidx.i.66.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.64.i.i.i
  store i32 %30, i32* %arrayidx.i.66.i.i.i
  %31 = bitcast float %.i1196.i.i.i to i32
  %add.i.61.i.i.i = add nsw i32 %sub.i.i.17.i.i, 7
  %arrayidx.i.63.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.61.i.i.i
  store i32 %31, i32* %arrayidx.i.63.i.i.i
  %32 = bitcast float %.i2197.i.i.i to i32
  %add.i.58.i.i.i = add nsw i32 %sub.i.i.17.i.i, 8
  %arrayidx.i.60.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.58.i.i.i
  store i32 %32, i32* %arrayidx.i.60.i.i.i
  %add.i.55.i.i.i = add nsw i32 %sub.i.i.17.i.i, 9
  %arrayidx.i.57.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.55.i.i.i
  store i32 1036831949, i32* %arrayidx.i.57.i.i.i
  %33 = bitcast float %v301.i.i.i to i32
  %add.i.52.i.i.i = add nsw i32 %sub.i.i.17.i.i, 10
  %arrayidx.i.54.i.89.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.52.i.i.i
  store i32 %33, i32* %arrayidx.i.54.i.89.i.i
  %34 = bitcast float %v302.i.i.i to i32
  %add.i.49.i.90.i.i = add nsw i32 %sub.i.i.17.i.i, 11
  %arrayidx.i.51.i.92.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.49.i.90.i.i
  store i32 %34, i32* %arrayidx.i.51.i.92.i.i
  %35 = bitcast float %v303.i.i.i to i32
  %add.i.46.i.93.i.i = add nsw i32 %sub.i.i.17.i.i, 12
  %arrayidx.i.48.i.95.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.46.i.93.i.i
  store i32 %35, i32* %arrayidx.i.48.i.95.i.i
  %add.i.43.i.i.i = add nsw i32 %sub.i.i.17.i.i, 13
  %arrayidx.i.45.i.97.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.43.i.i.i
  store i32 2139095039, i32* %arrayidx.i.45.i.97.i.i
  %add.i.40.i.i.i = add nsw i32 %sub.i.i.17.i.i, 14
  %arrayidx.i.42.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.40.i.i.i
  store i32 %new.payload.offset.i.88.i.i, i32* %arrayidx.i.42.i.i.i
  %add.i.37.i.i.i = add nsw i32 %sub.i.i.17.i.i, 18
  %arrayidx.i.39.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.37.i.i.i
  store i32 %DTidx.i, i32* %arrayidx.i.39.i.i.i
  %add.i.34.i.i.i = add nsw i32 %sub.i.i.17.i.i, 19
  %arrayidx.i.36.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.34.i.i.i
  store i32 %DTidy.i, i32* %arrayidx.i.36.i.i.i
  %36 = bitcast float %.i0307.i.i.i to i32
  %add.i.31.i.i.i = add nsw i32 %sub.i.i.17.i.i, 20
  %arrayidx.i.33.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.31.i.i.i
  store i32 %36, i32* %arrayidx.i.33.i.i.i
  %37 = bitcast float %.i1308.i.i.i to i32
  %add.i.28.i.i.i = add nsw i32 %sub.i.i.17.i.i, 21
  %arrayidx.i.30.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.28.i.i.i
  store i32 %37, i32* %arrayidx.i.30.i.i.i
  %38 = bitcast float %.i2309.i.i.i to i32
  %add.i.25.i.i.i = add nsw i32 %sub.i.i.17.i.i, 22
  %arrayidx.i.27.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.25.i.i.i
  store i32 %38, i32* %arrayidx.i.27.i.i.i
  %39 = bitcast float %v262.i.i.i to i32
  %add.i.22.i.i.i = add nsw i32 %sub.i.i.17.i.i, 23
  %arrayidx.i.24.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.22.i.i.i
  store i32 %39, i32* %arrayidx.i.24.i.i.i
  %40 = bitcast float %v263.i.i.i to i32
  %add.i.19.i.i.i = add nsw i32 %sub.i.i.17.i.i, 24
  %arrayidx.i.21.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.19.i.i.i
  store i32 %40, i32* %arrayidx.i.21.i.i.i
  %41 = bitcast float %v264.i.i.i to i32
  %add.i.16.i.i.i = add nsw i32 %sub.i.i.17.i.i, 25
  %arrayidx.i.18.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.16.i.i.i
  store i32 %41, i32* %arrayidx.i.18.i.i.i
  %42 = bitcast float %.i0319.i.i.i to i32
  %add.i.13.i.i.i = add nsw i32 %sub.i.i.17.i.i, 26
  %arrayidx.i.15.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.13.i.i.i
  store i32 %42, i32* %arrayidx.i.15.i.i.i
  %43 = bitcast float %.i1320.i.i.i to i32
  %add.i.10.i.i.i = add nsw i32 %sub.i.i.17.i.i, 27
  %arrayidx.i.12.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.10.i.i.i
  store i32 %43, i32* %arrayidx.i.12.i.i.i
  %44 = bitcast float %.i2321.i.i.i to i32
  %add.i.7.i.i.i = add nsw i32 %sub.i.i.17.i.i, 28
  %arrayidx.i.9.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.7.i.i.i
  store i32 %44, i32* %arrayidx.i.9.i.i.i
  %45 = bitcast float %.i0325.i.i.i to i32
  %add.i.4.i.i.i = add nsw i32 %sub.i.i.17.i.i, 29
  %arrayidx.i.6.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.4.i.i.i
  store i32 %45, i32* %arrayidx.i.6.i.i.i
  %46 = bitcast float %.i1326.i.i.i to i32
  %add.i.1.i.i.i = add nsw i32 %sub.i.i.17.i.i, 30
  %arrayidx.i.3.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.1.i.i.i
  store i32 %46, i32* %arrayidx.i.3.i.i.i
  %47 = bitcast float %.i2327.i.i.i to i32
  %add.i.i.98.i.i = add nsw i32 %sub.i.i.17.i.i, 31
  %arrayidx.i.i.100.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.i.98.i.i
  store i32 %47, i32* %arrayidx.i.i.100.i.i
  br label %while.cond.i

; <label>:48                                      ; preds = %"\01?CalculateUVDerivatives@@YAXV?$vector@M$02@@00000AIAV?$vector@M$01@@1@Z.exit.i.i.i"
  %v309.i.i.i = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %HitShaderConstants_cbuffer, i32 4) #2
  %v310.i.i.i = extractvalue %dx.types.CBufRet.f32 %v309.i.i.i, 0
  %v311.i.i.i = extractvalue %dx.types.CBufRet.f32 %v309.i.i.i, 1
  %v312.i.i.i = extractvalue %dx.types.CBufRet.f32 %v309.i.i.i, 2
  %v313.i.i.i = extractvalue %dx.types.CBufRet.f32 %v309.i.i.i, 3
  %v314.i.i.i = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %HitShaderConstants_cbuffer, i32 5) #2
  %v315.i.i.i = extractvalue %dx.types.CBufRet.f32 %v314.i.i.i, 0
  %v316.i.i.i = extractvalue %dx.types.CBufRet.f32 %v314.i.i.i, 1
  %v317.i.i.i = extractvalue %dx.types.CBufRet.f32 %v314.i.i.i, 2
  %v318.i.i.i = extractvalue %dx.types.CBufRet.f32 %v314.i.i.i, 3
  %v319.i.i.i = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %HitShaderConstants_cbuffer, i32 6) #2
  %v320.i.i.i = extractvalue %dx.types.CBufRet.f32 %v319.i.i.i, 0
  %v321.i.i.i = extractvalue %dx.types.CBufRet.f32 %v319.i.i.i, 1
  %v322.i.i.i = extractvalue %dx.types.CBufRet.f32 %v319.i.i.i, 2
  %v323.i.i.i = extractvalue %dx.types.CBufRet.f32 %v319.i.i.i, 3
  %v324.i.i.i = fmul fast float %v310.i.i.i, %.i0195.i.i.i
  %FMad45.i.i.i = call float @dx.op.tertiary.f32(i32 46, float %.i1196.i.i.i, float %v311.i.i.i, float %v324.i.i.i) #2
  %FMad44.i.i.i = call float @dx.op.tertiary.f32(i32 46, float %.i2197.i.i.i, float %v312.i.i.i, float %FMad45.i.i.i) #2
  %FMad43.i.i.i = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %v313.i.i.i, float %FMad44.i.i.i) #2
  %v325.i.i.i = fmul fast float %v315.i.i.i, %.i0195.i.i.i
  %FMad42.i.i.i = call float @dx.op.tertiary.f32(i32 46, float %.i1196.i.i.i, float %v316.i.i.i, float %v325.i.i.i) #2
  %FMad41.i.i.i = call float @dx.op.tertiary.f32(i32 46, float %.i2197.i.i.i, float %v317.i.i.i, float %FMad42.i.i.i) #2
  %FMad40.i.i.i = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %v318.i.i.i, float %FMad41.i.i.i) #2
  %v326.i.i.i = fmul fast float %v320.i.i.i, %.i0195.i.i.i
  %FMad39.i.i.i = call float @dx.op.tertiary.f32(i32 46, float %.i1196.i.i.i, float %v321.i.i.i, float %v326.i.i.i) #2
  %FMad38.i.i.i = call float @dx.op.tertiary.f32(i32 46, float %.i2197.i.i.i, float %v322.i.i.i, float %FMad39.i.i.i) #2
  %FMad37.i.i.i = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %v323.i.i.i, float %FMad38.i.i.i) #2
  %v327.i.i.i = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %HitShaderConstants_cbuffer, i32 3) #2
  %v328.i.i.i = extractvalue %dx.types.CBufRet.f32 %v327.i.i.i, 0
  %v329.i.i.i = fmul fast float %v328.i.i.i, 2.500000e-01
  %v330.i.i.i = fmul fast float %v328.i.i.i, 1.750000e+00
  %v331.i.i.i = fmul fast float %v328.i.i.i, 1.250000e+00
  %v332.i.i.i = fmul fast float %v328.i.i.i, 7.500000e-01
  %v335.i.i.i = call %dx.types.ResRet.f32 @dx.op.sampleCmpLevelZero.f32(i32 65, %dx.types.Handle %texShadow_texture_2d, %dx.types.Handle %shadowSampler_sampler, float %FMad43.i.i.i, float %FMad40.i.i.i, float undef, float undef, i32 undef, i32 undef, i32 undef, float %FMad37.i.i.i) #2
  %v336.i.i.i = extractvalue %dx.types.ResRet.f32 %v335.i.i.i, 0
  %v337.i.i.i = fmul fast float %v336.i.i.i, 2.000000e+00
  %.i0328.i.i.i = fsub fast float %FMad43.i.i.i, %v330.i.i.i
  %.i1329.i.i.i = fadd fast float %v329.i.i.i, %FMad40.i.i.i
  %v338.i.i.i = call %dx.types.ResRet.f32 @dx.op.sampleCmpLevelZero.f32(i32 65, %dx.types.Handle %texShadow_texture_2d, %dx.types.Handle %shadowSampler_sampler, float %.i0328.i.i.i, float %.i1329.i.i.i, float undef, float undef, i32 undef, i32 undef, i32 undef, float %FMad37.i.i.i) #2
  %v339.i.i.i = extractvalue %dx.types.ResRet.f32 %v338.i.i.i, 0
  %v340.i.i.i = fadd fast float %v339.i.i.i, %v337.i.i.i
  %.i0330.i.i.i = fsub fast float %FMad43.i.i.i, %v329.i.i.i
  %.i1331.i.i.i = fsub fast float %FMad40.i.i.i, %v330.i.i.i
  %v341.i.i.i = call %dx.types.ResRet.f32 @dx.op.sampleCmpLevelZero.f32(i32 65, %dx.types.Handle %texShadow_texture_2d, %dx.types.Handle %shadowSampler_sampler, float %.i0330.i.i.i, float %.i1331.i.i.i, float undef, float undef, i32 undef, i32 undef, i32 undef, float %FMad37.i.i.i) #2
  %v342.i.i.i = extractvalue %dx.types.ResRet.f32 %v341.i.i.i, 0
  %v343.i.i.i = fadd fast float %v340.i.i.i, %v342.i.i.i
  %.i0332.i.i.i = fadd fast float %v330.i.i.i, %FMad43.i.i.i
  %.i1333.i.i.i = fsub fast float %FMad40.i.i.i, %v329.i.i.i
  %v344.i.i.i = call %dx.types.ResRet.f32 @dx.op.sampleCmpLevelZero.f32(i32 65, %dx.types.Handle %texShadow_texture_2d, %dx.types.Handle %shadowSampler_sampler, float %.i0332.i.i.i, float %.i1333.i.i.i, float undef, float undef, i32 undef, i32 undef, i32 undef, float %FMad37.i.i.i) #2
  %v345.i.i.i = extractvalue %dx.types.ResRet.f32 %v344.i.i.i, 0
  %v346.i.i.i = fadd fast float %v343.i.i.i, %v345.i.i.i
  %.i0334.i.i.i = fadd fast float %v329.i.i.i, %FMad43.i.i.i
  %.i1335.i.i.i = fadd fast float %v330.i.i.i, %FMad40.i.i.i
  %v347.i.i.i = call %dx.types.ResRet.f32 @dx.op.sampleCmpLevelZero.f32(i32 65, %dx.types.Handle %texShadow_texture_2d, %dx.types.Handle %shadowSampler_sampler, float %.i0334.i.i.i, float %.i1335.i.i.i, float undef, float undef, i32 undef, i32 undef, i32 undef, float %FMad37.i.i.i) #2
  %v348.i.i.i = extractvalue %dx.types.ResRet.f32 %v347.i.i.i, 0
  %v349.i.i.i = fadd fast float %v346.i.i.i, %v348.i.i.i
  %.i0336.i.i.i = fsub fast float %FMad43.i.i.i, %v332.i.i.i
  %.i1337.i.i.i = fadd fast float %v331.i.i.i, %FMad40.i.i.i
  %v350.i.i.i = call %dx.types.ResRet.f32 @dx.op.sampleCmpLevelZero.f32(i32 65, %dx.types.Handle %texShadow_texture_2d, %dx.types.Handle %shadowSampler_sampler, float %.i0336.i.i.i, float %.i1337.i.i.i, float undef, float undef, i32 undef, i32 undef, i32 undef, float %FMad37.i.i.i) #2
  %v351.i.i.i = extractvalue %dx.types.ResRet.f32 %v350.i.i.i, 0
  %v352.i.i.i = fadd fast float %v349.i.i.i, %v351.i.i.i
  %.i0338.i.i.i = fsub fast float %FMad43.i.i.i, %v331.i.i.i
  %.i1339.i.i.i = fsub fast float %FMad40.i.i.i, %v332.i.i.i
  %v353.i.i.i = call %dx.types.ResRet.f32 @dx.op.sampleCmpLevelZero.f32(i32 65, %dx.types.Handle %texShadow_texture_2d, %dx.types.Handle %shadowSampler_sampler, float %.i0338.i.i.i, float %.i1339.i.i.i, float undef, float undef, i32 undef, i32 undef, i32 undef, float %FMad37.i.i.i) #2
  %v354.i.i.i = extractvalue %dx.types.ResRet.f32 %v353.i.i.i, 0
  %v355.i.i.i = fadd fast float %v352.i.i.i, %v354.i.i.i
  %.i0340.i.i.i = fadd fast float %v332.i.i.i, %FMad43.i.i.i
  %.i1341.i.i.i = fsub fast float %FMad40.i.i.i, %v331.i.i.i
  %v356.i.i.i = call %dx.types.ResRet.f32 @dx.op.sampleCmpLevelZero.f32(i32 65, %dx.types.Handle %texShadow_texture_2d, %dx.types.Handle %shadowSampler_sampler, float %.i0340.i.i.i, float %.i1341.i.i.i, float undef, float undef, i32 undef, i32 undef, i32 undef, float %FMad37.i.i.i) #2
  %v357.i.i.i = extractvalue %dx.types.ResRet.f32 %v356.i.i.i, 0
  %v358.i.i.i = fadd fast float %v355.i.i.i, %v357.i.i.i
  %.i0342.i.i.i = fadd fast float %v331.i.i.i, %FMad43.i.i.i
  %.i1343.i.i.i = fadd fast float %v332.i.i.i, %FMad40.i.i.i
  %v359.i.i.i = call %dx.types.ResRet.f32 @dx.op.sampleCmpLevelZero.f32(i32 65, %dx.types.Handle %texShadow_texture_2d, %dx.types.Handle %shadowSampler_sampler, float %.i0342.i.i.i, float %.i1343.i.i.i, float undef, float undef, i32 undef, i32 undef, i32 undef, float %FMad37.i.i.i) #2
  %v360.i.i.i = extractvalue %dx.types.ResRet.f32 %v359.i.i.i, 0
  %v361.i.i.i = fadd fast float %v358.i.i.i, %v360.i.i.i
  %v362.i.i.i = fmul fast float %v361.i.i.i, 0x3FB99999A0000000
  %v363.i.i.i = fmul fast float %v362.i.i.i, %v362.i.i.i
  %v364.i.i.i = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %HitShaderConstants_cbuffer, i32 1) #2
  %v365.i.i.i = extractvalue %dx.types.CBufRet.f32 %v364.i.i.i, 0
  %v366.i.i.i = extractvalue %dx.types.CBufRet.f32 %v364.i.i.i, 1
  %v367.i.i.i = extractvalue %dx.types.CBufRet.f32 %v364.i.i.i, 2
  %v368.i.i.i = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %HitShaderConstants_cbuffer, i32 0) #2
  %v369.i.i.i = extractvalue %dx.types.CBufRet.f32 %v368.i.i.i, 0
  %v370.i.i.i = extractvalue %dx.types.CBufRet.f32 %v368.i.i.i, 1
  %v371.i.i.i = extractvalue %dx.types.CBufRet.f32 %v368.i.i.i, 2
  %.i0344.i.i.i = fsub fast float %v369.i.i.i, %.i0307.i.i.i
  %.i1345.i.i.i = fsub fast float %v370.i.i.i, %.i1308.i.i.i
  %.i2346.i.i.i = fsub fast float %v371.i.i.i, %.i2309.i.i.i
  %v372.i.i.i = fmul fast float %.i0344.i.i.i, %.i0344.i.i.i
  %v373.i.i.i = fmul fast float %.i1345.i.i.i, %.i1345.i.i.i
  %v374.i.i.i = fadd fast float %v372.i.i.i, %v373.i.i.i
  %v375.i.i.i = fmul fast float %.i2346.i.i.i, %.i2346.i.i.i
  %v376.i.i.i = fadd fast float %v374.i.i.i, %v375.i.i.i
  %Sqrt.i.102.i.i = call float @dx.op.unary.f32(i32 24, float %v376.i.i.i) #2
  %.i0347.i.i.i = fdiv fast float %.i0344.i.i.i, %Sqrt.i.102.i.i
  %.i1348.i.i.i = fdiv fast float %.i1345.i.i.i, %Sqrt.i.102.i.i
  %.i2349.i.i.i = fdiv fast float %.i2346.i.i.i, %Sqrt.i.102.i.i
  %v377.i.i.i = call float @dx.op.dot3.f32(i32 55, float %v369.i.i.i, float %v370.i.i.i, float %v371.i.i.i, float %.i0347.i.i.i, float %.i1348.i.i.i, float %.i2349.i.i.i) #2
  %Saturate19.i.i.i = call float @dx.op.unary.f32(i32 7, float %v377.i.i.i) #2
  %v378.i.i.i = fsub fast float 1.000000e+00, %Saturate19.i.i.i
  %Log66.i.i.i = call float @dx.op.unary.f32(i32 23, float %v378.i.i.i) #2
  %v379.i.i.i = fmul fast float %Log66.i.i.i, 5.000000e+00
  %Exp67.i.i.i = call float @dx.op.unary.f32(i32 21, float %v379.i.i.i) #2
  %v380.i.i.i = fmul fast float %Exp67.i.i.i, %v262.i.i.i
  %v381.i.i.i = fmul fast float %Exp67.i.i.i, %v263.i.i.i
  %v382.i.i.i = fmul fast float %Exp67.i.i.i, %v264.i.i.i
  %.i0362.i.i.i = fsub fast float %v262.i.i.i, %v380.i.i.i
  %.i1363.i.i.i = fsub fast float %v263.i.i.i, %v381.i.i.i
  %.i2364.i.i.i = fsub fast float %v264.i.i.i, %v382.i.i.i
  %v383.i.i.i = call float @dx.op.dot3.f32(i32 55, float %.i0319.i.i.i, float %.i1320.i.i.i, float %.i2321.i.i.i, float %v369.i.i.i, float %v370.i.i.i, float %v371.i.i.i) #2
  %Saturate.i.i.i = call float @dx.op.unary.f32(i32 7, float %v383.i.i.i) #2
  %.i0365.i.i.i = fmul fast float %v365.i.i.i, %v363.i.i.i
  %.i0368.i.i.i = fmul fast float %.i0365.i.i.i, %Saturate.i.i.i
  %.i0371.i.i.i = fmul fast float %.i0368.i.i.i, %.i0362.i.i.i
  %.i1366.i.i.i = fmul fast float %v366.i.i.i, %v363.i.i.i
  %.i1369.i.i.i = fmul fast float %.i1366.i.i.i, %Saturate.i.i.i
  %.i1372.i.i.i = fmul fast float %.i1369.i.i.i, %.i1363.i.i.i
  %.i2367.i.i.i = fmul fast float %v367.i.i.i, %v363.i.i.i
  %.i2370.i.i.i = fmul fast float %.i2367.i.i.i, %Saturate.i.i.i
  %.i2373.i.i.i = fmul fast float %.i2370.i.i.i, %.i2364.i.i.i
  %.i0374.i.i.i = fadd fast float %.i0371.i.i.i, %.i0325.i.i.i
  %.i1375.i.i.i = fadd fast float %.i1372.i.i.i, %.i1326.i.i.i
  %.i2376.i.i.i = fadd fast float %.i2373.i.i.i, %.i2327.i.i.i
  %v384.i.i.i = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %HitShaderConstants_cbuffer, i32 8) #2
  %v385.i.i.i = extractvalue %dx.types.CBufRet.i32 %v384.i.i.i, 0
  %v386.i.i.i = icmp eq i32 %v385.i.i.i, 0
  br i1 %v386.i.i.i, label %._crit_edge.i.i.i, label %49

; <label>:49                                      ; preds = %48
  %.i0377.i.i.i = fmul fast float %.i0374.i.i.i, 0x3FD3333340000000
  %.i1378.i.i.i = fmul fast float %.i1375.i.i.i, 0x3FD3333340000000
  %.i2379.i.i.i = fmul fast float %.i2376.i.i.i, 0x3FD3333340000000
  %TextureLoad113.i.i.i = call %dx.types.ResRet.f32 @dx.op.textureLoad.f32(i32 66, %dx.types.Handle %g_screenOutput_UAV_2d, i32 undef, i32 %DTidx.i, i32 %DTidy.i, i32 undef, i32 undef, i32 undef, i32 undef) #2
  %v388.i.i.i = extractvalue %dx.types.ResRet.f32 %TextureLoad113.i.i.i, 0
  %v389.i.i.i = extractvalue %dx.types.ResRet.f32 %TextureLoad113.i.i.i, 1
  %v390.i.i.i = extractvalue %dx.types.ResRet.f32 %TextureLoad113.i.i.i, 2
  %.i0380.i.i.i = fadd fast float %v388.i.i.i, %.i0377.i.i.i
  %.i1381.i.i.i = fadd fast float %v389.i.i.i, %.i1378.i.i.i
  %.i2382.i.i.i = fadd fast float %v390.i.i.i, %.i2379.i.i.i
  br label %._crit_edge.i.i.i

._crit_edge.i.i.i:                                ; preds = %48, %49
  %outputColor.0.i0.i.i.i = phi float [ %.i0380.i.i.i, %49 ], [ %.i0374.i.i.i, %48 ]
  %outputColor.0.i1.i.i.i = phi float [ %.i1381.i.i.i, %49 ], [ %.i1375.i.i.i, %48 ]
  %outputColor.0.i2.i.i.i = phi float [ %.i2382.i.i.i, %49 ], [ %.i2376.i.i.i, %48 ]
  call void @dx.op.textureStore.f32(i32 67, %dx.types.Handle %g_screenOutput_UAV_2d, i32 %DTidx.i, i32 %DTidy.i, i32 undef, float %outputColor.0.i0.i.i.i, float %outputColor.0.i1.i.i.i, float %outputColor.0.i2.i.i.i, float 1.000000e+00, i8 15) #2
  br label %Hit.BB0._crit_edge.i.i.i

state_1003.Hit.ss_1.i.i:                          ; preds = %while.body.i
  %add.i.45.i.i.i = add nsw i32 %6, 18
  %arrayidx.i.47.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.45.i.i.i
  %DispatchRaysIndex.int.i.i.i = load i32, i32* %arrayidx.i.47.i.i.i
  %add.i.42.i.i.i = add nsw i32 %6, 19
  %arrayidx.i.44.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.42.i.i.i
  %DispatchRaysIndex106.int.i.i.i = load i32, i32* %arrayidx.i.44.i.i.i
  %add.i.39.i.i.i = add nsw i32 %6, 20
  %arrayidx.i.41.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.39.i.i.i
  %.i0307.int.i.i.i = load i32, i32* %arrayidx.i.41.i.i.i
  %50 = bitcast i32 %.i0307.int.i.i.i to float
  %add.i.36.i.i.i = add nsw i32 %6, 21
  %arrayidx.i.38.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.36.i.i.i
  %.i1308.int.i.i.i = load i32, i32* %arrayidx.i.38.i.i.i
  %51 = bitcast i32 %.i1308.int.i.i.i to float
  %add.i.33.i.i.i = add nsw i32 %6, 22
  %arrayidx.i.35.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.33.i.i.i
  %.i2309.int.i.i.i = load i32, i32* %arrayidx.i.35.i.i.i
  %52 = bitcast i32 %.i2309.int.i.i.i to float
  %add.i.30.i.i.i = add nsw i32 %6, 23
  %arrayidx.i.32.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.30.i.i.i
  %v262.int.i.i.i = load i32, i32* %arrayidx.i.32.i.i.i
  %53 = bitcast i32 %v262.int.i.i.i to float
  %add.i.27.i.i.i = add nsw i32 %6, 24
  %arrayidx.i.29.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.27.i.i.i
  %v263.int.i.i.i = load i32, i32* %arrayidx.i.29.i.i.i
  %54 = bitcast i32 %v263.int.i.i.i to float
  %add.i.24.i.i.i = add nsw i32 %6, 25
  %arrayidx.i.26.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.24.i.i.i
  %v264.int.i.i.i = load i32, i32* %arrayidx.i.26.i.i.i
  %55 = bitcast i32 %v264.int.i.i.i to float
  %add.i.21.i.i.i = add nsw i32 %6, 26
  %arrayidx.i.23.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.21.i.i.i
  %.i0319.int.i.i.i = load i32, i32* %arrayidx.i.23.i.i.i
  %56 = bitcast i32 %.i0319.int.i.i.i to float
  %add.i.18.i.i.i = add nsw i32 %6, 27
  %arrayidx.i.20.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.18.i.i.i
  %.i1320.int.i.i.i = load i32, i32* %arrayidx.i.20.i.i.i
  %57 = bitcast i32 %.i1320.int.i.i.i to float
  %add.i.15.i.i.i = add nsw i32 %6, 28
  %arrayidx.i.17.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.15.i.i.i
  %.i2321.int.i.i.i = load i32, i32* %arrayidx.i.17.i.i.i
  %58 = bitcast i32 %.i2321.int.i.i.i to float
  %add.i.12.i.i.i = add nsw i32 %6, 29
  %arrayidx.i.14.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.12.i.i.i
  %.i0325.int.i.i.i = load i32, i32* %arrayidx.i.14.i.i.i
  %59 = bitcast i32 %.i0325.int.i.i.i to float
  %add.i.9.i.i.i = add nsw i32 %6, 30
  %arrayidx.i.11.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.9.i.i.i
  %.i1326.int.i.i.i = load i32, i32* %arrayidx.i.11.i.i.i
  %60 = bitcast i32 %.i1326.int.i.i.i to float
  %add.i.6.i.i.i = add nsw i32 %6, 31
  %arrayidx.i.8.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.6.i.i.i
  %.i2327.int.i.i.i = load i32, i32* %arrayidx.i.8.i.i.i
  %61 = bitcast i32 %.i2327.int.i.i.i to float
  %add.i.3.i.i.i = add nsw i32 %6, 17
  %arrayidx.i.5.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.3.i.i.i
  %v305.remat.i.i.i = bitcast i32* %arrayidx.i.5.i.i.i to float*
  %v307.i.i.i = load float, float* %v305.remat.i.i.i, align 4
  %v308.i.i.i = fcmp fast olt float %v307.i.i.i, 0x47EFFFFFE0000000
  %shadow.0.i.i.i = select i1 %v308.i.i.i, float 0.000000e+00, float 1.000000e+00
  %v364.i.107.i.i = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %HitShaderConstants_cbuffer, i32 1) #2
  %v365.i.108.i.i = extractvalue %dx.types.CBufRet.f32 %v364.i.107.i.i, 0
  %v366.i.109.i.i = extractvalue %dx.types.CBufRet.f32 %v364.i.107.i.i, 1
  %v367.i.110.i.i = extractvalue %dx.types.CBufRet.f32 %v364.i.107.i.i, 2
  %v368.i.111.i.i = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %HitShaderConstants_cbuffer, i32 0) #2
  %v369.i.112.i.i = extractvalue %dx.types.CBufRet.f32 %v368.i.111.i.i, 0
  %v370.i.113.i.i = extractvalue %dx.types.CBufRet.f32 %v368.i.111.i.i, 1
  %v371.i.114.i.i = extractvalue %dx.types.CBufRet.f32 %v368.i.111.i.i, 2
  %.i0344.i.115.i.i = fsub fast float %v369.i.112.i.i, %50
  %.i1345.i.116.i.i = fsub fast float %v370.i.113.i.i, %51
  %.i2346.i.117.i.i = fsub fast float %v371.i.114.i.i, %52
  %v372.i.118.i.i = fmul fast float %.i0344.i.115.i.i, %.i0344.i.115.i.i
  %v373.i.119.i.i = fmul fast float %.i1345.i.116.i.i, %.i1345.i.116.i.i
  %v374.i.120.i.i = fadd fast float %v372.i.118.i.i, %v373.i.119.i.i
  %v375.i.121.i.i = fmul fast float %.i2346.i.117.i.i, %.i2346.i.117.i.i
  %v376.i.122.i.i = fadd fast float %v374.i.120.i.i, %v375.i.121.i.i
  %Sqrt.i.123.i.i = call float @dx.op.unary.f32(i32 24, float %v376.i.122.i.i) #2
  %.i0347.i.124.i.i = fdiv fast float %.i0344.i.115.i.i, %Sqrt.i.123.i.i
  %.i1348.i.125.i.i = fdiv fast float %.i1345.i.116.i.i, %Sqrt.i.123.i.i
  %.i2349.i.126.i.i = fdiv fast float %.i2346.i.117.i.i, %Sqrt.i.123.i.i
  %v377.i.127.i.i = call float @dx.op.dot3.f32(i32 55, float %v369.i.112.i.i, float %v370.i.113.i.i, float %v371.i.114.i.i, float %.i0347.i.124.i.i, float %.i1348.i.125.i.i, float %.i2349.i.126.i.i) #2
  %Saturate19.i.128.i.i = call float @dx.op.unary.f32(i32 7, float %v377.i.127.i.i) #2
  %v378.i.129.i.i = fsub fast float 1.000000e+00, %Saturate19.i.128.i.i
  %Log66.i.130.i.i = call float @dx.op.unary.f32(i32 23, float %v378.i.129.i.i) #2
  %v379.i.131.i.i = fmul fast float %Log66.i.130.i.i, 5.000000e+00
  %Exp67.i.132.i.i = call float @dx.op.unary.f32(i32 21, float %v379.i.131.i.i) #2
  %v380.i.133.i.i = fmul fast float %Exp67.i.132.i.i, %53
  %v381.i.134.i.i = fmul fast float %Exp67.i.132.i.i, %54
  %v382.i.135.i.i = fmul fast float %Exp67.i.132.i.i, %55
  %.i0362.i.136.i.i = fsub fast float %53, %v380.i.133.i.i
  %.i1363.i.137.i.i = fsub fast float %54, %v381.i.134.i.i
  %.i2364.i.138.i.i = fsub fast float %55, %v382.i.135.i.i
  %v383.i.139.i.i = call float @dx.op.dot3.f32(i32 55, float %56, float %57, float %58, float %v369.i.112.i.i, float %v370.i.113.i.i, float %v371.i.114.i.i) #2
  %Saturate.i.140.i.i = call float @dx.op.unary.f32(i32 7, float %v383.i.139.i.i) #2
  %.i0365.i.141.i.i = fmul fast float %v365.i.108.i.i, %shadow.0.i.i.i
  %.i0368.i.142.i.i = fmul fast float %.i0365.i.141.i.i, %Saturate.i.140.i.i
  %.i0371.i.143.i.i = fmul fast float %.i0368.i.142.i.i, %.i0362.i.136.i.i
  %.i1366.i.144.i.i = fmul fast float %v366.i.109.i.i, %shadow.0.i.i.i
  %.i1369.i.145.i.i = fmul fast float %.i1366.i.144.i.i, %Saturate.i.140.i.i
  %.i1372.i.146.i.i = fmul fast float %.i1369.i.145.i.i, %.i1363.i.137.i.i
  %.i2367.i.147.i.i = fmul fast float %v367.i.110.i.i, %shadow.0.i.i.i
  %.i2370.i.148.i.i = fmul fast float %.i2367.i.147.i.i, %Saturate.i.140.i.i
  %.i2373.i.149.i.i = fmul fast float %.i2370.i.148.i.i, %.i2364.i.138.i.i
  %.i0374.i.150.i.i = fadd fast float %.i0371.i.143.i.i, %59
  %.i1375.i.151.i.i = fadd fast float %.i1372.i.146.i.i, %60
  %.i2376.i.152.i.i = fadd fast float %.i2373.i.149.i.i, %61
  %v384.i.153.i.i = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %HitShaderConstants_cbuffer, i32 8) #2
  %v385.i.154.i.i = extractvalue %dx.types.CBufRet.i32 %v384.i.153.i.i, 0
  %v386.i.155.i.i = icmp eq i32 %v385.i.154.i.i, 0
  br i1 %v386.i.155.i.i, label %Hit.ss_1.exit.i.i, label %62

; <label>:62                                      ; preds = %state_1003.Hit.ss_1.i.i
  %.i0377.i.157.i.i = fmul fast float %.i0374.i.150.i.i, 0x3FD3333340000000
  %.i1378.i.158.i.i = fmul fast float %.i1375.i.151.i.i, 0x3FD3333340000000
  %.i2379.i.159.i.i = fmul fast float %.i2376.i.152.i.i, 0x3FD3333340000000
  %TextureLoad113.i.161.i.i = call %dx.types.ResRet.f32 @dx.op.textureLoad.f32(i32 66, %dx.types.Handle %g_screenOutput_UAV_2d, i32 undef, i32 %DispatchRaysIndex.int.i.i.i, i32 %DispatchRaysIndex106.int.i.i.i, i32 undef, i32 undef, i32 undef, i32 undef) #2
  %v388.i.162.i.i = extractvalue %dx.types.ResRet.f32 %TextureLoad113.i.161.i.i, 0
  %v389.i.163.i.i = extractvalue %dx.types.ResRet.f32 %TextureLoad113.i.161.i.i, 1
  %v390.i.164.i.i = extractvalue %dx.types.ResRet.f32 %TextureLoad113.i.161.i.i, 2
  %.i0380.i.165.i.i = fadd fast float %v388.i.162.i.i, %.i0377.i.157.i.i
  %.i1381.i.166.i.i = fadd fast float %v389.i.163.i.i, %.i1378.i.158.i.i
  %.i2382.i.167.i.i = fadd fast float %v390.i.164.i.i, %.i2379.i.159.i.i
  br label %Hit.ss_1.exit.i.i

Hit.ss_1.exit.i.i:                                ; preds = %state_1003.Hit.ss_1.i.i, %62
  %outputColor.0.i0.i.168.i.i = phi float [ %.i0380.i.165.i.i, %62 ], [ %.i0374.i.150.i.i, %state_1003.Hit.ss_1.i.i ]
  %outputColor.0.i1.i.169.i.i = phi float [ %.i1381.i.166.i.i, %62 ], [ %.i1375.i.151.i.i, %state_1003.Hit.ss_1.i.i ]
  %outputColor.0.i2.i.170.i.i = phi float [ %.i2382.i.167.i.i, %62 ], [ %.i2376.i.152.i.i, %state_1003.Hit.ss_1.i.i ]
  call void @dx.op.textureStore.f32(i32 67, %dx.types.Handle %g_screenOutput_UAV_2d, i32 %DispatchRaysIndex.int.i.i.i, i32 %DispatchRaysIndex106.int.i.i.i, i32 undef, float %outputColor.0.i0.i.168.i.i, float %outputColor.0.i1.i.169.i.i, float %outputColor.0.i2.i.170.i.i, float 1.000000e+00, i8 15) #2
  %add.i.i.173.i.i = add nsw i32 %6, 32
  %arrayidx.i.i.176.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.i.173.i.i
  %ret.stateId.i.177.i.i = load i32, i32* %arrayidx.i.i.176.i.i
  br label %while.cond.i

state_1004.Miss.ss_0.i.i:                         ; preds = %while.body.i
  %arrayidx.i.7.i.183.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %9
  %v1.i.184.i.i = load i32, i32* %arrayidx.i.7.i.183.i.i, align 4
  %add.i.i.185.i.i = add nsw i32 %9, 1
  %arrayidx.i.5.i.187.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.i.185.i.i
  %v2.i.188.i.i = bitcast i32* %arrayidx.i.5.i.187.i.i to float*
  %v3.i.189.i.i = load float, float* %v2.i.188.i.i, align 4
  %v4.i.190.i.i = icmp eq i32 %v1.i.184.i.i, 0
  br i1 %v4.i.190.i.i, label %63, label %Miss.ss_0.exit.i.i

; <label>:63                                      ; preds = %state_1004.Miss.ss_0.i.i
  call void @dx.op.textureStore.f32(i32 67, %dx.types.Handle %g_screenOutput_UAV_2d, i32 %DTidx.i, i32 %DTidy.i, i32 undef, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 1.000000e+00, i8 15) #2
  br label %Miss.ss_0.exit.i.i

Miss.ss_0.exit.i.i:                               ; preds = %state_1004.Miss.ss_0.i.i, %63
  store i32 %v1.i.184.i.i, i32* %arrayidx.i.7.i.183.i.i, align 4
  store float %v3.i.189.i.i, float* %v2.i.188.i.i, align 4
  %arrayidx.i.3.i.192.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %6
  %ret.stateId.i.193.i.i = load i32, i32* %arrayidx.i.3.i.192.i.i
  br label %while.cond.i

state_1005.Fallback_TraceRay.ss_0.i.i:            ; preds = %while.body.i
  %add.i.i.200.i.i = add nsw i32 %6, -1
  %arrayidx.i.i.202.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.i.200.i.i
  store i32 %8, i32* %arrayidx.i.i.202.i.i, align 4
  %add2.i.i.i.i = add nsw i32 %6, -2
  %arrayidx4.i.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add2.i.i.i.i
  store i32 %7, i32* %arrayidx4.i.i.i.i, align 4
  %sub.i.i.204.i.i = sub nsw i32 %6, 2
  %sub6.i.i.i.i = sub nsw i32 %sub.i.i.204.i.i, 8
  %sub9.i.i.i.i = sub nsw i32 %6, 2
  %sub10.i.i.i.i = sub nsw i32 %sub9.i.i.i.i, 16
  %sub.i.137.i.i.i = sub nsw i32 %6, 24
  %add.i.132.i.i.i = add nsw i32 %6, 1
  %arrayidx.i.134.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.132.i.i.i
  %arg0.int.i.i.i = load i32, i32* %arrayidx.i.134.i.i.i
  %add.i.129.i.i.i = add nsw i32 %6, 2
  %arrayidx.i.131.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.129.i.i.i
  %arg1.int.i.i.i = load i32, i32* %arrayidx.i.131.i.i.i
  %add.i.126.i.i.i = add nsw i32 %6, 3
  %arrayidx.i.128.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.126.i.i.i
  %arg2.int.i.i.i = load i32, i32* %arrayidx.i.128.i.i.i
  %add.i.123.i.i.i = add nsw i32 %6, 4
  %arrayidx.i.125.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.123.i.i.i
  %arg3.int.i.i.i = load i32, i32* %arrayidx.i.125.i.i.i
  %add.i.120.i.i.i = add nsw i32 %6, 5
  %arrayidx.i.122.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.120.i.i.i
  %arg4.int.i.i.i = load i32, i32* %arrayidx.i.122.i.i.i
  %add.i.117.i.i.i = add nsw i32 %6, 6
  %arrayidx.i.119.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.117.i.i.i
  %arg5.int.i.i.i = load i32, i32* %arrayidx.i.119.i.i.i
  %64 = bitcast i32 %arg5.int.i.i.i to float
  %add.i.114.i.205.i.i = add nsw i32 %6, 7
  %arrayidx.i.116.i.207.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.114.i.205.i.i
  %arg6.int.i.i.i = load i32, i32* %arrayidx.i.116.i.207.i.i
  %65 = bitcast i32 %arg6.int.i.i.i to float
  %add.i.111.i.208.i.i = add nsw i32 %6, 8
  %arrayidx.i.113.i.210.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.111.i.208.i.i
  %arg7.int.i.i.i = load i32, i32* %arrayidx.i.113.i.210.i.i
  %66 = bitcast i32 %arg7.int.i.i.i to float
  %add.i.108.i.i.i = add nsw i32 %6, 9
  %arrayidx.i.110.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.108.i.i.i
  %arg8.int.i.i.i = load i32, i32* %arrayidx.i.110.i.i.i
  %67 = bitcast i32 %arg8.int.i.i.i to float
  %add.i.105.i.i.i = add nsw i32 %6, 10
  %arrayidx.i.107.i.212.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.105.i.i.i
  %arg9.int.i.i.i = load i32, i32* %arrayidx.i.107.i.212.i.i
  %68 = bitcast i32 %arg9.int.i.i.i to float
  %add.i.102.i.i.i = add nsw i32 %6, 11
  %arrayidx.i.104.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.102.i.i.i
  %arg10.int.i.i.i = load i32, i32* %arrayidx.i.104.i.i.i
  %69 = bitcast i32 %arg10.int.i.i.i to float
  %add.i.99.i.i.i = add nsw i32 %6, 12
  %arrayidx.i.101.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.99.i.i.i
  %arg11.int.i.i.i = load i32, i32* %arrayidx.i.101.i.i.i
  %70 = bitcast i32 %arg11.int.i.i.i to float
  %add.i.96.i.i.i = add nsw i32 %6, 13
  %arrayidx.i.98.i.213.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.96.i.i.i
  %arg12.int.i.i.i = load i32, i32* %arrayidx.i.98.i.213.i.i
  %71 = bitcast i32 %arg12.int.i.i.i to float
  %add.i.93.i.i.i = add nsw i32 %6, 14
  %arrayidx.i.95.i.214.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.93.i.i.i
  %arg13.int.i.i.i = load i32, i32* %arrayidx.i.95.i.214.i.i
  %v32.i.235.i.i = getelementptr inbounds [3 x float], [3 x float]* %v15.i.32.i.i, i32 0, i32 0
  store float %68, float* %v32.i.235.i.i, align 4
  %v33.i.236.i.i = getelementptr inbounds [3 x float], [3 x float]* %v15.i.32.i.i, i32 0, i32 1
  store float %69, float* %v33.i.236.i.i, align 4
  %v34.i.237.i.i = getelementptr inbounds [3 x float], [3 x float]* %v15.i.32.i.i, i32 0, i32 2
  store float %70, float* %v34.i.237.i.i, align 4
  %.i0.i.238.i.i = fdiv fast float 1.000000e+00, %68
  %.i1.i.239.i.i = fdiv fast float 1.000000e+00, %69
  %.i2.i.240.i.i = fdiv fast float 1.000000e+00, %70
  %.i0296.i.i.i = fmul fast float %.i0.i.238.i.i, %64
  %.i1297.i.i.i = fmul fast float %.i1.i.239.i.i, %65
  %.i2298.i.i.i = fmul fast float %.i2.i.240.i.i, %66
  %FAbs241.i.i.i = call float @dx.op.unary.f32(i32 6, float %68) #2
  %FAbs242.i.i.i = call float @dx.op.unary.f32(i32 6, float %69) #2
  %FAbs243.i.i.i = call float @dx.op.unary.f32(i32 6, float %70) #2
  %v35.i.241.i.i = fcmp fast ogt float %FAbs241.i.i.i, %FAbs242.i.i.i
  %v36.i.242.i.i = fcmp fast ogt float %FAbs241.i.i.i, %FAbs243.i.i.i
  %v37.i.243.i.i = and i1 %v35.i.241.i.i, %v36.i.242.i.i
  br i1 %v37.i.243.i.i, label %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i", label %72

; <label>:72                                      ; preds = %state_1005.Fallback_TraceRay.ss_0.i.i
  %v38.i.244.i.i = fcmp fast ogt float %FAbs242.i.i.i, %FAbs243.i.i.i
  br i1 %v38.i.244.i.i, label %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i", label %73

; <label>:73                                      ; preds = %72
  br label %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i"

"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i": ; preds = %72, %state_1005.Fallback_TraceRay.ss_0.i.i, %73
  %.0171.i.i.i = phi i32 [ 2, %73 ], [ 0, %state_1005.Fallback_TraceRay.ss_0.i.i ], [ 1, %72 ]
  %v39.i.245.i.i = add nuw nsw i32 %.0171.i.i.i, 1
  %v40.i.246.i.i = urem i32 %v39.i.245.i.i, 3
  %v41.i.247.i.i = add nuw nsw i32 %.0171.i.i.i, 2
  %v42.i.248.i.i = urem i32 %v41.i.247.i.i, 3
  %v43.i.249.i.i = getelementptr [3 x float], [3 x float]* %v15.i.32.i.i, i32 0, i32 %.0171.i.i.i
  %v44.i.250.i.i = load float, float* %v43.i.249.i.i, align 4, !tbaa !91, !noalias !102
  %v45.i.251.i.i = fcmp fast olt float %v44.i.250.i.i, 0.000000e+00
  %worldRayData.i.i.5.0.i0.i.i.i = select i1 %v45.i.251.i.i, i32 %v42.i.248.i.i, i32 %v40.i.246.i.i
  %worldRayData.i.i.5.0.i1.i.i.i = select i1 %v45.i.251.i.i, i32 %v40.i.246.i.i, i32 %v42.i.248.i.i
  %v46.i.252.i.i = getelementptr [3 x float], [3 x float]* %v15.i.32.i.i, i32 0, i32 %worldRayData.i.i.5.0.i0.i.i.i
  %v47.i.253.i.i = load float, float* %v46.i.252.i.i, align 4, !tbaa !91, !noalias !102
  %v48.i.254.i.i = fdiv float %v47.i.253.i.i, %v44.i.250.i.i
  %v49.i.255.i.i = getelementptr [3 x float], [3 x float]* %v15.i.32.i.i, i32 0, i32 %worldRayData.i.i.5.0.i1.i.i.i
  %v50.i.256.i.i = load float, float* %v49.i.255.i.i, align 4, !tbaa !91, !noalias !102
  %v51.i.257.i.i = fdiv float %v50.i.256.i.i, %v44.i.250.i.i
  %v52.i.258.i.i = fdiv float 1.000000e+00, %v44.i.250.i.i
  %v53.i.259.i.i = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %AccelerationStructureList_cbuffer, i32 0) #2
  %v54.i.260.i.i = extractvalue %dx.types.CBufRet.i32 %v53.i.259.i.i, 0
  %v55.i.261.i.i = extractvalue %dx.types.CBufRet.i32 %v53.i.259.i.i, 1
  %v56.i.262.i.i = getelementptr inbounds [2 x i32], [2 x i32]* %nodesToProcess.i.i.i.i.i, i32 0, i32 0
  store i32 0, i32* %v56.i.262.i.i, align 4, !tbaa !105
  %74 = add i32 %v55.i.261.i.i, 0
  %DescriptorHeapBufferTable_UAV_rawbuf336 = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 1, i32 1, i32 %74, i1 true)
  %v59.i.265.i.i = add i32 %v54.i.260.i.i, 4
  %v61.i.267.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %DescriptorHeapBufferTable_UAV_rawbuf336, i32 %v59.i.265.i.i, i32 undef) #2
  %v62.i.268.i.i = extractvalue %dx.types.ResRet.i32 %v61.i.267.i.i, 0
  %v63.i.269.i.i = add i32 %v62.i.268.i.i, %v54.i.260.i.i
  %v64.i.270.i.i = add i32 %v54.i.260.i.i, 16
  %v65.i.271.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %DescriptorHeapBufferTable_UAV_rawbuf336, i32 %v64.i.270.i.i, i32 undef) #2
  %v66.i.272.i.i = extractvalue %dx.types.ResRet.i32 %v65.i.271.i.i, 0
  %v67.i.i.i = extractvalue %dx.types.ResRet.i32 %v65.i.271.i.i, 1
  %v68.i.i.i = extractvalue %dx.types.ResRet.i32 %v65.i.271.i.i, 2
  %v69.i.i.i = add i32 %v54.i.260.i.i, 32
  %v70.i.273.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %DescriptorHeapBufferTable_UAV_rawbuf336, i32 %v69.i.i.i, i32 undef) #2
  %v71.i.i.i = extractvalue %dx.types.ResRet.i32 %v70.i.273.i.i, 0
  %v72.i.274.i.i = extractvalue %dx.types.ResRet.i32 %v70.i.273.i.i, 1
  %v73.i.275.i.i = extractvalue %dx.types.ResRet.i32 %v70.i.273.i.i, 2
  %v74.i.276.i.i = bitcast i32 %v66.i.272.i.i to float
  %v75.i.277.i.i = bitcast i32 %v67.i.i.i to float
  %v76.i.278.i.i = bitcast i32 %v68.i.i.i to float
  %v77.i.279.i.i = bitcast i32 %v71.i.i.i to float
  %v78.i.280.i.i = bitcast i32 %v72.i.274.i.i to float
  %v79.i.281.i.i = bitcast i32 %v73.i.275.i.i to float
  %v80.i.282.i.i = fsub fast float %v74.i.276.i.i, %64
  %v81.i.283.i.i = fmul fast float %v80.i.282.i.i, %.i0.i.238.i.i
  %v82.i.284.i.i = fsub fast float %v75.i.277.i.i, %65
  %v83.i.285.i.i = fmul fast float %v82.i.284.i.i, %.i1.i.239.i.i
  %v84.i.286.i.i = fsub fast float %v76.i.278.i.i, %66
  %v85.i.287.i.i = fmul fast float %v84.i.286.i.i, %.i2.i.240.i.i
  %FAbs238.i.i.i = call float @dx.op.unary.f32(i32 6, float %.i0.i.238.i.i) #2
  %FAbs239.i.i.i = call float @dx.op.unary.f32(i32 6, float %.i1.i.239.i.i) #2
  %FAbs240.i.i.i = call float @dx.op.unary.f32(i32 6, float %.i2.i.240.i.i) #2
  %.i0305.i.i.i = fmul fast float %v77.i.279.i.i, %FAbs238.i.i.i
  %.i1306.i.i.i = fmul fast float %FAbs239.i.i.i, %v78.i.280.i.i
  %.i2307.i.i.i = fmul fast float %FAbs240.i.i.i, %v79.i.281.i.i
  %.i0308.i.i.i = fadd fast float %.i0305.i.i.i, %v81.i.283.i.i
  %.i1309.i.i.i = fadd fast float %.i1306.i.i.i, %v83.i.285.i.i
  %.i2310.i.i.i = fadd fast float %.i2307.i.i.i, %v85.i.287.i.i
  %.i0314.i.i.i = fsub fast float %v81.i.283.i.i, %.i0305.i.i.i
  %.i1315.i.i.i = fsub fast float %v83.i.285.i.i, %.i1306.i.i.i
  %.i2316.i.i.i = fsub fast float %v85.i.287.i.i, %.i2307.i.i.i
  %FMax277.i.i.i = call float @dx.op.binary.f32(i32 35, float %.i0314.i.i.i, float %.i1315.i.i.i) #2
  %FMax276.i.i.i = call float @dx.op.binary.f32(i32 35, float %FMax277.i.i.i, float %.i2316.i.i.i) #2
  %FMin275.i.i.i = call float @dx.op.binary.f32(i32 36, float %.i0308.i.i.i, float %.i1309.i.i.i) #2
  %FMin274.i.i.i = call float @dx.op.binary.f32(i32 36, float %FMin275.i.i.i, float %.i2310.i.i.i) #2
  %FMax273.i.i.i = call float @dx.op.binary.f32(i32 35, float %FMax276.i.i.i, float 0.000000e+00) #2
  %FMin272.i.i.i = call float @dx.op.binary.f32(i32 36, float %FMin274.i.i.i, float %71) #2
  %v86.i.288.i.i = fcmp fast olt float %FMax273.i.i.i, %FMin272.i.i.i
  br i1 %v86.i.288.i.i, label %.lr.ph.preheader.critedge.i.i.i, label %._crit_edge.i.289.i.i

._crit_edge.i.289.i.i:                            ; preds = %108, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i"
  %.i037 = phi float [ %.i0, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i" ], [ %.i049, %108 ]
  %.i138 = phi float [ %.i1, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i" ], [ %.i150, %108 ]
  %.i239 = phi float [ %.i2, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i" ], [ %.i251, %108 ]
  %.i340 = phi float [ %.i3, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i" ], [ %.i352, %108 ]
  %.i441 = phi float [ %.i4, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i" ], [ %.i453, %108 ]
  %.i542 = phi float [ %.i5, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i" ], [ %.i554, %108 ]
  %.i643 = phi float [ %.i6, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i" ], [ %.i655, %108 ]
  %.i744 = phi float [ %.i7, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i" ], [ %.i756, %108 ]
  %.i845 = phi float [ %.i8, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i" ], [ %.i857, %108 ]
  %.i946 = phi float [ %.i9, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i" ], [ %.i958, %108 ]
  %.i1047 = phi float [ %.i10, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i" ], [ %.i1059, %108 ]
  %.i1148 = phi float [ %.i11, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i" ], [ %.i1160, %108 ]
  %.i061 = phi float [ %.i013, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i" ], [ %.i073, %108 ]
  %.i162 = phi float [ %.i114, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i" ], [ %.i174, %108 ]
  %.i263 = phi float [ %.i215, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i" ], [ %.i275, %108 ]
  %.i364 = phi float [ %.i316, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i" ], [ %.i376, %108 ]
  %.i465 = phi float [ %.i417, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i" ], [ %.i477, %108 ]
  %.i566 = phi float [ %.i518, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i" ], [ %.i578, %108 ]
  %.i667 = phi float [ %.i619, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i" ], [ %.i679, %108 ]
  %.i768 = phi float [ %.i720, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i" ], [ %.i780, %108 ]
  %.i869 = phi float [ %.i821, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i" ], [ %.i881, %108 ]
  %.i970 = phi float [ %.i922, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i" ], [ %.i982, %108 ]
  %.i1071 = phi float [ %.i1023, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i" ], [ %.i1083, %108 ]
  %.i1172 = phi float [ %.i1124, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i" ], [ %.i1184, %108 ]
  %75 = phi i32 [ %sub10.i.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i" ], [ %102, %108 ]
  %76 = phi i32 [ %sub6.i.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i" ], [ %103, %108 ]
  %77 = phi i32 [ -1, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i" ], [ %104, %108 ]
  %78 = phi i32 [ %10, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i" ], [ %105, %108 ]
  %79 = phi i32 [ %11, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i" ], [ %106, %108 ]
  %80 = phi float [ %71, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i" ], [ %107, %108 ]
  %v427.i.i.i = icmp eq i32 %77, -1
  br i1 %v427.i.i.i, label %86, label %81

; <label>:81                                      ; preds = %._crit_edge.i.289.i.i
  %v428.i.i.i = and i32 %arg0.int.i.i.i, 8
  %v429.i.i.i = icmp eq i32 %v428.i.i.i, 0
  br i1 %v429.i.i.i, label %85, label %"\01?Traverse@@YAHIIII@Z.exit.i.i.i"

"\01?Traverse@@YAHIIII@Z.exit.i.i.i":             ; preds = %81, %86, %85
  %stateID.i.0.i.i.i = phi i32 [ %v436.i.i.i, %85 ], [ %v442.i.i.i, %86 ], [ 0, %81 ]
  %v443.i.i.i = icmp eq i32 %stateID.i.0.i.i.i, 0
  br i1 %v443.i.i.i, label %82, label %remat_begin.i.290.i.i

remat_begin.i.290.i.i:                            ; preds = %"\01?Traverse@@YAHIIII@Z.exit.i.i.i"
  %arrayidx.i.67.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %sub.i.137.i.i.i
  store i32 1006, i32* %arrayidx.i.67.i.i.i
  %add.i.63.i.i.i = add nsw i32 %sub.i.137.i.i.i, 4
  %arrayidx.i.65.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.63.i.i.i
  store i32 %9, i32* %arrayidx.i.65.i.i.i
  br label %while.cond.i

; <label>:82                                      ; preds = %"\01?Traverse@@YAHIIII@Z.exit.i.i.i"
  %add.i.52.i.292.i.i = add nsw i32 %6, -1
  %arrayidx.i.54.i.294.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.52.i.292.i.i
  %83 = load i32, i32* %arrayidx.i.54.i.294.i.i, align 4
  %add2.i.57.i.i.i = add nsw i32 %6, -2
  %arrayidx4.i.59.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add2.i.57.i.i.i
  %84 = load i32, i32* %arrayidx4.i.59.i.i.i, align 4
  %arrayidx.i.49.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %6
  %ret.stateId.i.295.i.i = load i32, i32* %arrayidx.i.49.i.i.i
  br label %while.cond.i

; <label>:85                                      ; preds = %81
  %v431.i.i.i = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %Constants_cbuffer, i32 0) #2
  %v432.i.i.i = extractvalue %dx.types.CBufRet.i32 %v431.i.i.i, 2
  %v433.i.i.i = mul i32 %v432.i.i.i, %78
  %v435.i.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %HitGroupShaderTable_texture_rawbuf, i32 %v433.i.i.i, i32 undef) #2
  %v436.i.i.i = extractvalue %dx.types.ResRet.i32 %v435.i.i.i, 0
  br label %"\01?Traverse@@YAHIIII@Z.exit.i.i.i"

; <label>:86                                      ; preds = %._crit_edge.i.289.i.i
  %v437.i.i.i = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %Constants_cbuffer, i32 0) #2
  %v438.i.i.i = extractvalue %dx.types.CBufRet.i32 %v437.i.i.i, 3
  %v439.i.i.i = mul i32 %v438.i.i.i, %arg4.int.i.i.i
  %v441.i.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %MissShaderTable_texture_rawbuf, i32 %v439.i.i.i, i32 undef) #2
  %v442.i.i.i = extractvalue %dx.types.ResRet.i32 %v441.i.i.i, 0
  br label %"\01?Traverse@@YAHIIII@Z.exit.i.i.i"

.lr.ph.preheader.critedge.i.i.i:                  ; preds = %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i"
  %v87.i.296.i.i = getelementptr [2048 x i32], [2048 x i32] addrspace(3)* @"\01?stack@@3PAIA", i32 0, i32 %groupIndex.i
  store i32 0, i32 addrspace(3)* %v87.i.296.i.i, align 4, !tbaa !105, !noalias !107
  store i32 1, i32* %v56.i.262.i.i, align 4, !tbaa !105
  %v88.i.297.i.i = getelementptr inbounds [3 x float], [3 x float]* %v14.i.31.i.i, i32 0, i32 0
  %v89.i.298.i.i = getelementptr inbounds [3 x float], [3 x float]* %v14.i.31.i.i, i32 0, i32 1
  %v90.i.299.i.i = getelementptr inbounds [3 x float], [3 x float]* %v14.i.31.i.i, i32 0, i32 2
  %v91.i.300.i.i = getelementptr inbounds [2 x i32], [2 x i32]* %nodesToProcess.i.i.i.i.i, i32 0, i32 1
  %v92.i.301.i.i = getelementptr inbounds [3 x float], [3 x float]* %v13.i.30.i.i, i32 0, i32 0
  %v93.i.302.i.i = getelementptr inbounds [3 x float], [3 x float]* %v13.i.30.i.i, i32 0, i32 1
  %v94.i.303.i.i = getelementptr inbounds [3 x float], [3 x float]* %v13.i.30.i.i, i32 0, i32 2
  %v95.i.304.i.i = getelementptr inbounds [3 x float], [3 x float]* %v11.i.28.i.i, i32 0, i32 0
  %v96.i.305.i.i = getelementptr inbounds [3 x float], [3 x float]* %v11.i.28.i.i, i32 0, i32 1
  %v97.i.306.i.i = getelementptr inbounds [3 x float], [3 x float]* %v11.i.28.i.i, i32 0, i32 2
  %v98.i.307.i.i = getelementptr inbounds [3 x float], [3 x float]* %v12.i.29.i.i, i32 0, i32 0
  %v99.i.308.i.i = getelementptr inbounds [3 x float], [3 x float]* %v12.i.29.i.i, i32 0, i32 1
  %v100.i.309.i.i = getelementptr inbounds [3 x float], [3 x float]* %v12.i.29.i.i, i32 0, i32 2
  br label %.lr.ph.i.i.i

.lr.ph.i.i.i:                                     ; preds = %108, %.lr.ph.preheader.critedge.i.i.i
  %.i085 = phi float [ %.i0, %.lr.ph.preheader.critedge.i.i.i ], [ %.i049, %108 ]
  %.i186 = phi float [ %.i1, %.lr.ph.preheader.critedge.i.i.i ], [ %.i150, %108 ]
  %.i287 = phi float [ %.i2, %.lr.ph.preheader.critedge.i.i.i ], [ %.i251, %108 ]
  %.i388 = phi float [ %.i3, %.lr.ph.preheader.critedge.i.i.i ], [ %.i352, %108 ]
  %.i489 = phi float [ %.i4, %.lr.ph.preheader.critedge.i.i.i ], [ %.i453, %108 ]
  %.i590 = phi float [ %.i5, %.lr.ph.preheader.critedge.i.i.i ], [ %.i554, %108 ]
  %.i691 = phi float [ %.i6, %.lr.ph.preheader.critedge.i.i.i ], [ %.i655, %108 ]
  %.i792 = phi float [ %.i7, %.lr.ph.preheader.critedge.i.i.i ], [ %.i756, %108 ]
  %.i893 = phi float [ %.i8, %.lr.ph.preheader.critedge.i.i.i ], [ %.i857, %108 ]
  %.i994 = phi float [ %.i9, %.lr.ph.preheader.critedge.i.i.i ], [ %.i958, %108 ]
  %.i1095 = phi float [ %.i10, %.lr.ph.preheader.critedge.i.i.i ], [ %.i1059, %108 ]
  %.i1196 = phi float [ %.i11, %.lr.ph.preheader.critedge.i.i.i ], [ %.i1160, %108 ]
  %.i097 = phi float [ %.i013, %.lr.ph.preheader.critedge.i.i.i ], [ %.i073, %108 ]
  %.i198 = phi float [ %.i114, %.lr.ph.preheader.critedge.i.i.i ], [ %.i174, %108 ]
  %.i299 = phi float [ %.i215, %.lr.ph.preheader.critedge.i.i.i ], [ %.i275, %108 ]
  %.i3100 = phi float [ %.i316, %.lr.ph.preheader.critedge.i.i.i ], [ %.i376, %108 ]
  %.i4101 = phi float [ %.i417, %.lr.ph.preheader.critedge.i.i.i ], [ %.i477, %108 ]
  %.i5102 = phi float [ %.i518, %.lr.ph.preheader.critedge.i.i.i ], [ %.i578, %108 ]
  %.i6103 = phi float [ %.i619, %.lr.ph.preheader.critedge.i.i.i ], [ %.i679, %108 ]
  %.i7104 = phi float [ %.i720, %.lr.ph.preheader.critedge.i.i.i ], [ %.i780, %108 ]
  %.i8105 = phi float [ %.i821, %.lr.ph.preheader.critedge.i.i.i ], [ %.i881, %108 ]
  %.i9106 = phi float [ %.i922, %.lr.ph.preheader.critedge.i.i.i ], [ %.i982, %108 ]
  %.i10107 = phi float [ %.i1023, %.lr.ph.preheader.critedge.i.i.i ], [ %.i1083, %108 ]
  %.i11108 = phi float [ %.i1124, %.lr.ph.preheader.critedge.i.i.i ], [ %.i1184, %108 ]
  %87 = phi i32 [ %sub10.i.i.i.i, %.lr.ph.preheader.critedge.i.i.i ], [ %102, %108 ]
  %88 = phi i32 [ %sub6.i.i.i.i, %.lr.ph.preheader.critedge.i.i.i ], [ %103, %108 ]
  %89 = phi i32 [ -1, %.lr.ph.preheader.critedge.i.i.i ], [ %104, %108 ]
  %90 = phi i32 [ %10, %.lr.ph.preheader.critedge.i.i.i ], [ %105, %108 ]
  %91 = phi i32 [ %11, %.lr.ph.preheader.critedge.i.i.i ], [ %106, %108 ]
  %92 = phi float [ %71, %.lr.ph.preheader.critedge.i.i.i ], [ %107, %108 ]
  %resultTriId.i.i.0188.i.i.i = phi i32 [ undef, %.lr.ph.preheader.critedge.i.i.i ], [ %resultTriId.i.i.2.i.i.i, %108 ]
  %resultBary.i.i.0187.i0.i.i.i = phi float [ undef, %.lr.ph.preheader.critedge.i.i.i ], [ %resultBary.i.i.2.i0.i.i.i, %108 ]
  %resultBary.i.i.0187.i1.i.i.i = phi float [ undef, %.lr.ph.preheader.critedge.i.i.i ], [ %resultBary.i.i.2.i1.i.i.i, %108 ]
  %stackPointer.i.i.1185.i.i.i = phi i32 [ 1, %.lr.ph.preheader.critedge.i.i.i ], [ %stackPointer.i.i.3.i.i.i, %108 ]
  %instId.i.i.0184.i.i.i = phi i32 [ 0, %.lr.ph.preheader.critedge.i.i.i ], [ %instId.i.i.2.i.i.i, %108 ]
  %instOffset.i.i.0183.i.i.i = phi i32 [ 0, %.lr.ph.preheader.critedge.i.i.i ], [ %instOffset.i.i.2.i.i.i, %108 ]
  %instFlags.i.i.0182.i.i.i = phi i32 [ 0, %.lr.ph.preheader.critedge.i.i.i ], [ %instFlags.i.i.2.i.i.i, %108 ]
  %instIdx.i.i.0181.i.i.i = phi i32 [ 0, %.lr.ph.preheader.critedge.i.i.i ], [ %instIdx.i.i.2.i.i.i, %108 ]
  %currentGpuVA.i.i.0180.i0.i.i.i = phi i32 [ %v54.i.260.i.i, %.lr.ph.preheader.critedge.i.i.i ], [ %v423.i.i.i, %108 ]
  %currentGpuVA.i.i.0180.i1.i.i.i = phi i32 [ %v55.i.261.i.i, %.lr.ph.preheader.critedge.i.i.i ], [ %v424.i.i.i, %108 ]
  %currentBVHIndex.i.i.0179.i.i.i = phi i32 [ 0, %.lr.ph.preheader.critedge.i.i.i ], [ %v421.i.i.i, %108 ]
  %CurrentWorldToObject.i.i168.0178.i0.i.i.i = phi float [ 0.000000e+00, %.lr.ph.preheader.critedge.i.i.i ], [ %CurrentWorldToObject.i.i168.2.i0.i.i.i, %108 ]
  %CurrentWorldToObject.i.i168.0178.i1.i.i.i = phi float [ 0.000000e+00, %.lr.ph.preheader.critedge.i.i.i ], [ %CurrentWorldToObject.i.i168.2.i1.i.i.i, %108 ]
  %CurrentWorldToObject.i.i168.0178.i2.i.i.i = phi float [ 0.000000e+00, %.lr.ph.preheader.critedge.i.i.i ], [ %CurrentWorldToObject.i.i168.2.i2.i.i.i, %108 ]
  %CurrentWorldToObject.i.i168.0178.i3.i.i.i = phi float [ 0.000000e+00, %.lr.ph.preheader.critedge.i.i.i ], [ %CurrentWorldToObject.i.i168.2.i3.i.i.i, %108 ]
  %CurrentWorldToObject.i.i168.0178.i4.i.i.i = phi float [ 0.000000e+00, %.lr.ph.preheader.critedge.i.i.i ], [ %CurrentWorldToObject.i.i168.2.i4.i.i.i, %108 ]
  %CurrentWorldToObject.i.i168.0178.i5.i.i.i = phi float [ 0.000000e+00, %.lr.ph.preheader.critedge.i.i.i ], [ %CurrentWorldToObject.i.i168.2.i5.i.i.i, %108 ]
  %CurrentWorldToObject.i.i168.0178.i6.i.i.i = phi float [ 0.000000e+00, %.lr.ph.preheader.critedge.i.i.i ], [ %CurrentWorldToObject.i.i168.2.i6.i.i.i, %108 ]
  %CurrentWorldToObject.i.i168.0178.i7.i.i.i = phi float [ 0.000000e+00, %.lr.ph.preheader.critedge.i.i.i ], [ %CurrentWorldToObject.i.i168.2.i7.i.i.i, %108 ]
  %CurrentWorldToObject.i.i168.0178.i8.i.i.i = phi float [ 0.000000e+00, %.lr.ph.preheader.critedge.i.i.i ], [ %CurrentWorldToObject.i.i168.2.i8.i.i.i, %108 ]
  %CurrentWorldToObject.i.i168.0178.i9.i.i.i = phi float [ 0.000000e+00, %.lr.ph.preheader.critedge.i.i.i ], [ %CurrentWorldToObject.i.i168.2.i9.i.i.i, %108 ]
  %CurrentWorldToObject.i.i168.0178.i10.i.i.i = phi float [ 0.000000e+00, %.lr.ph.preheader.critedge.i.i.i ], [ %CurrentWorldToObject.i.i168.2.i10.i.i.i, %108 ]
  %CurrentWorldToObject.i.i168.0178.i11.i.i.i = phi float [ 0.000000e+00, %.lr.ph.preheader.critedge.i.i.i ], [ %CurrentWorldToObject.i.i168.2.i11.i.i.i, %108 ]
  %CurrentObjectToWorld.i.i167.0177.i0.i.i.i = phi float [ 0.000000e+00, %.lr.ph.preheader.critedge.i.i.i ], [ %CurrentObjectToWorld.i.i167.2.i0.i.i.i, %108 ]
  %CurrentObjectToWorld.i.i167.0177.i1.i.i.i = phi float [ 0.000000e+00, %.lr.ph.preheader.critedge.i.i.i ], [ %CurrentObjectToWorld.i.i167.2.i1.i.i.i, %108 ]
  %CurrentObjectToWorld.i.i167.0177.i2.i.i.i = phi float [ 0.000000e+00, %.lr.ph.preheader.critedge.i.i.i ], [ %CurrentObjectToWorld.i.i167.2.i2.i.i.i, %108 ]
  %CurrentObjectToWorld.i.i167.0177.i3.i.i.i = phi float [ 0.000000e+00, %.lr.ph.preheader.critedge.i.i.i ], [ %CurrentObjectToWorld.i.i167.2.i3.i.i.i, %108 ]
  %CurrentObjectToWorld.i.i167.0177.i4.i.i.i = phi float [ 0.000000e+00, %.lr.ph.preheader.critedge.i.i.i ], [ %CurrentObjectToWorld.i.i167.2.i4.i.i.i, %108 ]
  %CurrentObjectToWorld.i.i167.0177.i5.i.i.i = phi float [ 0.000000e+00, %.lr.ph.preheader.critedge.i.i.i ], [ %CurrentObjectToWorld.i.i167.2.i5.i.i.i, %108 ]
  %CurrentObjectToWorld.i.i167.0177.i6.i.i.i = phi float [ 0.000000e+00, %.lr.ph.preheader.critedge.i.i.i ], [ %CurrentObjectToWorld.i.i167.2.i6.i.i.i, %108 ]
  %CurrentObjectToWorld.i.i167.0177.i7.i.i.i = phi float [ 0.000000e+00, %.lr.ph.preheader.critedge.i.i.i ], [ %CurrentObjectToWorld.i.i167.2.i7.i.i.i, %108 ]
  %CurrentObjectToWorld.i.i167.0177.i8.i.i.i = phi float [ 0.000000e+00, %.lr.ph.preheader.critedge.i.i.i ], [ %CurrentObjectToWorld.i.i167.2.i8.i.i.i, %108 ]
  %CurrentObjectToWorld.i.i167.0177.i9.i.i.i = phi float [ 0.000000e+00, %.lr.ph.preheader.critedge.i.i.i ], [ %CurrentObjectToWorld.i.i167.2.i9.i.i.i, %108 ]
  %CurrentObjectToWorld.i.i167.0177.i10.i.i.i = phi float [ 0.000000e+00, %.lr.ph.preheader.critedge.i.i.i ], [ %CurrentObjectToWorld.i.i167.2.i10.i.i.i, %108 ]
  %CurrentObjectToWorld.i.i167.0177.i11.i.i.i = phi float [ 0.000000e+00, %.lr.ph.preheader.critedge.i.i.i ], [ %CurrentObjectToWorld.i.i167.2.i11.i.i.i, %108 ]
  %.0174176.i0.i.i.i = phi float [ undef, %.lr.ph.preheader.critedge.i.i.i ], [ %.3.i0.i.i.i, %108 ]
  %.0174176.i1.i.i.i = phi float [ undef, %.lr.ph.preheader.critedge.i.i.i ], [ %.3.i1.i.i.i, %108 ]
  %.phi.trans.insert.i.i.i = getelementptr inbounds [2 x i32], [2 x i32]* %nodesToProcess.i.i.i.i.i, i32 0, i32 %currentBVHIndex.i.i.0179.i.i.i
  %.pre.i.i.i = load i32, i32* %.phi.trans.insert.i.i.i, align 4, !tbaa !105
  br label %._crit_edge.2.i.i.i

._crit_edge.2.i.i.i:                              ; preds = %._crit_edge.1.i.i.i, %.lr.ph.i.i.i
  %.i0109 = phi float [ %.i085, %.lr.ph.i.i.i ], [ %.i049, %._crit_edge.1.i.i.i ]
  %.i1110 = phi float [ %.i186, %.lr.ph.i.i.i ], [ %.i150, %._crit_edge.1.i.i.i ]
  %.i2111 = phi float [ %.i287, %.lr.ph.i.i.i ], [ %.i251, %._crit_edge.1.i.i.i ]
  %.i3112 = phi float [ %.i388, %.lr.ph.i.i.i ], [ %.i352, %._crit_edge.1.i.i.i ]
  %.i4113 = phi float [ %.i489, %.lr.ph.i.i.i ], [ %.i453, %._crit_edge.1.i.i.i ]
  %.i5114 = phi float [ %.i590, %.lr.ph.i.i.i ], [ %.i554, %._crit_edge.1.i.i.i ]
  %.i6115 = phi float [ %.i691, %.lr.ph.i.i.i ], [ %.i655, %._crit_edge.1.i.i.i ]
  %.i7116 = phi float [ %.i792, %.lr.ph.i.i.i ], [ %.i756, %._crit_edge.1.i.i.i ]
  %.i8117 = phi float [ %.i893, %.lr.ph.i.i.i ], [ %.i857, %._crit_edge.1.i.i.i ]
  %.i9118 = phi float [ %.i994, %.lr.ph.i.i.i ], [ %.i958, %._crit_edge.1.i.i.i ]
  %.i10119 = phi float [ %.i1095, %.lr.ph.i.i.i ], [ %.i1059, %._crit_edge.1.i.i.i ]
  %.i11120 = phi float [ %.i1196, %.lr.ph.i.i.i ], [ %.i1160, %._crit_edge.1.i.i.i ]
  %.i0121 = phi float [ %.i097, %.lr.ph.i.i.i ], [ %.i073, %._crit_edge.1.i.i.i ]
  %.i1122 = phi float [ %.i198, %.lr.ph.i.i.i ], [ %.i174, %._crit_edge.1.i.i.i ]
  %.i2123 = phi float [ %.i299, %.lr.ph.i.i.i ], [ %.i275, %._crit_edge.1.i.i.i ]
  %.i3124 = phi float [ %.i3100, %.lr.ph.i.i.i ], [ %.i376, %._crit_edge.1.i.i.i ]
  %.i4125 = phi float [ %.i4101, %.lr.ph.i.i.i ], [ %.i477, %._crit_edge.1.i.i.i ]
  %.i5126 = phi float [ %.i5102, %.lr.ph.i.i.i ], [ %.i578, %._crit_edge.1.i.i.i ]
  %.i6127 = phi float [ %.i6103, %.lr.ph.i.i.i ], [ %.i679, %._crit_edge.1.i.i.i ]
  %.i7128 = phi float [ %.i7104, %.lr.ph.i.i.i ], [ %.i780, %._crit_edge.1.i.i.i ]
  %.i8129 = phi float [ %.i8105, %.lr.ph.i.i.i ], [ %.i881, %._crit_edge.1.i.i.i ]
  %.i9130 = phi float [ %.i9106, %.lr.ph.i.i.i ], [ %.i982, %._crit_edge.1.i.i.i ]
  %.i10131 = phi float [ %.i10107, %.lr.ph.i.i.i ], [ %.i1083, %._crit_edge.1.i.i.i ]
  %.i11132 = phi float [ %.i11108, %.lr.ph.i.i.i ], [ %.i1184, %._crit_edge.1.i.i.i ]
  %93 = phi i32 [ %87, %.lr.ph.i.i.i ], [ %102, %._crit_edge.1.i.i.i ]
  %94 = phi i32 [ %88, %.lr.ph.i.i.i ], [ %103, %._crit_edge.1.i.i.i ]
  %95 = phi i32 [ %89, %.lr.ph.i.i.i ], [ %104, %._crit_edge.1.i.i.i ]
  %96 = phi i32 [ %90, %.lr.ph.i.i.i ], [ %105, %._crit_edge.1.i.i.i ]
  %97 = phi i32 [ %91, %.lr.ph.i.i.i ], [ %106, %._crit_edge.1.i.i.i ]
  %98 = phi float [ %92, %.lr.ph.i.i.i ], [ %107, %._crit_edge.1.i.i.i ]
  %v102.i.311.i.i = phi i32 [ %.pre.i.i.i, %.lr.ph.i.i.i ], [ %v419.i.i.i, %._crit_edge.1.i.i.i ]
  %.1.i0.i.i.i = phi float [ %.0174176.i0.i.i.i, %.lr.ph.i.i.i ], [ %.3.i0.i.i.i, %._crit_edge.1.i.i.i ]
  %.1.i1.i.i.i = phi float [ %.0174176.i1.i.i.i, %.lr.ph.i.i.i ], [ %.3.i1.i.i.i, %._crit_edge.1.i.i.i ]
  %currentRayData.i.i.0.1.i0.i.i.i = phi float [ %64, %.lr.ph.i.i.i ], [ %currentRayData.i.i.0.2.i0.i.i.i, %._crit_edge.1.i.i.i ]
  %currentRayData.i.i.0.1.i1.i.i.i = phi float [ %65, %.lr.ph.i.i.i ], [ %currentRayData.i.i.0.2.i1.i.i.i, %._crit_edge.1.i.i.i ]
  %currentRayData.i.i.0.1.i2.i.i.i = phi float [ %66, %.lr.ph.i.i.i ], [ %currentRayData.i.i.0.2.i2.i.i.i, %._crit_edge.1.i.i.i ]
  %currentRayData.i.i.1.1.i0.i.i.i = phi float [ %68, %.lr.ph.i.i.i ], [ %currentRayData.i.i.1.2.i0.i.i.i, %._crit_edge.1.i.i.i ]
  %currentRayData.i.i.1.1.i1.i.i.i = phi float [ %69, %.lr.ph.i.i.i ], [ %currentRayData.i.i.1.2.i1.i.i.i, %._crit_edge.1.i.i.i ]
  %currentRayData.i.i.1.1.i2.i.i.i = phi float [ %70, %.lr.ph.i.i.i ], [ %currentRayData.i.i.1.2.i2.i.i.i, %._crit_edge.1.i.i.i ]
  %currentRayData.i.i.2.1.i0.i.i.i = phi float [ %.i0.i.238.i.i, %.lr.ph.i.i.i ], [ %currentRayData.i.i.2.2.i0.i.i.i, %._crit_edge.1.i.i.i ]
  %currentRayData.i.i.2.1.i1.i.i.i = phi float [ %.i1.i.239.i.i, %.lr.ph.i.i.i ], [ %currentRayData.i.i.2.2.i1.i.i.i, %._crit_edge.1.i.i.i ]
  %currentRayData.i.i.2.1.i2.i.i.i = phi float [ %.i2.i.240.i.i, %.lr.ph.i.i.i ], [ %currentRayData.i.i.2.2.i2.i.i.i, %._crit_edge.1.i.i.i ]
  %currentRayData.i.i.3.1.i0.i.i.i = phi float [ %.i0296.i.i.i, %.lr.ph.i.i.i ], [ %currentRayData.i.i.3.2.i0.i.i.i, %._crit_edge.1.i.i.i ]
  %currentRayData.i.i.3.1.i1.i.i.i = phi float [ %.i1297.i.i.i, %.lr.ph.i.i.i ], [ %currentRayData.i.i.3.2.i1.i.i.i, %._crit_edge.1.i.i.i ]
  %currentRayData.i.i.3.1.i2.i.i.i = phi float [ %.i2298.i.i.i, %.lr.ph.i.i.i ], [ %currentRayData.i.i.3.2.i2.i.i.i, %._crit_edge.1.i.i.i ]
  %currentRayData.i.i.4.1.i0.i.i.i = phi float [ %v48.i.254.i.i, %.lr.ph.i.i.i ], [ %currentRayData.i.i.4.2.i0.i.i.i, %._crit_edge.1.i.i.i ]
  %currentRayData.i.i.4.1.i1.i.i.i = phi float [ %v51.i.257.i.i, %.lr.ph.i.i.i ], [ %currentRayData.i.i.4.2.i1.i.i.i, %._crit_edge.1.i.i.i ]
  %currentRayData.i.i.4.1.i2.i.i.i = phi float [ %v52.i.258.i.i, %.lr.ph.i.i.i ], [ %currentRayData.i.i.4.2.i2.i.i.i, %._crit_edge.1.i.i.i ]
  %currentRayData.i.i.5.1.i0.i.i.i = phi i32 [ %worldRayData.i.i.5.0.i0.i.i.i, %.lr.ph.i.i.i ], [ %currentRayData.i.i.5.2.i0.i.i.i, %._crit_edge.1.i.i.i ]
  %currentRayData.i.i.5.1.i1.i.i.i = phi i32 [ %worldRayData.i.i.5.0.i1.i.i.i, %.lr.ph.i.i.i ], [ %currentRayData.i.i.5.2.i1.i.i.i, %._crit_edge.1.i.i.i ]
  %currentRayData.i.i.5.1.i2.i.i.i = phi i32 [ %.0171.i.i.i, %.lr.ph.i.i.i ], [ %currentRayData.i.i.5.2.i2.i.i.i, %._crit_edge.1.i.i.i ]
  %CurrentObjectToWorld.i.i167.1.i0.i.i.i = phi float [ %CurrentObjectToWorld.i.i167.0177.i0.i.i.i, %.lr.ph.i.i.i ], [ %CurrentObjectToWorld.i.i167.2.i0.i.i.i, %._crit_edge.1.i.i.i ]
  %CurrentObjectToWorld.i.i167.1.i1.i.i.i = phi float [ %CurrentObjectToWorld.i.i167.0177.i1.i.i.i, %.lr.ph.i.i.i ], [ %CurrentObjectToWorld.i.i167.2.i1.i.i.i, %._crit_edge.1.i.i.i ]
  %CurrentObjectToWorld.i.i167.1.i2.i.i.i = phi float [ %CurrentObjectToWorld.i.i167.0177.i2.i.i.i, %.lr.ph.i.i.i ], [ %CurrentObjectToWorld.i.i167.2.i2.i.i.i, %._crit_edge.1.i.i.i ]
  %CurrentObjectToWorld.i.i167.1.i3.i.i.i = phi float [ %CurrentObjectToWorld.i.i167.0177.i3.i.i.i, %.lr.ph.i.i.i ], [ %CurrentObjectToWorld.i.i167.2.i3.i.i.i, %._crit_edge.1.i.i.i ]
  %CurrentObjectToWorld.i.i167.1.i4.i.i.i = phi float [ %CurrentObjectToWorld.i.i167.0177.i4.i.i.i, %.lr.ph.i.i.i ], [ %CurrentObjectToWorld.i.i167.2.i4.i.i.i, %._crit_edge.1.i.i.i ]
  %CurrentObjectToWorld.i.i167.1.i5.i.i.i = phi float [ %CurrentObjectToWorld.i.i167.0177.i5.i.i.i, %.lr.ph.i.i.i ], [ %CurrentObjectToWorld.i.i167.2.i5.i.i.i, %._crit_edge.1.i.i.i ]
  %CurrentObjectToWorld.i.i167.1.i6.i.i.i = phi float [ %CurrentObjectToWorld.i.i167.0177.i6.i.i.i, %.lr.ph.i.i.i ], [ %CurrentObjectToWorld.i.i167.2.i6.i.i.i, %._crit_edge.1.i.i.i ]
  %CurrentObjectToWorld.i.i167.1.i7.i.i.i = phi float [ %CurrentObjectToWorld.i.i167.0177.i7.i.i.i, %.lr.ph.i.i.i ], [ %CurrentObjectToWorld.i.i167.2.i7.i.i.i, %._crit_edge.1.i.i.i ]
  %CurrentObjectToWorld.i.i167.1.i8.i.i.i = phi float [ %CurrentObjectToWorld.i.i167.0177.i8.i.i.i, %.lr.ph.i.i.i ], [ %CurrentObjectToWorld.i.i167.2.i8.i.i.i, %._crit_edge.1.i.i.i ]
  %CurrentObjectToWorld.i.i167.1.i9.i.i.i = phi float [ %CurrentObjectToWorld.i.i167.0177.i9.i.i.i, %.lr.ph.i.i.i ], [ %CurrentObjectToWorld.i.i167.2.i9.i.i.i, %._crit_edge.1.i.i.i ]
  %CurrentObjectToWorld.i.i167.1.i10.i.i.i = phi float [ %CurrentObjectToWorld.i.i167.0177.i10.i.i.i, %.lr.ph.i.i.i ], [ %CurrentObjectToWorld.i.i167.2.i10.i.i.i, %._crit_edge.1.i.i.i ]
  %CurrentObjectToWorld.i.i167.1.i11.i.i.i = phi float [ %CurrentObjectToWorld.i.i167.0177.i11.i.i.i, %.lr.ph.i.i.i ], [ %CurrentObjectToWorld.i.i167.2.i11.i.i.i, %._crit_edge.1.i.i.i ]
  %CurrentWorldToObject.i.i168.1.i0.i.i.i = phi float [ %CurrentWorldToObject.i.i168.0178.i0.i.i.i, %.lr.ph.i.i.i ], [ %CurrentWorldToObject.i.i168.2.i0.i.i.i, %._crit_edge.1.i.i.i ]
  %CurrentWorldToObject.i.i168.1.i1.i.i.i = phi float [ %CurrentWorldToObject.i.i168.0178.i1.i.i.i, %.lr.ph.i.i.i ], [ %CurrentWorldToObject.i.i168.2.i1.i.i.i, %._crit_edge.1.i.i.i ]
  %CurrentWorldToObject.i.i168.1.i2.i.i.i = phi float [ %CurrentWorldToObject.i.i168.0178.i2.i.i.i, %.lr.ph.i.i.i ], [ %CurrentWorldToObject.i.i168.2.i2.i.i.i, %._crit_edge.1.i.i.i ]
  %CurrentWorldToObject.i.i168.1.i3.i.i.i = phi float [ %CurrentWorldToObject.i.i168.0178.i3.i.i.i, %.lr.ph.i.i.i ], [ %CurrentWorldToObject.i.i168.2.i3.i.i.i, %._crit_edge.1.i.i.i ]
  %CurrentWorldToObject.i.i168.1.i4.i.i.i = phi float [ %CurrentWorldToObject.i.i168.0178.i4.i.i.i, %.lr.ph.i.i.i ], [ %CurrentWorldToObject.i.i168.2.i4.i.i.i, %._crit_edge.1.i.i.i ]
  %CurrentWorldToObject.i.i168.1.i5.i.i.i = phi float [ %CurrentWorldToObject.i.i168.0178.i5.i.i.i, %.lr.ph.i.i.i ], [ %CurrentWorldToObject.i.i168.2.i5.i.i.i, %._crit_edge.1.i.i.i ]
  %CurrentWorldToObject.i.i168.1.i6.i.i.i = phi float [ %CurrentWorldToObject.i.i168.0178.i6.i.i.i, %.lr.ph.i.i.i ], [ %CurrentWorldToObject.i.i168.2.i6.i.i.i, %._crit_edge.1.i.i.i ]
  %CurrentWorldToObject.i.i168.1.i7.i.i.i = phi float [ %CurrentWorldToObject.i.i168.0178.i7.i.i.i, %.lr.ph.i.i.i ], [ %CurrentWorldToObject.i.i168.2.i7.i.i.i, %._crit_edge.1.i.i.i ]
  %CurrentWorldToObject.i.i168.1.i8.i.i.i = phi float [ %CurrentWorldToObject.i.i168.0178.i8.i.i.i, %.lr.ph.i.i.i ], [ %CurrentWorldToObject.i.i168.2.i8.i.i.i, %._crit_edge.1.i.i.i ]
  %CurrentWorldToObject.i.i168.1.i9.i.i.i = phi float [ %CurrentWorldToObject.i.i168.0178.i9.i.i.i, %.lr.ph.i.i.i ], [ %CurrentWorldToObject.i.i168.2.i9.i.i.i, %._crit_edge.1.i.i.i ]
  %CurrentWorldToObject.i.i168.1.i10.i.i.i = phi float [ %CurrentWorldToObject.i.i168.0178.i10.i.i.i, %.lr.ph.i.i.i ], [ %CurrentWorldToObject.i.i168.2.i10.i.i.i, %._crit_edge.1.i.i.i ]
  %CurrentWorldToObject.i.i168.1.i11.i.i.i = phi float [ %CurrentWorldToObject.i.i168.0178.i11.i.i.i, %.lr.ph.i.i.i ], [ %CurrentWorldToObject.i.i168.2.i11.i.i.i, %._crit_edge.1.i.i.i ]
  %ResetMatrices.i.i.1.i.i.i = phi i32 [ 1, %.lr.ph.i.i.i ], [ %ResetMatrices.i.i.3.i.i.i, %._crit_edge.1.i.i.i ]
  %currentBVHIndex.i.i.1.i.i.i = phi i32 [ %currentBVHIndex.i.i.0179.i.i.i, %.lr.ph.i.i.i ], [ %currentBVHIndex.i.i.2.i.i.i, %._crit_edge.1.i.i.i ]
  %currentGpuVA.i.i.1.i0.i.i.i = phi i32 [ %currentGpuVA.i.i.0180.i0.i.i.i, %.lr.ph.i.i.i ], [ %currentGpuVA.i.i.2.i0.i.i.i, %._crit_edge.1.i.i.i ]
  %currentGpuVA.i.i.1.i1.i.i.i = phi i32 [ %currentGpuVA.i.i.0180.i1.i.i.i, %.lr.ph.i.i.i ], [ %currentGpuVA.i.i.2.i1.i.i.i, %._crit_edge.1.i.i.i ]
  %instIdx.i.i.1.i.i.i = phi i32 [ %instIdx.i.i.0181.i.i.i, %.lr.ph.i.i.i ], [ %instIdx.i.i.2.i.i.i, %._crit_edge.1.i.i.i ]
  %instFlags.i.i.1.i.i.i = phi i32 [ %instFlags.i.i.0182.i.i.i, %.lr.ph.i.i.i ], [ %instFlags.i.i.2.i.i.i, %._crit_edge.1.i.i.i ]
  %instOffset.i.i.1.i.i.i = phi i32 [ %instOffset.i.i.0183.i.i.i, %.lr.ph.i.i.i ], [ %instOffset.i.i.2.i.i.i, %._crit_edge.1.i.i.i ]
  %instId.i.i.1.i.i.i = phi i32 [ %instId.i.i.0184.i.i.i, %.lr.ph.i.i.i ], [ %instId.i.i.2.i.i.i, %._crit_edge.1.i.i.i ]
  %stackPointer.i.i.2.i.i.i = phi i32 [ %stackPointer.i.i.1185.i.i.i, %.lr.ph.i.i.i ], [ %stackPointer.i.i.3.i.i.i, %._crit_edge.1.i.i.i ]
  %resultBary.i.i.1.i0.i.i.i = phi float [ %resultBary.i.i.0187.i0.i.i.i, %.lr.ph.i.i.i ], [ %resultBary.i.i.2.i0.i.i.i, %._crit_edge.1.i.i.i ]
  %resultBary.i.i.1.i1.i.i.i = phi float [ %resultBary.i.i.0187.i1.i.i.i, %.lr.ph.i.i.i ], [ %resultBary.i.i.2.i1.i.i.i, %._crit_edge.1.i.i.i ]
  %resultTriId.i.i.1.i.i.i = phi i32 [ %resultTriId.i.i.0188.i.i.i, %.lr.ph.i.i.i ], [ %resultTriId.i.i.2.i.i.i, %._crit_edge.1.i.i.i ]
  %v103.i.312.i.i = add nsw i32 %stackPointer.i.i.2.i.i.i, -1
  %v104.i.313.i.i = shl i32 %v103.i.312.i.i, 6
  %v105.i.314.i.i = add i32 %v104.i.313.i.i, %groupIndex.i
  %v106.i.315.i.i = getelementptr [2048 x i32], [2048 x i32] addrspace(3)* @"\01?stack@@3PAIA", i32 0, i32 %v105.i.314.i.i
  %v107.i.316.i.i = load i32, i32 addrspace(3)* %v106.i.315.i.i, align 4, !tbaa !105, !noalias !110
  %v108.i.317.i.i = getelementptr inbounds [2 x i32], [2 x i32]* %nodesToProcess.i.i.i.i.i, i32 0, i32 %currentBVHIndex.i.i.1.i.i.i
  %v109.i.318.i.i = add i32 %v102.i.311.i.i, -1
  store i32 %v109.i.318.i.i, i32* %v108.i.317.i.i, align 4, !tbaa !105
  %99 = add i32 %currentGpuVA.i.i.1.i1.i.i.i, 0
  %DescriptorHeapBufferTable_UAV_rawbuf = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 1, i32 1, i32 %99, i1 true)
  %v112.i.321.i.i = add i32 %currentGpuVA.i.i.1.i0.i.i.i, 16
  %v113.i.322.i.i = shl i32 %v107.i.316.i.i, 5
  %v114.i.323.i.i = add i32 %v112.i.321.i.i, %v113.i.322.i.i
  %v116.i.325.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %DescriptorHeapBufferTable_UAV_rawbuf, i32 %v114.i.323.i.i, i32 undef) #2
  %v117.i.326.i.i = extractvalue %dx.types.ResRet.i32 %v116.i.325.i.i, 3
  %v118.i.327.i.i = add i32 %v114.i.323.i.i, 16
  %v119.i.328.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %DescriptorHeapBufferTable_UAV_rawbuf, i32 %v118.i.327.i.i, i32 undef) #2
  %v120.i.329.i.i = extractvalue %dx.types.ResRet.i32 %v119.i.328.i.i, 3
  %v121.i.330.i.i = icmp slt i32 %v117.i.326.i.i, 0
  br i1 %v121.i.330.i.i, label %110, label %100

; <label>:100                                     ; preds = %._crit_edge.2.i.i.i
  %v369.i.331.i.i = and i32 %v117.i.326.i.i, 16777215
  %v370.i.332.i.i = shl nuw nsw i32 %v369.i.331.i.i, 5
  %v371.i.333.i.i = add i32 %v112.i.321.i.i, %v370.i.332.i.i
  %v373.i.335.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %DescriptorHeapBufferTable_UAV_rawbuf, i32 %v371.i.333.i.i, i32 undef) #2
  %v374.i.336.i.i = extractvalue %dx.types.ResRet.i32 %v373.i.335.i.i, 0
  %v375.i.337.i.i = extractvalue %dx.types.ResRet.i32 %v373.i.335.i.i, 1
  %v376.i.338.i.i = extractvalue %dx.types.ResRet.i32 %v373.i.335.i.i, 2
  %v377.i.339.i.i = add i32 %v371.i.333.i.i, 16
  %v378.i.340.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %DescriptorHeapBufferTable_UAV_rawbuf, i32 %v377.i.339.i.i, i32 undef) #2
  %v379.i.341.i.i = extractvalue %dx.types.ResRet.i32 %v378.i.340.i.i, 0
  %v380.i.342.i.i = extractvalue %dx.types.ResRet.i32 %v378.i.340.i.i, 1
  %v381.i.343.i.i = extractvalue %dx.types.ResRet.i32 %v378.i.340.i.i, 2
  %v382.i.344.i.i = bitcast i32 %v374.i.336.i.i to float
  %v383.i.345.i.i = bitcast i32 %v375.i.337.i.i to float
  %v384.i.346.i.i = bitcast i32 %v376.i.338.i.i to float
  %v385.i.347.i.i = bitcast i32 %v379.i.341.i.i to float
  %v386.i.348.i.i = bitcast i32 %v380.i.342.i.i to float
  %v387.i.349.i.i = bitcast i32 %v381.i.343.i.i to float
  %v388.i.350.i.i = shl i32 %v120.i.329.i.i, 5
  %v389.i.351.i.i = add i32 %v112.i.321.i.i, %v388.i.350.i.i
  %v390.i.352.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %DescriptorHeapBufferTable_UAV_rawbuf, i32 %v389.i.351.i.i, i32 undef) #2
  %v391.i.353.i.i = extractvalue %dx.types.ResRet.i32 %v390.i.352.i.i, 0
  %v392.i.i.i = extractvalue %dx.types.ResRet.i32 %v390.i.352.i.i, 1
  %v393.i.i.i = extractvalue %dx.types.ResRet.i32 %v390.i.352.i.i, 2
  %v394.i.i.i = add i32 %v389.i.351.i.i, 16
  %v395.i.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %DescriptorHeapBufferTable_UAV_rawbuf, i32 %v394.i.i.i, i32 undef) #2
  %v396.i.i.i = extractvalue %dx.types.ResRet.i32 %v395.i.i.i, 0
  %v397.i.i.i = extractvalue %dx.types.ResRet.i32 %v395.i.i.i, 1
  %v398.i.i.i = extractvalue %dx.types.ResRet.i32 %v395.i.i.i, 2
  %v399.i.i.i = bitcast i32 %v391.i.353.i.i to float
  %v400.i.i.i = bitcast i32 %v392.i.i.i to float
  %v401.i.i.i = bitcast i32 %v393.i.i.i to float
  %v402.i.i.i = bitcast i32 %v396.i.i.i to float
  %v403.i.i.i = bitcast i32 %v397.i.i.i to float
  %v404.i.i.i = bitcast i32 %v398.i.i.i to float
  %.i0378.i.i.i = fmul fast float %v382.i.344.i.i, %currentRayData.i.i.2.1.i0.i.i.i
  %.i1379.i.i.i = fmul fast float %v383.i.345.i.i, %currentRayData.i.i.2.1.i1.i.i.i
  %.i2380.i.i.i = fmul fast float %v384.i.346.i.i, %currentRayData.i.i.2.1.i2.i.i.i
  %.i0381.i.i.i = fsub fast float %.i0378.i.i.i, %currentRayData.i.i.3.1.i0.i.i.i
  %.i1382.i.i.i = fsub fast float %.i1379.i.i.i, %currentRayData.i.i.3.1.i1.i.i.i
  %.i2383.i.i.i = fsub fast float %.i2380.i.i.i, %currentRayData.i.i.3.1.i2.i.i.i
  %FAbs232.i.i.i = call float @dx.op.unary.f32(i32 6, float %currentRayData.i.i.2.1.i0.i.i.i) #2
  %FAbs233.i.i.i = call float @dx.op.unary.f32(i32 6, float %currentRayData.i.i.2.1.i1.i.i.i) #2
  %FAbs234.i.i.i = call float @dx.op.unary.f32(i32 6, float %currentRayData.i.i.2.1.i2.i.i.i) #2
  %.i0384.i.i.i = fmul fast float %FAbs232.i.i.i, %v385.i.347.i.i
  %.i1385.i.i.i = fmul fast float %FAbs233.i.i.i, %v386.i.348.i.i
  %.i2386.i.i.i = fmul fast float %FAbs234.i.i.i, %v387.i.349.i.i
  %.i0387.i.i.i = fadd fast float %.i0384.i.i.i, %.i0381.i.i.i
  %.i1388.i.i.i = fadd fast float %.i1385.i.i.i, %.i1382.i.i.i
  %.i2389.i.i.i = fadd fast float %.i2386.i.i.i, %.i2383.i.i.i
  %.i0393.i.i.i = fsub fast float %.i0381.i.i.i, %.i0384.i.i.i
  %.i1394.i.i.i = fsub fast float %.i1382.i.i.i, %.i1385.i.i.i
  %.i2395.i.i.i = fsub fast float %.i2383.i.i.i, %.i2386.i.i.i
  %FMax271.i.i.i = call float @dx.op.binary.f32(i32 35, float %.i0393.i.i.i, float %.i1394.i.i.i) #2
  %FMax270.i.i.i = call float @dx.op.binary.f32(i32 35, float %FMax271.i.i.i, float %.i2395.i.i.i) #2
  %FMin269.i.i.i = call float @dx.op.binary.f32(i32 36, float %.i0387.i.i.i, float %.i1388.i.i.i) #2
  %FMin268.i.i.i = call float @dx.op.binary.f32(i32 36, float %FMin269.i.i.i, float %.i2389.i.i.i) #2
  %FMax266.i.i.i = call float @dx.op.binary.f32(i32 35, float %FMax270.i.i.i, float 0.000000e+00) #2
  %FMin265.i.i.i = call float @dx.op.binary.f32(i32 36, float %FMin268.i.i.i, float %98) #2
  %v405.i.i.i = fcmp fast olt float %FMax266.i.i.i, %FMin265.i.i.i
  %.i0396.i.i.i = fmul fast float %v399.i.i.i, %currentRayData.i.i.2.1.i0.i.i.i
  %.i1397.i.i.i = fmul fast float %v400.i.i.i, %currentRayData.i.i.2.1.i1.i.i.i
  %.i2398.i.i.i = fmul fast float %v401.i.i.i, %currentRayData.i.i.2.1.i2.i.i.i
  %.i0399.i.i.i = fsub fast float %.i0396.i.i.i, %currentRayData.i.i.3.1.i0.i.i.i
  %.i1400.i.i.i = fsub fast float %.i1397.i.i.i, %currentRayData.i.i.3.1.i1.i.i.i
  %.i2401.i.i.i = fsub fast float %.i2398.i.i.i, %currentRayData.i.i.3.1.i2.i.i.i
  %.i0402.i.i.i = fmul fast float %FAbs232.i.i.i, %v402.i.i.i
  %.i1403.i.i.i = fmul fast float %FAbs233.i.i.i, %v403.i.i.i
  %.i2404.i.i.i = fmul fast float %FAbs234.i.i.i, %v404.i.i.i
  %.i0405.i.i.i = fadd fast float %.i0402.i.i.i, %.i0399.i.i.i
  %.i1406.i.i.i = fadd fast float %.i1403.i.i.i, %.i1400.i.i.i
  %.i2407.i.i.i = fadd fast float %.i2404.i.i.i, %.i2401.i.i.i
  %.i0411.i.i.i = fsub fast float %.i0399.i.i.i, %.i0402.i.i.i
  %.i1412.i.i.i = fsub fast float %.i1400.i.i.i, %.i1403.i.i.i
  %.i2413.i.i.i = fsub fast float %.i2401.i.i.i, %.i2404.i.i.i
  %FMax264.i.i.i = call float @dx.op.binary.f32(i32 35, float %.i0411.i.i.i, float %.i1412.i.i.i) #2
  %FMax263.i.i.i = call float @dx.op.binary.f32(i32 35, float %FMax264.i.i.i, float %.i2413.i.i.i) #2
  %FMin262.i.i.i = call float @dx.op.binary.f32(i32 36, float %.i0405.i.i.i, float %.i1406.i.i.i) #2
  %FMin261.i.i.i = call float @dx.op.binary.f32(i32 36, float %FMin262.i.i.i, float %.i2407.i.i.i) #2
  %FMax.i.i.i = call float @dx.op.binary.f32(i32 35, float %FMax263.i.i.i, float 0.000000e+00) #2
  %FMin259.i.i.i = call float @dx.op.binary.f32(i32 36, float %FMin261.i.i.i, float %98) #2
  %v406.i.i.i = fcmp fast olt float %FMax.i.i.i, %FMin259.i.i.i
  %v407.i.i.i = and i1 %v405.i.i.i, %v406.i.i.i
  br i1 %v407.i.i.i, label %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i", label %101

; <label>:101                                     ; preds = %100
  %v416.i.i.i = or i1 %v405.i.i.i, %v406.i.i.i
  br i1 %v416.i.i.i, label %109, label %._crit_edge.1.i.i.i

._crit_edge.1.i.i.i:                              ; preds = %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i", %._crit_edge.10.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i", %101, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i", %119, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i", %109
  %.i049 = phi float [ %.i0109, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %.i0109.CurrentWorldToObject.i.i168.1.i0.i.i.i, %119 ], [ %.i0109, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %.i0109, %109 ], [ %.i0109, %101 ], [ %.i0109, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %.i0109.CurrentWorldToObject.i.i168.1.i0.i.i.i, %._crit_edge.10.i.i.i ], [ %.i0109, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %.i150 = phi float [ %.i1110, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %.i1110.CurrentWorldToObject.i.i168.1.i1.i.i.i, %119 ], [ %.i1110, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %.i1110, %109 ], [ %.i1110, %101 ], [ %.i1110, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %.i1110.CurrentWorldToObject.i.i168.1.i1.i.i.i, %._crit_edge.10.i.i.i ], [ %.i1110, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %.i251 = phi float [ %.i2111, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %.i2111.CurrentWorldToObject.i.i168.1.i2.i.i.i, %119 ], [ %.i2111, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %.i2111, %109 ], [ %.i2111, %101 ], [ %.i2111, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %.i2111.CurrentWorldToObject.i.i168.1.i2.i.i.i, %._crit_edge.10.i.i.i ], [ %.i2111, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %.i352 = phi float [ %.i3112, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %.i3112.CurrentWorldToObject.i.i168.1.i3.i.i.i, %119 ], [ %.i3112, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %.i3112, %109 ], [ %.i3112, %101 ], [ %.i3112, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %.i3112.CurrentWorldToObject.i.i168.1.i3.i.i.i, %._crit_edge.10.i.i.i ], [ %.i3112, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %.i453 = phi float [ %.i4113, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %.i4113.CurrentWorldToObject.i.i168.1.i4.i.i.i, %119 ], [ %.i4113, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %.i4113, %109 ], [ %.i4113, %101 ], [ %.i4113, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %.i4113.CurrentWorldToObject.i.i168.1.i4.i.i.i, %._crit_edge.10.i.i.i ], [ %.i4113, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %.i554 = phi float [ %.i5114, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %.i5114.CurrentWorldToObject.i.i168.1.i5.i.i.i, %119 ], [ %.i5114, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %.i5114, %109 ], [ %.i5114, %101 ], [ %.i5114, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %.i5114.CurrentWorldToObject.i.i168.1.i5.i.i.i, %._crit_edge.10.i.i.i ], [ %.i5114, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %.i655 = phi float [ %.i6115, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %.i6115.CurrentWorldToObject.i.i168.1.i6.i.i.i, %119 ], [ %.i6115, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %.i6115, %109 ], [ %.i6115, %101 ], [ %.i6115, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %.i6115.CurrentWorldToObject.i.i168.1.i6.i.i.i, %._crit_edge.10.i.i.i ], [ %.i6115, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %.i756 = phi float [ %.i7116, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %.i7116.CurrentWorldToObject.i.i168.1.i7.i.i.i, %119 ], [ %.i7116, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %.i7116, %109 ], [ %.i7116, %101 ], [ %.i7116, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %.i7116.CurrentWorldToObject.i.i168.1.i7.i.i.i, %._crit_edge.10.i.i.i ], [ %.i7116, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %.i857 = phi float [ %.i8117, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %.i8117.CurrentWorldToObject.i.i168.1.i8.i.i.i, %119 ], [ %.i8117, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %.i8117, %109 ], [ %.i8117, %101 ], [ %.i8117, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %.i8117.CurrentWorldToObject.i.i168.1.i8.i.i.i, %._crit_edge.10.i.i.i ], [ %.i8117, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %.i958 = phi float [ %.i9118, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %.i9118.CurrentWorldToObject.i.i168.1.i9.i.i.i, %119 ], [ %.i9118, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %.i9118, %109 ], [ %.i9118, %101 ], [ %.i9118, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %.i9118.CurrentWorldToObject.i.i168.1.i9.i.i.i, %._crit_edge.10.i.i.i ], [ %.i9118, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %.i1059 = phi float [ %.i10119, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %.i10119.CurrentWorldToObject.i.i168.1.i10.i.i.i, %119 ], [ %.i10119, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %.i10119, %109 ], [ %.i10119, %101 ], [ %.i10119, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %.i10119.CurrentWorldToObject.i.i168.1.i10.i.i.i, %._crit_edge.10.i.i.i ], [ %.i10119, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %.i1160 = phi float [ %.i11120, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %.i11120.CurrentWorldToObject.i.i168.1.i11.i.i.i, %119 ], [ %.i11120, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %.i11120, %109 ], [ %.i11120, %101 ], [ %.i11120, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %.i11120.CurrentWorldToObject.i.i168.1.i11.i.i.i, %._crit_edge.10.i.i.i ], [ %.i11120, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %.i073 = phi float [ %.i0121, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %.i0121.CurrentObjectToWorld.i.i167.1.i0.i.i.i, %119 ], [ %.i0121, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %.i0121, %109 ], [ %.i0121, %101 ], [ %.i0121, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %.i0121.CurrentObjectToWorld.i.i167.1.i0.i.i.i, %._crit_edge.10.i.i.i ], [ %.i0121, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %.i174 = phi float [ %.i1122, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %.i1122.CurrentObjectToWorld.i.i167.1.i1.i.i.i, %119 ], [ %.i1122, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %.i1122, %109 ], [ %.i1122, %101 ], [ %.i1122, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %.i1122.CurrentObjectToWorld.i.i167.1.i1.i.i.i, %._crit_edge.10.i.i.i ], [ %.i1122, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %.i275 = phi float [ %.i2123, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %.i2123.CurrentObjectToWorld.i.i167.1.i2.i.i.i, %119 ], [ %.i2123, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %.i2123, %109 ], [ %.i2123, %101 ], [ %.i2123, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %.i2123.CurrentObjectToWorld.i.i167.1.i2.i.i.i, %._crit_edge.10.i.i.i ], [ %.i2123, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %.i376 = phi float [ %.i3124, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %.i3124.CurrentObjectToWorld.i.i167.1.i3.i.i.i, %119 ], [ %.i3124, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %.i3124, %109 ], [ %.i3124, %101 ], [ %.i3124, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %.i3124.CurrentObjectToWorld.i.i167.1.i3.i.i.i, %._crit_edge.10.i.i.i ], [ %.i3124, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %.i477 = phi float [ %.i4125, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %.i4125.CurrentObjectToWorld.i.i167.1.i4.i.i.i, %119 ], [ %.i4125, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %.i4125, %109 ], [ %.i4125, %101 ], [ %.i4125, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %.i4125.CurrentObjectToWorld.i.i167.1.i4.i.i.i, %._crit_edge.10.i.i.i ], [ %.i4125, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %.i578 = phi float [ %.i5126, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %.i5126.CurrentObjectToWorld.i.i167.1.i5.i.i.i, %119 ], [ %.i5126, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %.i5126, %109 ], [ %.i5126, %101 ], [ %.i5126, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %.i5126.CurrentObjectToWorld.i.i167.1.i5.i.i.i, %._crit_edge.10.i.i.i ], [ %.i5126, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %.i679 = phi float [ %.i6127, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %.i6127.CurrentObjectToWorld.i.i167.1.i6.i.i.i, %119 ], [ %.i6127, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %.i6127, %109 ], [ %.i6127, %101 ], [ %.i6127, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %.i6127.CurrentObjectToWorld.i.i167.1.i6.i.i.i, %._crit_edge.10.i.i.i ], [ %.i6127, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %.i780 = phi float [ %.i7128, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %.i7128.CurrentObjectToWorld.i.i167.1.i7.i.i.i, %119 ], [ %.i7128, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %.i7128, %109 ], [ %.i7128, %101 ], [ %.i7128, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %.i7128.CurrentObjectToWorld.i.i167.1.i7.i.i.i, %._crit_edge.10.i.i.i ], [ %.i7128, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %.i881 = phi float [ %.i8129, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %.i8129.CurrentObjectToWorld.i.i167.1.i8.i.i.i, %119 ], [ %.i8129, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %.i8129, %109 ], [ %.i8129, %101 ], [ %.i8129, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %.i8129.CurrentObjectToWorld.i.i167.1.i8.i.i.i, %._crit_edge.10.i.i.i ], [ %.i8129, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %.i982 = phi float [ %.i9130, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %.i9130.CurrentObjectToWorld.i.i167.1.i9.i.i.i, %119 ], [ %.i9130, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %.i9130, %109 ], [ %.i9130, %101 ], [ %.i9130, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %.i9130.CurrentObjectToWorld.i.i167.1.i9.i.i.i, %._crit_edge.10.i.i.i ], [ %.i9130, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %.i1083 = phi float [ %.i10131, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %.i10131.CurrentObjectToWorld.i.i167.1.i10.i.i.i, %119 ], [ %.i10131, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %.i10131, %109 ], [ %.i10131, %101 ], [ %.i10131, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %.i10131.CurrentObjectToWorld.i.i167.1.i10.i.i.i, %._crit_edge.10.i.i.i ], [ %.i10131, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %.i1184 = phi float [ %.i11132, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %.i11132.CurrentObjectToWorld.i.i167.1.i11.i.i.i, %119 ], [ %.i11132, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %.i11132, %109 ], [ %.i11132, %101 ], [ %.i11132, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %.i11132.CurrentObjectToWorld.i.i167.1.i11.i.i.i, %._crit_edge.10.i.i.i ], [ %.i11132, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %102 = phi i32 [ %93, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %94, %119 ], [ %93, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %93, %109 ], [ %93, %101 ], [ %93, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %94, %._crit_edge.10.i.i.i ], [ %93, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %103 = phi i32 [ %94, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %93, %119 ], [ %94, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %94, %109 ], [ %94, %101 ], [ %94, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %93, %._crit_edge.10.i.i.i ], [ %94, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %104 = phi i32 [ %95, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %instIdx.i.i.1.i.i.i, %119 ], [ %95, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %95, %109 ], [ %95, %101 ], [ %95, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %instIdx.i.i.1.i.i.i, %._crit_edge.10.i.i.i ], [ %95, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %105 = phi i32 [ %96, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %v353.i.479.i.i, %119 ], [ %96, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %96, %109 ], [ %96, %101 ], [ %96, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %v353.i.479.i.i, %._crit_edge.10.i.i.i ], [ %96, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %106 = phi i32 [ %97, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %v350.i.476.i.i, %119 ], [ %97, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %97, %109 ], [ %97, %101 ], [ %97, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %v350.i.476.i.i, %._crit_edge.10.i.i.i ], [ %97, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %107 = phi float [ %98, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %.0173.RayTCurrent.i.i.i, %119 ], [ %98, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %98, %109 ], [ %98, %101 ], [ %98, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %.0173.RayTCurrent.i.i.i, %._crit_edge.10.i.i.i ], [ %98, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %.3.i0.i.i.i = phi float [ %.1.i0.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %.2.i0.i.i.i, %119 ], [ %.1.i0.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %.1.i0.i.i.i, %109 ], [ %.1.i0.i.i.i, %101 ], [ %.2.i0.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %.2.i0.i.i.i, %._crit_edge.10.i.i.i ], [ %.1.i0.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %.3.i1.i.i.i = phi float [ %.1.i1.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %.2.i1.i.i.i, %119 ], [ %.1.i1.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %.1.i1.i.i.i, %109 ], [ %.1.i1.i.i.i, %101 ], [ %.2.i1.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %.2.i1.i.i.i, %._crit_edge.10.i.i.i ], [ %.1.i1.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %currentRayData.i.i.0.2.i0.i.i.i = phi float [ %FMad292.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %currentRayData.i.i.0.1.i0.i.i.i, %119 ], [ %currentRayData.i.i.0.1.i0.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %currentRayData.i.i.0.1.i0.i.i.i, %109 ], [ %currentRayData.i.i.0.1.i0.i.i.i, %101 ], [ %currentRayData.i.i.0.1.i0.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %currentRayData.i.i.0.1.i0.i.i.i, %._crit_edge.10.i.i.i ], [ %currentRayData.i.i.0.1.i0.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %currentRayData.i.i.0.2.i1.i.i.i = phi float [ %FMad289.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %currentRayData.i.i.0.1.i1.i.i.i, %119 ], [ %currentRayData.i.i.0.1.i1.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %currentRayData.i.i.0.1.i1.i.i.i, %109 ], [ %currentRayData.i.i.0.1.i1.i.i.i, %101 ], [ %currentRayData.i.i.0.1.i1.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %currentRayData.i.i.0.1.i1.i.i.i, %._crit_edge.10.i.i.i ], [ %currentRayData.i.i.0.1.i1.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %currentRayData.i.i.0.2.i2.i.i.i = phi float [ %FMad286.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %currentRayData.i.i.0.1.i2.i.i.i, %119 ], [ %currentRayData.i.i.0.1.i2.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %currentRayData.i.i.0.1.i2.i.i.i, %109 ], [ %currentRayData.i.i.0.1.i2.i.i.i, %101 ], [ %currentRayData.i.i.0.1.i2.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %currentRayData.i.i.0.1.i2.i.i.i, %._crit_edge.10.i.i.i ], [ %currentRayData.i.i.0.1.i2.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %currentRayData.i.i.1.2.i0.i.i.i = phi float [ %FMad283.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %currentRayData.i.i.1.1.i0.i.i.i, %119 ], [ %currentRayData.i.i.1.1.i0.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %currentRayData.i.i.1.1.i0.i.i.i, %109 ], [ %currentRayData.i.i.1.1.i0.i.i.i, %101 ], [ %currentRayData.i.i.1.1.i0.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %currentRayData.i.i.1.1.i0.i.i.i, %._crit_edge.10.i.i.i ], [ %currentRayData.i.i.1.1.i0.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %currentRayData.i.i.1.2.i1.i.i.i = phi float [ %FMad280.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %currentRayData.i.i.1.1.i1.i.i.i, %119 ], [ %currentRayData.i.i.1.1.i1.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %currentRayData.i.i.1.1.i1.i.i.i, %109 ], [ %currentRayData.i.i.1.1.i1.i.i.i, %101 ], [ %currentRayData.i.i.1.1.i1.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %currentRayData.i.i.1.1.i1.i.i.i, %._crit_edge.10.i.i.i ], [ %currentRayData.i.i.1.1.i1.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %currentRayData.i.i.1.2.i2.i.i.i = phi float [ %FMad.i.596.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %currentRayData.i.i.1.1.i2.i.i.i, %119 ], [ %currentRayData.i.i.1.1.i2.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %currentRayData.i.i.1.1.i2.i.i.i, %109 ], [ %currentRayData.i.i.1.1.i2.i.i.i, %101 ], [ %currentRayData.i.i.1.1.i2.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %currentRayData.i.i.1.1.i2.i.i.i, %._crit_edge.10.i.i.i ], [ %currentRayData.i.i.1.1.i2.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %currentRayData.i.i.2.2.i0.i.i.i = phi float [ %.i0340.i.600.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %currentRayData.i.i.2.1.i0.i.i.i, %119 ], [ %currentRayData.i.i.2.1.i0.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %currentRayData.i.i.2.1.i0.i.i.i, %109 ], [ %currentRayData.i.i.2.1.i0.i.i.i, %101 ], [ %currentRayData.i.i.2.1.i0.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %currentRayData.i.i.2.1.i0.i.i.i, %._crit_edge.10.i.i.i ], [ %currentRayData.i.i.2.1.i0.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %currentRayData.i.i.2.2.i1.i.i.i = phi float [ %.i1341.i.601.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %currentRayData.i.i.2.1.i1.i.i.i, %119 ], [ %currentRayData.i.i.2.1.i1.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %currentRayData.i.i.2.1.i1.i.i.i, %109 ], [ %currentRayData.i.i.2.1.i1.i.i.i, %101 ], [ %currentRayData.i.i.2.1.i1.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %currentRayData.i.i.2.1.i1.i.i.i, %._crit_edge.10.i.i.i ], [ %currentRayData.i.i.2.1.i1.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %currentRayData.i.i.2.2.i2.i.i.i = phi float [ %.i2342.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %currentRayData.i.i.2.1.i2.i.i.i, %119 ], [ %currentRayData.i.i.2.1.i2.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %currentRayData.i.i.2.1.i2.i.i.i, %109 ], [ %currentRayData.i.i.2.1.i2.i.i.i, %101 ], [ %currentRayData.i.i.2.1.i2.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %currentRayData.i.i.2.1.i2.i.i.i, %._crit_edge.10.i.i.i ], [ %currentRayData.i.i.2.1.i2.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %currentRayData.i.i.3.2.i0.i.i.i = phi float [ %.i0343.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %currentRayData.i.i.3.1.i0.i.i.i, %119 ], [ %currentRayData.i.i.3.1.i0.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %currentRayData.i.i.3.1.i0.i.i.i, %109 ], [ %currentRayData.i.i.3.1.i0.i.i.i, %101 ], [ %currentRayData.i.i.3.1.i0.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %currentRayData.i.i.3.1.i0.i.i.i, %._crit_edge.10.i.i.i ], [ %currentRayData.i.i.3.1.i0.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %currentRayData.i.i.3.2.i1.i.i.i = phi float [ %.i1344.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %currentRayData.i.i.3.1.i1.i.i.i, %119 ], [ %currentRayData.i.i.3.1.i1.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %currentRayData.i.i.3.1.i1.i.i.i, %109 ], [ %currentRayData.i.i.3.1.i1.i.i.i, %101 ], [ %currentRayData.i.i.3.1.i1.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %currentRayData.i.i.3.1.i1.i.i.i, %._crit_edge.10.i.i.i ], [ %currentRayData.i.i.3.1.i1.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %currentRayData.i.i.3.2.i2.i.i.i = phi float [ %.i2345.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %currentRayData.i.i.3.1.i2.i.i.i, %119 ], [ %currentRayData.i.i.3.1.i2.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %currentRayData.i.i.3.1.i2.i.i.i, %109 ], [ %currentRayData.i.i.3.1.i2.i.i.i, %101 ], [ %currentRayData.i.i.3.1.i2.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %currentRayData.i.i.3.1.i2.i.i.i, %._crit_edge.10.i.i.i ], [ %currentRayData.i.i.3.1.i2.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %currentRayData.i.i.4.2.i0.i.i.i = phi float [ %v217.i.616.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %currentRayData.i.i.4.1.i0.i.i.i, %119 ], [ %currentRayData.i.i.4.1.i0.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %currentRayData.i.i.4.1.i0.i.i.i, %109 ], [ %currentRayData.i.i.4.1.i0.i.i.i, %101 ], [ %currentRayData.i.i.4.1.i0.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %currentRayData.i.i.4.1.i0.i.i.i, %._crit_edge.10.i.i.i ], [ %currentRayData.i.i.4.1.i0.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %currentRayData.i.i.4.2.i1.i.i.i = phi float [ %v220.i.619.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %currentRayData.i.i.4.1.i1.i.i.i, %119 ], [ %currentRayData.i.i.4.1.i1.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %currentRayData.i.i.4.1.i1.i.i.i, %109 ], [ %currentRayData.i.i.4.1.i1.i.i.i, %101 ], [ %currentRayData.i.i.4.1.i1.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %currentRayData.i.i.4.1.i1.i.i.i, %._crit_edge.10.i.i.i ], [ %currentRayData.i.i.4.1.i1.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %currentRayData.i.i.4.2.i2.i.i.i = phi float [ %v221.i.620.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %currentRayData.i.i.4.1.i2.i.i.i, %119 ], [ %currentRayData.i.i.4.1.i2.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %currentRayData.i.i.4.1.i2.i.i.i, %109 ], [ %currentRayData.i.i.4.1.i2.i.i.i, %101 ], [ %currentRayData.i.i.4.1.i2.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %currentRayData.i.i.4.1.i2.i.i.i, %._crit_edge.10.i.i.i ], [ %currentRayData.i.i.4.1.i2.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %currentRayData.i.i.5.2.i0.i.i.i = phi i32 [ %.5.0.i0.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %currentRayData.i.i.5.1.i0.i.i.i, %119 ], [ %currentRayData.i.i.5.1.i0.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %currentRayData.i.i.5.1.i0.i.i.i, %109 ], [ %currentRayData.i.i.5.1.i0.i.i.i, %101 ], [ %currentRayData.i.i.5.1.i0.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %currentRayData.i.i.5.1.i0.i.i.i, %._crit_edge.10.i.i.i ], [ %currentRayData.i.i.5.1.i0.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %currentRayData.i.i.5.2.i1.i.i.i = phi i32 [ %.5.0.i1.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %currentRayData.i.i.5.1.i1.i.i.i, %119 ], [ %currentRayData.i.i.5.1.i1.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %currentRayData.i.i.5.1.i1.i.i.i, %109 ], [ %currentRayData.i.i.5.1.i1.i.i.i, %101 ], [ %currentRayData.i.i.5.1.i1.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %currentRayData.i.i.5.1.i1.i.i.i, %._crit_edge.10.i.i.i ], [ %currentRayData.i.i.5.1.i1.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %currentRayData.i.i.5.2.i2.i.i.i = phi i32 [ %.0172.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %currentRayData.i.i.5.1.i2.i.i.i, %119 ], [ %currentRayData.i.i.5.1.i2.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %currentRayData.i.i.5.1.i2.i.i.i, %109 ], [ %currentRayData.i.i.5.1.i2.i.i.i, %101 ], [ %currentRayData.i.i.5.1.i2.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %currentRayData.i.i.5.1.i2.i.i.i, %._crit_edge.10.i.i.i ], [ %currentRayData.i.i.5.1.i2.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %CurrentObjectToWorld.i.i167.2.i0.i.i.i = phi float [ %v185.i.580.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i0.i.i.i, %119 ], [ %CurrentObjectToWorld.i.i167.1.i0.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i0.i.i.i, %109 ], [ %CurrentObjectToWorld.i.i167.1.i0.i.i.i, %101 ], [ %CurrentObjectToWorld.i.i167.1.i0.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i0.i.i.i, %._crit_edge.10.i.i.i ], [ %CurrentObjectToWorld.i.i167.1.i0.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %CurrentObjectToWorld.i.i167.2.i1.i.i.i = phi float [ %v186.i.581.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i1.i.i.i, %119 ], [ %CurrentObjectToWorld.i.i167.1.i1.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i1.i.i.i, %109 ], [ %CurrentObjectToWorld.i.i167.1.i1.i.i.i, %101 ], [ %CurrentObjectToWorld.i.i167.1.i1.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i1.i.i.i, %._crit_edge.10.i.i.i ], [ %CurrentObjectToWorld.i.i167.1.i1.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %CurrentObjectToWorld.i.i167.2.i2.i.i.i = phi float [ %v187.i.582.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i2.i.i.i, %119 ], [ %CurrentObjectToWorld.i.i167.1.i2.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i2.i.i.i, %109 ], [ %CurrentObjectToWorld.i.i167.1.i2.i.i.i, %101 ], [ %CurrentObjectToWorld.i.i167.1.i2.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i2.i.i.i, %._crit_edge.10.i.i.i ], [ %CurrentObjectToWorld.i.i167.1.i2.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %CurrentObjectToWorld.i.i167.2.i3.i.i.i = phi float [ %v188.i.583.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i3.i.i.i, %119 ], [ %CurrentObjectToWorld.i.i167.1.i3.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i3.i.i.i, %109 ], [ %CurrentObjectToWorld.i.i167.1.i3.i.i.i, %101 ], [ %CurrentObjectToWorld.i.i167.1.i3.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i3.i.i.i, %._crit_edge.10.i.i.i ], [ %CurrentObjectToWorld.i.i167.1.i3.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %CurrentObjectToWorld.i.i167.2.i4.i.i.i = phi float [ %v189.i.584.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i4.i.i.i, %119 ], [ %CurrentObjectToWorld.i.i167.1.i4.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i4.i.i.i, %109 ], [ %CurrentObjectToWorld.i.i167.1.i4.i.i.i, %101 ], [ %CurrentObjectToWorld.i.i167.1.i4.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i4.i.i.i, %._crit_edge.10.i.i.i ], [ %CurrentObjectToWorld.i.i167.1.i4.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %CurrentObjectToWorld.i.i167.2.i5.i.i.i = phi float [ %v190.i.585.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i5.i.i.i, %119 ], [ %CurrentObjectToWorld.i.i167.1.i5.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i5.i.i.i, %109 ], [ %CurrentObjectToWorld.i.i167.1.i5.i.i.i, %101 ], [ %CurrentObjectToWorld.i.i167.1.i5.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i5.i.i.i, %._crit_edge.10.i.i.i ], [ %CurrentObjectToWorld.i.i167.1.i5.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %CurrentObjectToWorld.i.i167.2.i6.i.i.i = phi float [ %v191.i.586.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i6.i.i.i, %119 ], [ %CurrentObjectToWorld.i.i167.1.i6.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i6.i.i.i, %109 ], [ %CurrentObjectToWorld.i.i167.1.i6.i.i.i, %101 ], [ %CurrentObjectToWorld.i.i167.1.i6.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i6.i.i.i, %._crit_edge.10.i.i.i ], [ %CurrentObjectToWorld.i.i167.1.i6.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %CurrentObjectToWorld.i.i167.2.i7.i.i.i = phi float [ %v192.i.587.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i7.i.i.i, %119 ], [ %CurrentObjectToWorld.i.i167.1.i7.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i7.i.i.i, %109 ], [ %CurrentObjectToWorld.i.i167.1.i7.i.i.i, %101 ], [ %CurrentObjectToWorld.i.i167.1.i7.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i7.i.i.i, %._crit_edge.10.i.i.i ], [ %CurrentObjectToWorld.i.i167.1.i7.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %CurrentObjectToWorld.i.i167.2.i8.i.i.i = phi float [ %v193.i.588.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i8.i.i.i, %119 ], [ %CurrentObjectToWorld.i.i167.1.i8.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i8.i.i.i, %109 ], [ %CurrentObjectToWorld.i.i167.1.i8.i.i.i, %101 ], [ %CurrentObjectToWorld.i.i167.1.i8.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i8.i.i.i, %._crit_edge.10.i.i.i ], [ %CurrentObjectToWorld.i.i167.1.i8.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %CurrentObjectToWorld.i.i167.2.i9.i.i.i = phi float [ %v194.i.589.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i9.i.i.i, %119 ], [ %CurrentObjectToWorld.i.i167.1.i9.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i9.i.i.i, %109 ], [ %CurrentObjectToWorld.i.i167.1.i9.i.i.i, %101 ], [ %CurrentObjectToWorld.i.i167.1.i9.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i9.i.i.i, %._crit_edge.10.i.i.i ], [ %CurrentObjectToWorld.i.i167.1.i9.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %CurrentObjectToWorld.i.i167.2.i10.i.i.i = phi float [ %v195.i.590.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i10.i.i.i, %119 ], [ %CurrentObjectToWorld.i.i167.1.i10.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i10.i.i.i, %109 ], [ %CurrentObjectToWorld.i.i167.1.i10.i.i.i, %101 ], [ %CurrentObjectToWorld.i.i167.1.i10.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i10.i.i.i, %._crit_edge.10.i.i.i ], [ %CurrentObjectToWorld.i.i167.1.i10.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %CurrentObjectToWorld.i.i167.2.i11.i.i.i = phi float [ %v196.i.591.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i11.i.i.i, %119 ], [ %CurrentObjectToWorld.i.i167.1.i11.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i11.i.i.i, %109 ], [ %CurrentObjectToWorld.i.i167.1.i11.i.i.i, %101 ], [ %CurrentObjectToWorld.i.i167.1.i11.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i11.i.i.i, %._crit_edge.10.i.i.i ], [ %CurrentObjectToWorld.i.i167.1.i11.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %CurrentWorldToObject.i.i168.2.i0.i.i.i = phi float [ %v168.i.563.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i0.i.i.i, %119 ], [ %CurrentWorldToObject.i.i168.1.i0.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i0.i.i.i, %109 ], [ %CurrentWorldToObject.i.i168.1.i0.i.i.i, %101 ], [ %CurrentWorldToObject.i.i168.1.i0.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i0.i.i.i, %._crit_edge.10.i.i.i ], [ %CurrentWorldToObject.i.i168.1.i0.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %CurrentWorldToObject.i.i168.2.i1.i.i.i = phi float [ %v169.i.564.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i1.i.i.i, %119 ], [ %CurrentWorldToObject.i.i168.1.i1.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i1.i.i.i, %109 ], [ %CurrentWorldToObject.i.i168.1.i1.i.i.i, %101 ], [ %CurrentWorldToObject.i.i168.1.i1.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i1.i.i.i, %._crit_edge.10.i.i.i ], [ %CurrentWorldToObject.i.i168.1.i1.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %CurrentWorldToObject.i.i168.2.i2.i.i.i = phi float [ %v170.i.565.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i2.i.i.i, %119 ], [ %CurrentWorldToObject.i.i168.1.i2.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i2.i.i.i, %109 ], [ %CurrentWorldToObject.i.i168.1.i2.i.i.i, %101 ], [ %CurrentWorldToObject.i.i168.1.i2.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i2.i.i.i, %._crit_edge.10.i.i.i ], [ %CurrentWorldToObject.i.i168.1.i2.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %CurrentWorldToObject.i.i168.2.i3.i.i.i = phi float [ %v171.i.566.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i3.i.i.i, %119 ], [ %CurrentWorldToObject.i.i168.1.i3.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i3.i.i.i, %109 ], [ %CurrentWorldToObject.i.i168.1.i3.i.i.i, %101 ], [ %CurrentWorldToObject.i.i168.1.i3.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i3.i.i.i, %._crit_edge.10.i.i.i ], [ %CurrentWorldToObject.i.i168.1.i3.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %CurrentWorldToObject.i.i168.2.i4.i.i.i = phi float [ %v172.i.567.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i4.i.i.i, %119 ], [ %CurrentWorldToObject.i.i168.1.i4.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i4.i.i.i, %109 ], [ %CurrentWorldToObject.i.i168.1.i4.i.i.i, %101 ], [ %CurrentWorldToObject.i.i168.1.i4.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i4.i.i.i, %._crit_edge.10.i.i.i ], [ %CurrentWorldToObject.i.i168.1.i4.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %CurrentWorldToObject.i.i168.2.i5.i.i.i = phi float [ %v173.i.568.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i5.i.i.i, %119 ], [ %CurrentWorldToObject.i.i168.1.i5.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i5.i.i.i, %109 ], [ %CurrentWorldToObject.i.i168.1.i5.i.i.i, %101 ], [ %CurrentWorldToObject.i.i168.1.i5.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i5.i.i.i, %._crit_edge.10.i.i.i ], [ %CurrentWorldToObject.i.i168.1.i5.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %CurrentWorldToObject.i.i168.2.i6.i.i.i = phi float [ %v174.i.569.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i6.i.i.i, %119 ], [ %CurrentWorldToObject.i.i168.1.i6.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i6.i.i.i, %109 ], [ %CurrentWorldToObject.i.i168.1.i6.i.i.i, %101 ], [ %CurrentWorldToObject.i.i168.1.i6.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i6.i.i.i, %._crit_edge.10.i.i.i ], [ %CurrentWorldToObject.i.i168.1.i6.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %CurrentWorldToObject.i.i168.2.i7.i.i.i = phi float [ %v175.i.570.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i7.i.i.i, %119 ], [ %CurrentWorldToObject.i.i168.1.i7.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i7.i.i.i, %109 ], [ %CurrentWorldToObject.i.i168.1.i7.i.i.i, %101 ], [ %CurrentWorldToObject.i.i168.1.i7.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i7.i.i.i, %._crit_edge.10.i.i.i ], [ %CurrentWorldToObject.i.i168.1.i7.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %CurrentWorldToObject.i.i168.2.i8.i.i.i = phi float [ %v176.i.571.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i8.i.i.i, %119 ], [ %CurrentWorldToObject.i.i168.1.i8.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i8.i.i.i, %109 ], [ %CurrentWorldToObject.i.i168.1.i8.i.i.i, %101 ], [ %CurrentWorldToObject.i.i168.1.i8.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i8.i.i.i, %._crit_edge.10.i.i.i ], [ %CurrentWorldToObject.i.i168.1.i8.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %CurrentWorldToObject.i.i168.2.i9.i.i.i = phi float [ %v177.i.572.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i9.i.i.i, %119 ], [ %CurrentWorldToObject.i.i168.1.i9.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i9.i.i.i, %109 ], [ %CurrentWorldToObject.i.i168.1.i9.i.i.i, %101 ], [ %CurrentWorldToObject.i.i168.1.i9.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i9.i.i.i, %._crit_edge.10.i.i.i ], [ %CurrentWorldToObject.i.i168.1.i9.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %CurrentWorldToObject.i.i168.2.i10.i.i.i = phi float [ %v178.i.573.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i10.i.i.i, %119 ], [ %CurrentWorldToObject.i.i168.1.i10.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i10.i.i.i, %109 ], [ %CurrentWorldToObject.i.i168.1.i10.i.i.i, %101 ], [ %CurrentWorldToObject.i.i168.1.i10.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i10.i.i.i, %._crit_edge.10.i.i.i ], [ %CurrentWorldToObject.i.i168.1.i10.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %CurrentWorldToObject.i.i168.2.i11.i.i.i = phi float [ %v179.i.574.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i11.i.i.i, %119 ], [ %CurrentWorldToObject.i.i168.1.i11.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i11.i.i.i, %109 ], [ %CurrentWorldToObject.i.i168.1.i11.i.i.i, %101 ], [ %CurrentWorldToObject.i.i168.1.i11.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i11.i.i.i, %._crit_edge.10.i.i.i ], [ %CurrentWorldToObject.i.i168.1.i11.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %ResetMatrices.i.i.3.i.i.i = phi i32 [ %ResetMatrices.i.i.1.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ 0, %119 ], [ %ResetMatrices.i.i.1.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %ResetMatrices.i.i.1.i.i.i, %109 ], [ %ResetMatrices.i.i.1.i.i.i, %101 ], [ %ResetMatrices.i.i.1.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ 0, %._crit_edge.10.i.i.i ], [ %ResetMatrices.i.i.1.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %currentBVHIndex.i.i.2.i.i.i = phi i32 [ 1, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %currentBVHIndex.i.i.1.i.i.i, %119 ], [ %currentBVHIndex.i.i.1.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %currentBVHIndex.i.i.1.i.i.i, %109 ], [ %currentBVHIndex.i.i.1.i.i.i, %101 ], [ %currentBVHIndex.i.i.1.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %currentBVHIndex.i.i.1.i.i.i, %._crit_edge.10.i.i.i ], [ 0, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %currentGpuVA.i.i.2.i0.i.i.i = phi i32 [ %v148.i.543.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %currentGpuVA.i.i.1.i0.i.i.i, %119 ], [ %currentGpuVA.i.i.1.i0.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %currentGpuVA.i.i.1.i0.i.i.i, %109 ], [ %currentGpuVA.i.i.1.i0.i.i.i, %101 ], [ %currentGpuVA.i.i.1.i0.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %currentGpuVA.i.i.1.i0.i.i.i, %._crit_edge.10.i.i.i ], [ %currentGpuVA.i.i.1.i0.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %currentGpuVA.i.i.2.i1.i.i.i = phi i32 [ %v149.i.544.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %currentGpuVA.i.i.1.i1.i.i.i, %119 ], [ %currentGpuVA.i.i.1.i1.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %currentGpuVA.i.i.1.i1.i.i.i, %109 ], [ %currentGpuVA.i.i.1.i1.i.i.i, %101 ], [ %currentGpuVA.i.i.1.i1.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %currentGpuVA.i.i.1.i1.i.i.i, %._crit_edge.10.i.i.i ], [ %currentGpuVA.i.i.1.i1.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %instIdx.i.i.2.i.i.i = phi i32 [ %v123.i.518.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %instIdx.i.i.1.i.i.i, %119 ], [ %instIdx.i.i.1.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %instIdx.i.i.1.i.i.i, %109 ], [ %instIdx.i.i.1.i.i.i, %101 ], [ %instIdx.i.i.1.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %instIdx.i.i.1.i.i.i, %._crit_edge.10.i.i.i ], [ %v123.i.518.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %instFlags.i.i.2.i.i.i = phi i32 [ %v197.i.592.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %instFlags.i.i.1.i.i.i, %119 ], [ %instFlags.i.i.1.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %instFlags.i.i.1.i.i.i, %109 ], [ %instFlags.i.i.1.i.i.i, %101 ], [ %instFlags.i.i.1.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %instFlags.i.i.1.i.i.i, %._crit_edge.10.i.i.i ], [ %instFlags.i.i.1.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %instOffset.i.i.2.i.i.i = phi i32 [ %v180.i.575.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %instOffset.i.i.1.i.i.i, %119 ], [ %instOffset.i.i.1.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %instOffset.i.i.1.i.i.i, %109 ], [ %instOffset.i.i.1.i.i.i, %101 ], [ %instOffset.i.i.1.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %instOffset.i.i.1.i.i.i, %._crit_edge.10.i.i.i ], [ %v180.i.575.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %instId.i.i.2.i.i.i = phi i32 [ %v181.i.576.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %instId.i.i.1.i.i.i, %119 ], [ %instId.i.i.1.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %instId.i.i.1.i.i.i, %109 ], [ %instId.i.i.1.i.i.i, %101 ], [ %instId.i.i.1.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %instId.i.i.1.i.i.i, %._crit_edge.10.i.i.i ], [ %v181.i.576.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %stackPointer.i.i.3.i.i.i = phi i32 [ %stackPointer.i.i.2.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %v103.i.312.i.i, %119 ], [ %v414.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %stackPointer.i.i.2.i.i.i, %109 ], [ %v103.i.312.i.i, %101 ], [ %v103.i.312.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %v103.i.312.i.i, %._crit_edge.10.i.i.i ], [ %v103.i.312.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %resultBary.i.i.2.i0.i.i.i = phi float [ %resultBary.i.i.1.i0.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %.2.i0.resultBary.i.i.1.i0.i.i.i, %119 ], [ %resultBary.i.i.1.i0.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %resultBary.i.i.1.i0.i.i.i, %109 ], [ %resultBary.i.i.1.i0.i.i.i, %101 ], [ %.2.i0.resultBary.i.i.1.i0.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %.2.i0.resultBary.i.i.1.i0.i.i.i, %._crit_edge.10.i.i.i ], [ %resultBary.i.i.1.i0.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %resultBary.i.i.2.i1.i.i.i = phi float [ %resultBary.i.i.1.i1.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %.2.i1.resultBary.i.i.1.i1.i.i.i, %119 ], [ %resultBary.i.i.1.i1.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %resultBary.i.i.1.i1.i.i.i, %109 ], [ %resultBary.i.i.1.i1.i.i.i, %101 ], [ %.2.i1.resultBary.i.i.1.i1.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %.2.i1.resultBary.i.i.1.i1.i.i.i, %._crit_edge.10.i.i.i ], [ %resultBary.i.i.1.i1.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %resultTriId.i.i.2.i.i.i = phi i32 [ %resultTriId.i.i.1.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %.resultTriId.i.i.1.i.i.i, %119 ], [ %resultTriId.i.i.1.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %resultTriId.i.i.1.i.i.i, %109 ], [ %resultTriId.i.i.1.i.i.i, %101 ], [ %.resultTriId.i.i.1.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %.resultTriId.i.i.1.i.i.i, %._crit_edge.10.i.i.i ], [ %resultTriId.i.i.1.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %v418.i.i.i = getelementptr inbounds [2 x i32], [2 x i32]* %nodesToProcess.i.i.i.i.i, i32 0, i32 %currentBVHIndex.i.i.2.i.i.i
  %v419.i.i.i = load i32, i32* %v418.i.i.i, align 4, !tbaa !105
  %v420.i.i.i = icmp eq i32 %v419.i.i.i, 0
  br i1 %v420.i.i.i, label %108, label %._crit_edge.2.i.i.i

; <label>:108                                     ; preds = %._crit_edge.1.i.i.i
  %v421.i.i.i = add i32 %currentBVHIndex.i.i.2.i.i.i, -1
  %v422.i.i.i = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %AccelerationStructureList_cbuffer, i32 0) #2
  %v423.i.i.i = extractvalue %dx.types.CBufRet.i32 %v422.i.i.i, 0
  %v424.i.i.i = extractvalue %dx.types.CBufRet.i32 %v422.i.i.i, 1
  %v425.i.i.i = load i32, i32* %v56.i.262.i.i, align 4, !tbaa !105
  %v426.i.i.i = icmp eq i32 %v425.i.i.i, 0
  br i1 %v426.i.i.i, label %._crit_edge.i.289.i.i, label %.lr.ph.i.i.i

; <label>:109                                     ; preds = %101
  %v417.i.i.i = select i1 %v406.i.i.i, i32 %v120.i.329.i.i, i32 %v369.i.331.i.i
  store i32 %v417.i.i.i, i32 addrspace(3)* %v106.i.315.i.i, align 4, !tbaa !105, !noalias !113
  store i32 %v102.i.311.i.i, i32* %v108.i.317.i.i, align 4, !tbaa !105
  br label %._crit_edge.1.i.i.i

"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i": ; preds = %100
  %v408.i.i.i = fcmp fast olt float %FMax.i.i.i, %FMax266.i.i.i
  %v409.i.i.i = select i1 %v408.i.i.i, i32 %v369.i.331.i.i, i32 %v120.i.329.i.i
  %v410.i.i.i = select i1 %v408.i.i.i, i32 %v120.i.329.i.i, i32 %v369.i.331.i.i
  %v411.i.i.i = shl i32 %stackPointer.i.i.2.i.i.i, 6
  %v412.i.i.i = add i32 %v411.i.i.i, %groupIndex.i
  store i32 %v409.i.i.i, i32 addrspace(3)* %v106.i.315.i.i, align 4, !tbaa !105, !noalias !116
  %v413.i.i.i = getelementptr [2048 x i32], [2048 x i32] addrspace(3)* @"\01?stack@@3PAIA", i32 0, i32 %v412.i.i.i
  store i32 %v410.i.i.i, i32 addrspace(3)* %v413.i.i.i, align 4, !tbaa !105, !noalias !116
  %v414.i.i.i = add nsw i32 %stackPointer.i.i.2.i.i.i, 1
  %v415.i.i.i = add i32 %v102.i.311.i.i, 1
  store i32 %v415.i.i.i, i32* %v108.i.317.i.i, align 4, !tbaa !105
  br label %._crit_edge.1.i.i.i

; <label>:110                                     ; preds = %._crit_edge.2.i.i.i
  %v122.i.354.i.i = icmp eq i32 %currentBVHIndex.i.i.1.i.i.i, 0
  br i1 %v122.i.354.i.i, label %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i", label %"\01?IsOpaque@@YA_N_NII@Z.exit.i.i.i.i.i"

"\01?IsOpaque@@YA_N_NII@Z.exit.i.i.i.i.i":        ; preds = %110
  %v222.i.355.i.i = and i32 %arg0.int.i.i.i, 64
  %v223.i.356.i.i = icmp eq i32 %v222.i.355.i.i, 0
  %v224.i.357.i.i = and i32 %v117.i.326.i.i, 16777215
  %v225.i.358.i.i = add i32 %currentGpuVA.i.i.1.i0.i.i.i, 4
  %v227.i.360.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %DescriptorHeapBufferTable_UAV_rawbuf, i32 %v225.i.358.i.i, i32 undef) #2
  %v228.i.361.i.i = extractvalue %dx.types.ResRet.i32 %v227.i.360.i.i, 0
  %v229.i.362.i.i = mul nuw nsw i32 %v224.i.357.i.i, 36
  %v230.i.363.i.i = add i32 %v229.i.362.i.i, %currentGpuVA.i.i.1.i0.i.i.i
  %v231.i.364.i.i = add i32 %v230.i.363.i.i, %v228.i.361.i.i
  %v232.i.365.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %DescriptorHeapBufferTable_UAV_rawbuf, i32 %v231.i.364.i.i, i32 undef) #2
  %v233.i.366.i.i = extractvalue %dx.types.ResRet.i32 %v232.i.365.i.i, 0
  %v234.i.367.i.i = extractvalue %dx.types.ResRet.i32 %v232.i.365.i.i, 1
  %v235.i.368.i.i = extractvalue %dx.types.ResRet.i32 %v232.i.365.i.i, 2
  %v236.i.369.i.i = extractvalue %dx.types.ResRet.i32 %v232.i.365.i.i, 3
  %.i0346.i.i.i = bitcast i32 %v233.i.366.i.i to float
  %.i1347.i.i.i = bitcast i32 %v234.i.367.i.i to float
  %.i2348.i.i.i = bitcast i32 %v235.i.368.i.i to float
  %.i3349.i.i.i = bitcast i32 %v236.i.369.i.i to float
  %v237.i.370.i.i = add i32 %v231.i.364.i.i, 16
  %v238.i.371.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %DescriptorHeapBufferTable_UAV_rawbuf, i32 %v237.i.370.i.i, i32 undef) #2
  %v239.i.372.i.i = extractvalue %dx.types.ResRet.i32 %v238.i.371.i.i, 0
  %v240.i.373.i.i = extractvalue %dx.types.ResRet.i32 %v238.i.371.i.i, 1
  %v241.i.374.i.i = extractvalue %dx.types.ResRet.i32 %v238.i.371.i.i, 2
  %v242.i.375.i.i = extractvalue %dx.types.ResRet.i32 %v238.i.371.i.i, 3
  %.i0350.i.i.i = bitcast i32 %v239.i.372.i.i to float
  %.i1351.i.i.i = bitcast i32 %v240.i.373.i.i to float
  %.i2352.i.i.i = bitcast i32 %v241.i.374.i.i to float
  %.i3353.i.i.i = bitcast i32 %v242.i.375.i.i to float
  %v243.i.376.i.i = add i32 %v231.i.364.i.i, 32
  %v244.i.377.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %DescriptorHeapBufferTable_UAV_rawbuf, i32 %v243.i.376.i.i, i32 undef) #2
  %v245.i.378.i.i = extractvalue %dx.types.ResRet.i32 %v244.i.377.i.i, 0
  %v246.i.379.i.i = bitcast i32 %v245.i.378.i.i to float
  %v247.i.380.i.i = and i32 %instFlags.i.i.1.i.i.i, 1
  %v248.i.381.i.i = icmp eq i32 %v247.i.380.i.i, 0
  %v249.i.382.i.i = shl i32 %instFlags.i.i.1.i.i.i, 3
  %v250.i.383.i.i = and i32 %v249.i.382.i.i, 16
  %v251.i.384.i.i = add nuw nsw i32 %v250.i.383.i.i, 16
  %v252.i.385.i.i = xor i32 %v250.i.383.i.i, 16
  %v253.i.386.i.i = add nuw nsw i32 %v252.i.385.i.i, 16
  %v254.i.387.i.i = and i32 %arg0.int.i.i.i, %v251.i.384.i.i
  %v255.i.388.i.i = icmp ne i32 %v254.i.387.i.i, 0
  %v256.i.389.i.i = and i1 %v248.i.381.i.i, %v255.i.388.i.i
  %v257.i.390.i.i = and i32 %arg0.int.i.i.i, %v253.i.386.i.i
  %v258.i.391.i.i = icmp ne i32 %v257.i.390.i.i, 0
  %v259.i.392.i.i = and i1 %v248.i.381.i.i, %v258.i.391.i.i
  %.i0354.i.i.i = fsub fast float %.i0346.i.i.i, %currentRayData.i.i.0.1.i0.i.i.i
  %.i1355.i.i.i = fsub fast float %.i1347.i.i.i, %currentRayData.i.i.0.1.i1.i.i.i
  %.i2356.i.i.i = fsub fast float %.i2348.i.i.i, %currentRayData.i.i.0.1.i2.i.i.i
  store float %.i0354.i.i.i, float* %v92.i.301.i.i, align 4
  store float %.i1355.i.i.i, float* %v93.i.302.i.i, align 4
  store float %.i2356.i.i.i, float* %v94.i.303.i.i, align 4
  %v260.i.393.i.i = getelementptr [3 x float], [3 x float]* %v13.i.30.i.i, i32 0, i32 %currentRayData.i.i.5.1.i0.i.i.i
  %v261.i.394.i.i = load float, float* %v260.i.393.i.i, align 4, !tbaa !91, !noalias !119
  %v262.i.395.i.i = getelementptr [3 x float], [3 x float]* %v13.i.30.i.i, i32 0, i32 %currentRayData.i.i.5.1.i1.i.i.i
  %v263.i.396.i.i = load float, float* %v262.i.395.i.i, align 4, !tbaa !91, !noalias !119
  %v264.i.397.i.i = getelementptr [3 x float], [3 x float]* %v13.i.30.i.i, i32 0, i32 %currentRayData.i.i.5.1.i2.i.i.i
  %v265.i.398.i.i = load float, float* %v264.i.397.i.i, align 4, !tbaa !91, !noalias !119
  %.i0357.i.i.i = fsub fast float %.i3349.i.i.i, %currentRayData.i.i.0.1.i0.i.i.i
  %.i1358.i.i.i = fsub fast float %.i0350.i.i.i, %currentRayData.i.i.0.1.i1.i.i.i
  %.i2359.i.i.i = fsub fast float %.i1351.i.i.i, %currentRayData.i.i.0.1.i2.i.i.i
  store float %.i0357.i.i.i, float* %v95.i.304.i.i, align 4
  store float %.i1358.i.i.i, float* %v96.i.305.i.i, align 4
  store float %.i2359.i.i.i, float* %v97.i.306.i.i, align 4
  %v266.i.399.i.i = getelementptr [3 x float], [3 x float]* %v11.i.28.i.i, i32 0, i32 %currentRayData.i.i.5.1.i0.i.i.i
  %v267.i.400.i.i = load float, float* %v266.i.399.i.i, align 4, !tbaa !91, !noalias !119
  %v268.i.401.i.i = getelementptr [3 x float], [3 x float]* %v11.i.28.i.i, i32 0, i32 %currentRayData.i.i.5.1.i1.i.i.i
  %v269.i.402.i.i = load float, float* %v268.i.401.i.i, align 4, !tbaa !91, !noalias !119
  %v270.i.403.i.i = getelementptr [3 x float], [3 x float]* %v11.i.28.i.i, i32 0, i32 %currentRayData.i.i.5.1.i2.i.i.i
  %v271.i.404.i.i = load float, float* %v270.i.403.i.i, align 4, !tbaa !91, !noalias !119
  %.i0360.i.i.i = fsub fast float %.i2352.i.i.i, %currentRayData.i.i.0.1.i0.i.i.i
  %.i1361.i.i.i = fsub fast float %.i3353.i.i.i, %currentRayData.i.i.0.1.i1.i.i.i
  %.i2362.i.i.i = fsub fast float %v246.i.379.i.i, %currentRayData.i.i.0.1.i2.i.i.i
  store float %.i0360.i.i.i, float* %v98.i.307.i.i, align 4
  store float %.i1361.i.i.i, float* %v99.i.308.i.i, align 4
  store float %.i2362.i.i.i, float* %v100.i.309.i.i, align 4
  %v272.i.405.i.i = getelementptr [3 x float], [3 x float]* %v12.i.29.i.i, i32 0, i32 %currentRayData.i.i.5.1.i0.i.i.i
  %v273.i.406.i.i = load float, float* %v272.i.405.i.i, align 4, !tbaa !91, !noalias !119
  %v274.i.407.i.i = getelementptr [3 x float], [3 x float]* %v12.i.29.i.i, i32 0, i32 %currentRayData.i.i.5.1.i1.i.i.i
  %v275.i.408.i.i = load float, float* %v274.i.407.i.i, align 4, !tbaa !91, !noalias !119
  %v276.i.409.i.i = getelementptr [3 x float], [3 x float]* %v12.i.29.i.i, i32 0, i32 %currentRayData.i.i.5.1.i2.i.i.i
  %v277.i.410.i.i = load float, float* %v276.i.409.i.i, align 4, !tbaa !91, !noalias !119
  %.i0363.i.i.i = fmul float %currentRayData.i.i.4.1.i0.i.i.i, %v265.i.398.i.i
  %.i1364.i.i.i = fmul float %currentRayData.i.i.4.1.i1.i.i.i, %v265.i.398.i.i
  %.i0366.i.i.i = fsub float %v261.i.394.i.i, %.i0363.i.i.i
  %.i1367.i.i.i = fsub float %v263.i.396.i.i, %.i1364.i.i.i
  %.i0368.i.411.i.i = fmul float %currentRayData.i.i.4.1.i0.i.i.i, %v271.i.404.i.i
  %.i1369.i.412.i.i = fmul float %currentRayData.i.i.4.1.i1.i.i.i, %v271.i.404.i.i
  %.i0371.i.413.i.i = fsub float %v267.i.400.i.i, %.i0368.i.411.i.i
  %.i1372.i.414.i.i = fsub float %v269.i.402.i.i, %.i1369.i.412.i.i
  %.i0373.i.i.i = fmul float %currentRayData.i.i.4.1.i0.i.i.i, %v277.i.410.i.i
  %.i1374.i.i.i = fmul float %currentRayData.i.i.4.1.i1.i.i.i, %v277.i.410.i.i
  %.i0376.i.i.i = fsub float %v273.i.406.i.i, %.i0373.i.i.i
  %.i1377.i.i.i = fsub float %v275.i.408.i.i, %.i1374.i.i.i
  %v278.i.415.i.i = fmul float %.i1372.i.414.i.i, %.i0376.i.i.i
  %v279.i.416.i.i = fmul float %.i0371.i.413.i.i, %.i1377.i.i.i
  %v280.i.417.i.i = fsub float %v278.i.415.i.i, %v279.i.416.i.i
  %v281.i.418.i.i = fmul float %.i0366.i.i.i, %.i1377.i.i.i
  %v282.i.419.i.i = fmul float %.i1367.i.i.i, %.i0376.i.i.i
  %v283.i.420.i.i = fsub float %v281.i.418.i.i, %v282.i.419.i.i
  %v284.i.421.i.i = fmul float %.i1367.i.i.i, %.i0371.i.413.i.i
  %v285.i.422.i.i = fmul float %.i0366.i.i.i, %.i1372.i.414.i.i
  %v286.i.423.i.i = fsub float %v284.i.421.i.i, %v285.i.422.i.i
  %v287.i.424.i.i = fadd fast float %v283.i.420.i.i, %v286.i.423.i.i
  %v288.i.425.i.i = fadd fast float %v287.i.424.i.i, %v280.i.417.i.i
  br i1 %v259.i.392.i.i, label %121, label %111

; <label>:111                                     ; preds = %"\01?IsOpaque@@YA_N_NII@Z.exit.i.i.i.i.i"
  br i1 %v256.i.389.i.i, label %120, label %112

; <label>:112                                     ; preds = %111
  %v299.i.426.i.i = fcmp fast olt float %v280.i.417.i.i, 0.000000e+00
  %v300.i.427.i.i = fcmp fast olt float %v283.i.420.i.i, 0.000000e+00
  %v301.i.428.i.i = or i1 %v299.i.426.i.i, %v300.i.427.i.i
  %v302.i.429.i.i = fcmp fast olt float %v286.i.423.i.i, 0.000000e+00
  %v303.i.430.i.i = or i1 %v302.i.429.i.i, %v301.i.428.i.i
  %v304.i.i.i = fcmp fast ogt float %v280.i.417.i.i, 0.000000e+00
  %v305.i.431.i.i = fcmp fast ogt float %v283.i.420.i.i, 0.000000e+00
  %v306.i.432.i.i = or i1 %v304.i.i.i, %v305.i.431.i.i
  %v307.i.433.i.i = fcmp fast ogt float %v286.i.423.i.i, 0.000000e+00
  %v308.i.434.i.i = or i1 %v307.i.433.i.i, %v306.i.432.i.i
  %v309.i.435.i.i = and i1 %v303.i.430.i.i, %v308.i.434.i.i
  %v310.i.436.i.i = fcmp fast oeq float %v288.i.425.i.i, 0.000000e+00
  %or.cond193.i.i.i = or i1 %v310.i.436.i.i, %v309.i.435.i.i
  br i1 %or.cond193.i.i.i, label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i", label %._crit_edge.3.i.i.i

._crit_edge.3.i.i.i:                              ; preds = %121, %120, %112
  %v311.i.437.i.i = fmul fast float %v280.i.417.i.i, %v265.i.398.i.i
  %v312.i.438.i.i = fmul fast float %v283.i.420.i.i, %v271.i.404.i.i
  %v313.i.439.i.i = fmul fast float %v286.i.423.i.i, %v277.i.410.i.i
  %tmp.i.i.i = fadd fast float %v312.i.438.i.i, %v313.i.439.i.i
  %tmp517.i.i.i = fadd fast float %tmp.i.i.i, %v311.i.437.i.i
  %tmp518.i.i.i = fmul fast float %tmp517.i.i.i, %currentRayData.i.i.4.1.i2.i.i.i
  br i1 %v259.i.392.i.i, label %116, label %113

; <label>:113                                     ; preds = %._crit_edge.3.i.i.i
  br i1 %v256.i.389.i.i, label %115, label %114

; <label>:114                                     ; preds = %113
  %v322.i.440.i.i = fcmp fast ogt float %v288.i.425.i.i, 0.000000e+00
  %v323.i.441.i.i = select i1 %v322.i.440.i.i, i32 1, i32 -1
  %v324.i.442.i.i = bitcast float %tmp518.i.i.i to i32
  %v325.i.443.i.i = xor i32 %v324.i.442.i.i, %v323.i.441.i.i
  %v326.i.444.i.i = uitofp i32 %v325.i.443.i.i to float
  %v327.i.445.i.i = bitcast float %v288.i.425.i.i to i32
  %v328.i.446.i.i = xor i32 %v323.i.441.i.i, %v327.i.445.i.i
  %v329.i.447.i.i = uitofp i32 %v328.i.446.i.i to float
  %v330.i.448.i.i = fmul fast float %v329.i.447.i.i, %98
  %v331.i.449.i.i = fcmp fast ogt float %v326.i.444.i.i, %v330.i.448.i.i
  br i1 %v331.i.449.i.i, label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i", label %._crit_edge.5.i.i.i

._crit_edge.5.i.i.i:                              ; preds = %116, %115, %114
  %v332.i.450.i.i = fdiv fast float 1.000000e+00, %v288.i.425.i.i
  %v333.i.451.i.i = fmul fast float %v332.i.450.i.i, %v283.i.420.i.i
  %v334.i.452.i.i = fmul fast float %v332.i.450.i.i, %v286.i.423.i.i
  %v335.i.453.i.i = fmul fast float %v332.i.450.i.i, %tmp518.i.i.i
  br label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i"

; <label>:115                                     ; preds = %113
  %v318.i.454.i.i = fcmp fast olt float %tmp518.i.i.i, 0.000000e+00
  %v319.i.455.i.i = fmul fast float %v288.i.425.i.i, %98
  %v320.i.456.i.i = fcmp fast ogt float %tmp518.i.i.i, %v319.i.455.i.i
  %v321.i.457.i.i = or i1 %v318.i.454.i.i, %v320.i.456.i.i
  br i1 %v321.i.457.i.i, label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i", label %._crit_edge.5.i.i.i

; <label>:116                                     ; preds = %._crit_edge.3.i.i.i
  %v314.i.458.i.i = fcmp fast ogt float %tmp518.i.i.i, 0.000000e+00
  %v315.i.459.i.i = fmul fast float %v288.i.425.i.i, %98
  %v316.i.460.i.i = fcmp fast olt float %tmp518.i.i.i, %v315.i.459.i.i
  %v317.i.461.i.i = or i1 %v314.i.458.i.i, %v316.i.460.i.i
  br i1 %v317.i.461.i.i, label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i", label %._crit_edge.5.i.i.i

"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i": ; preds = %121, %120, %116, %115, %114, %112, %._crit_edge.5.i.i.i
  %.2.i0.i.i.i = phi float [ %v333.i.451.i.i, %._crit_edge.5.i.i.i ], [ %.1.i0.i.i.i, %112 ], [ %.1.i0.i.i.i, %114 ], [ %.1.i0.i.i.i, %115 ], [ %.1.i0.i.i.i, %116 ], [ %.1.i0.i.i.i, %120 ], [ %.1.i0.i.i.i, %121 ]
  %.2.i1.i.i.i = phi float [ %v334.i.452.i.i, %._crit_edge.5.i.i.i ], [ %.1.i1.i.i.i, %112 ], [ %.1.i1.i.i.i, %114 ], [ %.1.i1.i.i.i, %115 ], [ %.1.i1.i.i.i, %116 ], [ %.1.i1.i.i.i, %120 ], [ %.1.i1.i.i.i, %121 ]
  %.0173.i.i.i = phi float [ %v335.i.453.i.i, %._crit_edge.5.i.i.i ], [ %98, %112 ], [ %98, %114 ], [ %98, %115 ], [ %98, %116 ], [ %98, %120 ], [ %98, %121 ]
  %v336.i.462.i.i = fcmp fast olt float %.0173.i.i.i, %98
  %v337.i.463.i.i = fcmp fast ogt float %.0173.i.i.i, %67
  %v338.i.464.i.i = and i1 %v336.i.462.i.i, %v337.i.463.i.i
  %..i.i.i = select i1 %v338.i.464.i.i, i1 true, i1 false
  %.resultTriId.i.i.1.i.i.i = select i1 %v338.i.464.i.i, i32 %v224.i.357.i.i, i32 %resultTriId.i.i.1.i.i.i
  %.0173.RayTCurrent.i.i.i = select i1 %v338.i.464.i.i, float %.0173.i.i.i, float %98
  %.2.i0.resultBary.i.i.1.i0.i.i.i = select i1 %v338.i.464.i.i, float %.2.i0.i.i.i, float %resultBary.i.i.1.i0.i.i.i
  %.2.i1.resultBary.i.i.1.i1.i.i.i = select i1 %v338.i.464.i.i, float %.2.i1.i.i.i, float %resultBary.i.i.1.i1.i.i.i
  %v339.i.465.i.i = and i1 %v223.i.356.i.i, %..i.i.i
  br i1 %v339.i.465.i.i, label %._crit_edge.10.i.i.i, label %._crit_edge.1.i.i.i

._crit_edge.10.i.i.i:                             ; preds = %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i"
  %v341.i.467.i.i = add i32 %currentGpuVA.i.i.1.i0.i.i.i, 8
  %v343.i.469.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %DescriptorHeapBufferTable_UAV_rawbuf, i32 %v341.i.467.i.i, i32 undef) #2
  %v344.i.470.i.i = extractvalue %dx.types.ResRet.i32 %v343.i.469.i.i, 0
  %v345.i.471.i.i = shl i32 %.resultTriId.i.i.1.i.i.i, 3
  %v346.i.472.i.i = add i32 %v345.i.471.i.i, %currentGpuVA.i.i.1.i0.i.i.i
  %v347.i.473.i.i = add i32 %v346.i.472.i.i, %v344.i.470.i.i
  %v348.i.474.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %DescriptorHeapBufferTable_UAV_rawbuf, i32 %v347.i.473.i.i, i32 undef) #2
  %v349.i.475.i.i = extractvalue %dx.types.ResRet.i32 %v348.i.474.i.i, 0
  %v350.i.476.i.i = extractvalue %dx.types.ResRet.i32 %v348.i.474.i.i, 1
  %v351.i.477.i.i = mul i32 %v349.i.475.i.i, %arg3.int.i.i.i
  %v352.i.478.i.i = add i32 %instOffset.i.i.1.i.i.i, %arg2.int.i.i.i
  %v353.i.479.i.i = add i32 %v352.i.478.i.i, %v351.i.477.i.i
  %117 = bitcast float %.2.i0.resultBary.i.i.1.i0.i.i.i to i32
  %arrayidx.i.42.i.482.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %93
  store i32 %117, i32* %arrayidx.i.42.i.482.i.i
  %118 = bitcast float %.2.i1.resultBary.i.i.1.i1.i.i.i to i32
  %add.i.38.i.483.i.i = add nsw i32 %93, 1
  %arrayidx.i.40.i.485.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.38.i.483.i.i
  store i32 %118, i32* %arrayidx.i.40.i.485.i.i
  %v354.i.486.i.i = icmp eq i32 %ResetMatrices.i.i.1.i.i.i, 0
  %.i0109.CurrentWorldToObject.i.i168.1.i0.i.i.i = select i1 %v354.i.486.i.i, float %.i0109, float %CurrentWorldToObject.i.i168.1.i0.i.i.i
  %.i1110.CurrentWorldToObject.i.i168.1.i1.i.i.i = select i1 %v354.i.486.i.i, float %.i1110, float %CurrentWorldToObject.i.i168.1.i1.i.i.i
  %.i2111.CurrentWorldToObject.i.i168.1.i2.i.i.i = select i1 %v354.i.486.i.i, float %.i2111, float %CurrentWorldToObject.i.i168.1.i2.i.i.i
  %.i3112.CurrentWorldToObject.i.i168.1.i3.i.i.i = select i1 %v354.i.486.i.i, float %.i3112, float %CurrentWorldToObject.i.i168.1.i3.i.i.i
  %.i4113.CurrentWorldToObject.i.i168.1.i4.i.i.i = select i1 %v354.i.486.i.i, float %.i4113, float %CurrentWorldToObject.i.i168.1.i4.i.i.i
  %.i5114.CurrentWorldToObject.i.i168.1.i5.i.i.i = select i1 %v354.i.486.i.i, float %.i5114, float %CurrentWorldToObject.i.i168.1.i5.i.i.i
  %.i6115.CurrentWorldToObject.i.i168.1.i6.i.i.i = select i1 %v354.i.486.i.i, float %.i6115, float %CurrentWorldToObject.i.i168.1.i6.i.i.i
  %.i7116.CurrentWorldToObject.i.i168.1.i7.i.i.i = select i1 %v354.i.486.i.i, float %.i7116, float %CurrentWorldToObject.i.i168.1.i7.i.i.i
  %.i8117.CurrentWorldToObject.i.i168.1.i8.i.i.i = select i1 %v354.i.486.i.i, float %.i8117, float %CurrentWorldToObject.i.i168.1.i8.i.i.i
  %.i9118.CurrentWorldToObject.i.i168.1.i9.i.i.i = select i1 %v354.i.486.i.i, float %.i9118, float %CurrentWorldToObject.i.i168.1.i9.i.i.i
  %.i10119.CurrentWorldToObject.i.i168.1.i10.i.i.i = select i1 %v354.i.486.i.i, float %.i10119, float %CurrentWorldToObject.i.i168.1.i10.i.i.i
  %.i11120.CurrentWorldToObject.i.i168.1.i11.i.i.i = select i1 %v354.i.486.i.i, float %.i11120, float %CurrentWorldToObject.i.i168.1.i11.i.i.i
  %.i0121.CurrentObjectToWorld.i.i167.1.i0.i.i.i = select i1 %v354.i.486.i.i, float %.i0121, float %CurrentObjectToWorld.i.i167.1.i0.i.i.i
  %.i1122.CurrentObjectToWorld.i.i167.1.i1.i.i.i = select i1 %v354.i.486.i.i, float %.i1122, float %CurrentObjectToWorld.i.i167.1.i1.i.i.i
  %.i2123.CurrentObjectToWorld.i.i167.1.i2.i.i.i = select i1 %v354.i.486.i.i, float %.i2123, float %CurrentObjectToWorld.i.i167.1.i2.i.i.i
  %.i3124.CurrentObjectToWorld.i.i167.1.i3.i.i.i = select i1 %v354.i.486.i.i, float %.i3124, float %CurrentObjectToWorld.i.i167.1.i3.i.i.i
  %.i4125.CurrentObjectToWorld.i.i167.1.i4.i.i.i = select i1 %v354.i.486.i.i, float %.i4125, float %CurrentObjectToWorld.i.i167.1.i4.i.i.i
  %.i5126.CurrentObjectToWorld.i.i167.1.i5.i.i.i = select i1 %v354.i.486.i.i, float %.i5126, float %CurrentObjectToWorld.i.i167.1.i5.i.i.i
  %.i6127.CurrentObjectToWorld.i.i167.1.i6.i.i.i = select i1 %v354.i.486.i.i, float %.i6127, float %CurrentObjectToWorld.i.i167.1.i6.i.i.i
  %.i7128.CurrentObjectToWorld.i.i167.1.i7.i.i.i = select i1 %v354.i.486.i.i, float %.i7128, float %CurrentObjectToWorld.i.i167.1.i7.i.i.i
  %.i8129.CurrentObjectToWorld.i.i167.1.i8.i.i.i = select i1 %v354.i.486.i.i, float %.i8129, float %CurrentObjectToWorld.i.i167.1.i8.i.i.i
  %.i9130.CurrentObjectToWorld.i.i167.1.i9.i.i.i = select i1 %v354.i.486.i.i, float %.i9130, float %CurrentObjectToWorld.i.i167.1.i9.i.i.i
  %.i10131.CurrentObjectToWorld.i.i167.1.i10.i.i.i = select i1 %v354.i.486.i.i, float %.i10131, float %CurrentObjectToWorld.i.i167.1.i10.i.i.i
  %.i11132.CurrentObjectToWorld.i.i167.1.i11.i.i.i = select i1 %v354.i.486.i.i, float %.i11132, float %CurrentObjectToWorld.i.i167.1.i11.i.i.i
  %v367.i.506.i.i = and i32 %arg0.int.i.i.i, 4
  %v368.i.507.i.i = icmp eq i32 %v367.i.506.i.i, 0
  br i1 %v368.i.507.i.i, label %._crit_edge.1.i.i.i, label %119

; <label>:119                                     ; preds = %._crit_edge.10.i.i.i
  store i32 0, i32* %v91.i.300.i.i, align 4, !tbaa !105
  store i32 0, i32* %v56.i.262.i.i, align 4, !tbaa !105
  br label %._crit_edge.1.i.i.i

; <label>:120                                     ; preds = %111
  %v294.i.508.i.i = fcmp fast olt float %v280.i.417.i.i, 0.000000e+00
  %v295.i.509.i.i = fcmp fast olt float %v283.i.420.i.i, 0.000000e+00
  %v296.i.510.i.i = or i1 %v294.i.508.i.i, %v295.i.509.i.i
  %v297.i.511.i.i = fcmp fast olt float %v286.i.423.i.i, 0.000000e+00
  %v298.i.512.i.i = or i1 %v297.i.511.i.i, %v296.i.510.i.i
  %.old.i.i.i = fcmp fast oeq float %v288.i.425.i.i, 0.000000e+00
  %or.cond194.i.i.i = or i1 %v298.i.512.i.i, %.old.i.i.i
  br i1 %or.cond194.i.i.i, label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i", label %._crit_edge.3.i.i.i

; <label>:121                                     ; preds = %"\01?IsOpaque@@YA_N_NII@Z.exit.i.i.i.i.i"
  %v289.i.513.i.i = fcmp fast ogt float %v280.i.417.i.i, 0.000000e+00
  %v290.i.514.i.i = fcmp fast ogt float %v283.i.420.i.i, 0.000000e+00
  %v291.i.515.i.i = or i1 %v289.i.513.i.i, %v290.i.514.i.i
  %v292.i.516.i.i = fcmp fast ogt float %v286.i.423.i.i, 0.000000e+00
  %v293.i.517.i.i = or i1 %v292.i.516.i.i, %v291.i.515.i.i
  %.old.old.i.i.i = fcmp fast oeq float %v288.i.425.i.i, 0.000000e+00
  %or.cond195.i.i.i = or i1 %v293.i.517.i.i, %.old.old.i.i.i
  br i1 %or.cond195.i.i.i, label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i", label %._crit_edge.3.i.i.i

"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i": ; preds = %110
  %v123.i.518.i.i = and i32 %v117.i.326.i.i, 2147483647
  %v124.i.519.i.i = mul i32 %v123.i.518.i.i, 112
  %v125.i.520.i.i = add i32 %v63.i.269.i.i, %v124.i.519.i.i
  %v127.i.522.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %DescriptorHeapBufferTable_UAV_rawbuf336, i32 %v125.i.520.i.i, i32 undef) #2
  %v128.i.523.i.i = extractvalue %dx.types.ResRet.i32 %v127.i.522.i.i, 0
  %v129.i.524.i.i = extractvalue %dx.types.ResRet.i32 %v127.i.522.i.i, 1
  %v130.i.525.i.i = extractvalue %dx.types.ResRet.i32 %v127.i.522.i.i, 2
  %v131.i.526.i.i = extractvalue %dx.types.ResRet.i32 %v127.i.522.i.i, 3
  %v132.i.527.i.i = add i32 %v125.i.520.i.i, 16
  %v133.i.528.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %DescriptorHeapBufferTable_UAV_rawbuf336, i32 %v132.i.527.i.i, i32 undef) #2
  %v134.i.529.i.i = extractvalue %dx.types.ResRet.i32 %v133.i.528.i.i, 0
  %v135.i.530.i.i = extractvalue %dx.types.ResRet.i32 %v133.i.528.i.i, 1
  %v136.i.531.i.i = extractvalue %dx.types.ResRet.i32 %v133.i.528.i.i, 2
  %v137.i.532.i.i = extractvalue %dx.types.ResRet.i32 %v133.i.528.i.i, 3
  %v138.i.533.i.i = add i32 %v125.i.520.i.i, 32
  %v139.i.534.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %DescriptorHeapBufferTable_UAV_rawbuf336, i32 %v138.i.533.i.i, i32 undef) #2
  %v140.i.535.i.i = extractvalue %dx.types.ResRet.i32 %v139.i.534.i.i, 0
  %v141.i.536.i.i = extractvalue %dx.types.ResRet.i32 %v139.i.534.i.i, 1
  %v142.i.537.i.i = extractvalue %dx.types.ResRet.i32 %v139.i.534.i.i, 2
  %v143.i.538.i.i = extractvalue %dx.types.ResRet.i32 %v139.i.534.i.i, 3
  %v144.i.539.i.i = add i32 %v125.i.520.i.i, 48
  %v145.i.540.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %DescriptorHeapBufferTable_UAV_rawbuf336, i32 %v144.i.539.i.i, i32 undef) #2
  %v146.i.541.i.i = extractvalue %dx.types.ResRet.i32 %v145.i.540.i.i, 0
  %v147.i.542.i.i = extractvalue %dx.types.ResRet.i32 %v145.i.540.i.i, 1
  %v148.i.543.i.i = extractvalue %dx.types.ResRet.i32 %v145.i.540.i.i, 2
  %v149.i.544.i.i = extractvalue %dx.types.ResRet.i32 %v145.i.540.i.i, 3
  %v150.i.545.i.i = add i32 %v125.i.520.i.i, 64
  %v151.i.546.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %DescriptorHeapBufferTable_UAV_rawbuf336, i32 %v150.i.545.i.i, i32 undef) #2
  %v152.i.547.i.i = extractvalue %dx.types.ResRet.i32 %v151.i.546.i.i, 0
  %v153.i.548.i.i = extractvalue %dx.types.ResRet.i32 %v151.i.546.i.i, 1
  %v154.i.549.i.i = extractvalue %dx.types.ResRet.i32 %v151.i.546.i.i, 2
  %v155.i.550.i.i = extractvalue %dx.types.ResRet.i32 %v151.i.546.i.i, 3
  %v156.i.551.i.i = add i32 %v125.i.520.i.i, 80
  %v157.i.552.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %DescriptorHeapBufferTable_UAV_rawbuf336, i32 %v156.i.551.i.i, i32 undef) #2
  %v158.i.553.i.i = extractvalue %dx.types.ResRet.i32 %v157.i.552.i.i, 0
  %v159.i.554.i.i = extractvalue %dx.types.ResRet.i32 %v157.i.552.i.i, 1
  %v160.i.555.i.i = extractvalue %dx.types.ResRet.i32 %v157.i.552.i.i, 2
  %v161.i.556.i.i = extractvalue %dx.types.ResRet.i32 %v157.i.552.i.i, 3
  %v162.i.557.i.i = add i32 %v125.i.520.i.i, 96
  %v163.i.558.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %DescriptorHeapBufferTable_UAV_rawbuf336, i32 %v162.i.557.i.i, i32 undef) #2
  %v164.i.559.i.i = extractvalue %dx.types.ResRet.i32 %v163.i.558.i.i, 0
  %v165.i.560.i.i = extractvalue %dx.types.ResRet.i32 %v163.i.558.i.i, 1
  %v166.i.561.i.i = extractvalue %dx.types.ResRet.i32 %v163.i.558.i.i, 2
  %v167.i.562.i.i = extractvalue %dx.types.ResRet.i32 %v163.i.558.i.i, 3
  %v168.i.563.i.i = bitcast i32 %v128.i.523.i.i to float
  %v169.i.564.i.i = bitcast i32 %v129.i.524.i.i to float
  %v170.i.565.i.i = bitcast i32 %v130.i.525.i.i to float
  %v171.i.566.i.i = bitcast i32 %v131.i.526.i.i to float
  %v172.i.567.i.i = bitcast i32 %v134.i.529.i.i to float
  %v173.i.568.i.i = bitcast i32 %v135.i.530.i.i to float
  %v174.i.569.i.i = bitcast i32 %v136.i.531.i.i to float
  %v175.i.570.i.i = bitcast i32 %v137.i.532.i.i to float
  %v176.i.571.i.i = bitcast i32 %v140.i.535.i.i to float
  %v177.i.572.i.i = bitcast i32 %v141.i.536.i.i to float
  %v178.i.573.i.i = bitcast i32 %v142.i.537.i.i to float
  %v179.i.574.i.i = bitcast i32 %v143.i.538.i.i to float
  %v180.i.575.i.i = and i32 %v147.i.542.i.i, 16777215
  %v181.i.576.i.i = and i32 %v146.i.541.i.i, 16777215
  %v182.i.577.i.i = lshr i32 %v146.i.541.i.i, 24
  %v183.i.578.i.i = and i32 %v182.i.577.i.i, %arg1.int.i.i.i
  %v184.i.579.i.i = icmp eq i32 %v183.i.578.i.i, 0
  %v185.i.580.i.i = bitcast i32 %v152.i.547.i.i to float
  %v186.i.581.i.i = bitcast i32 %v153.i.548.i.i to float
  %v187.i.582.i.i = bitcast i32 %v154.i.549.i.i to float
  %v188.i.583.i.i = bitcast i32 %v155.i.550.i.i to float
  %v189.i.584.i.i = bitcast i32 %v158.i.553.i.i to float
  %v190.i.585.i.i = bitcast i32 %v159.i.554.i.i to float
  %v191.i.586.i.i = bitcast i32 %v160.i.555.i.i to float
  %v192.i.587.i.i = bitcast i32 %v161.i.556.i.i to float
  %v193.i.588.i.i = bitcast i32 %v164.i.559.i.i to float
  %v194.i.589.i.i = bitcast i32 %v165.i.560.i.i to float
  %v195.i.590.i.i = bitcast i32 %v166.i.561.i.i to float
  %v196.i.591.i.i = bitcast i32 %v167.i.562.i.i to float
  br i1 %v184.i.579.i.i, label %._crit_edge.1.i.i.i, label %122

; <label>:122                                     ; preds = %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i"
  store i32 0, i32 addrspace(3)* %v106.i.315.i.i, align 4, !tbaa !105, !noalias !127
  %v197.i.592.i.i = lshr i32 %v147.i.542.i.i, 24
  %v198.i.593.i.i = fmul fast float %68, %v168.i.563.i.i
  %FMad285.i.i.i = call float @dx.op.tertiary.f32(i32 46, float %69, float %v169.i.564.i.i, float %v198.i.593.i.i) #2
  %FMad284.i.i.i = call float @dx.op.tertiary.f32(i32 46, float %70, float %v170.i.565.i.i, float %FMad285.i.i.i) #2
  %FMad283.i.i.i = call float @dx.op.tertiary.f32(i32 46, float 0.000000e+00, float %v171.i.566.i.i, float %FMad284.i.i.i) #2
  %v199.i.594.i.i = fmul fast float %68, %v172.i.567.i.i
  %FMad282.i.i.i = call float @dx.op.tertiary.f32(i32 46, float %69, float %v173.i.568.i.i, float %v199.i.594.i.i) #2
  %FMad281.i.i.i = call float @dx.op.tertiary.f32(i32 46, float %70, float %v174.i.569.i.i, float %FMad282.i.i.i) #2
  %FMad280.i.i.i = call float @dx.op.tertiary.f32(i32 46, float 0.000000e+00, float %v175.i.570.i.i, float %FMad281.i.i.i) #2
  %v200.i.595.i.i = fmul fast float %68, %v176.i.571.i.i
  %FMad279.i.i.i = call float @dx.op.tertiary.f32(i32 46, float %69, float %v177.i.572.i.i, float %v200.i.595.i.i) #2
  %FMad278.i.i.i = call float @dx.op.tertiary.f32(i32 46, float %70, float %v178.i.573.i.i, float %FMad279.i.i.i) #2
  %FMad.i.596.i.i = call float @dx.op.tertiary.f32(i32 46, float 0.000000e+00, float %v179.i.574.i.i, float %FMad278.i.i.i) #2
  %v201.i.597.i.i = fmul fast float %64, %v168.i.563.i.i
  %FMad294.i.i.i = call float @dx.op.tertiary.f32(i32 46, float %65, float %v169.i.564.i.i, float %v201.i.597.i.i) #2
  %FMad293.i.i.i = call float @dx.op.tertiary.f32(i32 46, float %66, float %v170.i.565.i.i, float %FMad294.i.i.i) #2
  %FMad292.i.i.i = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %v171.i.566.i.i, float %FMad293.i.i.i) #2
  %v202.i.598.i.i = fmul fast float %64, %v172.i.567.i.i
  %FMad291.i.i.i = call float @dx.op.tertiary.f32(i32 46, float %65, float %v173.i.568.i.i, float %v202.i.598.i.i) #2
  %FMad290.i.i.i = call float @dx.op.tertiary.f32(i32 46, float %66, float %v174.i.569.i.i, float %FMad291.i.i.i) #2
  %FMad289.i.i.i = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %v175.i.570.i.i, float %FMad290.i.i.i) #2
  %v203.i.599.i.i = fmul fast float %64, %v176.i.571.i.i
  %FMad288.i.i.i = call float @dx.op.tertiary.f32(i32 46, float %65, float %v177.i.572.i.i, float %v203.i.599.i.i) #2
  %FMad287.i.i.i = call float @dx.op.tertiary.f32(i32 46, float %66, float %v178.i.573.i.i, float %FMad288.i.i.i) #2
  %FMad286.i.i.i = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %v179.i.574.i.i, float %FMad287.i.i.i) #2
  store float %FMad283.i.i.i, float* %v88.i.297.i.i, align 4
  store float %FMad280.i.i.i, float* %v89.i.298.i.i, align 4
  store float %FMad.i.596.i.i, float* %v90.i.299.i.i, align 4
  %.i0340.i.600.i.i = fdiv fast float 1.000000e+00, %FMad283.i.i.i
  %.i1341.i.601.i.i = fdiv fast float 1.000000e+00, %FMad280.i.i.i
  %.i2342.i.i.i = fdiv fast float 1.000000e+00, %FMad.i.596.i.i
  %.i0343.i.i.i = fmul fast float %.i0340.i.600.i.i, %FMad292.i.i.i
  %.i1344.i.i.i = fmul fast float %.i1341.i.601.i.i, %FMad289.i.i.i
  %.i2345.i.i.i = fmul fast float %.i2342.i.i.i, %FMad286.i.i.i
  %FAbs.i.602.i.i = call float @dx.op.unary.f32(i32 6, float %FMad283.i.i.i) #2
  %FAbs221.i.i.i = call float @dx.op.unary.f32(i32 6, float %FMad280.i.i.i) #2
  %FAbs222.i.i.i = call float @dx.op.unary.f32(i32 6, float %FMad.i.596.i.i) #2
  %v204.i.603.i.i = fcmp fast ogt float %FAbs.i.602.i.i, %FAbs221.i.i.i
  %v205.i.604.i.i = fcmp fast ogt float %FAbs.i.602.i.i, %FAbs222.i.i.i
  %v206.i.605.i.i = and i1 %v204.i.603.i.i, %v205.i.604.i.i
  br i1 %v206.i.605.i.i, label %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i", label %123

; <label>:123                                     ; preds = %122
  %v207.i.606.i.i = fcmp fast ogt float %FAbs221.i.i.i, %FAbs222.i.i.i
  br i1 %v207.i.606.i.i, label %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i", label %124

; <label>:124                                     ; preds = %123
  br label %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i"

"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i": ; preds = %123, %122, %124
  %.0172.i.i.i = phi i32 [ 2, %124 ], [ 0, %122 ], [ 1, %123 ]
  %v208.i.607.i.i = add nuw nsw i32 %.0172.i.i.i, 1
  %v209.i.608.i.i = urem i32 %v208.i.607.i.i, 3
  %v210.i.609.i.i = add nuw nsw i32 %.0172.i.i.i, 2
  %v211.i.610.i.i = urem i32 %v210.i.609.i.i, 3
  %v212.i.611.i.i = getelementptr [3 x float], [3 x float]* %v14.i.31.i.i, i32 0, i32 %.0172.i.i.i
  %v213.i.612.i.i = load float, float* %v212.i.611.i.i, align 4, !tbaa !91, !noalias !130
  %v214.i.613.i.i = fcmp fast olt float %v213.i.612.i.i, 0.000000e+00
  %.5.0.i0.i.i.i = select i1 %v214.i.613.i.i, i32 %v211.i.610.i.i, i32 %v209.i.608.i.i
  %.5.0.i1.i.i.i = select i1 %v214.i.613.i.i, i32 %v209.i.608.i.i, i32 %v211.i.610.i.i
  %v215.i.614.i.i = getelementptr [3 x float], [3 x float]* %v14.i.31.i.i, i32 0, i32 %.5.0.i0.i.i.i
  %v216.i.615.i.i = load float, float* %v215.i.614.i.i, align 4, !tbaa !91, !noalias !130
  %v217.i.616.i.i = fdiv float %v216.i.615.i.i, %v213.i.612.i.i
  %v218.i.617.i.i = getelementptr [3 x float], [3 x float]* %v14.i.31.i.i, i32 0, i32 %.5.0.i1.i.i.i
  %v219.i.618.i.i = load float, float* %v218.i.617.i.i, align 4, !tbaa !91, !noalias !130
  %v220.i.619.i.i = fdiv float %v219.i.618.i.i, %v213.i.612.i.i
  %v221.i.620.i.i = fdiv float 1.000000e+00, %v213.i.612.i.i
  store i32 1, i32* %v91.i.300.i.i, align 4, !tbaa !105
  br label %._crit_edge.1.i.i.i

state_1006.Fallback_TraceRay.ss_1.i.i:            ; preds = %while.body.i
  %add.i.7.i.623.i.i = add nsw i32 %6, 4
  %arrayidx.i.9.i.625.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.7.i.623.i.i
  %v24.int.i.i.i = load i32, i32* %arrayidx.i.9.i.625.i.i
  %add.i.6.i.628.i.i = add nsw i32 %6, 24
  %add.i.i.630.i.i = add nsw i32 %add.i.6.i.628.i.i, -1
  %arrayidx.i.4.i.632.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.i.630.i.i
  %125 = load i32, i32* %arrayidx.i.4.i.632.i.i, align 4
  %add2.i.i.635.i.i = add nsw i32 %add.i.6.i.628.i.i, -2
  %arrayidx4.i.i.637.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add2.i.i.635.i.i
  %126 = load i32, i32* %arrayidx4.i.i.637.i.i, align 4
  %arrayidx.i.i.641.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.6.i.628.i.i
  %ret.stateId.i.642.i.i = load i32, i32* %arrayidx.i.i.641.i.i
  br label %while.cond.i

fb_Fallback_Scheduler.exit:                       ; preds = %0, %while.cond.i
  ret void
}

; Function Attrs: nounwind readonly
declare %dx.types.Handle @dx.op.createHandle(i32, i8, i32, i32, i1) #0

attributes #0 = { nounwind readonly }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind }

!llvm.ident = !{!0, !0, !0, !0, !0, !1}
!llvm.module.flags = !{!2, !3}
!dx.version = !{!4}
!dx.valver = !{!5}
!dx.shaderModel = !{!6}
!dx.resources = !{!7}
!dx.typeAnnotations = !{!34, !84}
!dx.entryPoints = !{!88}

!0 = !{!"clang version 3.7 (tags/RELEASE_370/final)"}
!1 = !{!"FallbackLayer"}
!2 = !{i32 2, !"Dwarf Version", i32 4}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{i32 1, i32 0}
!5 = !{i32 1, i32 2}
!6 = !{!"cs", i32 6, i32 0}
!7 = !{!8, !23, !26, !31}
!8 = !{!9, !11, !13, !14, !15, !16, !17, !18, !19, !20, !21, !22}
!9 = !{i32 0, [27 x %"class.Texture2D<vector<float, 4> >"]* undef, !"g_texNormal", i32 1, i32 32, i32 27, i32 2, i32 0, !10}
!10 = !{i32 0, i32 9}
!11 = !{i32 1, %"class.StructuredBuffer<RayTraceMeshInfo>"* undef, !"g_meshInfo", i32 1, i32 0, i32 1, i32 12, i32 0, !12}
!12 = !{i32 1, i32 28}
!13 = !{i32 2, [27 x %"class.Texture2D<vector<float, 4> >"]* undef, !"g_texDiffuse", i32 1, i32 5, i32 27, i32 2, i32 0, !10}
!14 = !{i32 3, %struct.ByteAddressBuffer* undef, !"RayGenShaderTable", i32 214743647, i32 2, i32 1, i32 11, i32 0, null}
!15 = !{i32 4, %struct.ByteAddressBuffer* undef, !"g_indices", i32 1, i32 1, i32 1, i32 11, i32 0, null}
!16 = !{i32 5, %"class.Texture2D<float>"* undef, !"texSSAO", i32 1, i32 4, i32 1, i32 2, i32 0, !10}
!17 = !{i32 6, %struct.ByteAddressBuffer* undef, !"g_attributes", i32 1, i32 2, i32 1, i32 11, i32 0, null}
!18 = !{i32 7, %struct.ByteAddressBuffer* undef, !"HitGroupShaderTable", i32 214743647, i32 0, i32 1, i32 11, i32 0, null}
!19 = !{i32 8, %"class.Texture2D<float>"* undef, !"depth", i32 0, i32 1, i32 1, i32 2, i32 0, !10}
!20 = !{i32 9, %"class.Texture2D<float>"* undef, !"texShadow", i32 1, i32 3, i32 1, i32 2, i32 0, !10}
!21 = !{i32 10, %struct.ByteAddressBuffer* undef, !"MissShaderTable", i32 214743647, i32 1, i32 1, i32 11, i32 0, null}
!22 = !{i32 11, %"class.Texture2D<vector<float, 3> >"* undef, !"normals", i32 0, i32 2, i32 1, i32 2, i32 0, !10}
!23 = !{!24, !25}
!24 = !{i32 0, %"class.RWTexture2D<vector<float, 4> >"* undef, !"g_screenOutput", i32 0, i32 2, i32 1, i32 2, i1 false, i1 false, i1 false, !10}
!25 = !{i32 1, [0 x %struct.RWByteAddressBuffer]* undef, !"DescriptorHeapBufferTable", i32 214743648, i32 0, i32 -1, i32 11, i1 false, i1 false, i1 false, null}
!26 = !{!27, !28, !29, !30}
!27 = !{i32 0, %dx.alignment.legacy.b1* undef, !"b1", i32 0, i32 1, i32 1, i32 88, null}
!28 = !{i32 1, %AccelerationStructureList* undef, !"AccelerationStructureList", i32 214743647, i32 1, i32 1, i32 8, null}
!29 = !{i32 2, %dx.alignment.legacy.HitShaderConstants* undef, !"HitShaderConstants", i32 0, i32 0, i32 1, i32 136, null}
!30 = !{i32 3, %Constants* undef, !"Constants", i32 214743647, i32 0, i32 1, i32 32, null}
!31 = !{!32, !33}
!32 = !{i32 0, %struct.SamplerComparisonState* undef, !"shadowSampler", i32 0, i32 1, i32 1, i32 1, null}
!33 = !{i32 1, %struct.SamplerState* undef, !"g_s0", i32 0, i32 0, i32 1, i32 0, null}
!34 = !{i32 0, %Constants undef, !35, %struct.ByteAddressBuffer undef, !44, %struct.RaytracingAccelerationStructure undef, !44, %b1 undef, !46, %struct.DynamicCB undef, !48, %"class.Texture2D<float>" undef, !54, %"class.Texture2D<float>::mips_type" undef, !57, %"class.Texture2D<vector<float, 3> >" undef, !59, %"class.Texture2D<vector<float, 3> >::mips_type" undef, !57, %AccelerationStructureList undef, !61, %struct.RWByteAddressBuffer undef, !44, %"class.RWTexture2D<vector<float, 4> >" undef, !63, %"class.Texture2D<vector<float, 4> >" undef, !64, %"class.Texture2D<vector<float, 4> >::mips_type" undef, !57, %"class.StructuredBuffer<RayTraceMeshInfo>" undef, !66, %struct.RayTraceMeshInfo undef, !68, %HitShaderConstants undef, !76, %dx.alignment.legacy.struct.DynamicCB undef, !48, %dx.alignment.legacy.b1 undef, !46, %dx.alignment.legacy.HitShaderConstants undef, !76}
!35 = !{i32 32, !36, !37, !38, !39, !40, !41, !42, !43}
!36 = !{i32 6, !"RayDispatchDimensionsWidth", i32 3, i32 0, i32 7, i32 5}
!37 = !{i32 6, !"RayDispatchDimensionsHeight", i32 3, i32 4, i32 7, i32 5}
!38 = !{i32 6, !"HitGroupShaderRecordStride", i32 3, i32 8, i32 7, i32 5}
!39 = !{i32 6, !"MissShaderRecordStride", i32 3, i32 12, i32 7, i32 5}
!40 = !{i32 6, !"SamplerDescriptorHeapStartLo", i32 3, i32 16, i32 7, i32 5}
!41 = !{i32 6, !"SamplerDescriptorHeapStartHi", i32 3, i32 20, i32 7, i32 5}
!42 = !{i32 6, !"SrvCbvUavDescriptorHeapStartLo", i32 3, i32 24, i32 7, i32 5}
!43 = !{i32 6, !"SrvCbvUavDescriptorHeapStartHi", i32 3, i32 28, i32 7, i32 5}
!44 = !{i32 4, !45}
!45 = !{i32 6, !"h", i32 3, i32 0, i32 7, i32 4}
!46 = !{i32 88, !47}
!47 = !{i32 6, !"g_dynamic", i32 3, i32 0}
!48 = !{i32 88, !49, !51, !52, !53}
!49 = !{i32 6, !"cameraToWorld", i32 2, !50, i32 3, i32 0, i32 7, i32 9}
!50 = !{i32 4, i32 4, i32 1}
!51 = !{i32 6, !"worldCameraPosition", i32 3, i32 64, i32 7, i32 9}
!52 = !{i32 6, !"padding", i32 3, i32 76, i32 7, i32 5}
!53 = !{i32 6, !"resolution", i32 3, i32 80, i32 7, i32 9}
!54 = !{i32 8, !55, !56}
!55 = !{i32 6, !"h", i32 3, i32 0, i32 7, i32 9}
!56 = !{i32 6, !"mips", i32 3, i32 4}
!57 = !{i32 4, !58}
!58 = !{i32 6, !"handle", i32 3, i32 0, i32 7, i32 5}
!59 = !{i32 16, !55, !60}
!60 = !{i32 6, !"mips", i32 3, i32 12}
!61 = !{i32 8, !62}
!62 = !{i32 6, !"TopLevelAccelerationStructureGpuVA", i32 3, i32 0, i32 7, i32 5}
!63 = !{i32 16, !55}
!64 = !{i32 20, !55, !65}
!65 = !{i32 6, !"mips", i32 3, i32 16}
!66 = !{i32 28, !67}
!67 = !{i32 6, !"h", i32 3, i32 0}
!68 = !{i32 28, !69, !70, !71, !72, !73, !74, !75}
!69 = !{i32 6, !"m_indexOffsetBytes", i32 3, i32 0, i32 7, i32 5}
!70 = !{i32 6, !"m_uvAttributeOffsetBytes", i32 3, i32 4, i32 7, i32 5}
!71 = !{i32 6, !"m_normalAttributeOffsetBytes", i32 3, i32 8, i32 7, i32 5}
!72 = !{i32 6, !"m_tangentAttributeOffsetBytes", i32 3, i32 12, i32 7, i32 5}
!73 = !{i32 6, !"m_positionAttributeOffsetBytes", i32 3, i32 16, i32 7, i32 5}
!74 = !{i32 6, !"m_attributeStrideBytes", i32 3, i32 20, i32 7, i32 5}
!75 = !{i32 6, !"m_materialInstanceId", i32 3, i32 24, i32 7, i32 5}
!76 = !{i32 136, !77, !78, !79, !80, !81, !82, !83}
!77 = !{i32 6, !"SunDirection", i32 3, i32 0, i32 7, i32 9}
!78 = !{i32 6, !"SunColor", i32 3, i32 16, i32 7, i32 9}
!79 = !{i32 6, !"AmbientColor", i32 3, i32 32, i32 7, i32 9}
!80 = !{i32 6, !"ShadowTexelSize", i32 3, i32 48, i32 7, i32 9}
!81 = !{i32 6, !"ModelToShadow", i32 2, !50, i32 3, i32 64, i32 7, i32 9}
!82 = !{i32 6, !"IsReflection", i32 3, i32 128, i32 7, i32 5}
!83 = !{i32 6, !"UseShadowRays", i32 3, i32 132, i32 7, i32 5}
!84 = !{i32 1, void ()* @main, !85}
!85 = !{!86}
!86 = !{i32 0, !87, !87}
!87 = !{}
!88 = !{void ()* @main, !"main", null, !7, !89}
!89 = !{i32 0, i64 8208, i32 4, !90}
!90 = !{i32 8, i32 8, i32 1}
!91 = !{!92, !92, i64 0}
!92 = !{!"float", !93, i64 0}
!93 = !{!"omnipotent char", !94, i64 0}
!94 = !{!"Simple C/C++ TBAA"}
!95 = !{!96, !96, i64 0}
!96 = !{!"bool", !93, i64 0}
!97 = !{i32 0, i32 2}
!98 = !{!99, !101}
!99 = distinct !{!99, !100, !"\01?CalculateUVDerivatives@@YAXV?$vector@M$02@@00000AIAV?$vector@M$01@@1@Z: %ddX"}
!100 = distinct !{!100, !"\01?CalculateUVDerivatives@@YAXV?$vector@M$02@@00000AIAV?$vector@M$01@@1@Z"}
!101 = distinct !{!101, !100, !"\01?CalculateUVDerivatives@@YAXV?$vector@M$02@@00000AIAV?$vector@M$01@@1@Z: %ddY"}
!102 = !{!103}
!103 = distinct !{!103, !104, !"\01?GetRayData@@YA?AURayData@@V?$vector@M$02@@0@Z: %agg.result"}
!104 = distinct !{!104, !"\01?GetRayData@@YA?AURayData@@V?$vector@M$02@@0@Z"}
!105 = !{!106, !106, i64 0}
!106 = !{!"int", !93, i64 0}
!107 = !{!108}
!108 = distinct !{!108, !109, !"\01?StackPush@@YAXAIAHIII@Z: %stackTop"}
!109 = distinct !{!109, !"\01?StackPush@@YAXAIAHIII@Z"}
!110 = !{!111}
!111 = distinct !{!111, !112, !"\01?StackPop@@YAIAIAHAIAII@Z: %stackTop"}
!112 = distinct !{!112, !"\01?StackPop@@YAIAIAHAIAII@Z"}
!113 = !{!114}
!114 = distinct !{!114, !115, !"\01?StackPush@@YAXAIAHIII@Z: %stackTop"}
!115 = distinct !{!115, !"\01?StackPush@@YAXAIAHIII@Z"}
!116 = !{!117}
!117 = distinct !{!117, !118, !"\01?StackPush2@@YAXAIAH_NIIII@Z: %stackTop"}
!118 = distinct !{!118, !"\01?StackPush2@@YAXAIAH_NIIII@Z"}
!119 = !{!120, !122, !123, !125, !126}
!120 = distinct !{!120, !121, !"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z: %hitT"}
!121 = distinct !{!121, !"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z"}
!122 = distinct !{!122, !121, !"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z: %bary"}
!123 = distinct !{!123, !124, !"\01?TestLeafNodeIntersections@@YA_NURWByteAddressBufferPointer@@V?$vector@I$01@@IV?$vector@M$02@@2V?$vector@H$02@@2AIAV?$vector@M$01@@AIAMAIAI@Z: %resultBary"}
!124 = distinct !{!124, !"\01?TestLeafNodeIntersections@@YA_NURWByteAddressBufferPointer@@V?$vector@I$01@@IV?$vector@M$02@@2V?$vector@H$02@@2AIAV?$vector@M$01@@AIAMAIAI@Z"}
!125 = distinct !{!125, !124, !"\01?TestLeafNodeIntersections@@YA_NURWByteAddressBufferPointer@@V?$vector@I$01@@IV?$vector@M$02@@2V?$vector@H$02@@2AIAV?$vector@M$01@@AIAMAIAI@Z: %resultT"}
!126 = distinct !{!126, !124, !"\01?TestLeafNodeIntersections@@YA_NURWByteAddressBufferPointer@@V?$vector@I$01@@IV?$vector@M$02@@2V?$vector@H$02@@2AIAV?$vector@M$01@@AIAMAIAI@Z: %resultTriId"}
!127 = !{!128}
!128 = distinct !{!128, !129, !"\01?StackPush@@YAXAIAHIII@Z: %stackTop"}
!129 = distinct !{!129, !"\01?StackPush@@YAXAIAHIII@Z"}
!130 = !{!131}
!131 = distinct !{!131, !132, !"\01?GetRayData@@YA?AURayData@@V?$vector@M$02@@0@Z: %agg.result"}
!132 = distinct !{!132, !"\01?GetRayData@@YA?AURayData@@V?$vector@M$02@@0@Z"}
