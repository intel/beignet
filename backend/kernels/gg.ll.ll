; ModuleID = 'gg.ll'
target datalayout = "e-p:32:32-i64:64:64-f64:64:64-n1:8:16:32:64"
target triple = "ptx32--"

%struct.my_struct = type { i32, [2 x i32] }

@g = addrspace(1) constant [4 x i32] [i32 0, i32 1, i32 2, i32 3], align 4
@struct_cl.array = internal addrspace(4) global [256 x %struct.my_struct] zeroinitializer, align 4

define ptx_kernel void @struct_cl(%struct.my_struct* byval %s, i32 %x, i32* %mem, i32 %y) nounwind noinline {
entry:
  %x.addr = alloca i32, align 4
  %mem.addr = alloca i32*, align 4
  %y.addr = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 %x, i32* %x.addr, align 4
  store i32* %mem, i32** %mem.addr, align 4
  store i32 %y, i32* %y.addr, align 4
  store i32 0, i32* %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i32* %i, align 4
  %cmp = icmp slt i32 %0, 256
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %1 = load i32* %i, align 4
  %2 = load i32* %i, align 4
  %arrayidx = getelementptr inbounds [256 x %struct.my_struct] addrspace(4)* @struct_cl.array, i32 0, i32 %2
  %a = getelementptr inbounds %struct.my_struct addrspace(4)* %arrayidx, i32 0, i32 0
  store i32 %1, i32 addrspace(4)* %a, align 4
  %3 = load i32* %i, align 4
  %4 = load i32* %i, align 4
  %arrayidx1 = getelementptr inbounds [256 x %struct.my_struct] addrspace(4)* @struct_cl.array, i32 0, i32 %4
  %b = getelementptr inbounds %struct.my_struct addrspace(4)* %arrayidx1, i32 0, i32 1
  %arrayidx2 = getelementptr inbounds [2 x i32] addrspace(4)* %b, i32 0, i32 0
  store i32 %3, i32 addrspace(4)* %arrayidx2, align 4
  %5 = load i32* %i, align 4
  %add = add nsw i32 %5, 1
  %6 = load i32* %i, align 4
  %arrayidx3 = getelementptr inbounds [256 x %struct.my_struct] addrspace(4)* @struct_cl.array, i32 0, i32 %6
  %b4 = getelementptr inbounds %struct.my_struct addrspace(4)* %arrayidx3, i32 0, i32 1
  %arrayidx5 = getelementptr inbounds [2 x i32] addrspace(4)* %b4, i32 0, i32 0
  store i32 %add, i32 addrspace(4)* %arrayidx5, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %7 = load i32* %i, align 4
  %inc = add nsw i32 %7, 1
  store i32 %inc, i32* %i, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %8 = load i32* %y.addr, align 4
  %arrayidx6 = getelementptr inbounds [256 x %struct.my_struct] addrspace(4)* @struct_cl.array, i32 0, i32 %8
  %9 = bitcast %struct.my_struct addrspace(4)* %arrayidx6 to i8 addrspace(4)*
  call void @llvm.memcpy.p4i8.p4i8.i32(i8 addrspace(4)* bitcast ([256 x %struct.my_struct] addrspace(4)* @struct_cl.array to i8 addrspace(4)*), i8 addrspace(4)* %9, i32 12, i32 4, i1 false)
  %a7 = getelementptr inbounds %struct.my_struct* %s, i32 0, i32 0
  %10 = load i32* %a7, align 4
  %11 = load i32* %x.addr, align 4
  %arrayidx8 = getelementptr inbounds [256 x %struct.my_struct] addrspace(4)* @struct_cl.array, i32 0, i32 %11
  %a9 = getelementptr inbounds %struct.my_struct addrspace(4)* %arrayidx8, i32 0, i32 0
  %12 = load i32 addrspace(4)* %a9, align 4
  %add10 = add nsw i32 %10, %12
  %13 = load i32* %x.addr, align 4
  %add11 = add nsw i32 %13, 1
  %arrayidx12 = getelementptr inbounds [256 x %struct.my_struct] addrspace(4)* @struct_cl.array, i32 0, i32 %add11
  %b13 = getelementptr inbounds %struct.my_struct addrspace(4)* %arrayidx12, i32 0, i32 1
  %arrayidx14 = getelementptr inbounds [2 x i32] addrspace(4)* %b13, i32 0, i32 0
  %14 = load i32 addrspace(4)* %arrayidx14, align 4
  %add15 = add nsw i32 %add10, %14
  %15 = load i32* %x.addr, align 4
  %arrayidx16 = getelementptr inbounds [4 x i32] addrspace(1)* @g, i32 0, i32 %15
  %16 = load i32 addrspace(1)* %arrayidx16, align 4
  %add17 = add nsw i32 %add15, %16
  %17 = load i32 addrspace(1)* getelementptr inbounds ([4 x i32] addrspace(1)* @g, i32 0, i32 3), align 4
  %add18 = add nsw i32 %add17, %17
  %18 = load i32** %mem.addr, align 4
  %arrayidx19 = getelementptr inbounds i32* %18, i32 0
  store i32 %add18, i32* %arrayidx19
  ret void
}

declare void @llvm.memcpy.p4i8.p4i8.i32(i8 addrspace(4)* nocapture, i8 addrspace(4)* nocapture, i32, i32, i1) nounwind

!opencl.kernels = !{!0}

!0 = metadata !{void (%struct.my_struct*, i32, i32*, i32)* @struct_cl}
