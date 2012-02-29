; ModuleID = 'load_store.o'
target datalayout = "e-p:32:32-i64:64:64-f64:64:64-n1:8:16:32:64"
target triple = "ptx32--"

define ptx_kernel void @load_store(i32 addrspace(4)* nocapture %dst, i32 addrspace(4)* nocapture %src) nounwind noinline {
entry:
  %0 = load i32 addrspace(4)* %src, align 4, !tbaa !1
  store i32 %0, i32 addrspace(4)* %dst, align 4, !tbaa !1
  ret void
}

!opencl.kernels = !{!0}

!0 = metadata !{void (i32 addrspace(4)*, i32 addrspace(4)*)* @load_store}
!1 = metadata !{metadata !"int", metadata !2}
!2 = metadata !{metadata !"omnipotent char", metadata !3}
!3 = metadata !{metadata !"Simple C/C++ TBAA", null}
