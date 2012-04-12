; ModuleID = 'complex_struct.cl.o'
target datalayout = "e-p:32:32-i64:64:64-f64:64:64-n1:8:16:32:64"
target triple = "ptx32--"

%struct.my_struct = type { i32, [5 x %struct.hop] }
%struct.hop = type { float, float }

define ptx_kernel void @struct_cl(%struct.my_struct addrspace(1)* nocapture %dst, %struct.my_struct addrspace(1)* nocapture %src) nounwind noinline {
entry:
  %x = getelementptr inbounds %struct.my_struct addrspace(1)* %src, i32 1, i32 1, i32 3, i32 0
  %0 = load float addrspace(1)* %x, align 4, !tbaa !1
  %y = getelementptr inbounds %struct.my_struct addrspace(1)* %dst, i32 0, i32 1, i32 2, i32 1
  store float %0, float addrspace(1)* %y, align 4, !tbaa !1
  ret void
}

!opencl.kernels = !{!0}

!0 = metadata !{void (%struct.my_struct addrspace(1)*, %struct.my_struct addrspace(1)*)* @struct_cl}
!1 = metadata !{metadata !"float", metadata !2}
!2 = metadata !{metadata !"omnipotent char", metadata !3}
!3 = metadata !{metadata !"Simple C/C++ TBAA", null}
