; ModuleID = 'loop5.o'
target datalayout = "e-p:32:32-i64:64:64-f64:64:64-n1:8:16:32:64"
target triple = "ptx32--"

%struct.big = type { [10 x i32] }

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

define ptx_kernel void @add(i32 addrspace(1)* nocapture %dst0, i32 addrspace(1)* nocapture %dst1, i32 %x, i32 %y, %struct.big* nocapture byval %b) nounwind noinline {
get_local_id.exit:
  %cmp = icmp sgt i32 %y, 0
  %dst0.dst1 = select i1 %cmp, i32 addrspace(1)* %dst0, i32 addrspace(1)* %dst1
  %call3.i = tail call ptx_device i32 @__gen_ocl_get_local_id1() nounwind readnone
  %cmp1 = icmp ugt i32 %call3.i, 4
  br i1 %cmp1, label %for.cond.preheader, label %for.cond8.preheader

for.cond.preheader:                               ; preds = %get_local_id.exit
  %cmp328 = icmp eq i32 %x, 0
  br i1 %cmp328, label %if.end19, label %get_local_id.exit21.lr.ph

get_local_id.exit21.lr.ph:                        ; preds = %for.cond.preheader
  %call.i = tail call ptx_device i32 @__gen_ocl_get_local_id0() nounwind readnone
  br label %get_local_id.exit21

for.cond8.preheader:                              ; preds = %get_local_id.exit
  %mul.mask = and i32 %x, 2147483647
  %cmp925 = icmp eq i32 %mul.mask, 0
  br i1 %cmp925, label %if.end19, label %get_local_id.exit24.lr.ph

get_local_id.exit24.lr.ph:                        ; preds = %for.cond8.preheader
  %call.i22 = tail call ptx_device i32 @__gen_ocl_get_local_id0() nounwind readnone
  %0 = shl i32 %x, 1
  br label %get_local_id.exit24

get_local_id.exit21:                              ; preds = %get_local_id.exit21, %get_local_id.exit21.lr.ph
  %i.029 = phi i32 [ 0, %get_local_id.exit21.lr.ph ], [ %inc5, %get_local_id.exit21 ]
  %add = add i32 %call.i, %i.029
  %arrayidx = getelementptr inbounds i32 addrspace(1)* %dst0.dst1, i32 %add
  %1 = load i32 addrspace(1)* %arrayidx, align 4, !tbaa !1
  %inc = add nsw i32 %1, 1
  store i32 %inc, i32 addrspace(1)* %arrayidx, align 4, !tbaa !1
  %inc5 = add nsw i32 %i.029, 1
  %exitcond30 = icmp eq i32 %inc5, %x
  br i1 %exitcond30, label %if.end19, label %get_local_id.exit21

get_local_id.exit24:                              ; preds = %get_local_id.exit24, %get_local_id.exit24.lr.ph
  %i7.026 = phi i32 [ 0, %get_local_id.exit24.lr.ph ], [ %inc17, %get_local_id.exit24 ]
  %add12 = add i32 %i7.026, %x
  %add13 = add i32 %add12, %call.i22
  %arrayidx14 = getelementptr inbounds i32 addrspace(1)* %dst0.dst1, i32 %add13
  %2 = load i32 addrspace(1)* %arrayidx14, align 4, !tbaa !1
  %inc15 = add nsw i32 %2, 1
  store i32 %inc15, i32 addrspace(1)* %arrayidx14, align 4, !tbaa !1
  %inc17 = add nsw i32 %i7.026, 1
  %exitcond = icmp eq i32 %inc17, %0
  br i1 %exitcond, label %if.end19, label %get_local_id.exit24

if.end19:                                         ; preds = %get_local_id.exit24, %get_local_id.exit21, %for.cond8.preheader, %for.cond.preheader
  ret void
}

declare ptx_device i32 @__gen_ocl_get_local_id0() nounwind readnone

declare ptx_device i32 @__gen_ocl_get_local_id1() nounwind readnone

!opencl.kernels = !{!0}

!0 = metadata !{void (i32 addrspace(1)*, i32 addrspace(1)*, i32, i32, %struct.big*)* @add}
!1 = metadata !{metadata !"int", metadata !2}
!2 = metadata !{metadata !"omnipotent char", metadata !3}
!3 = metadata !{metadata !"Simple C/C++ TBAA", null}
