; ModuleID = 'struct2.o'
target datalayout = "e-p:32:32-i64:64:64-f64:64:64-n1:8:16:32:64"
target triple = "ptx32--"

%struct.my_struct = type { i32, [2 x i32] }

@g = addrspace(1) constant [4 x i32] [i32 0, i32 1, i32 2, i32 3], align 4

define ptx_kernel void @struct_cl(%struct.my_struct* nocapture byval %s, i32 %x, %struct.my_struct* nocapture %mem, i32 %y) nounwind noinline {
entry:
  %cmp = icmp eq i32 %y, 0
  br i1 %cmp, label %if.end, label %if.else

if.else:                                          ; preds = %entry
  %s.0 = getelementptr inbounds %struct.my_struct* %s, i32 0, i32 0
  %tmp4 = load i32* %s.0, align 4
  %s.1.0 = getelementptr inbounds %struct.my_struct* %s, i32 0, i32 1, i32 0
  %tmp5 = load i32* %s.1.0, align 4
  %s.1.1 = getelementptr inbounds %struct.my_struct* %s, i32 0, i32 1, i32 1
  %tmp6 = load i32* %s.1.1, align 4
  br label %if.end

if.end:                                           ; preds = %if.else, %entry
  %hop.1.1.0 = phi i32 [ %tmp6, %if.else ], [ 2, %entry ]
  %hop.1.0.0 = phi i32 [ %tmp5, %if.else ], [ 2, %entry ]
  %hop.0.0 = phi i32 [ %tmp4, %if.else ], [ 1, %entry ]
  %mem.0 = getelementptr inbounds %struct.my_struct* %mem, i32 0, i32 0
  store i32 %hop.0.0, i32* %mem.0, align 4
  %mem.1.0 = getelementptr inbounds %struct.my_struct* %mem, i32 0, i32 1, i32 0
  store i32 %hop.1.0.0, i32* %mem.1.0, align 4
  %mem.1.1 = getelementptr inbounds %struct.my_struct* %mem, i32 0, i32 1, i32 1
  store i32 %hop.1.1.0, i32* %mem.1.1, align 4
  ret void
}

!opencl.kernels = !{!0}

!0 = metadata !{void (%struct.my_struct*, i32, %struct.my_struct*, i32)* @struct_cl}
