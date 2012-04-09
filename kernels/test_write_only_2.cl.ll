; ModuleID = 'test_write_only_2.cl.o'
target datalayout = "e-p:32:32-i64:64:64-f64:64:64-n1:8:16:32:64"
target triple = "ptx32--"

define ptx_kernel void @test_write_only(float addrspace(1)* nocapture %dst) nounwind noinline {
get_global_id.exit:
  %call.i.i = tail call ptx_device i32 @__gen_ocl_get_local_id0() nounwind readnone
  %call.i3.i = tail call ptx_device i32 @__gen_ocl_get_local_size0() nounwind readnone
  %call.i10.i = tail call ptx_device i32 @__gen_ocl_get_num_groups0() nounwind readnone
  %mul.i = mul i32 %call.i10.i, %call.i3.i
  %add.i = add i32 %mul.i, %call.i.i
  %arrayidx = getelementptr inbounds float addrspace(1)* %dst, i32 %add.i
  store float 1.000000e+00, float addrspace(1)* %arrayidx, align 4, !tbaa !1
  ret void
}

declare ptx_device i32 @__gen_ocl_get_num_groups0() nounwind readnone

declare ptx_device i32 @__gen_ocl_get_local_size0() nounwind readnone

declare ptx_device i32 @__gen_ocl_get_local_id0() nounwind readnone

!opencl.kernels = !{!0}

!0 = metadata !{void (float addrspace(1)*)* @test_write_only}
!1 = metadata !{metadata !"float", metadata !2}
!2 = metadata !{metadata !"omnipotent char", metadata !3}
!3 = metadata !{metadata !"Simple C/C++ TBAA", null}
