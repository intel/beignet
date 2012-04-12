; ModuleID = 'add2.cl.o'
target datalayout = "e-p:32:32-i64:64:64-f64:64:64-n1:8:16:32:64"
target triple = "ptx32--"

%struct.big = type { i32, i32 }

define ptx_kernel void @add(%struct.big addrspace(1)* nocapture %b, i32 %x, i32 %y) nounwind noinline {
entry:
  %add = add i32 %y, %x
  %a = getelementptr inbounds %struct.big addrspace(1)* %b, i32 0, i32 0
  store i32 %add, i32 addrspace(1)* %a, align 4, !tbaa !1
  %sub = add i32 %x, 10
  %add1 = sub i32 %sub, %y
  %b2 = getelementptr inbounds %struct.big addrspace(1)* %b, i32 0, i32 1
  store i32 %add1, i32 addrspace(1)* %b2, align 4, !tbaa !1
  ret void
}

!opencl.kernels = !{!0}

!0 = metadata !{void (%struct.big addrspace(1)*, i32, i32)* @add}
!1 = metadata !{metadata !"int", metadata !2}
!2 = metadata !{metadata !"omnipotent char", metadata !3}
!3 = metadata !{metadata !"Simple C/C++ TBAA", null}
