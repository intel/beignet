; ModuleID = 'vector_constant.cl.o'
target datalayout = "e-p:32:32-i64:64:64-f64:64:64-n1:8:16:32:64"
target triple = "ptx32--"

define ptx_device <2 x float> @_Z3madDv2_fS_S_(<2 x float> %a, <2 x float> %b, <2 x float> %c) nounwind readnone {
entry:
  %0 = extractelement <2 x float> %a, i32 0
  %1 = extractelement <2 x float> %b, i32 0
  %2 = extractelement <2 x float> %c, i32 0
  %call = tail call ptx_device float @_Z3madfff(float %0, float %1, float %2) nounwind readnone
  %vecinit = insertelement <2 x float> undef, float %call, i32 0
  %3 = extractelement <2 x float> %a, i32 1
  %4 = extractelement <2 x float> %b, i32 1
  %5 = extractelement <2 x float> %c, i32 1
  %call1 = tail call ptx_device float @_Z3madfff(float %3, float %4, float %5) nounwind readnone
  %vecinit2 = insertelement <2 x float> %vecinit, float %call1, i32 1
  ret <2 x float> %vecinit2
}

declare ptx_device float @_Z3madfff(float, float, float) nounwind readnone

define ptx_device <3 x float> @_Z3madDv3_fS_S_(<3 x float> %a, <3 x float> %b, <3 x float> %c) nounwind readnone {
entry:
  %0 = extractelement <3 x float> %a, i32 0
  %1 = extractelement <3 x float> %b, i32 0
  %2 = extractelement <3 x float> %c, i32 0
  %call = tail call ptx_device float @_Z3madfff(float %0, float %1, float %2) nounwind readnone
  %vecinit = insertelement <3 x float> undef, float %call, i32 0
  %3 = extractelement <3 x float> %a, i32 1
  %4 = extractelement <3 x float> %b, i32 1
  %5 = extractelement <3 x float> %c, i32 1
  %call1 = tail call ptx_device float @_Z3madfff(float %3, float %4, float %5) nounwind readnone
  %vecinit2 = insertelement <3 x float> %vecinit, float %call1, i32 1
  %6 = extractelement <3 x float> %a, i32 2
  %7 = extractelement <3 x float> %b, i32 2
  %8 = extractelement <3 x float> %c, i32 2
  %call3 = tail call ptx_device float @_Z3madfff(float %6, float %7, float %8) nounwind readnone
  %vecinit4 = insertelement <3 x float> %vecinit2, float %call3, i32 2
  ret <3 x float> %vecinit4
}

define ptx_device <4 x float> @_Z3madDv4_fS_S_(<4 x float> %a, <4 x float> %b, <4 x float> %c) nounwind readnone {
entry:
  %0 = extractelement <4 x float> %a, i32 0
  %1 = extractelement <4 x float> %b, i32 0
  %2 = extractelement <4 x float> %c, i32 0
  %call = tail call ptx_device float @_Z3madfff(float %0, float %1, float %2) nounwind readnone
  %vecinit = insertelement <4 x float> undef, float %call, i32 0
  %3 = extractelement <4 x float> %a, i32 1
  %4 = extractelement <4 x float> %b, i32 1
  %5 = extractelement <4 x float> %c, i32 1
  %call1 = tail call ptx_device float @_Z3madfff(float %3, float %4, float %5) nounwind readnone
  %vecinit2 = insertelement <4 x float> %vecinit, float %call1, i32 1
  %6 = extractelement <4 x float> %a, i32 2
  %7 = extractelement <4 x float> %b, i32 2
  %8 = extractelement <4 x float> %c, i32 2
  %call3 = tail call ptx_device float @_Z3madfff(float %6, float %7, float %8) nounwind readnone
  %vecinit4 = insertelement <4 x float> %vecinit2, float %call3, i32 2
  %9 = extractelement <4 x float> %a, i32 3
  %10 = extractelement <4 x float> %b, i32 3
  %11 = extractelement <4 x float> %c, i32 3
  %call5 = tail call ptx_device float @_Z3madfff(float %9, float %10, float %11) nounwind readnone
  %vecinit6 = insertelement <4 x float> %vecinit4, float %call5, i32 3
  ret <4 x float> %vecinit6
}

define ptx_kernel void @simple_float4(<4 x float> addrspace(1)* nocapture %dst, <4 x float> addrspace(1)* nocapture %src) nounwind noinline {
get_global_id.exit5:
  %call.i = tail call ptx_device i32 @__gen_ocl_get_global_id0() nounwind readnone
  %arrayidx = getelementptr inbounds <4 x float> addrspace(1)* %src, i32 %call.i
  %0 = load <4 x float> addrspace(1)* %arrayidx, align 16, !tbaa !1
  %add = fadd <4 x float> %0, <float 0.000000e+00, float 1.000000e+00, float 2.000000e+00, float 3.000000e+00>
  %arrayidx2 = getelementptr inbounds <4 x float> addrspace(1)* %dst, i32 %call.i
  store <4 x float> %add, <4 x float> addrspace(1)* %arrayidx2, align 16, !tbaa !1
  ret void
}

declare ptx_device i32 @__gen_ocl_get_global_id0() nounwind readnone

!opencl.kernels = !{!0}

!0 = metadata !{void (<4 x float> addrspace(1)*, <4 x float> addrspace(1)*)* @simple_float4}
!1 = metadata !{metadata !"omnipotent char", metadata !2}
!2 = metadata !{metadata !"Simple C/C++ TBAA", null}
