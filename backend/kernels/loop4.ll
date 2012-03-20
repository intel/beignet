; ModuleID = 'loop4.o'
target datalayout = "e-p:32:32-i64:64:64-f64:64:64-n1:8:16:32:64"
target triple = "ptx32--"

%struct.big = type { [10 x i32] }

define ptx_kernel void @add(i32 addrspace(1)* nocapture %dst, i32 %x, %struct.big* nocapture byval %b) nounwind noinline {
get_local_id.exit:
  %call3.i = tail call ptx_device i32 @__gen_ocl_get_local_id1() nounwind readnone
  %cmp = icmp ugt i32 %call3.i, 4
  br i1 %cmp, label %for.cond.preheader, label %for.cond5.preheader

for.cond.preheader:                               ; preds = %get_local_id.exit
  %cmp124 = icmp eq i32 %x, 0
  br i1 %cmp124, label %if.end, label %get_local_id.exit17.lr.ph

get_local_id.exit17.lr.ph:                        ; preds = %for.cond.preheader
  %call.i = tail call ptx_device i32 @__gen_ocl_get_local_id0() nounwind readnone
  br label %get_local_id.exit17

for.cond5.preheader:                              ; preds = %get_local_id.exit
  %mul.mask = and i32 %x, 2147483647
  %cmp621 = icmp eq i32 %mul.mask, 0
  br i1 %cmp621, label %if.end, label %get_local_id.exit20.lr.ph

get_local_id.exit20.lr.ph:                        ; preds = %for.cond5.preheader
  %call.i18 = tail call ptx_device i32 @__gen_ocl_get_local_id0() nounwind readnone
  %0 = shl i32 %x, 1
  br label %get_local_id.exit20

get_local_id.exit17:                              ; preds = %get_local_id.exit17, %get_local_id.exit17.lr.ph
  %i.025 = phi i32 [ 0, %get_local_id.exit17.lr.ph ], [ %inc3, %get_local_id.exit17 ]
  %add = add i32 %call.i, %i.025
  %arrayidx = getelementptr inbounds i32 addrspace(1)* %dst, i32 %add
  %1 = load i32 addrspace(1)* %arrayidx, align 4, !tbaa !1
  %inc = add nsw i32 %1, 1
  store i32 %inc, i32 addrspace(1)* %arrayidx, align 4, !tbaa !1
  %inc3 = add nsw i32 %i.025, 1
  %exitcond26 = icmp eq i32 %inc3, %x
  br i1 %exitcond26, label %if.end, label %get_local_id.exit17

get_local_id.exit20:                              ; preds = %get_local_id.exit20, %get_local_id.exit20.lr.ph
  %i4.022 = phi i32 [ 0, %get_local_id.exit20.lr.ph ], [ %inc14, %get_local_id.exit20 ]
  %add9 = add i32 %i4.022, %x
  %add10 = add i32 %add9, %call.i18
  %arrayidx11 = getelementptr inbounds i32 addrspace(1)* %dst, i32 %add10
  %2 = load i32 addrspace(1)* %arrayidx11, align 4, !tbaa !1
  %inc12 = add nsw i32 %2, 1
  store i32 %inc12, i32 addrspace(1)* %arrayidx11, align 4, !tbaa !1
  %inc14 = add nsw i32 %i4.022, 1
  %exitcond = icmp eq i32 %inc14, %0
  br i1 %exitcond, label %if.end, label %get_local_id.exit20

if.end:                                           ; preds = %get_local_id.exit20, %get_local_id.exit17, %for.cond5.preheader, %for.cond.preheader
  ret void
}

declare ptx_device i32 @__gen_ocl_get_local_id0() nounwind readnone

declare ptx_device i32 @__gen_ocl_get_local_id1() nounwind readnone

!opencl.kernels = !{!0}

!0 = metadata !{void (i32 addrspace(1)*, i32, %struct.big*)* @add}
!1 = metadata !{metadata !"int", metadata !2}
!2 = metadata !{metadata !"omnipotent char", metadata !3}
!3 = metadata !{metadata !"Simple C/C++ TBAA", null}
