; ModuleID = 'function_param.o'
target datalayout = "e-p:32:32-i64:64:64-f64:64:64-n1:8:16:32:64"
target triple = "ptx32--"

%struct.struct0 = type { [5 x i32], i32, i32, i32 }

define ptx_kernel void @param(%struct.struct0 addrspace(1)* nocapture %dst, %struct.struct0* nocapture byval %s, i32 addrspace(4)* nocapture %h, i32 %x, i32 %y) nounwind noinline {
entry:
  %arrayidx = getelementptr inbounds i32 addrspace(4)* %h, i32 4
  %0 = load i32 addrspace(4)* %arrayidx, align 4, !tbaa !1
  %arrayidx1 = getelementptr inbounds %struct.struct0* %s, i32 0, i32 0, i32 4
  %1 = load i32* %arrayidx1, align 4, !tbaa !1
  %add = add i32 %0, %x
  %add2 = add i32 %add, %1
  store i32 %add2, i32* %arrayidx1, align 4, !tbaa !1
  %2 = bitcast %struct.struct0 addrspace(1)* %dst to i8 addrspace(1)*
  %3 = bitcast %struct.struct0* %s to i8*
  tail call void @llvm.memcpy.p1i8.p0i8.i32(i8 addrspace(1)* %2, i8* %3, i32 32, i32 4, i1 false)
  %y5 = getelementptr inbounds %struct.struct0 addrspace(1)* %dst, i32 0, i32 2
  %4 = load i32 addrspace(1)* %y5, align 4, !tbaa !1
  %add6 = add nsw i32 %4, %y
  store i32 %add6, i32 addrspace(1)* %y5, align 4, !tbaa !1
  ret void
}

declare void @llvm.memcpy.p1i8.p0i8.i32(i8 addrspace(1)* nocapture, i8* nocapture, i32, i32, i1) nounwind

!opencl.kernels = !{!0}

!0 = metadata !{void (%struct.struct0 addrspace(1)*, %struct.struct0*, i32 addrspace(4)*, i32, i32)* @param}
!1 = metadata !{metadata !"int", metadata !2}
!2 = metadata !{metadata !"omnipotent char", metadata !3}
!3 = metadata !{metadata !"Simple C/C++ TBAA", null}
