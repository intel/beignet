; ModuleID = 'test_copy_buffer.cl.o'
target datalayout = "e-p:32:32-i64:64:64-f64:64:64-n1:8:16:32:64"
target triple = "ptx32--"

define ptx_kernel void @test_copy_buffer(float addrspace(1)* nocapture %src, float addrspace(1)* nocapture %dst) nounwind noinline {
get_global_id.exit:
  %call.i.i = tail call ptx_device i32 @__gen_ocl_get_local_id0() nounwind readnone
  %call.i3.i = tail call ptx_device i32 @__gen_ocl_get_local_size0() nounwind readnone
  %call.i10.i = tail call ptx_device i32 @__gen_ocl_get_group_id0() nounwind readnone
  %mul.i = mul i32 %call.i10.i, %call.i3.i
  %add.i = add i32 %mul.i, %call.i.i
  %arrayidx = getelementptr inbounds float addrspace(1)* %src, i32 %add.i
  %0 = load float addrspace(1)* %arrayidx, align 4, !tbaa !1
  %arrayidx1 = getelementptr inbounds float addrspace(1)* %dst, i32 %add.i
  store float %0, float addrspace(1)* %arrayidx1, align 4, !tbaa !1
  ret void
}

declare ptx_device i32 @__gen_ocl_get_group_id0() nounwind readnone

declare ptx_device i32 @__gen_ocl_get_local_size0() nounwind readnone

declare ptx_device i32 @__gen_ocl_get_local_id0() nounwind readnone

!opencl.kernels = !{!0}

!0 = metadata !{void (float addrspace(1)*, float addrspace(1)*)* @test_copy_buffer}
!1 = metadata !{metadata !"float", metadata !2}
!2 = metadata !{metadata !"omnipotent char", metadata !3}
!3 = metadata !{metadata !"Simple C/C++ TBAA", null}
