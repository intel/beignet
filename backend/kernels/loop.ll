; ModuleID = 'loop.o'
target datalayout = "e-p:32:32-i64:64:64-f64:64:64-n1:8:16:32:64"
target triple = "ptx32--"

define ptx_kernel void @add(i32* nocapture %dst, i32 %x) nounwind noinline {
entry:
  %cmp2 = icmp eq i32 %x, 0
  br i1 %cmp2, label %for.end, label %for.body

for.body:                                         ; preds = %for.body, %entry
  %i.03 = phi i32 [ %inc1, %for.body ], [ 0, %entry ]
  %arrayidx = getelementptr inbounds i32* %dst, i32 %i.03
  %0 = load i32* %arrayidx, align 4, !tbaa !1
  %inc = add nsw i32 %0, 1
  store i32 %inc, i32* %arrayidx, align 4, !tbaa !1
  %inc1 = add nsw i32 %i.03, 1
  %exitcond = icmp eq i32 %inc1, %x
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body, %entry
  ret void
}

!opencl.kernels = !{!0}

!0 = metadata !{void (i32*, i32)* @add}
!1 = metadata !{metadata !"int", metadata !2}
!2 = metadata !{metadata !"omnipotent char", metadata !3}
!3 = metadata !{metadata !"Simple C/C++ TBAA", null}
