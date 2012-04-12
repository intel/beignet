; ModuleID = 'cycle.cl.o'
target datalayout = "e-p:32:32-i64:64:64-f64:64:64-n1:8:16:32:64"
target triple = "ptx32--"

define ptx_kernel void @cycle(i32 addrspace(1)* nocapture %dst) noreturn nounwind readnone noinline {
entry:
  br label %hop0

hop0:                                             ; preds = %hop0, %entry
  br label %hop0
}

!opencl.kernels = !{!0}

!0 = metadata !{void (i32 addrspace(1)*)* @cycle}
