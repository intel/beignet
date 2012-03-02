; ModuleID = 'struct.o'
target datalayout = "e-p:32:32-i64:64:64-f64:64:64-n1:8:16:32:64"
target triple = "ptx32--"

%struct.my_struct = type { i32, [2 x i32] }

@g = addrspace(1) global [4 x i32] [i32 0, i32 1, i32 2, i32 3], align 4
@struct_cl.array = internal addrspace(4) unnamed_addr global [256 x i32] zeroinitializer, align 4

define ptx_kernel void @struct_cl(%struct.my_struct* nocapture byval %s, i32 %x, i32* nocapture %mem) nounwind noinline {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %i.05 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %arrayidx = getelementptr inbounds [256 x i32] addrspace(4)* @struct_cl.array, i32 0, i32 %i.05
  store i32 %i.05, i32 addrspace(4)* %arrayidx, align 4, !tbaa !1
  %inc = add nsw i32 %i.05, 1
  %exitcond = icmp eq i32 %inc, 256
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  %a = getelementptr inbounds %struct.my_struct* %s, i32 0, i32 0
  %0 = load i32* %a, align 4, !tbaa !1
  %arrayidx1 = getelementptr inbounds [256 x i32] addrspace(4)* @struct_cl.array, i32 0, i32 %x
  %1 = load i32 addrspace(4)* %arrayidx1, align 4, !tbaa !1
  %arrayidx2 = getelementptr inbounds [4 x i32] addrspace(1)* @g, i32 0, i32 %x
  %2 = load i32 addrspace(1)* %arrayidx2, align 4, !tbaa !1
  %add = add i32 %1, %0
  %add3 = add i32 %add, %2
  store i32 %add3, i32* %mem, align 4, !tbaa !1
  ret void
}

!opencl.kernels = !{!0}

!0 = metadata !{void (%struct.my_struct*, i32, i32*)* @struct_cl}
!1 = metadata !{metadata !"int", metadata !2}
!2 = metadata !{metadata !"omnipotent char", metadata !3}
!3 = metadata !{metadata !"Simple C/C++ TBAA", null}
