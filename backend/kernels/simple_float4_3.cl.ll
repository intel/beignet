; ModuleID = 'simple_float4_3.cl.o'
target datalayout = "e-p:32:32-i64:64:64-f64:64:64-n1:8:16:32:64"
target triple = "ptx32--"

define ptx_kernel void @simple_float4(<4 x float> addrspace(1)* nocapture %dst, <4 x float> addrspace(1)* nocapture %src, i1 %b) nounwind noinline {
get_global_id.exit35:
  %call.i.i = tail call ptx_device i32 @__gen_ocl_get_local_id0() nounwind readnone
  %call.i3.i = tail call ptx_device i32 @__gen_ocl_get_local_size0() nounwind readnone
  %call.i10.i = tail call ptx_device i32 @__gen_ocl_get_num_groups0() nounwind readnone
  %mul.i = mul i32 %call.i10.i, %call.i3.i
  %add.i = add i32 %mul.i, %call.i.i
  %arrayidx = getelementptr inbounds <4 x float> addrspace(1)* %src, i32 %add.i
  %0 = load <4 x float> addrspace(1)* %arrayidx, align 16, !tbaa !1
  %arrayidx5 = getelementptr inbounds <4 x float> addrspace(1)* %dst, i32 %add.i
  store <4 x float> %0, <4 x float> addrspace(1)* %arrayidx5, align 16, !tbaa !1
  %arrayidx6 = getelementptr inbounds <4 x float> addrspace(1)* %src, i32 2
  %1 = load <4 x float> addrspace(1)* %arrayidx6, align 16
  %2 = extractelement <4 x float> %1, i32 0
  %vecinit = insertelement <4 x float> undef, float %2, i32 0
  %vecinit7 = insertelement <4 x float> %vecinit, float 1.000000e+00, i32 1
  %vecinit8 = insertelement <4 x float> %vecinit7, float 2.000000e+00, i32 2
  %vecinit9 = insertelement <4 x float> %vecinit8, float 3.000000e+00, i32 3
  %add = fadd <4 x float> %0, %vecinit9
  store <4 x float> %add, <4 x float> addrspace(1)* %arrayidx5, align 16, !tbaa !1
  ret void
}

declare ptx_device i32 @__gen_ocl_get_num_groups0() nounwind readnone

declare ptx_device i32 @__gen_ocl_get_local_size0() nounwind readnone

declare ptx_device i32 @__gen_ocl_get_local_id0() nounwind readnone

!opencl.kernels = !{!0}

!0 = metadata !{void (<4 x float> addrspace(1)*, <4 x float> addrspace(1)*, i1)* @simple_float4}
!1 = metadata !{metadata !"omnipotent char", metadata !2}
!2 = metadata !{metadata !"Simple C/C++ TBAA", null}
