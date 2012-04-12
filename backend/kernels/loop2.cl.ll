; ModuleID = 'loop2.cl.o'
target datalayout = "e-p:32:32-i64:64:64-f64:64:64-n1:8:16:32:64"
target triple = "ptx32--"

%struct.big = type { [10 x i32] }

define ptx_kernel void @add(i32 addrspace(1)* nocapture %dst, i32 %x, %struct.big* nocapture byval %b) nounwind noinline {
entry:
  %cmp6 = icmp eq i32 %x, 0
  br i1 %cmp6, label %for.end, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %entry
  %.pre = load i32 addrspace(1)* %dst, align 4, !tbaa !1
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %0 = phi i32 [ %.pre, %for.body.lr.ph ], [ %1, %for.body ]
  %i.07 = phi i32 [ 0, %for.body.lr.ph ], [ %add, %for.body ]
  %add = add nsw i32 %i.07, 1
  %arrayidx = getelementptr inbounds i32 addrspace(1)* %dst, i32 %add
  %1 = load i32 addrspace(1)* %arrayidx, align 4, !tbaa !1
  %cmp1 = icmp sgt i32 %1, 0
  %arrayidx2 = getelementptr inbounds i32 addrspace(1)* %dst, i32 %i.07
  %storemerge.v = select i1 %cmp1, i32 1, i32 2
  %storemerge = add i32 %storemerge.v, %0
  store i32 %storemerge, i32 addrspace(1)* %arrayidx2, align 4
  %exitcond = icmp eq i32 %add, %x
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body, %entry
  ret void
}

!opencl.kernels = !{!0}

!0 = metadata !{void (i32 addrspace(1)*, i32, %struct.big*)* @add}
!1 = metadata !{metadata !"int", metadata !2}
!2 = metadata !{metadata !"omnipotent char", metadata !3}
!3 = metadata !{metadata !"Simple C/C++ TBAA", null}
