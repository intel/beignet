; ModuleID = 'loop5.cl.o'
target datalayout = "e-p:32:32-i64:64:64-f64:64:64-n1:8:16:32:64"
target triple = "ptx32--"

%struct.big = type { [10 x i32] }

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
