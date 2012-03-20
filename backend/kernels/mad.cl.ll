; ModuleID = 'mad.cl.o'
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

define ptx_kernel void @add(i32 addrspace(1)* nocapture %dst, i32 %x, float %z) nounwind noinline {
entry:
  %cmp16 = icmp eq i32 %x, 0
  br i1 %cmp16, label %for.end, label %for.body

for.body:                                         ; preds = %for.body, %entry
  %i.017 = phi i32 [ %inc, %for.body ], [ 0, %entry ]
  %arrayidx = getelementptr inbounds i32 addrspace(1)* %dst, i32 %i.017
  %0 = load i32 addrspace(1)* %arrayidx, align 4, !tbaa !1
  %call2 = tail call ptx_device i32 @_Z3madiii(i32 %0, i32 2, i32 3) nounwind readonly
  %conv = sitofp i32 %0 to float
  %call5 = tail call ptx_device float @_Z3madfff(float %conv, float 2.000000e+00, float 3.000000e+00) nounwind readnone
  %call.i = tail call ptx_device float @_Z3madfff(float %conv, float 0.000000e+00, float 3.000000e+00) nounwind readnone
  %vecinit.i = insertelement <4 x float> undef, float %call.i, i32 0
  %call1.i = tail call ptx_device float @_Z3madfff(float %conv, float 1.000000e+00, float 3.000000e+00) nounwind readnone
  %vecinit2.i = insertelement <4 x float> %vecinit.i, float %call1.i, i32 1
  %vecinit4.i = insertelement <4 x float> %vecinit2.i, float %call5, i32 2
  %call5.i = tail call ptx_device float @_Z3madfff(float %conv, float 3.000000e+00, float 3.000000e+00) nounwind readnone
  %vecinit6.i = insertelement <4 x float> %vecinit4.i, float %call5.i, i32 3
  %mul = fmul <4 x float> %vecinit6.i, <float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00>
  %conv9 = fptosi float %call5 to i32
  %add = add nsw i32 %conv9, %call2
  %conv10 = sitofp i32 %add to float
  %1 = extractelement <4 x float> %mul, i32 0
  %add11 = fadd float %conv10, %1
  %2 = extractelement <4 x float> %mul, i32 1
  %add12 = fadd float %add11, %2
  %3 = extractelement <4 x float> %mul, i32 2
  %add13 = fadd float %add12, %3
  %conv14 = fptosi float %add13 to i32
  store i32 %conv14, i32 addrspace(1)* %arrayidx, align 4, !tbaa !1
  %inc = add nsw i32 %i.017, 1
  %exitcond = icmp eq i32 %inc, %x
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body, %entry
  ret void
}

declare ptx_device i32 @_Z3madiii(i32, i32, i32) nounwind readonly

!opencl.kernels = !{!0}

!0 = metadata !{void (i32 addrspace(1)*, i32, float)* @add}
!1 = metadata !{metadata !"int", metadata !2}
!2 = metadata !{metadata !"omnipotent char", metadata !3}
!3 = metadata !{metadata !"Simple C/C++ TBAA", null}
