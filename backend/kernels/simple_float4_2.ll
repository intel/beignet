; ModuleID = 'simple_float4_2.o'
target datalayout = "e-p:32:32-i64:64:64-f64:64:64-n1:8:16:32:64"
target triple = "ptx32--"

define ptx_kernel void @simple_float4(<4 x float>* nocapture %dst, <4 x float>* nocapture %src) nounwind noinline {
get_global_id.exit10:
  %call.i = tail call ptx_device i32 @__gen_ocl_get_global_id0() nounwind readnone
  %arrayidx = getelementptr inbounds <4 x float>* %src, i32 %call.i
  %0 = load <4 x float>* %arrayidx, align 16, !tbaa !1
  %mul = fmul <4 x float> %0, %0
  %arrayidx4 = getelementptr inbounds <4 x float>* %dst, i32 %call.i
  store <4 x float> %mul, <4 x float>* %arrayidx4, align 16, !tbaa !1
  ret void
}

declare ptx_device i32 @__gen_ocl_get_global_id0() nounwind readnone

!opencl.kernels = !{!0}

!0 = metadata !{void (<4 x float>*, <4 x float>*)* @simple_float4}
!1 = metadata !{metadata !"omnipotent char", metadata !2}
!2 = metadata !{metadata !"Simple C/C++ TBAA", null}
