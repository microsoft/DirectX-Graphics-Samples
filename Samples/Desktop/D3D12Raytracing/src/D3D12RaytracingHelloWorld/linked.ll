; ModuleID = 'main'
target triple = "dxil-ms-dx"

%"class.RWTexture2D<vector<float, 4> >" = type { <4 x float> }
%Constants = type { i32, i32, i32, i32, i32, i32, i32, i32 }
%struct.ByteAddressBuffer = type { i32 }
%AccelerationStructureList = type { <2 x i32> }
%struct.RWByteAddressBuffer = type { i32 }
%dx.types.CBufRet.i32 = type { i32, i32, i32, i32 }
%dx.types.Handle = type { i8* }
%dx.types.ResRet.f32 = type { float, float, float, float, i32 }
%dx.types.ResRet.i32 = type { i32, i32, i32, i32, i32 }
%struct.RaytracingAccelerationStructure = type { i32 }

@"\01?RenderTarget@@3V?$RWTexture2D@V?$vector@M$03@@@@A" = external global %"class.RWTexture2D<vector<float, 4> >"
@Constants = external global %Constants
@"\01?RayGenShaderTable@@3UByteAddressBuffer@@A" = external global %struct.ByteAddressBuffer
@"\01?stack@@3PAIA" = addrspace(3) global [2048 x i32] zeroinitializer
@AccelerationStructureList = external constant %AccelerationStructureList
@"\01?HitGroupShaderTable@@3UByteAddressBuffer@@A" = external global %struct.ByteAddressBuffer
@"\01?MissShaderTable@@3UByteAddressBuffer@@A" = external global %struct.ByteAddressBuffer
@"\01?DescriptorHeapBufferTable@@3PAURWByteAddressBuffer@@A" = external global [0 x %struct.RWByteAddressBuffer]
@llvm.used = appending global [7 x i8*] [i8* bitcast (%AccelerationStructureList* @AccelerationStructureList to i8*), i8* bitcast (%Constants* @Constants to i8*), i8* bitcast (%struct.ByteAddressBuffer* @"\01?HitGroupShaderTable@@3UByteAddressBuffer@@A" to i8*), i8* bitcast (%struct.ByteAddressBuffer* @"\01?RayGenShaderTable@@3UByteAddressBuffer@@A" to i8*), i8* bitcast (%struct.ByteAddressBuffer* @"\01?MissShaderTable@@3UByteAddressBuffer@@A" to i8*), i8* bitcast (%"class.RWTexture2D<vector<float, 4> >"* @"\01?RenderTarget@@3V?$RWTexture2D@V?$vector@M$03@@@@A" to i8*), i8* bitcast ([0 x %struct.RWByteAddressBuffer]* @"\01?DescriptorHeapBufferTable@@3PAURWByteAddressBuffer@@A" to i8*)], section "llvm.metadata"

; Function Attrs: nounwind readnone
declare float @dx.op.tertiary.f32(i32, float, float, float) #0

; Function Attrs: nounwind readonly
declare %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32, %dx.types.Handle, i32) #1

; Function Attrs: nounwind readonly
declare %dx.types.ResRet.f32 @dx.op.bufferLoad.f32(i32, %dx.types.Handle, i32, i32) #1

; Function Attrs: nounwind readonly
declare %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32, %dx.types.Handle, i32, i32) #1

; Function Attrs: nounwind
declare void @dx.op.textureStore.f32(i32, %dx.types.Handle, i32, i32, i32, float, float, float, float, i8) #2

declare i32 @dx.op.flattenedThreadIdInGroup.i32(i32)

; Function Attrs: nounwind readnone
declare float @dx.op.binary.f32(i32, float, float) #0

declare i32 @dx.op.threadId.i32(i32, i32)

; Function Attrs: nounwind readnone
declare float @dx.op.unary.f32(i32, float) #0

define void @main() {
  %RenderTarget_UAV_2d = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 1, i32 0, i32 0, i1 false)
  %MissShaderTable_texture_rawbuf = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 0, i32 2, i32 1, i1 false)
  %RayGenShaderTable_texture_rawbuf = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 0, i32 1, i32 2, i1 false)
  %HitGroupShaderTable_texture_rawbuf = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 0, i32 0, i32 0, i1 false)
  %Constants_cbuffer = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 2, i32 1, i32 0, i1 false)
  %AccelerationStructureList_cbuffer = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 2, i32 0, i32 1, i1 false)
  %v19.i.49.i.i = alloca [3 x float], align 4
  %v20.i.50.i.i = alloca [3 x float], align 4
  %v21.i.51.i.i = alloca [3 x float], align 4
  %v22.i.52.i.i = alloca [3 x float], align 4
  %v23.i.i.i = alloca [3 x float], align 4
  %nodesToProcess.i.i.i.i.i = alloca [2 x i32], align 4
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

while.cond.i:                                     ; preds = %state_1001.MyRaygenShader.ss_1.i.i, %state_1002.MyClosestHitShader.ss_0.i.i, %state_1003.MyMissShader.ss_0.i.i, %state_1005.Fallback_TraceRay.ss_1.i.i, %remat_begin.i.i.i, %11, %30, %remat_begin.i.57.i.i, %while.body.i, %if.end.i
  %.i0 = phi float [ undef, %if.end.i ], [ %.i0, %state_1001.MyRaygenShader.ss_1.i.i ], [ %.i0, %state_1002.MyClosestHitShader.ss_0.i.i ], [ %.i0, %state_1003.MyMissShader.ss_0.i.i ], [ %.i0, %state_1005.Fallback_TraceRay.ss_1.i.i ], [ %.i0, %remat_begin.i.i.i ], [ %.i0, %11 ], [ %.i038, %30 ], [ %.i038, %remat_begin.i.57.i.i ], [ %.i0, %while.body.i ]
  %.i1 = phi float [ undef, %if.end.i ], [ %.i1, %state_1001.MyRaygenShader.ss_1.i.i ], [ %.i1, %state_1002.MyClosestHitShader.ss_0.i.i ], [ %.i1, %state_1003.MyMissShader.ss_0.i.i ], [ %.i1, %state_1005.Fallback_TraceRay.ss_1.i.i ], [ %.i1, %remat_begin.i.i.i ], [ %.i1, %11 ], [ %.i139, %30 ], [ %.i139, %remat_begin.i.57.i.i ], [ %.i1, %while.body.i ]
  %.i2 = phi float [ undef, %if.end.i ], [ %.i2, %state_1001.MyRaygenShader.ss_1.i.i ], [ %.i2, %state_1002.MyClosestHitShader.ss_0.i.i ], [ %.i2, %state_1003.MyMissShader.ss_0.i.i ], [ %.i2, %state_1005.Fallback_TraceRay.ss_1.i.i ], [ %.i2, %remat_begin.i.i.i ], [ %.i2, %11 ], [ %.i240, %30 ], [ %.i240, %remat_begin.i.57.i.i ], [ %.i2, %while.body.i ]
  %.i3 = phi float [ undef, %if.end.i ], [ %.i3, %state_1001.MyRaygenShader.ss_1.i.i ], [ %.i3, %state_1002.MyClosestHitShader.ss_0.i.i ], [ %.i3, %state_1003.MyMissShader.ss_0.i.i ], [ %.i3, %state_1005.Fallback_TraceRay.ss_1.i.i ], [ %.i3, %remat_begin.i.i.i ], [ %.i3, %11 ], [ %.i341, %30 ], [ %.i341, %remat_begin.i.57.i.i ], [ %.i3, %while.body.i ]
  %.i4 = phi float [ undef, %if.end.i ], [ %.i4, %state_1001.MyRaygenShader.ss_1.i.i ], [ %.i4, %state_1002.MyClosestHitShader.ss_0.i.i ], [ %.i4, %state_1003.MyMissShader.ss_0.i.i ], [ %.i4, %state_1005.Fallback_TraceRay.ss_1.i.i ], [ %.i4, %remat_begin.i.i.i ], [ %.i4, %11 ], [ %.i442, %30 ], [ %.i442, %remat_begin.i.57.i.i ], [ %.i4, %while.body.i ]
  %.i5 = phi float [ undef, %if.end.i ], [ %.i5, %state_1001.MyRaygenShader.ss_1.i.i ], [ %.i5, %state_1002.MyClosestHitShader.ss_0.i.i ], [ %.i5, %state_1003.MyMissShader.ss_0.i.i ], [ %.i5, %state_1005.Fallback_TraceRay.ss_1.i.i ], [ %.i5, %remat_begin.i.i.i ], [ %.i5, %11 ], [ %.i543, %30 ], [ %.i543, %remat_begin.i.57.i.i ], [ %.i5, %while.body.i ]
  %.i6 = phi float [ undef, %if.end.i ], [ %.i6, %state_1001.MyRaygenShader.ss_1.i.i ], [ %.i6, %state_1002.MyClosestHitShader.ss_0.i.i ], [ %.i6, %state_1003.MyMissShader.ss_0.i.i ], [ %.i6, %state_1005.Fallback_TraceRay.ss_1.i.i ], [ %.i6, %remat_begin.i.i.i ], [ %.i6, %11 ], [ %.i644, %30 ], [ %.i644, %remat_begin.i.57.i.i ], [ %.i6, %while.body.i ]
  %.i7 = phi float [ undef, %if.end.i ], [ %.i7, %state_1001.MyRaygenShader.ss_1.i.i ], [ %.i7, %state_1002.MyClosestHitShader.ss_0.i.i ], [ %.i7, %state_1003.MyMissShader.ss_0.i.i ], [ %.i7, %state_1005.Fallback_TraceRay.ss_1.i.i ], [ %.i7, %remat_begin.i.i.i ], [ %.i7, %11 ], [ %.i745, %30 ], [ %.i745, %remat_begin.i.57.i.i ], [ %.i7, %while.body.i ]
  %.i8 = phi float [ undef, %if.end.i ], [ %.i8, %state_1001.MyRaygenShader.ss_1.i.i ], [ %.i8, %state_1002.MyClosestHitShader.ss_0.i.i ], [ %.i8, %state_1003.MyMissShader.ss_0.i.i ], [ %.i8, %state_1005.Fallback_TraceRay.ss_1.i.i ], [ %.i8, %remat_begin.i.i.i ], [ %.i8, %11 ], [ %.i846, %30 ], [ %.i846, %remat_begin.i.57.i.i ], [ %.i8, %while.body.i ]
  %.i9 = phi float [ undef, %if.end.i ], [ %.i9, %state_1001.MyRaygenShader.ss_1.i.i ], [ %.i9, %state_1002.MyClosestHitShader.ss_0.i.i ], [ %.i9, %state_1003.MyMissShader.ss_0.i.i ], [ %.i9, %state_1005.Fallback_TraceRay.ss_1.i.i ], [ %.i9, %remat_begin.i.i.i ], [ %.i9, %11 ], [ %.i947, %30 ], [ %.i947, %remat_begin.i.57.i.i ], [ %.i9, %while.body.i ]
  %.i10 = phi float [ undef, %if.end.i ], [ %.i10, %state_1001.MyRaygenShader.ss_1.i.i ], [ %.i10, %state_1002.MyClosestHitShader.ss_0.i.i ], [ %.i10, %state_1003.MyMissShader.ss_0.i.i ], [ %.i10, %state_1005.Fallback_TraceRay.ss_1.i.i ], [ %.i10, %remat_begin.i.i.i ], [ %.i10, %11 ], [ %.i1048, %30 ], [ %.i1048, %remat_begin.i.57.i.i ], [ %.i10, %while.body.i ]
  %.i11 = phi float [ undef, %if.end.i ], [ %.i11, %state_1001.MyRaygenShader.ss_1.i.i ], [ %.i11, %state_1002.MyClosestHitShader.ss_0.i.i ], [ %.i11, %state_1003.MyMissShader.ss_0.i.i ], [ %.i11, %state_1005.Fallback_TraceRay.ss_1.i.i ], [ %.i11, %remat_begin.i.i.i ], [ %.i11, %11 ], [ %.i1149, %30 ], [ %.i1149, %remat_begin.i.57.i.i ], [ %.i11, %while.body.i ]
  %.i014 = phi float [ undef, %if.end.i ], [ %.i014, %state_1001.MyRaygenShader.ss_1.i.i ], [ %.i014, %state_1002.MyClosestHitShader.ss_0.i.i ], [ %.i014, %state_1003.MyMissShader.ss_0.i.i ], [ %.i014, %state_1005.Fallback_TraceRay.ss_1.i.i ], [ %.i014, %remat_begin.i.i.i ], [ %.i014, %11 ], [ %.i062, %30 ], [ %.i062, %remat_begin.i.57.i.i ], [ %.i014, %while.body.i ]
  %.i115 = phi float [ undef, %if.end.i ], [ %.i115, %state_1001.MyRaygenShader.ss_1.i.i ], [ %.i115, %state_1002.MyClosestHitShader.ss_0.i.i ], [ %.i115, %state_1003.MyMissShader.ss_0.i.i ], [ %.i115, %state_1005.Fallback_TraceRay.ss_1.i.i ], [ %.i115, %remat_begin.i.i.i ], [ %.i115, %11 ], [ %.i163, %30 ], [ %.i163, %remat_begin.i.57.i.i ], [ %.i115, %while.body.i ]
  %.i216 = phi float [ undef, %if.end.i ], [ %.i216, %state_1001.MyRaygenShader.ss_1.i.i ], [ %.i216, %state_1002.MyClosestHitShader.ss_0.i.i ], [ %.i216, %state_1003.MyMissShader.ss_0.i.i ], [ %.i216, %state_1005.Fallback_TraceRay.ss_1.i.i ], [ %.i216, %remat_begin.i.i.i ], [ %.i216, %11 ], [ %.i264, %30 ], [ %.i264, %remat_begin.i.57.i.i ], [ %.i216, %while.body.i ]
  %.i317 = phi float [ undef, %if.end.i ], [ %.i317, %state_1001.MyRaygenShader.ss_1.i.i ], [ %.i317, %state_1002.MyClosestHitShader.ss_0.i.i ], [ %.i317, %state_1003.MyMissShader.ss_0.i.i ], [ %.i317, %state_1005.Fallback_TraceRay.ss_1.i.i ], [ %.i317, %remat_begin.i.i.i ], [ %.i317, %11 ], [ %.i365, %30 ], [ %.i365, %remat_begin.i.57.i.i ], [ %.i317, %while.body.i ]
  %.i418 = phi float [ undef, %if.end.i ], [ %.i418, %state_1001.MyRaygenShader.ss_1.i.i ], [ %.i418, %state_1002.MyClosestHitShader.ss_0.i.i ], [ %.i418, %state_1003.MyMissShader.ss_0.i.i ], [ %.i418, %state_1005.Fallback_TraceRay.ss_1.i.i ], [ %.i418, %remat_begin.i.i.i ], [ %.i418, %11 ], [ %.i466, %30 ], [ %.i466, %remat_begin.i.57.i.i ], [ %.i418, %while.body.i ]
  %.i519 = phi float [ undef, %if.end.i ], [ %.i519, %state_1001.MyRaygenShader.ss_1.i.i ], [ %.i519, %state_1002.MyClosestHitShader.ss_0.i.i ], [ %.i519, %state_1003.MyMissShader.ss_0.i.i ], [ %.i519, %state_1005.Fallback_TraceRay.ss_1.i.i ], [ %.i519, %remat_begin.i.i.i ], [ %.i519, %11 ], [ %.i567, %30 ], [ %.i567, %remat_begin.i.57.i.i ], [ %.i519, %while.body.i ]
  %.i620 = phi float [ undef, %if.end.i ], [ %.i620, %state_1001.MyRaygenShader.ss_1.i.i ], [ %.i620, %state_1002.MyClosestHitShader.ss_0.i.i ], [ %.i620, %state_1003.MyMissShader.ss_0.i.i ], [ %.i620, %state_1005.Fallback_TraceRay.ss_1.i.i ], [ %.i620, %remat_begin.i.i.i ], [ %.i620, %11 ], [ %.i668, %30 ], [ %.i668, %remat_begin.i.57.i.i ], [ %.i620, %while.body.i ]
  %.i721 = phi float [ undef, %if.end.i ], [ %.i721, %state_1001.MyRaygenShader.ss_1.i.i ], [ %.i721, %state_1002.MyClosestHitShader.ss_0.i.i ], [ %.i721, %state_1003.MyMissShader.ss_0.i.i ], [ %.i721, %state_1005.Fallback_TraceRay.ss_1.i.i ], [ %.i721, %remat_begin.i.i.i ], [ %.i721, %11 ], [ %.i769, %30 ], [ %.i769, %remat_begin.i.57.i.i ], [ %.i721, %while.body.i ]
  %.i822 = phi float [ undef, %if.end.i ], [ %.i822, %state_1001.MyRaygenShader.ss_1.i.i ], [ %.i822, %state_1002.MyClosestHitShader.ss_0.i.i ], [ %.i822, %state_1003.MyMissShader.ss_0.i.i ], [ %.i822, %state_1005.Fallback_TraceRay.ss_1.i.i ], [ %.i822, %remat_begin.i.i.i ], [ %.i822, %11 ], [ %.i870, %30 ], [ %.i870, %remat_begin.i.57.i.i ], [ %.i822, %while.body.i ]
  %.i923 = phi float [ undef, %if.end.i ], [ %.i923, %state_1001.MyRaygenShader.ss_1.i.i ], [ %.i923, %state_1002.MyClosestHitShader.ss_0.i.i ], [ %.i923, %state_1003.MyMissShader.ss_0.i.i ], [ %.i923, %state_1005.Fallback_TraceRay.ss_1.i.i ], [ %.i923, %remat_begin.i.i.i ], [ %.i923, %11 ], [ %.i971, %30 ], [ %.i971, %remat_begin.i.57.i.i ], [ %.i923, %while.body.i ]
  %.i1024 = phi float [ undef, %if.end.i ], [ %.i1024, %state_1001.MyRaygenShader.ss_1.i.i ], [ %.i1024, %state_1002.MyClosestHitShader.ss_0.i.i ], [ %.i1024, %state_1003.MyMissShader.ss_0.i.i ], [ %.i1024, %state_1005.Fallback_TraceRay.ss_1.i.i ], [ %.i1024, %remat_begin.i.i.i ], [ %.i1024, %11 ], [ %.i1072, %30 ], [ %.i1072, %remat_begin.i.57.i.i ], [ %.i1024, %while.body.i ]
  %.i1125 = phi float [ undef, %if.end.i ], [ %.i1125, %state_1001.MyRaygenShader.ss_1.i.i ], [ %.i1125, %state_1002.MyClosestHitShader.ss_0.i.i ], [ %.i1125, %state_1003.MyMissShader.ss_0.i.i ], [ %.i1125, %state_1005.Fallback_TraceRay.ss_1.i.i ], [ %.i1125, %remat_begin.i.i.i ], [ %.i1125, %11 ], [ %.i1173, %30 ], [ %.i1173, %remat_begin.i.57.i.i ], [ %.i1125, %while.body.i ]
  %6 = phi i32 [ 255, %if.end.i ], [ %add.i.i.8.i.i, %state_1001.MyRaygenShader.ss_1.i.i ], [ %6, %state_1002.MyClosestHitShader.ss_0.i.i ], [ %6, %state_1003.MyMissShader.ss_0.i.i ], [ %add.i.6.i.i.i, %state_1005.Fallback_TraceRay.ss_1.i.i ], [ %6, %11 ], [ %sub.i.i.i.i, %remat_begin.i.i.i ], [ %sub.i.137.i.i.i, %remat_begin.i.57.i.i ], [ %6, %30 ], [ %6, %while.body.i ]
  %7 = phi i32 [ 3333, %if.end.i ], [ %7, %state_1001.MyRaygenShader.ss_1.i.i ], [ %7, %state_1002.MyClosestHitShader.ss_0.i.i ], [ %7, %state_1003.MyMissShader.ss_0.i.i ], [ %71, %state_1005.Fallback_TraceRay.ss_1.i.i ], [ %7, %remat_begin.i.i.i ], [ %7, %11 ], [ %25, %remat_begin.i.57.i.i ], [ %32, %30 ], [ %7, %while.body.i ]
  %8 = phi i32 [ 2222, %if.end.i ], [ %8, %state_1001.MyRaygenShader.ss_1.i.i ], [ %8, %state_1002.MyClosestHitShader.ss_0.i.i ], [ %8, %state_1003.MyMissShader.ss_0.i.i ], [ %70, %state_1005.Fallback_TraceRay.ss_1.i.i ], [ %8, %remat_begin.i.i.i ], [ %8, %11 ], [ %26, %remat_begin.i.57.i.i ], [ %31, %30 ], [ %8, %while.body.i ]
  %9 = phi i32 [ 1111, %if.end.i ], [ %9, %state_1001.MyRaygenShader.ss_1.i.i ], [ %9, %state_1002.MyClosestHitShader.ss_0.i.i ], [ %9, %state_1003.MyMissShader.ss_0.i.i ], [ %v24.int.i.i.i, %state_1005.Fallback_TraceRay.ss_1.i.i ], [ %9, %remat_begin.i.i.i ], [ %9, %11 ], [ %arg13.int.i.i.i, %remat_begin.i.57.i.i ], [ %9, %30 ], [ %9, %while.body.i ]
  %10 = phi i32 [ undef, %if.end.i ], [ %10, %state_1001.MyRaygenShader.ss_1.i.i ], [ %10, %state_1002.MyClosestHitShader.ss_0.i.i ], [ %10, %state_1003.MyMissShader.ss_0.i.i ], [ %10, %state_1005.Fallback_TraceRay.ss_1.i.i ], [ %10, %remat_begin.i.i.i ], [ %10, %11 ], [ %28, %30 ], [ %28, %remat_begin.i.57.i.i ], [ %10, %while.body.i ]
  %stateId.0.i = phi i32 [ %2, %if.end.i ], [ %ret.stateId.i.11.i.i, %state_1001.MyRaygenShader.ss_1.i.i ], [ %ret.stateId.i.24.i.i, %state_1002.MyClosestHitShader.ss_0.i.i ], [ %ret.stateId.i.37.i.i, %state_1003.MyMissShader.ss_0.i.i ], [ %ret.stateId.i.83.i.i, %state_1005.Fallback_TraceRay.ss_1.i.i ], [ %ret.stateId.i.i.i, %11 ], [ 1004, %remat_begin.i.i.i ], [ %stateID.i.0.i.i.i, %remat_begin.i.57.i.i ], [ %ret.stateId.i.61.i.i, %30 ], [ -3, %while.body.i ]
  %cmp11.i = icmp sge i32 %stateId.0.i, 0
  br i1 %cmp11.i, label %while.body.i, label %fb_Fallback_Scheduler.exit

