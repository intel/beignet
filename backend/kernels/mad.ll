; ModuleID = 'mad.o'
target datalayout = "e-p:32:32-i64:64:64-f64:64:64-n1:8:16:32:64"
target triple = "ptx32--"

define ptx_kernel void @add(i32* nocapture %dst, i32 %x, float %z) nounwind noinline {
entry:
  %cmp16 = icmp eq i32 %x, 0
  br i1 %cmp16, label %for.end, label %for.body

for.body:                                         ; preds = %for.body, %entry
  %i.017 = phi i32 [ %inc, %for.body ], [ 0, %entry ]
  %arrayidx = getelementptr inbounds i32* %dst, i32 %i.017
  %0 = load i32* %arrayidx, align 4, !tbaa !1
  %call2 = tail call ptx_device i32 @_Z3madiii(i32 %0, i32 2, i32 3) nounwind readonly
  %conv = sitofp i32 %0 to float
  %call5 = tail call ptx_device float @_Z3madfff(float %conv, float 2.000000e+00, float 3.000000e+00) nounwind readonly
  %1 = insertelement <4 x float> undef, float %conv, i32 0
  %splat = shufflevector <4 x float> %1, <4 x float> undef, <4 x i32> zeroinitializer
  %call8 = tail call ptx_device <4 x float> @_Z3madDv4_fS_S_(<4 x float> %splat, <4 x float> <float 0.000000e+00, float 1.000000e+00, float 2.000000e+00, float 3.000000e+00>, <4 x float> <float 3.000000e+00, float 3.000000e+00, float 3.000000e+00, float 3.000000e+00>) nounwind readonly
  %mul = fmul <4 x float> %call8, <float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00>
  %conv9 = fptosi float %call5 to i32
  %add = add nsw i32 %conv9, %call2
  %conv10 = sitofp i32 %add to float
  %2 = extractelement <4 x float> %mul, i32 0
  %add11 = fadd float %conv10, %2
  %3 = extractelement <4 x float> %mul, i32 1
  %add12 = fadd float %add11, %3
  %4 = extractelement <4 x float> %mul, i32 2
  %add13 = fadd float %add12, %4
  %conv14 = fptosi float %add13 to i32
  store i32 %conv14, i32* %arrayidx, align 4, !tbaa !1
  %inc = add nsw i32 %i.017, 1
  %exitcond = icmp eq i32 %inc, %x
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body, %entry
  ret void
}

declare ptx_device i32 @_Z3madiii(i32, i32, i32) nounwind readonly

declare ptx_device float @_Z3madfff(float, float, float) nounwind readonly

declare ptx_device <4 x float> @_Z3madDv4_fS_S_(<4 x float>, <4 x float>, <4 x float>) nounwind readonly

!opencl.kernels = !{!0}

!0 = metadata !{void (i32*, i32, float)* @add}
!1 = metadata !{metadata !"int", metadata !2}
!2 = metadata !{metadata !"omnipotent char", metadata !3}
!3 = metadata !{metadata !"Simple C/C++ TBAA", null}
