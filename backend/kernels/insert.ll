; ModuleID = 'insert.o'
target datalayout = "e-p:32:32-i64:64:64-f64:64:64-n1:8:16:32:64"
target triple = "ptx32--"

define ptx_kernel void @insert(<4 x i32>* nocapture %dst, <4 x i32>* nocapture %src, i32 %c) nounwind noinline {
entry:
  %0 = load <4 x i32>* %dst, align 16, !tbaa !1
  %vecext = extractelement <4 x i32> %0, i32 %c
  %1 = insertelement <4 x i32> %0, i32 %vecext, i32 0
  store <4 x i32> %1, <4 x i32>* %dst, align 16
  %arrayidx2 = getelementptr inbounds <4 x i32>* %dst, i32 1
  %2 = load <4 x i32>* %arrayidx2, align 16
  %3 = shufflevector <4 x i32> %2, <4 x i32> undef, <3 x i32> <i32 0, i32 1, i32 2>
  %4 = load <4 x i32>* %src, align 16
  %5 = shufflevector <4 x i32> %4, <4 x i32> undef, <3 x i32> <i32 0, i32 1, i32 2>
  %add = add <3 x i32> %3, %5
  %6 = shufflevector <3 x i32> %add, <3 x i32> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %7 = shufflevector <4 x i32> %1, <4 x i32> %6, <4 x i32> <i32 0, i32 4, i32 5, i32 6>
  store <4 x i32> %7, <4 x i32>* %dst, align 16
  ret void
}

!opencl.kernels = !{!0}

!0 = metadata !{void (<4 x i32>*, <4 x i32>*, i32)* @insert}
!1 = metadata !{metadata !"omnipotent char", metadata !2}
!2 = metadata !{metadata !"Simple C/C++ TBAA", null}
