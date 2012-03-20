; ModuleID = 'loop3.o'
target datalayout = "e-p:32:32-i64:64:64-f64:64:64-n1:8:16:32:64"
target triple = "ptx32--"

%struct.big = type { [10 x i32] }

define ptx_kernel void @add(i32 addrspace(1)* nocapture %dst, i32 %x, %struct.big* nocapture byval %b) nounwind noinline {
entry:
  %cmp2 = icmp eq i32 %x, 0
  br i1 %cmp2, label %for.end, label %get_local_id.exit.lr.ph

get_local_id.exit.lr.ph:                          ; preds = %entry
  %call.i = tail call ptx_device i32 @__gen_ocl_get_local_id0() nounwind readnone
  br label %get_local_id.exit

get_local_id.exit:                                ; preds = %get_local_id.exit, %get_local_id.exit.lr.ph
  %i.03 = phi i32 [ 0, %get_local_id.exit.lr.ph ], [ %inc1, %get_local_id.exit ]
  %add = add i32 %call.i, %i.03
  %arrayidx = getelementptr inbounds i32 addrspace(1)* %dst, i32 %add
  %0 = load i32 addrspace(1)* %arrayidx, align 4, !tbaa !1
  %inc = add nsw i32 %0, 1
  store i32 %inc, i32 addrspace(1)* %arrayidx, align 4, !tbaa !1
  %inc1 = add nsw i32 %i.03, 1
  %exitcond = icmp eq i32 %inc1, %x
  br i1 %exitcond, label %for.end, label %get_local_id.exit

for.end:                                          ; preds = %get_local_id.exit, %entry
  ret void
}

declare ptx_device i32 @__gen_ocl_get_local_id0() nounwind readnone

!opencl.kernels = !{!0}

!0 = metadata !{void (i32 addrspace(1)*, i32, %struct.big*)* @add}
!1 = metadata !{metadata !"int", metadata !2}
!2 = metadata !{metadata !"omnipotent char", metadata !3}
!3 = metadata !{metadata !"Simple C/C++ TBAA", null}
