; ModuleID = 'insert.o'
target datalayout = "e-p:32:32-i64:64:64-f64:64:64-n1:8:16:32:64"
target triple = "ptx32--"

define ptx_kernel void @insert(<4 x i32>* nocapture %dst, <4 x i32>* nocapture %src, i32 %c) nounwind noinline {
entry:
  %0 = load <4 x i32>* %src, align 16
  %1 = insertelement <4 x i32> %0, i32 1, i32 2
  store <4 x i32> %1, <4 x i32>* %src, align 16
  store <4 x i32> %1, <4 x i32>* %dst, align 16, !tbaa !1
  ret void
}

!opencl.kernels = !{!0}

!0 = metadata !{void (<4 x i32>*, <4 x i32>*, i32)* @insert}
!1 = metadata !{metadata !"omnipotent char", metadata !2}
!2 = metadata !{metadata !"Simple C/C++ TBAA", null}