while.body.i:                                     ; preds = %while.cond.i
  switch i32 %stateId.0.i, label %while.cond.i [
    i32 1000, label %state_1000.MyRaygenShader.ss_0.i.i
    i32 1001, label %state_1001.MyRaygenShader.ss_1.i.i
    i32 1002, label %state_1002.MyClosestHitShader.ss_0.i.i
    i32 1003, label %state_1003.MyMissShader.ss_0.i.i
    i32 1004, label %state_1004.Fallback_TraceRay.ss_0.i.i
    i32 1005, label %state_1005.Fallback_TraceRay.ss_1.i.i
  ]

state_1000.MyRaygenShader.ss_0.i.i:               ; preds = %while.body.i
  %sub.i.i.i.i = sub nsw i32 %6, 20
  %.i0.i.i.i = uitofp i32 %DTidx.i to float
  %.i1.i.i.i = uitofp i32 %DTidy.i to float
  %.i05.i.i.i = uitofp i32 %5 to float
  %.i16.i.i.i = uitofp i32 %4 to float
  %.i07.i.i.i = fdiv fast float %.i0.i.i.i, %.i05.i.i.i
  %.i18.i.i.i = fdiv fast float %.i1.i.i.i, %.i16.i.i.i
  %ShaderRecordBuffer1.i.i.i = call %dx.types.ResRet.f32 @dx.op.bufferLoad.f32(i32 68, %dx.types.Handle %RayGenShaderTable_texture_rawbuf, i32 8, i32 undef) #2
  %v1.i.i.i = extractvalue %dx.types.ResRet.f32 %ShaderRecordBuffer1.i.i.i, 2
  %v2.i.i.i = extractvalue %dx.types.ResRet.f32 %ShaderRecordBuffer1.i.i.i, 0
  %v3.i.i.i = fsub fast float %v1.i.i.i, %v2.i.i.i
  %v4.i.i.i = fmul fast float %v3.i.i.i, %.i07.i.i.i
  %v5.i.i.i = fadd fast float %v4.i.i.i, %v2.i.i.i
  %v6.i.i.i = extractvalue %dx.types.ResRet.f32 %ShaderRecordBuffer1.i.i.i, 3
  %v7.i.i.i = extractvalue %dx.types.ResRet.f32 %ShaderRecordBuffer1.i.i.i, 1
  %v8.i.i.i = fsub fast float %v6.i.i.i, %v7.i.i.i
  %v9.i.i.i = fmul fast float %v8.i.i.i, %.i18.i.i.i
  %v10.i.i.i = fadd fast float %v9.i.i.i, %v7.i.i.i
  %ShaderRecordBuffer.i.i.i = call %dx.types.ResRet.f32 @dx.op.bufferLoad.f32(i32 68, %dx.types.Handle %RayGenShaderTable_texture_rawbuf, i32 24, i32 undef) #2
  %v11.i.i.i = extractvalue %dx.types.ResRet.f32 %ShaderRecordBuffer.i.i.i, 0
  %v12.i.i.i = fcmp fast oge float %v5.i.i.i, %v11.i.i.i
  %v13.i.i.i = extractvalue %dx.types.ResRet.f32 %ShaderRecordBuffer.i.i.i, 2
  %v14.i.i.i = fcmp fast ole float %v5.i.i.i, %v13.i.i.i
  %v15.i.i.i = and i1 %v12.i.i.i, %v14.i.i.i
  %v16.i.i.i = extractvalue %dx.types.ResRet.f32 %ShaderRecordBuffer.i.i.i, 1
  %v17.i.i.i = fcmp fast oge float %v10.i.i.i, %v16.i.i.i
  %v18.i.i.i = extractvalue %dx.types.ResRet.f32 %ShaderRecordBuffer.i.i.i, 3
  %v19.i.i.i = fcmp fast ole float %v10.i.i.i, %v18.i.i.i
  %v20.i.i.i = and i1 %v17.i.i.i, %v19.i.i.i
  %v21.i.i.i = and i1 %v15.i.i.i, %v20.i.i.i
  br i1 %v21.i.i.i, label %remat_begin.i.i.i, label %11

; <label>:11                                      ; preds = %state_1000.MyRaygenShader.ss_0.i.i
  call void @dx.op.textureStore.f32(i32 67, %dx.types.Handle %RenderTarget_UAV_2d, i32 %DTidx.i, i32 %DTidy.i, i32 undef, float %.i07.i.i.i, float %.i18.i.i.i, float 0.000000e+00, float 1.000000e+00, i8 15) #2
  %arrayidx.i.46.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %6
  %ret.stateId.i.i.i = load i32, i32* %arrayidx.i.46.i.i.i
  br label %while.cond.i

remat_begin.i.i.i:                                ; preds = %state_1000.MyRaygenShader.ss_0.i.i
  %add.i.42.i.i.i = add nsw i32 %sub.i.i.i.i, 16
  %arrayidx.i.44.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.42.i.i.i
  store i32 0, i32* %arrayidx.i.44.i.i.i, align 8
  %new.payload.offset.i.i.i = add i32 %sub.i.i.i.i, 16
  %arrayidx.i.41.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %sub.i.i.i.i
  store i32 1001, i32* %arrayidx.i.41.i.i.i
  %add.i.37.i.i.i = add nsw i32 %sub.i.i.i.i, 1
  %arrayidx.i.39.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.37.i.i.i
  store i32 16, i32* %arrayidx.i.39.i.i.i
  %add.i.34.i.i.i = add nsw i32 %sub.i.i.i.i, 2
  %arrayidx.i.36.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.34.i.i.i
  store i32 -1, i32* %arrayidx.i.36.i.i.i
  %add.i.31.i.i.i = add nsw i32 %sub.i.i.i.i, 3
  %arrayidx.i.33.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.31.i.i.i
  store i32 0, i32* %arrayidx.i.33.i.i.i
  %add.i.28.i.i.i = add nsw i32 %sub.i.i.i.i, 4
  %arrayidx.i.30.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.28.i.i.i
  store i32 1, i32* %arrayidx.i.30.i.i.i
  %add.i.25.i.i.i = add nsw i32 %sub.i.i.i.i, 5
  %arrayidx.i.27.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.25.i.i.i
  store i32 0, i32* %arrayidx.i.27.i.i.i
  %12 = bitcast float %v5.i.i.i to i32
  %add.i.22.i.i.i = add nsw i32 %sub.i.i.i.i, 6
  %arrayidx.i.24.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.22.i.i.i
  store i32 %12, i32* %arrayidx.i.24.i.i.i
  %13 = bitcast float %v10.i.i.i to i32
  %add.i.19.i.i.i = add nsw i32 %sub.i.i.i.i, 7
  %arrayidx.i.21.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.19.i.i.i
  store i32 %13, i32* %arrayidx.i.21.i.i.i
  %add.i.16.i.i.i = add nsw i32 %sub.i.i.i.i, 8
  %arrayidx.i.18.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.16.i.i.i
  store i32 0, i32* %arrayidx.i.18.i.i.i
  %add.i.13.i.i.i = add nsw i32 %sub.i.i.i.i, 9
  %arrayidx.i.15.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.13.i.i.i
  store i32 1008981770, i32* %arrayidx.i.15.i.i.i
  %add.i.10.i.i.i = add nsw i32 %sub.i.i.i.i, 10
  %arrayidx.i.12.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.10.i.i.i
  store i32 0, i32* %arrayidx.i.12.i.i.i
  %add.i.7.i.i.i = add nsw i32 %sub.i.i.i.i, 11
  %arrayidx.i.9.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.7.i.i.i
  store i32 0, i32* %arrayidx.i.9.i.i.i
  %add.i.4.i.i.i = add nsw i32 %sub.i.i.i.i, 12
  %arrayidx.i.6.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.4.i.i.i
  store i32 1065353216, i32* %arrayidx.i.6.i.i.i
  %add.i.1.i.i.i = add nsw i32 %sub.i.i.i.i, 13
  %arrayidx.i.3.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.1.i.i.i
  store i32 1176256512, i32* %arrayidx.i.3.i.i.i
  %add.i.i.i.i = add nsw i32 %sub.i.i.i.i, 14
  %arrayidx.i.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.i.i.i
  store i32 %new.payload.offset.i.i.i, i32* %arrayidx.i.i.i.i
  br label %while.cond.i

state_1001.MyRaygenShader.ss_1.i.i:               ; preds = %while.body.i
  %add.i.i.8.i.i = add nsw i32 %6, 20
  %arrayidx.i.i.10.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.i.8.i.i
  %ret.stateId.i.11.i.i = load i32, i32* %arrayidx.i.i.10.i.i
  br label %while.cond.i

state_1002.MyClosestHitShader.ss_0.i.i:           ; preds = %while.body.i
  %arrayidx.i.7.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %8
  %v1.v.i.i.i = bitcast i32* %arrayidx.i.7.i.i.i to float*
  %add.i.i.16.i.i = add nsw i32 %8, 1
  %arrayidx.i.5.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.i.16.i.i
  %v1.v2.i.i.i = bitcast i32* %arrayidx.i.5.i.i.i to float*
  %el.i.i.i = load float, float* %v1.v.i.i.i
  %el3.i.i.i = load float, float* %v1.v2.i.i.i
  %v4.i.17.i.i = fsub fast float 1.000000e+00, %el.i.i.i
  %v6.i.18.i.i = fsub fast float %v4.i.17.i.i, %el3.i.i.i
  call void @dx.op.textureStore.f32(i32 67, %dx.types.Handle %RenderTarget_UAV_2d, i32 %DTidx.i, i32 %DTidy.i, i32 undef, float %v6.i.18.i.i, float %el.i.i.i, float %el3.i.i.i, float 1.000000e+00, i8 15) #2
  %arrayidx.i.i.23.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %6
  %ret.stateId.i.24.i.i = load i32, i32* %arrayidx.i.i.23.i.i
  br label %while.cond.i

state_1003.MyMissShader.ss_0.i.i:                 ; preds = %while.body.i
  call void @dx.op.textureStore.f32(i32 67, %dx.types.Handle %RenderTarget_UAV_2d, i32 %DTidx.i, i32 %DTidy.i, i32 undef, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 1.000000e+00, i8 15) #2
  %arrayidx.i.i.36.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %6
  %ret.stateId.i.37.i.i = load i32, i32* %arrayidx.i.i.36.i.i
  br label %while.cond.i

state_1004.Fallback_TraceRay.ss_0.i.i:            ; preds = %while.body.i
  %add.i.i.40.i.i = add nsw i32 %6, -1
  %arrayidx.i.i.42.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.i.40.i.i
  store i32 %8, i32* %arrayidx.i.i.42.i.i, align 4
  %add2.i.i.i.i = add nsw i32 %6, -2
  %arrayidx4.i.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add2.i.i.i.i
  store i32 %7, i32* %arrayidx4.i.i.i.i, align 4
  %sub.i.i.44.i.i = sub nsw i32 %6, 2
  %sub6.i.i.i.i = sub nsw i32 %sub.i.i.44.i.i, 8
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
  %14 = bitcast i32 %arg5.int.i.i.i to float
  %add.i.114.i.i.i = add nsw i32 %6, 7
  %arrayidx.i.116.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.114.i.i.i
  %arg6.int.i.i.i = load i32, i32* %arrayidx.i.116.i.i.i
  %15 = bitcast i32 %arg6.int.i.i.i to float
  %add.i.111.i.i.i = add nsw i32 %6, 8
  %arrayidx.i.113.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.111.i.i.i
  %arg7.int.i.i.i = load i32, i32* %arrayidx.i.113.i.i.i
  %16 = bitcast i32 %arg7.int.i.i.i to float
  %add.i.108.i.i.i = add nsw i32 %6, 9
  %arrayidx.i.110.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.108.i.i.i
  %arg8.int.i.i.i = load i32, i32* %arrayidx.i.110.i.i.i
  %17 = bitcast i32 %arg8.int.i.i.i to float
  %add.i.105.i.i.i = add nsw i32 %6, 10
  %arrayidx.i.107.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.105.i.i.i
  %arg9.int.i.i.i = load i32, i32* %arrayidx.i.107.i.i.i
  %18 = bitcast i32 %arg9.int.i.i.i to float
  %add.i.102.i.i.i = add nsw i32 %6, 11
  %arrayidx.i.104.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.102.i.i.i
  %arg10.int.i.i.i = load i32, i32* %arrayidx.i.104.i.i.i
  %19 = bitcast i32 %arg10.int.i.i.i to float
  %add.i.99.i.i.i = add nsw i32 %6, 12
  %arrayidx.i.101.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.99.i.i.i
  %arg11.int.i.i.i = load i32, i32* %arrayidx.i.101.i.i.i
  %20 = bitcast i32 %arg11.int.i.i.i to float
  %add.i.96.i.i.i = add nsw i32 %6, 13
  %arrayidx.i.98.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.96.i.i.i
  %arg12.int.i.i.i = load i32, i32* %arrayidx.i.98.i.i.i
  %21 = bitcast i32 %arg12.int.i.i.i to float
  %add.i.93.i.i.i = add nsw i32 %6, 14
  %arrayidx.i.95.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.93.i.i.i
  %arg13.int.i.i.i = load i32, i32* %arrayidx.i.95.i.i.i
  %v32.i.i.i = getelementptr inbounds [3 x float], [3 x float]* %v23.i.i.i, i32 0, i32 0
  store float %18, float* %v32.i.i.i, align 4
  %v33.i.i.i = getelementptr inbounds [3 x float], [3 x float]* %v23.i.i.i, i32 0, i32 1
  store float %19, float* %v33.i.i.i, align 4
  %v34.i.i.i = getelementptr inbounds [3 x float], [3 x float]* %v23.i.i.i, i32 0, i32 2
  store float %20, float* %v34.i.i.i, align 4
  %.i0.i.55.i.i = fdiv fast float 1.000000e+00, %18
  %.i1.i.56.i.i = fdiv fast float 1.000000e+00, %19
  %.i2.i.i.i = fdiv fast float 1.000000e+00, %20
  %.i0296.i.i.i = fmul fast float %.i0.i.55.i.i, %14
  %.i1297.i.i.i = fmul fast float %.i1.i.56.i.i, %15
  %.i2298.i.i.i = fmul fast float %.i2.i.i.i, %16
  %FAbs241.i.i.i = call float @dx.op.unary.f32(i32 6, float %18) #2
  %FAbs242.i.i.i = call float @dx.op.unary.f32(i32 6, float %19) #2
  %FAbs243.i.i.i = call float @dx.op.unary.f32(i32 6, float %20) #2
  %v35.i.i.i = fcmp fast ogt float %FAbs241.i.i.i, %FAbs242.i.i.i
  %v36.i.i.i = fcmp fast ogt float %FAbs241.i.i.i, %FAbs243.i.i.i
  %v37.i.i.i = and i1 %v35.i.i.i, %v36.i.i.i
  br i1 %v37.i.i.i, label %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i", label %22

; <label>:22                                      ; preds = %state_1004.Fallback_TraceRay.ss_0.i.i
  %v38.i.i.i = fcmp fast ogt float %FAbs242.i.i.i, %FAbs243.i.i.i
  br i1 %v38.i.i.i, label %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i", label %23

; <label>:23                                      ; preds = %22
  br label %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i"

"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i": ; preds = %22, %state_1004.Fallback_TraceRay.ss_0.i.i, %23
  %.0171.i.i.i = phi i32 [ 2, %23 ], [ 0, %state_1004.Fallback_TraceRay.ss_0.i.i ], [ 1, %22 ]
  %v39.i.i.i = add nuw nsw i32 %.0171.i.i.i, 1
  %v40.i.i.i = urem i32 %v39.i.i.i, 3
  %v41.i.i.i = add nuw nsw i32 %.0171.i.i.i, 2
  %v42.i.i.i = urem i32 %v41.i.i.i, 3
  %v43.i.i.i = getelementptr [3 x float], [3 x float]* %v23.i.i.i, i32 0, i32 %.0171.i.i.i
  %v44.i.i.i = load float, float* %v43.i.i.i, align 4, !tbaa !42, !noalias !46
  %v45.i.i.i = fcmp fast olt float %v44.i.i.i, 0.000000e+00
  %worldRayData.i.i.5.0.i0.i.i.i = select i1 %v45.i.i.i, i32 %v42.i.i.i, i32 %v40.i.i.i
  %worldRayData.i.i.5.0.i1.i.i.i = select i1 %v45.i.i.i, i32 %v40.i.i.i, i32 %v42.i.i.i
  %v46.i.i.i = getelementptr [3 x float], [3 x float]* %v23.i.i.i, i32 0, i32 %worldRayData.i.i.5.0.i0.i.i.i
  %v47.i.i.i = load float, float* %v46.i.i.i, align 4, !tbaa !42, !noalias !46
  %v48.i.i.i = fdiv float %v47.i.i.i, %v44.i.i.i
  %v49.i.i.i = getelementptr [3 x float], [3 x float]* %v23.i.i.i, i32 0, i32 %worldRayData.i.i.5.0.i1.i.i.i
  %v50.i.i.i = load float, float* %v49.i.i.i, align 4, !tbaa !42, !noalias !46
  %v51.i.i.i = fdiv float %v50.i.i.i, %v44.i.i.i
  %v52.i.i.i = fdiv float 1.000000e+00, %v44.i.i.i
  %v53.i.i.i = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %AccelerationStructureList_cbuffer, i32 0) #2
  %v54.i.i.i = extractvalue %dx.types.CBufRet.i32 %v53.i.i.i, 0
  %v55.i.i.i = extractvalue %dx.types.CBufRet.i32 %v53.i.i.i, 1
  %v56.i.i.i = getelementptr inbounds [2 x i32], [2 x i32]* %nodesToProcess.i.i.i.i.i, i32 0, i32 0
  store i32 0, i32* %v56.i.i.i, align 4, !tbaa !49
  %24 = add i32 %v55.i.i.i, 0
  %DescriptorHeapBufferTable_UAV_rawbuf337 = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 1, i32 1, i32 %24, i1 true)
  %v59.i.i.i = add i32 %v54.i.i.i, 4
  %v61.i.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %DescriptorHeapBufferTable_UAV_rawbuf337, i32 %v59.i.i.i, i32 undef) #2
  %v62.i.i.i = extractvalue %dx.types.ResRet.i32 %v61.i.i.i, 0
  %v63.i.i.i = add i32 %v62.i.i.i, %v54.i.i.i
  %v64.i.i.i = add i32 %v54.i.i.i, 16
  %v65.i.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %DescriptorHeapBufferTable_UAV_rawbuf337, i32 %v64.i.i.i, i32 undef) #2
  %v66.i.i.i = extractvalue %dx.types.ResRet.i32 %v65.i.i.i, 0
  %v67.i.i.i = extractvalue %dx.types.ResRet.i32 %v65.i.i.i, 1
  %v68.i.i.i = extractvalue %dx.types.ResRet.i32 %v65.i.i.i, 2
  %v69.i.i.i = add i32 %v54.i.i.i, 32
  %v70.i.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %DescriptorHeapBufferTable_UAV_rawbuf337, i32 %v69.i.i.i, i32 undef) #2
  %v71.i.i.i = extractvalue %dx.types.ResRet.i32 %v70.i.i.i, 0
  %v72.i.i.i = extractvalue %dx.types.ResRet.i32 %v70.i.i.i, 1
  %v73.i.i.i = extractvalue %dx.types.ResRet.i32 %v70.i.i.i, 2
  %v74.i.i.i = bitcast i32 %v66.i.i.i to float
  %v75.i.i.i = bitcast i32 %v67.i.i.i to float
  %v76.i.i.i = bitcast i32 %v68.i.i.i to float
  %v77.i.i.i = bitcast i32 %v71.i.i.i to float
  %v78.i.i.i = bitcast i32 %v72.i.i.i to float
  %v79.i.i.i = bitcast i32 %v73.i.i.i to float
  %v80.i.i.i = fsub fast float %v74.i.i.i, %14
  %v81.i.i.i = fmul fast float %v80.i.i.i, %.i0.i.55.i.i
  %v82.i.i.i = fsub fast float %v75.i.i.i, %15
  %v83.i.i.i = fmul fast float %v82.i.i.i, %.i1.i.56.i.i
  %v84.i.i.i = fsub fast float %v76.i.i.i, %16
  %v85.i.i.i = fmul fast float %v84.i.i.i, %.i2.i.i.i
  %FAbs238.i.i.i = call float @dx.op.unary.f32(i32 6, float %.i0.i.55.i.i) #2
  %FAbs239.i.i.i = call float @dx.op.unary.f32(i32 6, float %.i1.i.56.i.i) #2
  %FAbs240.i.i.i = call float @dx.op.unary.f32(i32 6, float %.i2.i.i.i) #2
  %.i0305.i.i.i = fmul fast float %v77.i.i.i, %FAbs238.i.i.i
  %.i1306.i.i.i = fmul fast float %FAbs239.i.i.i, %v78.i.i.i
  %.i2307.i.i.i = fmul fast float %FAbs240.i.i.i, %v79.i.i.i
  %.i0308.i.i.i = fadd fast float %.i0305.i.i.i, %v81.i.i.i
  %.i1309.i.i.i = fadd fast float %.i1306.i.i.i, %v83.i.i.i
  %.i2310.i.i.i = fadd fast float %.i2307.i.i.i, %v85.i.i.i
  %.i0314.i.i.i = fsub fast float %v81.i.i.i, %.i0305.i.i.i
  %.i1315.i.i.i = fsub fast float %v83.i.i.i, %.i1306.i.i.i
  %.i2316.i.i.i = fsub fast float %v85.i.i.i, %.i2307.i.i.i
  %FMax277.i.i.i = call float @dx.op.binary.f32(i32 35, float %.i0314.i.i.i, float %.i1315.i.i.i) #2
  %FMax276.i.i.i = call float @dx.op.binary.f32(i32 35, float %FMax277.i.i.i, float %.i2316.i.i.i) #2
  %FMin275.i.i.i = call float @dx.op.binary.f32(i32 36, float %.i0308.i.i.i, float %.i1309.i.i.i) #2
  %FMin274.i.i.i = call float @dx.op.binary.f32(i32 36, float %FMin275.i.i.i, float %.i2310.i.i.i) #2
  %FMax273.i.i.i = call float @dx.op.binary.f32(i32 35, float %FMax276.i.i.i, float 0.000000e+00) #2
  %FMin272.i.i.i = call float @dx.op.binary.f32(i32 36, float %FMin274.i.i.i, float %21) #2
  %v86.i.i.i = fcmp fast olt float %FMax273.i.i.i, %FMin272.i.i.i
  br i1 %v86.i.i.i, label %.lr.ph.preheader.critedge.i.i.i, label %._crit_edge.i.i.i

