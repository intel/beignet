; ModuleID = 'select.o'
target datalayout = "e-p:32:32-i64:64:64-f64:64:64-n1:8:16:32:64"
target triple = "ptx32--"

define ptx_kernel void @test_select(<4 x i32>* nocapture %dst, <4 x i32>* nocapture %src0, <4 x i32>* nocapture %src1) nounwind noinline {
entry:
  %0 = load <4 x i32>* %src0, align 16, !tbaa !1
  %arrayidx1 = getelementptr inbounds <4 x i32>* %src0, i32 1
  %1 = load <4 x i32>* %arrayidx1, align 16, !tbaa !1
  %2 = extractelement <4 x i32> %0, i32 0
  %3 = extractelement <4 x i32> %1, i32 0
  %4 = extractelement <4 x i32> %0, i32 1
  %5 = extractelement <4 x i32> %1, i32 1
  %6 = extractelement <4 x i32> %0, i32 2
  %7 = extractelement <4 x i32> %1, i32 2
  %8 = extractelement <4 x i32> %0, i32 3
  %9 = extractelement <4 x i32> %1, i32 3
  %tobool.i = icmp slt i32 %3, 0
  %cond1.i = select i1 %tobool.i, i32 %3, i32 %2
  %10 = insertelement <4 x i32> undef, i32 %cond1.i, i32 0
  %tobool3.i = icmp slt i32 %5, 0
  %cond7.i = select i1 %tobool3.i, i32 %5, i32 %4
  %11 = insertelement <4 x i32> %10, i32 %cond7.i, i32 1
  %tobool9.i = icmp slt i32 %7, 0
  %cond13.i = select i1 %tobool9.i, i32 %7, i32 %6
  %12 = insertelement <4 x i32> %11, i32 %cond13.i, i32 2
  %tobool15.i = icmp slt i32 %9, 0
  %cond19.i = select i1 %tobool15.i, i32 %9, i32 %8
  %13 = insertelement <4 x i32> %12, i32 %cond19.i, i32 3
  store <4 x i32> %13, <4 x i32>* %dst, align 16, !tbaa !1
  ret void
}

!opencl.kernels = !{!0}

!0 = metadata !{void (<4 x i32>*, <4 x i32>*, <4 x i32>*)* @test_select}
!1 = metadata !{metadata !"omnipotent char", metadata !2}
!2 = metadata !{metadata !"Simple C/C++ TBAA", null}
