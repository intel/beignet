; ModuleID = 'store.o'
target datalayout = "e-p:32:32-i64:64:64-f64:64:64-n1:8:16:32:64"
target triple = "ptx32--"

define ptx_kernel void @store(i32* nocapture %dst, i32 addrspace(4)* nocapture %dst0, i32 %x) nounwind noinline {
entry:
  store i32 1, i32* %dst, align 4, !tbaa !1
  ret void
}

!opencl.kernels = !{!0}

!0 = metadata !{void (i32*, i32 addrspace(4)*, i32)* @store}
!1 = metadata !{metadata !"int", metadata !2}
!2 = metadata !{metadata !"omnipotent char", metadata !3}
!3 = metadata !{metadata !"Simple C/C++ TBAA", null}
