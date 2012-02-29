; ModuleID = 'get_global_id.o'
target datalayout = "e-p:32:32-i64:64:64-f64:64:64-n1:8:16:32:64"
target triple = "ptx32--"

define ptx_kernel void @test_global_id(i32* nocapture %dst) nounwind noinline {
get_global_id.exit5:
  %call.i = tail call ptx_device i32 @__gen_get_global_id0() nounwind readonly
  %sext = shl i32 %call.i, 16
  %conv1 = ashr exact i32 %sext, 16
  %arrayidx = getelementptr inbounds i32* %dst, i32 %call.i
  store i32 %conv1, i32* %arrayidx, align 4, !tbaa !1
  ret void
}

declare ptx_device i32 @__gen_get_global_id0() nounwind readonly

!opencl.kernels = !{!0}

!0 = metadata !{void (i32*)* @test_global_id}
!1 = metadata !{metadata !"int", metadata !2}
!2 = metadata !{metadata !"omnipotent char", metadata !3}
!3 = metadata !{metadata !"Simple C/C++ TBAA", null}
