; ModuleID = 'extract.o'
target datalayout = "e-p:32:32-i64:64:64-f64:64:64-n1:8:16:32:64"
target triple = "ptx32--"

define ptx_kernel void @extract(<4 x i32>* nocapture %dst, <4 x i32>* nocapture %src, i32 %c) nounwind noinline {
entry:
  %0 = load <4 x i32>* %src, align 16, !tbaa !1
  %1 = extractelement <4 x i32> %0, i32 0
  %vecinit = insertelement <4 x i32> undef, i32 %1, i32 0
  %vecinit1 = insertelement <4 x i32> %vecinit, i32 1, i32 1
  %vecinit2 = insertelement <4 x i32> %vecinit1, i32 2, i32 2
  %vecinit3 = insertelement <4 x i32> %vecinit2, i32 3, i32 3
  store <4 x i32> %vecinit3, <4 x i32>* %dst, align 16, !tbaa !1
  ret void
}

!opencl.kernels = !{!0}

!0 = metadata !{void (<4 x i32>*, <4 x i32>*, i32)* @extract}
!1 = metadata !{metadata !"omnipotent char", metadata !2}
!2 = metadata !{metadata !"Simple C/C++ TBAA", null}
