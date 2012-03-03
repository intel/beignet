; ModuleID = 'struct.o'
target datalayout = "e-p:32:32-i64:64:64-f64:64:64-n1:8:16:32:64"
target triple = "ptx32--"

%struct.my_struct = type { i32, [2 x i32] }

@g = addrspace(1) constant [4 x i32] [i32 0, i32 1, i32 2, i32 3], align 4
@struct_cl.array = internal addrspace(4) global [256 x %struct.my_struct] zeroinitializer, align 4

define ptx_kernel void @struct_cl(%struct.my_struct* nocapture byval %s, i32 %x, i32* nocapture %mem, i32 %y) nounwind noinline {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %i.020 = phi i32 [ 0, %entry ], [ %add, %for.body ]
  %a = getelementptr inbounds [256 x %struct.my_struct] addrspace(4)* @struct_cl.array, i32 0, i32 %i.020, i32 0
  store i32 %i.020, i32 addrspace(4)* %a, align 4, !tbaa !1
  %arrayidx2 = getelementptr inbounds [256 x %struct.my_struct] addrspace(4)* @struct_cl.array, i32 0, i32 %i.020, i32 1, i32 0
  %add = add nsw i32 %i.020, 1
  store i32 %add, i32 addrspace(4)* %arrayidx2, align 4, !tbaa !1
  %exitcond = icmp eq i32 %add, 256
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  %arrayidx6 = getelementptr inbounds [256 x %struct.my_struct] addrspace(4)* @struct_cl.array, i32 0, i32 %y
  %0 = bitcast %struct.my_struct addrspace(4)* %arrayidx6 to i8 addrspace(4)*
  tail call void @llvm.memcpy.p4i8.p4i8.i32(i8 addrspace(4)* bitcast ([256 x %struct.my_struct] addrspace(4)* @struct_cl.array to i8 addrspace(4)*), i8 addrspace(4)* %0, i32 12, i32 4, i1 false)
  %a7 = getelementptr inbounds %struct.my_struct* %s, i32 0, i32 0
  %1 = load i32* %a7, align 4, !tbaa !1
  %a9 = getelementptr inbounds [256 x %struct.my_struct] addrspace(4)* @struct_cl.array, i32 0, i32 %x, i32 0
  %2 = load i32 addrspace(4)* %a9, align 4, !tbaa !1
  %add11 = add nsw i32 %x, 1
  %arrayidx14 = getelementptr inbounds [256 x %struct.my_struct] addrspace(4)* @struct_cl.array, i32 0, i32 %add11, i32 1, i32 0
  %3 = load i32 addrspace(4)* %arrayidx14, align 4, !tbaa !1
  %arrayidx16 = getelementptr inbounds [4 x i32] addrspace(1)* @g, i32 0, i32 %x
  %4 = load i32 addrspace(1)* %arrayidx16, align 4, !tbaa !1
  %add10 = add i32 %1, 3
  %add15 = add i32 %add10, %2
  %add17 = add i32 %add15, %3
  %add18 = add i32 %add17, %4
  store i32 %add18, i32* %mem, align 4, !tbaa !1
  ret void
}

declare void @llvm.memcpy.p4i8.p4i8.i32(i8 addrspace(4)* nocapture, i8 addrspace(4)* nocapture, i32, i32, i1) nounwind

!opencl.kernels = !{!0}

!0 = metadata !{void (%struct.my_struct*, i32, i32*, i32)* @struct_cl}
!1 = metadata !{metadata !"int", metadata !2}
!2 = metadata !{metadata !"omnipotent char", metadata !3}
!3 = metadata !{metadata !"Simple C/C++ TBAA", null}
