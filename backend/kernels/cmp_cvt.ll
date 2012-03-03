; ModuleID = 'cmp_cvt.o'
target datalayout = "e-p:32:32-i64:64:64-f64:64:64-n1:8:16:32:64"
target triple = "ptx32--"

define ptx_kernel void @cmp_cvt(i32* nocapture %dst, i32 %x, i32 %y) nounwind noinline {
get_local_id.exit:
  %add = add nsw i32 %y, %x
  %call.i = tail call ptx_device i32 @__gen_ocl_get_local_id0() nounwind readnone
  %cmp = icmp ult i32 %add, %call.i
  %conv = zext i1 %cmp to i32
  store i32 %conv, i32* %dst, align 4, !tbaa !1
  ret void
}

declare ptx_device i32 @__gen_ocl_get_local_id0() nounwind readnone

!opencl.kernels = !{!0}

!0 = metadata !{void (i32*, i32, i32)* @cmp_cvt}
!1 = metadata !{metadata !"int", metadata !2}
!2 = metadata !{metadata !"omnipotent char", metadata !3}
!3 = metadata !{metadata !"Simple C/C++ TBAA", null}
