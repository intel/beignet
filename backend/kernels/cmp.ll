; ModuleID = 'cmp.o'
target datalayout = "e-p:32:32-i64:64:64-f64:64:64-n1:8:16:32:64"
target triple = "ptx32--"

define ptx_kernel void @test_cmp(i8* nocapture %dst, i32 %x, i32 %y, float %z, float %w) nounwind noinline {
entry:
  %cmp = icmp slt i32 %x, %y
  %conv = zext i1 %cmp to i32
  %cmp1 = fcmp ogt float %z, %w
  %add = sext i1 %cmp1 to i32
  %tobool = icmp ne i32 %conv, %add
  %frombool = zext i1 %tobool to i8
  store i8 %frombool, i8* %dst, align 1, !tbaa !1
  ret void
}

!opencl.kernels = !{!0}

!0 = metadata !{void (i8*, i32, i32, float, float)* @test_cmp}
!1 = metadata !{metadata !"bool", metadata !2}
!2 = metadata !{metadata !"omnipotent char", metadata !3}
!3 = metadata !{metadata !"Simple C/C++ TBAA", null}
