; ModuleID = 'add.o'
target datalayout = "e-p:32:32-i64:64:64-f64:64:64-n1:8:16:32:64"
target triple = "ptx32--"

define ptx_kernel i32 @add(i32 %x, i32 %y) nounwind readnone noinline {
entry:
  %add = add i32 %y, %x
  ret i32 %add
}

!opencl.kernels = !{!0}

!0 = metadata !{i32 (i32, i32)* @add}