._crit_edge.i.i.i:                                ; preds = %53, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i"
  %.i038 = phi float [ %.i0, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i" ], [ %.i050, %53 ]
  %.i139 = phi float [ %.i1, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i" ], [ %.i151, %53 ]
  %.i240 = phi float [ %.i2, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i" ], [ %.i252, %53 ]
  %.i341 = phi float [ %.i3, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i" ], [ %.i353, %53 ]
  %.i442 = phi float [ %.i4, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i" ], [ %.i454, %53 ]
  %.i543 = phi float [ %.i5, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i" ], [ %.i555, %53 ]
  %.i644 = phi float [ %.i6, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i" ], [ %.i656, %53 ]
  %.i745 = phi float [ %.i7, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i" ], [ %.i757, %53 ]
  %.i846 = phi float [ %.i8, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i" ], [ %.i858, %53 ]
  %.i947 = phi float [ %.i9, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i" ], [ %.i959, %53 ]
  %.i1048 = phi float [ %.i10, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i" ], [ %.i1060, %53 ]
  %.i1149 = phi float [ %.i11, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i" ], [ %.i1161, %53 ]
  %.i062 = phi float [ %.i014, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i" ], [ %.i074, %53 ]
  %.i163 = phi float [ %.i115, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i" ], [ %.i175, %53 ]
  %.i264 = phi float [ %.i216, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i" ], [ %.i276, %53 ]
  %.i365 = phi float [ %.i317, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i" ], [ %.i377, %53 ]
  %.i466 = phi float [ %.i418, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i" ], [ %.i478, %53 ]
  %.i567 = phi float [ %.i519, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i" ], [ %.i579, %53 ]
  %.i668 = phi float [ %.i620, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i" ], [ %.i680, %53 ]
  %.i769 = phi float [ %.i721, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i" ], [ %.i781, %53 ]
  %.i870 = phi float [ %.i822, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i" ], [ %.i882, %53 ]
  %.i971 = phi float [ %.i923, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i" ], [ %.i983, %53 ]
  %.i1072 = phi float [ %.i1024, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i" ], [ %.i1084, %53 ]
  %.i1173 = phi float [ %.i1125, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i" ], [ %.i1185, %53 ]
  %25 = phi i32 [ %sub10.i.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i" ], [ %48, %53 ]
  %26 = phi i32 [ %sub6.i.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i" ], [ %49, %53 ]
  %27 = phi i32 [ -1, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i" ], [ %50, %53 ]
  %28 = phi i32 [ %10, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i" ], [ %51, %53 ]
  %v427.i.i.i = icmp eq i32 %27, -1
  br i1 %v427.i.i.i, label %34, label %29

; <label>:29                                      ; preds = %._crit_edge.i.i.i
  %v428.i.i.i = and i32 %arg0.int.i.i.i, 8
  %v429.i.i.i = icmp eq i32 %v428.i.i.i, 0
  br i1 %v429.i.i.i, label %33, label %"\01?Traverse@@YAHIIII@Z.exit.i.i.i"

"\01?Traverse@@YAHIIII@Z.exit.i.i.i":             ; preds = %29, %34, %33
  %stateID.i.0.i.i.i = phi i32 [ %v436.i.i.i, %33 ], [ %v442.i.i.i, %34 ], [ 0, %29 ]
  %v443.i.i.i = icmp eq i32 %stateID.i.0.i.i.i, 0
  br i1 %v443.i.i.i, label %30, label %remat_begin.i.57.i.i

remat_begin.i.57.i.i:                             ; preds = %"\01?Traverse@@YAHIIII@Z.exit.i.i.i"
  %arrayidx.i.67.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %sub.i.137.i.i.i
  store i32 1005, i32* %arrayidx.i.67.i.i.i
  %add.i.63.i.i.i = add nsw i32 %sub.i.137.i.i.i, 4
  %arrayidx.i.65.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.63.i.i.i
  store i32 %9, i32* %arrayidx.i.65.i.i.i
  br label %while.cond.i

; <label>:30                                      ; preds = %"\01?Traverse@@YAHIIII@Z.exit.i.i.i"
  %add.i.52.i.i.i = add nsw i32 %6, -1
  %arrayidx.i.54.i.59.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.52.i.i.i
  %31 = load i32, i32* %arrayidx.i.54.i.59.i.i, align 4
  %add2.i.57.i.i.i = add nsw i32 %6, -2
  %arrayidx4.i.59.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add2.i.57.i.i.i
  %32 = load i32, i32* %arrayidx4.i.59.i.i.i, align 4
  %arrayidx.i.49.i.60.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %6
  %ret.stateId.i.61.i.i = load i32, i32* %arrayidx.i.49.i.60.i.i
  br label %while.cond.i

; <label>:33                                      ; preds = %29
  %v431.i.i.i = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %Constants_cbuffer, i32 0) #2
  %v432.i.i.i = extractvalue %dx.types.CBufRet.i32 %v431.i.i.i, 2
  %v433.i.i.i = mul i32 %v432.i.i.i, %28
  %v435.i.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %HitGroupShaderTable_texture_rawbuf, i32 %v433.i.i.i, i32 undef) #2
  %v436.i.i.i = extractvalue %dx.types.ResRet.i32 %v435.i.i.i, 0
  br label %"\01?Traverse@@YAHIIII@Z.exit.i.i.i"

; <label>:34                                      ; preds = %._crit_edge.i.i.i
  %v437.i.i.i = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %Constants_cbuffer, i32 0) #2
  %v438.i.i.i = extractvalue %dx.types.CBufRet.i32 %v437.i.i.i, 3
  %v439.i.i.i = mul i32 %v438.i.i.i, %arg4.int.i.i.i
  %v441.i.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %MissShaderTable_texture_rawbuf, i32 %v439.i.i.i, i32 undef) #2
  %v442.i.i.i = extractvalue %dx.types.ResRet.i32 %v441.i.i.i, 0
  br label %"\01?Traverse@@YAHIIII@Z.exit.i.i.i"

.lr.ph.preheader.critedge.i.i.i:                  ; preds = %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.i.i.i.i.i"
  %v87.i.i.i = getelementptr [2048 x i32], [2048 x i32] addrspace(3)* @"\01?stack@@3PAIA", i32 0, i32 %groupIndex.i
  store i32 0, i32 addrspace(3)* %v87.i.i.i, align 4, !tbaa !49, !noalias !51
  store i32 1, i32* %v56.i.i.i, align 4, !tbaa !49
  %v88.i.i.i = getelementptr inbounds [3 x float], [3 x float]* %v22.i.52.i.i, i32 0, i32 0
  %v89.i.i.i = getelementptr inbounds [3 x float], [3 x float]* %v22.i.52.i.i, i32 0, i32 1
  %v90.i.i.i = getelementptr inbounds [3 x float], [3 x float]* %v22.i.52.i.i, i32 0, i32 2
  %v91.i.i.i = getelementptr inbounds [2 x i32], [2 x i32]* %nodesToProcess.i.i.i.i.i, i32 0, i32 1
  %v92.i.i.i = getelementptr inbounds [3 x float], [3 x float]* %v21.i.51.i.i, i32 0, i32 0
  %v93.i.i.i = getelementptr inbounds [3 x float], [3 x float]* %v21.i.51.i.i, i32 0, i32 1
  %v94.i.i.i = getelementptr inbounds [3 x float], [3 x float]* %v21.i.51.i.i, i32 0, i32 2
  %v95.i.i.i = getelementptr inbounds [3 x float], [3 x float]* %v19.i.49.i.i, i32 0, i32 0
  %v96.i.i.i = getelementptr inbounds [3 x float], [3 x float]* %v19.i.49.i.i, i32 0, i32 1
  %v97.i.i.i = getelementptr inbounds [3 x float], [3 x float]* %v19.i.49.i.i, i32 0, i32 2
  %v98.i.i.i = getelementptr inbounds [3 x float], [3 x float]* %v20.i.50.i.i, i32 0, i32 0
  %v99.i.i.i = getelementptr inbounds [3 x float], [3 x float]* %v20.i.50.i.i, i32 0, i32 1
  %v100.i.i.i = getelementptr inbounds [3 x float], [3 x float]* %v20.i.50.i.i, i32 0, i32 2
  br label %.lr.ph.i.i.i

.lr.ph.i.i.i:                                     ; preds = %53, %.lr.ph.preheader.critedge.i.i.i
  %.i086 = phi float [ %.i0, %.lr.ph.preheader.critedge.i.i.i ], [ %.i050, %53 ]
  %.i187 = phi float [ %.i1, %.lr.ph.preheader.critedge.i.i.i ], [ %.i151, %53 ]
  %.i288 = phi float [ %.i2, %.lr.ph.preheader.critedge.i.i.i ], [ %.i252, %53 ]
  %.i389 = phi float [ %.i3, %.lr.ph.preheader.critedge.i.i.i ], [ %.i353, %53 ]
  %.i490 = phi float [ %.i4, %.lr.ph.preheader.critedge.i.i.i ], [ %.i454, %53 ]
  %.i591 = phi float [ %.i5, %.lr.ph.preheader.critedge.i.i.i ], [ %.i555, %53 ]
  %.i692 = phi float [ %.i6, %.lr.ph.preheader.critedge.i.i.i ], [ %.i656, %53 ]
  %.i793 = phi float [ %.i7, %.lr.ph.preheader.critedge.i.i.i ], [ %.i757, %53 ]
  %.i894 = phi float [ %.i8, %.lr.ph.preheader.critedge.i.i.i ], [ %.i858, %53 ]
  %.i995 = phi float [ %.i9, %.lr.ph.preheader.critedge.i.i.i ], [ %.i959, %53 ]
  %.i1096 = phi float [ %.i10, %.lr.ph.preheader.critedge.i.i.i ], [ %.i1060, %53 ]
  %.i1197 = phi float [ %.i11, %.lr.ph.preheader.critedge.i.i.i ], [ %.i1161, %53 ]
  %.i098 = phi float [ %.i014, %.lr.ph.preheader.critedge.i.i.i ], [ %.i074, %53 ]
  %.i199 = phi float [ %.i115, %.lr.ph.preheader.critedge.i.i.i ], [ %.i175, %53 ]
  %.i2100 = phi float [ %.i216, %.lr.ph.preheader.critedge.i.i.i ], [ %.i276, %53 ]
  %.i3101 = phi float [ %.i317, %.lr.ph.preheader.critedge.i.i.i ], [ %.i377, %53 ]
  %.i4102 = phi float [ %.i418, %.lr.ph.preheader.critedge.i.i.i ], [ %.i478, %53 ]
  %.i5103 = phi float [ %.i519, %.lr.ph.preheader.critedge.i.i.i ], [ %.i579, %53 ]
  %.i6104 = phi float [ %.i620, %.lr.ph.preheader.critedge.i.i.i ], [ %.i680, %53 ]
  %.i7105 = phi float [ %.i721, %.lr.ph.preheader.critedge.i.i.i ], [ %.i781, %53 ]
  %.i8106 = phi float [ %.i822, %.lr.ph.preheader.critedge.i.i.i ], [ %.i882, %53 ]
  %.i9107 = phi float [ %.i923, %.lr.ph.preheader.critedge.i.i.i ], [ %.i983, %53 ]
  %.i10108 = phi float [ %.i1024, %.lr.ph.preheader.critedge.i.i.i ], [ %.i1084, %53 ]
  %.i11109 = phi float [ %.i1125, %.lr.ph.preheader.critedge.i.i.i ], [ %.i1185, %53 ]
  %35 = phi i32 [ %sub10.i.i.i.i, %.lr.ph.preheader.critedge.i.i.i ], [ %48, %53 ]
  %36 = phi i32 [ %sub6.i.i.i.i, %.lr.ph.preheader.critedge.i.i.i ], [ %49, %53 ]
  %37 = phi i32 [ -1, %.lr.ph.preheader.critedge.i.i.i ], [ %50, %53 ]
  %38 = phi i32 [ %10, %.lr.ph.preheader.critedge.i.i.i ], [ %51, %53 ]
  %39 = phi float [ %21, %.lr.ph.preheader.critedge.i.i.i ], [ %52, %53 ]
  %resultTriId.i.i.0188.i.i.i = phi i32 [ undef, %.lr.ph.preheader.critedge.i.i.i ], [ %resultTriId.i.i.2.i.i.i, %53 ]
  %resultBary.i.i.0187.i0.i.i.i = phi float [ undef, %.lr.ph.preheader.critedge.i.i.i ], [ %resultBary.i.i.2.i0.i.i.i, %53 ]
  %resultBary.i.i.0187.i1.i.i.i = phi float [ undef, %.lr.ph.preheader.critedge.i.i.i ], [ %resultBary.i.i.2.i1.i.i.i, %53 ]
  %stackPointer.i.i.1185.i.i.i = phi i32 [ 1, %.lr.ph.preheader.critedge.i.i.i ], [ %stackPointer.i.i.3.i.i.i, %53 ]
  %instId.i.i.0184.i.i.i = phi i32 [ 0, %.lr.ph.preheader.critedge.i.i.i ], [ %instId.i.i.2.i.i.i, %53 ]
  %instOffset.i.i.0183.i.i.i = phi i32 [ 0, %.lr.ph.preheader.critedge.i.i.i ], [ %instOffset.i.i.2.i.i.i, %53 ]
  %instFlags.i.i.0182.i.i.i = phi i32 [ 0, %.lr.ph.preheader.critedge.i.i.i ], [ %instFlags.i.i.2.i.i.i, %53 ]
  %instIdx.i.i.0181.i.i.i = phi i32 [ 0, %.lr.ph.preheader.critedge.i.i.i ], [ %instIdx.i.i.2.i.i.i, %53 ]
  %currentGpuVA.i.i.0180.i0.i.i.i = phi i32 [ %v54.i.i.i, %.lr.ph.preheader.critedge.i.i.i ], [ %v423.i.i.i, %53 ]
  %currentGpuVA.i.i.0180.i1.i.i.i = phi i32 [ %v55.i.i.i, %.lr.ph.preheader.critedge.i.i.i ], [ %v424.i.i.i, %53 ]
  %currentBVHIndex.i.i.0179.i.i.i = phi i32 [ 0, %.lr.ph.preheader.critedge.i.i.i ], [ %v421.i.i.i, %53 ]
  %CurrentWorldToObject.i.i168.0178.i0.i.i.i = phi float [ 0.000000e+00, %.lr.ph.preheader.critedge.i.i.i ], [ %CurrentWorldToObject.i.i168.2.i0.i.i.i, %53 ]
  %CurrentWorldToObject.i.i168.0178.i1.i.i.i = phi float [ 0.000000e+00, %.lr.ph.preheader.critedge.i.i.i ], [ %CurrentWorldToObject.i.i168.2.i1.i.i.i, %53 ]
  %CurrentWorldToObject.i.i168.0178.i2.i.i.i = phi float [ 0.000000e+00, %.lr.ph.preheader.critedge.i.i.i ], [ %CurrentWorldToObject.i.i168.2.i2.i.i.i, %53 ]
  %CurrentWorldToObject.i.i168.0178.i3.i.i.i = phi float [ 0.000000e+00, %.lr.ph.preheader.critedge.i.i.i ], [ %CurrentWorldToObject.i.i168.2.i3.i.i.i, %53 ]
  %CurrentWorldToObject.i.i168.0178.i4.i.i.i = phi float [ 0.000000e+00, %.lr.ph.preheader.critedge.i.i.i ], [ %CurrentWorldToObject.i.i168.2.i4.i.i.i, %53 ]
  %CurrentWorldToObject.i.i168.0178.i5.i.i.i = phi float [ 0.000000e+00, %.lr.ph.preheader.critedge.i.i.i ], [ %CurrentWorldToObject.i.i168.2.i5.i.i.i, %53 ]
  %CurrentWorldToObject.i.i168.0178.i6.i.i.i = phi float [ 0.000000e+00, %.lr.ph.preheader.critedge.i.i.i ], [ %CurrentWorldToObject.i.i168.2.i6.i.i.i, %53 ]
  %CurrentWorldToObject.i.i168.0178.i7.i.i.i = phi float [ 0.000000e+00, %.lr.ph.preheader.critedge.i.i.i ], [ %CurrentWorldToObject.i.i168.2.i7.i.i.i, %53 ]
  %CurrentWorldToObject.i.i168.0178.i8.i.i.i = phi float [ 0.000000e+00, %.lr.ph.preheader.critedge.i.i.i ], [ %CurrentWorldToObject.i.i168.2.i8.i.i.i, %53 ]
  %CurrentWorldToObject.i.i168.0178.i9.i.i.i = phi float [ 0.000000e+00, %.lr.ph.preheader.critedge.i.i.i ], [ %CurrentWorldToObject.i.i168.2.i9.i.i.i, %53 ]
  %CurrentWorldToObject.i.i168.0178.i10.i.i.i = phi float [ 0.000000e+00, %.lr.ph.preheader.critedge.i.i.i ], [ %CurrentWorldToObject.i.i168.2.i10.i.i.i, %53 ]
  %CurrentWorldToObject.i.i168.0178.i11.i.i.i = phi float [ 0.000000e+00, %.lr.ph.preheader.critedge.i.i.i ], [ %CurrentWorldToObject.i.i168.2.i11.i.i.i, %53 ]
  %CurrentObjectToWorld.i.i167.0177.i0.i.i.i = phi float [ 0.000000e+00, %.lr.ph.preheader.critedge.i.i.i ], [ %CurrentObjectToWorld.i.i167.2.i0.i.i.i, %53 ]
  %CurrentObjectToWorld.i.i167.0177.i1.i.i.i = phi float [ 0.000000e+00, %.lr.ph.preheader.critedge.i.i.i ], [ %CurrentObjectToWorld.i.i167.2.i1.i.i.i, %53 ]
  %CurrentObjectToWorld.i.i167.0177.i2.i.i.i = phi float [ 0.000000e+00, %.lr.ph.preheader.critedge.i.i.i ], [ %CurrentObjectToWorld.i.i167.2.i2.i.i.i, %53 ]
  %CurrentObjectToWorld.i.i167.0177.i3.i.i.i = phi float [ 0.000000e+00, %.lr.ph.preheader.critedge.i.i.i ], [ %CurrentObjectToWorld.i.i167.2.i3.i.i.i, %53 ]
  %CurrentObjectToWorld.i.i167.0177.i4.i.i.i = phi float [ 0.000000e+00, %.lr.ph.preheader.critedge.i.i.i ], [ %CurrentObjectToWorld.i.i167.2.i4.i.i.i, %53 ]
  %CurrentObjectToWorld.i.i167.0177.i5.i.i.i = phi float [ 0.000000e+00, %.lr.ph.preheader.critedge.i.i.i ], [ %CurrentObjectToWorld.i.i167.2.i5.i.i.i, %53 ]
  %CurrentObjectToWorld.i.i167.0177.i6.i.i.i = phi float [ 0.000000e+00, %.lr.ph.preheader.critedge.i.i.i ], [ %CurrentObjectToWorld.i.i167.2.i6.i.i.i, %53 ]
  %CurrentObjectToWorld.i.i167.0177.i7.i.i.i = phi float [ 0.000000e+00, %.lr.ph.preheader.critedge.i.i.i ], [ %CurrentObjectToWorld.i.i167.2.i7.i.i.i, %53 ]
  %CurrentObjectToWorld.i.i167.0177.i8.i.i.i = phi float [ 0.000000e+00, %.lr.ph.preheader.critedge.i.i.i ], [ %CurrentObjectToWorld.i.i167.2.i8.i.i.i, %53 ]
  %CurrentObjectToWorld.i.i167.0177.i9.i.i.i = phi float [ 0.000000e+00, %.lr.ph.preheader.critedge.i.i.i ], [ %CurrentObjectToWorld.i.i167.2.i9.i.i.i, %53 ]
  %CurrentObjectToWorld.i.i167.0177.i10.i.i.i = phi float [ 0.000000e+00, %.lr.ph.preheader.critedge.i.i.i ], [ %CurrentObjectToWorld.i.i167.2.i10.i.i.i, %53 ]
  %CurrentObjectToWorld.i.i167.0177.i11.i.i.i = phi float [ 0.000000e+00, %.lr.ph.preheader.critedge.i.i.i ], [ %CurrentObjectToWorld.i.i167.2.i11.i.i.i, %53 ]
  %.0174176.i0.i.i.i = phi float [ undef, %.lr.ph.preheader.critedge.i.i.i ], [ %.3.i0.i.i.i, %53 ]
  %.0174176.i1.i.i.i = phi float [ undef, %.lr.ph.preheader.critedge.i.i.i ], [ %.3.i1.i.i.i, %53 ]
  %.phi.trans.insert.i.i.i = getelementptr inbounds [2 x i32], [2 x i32]* %nodesToProcess.i.i.i.i.i, i32 0, i32 %currentBVHIndex.i.i.0179.i.i.i
  %.pre.i.i.i = load i32, i32* %.phi.trans.insert.i.i.i, align 4, !tbaa !49
  br label %._crit_edge.2.i.i.i

._crit_edge.2.i.i.i:                              ; preds = %._crit_edge.1.i.i.i, %.lr.ph.i.i.i
  %.i0110 = phi float [ %.i086, %.lr.ph.i.i.i ], [ %.i050, %._crit_edge.1.i.i.i ]
  %.i1111 = phi float [ %.i187, %.lr.ph.i.i.i ], [ %.i151, %._crit_edge.1.i.i.i ]
  %.i2112 = phi float [ %.i288, %.lr.ph.i.i.i ], [ %.i252, %._crit_edge.1.i.i.i ]
  %.i3113 = phi float [ %.i389, %.lr.ph.i.i.i ], [ %.i353, %._crit_edge.1.i.i.i ]
  %.i4114 = phi float [ %.i490, %.lr.ph.i.i.i ], [ %.i454, %._crit_edge.1.i.i.i ]
  %.i5115 = phi float [ %.i591, %.lr.ph.i.i.i ], [ %.i555, %._crit_edge.1.i.i.i ]
  %.i6116 = phi float [ %.i692, %.lr.ph.i.i.i ], [ %.i656, %._crit_edge.1.i.i.i ]
  %.i7117 = phi float [ %.i793, %.lr.ph.i.i.i ], [ %.i757, %._crit_edge.1.i.i.i ]
  %.i8118 = phi float [ %.i894, %.lr.ph.i.i.i ], [ %.i858, %._crit_edge.1.i.i.i ]
  %.i9119 = phi float [ %.i995, %.lr.ph.i.i.i ], [ %.i959, %._crit_edge.1.i.i.i ]
  %.i10120 = phi float [ %.i1096, %.lr.ph.i.i.i ], [ %.i1060, %._crit_edge.1.i.i.i ]
  %.i11121 = phi float [ %.i1197, %.lr.ph.i.i.i ], [ %.i1161, %._crit_edge.1.i.i.i ]
  %.i0122 = phi float [ %.i098, %.lr.ph.i.i.i ], [ %.i074, %._crit_edge.1.i.i.i ]
  %.i1123 = phi float [ %.i199, %.lr.ph.i.i.i ], [ %.i175, %._crit_edge.1.i.i.i ]
  %.i2124 = phi float [ %.i2100, %.lr.ph.i.i.i ], [ %.i276, %._crit_edge.1.i.i.i ]
  %.i3125 = phi float [ %.i3101, %.lr.ph.i.i.i ], [ %.i377, %._crit_edge.1.i.i.i ]
  %.i4126 = phi float [ %.i4102, %.lr.ph.i.i.i ], [ %.i478, %._crit_edge.1.i.i.i ]
  %.i5127 = phi float [ %.i5103, %.lr.ph.i.i.i ], [ %.i579, %._crit_edge.1.i.i.i ]
  %.i6128 = phi float [ %.i6104, %.lr.ph.i.i.i ], [ %.i680, %._crit_edge.1.i.i.i ]
  %.i7129 = phi float [ %.i7105, %.lr.ph.i.i.i ], [ %.i781, %._crit_edge.1.i.i.i ]
  %.i8130 = phi float [ %.i8106, %.lr.ph.i.i.i ], [ %.i882, %._crit_edge.1.i.i.i ]
  %.i9131 = phi float [ %.i9107, %.lr.ph.i.i.i ], [ %.i983, %._crit_edge.1.i.i.i ]
  %.i10132 = phi float [ %.i10108, %.lr.ph.i.i.i ], [ %.i1084, %._crit_edge.1.i.i.i ]
  %.i11133 = phi float [ %.i11109, %.lr.ph.i.i.i ], [ %.i1185, %._crit_edge.1.i.i.i ]
  %40 = phi i32 [ %35, %.lr.ph.i.i.i ], [ %48, %._crit_edge.1.i.i.i ]
  %41 = phi i32 [ %36, %.lr.ph.i.i.i ], [ %49, %._crit_edge.1.i.i.i ]
  %42 = phi i32 [ %37, %.lr.ph.i.i.i ], [ %50, %._crit_edge.1.i.i.i ]
  %43 = phi i32 [ %38, %.lr.ph.i.i.i ], [ %51, %._crit_edge.1.i.i.i ]
  %44 = phi float [ %39, %.lr.ph.i.i.i ], [ %52, %._crit_edge.1.i.i.i ]
  %v102.i.i.i = phi i32 [ %.pre.i.i.i, %.lr.ph.i.i.i ], [ %v419.i.i.i, %._crit_edge.1.i.i.i ]
  %.1.i0.i.i.i = phi float [ %.0174176.i0.i.i.i, %.lr.ph.i.i.i ], [ %.3.i0.i.i.i, %._crit_edge.1.i.i.i ]
  %.1.i1.i.i.i = phi float [ %.0174176.i1.i.i.i, %.lr.ph.i.i.i ], [ %.3.i1.i.i.i, %._crit_edge.1.i.i.i ]
  %currentRayData.i.i.0.1.i0.i.i.i = phi float [ %14, %.lr.ph.i.i.i ], [ %currentRayData.i.i.0.2.i0.i.i.i, %._crit_edge.1.i.i.i ]
  %currentRayData.i.i.0.1.i1.i.i.i = phi float [ %15, %.lr.ph.i.i.i ], [ %currentRayData.i.i.0.2.i1.i.i.i, %._crit_edge.1.i.i.i ]
  %currentRayData.i.i.0.1.i2.i.i.i = phi float [ %16, %.lr.ph.i.i.i ], [ %currentRayData.i.i.0.2.i2.i.i.i, %._crit_edge.1.i.i.i ]
  %currentRayData.i.i.1.1.i0.i.i.i = phi float [ %18, %.lr.ph.i.i.i ], [ %currentRayData.i.i.1.2.i0.i.i.i, %._crit_edge.1.i.i.i ]
  %currentRayData.i.i.1.1.i1.i.i.i = phi float [ %19, %.lr.ph.i.i.i ], [ %currentRayData.i.i.1.2.i1.i.i.i, %._crit_edge.1.i.i.i ]
  %currentRayData.i.i.1.1.i2.i.i.i = phi float [ %20, %.lr.ph.i.i.i ], [ %currentRayData.i.i.1.2.i2.i.i.i, %._crit_edge.1.i.i.i ]
  %currentRayData.i.i.2.1.i0.i.i.i = phi float [ %.i0.i.55.i.i, %.lr.ph.i.i.i ], [ %currentRayData.i.i.2.2.i0.i.i.i, %._crit_edge.1.i.i.i ]
  %currentRayData.i.i.2.1.i1.i.i.i = phi float [ %.i1.i.56.i.i, %.lr.ph.i.i.i ], [ %currentRayData.i.i.2.2.i1.i.i.i, %._crit_edge.1.i.i.i ]
  %currentRayData.i.i.2.1.i2.i.i.i = phi float [ %.i2.i.i.i, %.lr.ph.i.i.i ], [ %currentRayData.i.i.2.2.i2.i.i.i, %._crit_edge.1.i.i.i ]
  %currentRayData.i.i.3.1.i0.i.i.i = phi float [ %.i0296.i.i.i, %.lr.ph.i.i.i ], [ %currentRayData.i.i.3.2.i0.i.i.i, %._crit_edge.1.i.i.i ]
  %currentRayData.i.i.3.1.i1.i.i.i = phi float [ %.i1297.i.i.i, %.lr.ph.i.i.i ], [ %currentRayData.i.i.3.2.i1.i.i.i, %._crit_edge.1.i.i.i ]
  %currentRayData.i.i.3.1.i2.i.i.i = phi float [ %.i2298.i.i.i, %.lr.ph.i.i.i ], [ %currentRayData.i.i.3.2.i2.i.i.i, %._crit_edge.1.i.i.i ]
  %currentRayData.i.i.4.1.i0.i.i.i = phi float [ %v48.i.i.i, %.lr.ph.i.i.i ], [ %currentRayData.i.i.4.2.i0.i.i.i, %._crit_edge.1.i.i.i ]
  %currentRayData.i.i.4.1.i1.i.i.i = phi float [ %v51.i.i.i, %.lr.ph.i.i.i ], [ %currentRayData.i.i.4.2.i1.i.i.i, %._crit_edge.1.i.i.i ]
  %currentRayData.i.i.4.1.i2.i.i.i = phi float [ %v52.i.i.i, %.lr.ph.i.i.i ], [ %currentRayData.i.i.4.2.i2.i.i.i, %._crit_edge.1.i.i.i ]
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
  %v103.i.i.i = add nsw i32 %stackPointer.i.i.2.i.i.i, -1
  %v104.i.i.i = shl i32 %v103.i.i.i, 6
  %v105.i.i.i = add i32 %v104.i.i.i, %groupIndex.i
  %v106.i.i.i = getelementptr [2048 x i32], [2048 x i32] addrspace(3)* @"\01?stack@@3PAIA", i32 0, i32 %v105.i.i.i
  %v107.i.i.i = load i32, i32 addrspace(3)* %v106.i.i.i, align 4, !tbaa !49, !noalias !54
  %v108.i.i.i = getelementptr inbounds [2 x i32], [2 x i32]* %nodesToProcess.i.i.i.i.i, i32 0, i32 %currentBVHIndex.i.i.1.i.i.i
  %v109.i.i.i = add i32 %v102.i.i.i, -1
  store i32 %v109.i.i.i, i32* %v108.i.i.i, align 4, !tbaa !49
  %45 = add i32 %currentGpuVA.i.i.1.i1.i.i.i, 0
  %DescriptorHeapBufferTable_UAV_rawbuf = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 1, i32 1, i32 %45, i1 true)
  %v112.i.i.i = add i32 %currentGpuVA.i.i.1.i0.i.i.i, 16
  %v113.i.i.i = shl i32 %v107.i.i.i, 5
  %v114.i.i.i = add i32 %v112.i.i.i, %v113.i.i.i
  %v116.i.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %DescriptorHeapBufferTable_UAV_rawbuf, i32 %v114.i.i.i, i32 undef) #2
  %v117.i.i.i = extractvalue %dx.types.ResRet.i32 %v116.i.i.i, 3
  %v118.i.i.i = add i32 %v114.i.i.i, 16
  %v119.i.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %DescriptorHeapBufferTable_UAV_rawbuf, i32 %v118.i.i.i, i32 undef) #2
  %v120.i.i.i = extractvalue %dx.types.ResRet.i32 %v119.i.i.i, 3
  %v121.i.i.i = icmp slt i32 %v117.i.i.i, 0
  br i1 %v121.i.i.i, label %55, label %46

; <label>:46                                      ; preds = %._crit_edge.2.i.i.i
  %v369.i.i.i = and i32 %v117.i.i.i, 16777215
  %v370.i.i.i = shl nuw nsw i32 %v369.i.i.i, 5
  %v371.i.i.i = add i32 %v112.i.i.i, %v370.i.i.i
  %v373.i.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %DescriptorHeapBufferTable_UAV_rawbuf, i32 %v371.i.i.i, i32 undef) #2
  %v374.i.i.i = extractvalue %dx.types.ResRet.i32 %v373.i.i.i, 0
  %v375.i.i.i = extractvalue %dx.types.ResRet.i32 %v373.i.i.i, 1
  %v376.i.i.i = extractvalue %dx.types.ResRet.i32 %v373.i.i.i, 2
  %v377.i.i.i = add i32 %v371.i.i.i, 16
  %v378.i.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %DescriptorHeapBufferTable_UAV_rawbuf, i32 %v377.i.i.i, i32 undef) #2
  %v379.i.i.i = extractvalue %dx.types.ResRet.i32 %v378.i.i.i, 0
  %v380.i.i.i = extractvalue %dx.types.ResRet.i32 %v378.i.i.i, 1
  %v381.i.i.i = extractvalue %dx.types.ResRet.i32 %v378.i.i.i, 2
  %v382.i.i.i = bitcast i32 %v374.i.i.i to float
  %v383.i.i.i = bitcast i32 %v375.i.i.i to float
  %v384.i.i.i = bitcast i32 %v376.i.i.i to float
  %v385.i.i.i = bitcast i32 %v379.i.i.i to float
  %v386.i.i.i = bitcast i32 %v380.i.i.i to float
  %v387.i.i.i = bitcast i32 %v381.i.i.i to float
  %v388.i.i.i = shl i32 %v120.i.i.i, 5
  %v389.i.i.i = add i32 %v112.i.i.i, %v388.i.i.i
  %v390.i.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %DescriptorHeapBufferTable_UAV_rawbuf, i32 %v389.i.i.i, i32 undef) #2
  %v391.i.i.i = extractvalue %dx.types.ResRet.i32 %v390.i.i.i, 0
  %v392.i.i.i = extractvalue %dx.types.ResRet.i32 %v390.i.i.i, 1
  %v393.i.i.i = extractvalue %dx.types.ResRet.i32 %v390.i.i.i, 2
  %v394.i.i.i = add i32 %v389.i.i.i, 16
  %v395.i.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %DescriptorHeapBufferTable_UAV_rawbuf, i32 %v394.i.i.i, i32 undef) #2
  %v396.i.i.i = extractvalue %dx.types.ResRet.i32 %v395.i.i.i, 0
  %v397.i.i.i = extractvalue %dx.types.ResRet.i32 %v395.i.i.i, 1
  %v398.i.i.i = extractvalue %dx.types.ResRet.i32 %v395.i.i.i, 2
  %v399.i.i.i = bitcast i32 %v391.i.i.i to float
  %v400.i.i.i = bitcast i32 %v392.i.i.i to float
  %v401.i.i.i = bitcast i32 %v393.i.i.i to float
  %v402.i.i.i = bitcast i32 %v396.i.i.i to float
  %v403.i.i.i = bitcast i32 %v397.i.i.i to float
  %v404.i.i.i = bitcast i32 %v398.i.i.i to float
  %.i0378.i.i.i = fmul fast float %v382.i.i.i, %currentRayData.i.i.2.1.i0.i.i.i
  %.i1379.i.i.i = fmul fast float %v383.i.i.i, %currentRayData.i.i.2.1.i1.i.i.i
  %.i2380.i.i.i = fmul fast float %v384.i.i.i, %currentRayData.i.i.2.1.i2.i.i.i
  %.i0381.i.i.i = fsub fast float %.i0378.i.i.i, %currentRayData.i.i.3.1.i0.i.i.i
  %.i1382.i.i.i = fsub fast float %.i1379.i.i.i, %currentRayData.i.i.3.1.i1.i.i.i
  %.i2383.i.i.i = fsub fast float %.i2380.i.i.i, %currentRayData.i.i.3.1.i2.i.i.i
  %FAbs232.i.i.i = call float @dx.op.unary.f32(i32 6, float %currentRayData.i.i.2.1.i0.i.i.i) #2
  %FAbs233.i.i.i = call float @dx.op.unary.f32(i32 6, float %currentRayData.i.i.2.1.i1.i.i.i) #2
  %FAbs234.i.i.i = call float @dx.op.unary.f32(i32 6, float %currentRayData.i.i.2.1.i2.i.i.i) #2
  %.i0384.i.i.i = fmul fast float %FAbs232.i.i.i, %v385.i.i.i
  %.i1385.i.i.i = fmul fast float %FAbs233.i.i.i, %v386.i.i.i
  %.i2386.i.i.i = fmul fast float %FAbs234.i.i.i, %v387.i.i.i
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
  %FMin265.i.i.i = call float @dx.op.binary.f32(i32 36, float %FMin268.i.i.i, float %44) #2
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
  %FMin259.i.i.i = call float @dx.op.binary.f32(i32 36, float %FMin261.i.i.i, float %44) #2
  %v406.i.i.i = fcmp fast olt float %FMax.i.i.i, %FMin259.i.i.i
  %v407.i.i.i = and i1 %v405.i.i.i, %v406.i.i.i
  br i1 %v407.i.i.i, label %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i", label %47

; <label>:47                                      ; preds = %46
  %v416.i.i.i = or i1 %v405.i.i.i, %v406.i.i.i
  br i1 %v416.i.i.i, label %54, label %._crit_edge.1.i.i.i

._crit_edge.1.i.i.i:                              ; preds = %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i", %._crit_edge.10.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i", %47, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i", %64, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i", %54
  %.i050 = phi float [ %.i0110, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %.i0110.CurrentWorldToObject.i.i168.1.i0.i.i.i, %64 ], [ %.i0110, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %.i0110, %54 ], [ %.i0110, %47 ], [ %.i0110, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %.i0110.CurrentWorldToObject.i.i168.1.i0.i.i.i, %._crit_edge.10.i.i.i ], [ %.i0110, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %.i151 = phi float [ %.i1111, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %.i1111.CurrentWorldToObject.i.i168.1.i1.i.i.i, %64 ], [ %.i1111, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %.i1111, %54 ], [ %.i1111, %47 ], [ %.i1111, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %.i1111.CurrentWorldToObject.i.i168.1.i1.i.i.i, %._crit_edge.10.i.i.i ], [ %.i1111, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %.i252 = phi float [ %.i2112, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %.i2112.CurrentWorldToObject.i.i168.1.i2.i.i.i, %64 ], [ %.i2112, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %.i2112, %54 ], [ %.i2112, %47 ], [ %.i2112, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %.i2112.CurrentWorldToObject.i.i168.1.i2.i.i.i, %._crit_edge.10.i.i.i ], [ %.i2112, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %.i353 = phi float [ %.i3113, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %.i3113.CurrentWorldToObject.i.i168.1.i3.i.i.i, %64 ], [ %.i3113, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %.i3113, %54 ], [ %.i3113, %47 ], [ %.i3113, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %.i3113.CurrentWorldToObject.i.i168.1.i3.i.i.i, %._crit_edge.10.i.i.i ], [ %.i3113, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %.i454 = phi float [ %.i4114, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %.i4114.CurrentWorldToObject.i.i168.1.i4.i.i.i, %64 ], [ %.i4114, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %.i4114, %54 ], [ %.i4114, %47 ], [ %.i4114, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %.i4114.CurrentWorldToObject.i.i168.1.i4.i.i.i, %._crit_edge.10.i.i.i ], [ %.i4114, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %.i555 = phi float [ %.i5115, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %.i5115.CurrentWorldToObject.i.i168.1.i5.i.i.i, %64 ], [ %.i5115, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %.i5115, %54 ], [ %.i5115, %47 ], [ %.i5115, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %.i5115.CurrentWorldToObject.i.i168.1.i5.i.i.i, %._crit_edge.10.i.i.i ], [ %.i5115, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %.i656 = phi float [ %.i6116, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %.i6116.CurrentWorldToObject.i.i168.1.i6.i.i.i, %64 ], [ %.i6116, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %.i6116, %54 ], [ %.i6116, %47 ], [ %.i6116, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %.i6116.CurrentWorldToObject.i.i168.1.i6.i.i.i, %._crit_edge.10.i.i.i ], [ %.i6116, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %.i757 = phi float [ %.i7117, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %.i7117.CurrentWorldToObject.i.i168.1.i7.i.i.i, %64 ], [ %.i7117, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %.i7117, %54 ], [ %.i7117, %47 ], [ %.i7117, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %.i7117.CurrentWorldToObject.i.i168.1.i7.i.i.i, %._crit_edge.10.i.i.i ], [ %.i7117, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %.i858 = phi float [ %.i8118, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %.i8118.CurrentWorldToObject.i.i168.1.i8.i.i.i, %64 ], [ %.i8118, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %.i8118, %54 ], [ %.i8118, %47 ], [ %.i8118, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %.i8118.CurrentWorldToObject.i.i168.1.i8.i.i.i, %._crit_edge.10.i.i.i ], [ %.i8118, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %.i959 = phi float [ %.i9119, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %.i9119.CurrentWorldToObject.i.i168.1.i9.i.i.i, %64 ], [ %.i9119, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %.i9119, %54 ], [ %.i9119, %47 ], [ %.i9119, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %.i9119.CurrentWorldToObject.i.i168.1.i9.i.i.i, %._crit_edge.10.i.i.i ], [ %.i9119, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %.i1060 = phi float [ %.i10120, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %.i10120.CurrentWorldToObject.i.i168.1.i10.i.i.i, %64 ], [ %.i10120, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %.i10120, %54 ], [ %.i10120, %47 ], [ %.i10120, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %.i10120.CurrentWorldToObject.i.i168.1.i10.i.i.i, %._crit_edge.10.i.i.i ], [ %.i10120, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %.i1161 = phi float [ %.i11121, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %.i11121.CurrentWorldToObject.i.i168.1.i11.i.i.i, %64 ], [ %.i11121, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %.i11121, %54 ], [ %.i11121, %47 ], [ %.i11121, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %.i11121.CurrentWorldToObject.i.i168.1.i11.i.i.i, %._crit_edge.10.i.i.i ], [ %.i11121, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %.i074 = phi float [ %.i0122, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %.i0122.CurrentObjectToWorld.i.i167.1.i0.i.i.i, %64 ], [ %.i0122, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %.i0122, %54 ], [ %.i0122, %47 ], [ %.i0122, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %.i0122.CurrentObjectToWorld.i.i167.1.i0.i.i.i, %._crit_edge.10.i.i.i ], [ %.i0122, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %.i175 = phi float [ %.i1123, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %.i1123.CurrentObjectToWorld.i.i167.1.i1.i.i.i, %64 ], [ %.i1123, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %.i1123, %54 ], [ %.i1123, %47 ], [ %.i1123, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %.i1123.CurrentObjectToWorld.i.i167.1.i1.i.i.i, %._crit_edge.10.i.i.i ], [ %.i1123, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %.i276 = phi float [ %.i2124, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %.i2124.CurrentObjectToWorld.i.i167.1.i2.i.i.i, %64 ], [ %.i2124, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %.i2124, %54 ], [ %.i2124, %47 ], [ %.i2124, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %.i2124.CurrentObjectToWorld.i.i167.1.i2.i.i.i, %._crit_edge.10.i.i.i ], [ %.i2124, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %.i377 = phi float [ %.i3125, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %.i3125.CurrentObjectToWorld.i.i167.1.i3.i.i.i, %64 ], [ %.i3125, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %.i3125, %54 ], [ %.i3125, %47 ], [ %.i3125, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %.i3125.CurrentObjectToWorld.i.i167.1.i3.i.i.i, %._crit_edge.10.i.i.i ], [ %.i3125, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %.i478 = phi float [ %.i4126, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %.i4126.CurrentObjectToWorld.i.i167.1.i4.i.i.i, %64 ], [ %.i4126, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %.i4126, %54 ], [ %.i4126, %47 ], [ %.i4126, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %.i4126.CurrentObjectToWorld.i.i167.1.i4.i.i.i, %._crit_edge.10.i.i.i ], [ %.i4126, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %.i579 = phi float [ %.i5127, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %.i5127.CurrentObjectToWorld.i.i167.1.i5.i.i.i, %64 ], [ %.i5127, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %.i5127, %54 ], [ %.i5127, %47 ], [ %.i5127, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %.i5127.CurrentObjectToWorld.i.i167.1.i5.i.i.i, %._crit_edge.10.i.i.i ], [ %.i5127, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %.i680 = phi float [ %.i6128, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %.i6128.CurrentObjectToWorld.i.i167.1.i6.i.i.i, %64 ], [ %.i6128, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %.i6128, %54 ], [ %.i6128, %47 ], [ %.i6128, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %.i6128.CurrentObjectToWorld.i.i167.1.i6.i.i.i, %._crit_edge.10.i.i.i ], [ %.i6128, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %.i781 = phi float [ %.i7129, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %.i7129.CurrentObjectToWorld.i.i167.1.i7.i.i.i, %64 ], [ %.i7129, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %.i7129, %54 ], [ %.i7129, %47 ], [ %.i7129, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %.i7129.CurrentObjectToWorld.i.i167.1.i7.i.i.i, %._crit_edge.10.i.i.i ], [ %.i7129, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %.i882 = phi float [ %.i8130, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %.i8130.CurrentObjectToWorld.i.i167.1.i8.i.i.i, %64 ], [ %.i8130, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %.i8130, %54 ], [ %.i8130, %47 ], [ %.i8130, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %.i8130.CurrentObjectToWorld.i.i167.1.i8.i.i.i, %._crit_edge.10.i.i.i ], [ %.i8130, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %.i983 = phi float [ %.i9131, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %.i9131.CurrentObjectToWorld.i.i167.1.i9.i.i.i, %64 ], [ %.i9131, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %.i9131, %54 ], [ %.i9131, %47 ], [ %.i9131, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %.i9131.CurrentObjectToWorld.i.i167.1.i9.i.i.i, %._crit_edge.10.i.i.i ], [ %.i9131, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %.i1084 = phi float [ %.i10132, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %.i10132.CurrentObjectToWorld.i.i167.1.i10.i.i.i, %64 ], [ %.i10132, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %.i10132, %54 ], [ %.i10132, %47 ], [ %.i10132, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %.i10132.CurrentObjectToWorld.i.i167.1.i10.i.i.i, %._crit_edge.10.i.i.i ], [ %.i10132, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %.i1185 = phi float [ %.i11133, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %.i11133.CurrentObjectToWorld.i.i167.1.i11.i.i.i, %64 ], [ %.i11133, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %.i11133, %54 ], [ %.i11133, %47 ], [ %.i11133, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %.i11133.CurrentObjectToWorld.i.i167.1.i11.i.i.i, %._crit_edge.10.i.i.i ], [ %.i11133, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %48 = phi i32 [ %40, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %41, %64 ], [ %40, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %40, %54 ], [ %40, %47 ], [ %40, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %41, %._crit_edge.10.i.i.i ], [ %40, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %49 = phi i32 [ %41, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %40, %64 ], [ %41, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %41, %54 ], [ %41, %47 ], [ %41, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %40, %._crit_edge.10.i.i.i ], [ %41, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %50 = phi i32 [ %42, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %instIdx.i.i.1.i.i.i, %64 ], [ %42, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %42, %54 ], [ %42, %47 ], [ %42, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %instIdx.i.i.1.i.i.i, %._crit_edge.10.i.i.i ], [ %42, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %51 = phi i32 [ %43, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %v353.i.i.i, %64 ], [ %43, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %43, %54 ], [ %43, %47 ], [ %43, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %v353.i.i.i, %._crit_edge.10.i.i.i ], [ %43, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %52 = phi float [ %44, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %.0173.RayTCurrent.i.i.i, %64 ], [ %44, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %44, %54 ], [ %44, %47 ], [ %44, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %.0173.RayTCurrent.i.i.i, %._crit_edge.10.i.i.i ], [ %44, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %.3.i0.i.i.i = phi float [ %.1.i0.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %.2.i0.i.i.i, %64 ], [ %.1.i0.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %.1.i0.i.i.i, %54 ], [ %.1.i0.i.i.i, %47 ], [ %.2.i0.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %.2.i0.i.i.i, %._crit_edge.10.i.i.i ], [ %.1.i0.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %.3.i1.i.i.i = phi float [ %.1.i1.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %.2.i1.i.i.i, %64 ], [ %.1.i1.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %.1.i1.i.i.i, %54 ], [ %.1.i1.i.i.i, %47 ], [ %.2.i1.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %.2.i1.i.i.i, %._crit_edge.10.i.i.i ], [ %.1.i1.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %currentRayData.i.i.0.2.i0.i.i.i = phi float [ %FMad292.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %currentRayData.i.i.0.1.i0.i.i.i, %64 ], [ %currentRayData.i.i.0.1.i0.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %currentRayData.i.i.0.1.i0.i.i.i, %54 ], [ %currentRayData.i.i.0.1.i0.i.i.i, %47 ], [ %currentRayData.i.i.0.1.i0.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %currentRayData.i.i.0.1.i0.i.i.i, %._crit_edge.10.i.i.i ], [ %currentRayData.i.i.0.1.i0.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %currentRayData.i.i.0.2.i1.i.i.i = phi float [ %FMad289.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %currentRayData.i.i.0.1.i1.i.i.i, %64 ], [ %currentRayData.i.i.0.1.i1.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %currentRayData.i.i.0.1.i1.i.i.i, %54 ], [ %currentRayData.i.i.0.1.i1.i.i.i, %47 ], [ %currentRayData.i.i.0.1.i1.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %currentRayData.i.i.0.1.i1.i.i.i, %._crit_edge.10.i.i.i ], [ %currentRayData.i.i.0.1.i1.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %currentRayData.i.i.0.2.i2.i.i.i = phi float [ %FMad286.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %currentRayData.i.i.0.1.i2.i.i.i, %64 ], [ %currentRayData.i.i.0.1.i2.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %currentRayData.i.i.0.1.i2.i.i.i, %54 ], [ %currentRayData.i.i.0.1.i2.i.i.i, %47 ], [ %currentRayData.i.i.0.1.i2.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %currentRayData.i.i.0.1.i2.i.i.i, %._crit_edge.10.i.i.i ], [ %currentRayData.i.i.0.1.i2.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %currentRayData.i.i.1.2.i0.i.i.i = phi float [ %FMad283.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %currentRayData.i.i.1.1.i0.i.i.i, %64 ], [ %currentRayData.i.i.1.1.i0.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %currentRayData.i.i.1.1.i0.i.i.i, %54 ], [ %currentRayData.i.i.1.1.i0.i.i.i, %47 ], [ %currentRayData.i.i.1.1.i0.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %currentRayData.i.i.1.1.i0.i.i.i, %._crit_edge.10.i.i.i ], [ %currentRayData.i.i.1.1.i0.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %currentRayData.i.i.1.2.i1.i.i.i = phi float [ %FMad280.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %currentRayData.i.i.1.1.i1.i.i.i, %64 ], [ %currentRayData.i.i.1.1.i1.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %currentRayData.i.i.1.1.i1.i.i.i, %54 ], [ %currentRayData.i.i.1.1.i1.i.i.i, %47 ], [ %currentRayData.i.i.1.1.i1.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %currentRayData.i.i.1.1.i1.i.i.i, %._crit_edge.10.i.i.i ], [ %currentRayData.i.i.1.1.i1.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %currentRayData.i.i.1.2.i2.i.i.i = phi float [ %FMad.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %currentRayData.i.i.1.1.i2.i.i.i, %64 ], [ %currentRayData.i.i.1.1.i2.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %currentRayData.i.i.1.1.i2.i.i.i, %54 ], [ %currentRayData.i.i.1.1.i2.i.i.i, %47 ], [ %currentRayData.i.i.1.1.i2.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %currentRayData.i.i.1.1.i2.i.i.i, %._crit_edge.10.i.i.i ], [ %currentRayData.i.i.1.1.i2.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %currentRayData.i.i.2.2.i0.i.i.i = phi float [ %.i0340.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %currentRayData.i.i.2.1.i0.i.i.i, %64 ], [ %currentRayData.i.i.2.1.i0.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %currentRayData.i.i.2.1.i0.i.i.i, %54 ], [ %currentRayData.i.i.2.1.i0.i.i.i, %47 ], [ %currentRayData.i.i.2.1.i0.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %currentRayData.i.i.2.1.i0.i.i.i, %._crit_edge.10.i.i.i ], [ %currentRayData.i.i.2.1.i0.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %currentRayData.i.i.2.2.i1.i.i.i = phi float [ %.i1341.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %currentRayData.i.i.2.1.i1.i.i.i, %64 ], [ %currentRayData.i.i.2.1.i1.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %currentRayData.i.i.2.1.i1.i.i.i, %54 ], [ %currentRayData.i.i.2.1.i1.i.i.i, %47 ], [ %currentRayData.i.i.2.1.i1.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %currentRayData.i.i.2.1.i1.i.i.i, %._crit_edge.10.i.i.i ], [ %currentRayData.i.i.2.1.i1.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %currentRayData.i.i.2.2.i2.i.i.i = phi float [ %.i2342.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %currentRayData.i.i.2.1.i2.i.i.i, %64 ], [ %currentRayData.i.i.2.1.i2.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %currentRayData.i.i.2.1.i2.i.i.i, %54 ], [ %currentRayData.i.i.2.1.i2.i.i.i, %47 ], [ %currentRayData.i.i.2.1.i2.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %currentRayData.i.i.2.1.i2.i.i.i, %._crit_edge.10.i.i.i ], [ %currentRayData.i.i.2.1.i2.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %currentRayData.i.i.3.2.i0.i.i.i = phi float [ %.i0343.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %currentRayData.i.i.3.1.i0.i.i.i, %64 ], [ %currentRayData.i.i.3.1.i0.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %currentRayData.i.i.3.1.i0.i.i.i, %54 ], [ %currentRayData.i.i.3.1.i0.i.i.i, %47 ], [ %currentRayData.i.i.3.1.i0.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %currentRayData.i.i.3.1.i0.i.i.i, %._crit_edge.10.i.i.i ], [ %currentRayData.i.i.3.1.i0.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %currentRayData.i.i.3.2.i1.i.i.i = phi float [ %.i1344.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %currentRayData.i.i.3.1.i1.i.i.i, %64 ], [ %currentRayData.i.i.3.1.i1.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %currentRayData.i.i.3.1.i1.i.i.i, %54 ], [ %currentRayData.i.i.3.1.i1.i.i.i, %47 ], [ %currentRayData.i.i.3.1.i1.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %currentRayData.i.i.3.1.i1.i.i.i, %._crit_edge.10.i.i.i ], [ %currentRayData.i.i.3.1.i1.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %currentRayData.i.i.3.2.i2.i.i.i = phi float [ %.i2345.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %currentRayData.i.i.3.1.i2.i.i.i, %64 ], [ %currentRayData.i.i.3.1.i2.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %currentRayData.i.i.3.1.i2.i.i.i, %54 ], [ %currentRayData.i.i.3.1.i2.i.i.i, %47 ], [ %currentRayData.i.i.3.1.i2.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %currentRayData.i.i.3.1.i2.i.i.i, %._crit_edge.10.i.i.i ], [ %currentRayData.i.i.3.1.i2.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %currentRayData.i.i.4.2.i0.i.i.i = phi float [ %v217.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %currentRayData.i.i.4.1.i0.i.i.i, %64 ], [ %currentRayData.i.i.4.1.i0.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %currentRayData.i.i.4.1.i0.i.i.i, %54 ], [ %currentRayData.i.i.4.1.i0.i.i.i, %47 ], [ %currentRayData.i.i.4.1.i0.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %currentRayData.i.i.4.1.i0.i.i.i, %._crit_edge.10.i.i.i ], [ %currentRayData.i.i.4.1.i0.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %currentRayData.i.i.4.2.i1.i.i.i = phi float [ %v220.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %currentRayData.i.i.4.1.i1.i.i.i, %64 ], [ %currentRayData.i.i.4.1.i1.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %currentRayData.i.i.4.1.i1.i.i.i, %54 ], [ %currentRayData.i.i.4.1.i1.i.i.i, %47 ], [ %currentRayData.i.i.4.1.i1.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %currentRayData.i.i.4.1.i1.i.i.i, %._crit_edge.10.i.i.i ], [ %currentRayData.i.i.4.1.i1.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %currentRayData.i.i.4.2.i2.i.i.i = phi float [ %v221.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %currentRayData.i.i.4.1.i2.i.i.i, %64 ], [ %currentRayData.i.i.4.1.i2.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %currentRayData.i.i.4.1.i2.i.i.i, %54 ], [ %currentRayData.i.i.4.1.i2.i.i.i, %47 ], [ %currentRayData.i.i.4.1.i2.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %currentRayData.i.i.4.1.i2.i.i.i, %._crit_edge.10.i.i.i ], [ %currentRayData.i.i.4.1.i2.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %currentRayData.i.i.5.2.i0.i.i.i = phi i32 [ %.5.0.i0.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %currentRayData.i.i.5.1.i0.i.i.i, %64 ], [ %currentRayData.i.i.5.1.i0.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %currentRayData.i.i.5.1.i0.i.i.i, %54 ], [ %currentRayData.i.i.5.1.i0.i.i.i, %47 ], [ %currentRayData.i.i.5.1.i0.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %currentRayData.i.i.5.1.i0.i.i.i, %._crit_edge.10.i.i.i ], [ %currentRayData.i.i.5.1.i0.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %currentRayData.i.i.5.2.i1.i.i.i = phi i32 [ %.5.0.i1.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %currentRayData.i.i.5.1.i1.i.i.i, %64 ], [ %currentRayData.i.i.5.1.i1.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %currentRayData.i.i.5.1.i1.i.i.i, %54 ], [ %currentRayData.i.i.5.1.i1.i.i.i, %47 ], [ %currentRayData.i.i.5.1.i1.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %currentRayData.i.i.5.1.i1.i.i.i, %._crit_edge.10.i.i.i ], [ %currentRayData.i.i.5.1.i1.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %currentRayData.i.i.5.2.i2.i.i.i = phi i32 [ %.0172.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %currentRayData.i.i.5.1.i2.i.i.i, %64 ], [ %currentRayData.i.i.5.1.i2.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %currentRayData.i.i.5.1.i2.i.i.i, %54 ], [ %currentRayData.i.i.5.1.i2.i.i.i, %47 ], [ %currentRayData.i.i.5.1.i2.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %currentRayData.i.i.5.1.i2.i.i.i, %._crit_edge.10.i.i.i ], [ %currentRayData.i.i.5.1.i2.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %CurrentObjectToWorld.i.i167.2.i0.i.i.i = phi float [ %v185.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i0.i.i.i, %64 ], [ %CurrentObjectToWorld.i.i167.1.i0.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i0.i.i.i, %54 ], [ %CurrentObjectToWorld.i.i167.1.i0.i.i.i, %47 ], [ %CurrentObjectToWorld.i.i167.1.i0.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i0.i.i.i, %._crit_edge.10.i.i.i ], [ %CurrentObjectToWorld.i.i167.1.i0.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %CurrentObjectToWorld.i.i167.2.i1.i.i.i = phi float [ %v186.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i1.i.i.i, %64 ], [ %CurrentObjectToWorld.i.i167.1.i1.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i1.i.i.i, %54 ], [ %CurrentObjectToWorld.i.i167.1.i1.i.i.i, %47 ], [ %CurrentObjectToWorld.i.i167.1.i1.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i1.i.i.i, %._crit_edge.10.i.i.i ], [ %CurrentObjectToWorld.i.i167.1.i1.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %CurrentObjectToWorld.i.i167.2.i2.i.i.i = phi float [ %v187.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i2.i.i.i, %64 ], [ %CurrentObjectToWorld.i.i167.1.i2.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i2.i.i.i, %54 ], [ %CurrentObjectToWorld.i.i167.1.i2.i.i.i, %47 ], [ %CurrentObjectToWorld.i.i167.1.i2.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i2.i.i.i, %._crit_edge.10.i.i.i ], [ %CurrentObjectToWorld.i.i167.1.i2.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %CurrentObjectToWorld.i.i167.2.i3.i.i.i = phi float [ %v188.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i3.i.i.i, %64 ], [ %CurrentObjectToWorld.i.i167.1.i3.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i3.i.i.i, %54 ], [ %CurrentObjectToWorld.i.i167.1.i3.i.i.i, %47 ], [ %CurrentObjectToWorld.i.i167.1.i3.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i3.i.i.i, %._crit_edge.10.i.i.i ], [ %CurrentObjectToWorld.i.i167.1.i3.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %CurrentObjectToWorld.i.i167.2.i4.i.i.i = phi float [ %v189.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i4.i.i.i, %64 ], [ %CurrentObjectToWorld.i.i167.1.i4.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i4.i.i.i, %54 ], [ %CurrentObjectToWorld.i.i167.1.i4.i.i.i, %47 ], [ %CurrentObjectToWorld.i.i167.1.i4.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i4.i.i.i, %._crit_edge.10.i.i.i ], [ %CurrentObjectToWorld.i.i167.1.i4.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %CurrentObjectToWorld.i.i167.2.i5.i.i.i = phi float [ %v190.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i5.i.i.i, %64 ], [ %CurrentObjectToWorld.i.i167.1.i5.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i5.i.i.i, %54 ], [ %CurrentObjectToWorld.i.i167.1.i5.i.i.i, %47 ], [ %CurrentObjectToWorld.i.i167.1.i5.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i5.i.i.i, %._crit_edge.10.i.i.i ], [ %CurrentObjectToWorld.i.i167.1.i5.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %CurrentObjectToWorld.i.i167.2.i6.i.i.i = phi float [ %v191.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i6.i.i.i, %64 ], [ %CurrentObjectToWorld.i.i167.1.i6.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i6.i.i.i, %54 ], [ %CurrentObjectToWorld.i.i167.1.i6.i.i.i, %47 ], [ %CurrentObjectToWorld.i.i167.1.i6.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i6.i.i.i, %._crit_edge.10.i.i.i ], [ %CurrentObjectToWorld.i.i167.1.i6.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %CurrentObjectToWorld.i.i167.2.i7.i.i.i = phi float [ %v192.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i7.i.i.i, %64 ], [ %CurrentObjectToWorld.i.i167.1.i7.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i7.i.i.i, %54 ], [ %CurrentObjectToWorld.i.i167.1.i7.i.i.i, %47 ], [ %CurrentObjectToWorld.i.i167.1.i7.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i7.i.i.i, %._crit_edge.10.i.i.i ], [ %CurrentObjectToWorld.i.i167.1.i7.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %CurrentObjectToWorld.i.i167.2.i8.i.i.i = phi float [ %v193.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i8.i.i.i, %64 ], [ %CurrentObjectToWorld.i.i167.1.i8.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i8.i.i.i, %54 ], [ %CurrentObjectToWorld.i.i167.1.i8.i.i.i, %47 ], [ %CurrentObjectToWorld.i.i167.1.i8.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i8.i.i.i, %._crit_edge.10.i.i.i ], [ %CurrentObjectToWorld.i.i167.1.i8.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %CurrentObjectToWorld.i.i167.2.i9.i.i.i = phi float [ %v194.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i9.i.i.i, %64 ], [ %CurrentObjectToWorld.i.i167.1.i9.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i9.i.i.i, %54 ], [ %CurrentObjectToWorld.i.i167.1.i9.i.i.i, %47 ], [ %CurrentObjectToWorld.i.i167.1.i9.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i9.i.i.i, %._crit_edge.10.i.i.i ], [ %CurrentObjectToWorld.i.i167.1.i9.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %CurrentObjectToWorld.i.i167.2.i10.i.i.i = phi float [ %v195.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i10.i.i.i, %64 ], [ %CurrentObjectToWorld.i.i167.1.i10.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i10.i.i.i, %54 ], [ %CurrentObjectToWorld.i.i167.1.i10.i.i.i, %47 ], [ %CurrentObjectToWorld.i.i167.1.i10.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i10.i.i.i, %._crit_edge.10.i.i.i ], [ %CurrentObjectToWorld.i.i167.1.i10.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %CurrentObjectToWorld.i.i167.2.i11.i.i.i = phi float [ %v196.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i11.i.i.i, %64 ], [ %CurrentObjectToWorld.i.i167.1.i11.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i11.i.i.i, %54 ], [ %CurrentObjectToWorld.i.i167.1.i11.i.i.i, %47 ], [ %CurrentObjectToWorld.i.i167.1.i11.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %CurrentObjectToWorld.i.i167.1.i11.i.i.i, %._crit_edge.10.i.i.i ], [ %CurrentObjectToWorld.i.i167.1.i11.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %CurrentWorldToObject.i.i168.2.i0.i.i.i = phi float [ %v168.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i0.i.i.i, %64 ], [ %CurrentWorldToObject.i.i168.1.i0.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i0.i.i.i, %54 ], [ %CurrentWorldToObject.i.i168.1.i0.i.i.i, %47 ], [ %CurrentWorldToObject.i.i168.1.i0.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i0.i.i.i, %._crit_edge.10.i.i.i ], [ %CurrentWorldToObject.i.i168.1.i0.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %CurrentWorldToObject.i.i168.2.i1.i.i.i = phi float [ %v169.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i1.i.i.i, %64 ], [ %CurrentWorldToObject.i.i168.1.i1.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i1.i.i.i, %54 ], [ %CurrentWorldToObject.i.i168.1.i1.i.i.i, %47 ], [ %CurrentWorldToObject.i.i168.1.i1.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i1.i.i.i, %._crit_edge.10.i.i.i ], [ %CurrentWorldToObject.i.i168.1.i1.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %CurrentWorldToObject.i.i168.2.i2.i.i.i = phi float [ %v170.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i2.i.i.i, %64 ], [ %CurrentWorldToObject.i.i168.1.i2.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i2.i.i.i, %54 ], [ %CurrentWorldToObject.i.i168.1.i2.i.i.i, %47 ], [ %CurrentWorldToObject.i.i168.1.i2.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i2.i.i.i, %._crit_edge.10.i.i.i ], [ %CurrentWorldToObject.i.i168.1.i2.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %CurrentWorldToObject.i.i168.2.i3.i.i.i = phi float [ %v171.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i3.i.i.i, %64 ], [ %CurrentWorldToObject.i.i168.1.i3.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i3.i.i.i, %54 ], [ %CurrentWorldToObject.i.i168.1.i3.i.i.i, %47 ], [ %CurrentWorldToObject.i.i168.1.i3.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i3.i.i.i, %._crit_edge.10.i.i.i ], [ %CurrentWorldToObject.i.i168.1.i3.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %CurrentWorldToObject.i.i168.2.i4.i.i.i = phi float [ %v172.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i4.i.i.i, %64 ], [ %CurrentWorldToObject.i.i168.1.i4.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i4.i.i.i, %54 ], [ %CurrentWorldToObject.i.i168.1.i4.i.i.i, %47 ], [ %CurrentWorldToObject.i.i168.1.i4.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i4.i.i.i, %._crit_edge.10.i.i.i ], [ %CurrentWorldToObject.i.i168.1.i4.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %CurrentWorldToObject.i.i168.2.i5.i.i.i = phi float [ %v173.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i5.i.i.i, %64 ], [ %CurrentWorldToObject.i.i168.1.i5.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i5.i.i.i, %54 ], [ %CurrentWorldToObject.i.i168.1.i5.i.i.i, %47 ], [ %CurrentWorldToObject.i.i168.1.i5.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i5.i.i.i, %._crit_edge.10.i.i.i ], [ %CurrentWorldToObject.i.i168.1.i5.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %CurrentWorldToObject.i.i168.2.i6.i.i.i = phi float [ %v174.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i6.i.i.i, %64 ], [ %CurrentWorldToObject.i.i168.1.i6.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i6.i.i.i, %54 ], [ %CurrentWorldToObject.i.i168.1.i6.i.i.i, %47 ], [ %CurrentWorldToObject.i.i168.1.i6.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i6.i.i.i, %._crit_edge.10.i.i.i ], [ %CurrentWorldToObject.i.i168.1.i6.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %CurrentWorldToObject.i.i168.2.i7.i.i.i = phi float [ %v175.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i7.i.i.i, %64 ], [ %CurrentWorldToObject.i.i168.1.i7.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i7.i.i.i, %54 ], [ %CurrentWorldToObject.i.i168.1.i7.i.i.i, %47 ], [ %CurrentWorldToObject.i.i168.1.i7.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i7.i.i.i, %._crit_edge.10.i.i.i ], [ %CurrentWorldToObject.i.i168.1.i7.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %CurrentWorldToObject.i.i168.2.i8.i.i.i = phi float [ %v176.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i8.i.i.i, %64 ], [ %CurrentWorldToObject.i.i168.1.i8.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i8.i.i.i, %54 ], [ %CurrentWorldToObject.i.i168.1.i8.i.i.i, %47 ], [ %CurrentWorldToObject.i.i168.1.i8.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i8.i.i.i, %._crit_edge.10.i.i.i ], [ %CurrentWorldToObject.i.i168.1.i8.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %CurrentWorldToObject.i.i168.2.i9.i.i.i = phi float [ %v177.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i9.i.i.i, %64 ], [ %CurrentWorldToObject.i.i168.1.i9.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i9.i.i.i, %54 ], [ %CurrentWorldToObject.i.i168.1.i9.i.i.i, %47 ], [ %CurrentWorldToObject.i.i168.1.i9.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i9.i.i.i, %._crit_edge.10.i.i.i ], [ %CurrentWorldToObject.i.i168.1.i9.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %CurrentWorldToObject.i.i168.2.i10.i.i.i = phi float [ %v178.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i10.i.i.i, %64 ], [ %CurrentWorldToObject.i.i168.1.i10.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i10.i.i.i, %54 ], [ %CurrentWorldToObject.i.i168.1.i10.i.i.i, %47 ], [ %CurrentWorldToObject.i.i168.1.i10.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i10.i.i.i, %._crit_edge.10.i.i.i ], [ %CurrentWorldToObject.i.i168.1.i10.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %CurrentWorldToObject.i.i168.2.i11.i.i.i = phi float [ %v179.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i11.i.i.i, %64 ], [ %CurrentWorldToObject.i.i168.1.i11.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i11.i.i.i, %54 ], [ %CurrentWorldToObject.i.i168.1.i11.i.i.i, %47 ], [ %CurrentWorldToObject.i.i168.1.i11.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %CurrentWorldToObject.i.i168.1.i11.i.i.i, %._crit_edge.10.i.i.i ], [ %CurrentWorldToObject.i.i168.1.i11.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %ResetMatrices.i.i.3.i.i.i = phi i32 [ %ResetMatrices.i.i.1.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ 0, %64 ], [ %ResetMatrices.i.i.1.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %ResetMatrices.i.i.1.i.i.i, %54 ], [ %ResetMatrices.i.i.1.i.i.i, %47 ], [ %ResetMatrices.i.i.1.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ 0, %._crit_edge.10.i.i.i ], [ %ResetMatrices.i.i.1.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %currentBVHIndex.i.i.2.i.i.i = phi i32 [ 1, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %currentBVHIndex.i.i.1.i.i.i, %64 ], [ %currentBVHIndex.i.i.1.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %currentBVHIndex.i.i.1.i.i.i, %54 ], [ %currentBVHIndex.i.i.1.i.i.i, %47 ], [ %currentBVHIndex.i.i.1.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %currentBVHIndex.i.i.1.i.i.i, %._crit_edge.10.i.i.i ], [ 0, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %currentGpuVA.i.i.2.i0.i.i.i = phi i32 [ %v148.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %currentGpuVA.i.i.1.i0.i.i.i, %64 ], [ %currentGpuVA.i.i.1.i0.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %currentGpuVA.i.i.1.i0.i.i.i, %54 ], [ %currentGpuVA.i.i.1.i0.i.i.i, %47 ], [ %currentGpuVA.i.i.1.i0.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %currentGpuVA.i.i.1.i0.i.i.i, %._crit_edge.10.i.i.i ], [ %currentGpuVA.i.i.1.i0.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %currentGpuVA.i.i.2.i1.i.i.i = phi i32 [ %v149.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %currentGpuVA.i.i.1.i1.i.i.i, %64 ], [ %currentGpuVA.i.i.1.i1.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %currentGpuVA.i.i.1.i1.i.i.i, %54 ], [ %currentGpuVA.i.i.1.i1.i.i.i, %47 ], [ %currentGpuVA.i.i.1.i1.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %currentGpuVA.i.i.1.i1.i.i.i, %._crit_edge.10.i.i.i ], [ %currentGpuVA.i.i.1.i1.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %instIdx.i.i.2.i.i.i = phi i32 [ %v123.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %instIdx.i.i.1.i.i.i, %64 ], [ %instIdx.i.i.1.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %instIdx.i.i.1.i.i.i, %54 ], [ %instIdx.i.i.1.i.i.i, %47 ], [ %instIdx.i.i.1.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %instIdx.i.i.1.i.i.i, %._crit_edge.10.i.i.i ], [ %v123.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %instFlags.i.i.2.i.i.i = phi i32 [ %v197.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %instFlags.i.i.1.i.i.i, %64 ], [ %instFlags.i.i.1.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %instFlags.i.i.1.i.i.i, %54 ], [ %instFlags.i.i.1.i.i.i, %47 ], [ %instFlags.i.i.1.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %instFlags.i.i.1.i.i.i, %._crit_edge.10.i.i.i ], [ %instFlags.i.i.1.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %instOffset.i.i.2.i.i.i = phi i32 [ %v180.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %instOffset.i.i.1.i.i.i, %64 ], [ %instOffset.i.i.1.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %instOffset.i.i.1.i.i.i, %54 ], [ %instOffset.i.i.1.i.i.i, %47 ], [ %instOffset.i.i.1.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %instOffset.i.i.1.i.i.i, %._crit_edge.10.i.i.i ], [ %v180.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %instId.i.i.2.i.i.i = phi i32 [ %v181.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %instId.i.i.1.i.i.i, %64 ], [ %instId.i.i.1.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %instId.i.i.1.i.i.i, %54 ], [ %instId.i.i.1.i.i.i, %47 ], [ %instId.i.i.1.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %instId.i.i.1.i.i.i, %._crit_edge.10.i.i.i ], [ %v181.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %stackPointer.i.i.3.i.i.i = phi i32 [ %stackPointer.i.i.2.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %v103.i.i.i, %64 ], [ %v414.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %stackPointer.i.i.2.i.i.i, %54 ], [ %v103.i.i.i, %47 ], [ %v103.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %v103.i.i.i, %._crit_edge.10.i.i.i ], [ %v103.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %resultBary.i.i.2.i0.i.i.i = phi float [ %resultBary.i.i.1.i0.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %.2.i0.resultBary.i.i.1.i0.i.i.i, %64 ], [ %resultBary.i.i.1.i0.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %resultBary.i.i.1.i0.i.i.i, %54 ], [ %resultBary.i.i.1.i0.i.i.i, %47 ], [ %.2.i0.resultBary.i.i.1.i0.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %.2.i0.resultBary.i.i.1.i0.i.i.i, %._crit_edge.10.i.i.i ], [ %resultBary.i.i.1.i0.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %resultBary.i.i.2.i1.i.i.i = phi float [ %resultBary.i.i.1.i1.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %.2.i1.resultBary.i.i.1.i1.i.i.i, %64 ], [ %resultBary.i.i.1.i1.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %resultBary.i.i.1.i1.i.i.i, %54 ], [ %resultBary.i.i.1.i1.i.i.i, %47 ], [ %.2.i1.resultBary.i.i.1.i1.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %.2.i1.resultBary.i.i.1.i1.i.i.i, %._crit_edge.10.i.i.i ], [ %resultBary.i.i.1.i1.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %resultTriId.i.i.2.i.i.i = phi i32 [ %resultTriId.i.i.1.i.i.i, %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i" ], [ %.resultTriId.i.i.1.i.i.i, %64 ], [ %resultTriId.i.i.1.i.i.i, %"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i" ], [ %resultTriId.i.i.1.i.i.i, %54 ], [ %resultTriId.i.i.1.i.i.i, %47 ], [ %.resultTriId.i.i.1.i.i.i, %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i" ], [ %.resultTriId.i.i.1.i.i.i, %._crit_edge.10.i.i.i ], [ %resultTriId.i.i.1.i.i.i, %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i" ]
  %v418.i.i.i = getelementptr inbounds [2 x i32], [2 x i32]* %nodesToProcess.i.i.i.i.i, i32 0, i32 %currentBVHIndex.i.i.2.i.i.i
  %v419.i.i.i = load i32, i32* %v418.i.i.i, align 4, !tbaa !49
  %v420.i.i.i = icmp eq i32 %v419.i.i.i, 0
  br i1 %v420.i.i.i, label %53, label %._crit_edge.2.i.i.i

; <label>:53                                      ; preds = %._crit_edge.1.i.i.i
  %v421.i.i.i = add i32 %currentBVHIndex.i.i.2.i.i.i, -1
  %v422.i.i.i = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %AccelerationStructureList_cbuffer, i32 0) #2
  %v423.i.i.i = extractvalue %dx.types.CBufRet.i32 %v422.i.i.i, 0
  %v424.i.i.i = extractvalue %dx.types.CBufRet.i32 %v422.i.i.i, 1
  %v425.i.i.i = load i32, i32* %v56.i.i.i, align 4, !tbaa !49
  %v426.i.i.i = icmp eq i32 %v425.i.i.i, 0
  br i1 %v426.i.i.i, label %._crit_edge.i.i.i, label %.lr.ph.i.i.i

; <label>:54                                      ; preds = %47
  %v417.i.i.i = select i1 %v406.i.i.i, i32 %v120.i.i.i, i32 %v369.i.i.i
  store i32 %v417.i.i.i, i32 addrspace(3)* %v106.i.i.i, align 4, !tbaa !49, !noalias !57
  store i32 %v102.i.i.i, i32* %v108.i.i.i, align 4, !tbaa !49
  br label %._crit_edge.1.i.i.i

"\01?StackPush2@@YAXAIAH_NIIII@Z.exit.i.i.i.i.i": ; preds = %46
  %v408.i.i.i = fcmp fast olt float %FMax.i.i.i, %FMax266.i.i.i
  %v409.i.i.i = select i1 %v408.i.i.i, i32 %v369.i.i.i, i32 %v120.i.i.i
  %v410.i.i.i = select i1 %v408.i.i.i, i32 %v120.i.i.i, i32 %v369.i.i.i
  %v411.i.i.i = shl i32 %stackPointer.i.i.2.i.i.i, 6
  %v412.i.i.i = add i32 %v411.i.i.i, %groupIndex.i
  store i32 %v409.i.i.i, i32 addrspace(3)* %v106.i.i.i, align 4, !tbaa !49, !noalias !60
  %v413.i.i.i = getelementptr [2048 x i32], [2048 x i32] addrspace(3)* @"\01?stack@@3PAIA", i32 0, i32 %v412.i.i.i
  store i32 %v410.i.i.i, i32 addrspace(3)* %v413.i.i.i, align 4, !tbaa !49, !noalias !60
  %v414.i.i.i = add nsw i32 %stackPointer.i.i.2.i.i.i, 1
  %v415.i.i.i = add i32 %v102.i.i.i, 1
  store i32 %v415.i.i.i, i32* %v108.i.i.i, align 4, !tbaa !49
  br label %._crit_edge.1.i.i.i

; <label>:55                                      ; preds = %._crit_edge.2.i.i.i
  %v122.i.i.i = icmp eq i32 %currentBVHIndex.i.i.1.i.i.i, 0
  br i1 %v122.i.i.i, label %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i", label %"\01?IsOpaque@@YA_N_NII@Z.exit.i.i.i.i.i"

"\01?IsOpaque@@YA_N_NII@Z.exit.i.i.i.i.i":        ; preds = %55
  %v222.i.i.i = and i32 %arg0.int.i.i.i, 64
  %v223.i.i.i = icmp eq i32 %v222.i.i.i, 0
  %v224.i.i.i = and i32 %v117.i.i.i, 16777215
  %v225.i.i.i = add i32 %currentGpuVA.i.i.1.i0.i.i.i, 4
  %v227.i.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %DescriptorHeapBufferTable_UAV_rawbuf, i32 %v225.i.i.i, i32 undef) #2
  %v228.i.i.i = extractvalue %dx.types.ResRet.i32 %v227.i.i.i, 0
  %v229.i.i.i = mul nuw nsw i32 %v224.i.i.i, 36
  %v230.i.i.i = add i32 %v229.i.i.i, %currentGpuVA.i.i.1.i0.i.i.i
  %v231.i.i.i = add i32 %v230.i.i.i, %v228.i.i.i
  %v232.i.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %DescriptorHeapBufferTable_UAV_rawbuf, i32 %v231.i.i.i, i32 undef) #2
  %v233.i.i.i = extractvalue %dx.types.ResRet.i32 %v232.i.i.i, 0
  %v234.i.i.i = extractvalue %dx.types.ResRet.i32 %v232.i.i.i, 1
  %v235.i.i.i = extractvalue %dx.types.ResRet.i32 %v232.i.i.i, 2
  %v236.i.i.i = extractvalue %dx.types.ResRet.i32 %v232.i.i.i, 3
  %.i0346.i.i.i = bitcast i32 %v233.i.i.i to float
  %.i1347.i.i.i = bitcast i32 %v234.i.i.i to float
  %.i2348.i.i.i = bitcast i32 %v235.i.i.i to float
  %.i3349.i.i.i = bitcast i32 %v236.i.i.i to float
  %v237.i.i.i = add i32 %v231.i.i.i, 16
  %v238.i.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %DescriptorHeapBufferTable_UAV_rawbuf, i32 %v237.i.i.i, i32 undef) #2
  %v239.i.i.i = extractvalue %dx.types.ResRet.i32 %v238.i.i.i, 0
  %v240.i.i.i = extractvalue %dx.types.ResRet.i32 %v238.i.i.i, 1
  %v241.i.i.i = extractvalue %dx.types.ResRet.i32 %v238.i.i.i, 2
  %v242.i.i.i = extractvalue %dx.types.ResRet.i32 %v238.i.i.i, 3
  %.i0350.i.i.i = bitcast i32 %v239.i.i.i to float
  %.i1351.i.i.i = bitcast i32 %v240.i.i.i to float
  %.i2352.i.i.i = bitcast i32 %v241.i.i.i to float
  %.i3353.i.i.i = bitcast i32 %v242.i.i.i to float
  %v243.i.i.i = add i32 %v231.i.i.i, 32
  %v244.i.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %DescriptorHeapBufferTable_UAV_rawbuf, i32 %v243.i.i.i, i32 undef) #2
  %v245.i.i.i = extractvalue %dx.types.ResRet.i32 %v244.i.i.i, 0
  %v246.i.i.i = bitcast i32 %v245.i.i.i to float
  %v247.i.i.i = and i32 %instFlags.i.i.1.i.i.i, 1
  %v248.i.i.i = icmp eq i32 %v247.i.i.i, 0
  %v249.i.i.i = shl i32 %instFlags.i.i.1.i.i.i, 3
  %v250.i.i.i = and i32 %v249.i.i.i, 16
  %v251.i.i.i = add nuw nsw i32 %v250.i.i.i, 16
  %v252.i.i.i = xor i32 %v250.i.i.i, 16
  %v253.i.i.i = add nuw nsw i32 %v252.i.i.i, 16
  %v254.i.i.i = and i32 %arg0.int.i.i.i, %v251.i.i.i
  %v255.i.i.i = icmp ne i32 %v254.i.i.i, 0
  %v256.i.i.i = and i1 %v248.i.i.i, %v255.i.i.i
  %v257.i.i.i = and i32 %arg0.int.i.i.i, %v253.i.i.i
  %v258.i.i.i = icmp ne i32 %v257.i.i.i, 0
  %v259.i.i.i = and i1 %v248.i.i.i, %v258.i.i.i
  %.i0354.i.i.i = fsub fast float %.i0346.i.i.i, %currentRayData.i.i.0.1.i0.i.i.i
  %.i1355.i.i.i = fsub fast float %.i1347.i.i.i, %currentRayData.i.i.0.1.i1.i.i.i
  %.i2356.i.i.i = fsub fast float %.i2348.i.i.i, %currentRayData.i.i.0.1.i2.i.i.i
  store float %.i0354.i.i.i, float* %v92.i.i.i, align 4
  store float %.i1355.i.i.i, float* %v93.i.i.i, align 4
  store float %.i2356.i.i.i, float* %v94.i.i.i, align 4
  %v260.i.i.i = getelementptr [3 x float], [3 x float]* %v21.i.51.i.i, i32 0, i32 %currentRayData.i.i.5.1.i0.i.i.i
  %v261.i.i.i = load float, float* %v260.i.i.i, align 4, !tbaa !42, !noalias !63
  %v262.i.i.i = getelementptr [3 x float], [3 x float]* %v21.i.51.i.i, i32 0, i32 %currentRayData.i.i.5.1.i1.i.i.i
  %v263.i.i.i = load float, float* %v262.i.i.i, align 4, !tbaa !42, !noalias !63
  %v264.i.i.i = getelementptr [3 x float], [3 x float]* %v21.i.51.i.i, i32 0, i32 %currentRayData.i.i.5.1.i2.i.i.i
  %v265.i.i.i = load float, float* %v264.i.i.i, align 4, !tbaa !42, !noalias !63
  %.i0357.i.i.i = fsub fast float %.i3349.i.i.i, %currentRayData.i.i.0.1.i0.i.i.i
  %.i1358.i.i.i = fsub fast float %.i0350.i.i.i, %currentRayData.i.i.0.1.i1.i.i.i
  %.i2359.i.i.i = fsub fast float %.i1351.i.i.i, %currentRayData.i.i.0.1.i2.i.i.i
  store float %.i0357.i.i.i, float* %v95.i.i.i, align 4
  store float %.i1358.i.i.i, float* %v96.i.i.i, align 4
  store float %.i2359.i.i.i, float* %v97.i.i.i, align 4
  %v266.i.i.i = getelementptr [3 x float], [3 x float]* %v19.i.49.i.i, i32 0, i32 %currentRayData.i.i.5.1.i0.i.i.i
  %v267.i.i.i = load float, float* %v266.i.i.i, align 4, !tbaa !42, !noalias !63
  %v268.i.i.i = getelementptr [3 x float], [3 x float]* %v19.i.49.i.i, i32 0, i32 %currentRayData.i.i.5.1.i1.i.i.i
  %v269.i.i.i = load float, float* %v268.i.i.i, align 4, !tbaa !42, !noalias !63
  %v270.i.i.i = getelementptr [3 x float], [3 x float]* %v19.i.49.i.i, i32 0, i32 %currentRayData.i.i.5.1.i2.i.i.i
  %v271.i.i.i = load float, float* %v270.i.i.i, align 4, !tbaa !42, !noalias !63
  %.i0360.i.i.i = fsub fast float %.i2352.i.i.i, %currentRayData.i.i.0.1.i0.i.i.i
  %.i1361.i.i.i = fsub fast float %.i3353.i.i.i, %currentRayData.i.i.0.1.i1.i.i.i
  %.i2362.i.i.i = fsub fast float %v246.i.i.i, %currentRayData.i.i.0.1.i2.i.i.i
  store float %.i0360.i.i.i, float* %v98.i.i.i, align 4
  store float %.i1361.i.i.i, float* %v99.i.i.i, align 4
  store float %.i2362.i.i.i, float* %v100.i.i.i, align 4
  %v272.i.i.i = getelementptr [3 x float], [3 x float]* %v20.i.50.i.i, i32 0, i32 %currentRayData.i.i.5.1.i0.i.i.i
  %v273.i.i.i = load float, float* %v272.i.i.i, align 4, !tbaa !42, !noalias !63
  %v274.i.i.i = getelementptr [3 x float], [3 x float]* %v20.i.50.i.i, i32 0, i32 %currentRayData.i.i.5.1.i1.i.i.i
  %v275.i.i.i = load float, float* %v274.i.i.i, align 4, !tbaa !42, !noalias !63
  %v276.i.i.i = getelementptr [3 x float], [3 x float]* %v20.i.50.i.i, i32 0, i32 %currentRayData.i.i.5.1.i2.i.i.i
  %v277.i.i.i = load float, float* %v276.i.i.i, align 4, !tbaa !42, !noalias !63
  %.i0363.i.i.i = fmul float %currentRayData.i.i.4.1.i0.i.i.i, %v265.i.i.i
  %.i1364.i.i.i = fmul float %currentRayData.i.i.4.1.i1.i.i.i, %v265.i.i.i
  %.i0366.i.i.i = fsub float %v261.i.i.i, %.i0363.i.i.i
  %.i1367.i.i.i = fsub float %v263.i.i.i, %.i1364.i.i.i
  %.i0368.i.i.i = fmul float %currentRayData.i.i.4.1.i0.i.i.i, %v271.i.i.i
  %.i1369.i.i.i = fmul float %currentRayData.i.i.4.1.i1.i.i.i, %v271.i.i.i
  %.i0371.i.i.i = fsub float %v267.i.i.i, %.i0368.i.i.i
  %.i1372.i.i.i = fsub float %v269.i.i.i, %.i1369.i.i.i
  %.i0373.i.i.i = fmul float %currentRayData.i.i.4.1.i0.i.i.i, %v277.i.i.i
  %.i1374.i.i.i = fmul float %currentRayData.i.i.4.1.i1.i.i.i, %v277.i.i.i
  %.i0376.i.i.i = fsub float %v273.i.i.i, %.i0373.i.i.i
  %.i1377.i.i.i = fsub float %v275.i.i.i, %.i1374.i.i.i
  %v278.i.i.i = fmul float %.i1372.i.i.i, %.i0376.i.i.i
  %v279.i.i.i = fmul float %.i0371.i.i.i, %.i1377.i.i.i
  %v280.i.i.i = fsub float %v278.i.i.i, %v279.i.i.i
  %v281.i.i.i = fmul float %.i0366.i.i.i, %.i1377.i.i.i
  %v282.i.i.i = fmul float %.i1367.i.i.i, %.i0376.i.i.i
  %v283.i.i.i = fsub float %v281.i.i.i, %v282.i.i.i
  %v284.i.i.i = fmul float %.i1367.i.i.i, %.i0371.i.i.i
  %v285.i.i.i = fmul float %.i0366.i.i.i, %.i1372.i.i.i
  %v286.i.i.i = fsub float %v284.i.i.i, %v285.i.i.i
  %v287.i.i.i = fadd fast float %v283.i.i.i, %v286.i.i.i
  %v288.i.i.i = fadd fast float %v287.i.i.i, %v280.i.i.i
  br i1 %v259.i.i.i, label %66, label %56

; <label>:56                                      ; preds = %"\01?IsOpaque@@YA_N_NII@Z.exit.i.i.i.i.i"
  br i1 %v256.i.i.i, label %65, label %57

; <label>:57                                      ; preds = %56
  %v299.i.i.i = fcmp fast olt float %v280.i.i.i, 0.000000e+00
  %v300.i.i.i = fcmp fast olt float %v283.i.i.i, 0.000000e+00
  %v301.i.i.i = or i1 %v299.i.i.i, %v300.i.i.i
  %v302.i.i.i = fcmp fast olt float %v286.i.i.i, 0.000000e+00
  %v303.i.i.i = or i1 %v302.i.i.i, %v301.i.i.i
  %v304.i.i.i = fcmp fast ogt float %v280.i.i.i, 0.000000e+00
  %v305.i.i.i = fcmp fast ogt float %v283.i.i.i, 0.000000e+00
  %v306.i.i.i = or i1 %v304.i.i.i, %v305.i.i.i
  %v307.i.i.i = fcmp fast ogt float %v286.i.i.i, 0.000000e+00
  %v308.i.i.i = or i1 %v307.i.i.i, %v306.i.i.i
  %v309.i.i.i = and i1 %v303.i.i.i, %v308.i.i.i
  %v310.i.i.i = fcmp fast oeq float %v288.i.i.i, 0.000000e+00
  %or.cond193.i.i.i = or i1 %v310.i.i.i, %v309.i.i.i
  br i1 %or.cond193.i.i.i, label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i", label %._crit_edge.3.i.i.i

._crit_edge.3.i.i.i:                              ; preds = %66, %65, %57
  %v311.i.i.i = fmul fast float %v280.i.i.i, %v265.i.i.i
  %v312.i.i.i = fmul fast float %v283.i.i.i, %v271.i.i.i
  %v313.i.i.i = fmul fast float %v286.i.i.i, %v277.i.i.i
  %tmp.i.i.i = fadd fast float %v312.i.i.i, %v313.i.i.i
  %tmp517.i.i.i = fadd fast float %tmp.i.i.i, %v311.i.i.i
  %tmp518.i.i.i = fmul fast float %tmp517.i.i.i, %currentRayData.i.i.4.1.i2.i.i.i
  br i1 %v259.i.i.i, label %61, label %58

; <label>:58                                      ; preds = %._crit_edge.3.i.i.i
  br i1 %v256.i.i.i, label %60, label %59

; <label>:59                                      ; preds = %58
  %v322.i.i.i = fcmp fast ogt float %v288.i.i.i, 0.000000e+00
  %v323.i.i.i = select i1 %v322.i.i.i, i32 1, i32 -1
  %v324.i.i.i = bitcast float %tmp518.i.i.i to i32
  %v325.i.i.i = xor i32 %v324.i.i.i, %v323.i.i.i
  %v326.i.i.i = uitofp i32 %v325.i.i.i to float
  %v327.i.i.i = bitcast float %v288.i.i.i to i32
  %v328.i.i.i = xor i32 %v323.i.i.i, %v327.i.i.i
  %v329.i.i.i = uitofp i32 %v328.i.i.i to float
  %v330.i.i.i = fmul fast float %v329.i.i.i, %44
  %v331.i.i.i = fcmp fast ogt float %v326.i.i.i, %v330.i.i.i
  br i1 %v331.i.i.i, label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i", label %._crit_edge.5.i.i.i

._crit_edge.5.i.i.i:                              ; preds = %61, %60, %59
  %v332.i.i.i = fdiv fast float 1.000000e+00, %v288.i.i.i
  %v333.i.i.i = fmul fast float %v332.i.i.i, %v283.i.i.i
  %v334.i.i.i = fmul fast float %v332.i.i.i, %v286.i.i.i
  %v335.i.i.i = fmul fast float %v332.i.i.i, %tmp518.i.i.i
  br label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i"

; <label>:60                                      ; preds = %58
  %v318.i.i.i = fcmp fast olt float %tmp518.i.i.i, 0.000000e+00
  %v319.i.i.i = fmul fast float %v288.i.i.i, %44
  %v320.i.i.i = fcmp fast ogt float %tmp518.i.i.i, %v319.i.i.i
  %v321.i.i.i = or i1 %v318.i.i.i, %v320.i.i.i
  br i1 %v321.i.i.i, label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i", label %._crit_edge.5.i.i.i

; <label>:61                                      ; preds = %._crit_edge.3.i.i.i
  %v314.i.i.i = fcmp fast ogt float %tmp518.i.i.i, 0.000000e+00
  %v315.i.i.i = fmul fast float %v288.i.i.i, %44
  %v316.i.i.i = fcmp fast olt float %tmp518.i.i.i, %v315.i.i.i
  %v317.i.i.i = or i1 %v314.i.i.i, %v316.i.i.i
  br i1 %v317.i.i.i, label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i", label %._crit_edge.5.i.i.i

"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i": ; preds = %66, %65, %61, %60, %59, %57, %._crit_edge.5.i.i.i
  %.2.i0.i.i.i = phi float [ %v333.i.i.i, %._crit_edge.5.i.i.i ], [ %.1.i0.i.i.i, %57 ], [ %.1.i0.i.i.i, %59 ], [ %.1.i0.i.i.i, %60 ], [ %.1.i0.i.i.i, %61 ], [ %.1.i0.i.i.i, %65 ], [ %.1.i0.i.i.i, %66 ]
  %.2.i1.i.i.i = phi float [ %v334.i.i.i, %._crit_edge.5.i.i.i ], [ %.1.i1.i.i.i, %57 ], [ %.1.i1.i.i.i, %59 ], [ %.1.i1.i.i.i, %60 ], [ %.1.i1.i.i.i, %61 ], [ %.1.i1.i.i.i, %65 ], [ %.1.i1.i.i.i, %66 ]
  %.0173.i.i.i = phi float [ %v335.i.i.i, %._crit_edge.5.i.i.i ], [ %44, %57 ], [ %44, %59 ], [ %44, %60 ], [ %44, %61 ], [ %44, %65 ], [ %44, %66 ]
  %v336.i.i.i = fcmp fast olt float %.0173.i.i.i, %44
  %v337.i.i.i = fcmp fast ogt float %.0173.i.i.i, %17
  %v338.i.i.i = and i1 %v336.i.i.i, %v337.i.i.i
  %..i.i.i = select i1 %v338.i.i.i, i1 true, i1 false
  %.resultTriId.i.i.1.i.i.i = select i1 %v338.i.i.i, i32 %v224.i.i.i, i32 %resultTriId.i.i.1.i.i.i
  %.0173.RayTCurrent.i.i.i = select i1 %v338.i.i.i, float %.0173.i.i.i, float %44
  %.2.i0.resultBary.i.i.1.i0.i.i.i = select i1 %v338.i.i.i, float %.2.i0.i.i.i, float %resultBary.i.i.1.i0.i.i.i
  %.2.i1.resultBary.i.i.1.i1.i.i.i = select i1 %v338.i.i.i, float %.2.i1.i.i.i, float %resultBary.i.i.1.i1.i.i.i
  %v339.i.i.i = and i1 %v223.i.i.i, %..i.i.i
  br i1 %v339.i.i.i, label %._crit_edge.10.i.i.i, label %._crit_edge.1.i.i.i

._crit_edge.10.i.i.i:                             ; preds = %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i"
  %v341.i.i.i = add i32 %currentGpuVA.i.i.1.i0.i.i.i, 8
  %v343.i.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %DescriptorHeapBufferTable_UAV_rawbuf, i32 %v341.i.i.i, i32 undef) #2
  %v344.i.i.i = extractvalue %dx.types.ResRet.i32 %v343.i.i.i, 0
  %v345.i.i.i = shl i32 %.resultTriId.i.i.1.i.i.i, 3
  %v346.i.i.i = add i32 %v345.i.i.i, %currentGpuVA.i.i.1.i0.i.i.i
  %v347.i.i.i = add i32 %v346.i.i.i, %v344.i.i.i
  %v348.i.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %DescriptorHeapBufferTable_UAV_rawbuf, i32 %v347.i.i.i, i32 undef) #2
  %v349.i.i.i = extractvalue %dx.types.ResRet.i32 %v348.i.i.i, 0
  %v351.i.i.i = mul i32 %v349.i.i.i, %arg3.int.i.i.i
  %v352.i.i.i = add i32 %instOffset.i.i.1.i.i.i, %arg2.int.i.i.i
  %v353.i.i.i = add i32 %v352.i.i.i, %v351.i.i.i
  %62 = bitcast float %.2.i0.resultBary.i.i.1.i0.i.i.i to i32
  %arrayidx.i.42.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %40
  store i32 %62, i32* %arrayidx.i.42.i.i.i
  %63 = bitcast float %.2.i1.resultBary.i.i.1.i1.i.i.i to i32
  %add.i.38.i.i.i = add nsw i32 %40, 1
  %arrayidx.i.40.i.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.38.i.i.i
  store i32 %63, i32* %arrayidx.i.40.i.i.i
  %v354.i.i.i = icmp eq i32 %ResetMatrices.i.i.1.i.i.i, 0
  %.i0110.CurrentWorldToObject.i.i168.1.i0.i.i.i = select i1 %v354.i.i.i, float %.i0110, float %CurrentWorldToObject.i.i168.1.i0.i.i.i
  %.i1111.CurrentWorldToObject.i.i168.1.i1.i.i.i = select i1 %v354.i.i.i, float %.i1111, float %CurrentWorldToObject.i.i168.1.i1.i.i.i
  %.i2112.CurrentWorldToObject.i.i168.1.i2.i.i.i = select i1 %v354.i.i.i, float %.i2112, float %CurrentWorldToObject.i.i168.1.i2.i.i.i
  %.i3113.CurrentWorldToObject.i.i168.1.i3.i.i.i = select i1 %v354.i.i.i, float %.i3113, float %CurrentWorldToObject.i.i168.1.i3.i.i.i
  %.i4114.CurrentWorldToObject.i.i168.1.i4.i.i.i = select i1 %v354.i.i.i, float %.i4114, float %CurrentWorldToObject.i.i168.1.i4.i.i.i
  %.i5115.CurrentWorldToObject.i.i168.1.i5.i.i.i = select i1 %v354.i.i.i, float %.i5115, float %CurrentWorldToObject.i.i168.1.i5.i.i.i
  %.i6116.CurrentWorldToObject.i.i168.1.i6.i.i.i = select i1 %v354.i.i.i, float %.i6116, float %CurrentWorldToObject.i.i168.1.i6.i.i.i
  %.i7117.CurrentWorldToObject.i.i168.1.i7.i.i.i = select i1 %v354.i.i.i, float %.i7117, float %CurrentWorldToObject.i.i168.1.i7.i.i.i
  %.i8118.CurrentWorldToObject.i.i168.1.i8.i.i.i = select i1 %v354.i.i.i, float %.i8118, float %CurrentWorldToObject.i.i168.1.i8.i.i.i
  %.i9119.CurrentWorldToObject.i.i168.1.i9.i.i.i = select i1 %v354.i.i.i, float %.i9119, float %CurrentWorldToObject.i.i168.1.i9.i.i.i
  %.i10120.CurrentWorldToObject.i.i168.1.i10.i.i.i = select i1 %v354.i.i.i, float %.i10120, float %CurrentWorldToObject.i.i168.1.i10.i.i.i
  %.i11121.CurrentWorldToObject.i.i168.1.i11.i.i.i = select i1 %v354.i.i.i, float %.i11121, float %CurrentWorldToObject.i.i168.1.i11.i.i.i
  %.i0122.CurrentObjectToWorld.i.i167.1.i0.i.i.i = select i1 %v354.i.i.i, float %.i0122, float %CurrentObjectToWorld.i.i167.1.i0.i.i.i
  %.i1123.CurrentObjectToWorld.i.i167.1.i1.i.i.i = select i1 %v354.i.i.i, float %.i1123, float %CurrentObjectToWorld.i.i167.1.i1.i.i.i
  %.i2124.CurrentObjectToWorld.i.i167.1.i2.i.i.i = select i1 %v354.i.i.i, float %.i2124, float %CurrentObjectToWorld.i.i167.1.i2.i.i.i
  %.i3125.CurrentObjectToWorld.i.i167.1.i3.i.i.i = select i1 %v354.i.i.i, float %.i3125, float %CurrentObjectToWorld.i.i167.1.i3.i.i.i
  %.i4126.CurrentObjectToWorld.i.i167.1.i4.i.i.i = select i1 %v354.i.i.i, float %.i4126, float %CurrentObjectToWorld.i.i167.1.i4.i.i.i
  %.i5127.CurrentObjectToWorld.i.i167.1.i5.i.i.i = select i1 %v354.i.i.i, float %.i5127, float %CurrentObjectToWorld.i.i167.1.i5.i.i.i
  %.i6128.CurrentObjectToWorld.i.i167.1.i6.i.i.i = select i1 %v354.i.i.i, float %.i6128, float %CurrentObjectToWorld.i.i167.1.i6.i.i.i
  %.i7129.CurrentObjectToWorld.i.i167.1.i7.i.i.i = select i1 %v354.i.i.i, float %.i7129, float %CurrentObjectToWorld.i.i167.1.i7.i.i.i
  %.i8130.CurrentObjectToWorld.i.i167.1.i8.i.i.i = select i1 %v354.i.i.i, float %.i8130, float %CurrentObjectToWorld.i.i167.1.i8.i.i.i
  %.i9131.CurrentObjectToWorld.i.i167.1.i9.i.i.i = select i1 %v354.i.i.i, float %.i9131, float %CurrentObjectToWorld.i.i167.1.i9.i.i.i
  %.i10132.CurrentObjectToWorld.i.i167.1.i10.i.i.i = select i1 %v354.i.i.i, float %.i10132, float %CurrentObjectToWorld.i.i167.1.i10.i.i.i
  %.i11133.CurrentObjectToWorld.i.i167.1.i11.i.i.i = select i1 %v354.i.i.i, float %.i11133, float %CurrentObjectToWorld.i.i167.1.i11.i.i.i
  %v367.i.i.i = and i32 %arg0.int.i.i.i, 4
  %v368.i.i.i = icmp eq i32 %v367.i.i.i, 0
  br i1 %v368.i.i.i, label %._crit_edge.1.i.i.i, label %64

; <label>:64                                      ; preds = %._crit_edge.10.i.i.i
  store i32 0, i32* %v91.i.i.i, align 4, !tbaa !49
  store i32 0, i32* %v56.i.i.i, align 4, !tbaa !49
  br label %._crit_edge.1.i.i.i

; <label>:65                                      ; preds = %56
  %v294.i.i.i = fcmp fast olt float %v280.i.i.i, 0.000000e+00
  %v295.i.i.i = fcmp fast olt float %v283.i.i.i, 0.000000e+00
  %v296.i.i.i = or i1 %v294.i.i.i, %v295.i.i.i
  %v297.i.i.i = fcmp fast olt float %v286.i.i.i, 0.000000e+00
  %v298.i.i.i = or i1 %v297.i.i.i, %v296.i.i.i
  %.old.i.i.i = fcmp fast oeq float %v288.i.i.i, 0.000000e+00
  %or.cond194.i.i.i = or i1 %v298.i.i.i, %.old.i.i.i
  br i1 %or.cond194.i.i.i, label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i", label %._crit_edge.3.i.i.i

; <label>:66                                      ; preds = %"\01?IsOpaque@@YA_N_NII@Z.exit.i.i.i.i.i"
  %v289.i.i.i = fcmp fast ogt float %v280.i.i.i, 0.000000e+00
  %v290.i.i.i = fcmp fast ogt float %v283.i.i.i, 0.000000e+00
  %v291.i.i.i = or i1 %v289.i.i.i, %v290.i.i.i
  %v292.i.i.i = fcmp fast ogt float %v286.i.i.i, 0.000000e+00
  %v293.i.i.i = or i1 %v292.i.i.i, %v291.i.i.i
  %.old.old.i.i.i = fcmp fast oeq float %v288.i.i.i, 0.000000e+00
  %or.cond195.i.i.i = or i1 %v293.i.i.i, %.old.old.i.i.i
  br i1 %or.cond195.i.i.i, label %"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z.exit.i.i.i.i.i.i", label %._crit_edge.3.i.i.i

"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i": ; preds = %55
  %v123.i.i.i = and i32 %v117.i.i.i, 2147483647
  %v124.i.i.i = mul i32 %v123.i.i.i, 112
  %v125.i.i.i = add i32 %v63.i.i.i, %v124.i.i.i
  %v127.i.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %DescriptorHeapBufferTable_UAV_rawbuf337, i32 %v125.i.i.i, i32 undef) #2
  %v128.i.i.i = extractvalue %dx.types.ResRet.i32 %v127.i.i.i, 0
  %v129.i.i.i = extractvalue %dx.types.ResRet.i32 %v127.i.i.i, 1
  %v130.i.i.i = extractvalue %dx.types.ResRet.i32 %v127.i.i.i, 2
  %v131.i.i.i = extractvalue %dx.types.ResRet.i32 %v127.i.i.i, 3
  %v132.i.i.i = add i32 %v125.i.i.i, 16
  %v133.i.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %DescriptorHeapBufferTable_UAV_rawbuf337, i32 %v132.i.i.i, i32 undef) #2
  %v134.i.i.i = extractvalue %dx.types.ResRet.i32 %v133.i.i.i, 0
  %v135.i.i.i = extractvalue %dx.types.ResRet.i32 %v133.i.i.i, 1
  %v136.i.i.i = extractvalue %dx.types.ResRet.i32 %v133.i.i.i, 2
  %v137.i.i.i = extractvalue %dx.types.ResRet.i32 %v133.i.i.i, 3
  %v138.i.i.i = add i32 %v125.i.i.i, 32
  %v139.i.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %DescriptorHeapBufferTable_UAV_rawbuf337, i32 %v138.i.i.i, i32 undef) #2
  %v140.i.i.i = extractvalue %dx.types.ResRet.i32 %v139.i.i.i, 0
  %v141.i.i.i = extractvalue %dx.types.ResRet.i32 %v139.i.i.i, 1
  %v142.i.i.i = extractvalue %dx.types.ResRet.i32 %v139.i.i.i, 2
  %v143.i.i.i = extractvalue %dx.types.ResRet.i32 %v139.i.i.i, 3
  %v144.i.i.i = add i32 %v125.i.i.i, 48
  %v145.i.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %DescriptorHeapBufferTable_UAV_rawbuf337, i32 %v144.i.i.i, i32 undef) #2
  %v146.i.i.i = extractvalue %dx.types.ResRet.i32 %v145.i.i.i, 0
  %v147.i.i.i = extractvalue %dx.types.ResRet.i32 %v145.i.i.i, 1
  %v148.i.i.i = extractvalue %dx.types.ResRet.i32 %v145.i.i.i, 2
  %v149.i.i.i = extractvalue %dx.types.ResRet.i32 %v145.i.i.i, 3
  %v150.i.i.i = add i32 %v125.i.i.i, 64
  %v151.i.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %DescriptorHeapBufferTable_UAV_rawbuf337, i32 %v150.i.i.i, i32 undef) #2
  %v152.i.i.i = extractvalue %dx.types.ResRet.i32 %v151.i.i.i, 0
  %v153.i.i.i = extractvalue %dx.types.ResRet.i32 %v151.i.i.i, 1
  %v154.i.i.i = extractvalue %dx.types.ResRet.i32 %v151.i.i.i, 2
  %v155.i.i.i = extractvalue %dx.types.ResRet.i32 %v151.i.i.i, 3
  %v156.i.i.i = add i32 %v125.i.i.i, 80
  %v157.i.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %DescriptorHeapBufferTable_UAV_rawbuf337, i32 %v156.i.i.i, i32 undef) #2
  %v158.i.i.i = extractvalue %dx.types.ResRet.i32 %v157.i.i.i, 0
  %v159.i.i.i = extractvalue %dx.types.ResRet.i32 %v157.i.i.i, 1
  %v160.i.i.i = extractvalue %dx.types.ResRet.i32 %v157.i.i.i, 2
  %v161.i.i.i = extractvalue %dx.types.ResRet.i32 %v157.i.i.i, 3
  %v162.i.i.i = add i32 %v125.i.i.i, 96
  %v163.i.i.i = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %DescriptorHeapBufferTable_UAV_rawbuf337, i32 %v162.i.i.i, i32 undef) #2
  %v164.i.i.i = extractvalue %dx.types.ResRet.i32 %v163.i.i.i, 0
  %v165.i.i.i = extractvalue %dx.types.ResRet.i32 %v163.i.i.i, 1
  %v166.i.i.i = extractvalue %dx.types.ResRet.i32 %v163.i.i.i, 2
  %v167.i.i.i = extractvalue %dx.types.ResRet.i32 %v163.i.i.i, 3
  %v168.i.i.i = bitcast i32 %v128.i.i.i to float
  %v169.i.i.i = bitcast i32 %v129.i.i.i to float
  %v170.i.i.i = bitcast i32 %v130.i.i.i to float
  %v171.i.i.i = bitcast i32 %v131.i.i.i to float
  %v172.i.i.i = bitcast i32 %v134.i.i.i to float
  %v173.i.i.i = bitcast i32 %v135.i.i.i to float
  %v174.i.i.i = bitcast i32 %v136.i.i.i to float
  %v175.i.i.i = bitcast i32 %v137.i.i.i to float
  %v176.i.i.i = bitcast i32 %v140.i.i.i to float
  %v177.i.i.i = bitcast i32 %v141.i.i.i to float
  %v178.i.i.i = bitcast i32 %v142.i.i.i to float
  %v179.i.i.i = bitcast i32 %v143.i.i.i to float
  %v180.i.i.i = and i32 %v147.i.i.i, 16777215
  %v181.i.i.i = and i32 %v146.i.i.i, 16777215
  %v182.i.i.i = lshr i32 %v146.i.i.i, 24
  %v183.i.i.i = and i32 %v182.i.i.i, %arg1.int.i.i.i
  %v184.i.i.i = icmp eq i32 %v183.i.i.i, 0
  %v185.i.i.i = bitcast i32 %v152.i.i.i to float
  %v186.i.i.i = bitcast i32 %v153.i.i.i to float
  %v187.i.i.i = bitcast i32 %v154.i.i.i to float
  %v188.i.i.i = bitcast i32 %v155.i.i.i to float
  %v189.i.i.i = bitcast i32 %v158.i.i.i to float
  %v190.i.i.i = bitcast i32 %v159.i.i.i to float
  %v191.i.i.i = bitcast i32 %v160.i.i.i to float
  %v192.i.i.i = bitcast i32 %v161.i.i.i to float
  %v193.i.i.i = bitcast i32 %v164.i.i.i to float
  %v194.i.i.i = bitcast i32 %v165.i.i.i to float
  %v195.i.i.i = bitcast i32 %v166.i.i.i to float
  %v196.i.i.i = bitcast i32 %v167.i.i.i to float
  br i1 %v184.i.i.i, label %._crit_edge.1.i.i.i, label %67

; <label>:67                                      ; preds = %"\01?LoadBVHMetadata@@YA?AUBVHMetadata@@URWByteAddressBuffer@@I@Z.exit.i.i189.i.i.i"
  store i32 0, i32 addrspace(3)* %v106.i.i.i, align 4, !tbaa !49, !noalias !71
  %v197.i.i.i = lshr i32 %v147.i.i.i, 24
  %v198.i.i.i = fmul fast float %18, %v168.i.i.i
  %FMad285.i.i.i = call float @dx.op.tertiary.f32(i32 46, float %19, float %v169.i.i.i, float %v198.i.i.i) #2
  %FMad284.i.i.i = call float @dx.op.tertiary.f32(i32 46, float %20, float %v170.i.i.i, float %FMad285.i.i.i) #2
  %FMad283.i.i.i = call float @dx.op.tertiary.f32(i32 46, float 0.000000e+00, float %v171.i.i.i, float %FMad284.i.i.i) #2
  %v199.i.i.i = fmul fast float %18, %v172.i.i.i
  %FMad282.i.i.i = call float @dx.op.tertiary.f32(i32 46, float %19, float %v173.i.i.i, float %v199.i.i.i) #2
  %FMad281.i.i.i = call float @dx.op.tertiary.f32(i32 46, float %20, float %v174.i.i.i, float %FMad282.i.i.i) #2
  %FMad280.i.i.i = call float @dx.op.tertiary.f32(i32 46, float 0.000000e+00, float %v175.i.i.i, float %FMad281.i.i.i) #2
  %v200.i.i.i = fmul fast float %18, %v176.i.i.i
  %FMad279.i.i.i = call float @dx.op.tertiary.f32(i32 46, float %19, float %v177.i.i.i, float %v200.i.i.i) #2
  %FMad278.i.i.i = call float @dx.op.tertiary.f32(i32 46, float %20, float %v178.i.i.i, float %FMad279.i.i.i) #2
  %FMad.i.i.i = call float @dx.op.tertiary.f32(i32 46, float 0.000000e+00, float %v179.i.i.i, float %FMad278.i.i.i) #2
  %v201.i.i.i = fmul fast float %14, %v168.i.i.i
  %FMad294.i.i.i = call float @dx.op.tertiary.f32(i32 46, float %15, float %v169.i.i.i, float %v201.i.i.i) #2
  %FMad293.i.i.i = call float @dx.op.tertiary.f32(i32 46, float %16, float %v170.i.i.i, float %FMad294.i.i.i) #2
  %FMad292.i.i.i = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %v171.i.i.i, float %FMad293.i.i.i) #2
  %v202.i.i.i = fmul fast float %14, %v172.i.i.i
  %FMad291.i.i.i = call float @dx.op.tertiary.f32(i32 46, float %15, float %v173.i.i.i, float %v202.i.i.i) #2
  %FMad290.i.i.i = call float @dx.op.tertiary.f32(i32 46, float %16, float %v174.i.i.i, float %FMad291.i.i.i) #2
  %FMad289.i.i.i = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %v175.i.i.i, float %FMad290.i.i.i) #2
  %v203.i.i.i = fmul fast float %14, %v176.i.i.i
  %FMad288.i.i.i = call float @dx.op.tertiary.f32(i32 46, float %15, float %v177.i.i.i, float %v203.i.i.i) #2
  %FMad287.i.i.i = call float @dx.op.tertiary.f32(i32 46, float %16, float %v178.i.i.i, float %FMad288.i.i.i) #2
  %FMad286.i.i.i = call float @dx.op.tertiary.f32(i32 46, float 1.000000e+00, float %v179.i.i.i, float %FMad287.i.i.i) #2
  store float %FMad283.i.i.i, float* %v88.i.i.i, align 4
  store float %FMad280.i.i.i, float* %v89.i.i.i, align 4
  store float %FMad.i.i.i, float* %v90.i.i.i, align 4
  %.i0340.i.i.i = fdiv fast float 1.000000e+00, %FMad283.i.i.i
  %.i1341.i.i.i = fdiv fast float 1.000000e+00, %FMad280.i.i.i
  %.i2342.i.i.i = fdiv fast float 1.000000e+00, %FMad.i.i.i
  %.i0343.i.i.i = fmul fast float %.i0340.i.i.i, %FMad292.i.i.i
  %.i1344.i.i.i = fmul fast float %.i1341.i.i.i, %FMad289.i.i.i
  %.i2345.i.i.i = fmul fast float %.i2342.i.i.i, %FMad286.i.i.i
  %FAbs.i.i.i = call float @dx.op.unary.f32(i32 6, float %FMad283.i.i.i) #2
  %FAbs221.i.i.i = call float @dx.op.unary.f32(i32 6, float %FMad280.i.i.i) #2
  %FAbs222.i.i.i = call float @dx.op.unary.f32(i32 6, float %FMad.i.i.i) #2
  %v204.i.i.i = fcmp fast ogt float %FAbs.i.i.i, %FAbs221.i.i.i
  %v205.i.i.i = fcmp fast ogt float %FAbs.i.i.i, %FAbs222.i.i.i
  %v206.i.i.i = and i1 %v204.i.i.i, %v205.i.i.i
  br i1 %v206.i.i.i, label %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i", label %68

; <label>:68                                      ; preds = %67
  %v207.i.i.i = fcmp fast ogt float %FAbs221.i.i.i, %FAbs222.i.i.i
  br i1 %v207.i.i.i, label %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i", label %69

; <label>:69                                      ; preds = %68
  br label %"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i"

"\01?GetIndexOfBiggestChannel@@YAHV?$vector@M$02@@@Z.exit.i.12.i.i.i.i.i": ; preds = %68, %67, %69
  %.0172.i.i.i = phi i32 [ 2, %69 ], [ 0, %67 ], [ 1, %68 ]
  %v208.i.i.i = add nuw nsw i32 %.0172.i.i.i, 1
  %v209.i.i.i = urem i32 %v208.i.i.i, 3
  %v210.i.i.i = add nuw nsw i32 %.0172.i.i.i, 2
  %v211.i.i.i = urem i32 %v210.i.i.i, 3
  %v212.i.i.i = getelementptr [3 x float], [3 x float]* %v22.i.52.i.i, i32 0, i32 %.0172.i.i.i
  %v213.i.i.i = load float, float* %v212.i.i.i, align 4, !tbaa !42, !noalias !74
  %v214.i.i.i = fcmp fast olt float %v213.i.i.i, 0.000000e+00
  %.5.0.i0.i.i.i = select i1 %v214.i.i.i, i32 %v211.i.i.i, i32 %v209.i.i.i
  %.5.0.i1.i.i.i = select i1 %v214.i.i.i, i32 %v209.i.i.i, i32 %v211.i.i.i
  %v215.i.i.i = getelementptr [3 x float], [3 x float]* %v22.i.52.i.i, i32 0, i32 %.5.0.i0.i.i.i
  %v216.i.i.i = load float, float* %v215.i.i.i, align 4, !tbaa !42, !noalias !74
  %v217.i.i.i = fdiv float %v216.i.i.i, %v213.i.i.i
  %v218.i.i.i = getelementptr [3 x float], [3 x float]* %v22.i.52.i.i, i32 0, i32 %.5.0.i1.i.i.i
  %v219.i.i.i = load float, float* %v218.i.i.i, align 4, !tbaa !42, !noalias !74
  %v220.i.i.i = fdiv float %v219.i.i.i, %v213.i.i.i
  %v221.i.i.i = fdiv float 1.000000e+00, %v213.i.i.i
  store i32 1, i32* %v91.i.i.i, align 4, !tbaa !49
  br label %._crit_edge.1.i.i.i

state_1005.Fallback_TraceRay.ss_1.i.i:            ; preds = %while.body.i
  %add.i.7.i.66.i.i = add nsw i32 %6, 4
  %arrayidx.i.9.i.68.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.7.i.66.i.i
  %v24.int.i.i.i = load i32, i32* %arrayidx.i.9.i.68.i.i
  %add.i.6.i.i.i = add nsw i32 %6, 24
  %add.i.i.72.i.i = add nsw i32 %add.i.6.i.i.i, -1
  %arrayidx.i.4.i.73.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.i.72.i.i
  %70 = load i32, i32* %arrayidx.i.4.i.73.i.i, align 4
  %add2.i.i.76.i.i = add nsw i32 %add.i.6.i.i.i, -2
  %arrayidx4.i.i.78.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add2.i.i.76.i.i
  %71 = load i32, i32* %arrayidx4.i.i.78.i.i, align 4
  %arrayidx.i.i.82.i.i = getelementptr inbounds [256 x i32], [256 x i32]* %theStack.i, i32 0, i32 %add.i.6.i.i.i
  %ret.stateId.i.83.i.i = load i32, i32* %arrayidx.i.i.82.i.i
  br label %while.cond.i

fb_Fallback_Scheduler.exit:                       ; preds = %0, %while.cond.i
  ret void
}

; Function Attrs: nounwind readonly
declare %dx.types.Handle @dx.op.createHandle(i32, i8, i32, i32, i1) #1

attributes #0 = { nounwind readnone }
attributes #1 = { nounwind readonly }
attributes #2 = { nounwind }

!llvm.ident = !{!0, !0, !0, !1}
!llvm.module.flags = !{!2, !3}
!dx.version = !{!4}
!dx.valver = !{!5}
!dx.shaderModel = !{!6}
!dx.resources = !{!7}
!dx.typeAnnotations = !{!19, !35}
!dx.entryPoints = !{!39}

!0 = !{!"clang version 3.7 (tags/RELEASE_370/final)"}
!1 = !{!"FallbackLayer"}
!2 = !{i32 2, !"Dwarf Version", i32 4}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{i32 1, i32 0}
!5 = !{i32 1, i32 2}
!6 = !{!"cs", i32 6, i32 0}
!7 = !{!8, !12, !16, null}
!8 = !{!9, !10, !11}
!9 = !{i32 0, %struct.ByteAddressBuffer* undef, !"HitGroupShaderTable", i32 214743647, i32 0, i32 1, i32 11, i32 0, null}
!10 = !{i32 1, %struct.ByteAddressBuffer* undef, !"RayGenShaderTable", i32 214743647, i32 2, i32 1, i32 11, i32 0, null}
!11 = !{i32 2, %struct.ByteAddressBuffer* undef, !"MissShaderTable", i32 214743647, i32 1, i32 1, i32 11, i32 0, null}
!12 = !{!13, !15}
!13 = !{i32 0, %"class.RWTexture2D<vector<float, 4> >"* undef, !"RenderTarget", i32 0, i32 0, i32 1, i32 2, i1 false, i1 false, i1 false, !14}
!14 = !{i32 0, i32 9}
!15 = !{i32 1, [0 x %struct.RWByteAddressBuffer]* undef, !"DescriptorHeapBufferTable", i32 214743648, i32 0, i32 -1, i32 11, i1 false, i1 false, i1 false, null}
!16 = !{!17, !18}
!17 = !{i32 0, %AccelerationStructureList* undef, !"AccelerationStructureList", i32 214743647, i32 1, i32 1, i32 8, null}
!18 = !{i32 1, %Constants* undef, !"Constants", i32 214743647, i32 0, i32 1, i32 32, null}
!19 = !{i32 0, %"class.RWTexture2D<vector<float, 4> >" undef, !20, %Constants undef, !22, %struct.ByteAddressBuffer undef, !31, %AccelerationStructureList undef, !33, %struct.RWByteAddressBuffer undef, !31, %struct.RaytracingAccelerationStructure undef, !31}
!20 = !{i32 16, !21}
!21 = !{i32 6, !"h", i32 3, i32 0, i32 7, i32 9}
!22 = !{i32 32, !23, !24, !25, !26, !27, !28, !29, !30}
!23 = !{i32 6, !"Constants0", i32 3, i32 0, i32 7, i32 4}
!24 = !{i32 6, !"Constants1", i32 3, i32 4, i32 7, i32 4}
!25 = !{i32 6, !"Constants2", i32 3, i32 8, i32 7, i32 4}
!26 = !{i32 6, !"Constants3", i32 3, i32 12, i32 7, i32 4}
!27 = !{i32 6, !"Constants4", i32 3, i32 16, i32 7, i32 4}
!28 = !{i32 6, !"Constants5", i32 3, i32 20, i32 7, i32 4}
!29 = !{i32 6, !"Constants6", i32 3, i32 24, i32 7, i32 4}
!30 = !{i32 6, !"Constants7", i32 3, i32 28, i32 7, i32 4}
!31 = !{i32 4, !32}
!32 = !{i32 6, !"h", i32 3, i32 0, i32 7, i32 4}
!33 = !{i32 8, !34}
!34 = !{i32 6, !"TopLevelAccelerationStructureGpuVA", i32 3, i32 0, i32 7, i32 5}
!35 = !{i32 1, void ()* @main, !36}
!36 = !{!37}
!37 = !{i32 0, !38, !38}
!38 = !{}
!39 = !{void ()* @main, !"main", null, !7, !40}
!40 = !{i32 0, i64 16, i32 4, !41}
!41 = !{i32 8, i32 8, i32 1}
!42 = !{!43, !43, i64 0}
!43 = !{!"float", !44, i64 0}
!44 = !{!"omnipotent char", !45, i64 0}
!45 = !{!"Simple C/C++ TBAA"}
!46 = !{!47}
!47 = distinct !{!47, !48, !"\01?GetRayData@@YA?AURayData@@V?$vector@M$02@@0@Z: %agg.result"}
!48 = distinct !{!48, !"\01?GetRayData@@YA?AURayData@@V?$vector@M$02@@0@Z"}
!49 = !{!50, !50, i64 0}
!50 = !{!"int", !44, i64 0}
!51 = !{!52}
!52 = distinct !{!52, !53, !"\01?StackPush@@YAXAIAHIII@Z: %stackTop"}
!53 = distinct !{!53, !"\01?StackPush@@YAXAIAHIII@Z"}
!54 = !{!55}
!55 = distinct !{!55, !56, !"\01?StackPop@@YAIAIAHAIAII@Z: %stackTop"}
!56 = distinct !{!56, !"\01?StackPop@@YAIAIAHAIAII@Z"}
!57 = !{!58}
!58 = distinct !{!58, !59, !"\01?StackPush@@YAXAIAHIII@Z: %stackTop"}
!59 = distinct !{!59, !"\01?StackPush@@YAXAIAHIII@Z"}
!60 = !{!61}
!61 = distinct !{!61, !62, !"\01?StackPush2@@YAXAIAH_NIIII@Z: %stackTop"}
!62 = distinct !{!62, !"\01?StackPush2@@YAXAIAH_NIIII@Z"}
!63 = !{!64, !66, !67, !69, !70}
!64 = distinct !{!64, !65, !"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z: %hitT"}
!65 = distinct !{!65, !"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z"}
!66 = distinct !{!66, !65, !"\01?RayTriangleIntersect@@YAXAIAMIAIAV?$vector@M$01@@V?$vector@M$02@@2V?$vector@H$02@@2222@Z: %bary"}
!67 = distinct !{!67, !68, !"\01?TestLeafNodeIntersections@@YA_NURWByteAddressBufferPointer@@V?$vector@I$01@@IV?$vector@M$02@@2V?$vector@H$02@@2AIAV?$vector@M$01@@AIAMAIAI@Z: %resultBary"}
!68 = distinct !{!68, !"\01?TestLeafNodeIntersections@@YA_NURWByteAddressBufferPointer@@V?$vector@I$01@@IV?$vector@M$02@@2V?$vector@H$02@@2AIAV?$vector@M$01@@AIAMAIAI@Z"}
!69 = distinct !{!69, !68, !"\01?TestLeafNodeIntersections@@YA_NURWByteAddressBufferPointer@@V?$vector@I$01@@IV?$vector@M$02@@2V?$vector@H$02@@2AIAV?$vector@M$01@@AIAMAIAI@Z: %resultT"}
!70 = distinct !{!70, !68, !"\01?TestLeafNodeIntersections@@YA_NURWByteAddressBufferPointer@@V?$vector@I$01@@IV?$vector@M$02@@2V?$vector@H$02@@2AIAV?$vector@M$01@@AIAMAIAI@Z: %resultTriId"}
!71 = !{!72}
!72 = distinct !{!72, !73, !"\01?StackPush@@YAXAIAHIII@Z: %stackTop"}
!73 = distinct !{!73, !"\01?StackPush@@YAXAIAHIII@Z"}
!74 = !{!75}
!75 = distinct !{!75, !76, !"\01?GetRayData@@YA?AURayData@@V?$vector@M$02@@0@Z: %agg.result"}
!76 = distinct !{!76, !"\01?GetRayData@@YA?AURayData@@V?$vector@M$02@@0@Z"}
