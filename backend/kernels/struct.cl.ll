; ModuleID = 'struct.cl.o'
target datalayout = "e-p:32:32-i64:64:64-f64:64:64-n1:8:16:32:64"
target triple = "ptx32--"

%struct.my_struct = type { i32, [2 x i32] }

@g = addrspace(2) constant [4 x i32] [i32 0, i32 1, i32 2, i32 3], align 4
@struct_cl.hop = internal addrspace(4) unnamed_addr global %struct.my_struct zeroinitializer, align 4
@struct_cl.array = internal addrspace(4) global [256 x %struct.my_struct] zeroinitializer, align 4

define ptx_kernel void @struct_cl(%struct.my_struct* nocapture byval %s, i32 %x, i32 addrspace(1)* nocapture %mem, i32 %y) nounwind noinline {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %i.023 = phi i32 [ 0, %entry ], [ %add, %for.body ]
  %a = getelementptr inbounds [256 x %struct.my_struct] addrspace(4)* @struct_cl.array, i32 0, i32 %i.023, i32 0
  store i32 %i.023, i32 addrspace(4)* %a, align 4, !tbaa !1
  %arrayidx2 = getelementptr inbounds [256 x %struct.my_struct] addrspace(4)* @struct_cl.array, i32 0, i32 %i.023, i32 1, i32 0
  %add = add nsw i32 %i.023, 1
  store i32 %add, i32 addrspace(4)* %arrayidx2, align 4, !tbaa !1
  %exitcond = icmp eq i32 %add, 256
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  %cmp6 = icmp eq i32 %y, 0
  br i1 %cmp6, label %if.then, label %if.else

if.then:                                          ; preds = %for.end
  tail call void @llvm.memcpy.p4i8.p4i8.i32(i8 addrspace(4)* bitcast (%struct.my_struct addrspace(4)* @struct_cl.hop to i8 addrspace(4)*), i8 addrspace(4)* bitcast ([256 x %struct.my_struct] addrspace(4)* @struct_cl.array to i8 addrspace(4)*), i32 12, i32 4, i1 false)
  br label %if.end

if.else:                                          ; preds = %for.end
  %add8 = add nsw i32 %y, 1
  %arrayidx9 = getelementptr inbounds [256 x %struct.my_struct] addrspace(4)* @struct_cl.array, i32 0, i32 %add8
  %0 = bitcast %struct.my_struct addrspace(4)* %arrayidx9 to i8 addrspace(4)*
  tail call void @llvm.memcpy.p4i8.p4i8.i32(i8 addrspace(4)* bitcast (%struct.my_struct addrspace(4)* @struct_cl.hop to i8 addrspace(4)*), i8 addrspace(4)* %0, i32 12, i32 4, i1 false)
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  tail call void @llvm.memcpy.p4i8.p4i8.i32(i8 addrspace(4)* bitcast ([256 x %struct.my_struct] addrspace(4)* @struct_cl.array to i8 addrspace(4)*), i8 addrspace(4)* bitcast (%struct.my_struct addrspace(4)* @struct_cl.hop to i8 addrspace(4)*), i32 12, i32 4, i1 false)
  %a10 = getelementptr inbounds %struct.my_struct* %s, i32 0, i32 0
  %1 = load i32* %a10, align 4, !tbaa !1
  %a12 = getelementptr inbounds [256 x %struct.my_struct] addrspace(4)* @struct_cl.array, i32 0, i32 %x, i32 0
  %2 = load i32 addrspace(4)* %a12, align 4, !tbaa !1
  %add14 = add nsw i32 %x, 1
  %arrayidx17 = getelementptr inbounds [256 x %struct.my_struct] addrspace(4)* @struct_cl.array, i32 0, i32 %add14, i32 1, i32 0
  %3 = load i32 addrspace(4)* %arrayidx17, align 4, !tbaa !1
  %arrayidx19 = getelementptr inbounds [4 x i32] addrspace(2)* @g, i32 0, i32 %x
  %4 = load i32 addrspace(2)* %arrayidx19, align 4, !tbaa !1
  %add13 = add i32 %1, 3
  %add18 = add i32 %add13, %2
  %add20 = add i32 %add18, %3
  %add21 = add i32 %add20, %4
  store i32 %add21, i32 addrspace(1)* %mem, align 4, !tbaa !1
  ret void
}

declare void @llvm.memcpy.p4i8.p4i8.i32(i8 addrspace(4)* nocapture, i8 addrspace(4)* nocapture, i32, i32, i1) nounwind

!opencl.kernels = !{!0}

!0 = metadata !{void (%struct.my_struct*, i32, i32 addrspace(1)*, i32)* @struct_cl}
!1 = metadata !{metadata !"int", metadata !2}
!2 = metadata !{metadata !"omnipotent char", metadata !3}
!3 = metadata !{metadata !"Simple C/C++ TBAA", null}
