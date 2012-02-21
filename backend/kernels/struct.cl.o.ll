; ModuleID = 'struct.cl.o'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @struct_cl(i64 %s.coerce0, i32 %s.coerce1) nounwind uwtable readnone {
entry:
  ret void
}

!opencl.kernels = !{!0}

!0 = metadata !{void (i64, i32)* @struct_cl}
