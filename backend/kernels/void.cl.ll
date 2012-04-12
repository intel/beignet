; ModuleID = 'void.cl.o'
target datalayout = "e-p:32:32-i64:64:64-f64:64:64-n1:8:16:32:64"
target triple = "ptx32--"

define ptx_kernel void @hop() nounwind readnone noinline {
entry:
  ret void
}

!opencl.kernels = !{!0}

!0 = metadata !{void ()* @hop}
