; ModuleID = 'add2.o'
target datalayout = "e-p:32:32-i64:64:64-f64:64:64-n1:8:16:32:64"
target triple = "ptx32--"

%struct.big = type { i32, i32 }

define ptx_kernel void @add(%struct.big* noalias nocapture sret %agg.result, i32 %x, i32 %y) nounwind noinline {
entry:
  %add = add i32 %y, %x
  %sub = sub i32 %x, %y
  %agg.result.0 = getelementptr inbounds %struct.big* %agg.result, i32 0, i32 0
  store i32 %add, i32* %agg.result.0, align 4
  %agg.result.1 = getelementptr inbounds %struct.big* %agg.result, i32 0, i32 1
  store i32 %sub, i32* %agg.result.1, align 4
  ret void
}

!opencl.kernels = !{!0}

!0 = metadata !{void (%struct.big*, i32, i32)* @add}
