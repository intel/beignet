; ModuleID = 'struct2.cl.o'
target datalayout = "e-p:32:32-i64:64:64-f64:64:64-n1:8:16:32:64"
target triple = "ptx32--"

%struct.my_struct = type { i32, [2 x i32] }

@g = addrspace(2) constant [4 x i32] [i32 0, i32 1, i32 2, i32 3], align 4

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

define ptx_kernel void @struct_cl(%struct.my_struct* nocapture byval %s, i32 %x, %struct.my_struct addrspace(1)* nocapture %mem, i32 %y) nounwind noinline {
entry:
  %cmp = icmp eq i32 %y, 0
  br i1 %cmp, label %if.end, label %if.else

if.else:                                          ; preds = %entry
  %s.0 = getelementptr inbounds %struct.my_struct* %s, i32 0, i32 0
  %tmp4 = load i32* %s.0, align 4
  %s.1.0 = getelementptr inbounds %struct.my_struct* %s, i32 0, i32 1, i32 0
  %tmp5 = load i32* %s.1.0, align 4
  %s.1.1 = getelementptr inbounds %struct.my_struct* %s, i32 0, i32 1, i32 1
  %tmp6 = load i32* %s.1.1, align 4
  br label %if.end

if.end:                                           ; preds = %if.else, %entry
  %hop.1.1.0 = phi i32 [ %tmp6, %if.else ], [ 2, %entry ]
  %hop.1.0.0 = phi i32 [ %tmp5, %if.else ], [ 2, %entry ]
  %hop.0.0 = phi i32 [ %tmp4, %if.else ], [ 1, %entry ]
  %mem.0 = getelementptr inbounds %struct.my_struct addrspace(1)* %mem, i32 0, i32 0
  store i32 %hop.0.0, i32 addrspace(1)* %mem.0, align 4
  %mem.1.0 = getelementptr inbounds %struct.my_struct addrspace(1)* %mem, i32 0, i32 1, i32 0
  store i32 %hop.1.0.0, i32 addrspace(1)* %mem.1.0, align 4
  %mem.1.1 = getelementptr inbounds %struct.my_struct addrspace(1)* %mem, i32 0, i32 1, i32 1
  store i32 %hop.1.1.0, i32 addrspace(1)* %mem.1.1, align 4
  ret void
}

!opencl.kernels = !{!0}

!0 = metadata !{void (%struct.my_struct*, i32, %struct.my_struct addrspace(1)*, i32)* @struct_cl}
