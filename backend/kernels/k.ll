; ModuleID = 'k'
target datalayout = "e-p:32:32-i64:64:64-f64:64:64-n1:8:16:32:64"
target triple = "ptx32--"

%struct.my_struct = type { i32, [2 x i32] }

define ptx_device void @struct_cl(%struct.my_struct* nocapture byval %s) nounwind readnone {
entry:
  ret void
}
