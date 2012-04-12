; ModuleID = 'vector_constant.cl.o'
target datalayout = "e-p:32:32-i64:64:64-f64:64:64-n1:8:16:32:64"
target triple = "ptx32--"

define ptx_kernel void @simple_float4(<4 x float> addrspace(1)* nocapture %dst, <4 x float> addrspace(1)* nocapture %src) nounwind noinline {
get_global_id.exit11:
  %call.i.i = tail call ptx_device i32 @__gen_ocl_get_local_id0() nounwind readnone
  %call.i3.i = tail call ptx_device i32 @__gen_ocl_get_local_size0() nounwind readnone
  %call.i10.i = tail call ptx_device i32 @__gen_ocl_get_num_groups0() nounwind readnone
  %mul.i = mul i32 %call.i10.i, %call.i3.i
  %add.i = add i32 %mul.i, %call.i.i
  %arrayidx = getelementptr inbounds <4 x float> addrspace(1)* %src, i32 %add.i
  %0 = load <4 x float> addrspace(1)* %arrayidx, align 16, !tbaa !1
  %add = fadd <4 x float> %0, <float 0.000000e+00, float 1.000000e+00, float 2.000000e+00, float 3.000000e+00>
  %arrayidx2 = getelementptr inbounds <4 x float> addrspace(1)* %dst, i32 %add.i
  store <4 x float> %add, <4 x float> addrspace(1)* %arrayidx2, align 16, !tbaa !1
  ret void
}

declare ptx_device i32 @__gen_ocl_get_num_groups0() nounwind readnone

declare ptx_device i32 @__gen_ocl_get_local_size0() nounwind readnone

declare ptx_device i32 @__gen_ocl_get_local_id0() nounwind readnone

!opencl.kernels = !{!0}

!0 = metadata !{void (<4 x float> addrspace(1)*, <4 x float> addrspace(1)*)* @simple_float4}
!1 = metadata !{metadata !"omnipotent char", metadata !2}
!2 = metadata !{metadata !"Simple C/C++ TBAA", null}
