; ModuleID = 'select.cl.o'
target datalayout = "e-p:32:32-i64:64:64-f64:64:64-n1:8:16:32:64"
target triple = "ptx32--"

define ptx_device <2 x float> @_Z3madDv2_fS_S_(<2 x float> %a, <2 x float> %b, <2 x float> %c) nounwind readnone {
entry:
  %0 = extractelement <2 x float> %a, i32 0
  %1 = extractelement <2 x float> %b, i32 0
  %2 = extractelement <2 x float> %c, i32 0
  %call = tail call ptx_device float @_Z3madfff(float %0, float %1, float %2) nounwind readnone
  %vecinit = insertelement <2 x float> undef, float %call, i32 0
  %3 = extractelement <2 x float> %a, i32 1
  %4 = extractelement <2 x float> %b, i32 1
  %5 = extractelement <2 x float> %c, i32 1
  %call1 = tail call ptx_device float @_Z3madfff(float %3, float %4, float %5) nounwind readnone
  %vecinit2 = insertelement <2 x float> %vecinit, float %call1, i32 1
  ret <2 x float> %vecinit2
}

declare ptx_device float @_Z3madfff(float, float, float) nounwind readnone

define ptx_device <3 x float> @_Z3madDv3_fS_S_(<3 x float> %a, <3 x float> %b, <3 x float> %c) nounwind readnone {
entry:
  %0 = extractelement <3 x float> %a, i32 0
  %1 = extractelement <3 x float> %b, i32 0
  %2 = extractelement <3 x float> %c, i32 0
  %call = tail call ptx_device float @_Z3madfff(float %0, float %1, float %2) nounwind readnone
  %vecinit = insertelement <3 x float> undef, float %call, i32 0
  %3 = extractelement <3 x float> %a, i32 1
  %4 = extractelement <3 x float> %b, i32 1
  %5 = extractelement <3 x float> %c, i32 1
  %call1 = tail call ptx_device float @_Z3madfff(float %3, float %4, float %5) nounwind readnone
  %vecinit2 = insertelement <3 x float> %vecinit, float %call1, i32 1
  %6 = extractelement <3 x float> %a, i32 2
  %7 = extractelement <3 x float> %b, i32 2
  %8 = extractelement <3 x float> %c, i32 2
  %call3 = tail call ptx_device float @_Z3madfff(float %6, float %7, float %8) nounwind readnone
  %vecinit4 = insertelement <3 x float> %vecinit2, float %call3, i32 2
  ret <3 x float> %vecinit4
}

define ptx_device <4 x float> @_Z3madDv4_fS_S_(<4 x float> %a, <4 x float> %b, <4 x float> %c) nounwind readnone {
entry:
  %0 = extractelement <4 x float> %a, i32 0
  %1 = extractelement <4 x float> %b, i32 0
  %2 = extractelement <4 x float> %c, i32 0
  %call = tail call ptx_device float @_Z3madfff(float %0, float %1, float %2) nounwind readnone
  %vecinit = insertelement <4 x float> undef, float %call, i32 0
  %3 = extractelement <4 x float> %a, i32 1
  %4 = extractelement <4 x float> %b, i32 1
  %5 = extractelement <4 x float> %c, i32 1
  %call1 = tail call ptx_device float @_Z3madfff(float %3, float %4, float %5) nounwind readnone
  %vecinit2 = insertelement <4 x float> %vecinit, float %call1, i32 1
  %6 = extractelement <4 x float> %a, i32 2
  %7 = extractelement <4 x float> %b, i32 2
  %8 = extractelement <4 x float> %c, i32 2
  %call3 = tail call ptx_device float @_Z3madfff(float %6, float %7, float %8) nounwind readnone
  %vecinit4 = insertelement <4 x float> %vecinit2, float %call3, i32 2
  %9 = extractelement <4 x float> %a, i32 3
  %10 = extractelement <4 x float> %b, i32 3
  %11 = extractelement <4 x float> %c, i32 3
  %call5 = tail call ptx_device float @_Z3madfff(float %9, float %10, float %11) nounwind readnone
  %vecinit6 = insertelement <4 x float> %vecinit4, float %call5, i32 3
  ret <4 x float> %vecinit6
}

define ptx_kernel void @test_select(<4 x i32> addrspace(1)* nocapture %dst, <4 x i32> addrspace(1)* nocapture %src0, <4 x i32> addrspace(1)* nocapture %src1) nounwind noinline {
entry:
  %0 = load <4 x i32> addrspace(1)* %src0, align 16, !tbaa !1
  %arrayidx1 = getelementptr inbounds <4 x i32> addrspace(1)* %src0, i32 1
  %1 = load <4 x i32> addrspace(1)* %arrayidx1, align 16, !tbaa !1
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
  store <4 x i32> %13, <4 x i32> addrspace(1)* %dst, align 16, !tbaa !1
  ret void
}

!opencl.kernels = !{!0}

!0 = metadata !{void (<4 x i32> addrspace(1)*, <4 x i32> addrspace(1)*, <4 x i32> addrspace(1)*)* @test_select}
!1 = metadata !{metadata !"omnipotent char", metadata !2}
!2 = metadata !{metadata !"Simple C/C++ TBAA", null}
