; ModuleID = 'function_param.cl.o'
target datalayout = "e-p:32:32-i64:64:64-f64:64:64-n1:8:16:32:64"
target triple = "ptx32--"

%struct.struct0 = type { [5 x i32], i32, i32, i32 }

define ptx_device <2 x float> @_Z3madDv2_fS_S_(<2 x float> %a, <2 x float> %b, <2 x float> %c) nounwind readnone {
entry:
  %0 = extractelement <2 x float> %a, i32 0
  %1 = extractelement <2 x float> %b, i32 0
  %2 = extractelement <2 x float> %c, i32 0
  %call = tail call ptx_device float @_Z3madfff(float %0, float %1, float %2) nounwind readnone
  %vecinit = insertelement <2 x float> undef, float %call, i32 0
  %3 = extractelement <2 x float> %a, i32 1
  %4 = extractelement <2 x float> %b, i32 1
  %5 = extractelement <2 x float> %c, i32 1
  %call1 = tail call ptx_device float @_Z3madfff(float %3, float %4, float %5) nounwind readnone
  %vecinit2 = insertelement <2 x float> %vecinit, float %call1, i32 1
  ret <2 x float> %vecinit2
}

declare ptx_device float @_Z3madfff(float, float, float) nounwind readnone

define ptx_device <3 x float> @_Z3madDv3_fS_S_(<3 x float> %a, <3 x float> %b, <3 x float> %c) nounwind readnone {
entry:
  %0 = extractelement <3 x float> %a, i32 0
  %1 = extractelement <3 x float> %b, i32 0
  %2 = extractelement <3 x float> %c, i32 0
  %call = tail call ptx_device float @_Z3madfff(float %0, float %1, float %2) nounwind readnone
  %vecinit = insertelement <3 x float> undef, float %call, i32 0
  %3 = extractelement <3 x float> %a, i32 1
  %4 = extractelement <3 x float> %b, i32 1
  %5 = extractelement <3 x float> %c, i32 1
  %call1 = tail call ptx_device float @_Z3madfff(float %3, float %4, float %5) nounwind readnone
  %vecinit2 = insertelement <3 x float> %vecinit, float %call1, i32 1
  %6 = extractelement <3 x float> %a, i32 2
  %7 = extractelement <3 x float> %b, i32 2
  %8 = extractelement <3 x float> %c, i32 2
  %call3 = tail call ptx_device float @_Z3madfff(float %6, float %7, float %8) nounwind readnone
  %vecinit4 = insertelement <3 x float> %vecinit2, float %call3, i32 2
  ret <3 x float> %vecinit4
}

define ptx_device <4 x float> @_Z3madDv4_fS_S_(<4 x float> %a, <4 x float> %b, <4 x float> %c) nounwind readnone {
entry:
  %0 = extractelement <4 x float> %a, i32 0
  %1 = extractelement <4 x float> %b, i32 0
  %2 = extractelement <4 x float> %c, i32 0
  %call = tail call ptx_device float @_Z3madfff(float %0, float %1, float %2) nounwind readnone
  %vecinit = insertelement <4 x float> undef, float %call, i32 0
  %3 = extractelement <4 x float> %a, i32 1
  %4 = extractelement <4 x float> %b, i32 1
  %5 = extractelement <4 x float> %c, i32 1
  %call1 = tail call ptx_device float @_Z3madfff(float %3, float %4, float %5) nounwind readnone
  %vecinit2 = insertelement <4 x float> %vecinit, float %call1, i32 1
  %6 = extractelement <4 x float> %a, i32 2
  %7 = extractelement <4 x float> %b, i32 2
  %8 = extractelement <4 x float> %c, i32 2
  %call3 = tail call ptx_device float @_Z3madfff(float %6, float %7, float %8) nounwind readnone
  %vecinit4 = insertelement <4 x float> %vecinit2, float %call3, i32 2
  %9 = extractelement <4 x float> %a, i32 3
  %10 = extractelement <4 x float> %b, i32 3
  %11 = extractelement <4 x float> %c, i32 3
  %call5 = tail call ptx_device float @_Z3madfff(float %9, float %10, float %11) nounwind readnone
  %vecinit6 = insertelement <4 x float> %vecinit4, float %call5, i32 3
  ret <4 x float> %vecinit6
}

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
