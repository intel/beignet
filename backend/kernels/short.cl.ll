; ModuleID = 'short.cl.o'
target datalayout = "e-p:32:32-i64:64:64-f64:64:64-n1:8:16:32:64"
target triple = "ptx32--"

define ptx_kernel void @short_write(i16 addrspace(1)* nocapture %dst, i16 %x, i16 %y) nounwind noinline {
entry:
  %add = add i16 %y, %x
  store i16 %add, i16 addrspace(1)* %dst, align 2, !tbaa !1
  ret void
}

!opencl.kernels = !{!0}

!0 = metadata !{void (i16 addrspace(1)*, i16, i16)* @short_write}
!1 = metadata !{metadata !"short", metadata !2}
!2 = metadata !{metadata !"omnipotent char", metadata !3}
!3 = metadata !{metadata !"Simple C/C++ TBAA", null}
