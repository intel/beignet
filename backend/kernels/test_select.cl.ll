; ModuleID = 'test_select.cl.o'
target datalayout = "e-p:32:32-i64:64:64-f64:64:64-n1:8:16:32:64"
target triple = "ptx32--"

define ptx_kernel void @test_select(i32 addrspace(1)* nocapture %dst, i32 addrspace(1)* nocapture %src) nounwind noinline {
get_global_id.exit13:
  %call.i.i = tail call ptx_device i32 @__gen_ocl_get_local_id0() nounwind readnone
  %call.i3.i = tail call ptx_device i32 @__gen_ocl_get_local_size0() nounwind readnone
  %call.i10.i = tail call ptx_device i32 @__gen_ocl_get_num_groups0() nounwind readnone
  %mul.i = mul i32 %call.i10.i, %call.i3.i
  %add.i = add i32 %mul.i, %call.i.i
  %arrayidx = getelementptr inbounds i32 addrspace(1)* %src, i32 %add.i
  %0 = load i32 addrspace(1)* %arrayidx, align 4, !tbaa !1
  %cmp = icmp sgt i32 %0, 1
  %arrayidx2 = getelementptr inbounds i32 addrspace(1)* %dst, i32 %add.i
  %. = select i1 %cmp, i32 1, i32 2
  store i32 %., i32 addrspace(1)* %arrayidx2, align 4
  ret void
}

declare ptx_device i32 @__gen_ocl_get_num_groups0() nounwind readnone

declare ptx_device i32 @__gen_ocl_get_local_size0() nounwind readnone

declare ptx_device i32 @__gen_ocl_get_local_id0() nounwind readnone

!opencl.kernels = !{!0}

!0 = metadata !{void (i32 addrspace(1)*, i32 addrspace(1)*)* @test_select}
!1 = metadata !{metadata !"int", metadata !2}
!2 = metadata !{metadata !"omnipotent char", metadata !3}
!3 = metadata !{metadata !"Simple C/C++ TBAA", null}
