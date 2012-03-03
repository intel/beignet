; ModuleID = 'simple_float4_3.o'
target datalayout = "e-p:32:32-i64:64:64-f64:64:64-n1:8:16:32:64"
target triple = "ptx32--"

define ptx_kernel void @simple_float4(<4 x float>* nocapture %dst, <4 x float>* nocapture %src, i1 %b) nounwind noinline {
get_global_id.exit16:
  %call.i = tail call ptx_device i32 @__gen_ocl_get_global_id0() nounwind readnone
  %arrayidx = getelementptr inbounds <4 x float>* %src, i32 %call.i
  %0 = load <4 x float>* %arrayidx, align 16, !tbaa !1
  %call3.i = tail call ptx_device i32 @__gen_ocl_get_global_id1() nounwind readnone
  %arrayidx2 = getelementptr inbounds <4 x float>* %src, i32 %call3.i
  %1 = load <4 x float>* %arrayidx2, align 16, !tbaa !1
  %x.y.i = select i1 %b, <4 x float> %0, <4 x float> %1
  %arrayidx5 = getelementptr inbounds <4 x float>* %dst, i32 %call.i
  store <4 x float> %x.y.i, <4 x float>* %arrayidx5, align 16, !tbaa !1
  %arrayidx6 = getelementptr inbounds <4 x float>* %src, i32 2
  %2 = load <4 x float>* %arrayidx6, align 16
  %3 = extractelement <4 x float> %2, i32 0
  %vecinit = insertelement <4 x float> undef, float %3, i32 0
  %vecinit7 = insertelement <4 x float> %vecinit, float 1.000000e+00, i32 1
  %vecinit8 = insertelement <4 x float> %vecinit7, float 2.000000e+00, i32 2
  %vecinit9 = insertelement <4 x float> %vecinit8, float 3.000000e+00, i32 3
  %add = fadd <4 x float> %x.y.i, %vecinit9
  store <4 x float> %add, <4 x float>* %arrayidx5, align 16, !tbaa !1
  ret void
}

declare ptx_device i32 @__gen_ocl_get_global_id0() nounwind readnone

declare ptx_device i32 @__gen_ocl_get_global_id1() nounwind readnone

!opencl.kernels = !{!0}

!0 = metadata !{void (<4 x float>*, <4 x float>*, i1)* @simple_float4}
!1 = metadata !{metadata !"omnipotent char", metadata !2}
!2 = metadata !{metadata !"Simple C/C++ TBAA", null}
