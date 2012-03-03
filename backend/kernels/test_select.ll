; ModuleID = 'test_select.o'
target datalayout = "e-p:32:32-i64:64:64-f64:64:64-n1:8:16:32:64"
target triple = "ptx32--"

define ptx_kernel void @test_select(i32* nocapture %dst, i32* nocapture %src) nounwind noinline {
get_global_id.exit7:
  %call.i = tail call ptx_device i32 @__gen_ocl_get_global_id0() nounwind readnone
  %arrayidx = getelementptr inbounds i32* %src, i32 %call.i
  %0 = load i32* %arrayidx, align 4, !tbaa !1
  %cmp = icmp sgt i32 %0, 1
  %arrayidx2 = getelementptr inbounds i32* %dst, i32 %call.i
  %. = select i1 %cmp, i32 1, i32 2
  store i32 %., i32* %arrayidx2, align 4
  ret void
}

declare ptx_device i32 @__gen_ocl_get_global_id0() nounwind readnone

!opencl.kernels = !{!0}

!0 = metadata !{void (i32*, i32*)* @test_select}
!1 = metadata !{metadata !"int", metadata !2}
!2 = metadata !{metadata !"omnipotent char", metadata !3}
!3 = metadata !{metadata !"Simple C/C++ TBAA", null}
