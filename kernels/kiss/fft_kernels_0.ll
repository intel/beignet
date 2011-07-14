; ModuleID = '-'
target datalayout = "E-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-n32"
target triple = "GHAL3D_2_1"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_kfft_local_lds = internal addrspace(3) global [2176 x float] zeroinitializer, align 4 ; <[2176 x float] addrspace(3)*> [#uses=2]
@opencl_kfft_locals = appending global [2 x i8*] [i8* bitcast ([2176 x float] addrspace(3)* @opencl_kfft_local_lds to i8*), i8* null], section "llvm.metadata" ; <[2 x i8*]*> [#uses=1]
@opencl_kfft_parameters = appending global [85 x i8] c"float __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *\00", section "llvm.metadata" ; <[85 x i8]*> [#uses=1]
@opencl_metadata = appending global [1 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (float addrspace(1)*, float addrspace(1)*)* @kfft to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([2 x i8*]* @opencl_kfft_locals to i8*), i8* getelementptr inbounds ([85 x i8]* @opencl_kfft_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[1 x %opencl_metadata_type]*> [#uses=0]

define float @k_sincos(i32 %i, float* %cretp) nounwind alwaysinline {
  %1 = alloca float, align 4                      ; <float*> [#uses=2]
  %2 = alloca i32, align 4                        ; <i32*> [#uses=5]
  %3 = alloca float*, align 4                     ; <float**> [#uses=2]
  %x = alloca float, align 4                      ; <float*> [#uses=3]
  store i32 %i, i32* %2
  store float* %cretp, float** %3
  %4 = load i32* %2                               ; <i32> [#uses=1]
  %5 = icmp sgt i32 %4, 512                       ; <i1> [#uses=1]
  br i1 %5, label %6, label %9

; <label>:6                                       ; preds = %0
  %7 = load i32* %2                               ; <i32> [#uses=1]
  %8 = sub i32 %7, 1024                           ; <i32> [#uses=1]
  store i32 %8, i32* %2
  br label %9

; <label>:9                                       ; preds = %6, %0
  %10 = load i32* %2                              ; <i32> [#uses=1]
  %11 = sitofp i32 %10 to float                   ; <float> [#uses=1]
  %12 = fmul float %11, 0xBF7921FB60000000        ; <float> [#uses=1]
  store float %12, float* %x
  %13 = load float* %x                            ; <float> [#uses=1]
  %14 = call float @_Z10native_cosf(float %13)    ; <float> [#uses=1]
  %15 = load float** %3                           ; <float*> [#uses=1]
  store float %14, float* %15
  %16 = load float* %x                            ; <float> [#uses=1]
  %17 = call float @_Z10native_sinf(float %16)    ; <float> [#uses=1]
  store float %17, float* %1
  %18 = load float* %1                            ; <float> [#uses=1]
  ret float %18
}

declare float @_Z10native_cosf(float)

declare float @_Z10native_sinf(float)

define <4 x float> @k_sincos4(<4 x i32> %i, <4 x float>* %cretp) nounwind alwaysinline {
  %1 = alloca <4 x float>, align 16               ; <<4 x float>*> [#uses=2]
  %2 = alloca <4 x i32>, align 16                 ; <<4 x i32>*> [#uses=5]
  %3 = alloca <4 x float>*, align 4               ; <<4 x float>**> [#uses=2]
  %x = alloca <4 x float>, align 16               ; <<4 x float>*> [#uses=3]
  store <4 x i32> %i, <4 x i32>* %2
  store <4 x float>* %cretp, <4 x float>** %3
  %4 = load <4 x i32>* %2                         ; <<4 x i32>> [#uses=1]
  %5 = icmp sgt <4 x i32> %4, <i32 512, i32 512, i32 512, i32 512> ; <<4 x i1>> [#uses=1]
  %6 = sext <4 x i1> %5 to <4 x i32>              ; <<4 x i32>> [#uses=1]
  %7 = and <4 x i32> %6, <i32 1024, i32 1024, i32 1024, i32 1024> ; <<4 x i32>> [#uses=1]
  %8 = load <4 x i32>* %2                         ; <<4 x i32>> [#uses=1]
  %9 = sub <4 x i32> %8, %7                       ; <<4 x i32>> [#uses=1]
  store <4 x i32> %9, <4 x i32>* %2
  %10 = load <4 x i32>* %2                        ; <<4 x i32>> [#uses=1]
  %11 = call <4 x float> @_Z14convert_float4U8__vector4i(<4 x i32> %10) ; <<4 x float>> [#uses=1]
  %12 = fmul <4 x float> %11, <float 0xBF7921FB60000000, float 0xBF7921FB60000000, float 0xBF7921FB60000000, float 0xBF7921FB60000000> ; <<4 x float>> [#uses=1]
  store <4 x float> %12, <4 x float>* %x
  %13 = load <4 x float>* %x                      ; <<4 x float>> [#uses=1]
  %14 = call <4 x float> @_Z10native_cosU8__vector4f(<4 x float> %13) ; <<4 x float>> [#uses=1]
  %15 = load <4 x float>** %3                     ; <<4 x float>*> [#uses=1]
  store <4 x float> %14, <4 x float>* %15
  %16 = load <4 x float>* %x                      ; <<4 x float>> [#uses=1]
  %17 = call <4 x float> @_Z10native_sinU8__vector4f(<4 x float> %16) ; <<4 x float>> [#uses=1]
  store <4 x float> %17, <4 x float>* %1
  %18 = load <4 x float>* %1                      ; <<4 x float>> [#uses=1]
  ret <4 x float> %18
}

declare <4 x float> @_Z14convert_float4U8__vector4i(<4 x i32>)

declare <4 x float> @_Z10native_cosU8__vector4f(<4 x float>)

declare <4 x float> @_Z10native_sinU8__vector4f(<4 x float>)

define void @kfft_pass1(i32 %me, float addrspace(1)* %gr, float addrspace(1)* %gi, float addrspace(3)* %lds) nounwind alwaysinline {
  %1 = alloca <4 x float>, align 16               ; <<4 x float>*> [#uses=2]
  %2 = alloca <4 x i32>, align 16                 ; <<4 x i32>*> [#uses=5]
  %3 = alloca <4 x float>*, align 4               ; <<4 x float>**> [#uses=2]
  %x.i4 = alloca <4 x float>, align 16            ; <<4 x float>*> [#uses=3]
  %4 = alloca <4 x float>, align 16               ; <<4 x float>*> [#uses=2]
  %5 = alloca <4 x i32>, align 16                 ; <<4 x i32>*> [#uses=5]
  %6 = alloca <4 x float>*, align 4               ; <<4 x float>**> [#uses=2]
  %x.i3 = alloca <4 x float>, align 16            ; <<4 x float>*> [#uses=3]
  %7 = alloca <4 x float>, align 16               ; <<4 x float>*> [#uses=2]
  %8 = alloca <4 x i32>, align 16                 ; <<4 x i32>*> [#uses=5]
  %9 = alloca <4 x float>*, align 4               ; <<4 x float>**> [#uses=2]
  %x.i = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %10 = alloca i32, align 4                       ; <i32*> [#uses=6]
  %11 = alloca float addrspace(1)*, align 16      ; <float addrspace(1)**> [#uses=2]
  %12 = alloca float addrspace(1)*, align 16      ; <float addrspace(1)**> [#uses=2]
  %13 = alloca float addrspace(3)*, align 16      ; <float addrspace(3)**> [#uses=2]
  %gp = alloca <4 x float> addrspace(1)*, align 16 ; <<4 x float> addrspace(1)**> [#uses=10]
  %lp = alloca float addrspace(3)*, align 16      ; <float addrspace(3)**> [#uses=47]
  %zr0 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=8]
  %zr1 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=11]
  %zr2 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=11]
  %zr3 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=11]
  %zi0 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=8]
  %zi1 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=11]
  %zi2 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=11]
  %zi3 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=11]
  %ar0 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %ar2 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %br0 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %br1 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %br2 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %br3 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %ai0 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %ai2 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %bi0 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %bi1 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %bi2 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %bi3 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %tbase = alloca <4 x i32>, align 16             ; <<4 x i32>*> [#uses=4]
  %14 = alloca <4 x i32>, align 16                ; <<4 x i32>*> [#uses=2]
  %c1 = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=3]
  %s1 = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=3]
  %__r = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %c2 = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=3]
  %s2 = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=3]
  %__r1 = alloca <4 x float>, align 16            ; <<4 x float>*> [#uses=2]
  %c3 = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=3]
  %s3 = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=3]
  %__r2 = alloca <4 x float>, align 16            ; <<4 x float>*> [#uses=2]
  store i32 %me, i32* %10
  store float addrspace(1)* %gr, float addrspace(1)** %11
  store float addrspace(1)* %gi, float addrspace(1)** %12
  store float addrspace(3)* %lds, float addrspace(3)** %13
  %15 = load float addrspace(1)** %11             ; <float addrspace(1)*> [#uses=1]
  %16 = load i32* %10                             ; <i32> [#uses=1]
  %17 = shl i32 %16, 2                            ; <i32> [#uses=1]
  %18 = getelementptr inbounds float addrspace(1)* %15, i32 %17 ; <float addrspace(1)*> [#uses=1]
  %19 = bitcast float addrspace(1)* %18 to <4 x float> addrspace(1)* ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> addrspace(1)* %19, <4 x float> addrspace(1)** %gp
  %20 = load <4 x float> addrspace(1)** %gp       ; <<4 x float> addrspace(1)*> [#uses=1]
  %21 = getelementptr inbounds <4 x float> addrspace(1)* %20, i32 0 ; <<4 x float> addrspace(1)*> [#uses=1]
  %22 = load <4 x float> addrspace(1)* %21        ; <<4 x float>> [#uses=1]
  store <4 x float> %22, <4 x float>* %zr0
  %23 = load <4 x float> addrspace(1)** %gp       ; <<4 x float> addrspace(1)*> [#uses=1]
  %24 = getelementptr inbounds <4 x float> addrspace(1)* %23, i32 64 ; <<4 x float> addrspace(1)*> [#uses=1]
  %25 = load <4 x float> addrspace(1)* %24        ; <<4 x float>> [#uses=1]
  store <4 x float> %25, <4 x float>* %zr1
  %26 = load <4 x float> addrspace(1)** %gp       ; <<4 x float> addrspace(1)*> [#uses=1]
  %27 = getelementptr inbounds <4 x float> addrspace(1)* %26, i32 128 ; <<4 x float> addrspace(1)*> [#uses=1]
  %28 = load <4 x float> addrspace(1)* %27        ; <<4 x float>> [#uses=1]
  store <4 x float> %28, <4 x float>* %zr2
  %29 = load <4 x float> addrspace(1)** %gp       ; <<4 x float> addrspace(1)*> [#uses=1]
  %30 = getelementptr inbounds <4 x float> addrspace(1)* %29, i32 192 ; <<4 x float> addrspace(1)*> [#uses=1]
  %31 = load <4 x float> addrspace(1)* %30        ; <<4 x float>> [#uses=1]
  store <4 x float> %31, <4 x float>* %zr3
  %32 = load float addrspace(1)** %12             ; <float addrspace(1)*> [#uses=1]
  %33 = load i32* %10                             ; <i32> [#uses=1]
  %34 = shl i32 %33, 2                            ; <i32> [#uses=1]
  %35 = getelementptr inbounds float addrspace(1)* %32, i32 %34 ; <float addrspace(1)*> [#uses=1]
  %36 = bitcast float addrspace(1)* %35 to <4 x float> addrspace(1)* ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> addrspace(1)* %36, <4 x float> addrspace(1)** %gp
  %37 = load <4 x float> addrspace(1)** %gp       ; <<4 x float> addrspace(1)*> [#uses=1]
  %38 = getelementptr inbounds <4 x float> addrspace(1)* %37, i32 0 ; <<4 x float> addrspace(1)*> [#uses=1]
  %39 = load <4 x float> addrspace(1)* %38        ; <<4 x float>> [#uses=1]
  store <4 x float> %39, <4 x float>* %zi0
  %40 = load <4 x float> addrspace(1)** %gp       ; <<4 x float> addrspace(1)*> [#uses=1]
  %41 = getelementptr inbounds <4 x float> addrspace(1)* %40, i32 64 ; <<4 x float> addrspace(1)*> [#uses=1]
  %42 = load <4 x float> addrspace(1)* %41        ; <<4 x float>> [#uses=1]
  store <4 x float> %42, <4 x float>* %zi1
  %43 = load <4 x float> addrspace(1)** %gp       ; <<4 x float> addrspace(1)*> [#uses=1]
  %44 = getelementptr inbounds <4 x float> addrspace(1)* %43, i32 128 ; <<4 x float> addrspace(1)*> [#uses=1]
  %45 = load <4 x float> addrspace(1)* %44        ; <<4 x float>> [#uses=1]
  store <4 x float> %45, <4 x float>* %zi2
  %46 = load <4 x float> addrspace(1)** %gp       ; <<4 x float> addrspace(1)*> [#uses=1]
  %47 = getelementptr inbounds <4 x float> addrspace(1)* %46, i32 192 ; <<4 x float> addrspace(1)*> [#uses=1]
  %48 = load <4 x float> addrspace(1)* %47        ; <<4 x float>> [#uses=1]
  store <4 x float> %48, <4 x float>* %zi3
  br label %49

; <label>:49                                      ; preds = %0
  %50 = load <4 x float>* %zr0                    ; <<4 x float>> [#uses=1]
  %51 = load <4 x float>* %zr2                    ; <<4 x float>> [#uses=1]
  %52 = fadd <4 x float> %50, %51                 ; <<4 x float>> [#uses=1]
  store <4 x float> %52, <4 x float>* %ar0
  %53 = load <4 x float>* %zr1                    ; <<4 x float>> [#uses=1]
  %54 = load <4 x float>* %zr3                    ; <<4 x float>> [#uses=1]
  %55 = fadd <4 x float> %53, %54                 ; <<4 x float>> [#uses=1]
  store <4 x float> %55, <4 x float>* %ar2
  %56 = load <4 x float>* %ar0                    ; <<4 x float>> [#uses=1]
  %57 = load <4 x float>* %ar2                    ; <<4 x float>> [#uses=1]
  %58 = fadd <4 x float> %56, %57                 ; <<4 x float>> [#uses=1]
  store <4 x float> %58, <4 x float>* %br0
  %59 = load <4 x float>* %zr0                    ; <<4 x float>> [#uses=1]
  %60 = load <4 x float>* %zr2                    ; <<4 x float>> [#uses=1]
  %61 = fsub <4 x float> %59, %60                 ; <<4 x float>> [#uses=1]
  store <4 x float> %61, <4 x float>* %br1
  %62 = load <4 x float>* %ar0                    ; <<4 x float>> [#uses=1]
  %63 = load <4 x float>* %ar2                    ; <<4 x float>> [#uses=1]
  %64 = fsub <4 x float> %62, %63                 ; <<4 x float>> [#uses=1]
  store <4 x float> %64, <4 x float>* %br2
  %65 = load <4 x float>* %zr1                    ; <<4 x float>> [#uses=1]
  %66 = load <4 x float>* %zr3                    ; <<4 x float>> [#uses=1]
  %67 = fsub <4 x float> %65, %66                 ; <<4 x float>> [#uses=1]
  store <4 x float> %67, <4 x float>* %br3
  %68 = load <4 x float>* %zi0                    ; <<4 x float>> [#uses=1]
  %69 = load <4 x float>* %zi2                    ; <<4 x float>> [#uses=1]
  %70 = fadd <4 x float> %68, %69                 ; <<4 x float>> [#uses=1]
  store <4 x float> %70, <4 x float>* %ai0
  %71 = load <4 x float>* %zi1                    ; <<4 x float>> [#uses=1]
  %72 = load <4 x float>* %zi3                    ; <<4 x float>> [#uses=1]
  %73 = fadd <4 x float> %71, %72                 ; <<4 x float>> [#uses=1]
  store <4 x float> %73, <4 x float>* %ai2
  %74 = load <4 x float>* %ai0                    ; <<4 x float>> [#uses=1]
  %75 = load <4 x float>* %ai2                    ; <<4 x float>> [#uses=1]
  %76 = fadd <4 x float> %74, %75                 ; <<4 x float>> [#uses=1]
  store <4 x float> %76, <4 x float>* %bi0
  %77 = load <4 x float>* %zi0                    ; <<4 x float>> [#uses=1]
  %78 = load <4 x float>* %zi2                    ; <<4 x float>> [#uses=1]
  %79 = fsub <4 x float> %77, %78                 ; <<4 x float>> [#uses=1]
  store <4 x float> %79, <4 x float>* %bi1
  %80 = load <4 x float>* %ai0                    ; <<4 x float>> [#uses=1]
  %81 = load <4 x float>* %ai2                    ; <<4 x float>> [#uses=1]
  %82 = fsub <4 x float> %80, %81                 ; <<4 x float>> [#uses=1]
  store <4 x float> %82, <4 x float>* %bi2
  %83 = load <4 x float>* %zi1                    ; <<4 x float>> [#uses=1]
  %84 = load <4 x float>* %zi3                    ; <<4 x float>> [#uses=1]
  %85 = fsub <4 x float> %83, %84                 ; <<4 x float>> [#uses=1]
  store <4 x float> %85, <4 x float>* %bi3
  %86 = load <4 x float>* %br0                    ; <<4 x float>> [#uses=1]
  store <4 x float> %86, <4 x float>* %zr0
  %87 = load <4 x float>* %bi0                    ; <<4 x float>> [#uses=1]
  store <4 x float> %87, <4 x float>* %zi0
  %88 = load <4 x float>* %br1                    ; <<4 x float>> [#uses=1]
  %89 = load <4 x float>* %bi3                    ; <<4 x float>> [#uses=1]
  %90 = fadd <4 x float> %88, %89                 ; <<4 x float>> [#uses=1]
  store <4 x float> %90, <4 x float>* %zr1
  %91 = load <4 x float>* %bi1                    ; <<4 x float>> [#uses=1]
  %92 = load <4 x float>* %br3                    ; <<4 x float>> [#uses=1]
  %93 = fsub <4 x float> %91, %92                 ; <<4 x float>> [#uses=1]
  store <4 x float> %93, <4 x float>* %zi1
  %94 = load <4 x float>* %br1                    ; <<4 x float>> [#uses=1]
  %95 = load <4 x float>* %bi3                    ; <<4 x float>> [#uses=1]
  %96 = fsub <4 x float> %94, %95                 ; <<4 x float>> [#uses=1]
  store <4 x float> %96, <4 x float>* %zr3
  %97 = load <4 x float>* %br3                    ; <<4 x float>> [#uses=1]
  %98 = load <4 x float>* %bi1                    ; <<4 x float>> [#uses=1]
  %99 = fadd <4 x float> %97, %98                 ; <<4 x float>> [#uses=1]
  store <4 x float> %99, <4 x float>* %zi3
  %100 = load <4 x float>* %br2                   ; <<4 x float>> [#uses=1]
  store <4 x float> %100, <4 x float>* %zr2
  %101 = load <4 x float>* %bi2                   ; <<4 x float>> [#uses=1]
  store <4 x float> %101, <4 x float>* %zi2
  br label %102

; <label>:102                                     ; preds = %49
  %103 = load i32* %10                            ; <i32> [#uses=1]
  %104 = shl i32 %103, 2                          ; <i32> [#uses=1]
  %105 = insertelement <4 x i32> undef, i32 %104, i32 0 ; <<4 x i32>> [#uses=2]
  %106 = shufflevector <4 x i32> %105, <4 x i32> %105, <4 x i32> zeroinitializer ; <<4 x i32>> [#uses=1]
  store <4 x i32> <i32 0, i32 1, i32 2, i32 3>, <4 x i32>* %14
  %107 = load <4 x i32>* %14                      ; <<4 x i32>> [#uses=1]
  %108 = add nsw <4 x i32> %106, %107             ; <<4 x i32>> [#uses=1]
  store <4 x i32> %108, <4 x i32>* %tbase
  br label %109

; <label>:109                                     ; preds = %102
  %110 = load <4 x i32>* %tbase                   ; <<4 x i32>> [#uses=1]
  %111 = mul <4 x i32> %110, <i32 1, i32 1, i32 1, i32 1> ; <<4 x i32>> [#uses=1]
  store <4 x i32> %111, <4 x i32>* %8
  store <4 x float>* %c1, <4 x float>** %9
  %112 = load <4 x i32>* %8                       ; <<4 x i32>> [#uses=1]
  %113 = icmp sgt <4 x i32> %112, <i32 512, i32 512, i32 512, i32 512> ; <<4 x i1>> [#uses=1]
  %114 = sext <4 x i1> %113 to <4 x i32>          ; <<4 x i32>> [#uses=1]
  %115 = and <4 x i32> %114, <i32 1024, i32 1024, i32 1024, i32 1024> ; <<4 x i32>> [#uses=1]
  %116 = load <4 x i32>* %8                       ; <<4 x i32>> [#uses=1]
  %117 = sub <4 x i32> %116, %115                 ; <<4 x i32>> [#uses=1]
  store <4 x i32> %117, <4 x i32>* %8
  %118 = load <4 x i32>* %8                       ; <<4 x i32>> [#uses=1]
  %119 = call <4 x float> @_Z14convert_float4U8__vector4i(<4 x i32> %118) nounwind ; <<4 x float>> [#uses=1]
  %120 = fmul <4 x float> %119, <float 0xBF7921FB60000000, float 0xBF7921FB60000000, float 0xBF7921FB60000000, float 0xBF7921FB60000000> ; <<4 x float>> [#uses=1]
  store <4 x float> %120, <4 x float>* %x.i
  %121 = load <4 x float>* %x.i                   ; <<4 x float>> [#uses=1]
  %122 = call <4 x float> @_Z10native_cosU8__vector4f(<4 x float> %121) nounwind ; <<4 x float>> [#uses=1]
  %123 = load <4 x float>** %9                    ; <<4 x float>*> [#uses=1]
  store <4 x float> %122, <4 x float>* %123
  %124 = load <4 x float>* %x.i                   ; <<4 x float>> [#uses=1]
  %125 = call <4 x float> @_Z10native_sinU8__vector4f(<4 x float> %124) nounwind ; <<4 x float>> [#uses=1]
  store <4 x float> %125, <4 x float>* %7
  %126 = load <4 x float>* %7                     ; <<4 x float>> [#uses=1]
  store <4 x float> %126, <4 x float>* %s1
  br label %127

; <label>:127                                     ; preds = %109
  %128 = load <4 x float>* %c1                    ; <<4 x float>> [#uses=1]
  %129 = load <4 x float>* %zr1                   ; <<4 x float>> [#uses=1]
  %130 = fmul <4 x float> %128, %129              ; <<4 x float>> [#uses=1]
  %131 = load <4 x float>* %s1                    ; <<4 x float>> [#uses=1]
  %132 = load <4 x float>* %zi1                   ; <<4 x float>> [#uses=1]
  %133 = fmul <4 x float> %131, %132              ; <<4 x float>> [#uses=1]
  %134 = fsub <4 x float> %130, %133              ; <<4 x float>> [#uses=1]
  store <4 x float> %134, <4 x float>* %__r
  %135 = load <4 x float>* %c1                    ; <<4 x float>> [#uses=1]
  %136 = load <4 x float>* %zi1                   ; <<4 x float>> [#uses=1]
  %137 = fmul <4 x float> %135, %136              ; <<4 x float>> [#uses=1]
  %138 = load <4 x float>* %s1                    ; <<4 x float>> [#uses=1]
  %139 = load <4 x float>* %zr1                   ; <<4 x float>> [#uses=1]
  %140 = fmul <4 x float> %138, %139              ; <<4 x float>> [#uses=1]
  %141 = fadd <4 x float> %137, %140              ; <<4 x float>> [#uses=1]
  store <4 x float> %141, <4 x float>* %zi1
  %142 = load <4 x float>* %__r                   ; <<4 x float>> [#uses=1]
  store <4 x float> %142, <4 x float>* %zr1
  br label %143

; <label>:143                                     ; preds = %127
  %144 = load <4 x i32>* %tbase                   ; <<4 x i32>> [#uses=1]
  %145 = mul <4 x i32> %144, <i32 2, i32 2, i32 2, i32 2> ; <<4 x i32>> [#uses=1]
  store <4 x i32> %145, <4 x i32>* %2
  store <4 x float>* %c2, <4 x float>** %3
  %146 = load <4 x i32>* %2                       ; <<4 x i32>> [#uses=1]
  %147 = icmp sgt <4 x i32> %146, <i32 512, i32 512, i32 512, i32 512> ; <<4 x i1>> [#uses=1]
  %148 = sext <4 x i1> %147 to <4 x i32>          ; <<4 x i32>> [#uses=1]
  %149 = and <4 x i32> %148, <i32 1024, i32 1024, i32 1024, i32 1024> ; <<4 x i32>> [#uses=1]
  %150 = load <4 x i32>* %2                       ; <<4 x i32>> [#uses=1]
  %151 = sub <4 x i32> %150, %149                 ; <<4 x i32>> [#uses=1]
  store <4 x i32> %151, <4 x i32>* %2
  %152 = load <4 x i32>* %2                       ; <<4 x i32>> [#uses=1]
  %153 = call <4 x float> @_Z14convert_float4U8__vector4i(<4 x i32> %152) nounwind ; <<4 x float>> [#uses=1]
  %154 = fmul <4 x float> %153, <float 0xBF7921FB60000000, float 0xBF7921FB60000000, float 0xBF7921FB60000000, float 0xBF7921FB60000000> ; <<4 x float>> [#uses=1]
  store <4 x float> %154, <4 x float>* %x.i4
  %155 = load <4 x float>* %x.i4                  ; <<4 x float>> [#uses=1]
  %156 = call <4 x float> @_Z10native_cosU8__vector4f(<4 x float> %155) nounwind ; <<4 x float>> [#uses=1]
  %157 = load <4 x float>** %3                    ; <<4 x float>*> [#uses=1]
  store <4 x float> %156, <4 x float>* %157
  %158 = load <4 x float>* %x.i4                  ; <<4 x float>> [#uses=1]
  %159 = call <4 x float> @_Z10native_sinU8__vector4f(<4 x float> %158) nounwind ; <<4 x float>> [#uses=1]
  store <4 x float> %159, <4 x float>* %1
  %160 = load <4 x float>* %1                     ; <<4 x float>> [#uses=1]
  store <4 x float> %160, <4 x float>* %s2
  br label %161

; <label>:161                                     ; preds = %143
  %162 = load <4 x float>* %c2                    ; <<4 x float>> [#uses=1]
  %163 = load <4 x float>* %zr2                   ; <<4 x float>> [#uses=1]
  %164 = fmul <4 x float> %162, %163              ; <<4 x float>> [#uses=1]
  %165 = load <4 x float>* %s2                    ; <<4 x float>> [#uses=1]
  %166 = load <4 x float>* %zi2                   ; <<4 x float>> [#uses=1]
  %167 = fmul <4 x float> %165, %166              ; <<4 x float>> [#uses=1]
  %168 = fsub <4 x float> %164, %167              ; <<4 x float>> [#uses=1]
  store <4 x float> %168, <4 x float>* %__r1
  %169 = load <4 x float>* %c2                    ; <<4 x float>> [#uses=1]
  %170 = load <4 x float>* %zi2                   ; <<4 x float>> [#uses=1]
  %171 = fmul <4 x float> %169, %170              ; <<4 x float>> [#uses=1]
  %172 = load <4 x float>* %s2                    ; <<4 x float>> [#uses=1]
  %173 = load <4 x float>* %zr2                   ; <<4 x float>> [#uses=1]
  %174 = fmul <4 x float> %172, %173              ; <<4 x float>> [#uses=1]
  %175 = fadd <4 x float> %171, %174              ; <<4 x float>> [#uses=1]
  store <4 x float> %175, <4 x float>* %zi2
  %176 = load <4 x float>* %__r1                  ; <<4 x float>> [#uses=1]
  store <4 x float> %176, <4 x float>* %zr2
  br label %177

; <label>:177                                     ; preds = %161
  %178 = load <4 x i32>* %tbase                   ; <<4 x i32>> [#uses=1]
  %179 = mul <4 x i32> %178, <i32 3, i32 3, i32 3, i32 3> ; <<4 x i32>> [#uses=1]
  store <4 x i32> %179, <4 x i32>* %5
  store <4 x float>* %c3, <4 x float>** %6
  %180 = load <4 x i32>* %5                       ; <<4 x i32>> [#uses=1]
  %181 = icmp sgt <4 x i32> %180, <i32 512, i32 512, i32 512, i32 512> ; <<4 x i1>> [#uses=1]
  %182 = sext <4 x i1> %181 to <4 x i32>          ; <<4 x i32>> [#uses=1]
  %183 = and <4 x i32> %182, <i32 1024, i32 1024, i32 1024, i32 1024> ; <<4 x i32>> [#uses=1]
  %184 = load <4 x i32>* %5                       ; <<4 x i32>> [#uses=1]
  %185 = sub <4 x i32> %184, %183                 ; <<4 x i32>> [#uses=1]
  store <4 x i32> %185, <4 x i32>* %5
  %186 = load <4 x i32>* %5                       ; <<4 x i32>> [#uses=1]
  %187 = call <4 x float> @_Z14convert_float4U8__vector4i(<4 x i32> %186) nounwind ; <<4 x float>> [#uses=1]
  %188 = fmul <4 x float> %187, <float 0xBF7921FB60000000, float 0xBF7921FB60000000, float 0xBF7921FB60000000, float 0xBF7921FB60000000> ; <<4 x float>> [#uses=1]
  store <4 x float> %188, <4 x float>* %x.i3
  %189 = load <4 x float>* %x.i3                  ; <<4 x float>> [#uses=1]
  %190 = call <4 x float> @_Z10native_cosU8__vector4f(<4 x float> %189) nounwind ; <<4 x float>> [#uses=1]
  %191 = load <4 x float>** %6                    ; <<4 x float>*> [#uses=1]
  store <4 x float> %190, <4 x float>* %191
  %192 = load <4 x float>* %x.i3                  ; <<4 x float>> [#uses=1]
  %193 = call <4 x float> @_Z10native_sinU8__vector4f(<4 x float> %192) nounwind ; <<4 x float>> [#uses=1]
  store <4 x float> %193, <4 x float>* %4
  %194 = load <4 x float>* %4                     ; <<4 x float>> [#uses=1]
  store <4 x float> %194, <4 x float>* %s3
  br label %195

; <label>:195                                     ; preds = %177
  %196 = load <4 x float>* %c3                    ; <<4 x float>> [#uses=1]
  %197 = load <4 x float>* %zr3                   ; <<4 x float>> [#uses=1]
  %198 = fmul <4 x float> %196, %197              ; <<4 x float>> [#uses=1]
  %199 = load <4 x float>* %s3                    ; <<4 x float>> [#uses=1]
  %200 = load <4 x float>* %zi3                   ; <<4 x float>> [#uses=1]
  %201 = fmul <4 x float> %199, %200              ; <<4 x float>> [#uses=1]
  %202 = fsub <4 x float> %198, %201              ; <<4 x float>> [#uses=1]
  store <4 x float> %202, <4 x float>* %__r2
  %203 = load <4 x float>* %c3                    ; <<4 x float>> [#uses=1]
  %204 = load <4 x float>* %zi3                   ; <<4 x float>> [#uses=1]
  %205 = fmul <4 x float> %203, %204              ; <<4 x float>> [#uses=1]
  %206 = load <4 x float>* %s3                    ; <<4 x float>> [#uses=1]
  %207 = load <4 x float>* %zr3                   ; <<4 x float>> [#uses=1]
  %208 = fmul <4 x float> %206, %207              ; <<4 x float>> [#uses=1]
  %209 = fadd <4 x float> %205, %208              ; <<4 x float>> [#uses=1]
  store <4 x float> %209, <4 x float>* %zi3
  %210 = load <4 x float>* %__r2                  ; <<4 x float>> [#uses=1]
  store <4 x float> %210, <4 x float>* %zr3
  br label %211

; <label>:211                                     ; preds = %195
  br label %212

; <label>:212                                     ; preds = %211
  %213 = load float addrspace(3)** %13            ; <float addrspace(3)*> [#uses=1]
  %214 = load i32* %10                            ; <i32> [#uses=1]
  %215 = shl i32 %214, 2                          ; <i32> [#uses=1]
  %216 = load i32* %10                            ; <i32> [#uses=1]
  %217 = lshr i32 %216, 3                         ; <i32> [#uses=1]
  %218 = add i32 %215, %217                       ; <i32> [#uses=1]
  %219 = getelementptr inbounds float addrspace(3)* %213, i32 %218 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %219, float addrspace(3)** %lp
  %220 = load <4 x float>* %zr0                   ; <<4 x float>> [#uses=1]
  %221 = extractelement <4 x float> %220, i32 0   ; <float> [#uses=1]
  %222 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %223 = getelementptr inbounds float addrspace(3)* %222, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %221, float addrspace(3)* %223
  %224 = load <4 x float>* %zr0                   ; <<4 x float>> [#uses=1]
  %225 = extractelement <4 x float> %224, i32 1   ; <float> [#uses=1]
  %226 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %227 = getelementptr inbounds float addrspace(3)* %226, i32 1 ; <float addrspace(3)*> [#uses=1]
  store float %225, float addrspace(3)* %227
  %228 = load <4 x float>* %zr0                   ; <<4 x float>> [#uses=1]
  %229 = extractelement <4 x float> %228, i32 2   ; <float> [#uses=1]
  %230 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %231 = getelementptr inbounds float addrspace(3)* %230, i32 2 ; <float addrspace(3)*> [#uses=1]
  store float %229, float addrspace(3)* %231
  %232 = load <4 x float>* %zr0                   ; <<4 x float>> [#uses=1]
  %233 = extractelement <4 x float> %232, i32 3   ; <float> [#uses=1]
  %234 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %235 = getelementptr inbounds float addrspace(3)* %234, i32 3 ; <float addrspace(3)*> [#uses=1]
  store float %233, float addrspace(3)* %235
  %236 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %237 = getelementptr inbounds float addrspace(3)* %236, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %237, float addrspace(3)** %lp
  %238 = load <4 x float>* %zr1                   ; <<4 x float>> [#uses=1]
  %239 = extractelement <4 x float> %238, i32 0   ; <float> [#uses=1]
  %240 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %241 = getelementptr inbounds float addrspace(3)* %240, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %239, float addrspace(3)* %241
  %242 = load <4 x float>* %zr1                   ; <<4 x float>> [#uses=1]
  %243 = extractelement <4 x float> %242, i32 1   ; <float> [#uses=1]
  %244 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %245 = getelementptr inbounds float addrspace(3)* %244, i32 1 ; <float addrspace(3)*> [#uses=1]
  store float %243, float addrspace(3)* %245
  %246 = load <4 x float>* %zr1                   ; <<4 x float>> [#uses=1]
  %247 = extractelement <4 x float> %246, i32 2   ; <float> [#uses=1]
  %248 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %249 = getelementptr inbounds float addrspace(3)* %248, i32 2 ; <float addrspace(3)*> [#uses=1]
  store float %247, float addrspace(3)* %249
  %250 = load <4 x float>* %zr1                   ; <<4 x float>> [#uses=1]
  %251 = extractelement <4 x float> %250, i32 3   ; <float> [#uses=1]
  %252 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %253 = getelementptr inbounds float addrspace(3)* %252, i32 3 ; <float addrspace(3)*> [#uses=1]
  store float %251, float addrspace(3)* %253
  %254 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %255 = getelementptr inbounds float addrspace(3)* %254, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %255, float addrspace(3)** %lp
  %256 = load <4 x float>* %zr2                   ; <<4 x float>> [#uses=1]
  %257 = extractelement <4 x float> %256, i32 0   ; <float> [#uses=1]
  %258 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %259 = getelementptr inbounds float addrspace(3)* %258, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %257, float addrspace(3)* %259
  %260 = load <4 x float>* %zr2                   ; <<4 x float>> [#uses=1]
  %261 = extractelement <4 x float> %260, i32 1   ; <float> [#uses=1]
  %262 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %263 = getelementptr inbounds float addrspace(3)* %262, i32 1 ; <float addrspace(3)*> [#uses=1]
  store float %261, float addrspace(3)* %263
  %264 = load <4 x float>* %zr2                   ; <<4 x float>> [#uses=1]
  %265 = extractelement <4 x float> %264, i32 2   ; <float> [#uses=1]
  %266 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %267 = getelementptr inbounds float addrspace(3)* %266, i32 2 ; <float addrspace(3)*> [#uses=1]
  store float %265, float addrspace(3)* %267
  %268 = load <4 x float>* %zr2                   ; <<4 x float>> [#uses=1]
  %269 = extractelement <4 x float> %268, i32 3   ; <float> [#uses=1]
  %270 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %271 = getelementptr inbounds float addrspace(3)* %270, i32 3 ; <float addrspace(3)*> [#uses=1]
  store float %269, float addrspace(3)* %271
  %272 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %273 = getelementptr inbounds float addrspace(3)* %272, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %273, float addrspace(3)** %lp
  %274 = load <4 x float>* %zr3                   ; <<4 x float>> [#uses=1]
  %275 = extractelement <4 x float> %274, i32 0   ; <float> [#uses=1]
  %276 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %277 = getelementptr inbounds float addrspace(3)* %276, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %275, float addrspace(3)* %277
  %278 = load <4 x float>* %zr3                   ; <<4 x float>> [#uses=1]
  %279 = extractelement <4 x float> %278, i32 1   ; <float> [#uses=1]
  %280 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %281 = getelementptr inbounds float addrspace(3)* %280, i32 1 ; <float addrspace(3)*> [#uses=1]
  store float %279, float addrspace(3)* %281
  %282 = load <4 x float>* %zr3                   ; <<4 x float>> [#uses=1]
  %283 = extractelement <4 x float> %282, i32 2   ; <float> [#uses=1]
  %284 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %285 = getelementptr inbounds float addrspace(3)* %284, i32 2 ; <float addrspace(3)*> [#uses=1]
  store float %283, float addrspace(3)* %285
  %286 = load <4 x float>* %zr3                   ; <<4 x float>> [#uses=1]
  %287 = extractelement <4 x float> %286, i32 3   ; <float> [#uses=1]
  %288 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %289 = getelementptr inbounds float addrspace(3)* %288, i32 3 ; <float addrspace(3)*> [#uses=1]
  store float %287, float addrspace(3)* %289
  %290 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %291 = getelementptr inbounds float addrspace(3)* %290, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %291, float addrspace(3)** %lp
  %292 = load <4 x float>* %zi0                   ; <<4 x float>> [#uses=1]
  %293 = extractelement <4 x float> %292, i32 0   ; <float> [#uses=1]
  %294 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %295 = getelementptr inbounds float addrspace(3)* %294, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %293, float addrspace(3)* %295
  %296 = load <4 x float>* %zi0                   ; <<4 x float>> [#uses=1]
  %297 = extractelement <4 x float> %296, i32 1   ; <float> [#uses=1]
  %298 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %299 = getelementptr inbounds float addrspace(3)* %298, i32 1 ; <float addrspace(3)*> [#uses=1]
  store float %297, float addrspace(3)* %299
  %300 = load <4 x float>* %zi0                   ; <<4 x float>> [#uses=1]
  %301 = extractelement <4 x float> %300, i32 2   ; <float> [#uses=1]
  %302 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %303 = getelementptr inbounds float addrspace(3)* %302, i32 2 ; <float addrspace(3)*> [#uses=1]
  store float %301, float addrspace(3)* %303
  %304 = load <4 x float>* %zi0                   ; <<4 x float>> [#uses=1]
  %305 = extractelement <4 x float> %304, i32 3   ; <float> [#uses=1]
  %306 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %307 = getelementptr inbounds float addrspace(3)* %306, i32 3 ; <float addrspace(3)*> [#uses=1]
  store float %305, float addrspace(3)* %307
  %308 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %309 = getelementptr inbounds float addrspace(3)* %308, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %309, float addrspace(3)** %lp
  %310 = load <4 x float>* %zi1                   ; <<4 x float>> [#uses=1]
  %311 = extractelement <4 x float> %310, i32 0   ; <float> [#uses=1]
  %312 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %313 = getelementptr inbounds float addrspace(3)* %312, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %311, float addrspace(3)* %313
  %314 = load <4 x float>* %zi1                   ; <<4 x float>> [#uses=1]
  %315 = extractelement <4 x float> %314, i32 1   ; <float> [#uses=1]
  %316 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %317 = getelementptr inbounds float addrspace(3)* %316, i32 1 ; <float addrspace(3)*> [#uses=1]
  store float %315, float addrspace(3)* %317
  %318 = load <4 x float>* %zi1                   ; <<4 x float>> [#uses=1]
  %319 = extractelement <4 x float> %318, i32 2   ; <float> [#uses=1]
  %320 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %321 = getelementptr inbounds float addrspace(3)* %320, i32 2 ; <float addrspace(3)*> [#uses=1]
  store float %319, float addrspace(3)* %321
  %322 = load <4 x float>* %zi1                   ; <<4 x float>> [#uses=1]
  %323 = extractelement <4 x float> %322, i32 3   ; <float> [#uses=1]
  %324 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %325 = getelementptr inbounds float addrspace(3)* %324, i32 3 ; <float addrspace(3)*> [#uses=1]
  store float %323, float addrspace(3)* %325
  %326 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %327 = getelementptr inbounds float addrspace(3)* %326, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %327, float addrspace(3)** %lp
  %328 = load <4 x float>* %zi2                   ; <<4 x float>> [#uses=1]
  %329 = extractelement <4 x float> %328, i32 0   ; <float> [#uses=1]
  %330 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %331 = getelementptr inbounds float addrspace(3)* %330, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %329, float addrspace(3)* %331
  %332 = load <4 x float>* %zi2                   ; <<4 x float>> [#uses=1]
  %333 = extractelement <4 x float> %332, i32 1   ; <float> [#uses=1]
  %334 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %335 = getelementptr inbounds float addrspace(3)* %334, i32 1 ; <float addrspace(3)*> [#uses=1]
  store float %333, float addrspace(3)* %335
  %336 = load <4 x float>* %zi2                   ; <<4 x float>> [#uses=1]
  %337 = extractelement <4 x float> %336, i32 2   ; <float> [#uses=1]
  %338 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %339 = getelementptr inbounds float addrspace(3)* %338, i32 2 ; <float addrspace(3)*> [#uses=1]
  store float %337, float addrspace(3)* %339
  %340 = load <4 x float>* %zi2                   ; <<4 x float>> [#uses=1]
  %341 = extractelement <4 x float> %340, i32 3   ; <float> [#uses=1]
  %342 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %343 = getelementptr inbounds float addrspace(3)* %342, i32 3 ; <float addrspace(3)*> [#uses=1]
  store float %341, float addrspace(3)* %343
  %344 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %345 = getelementptr inbounds float addrspace(3)* %344, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %345, float addrspace(3)** %lp
  %346 = load <4 x float>* %zi3                   ; <<4 x float>> [#uses=1]
  %347 = extractelement <4 x float> %346, i32 0   ; <float> [#uses=1]
  %348 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %349 = getelementptr inbounds float addrspace(3)* %348, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %347, float addrspace(3)* %349
  %350 = load <4 x float>* %zi3                   ; <<4 x float>> [#uses=1]
  %351 = extractelement <4 x float> %350, i32 1   ; <float> [#uses=1]
  %352 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %353 = getelementptr inbounds float addrspace(3)* %352, i32 1 ; <float addrspace(3)*> [#uses=1]
  store float %351, float addrspace(3)* %353
  %354 = load <4 x float>* %zi3                   ; <<4 x float>> [#uses=1]
  %355 = extractelement <4 x float> %354, i32 2   ; <float> [#uses=1]
  %356 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %357 = getelementptr inbounds float addrspace(3)* %356, i32 2 ; <float addrspace(3)*> [#uses=1]
  store float %355, float addrspace(3)* %357
  %358 = load <4 x float>* %zi3                   ; <<4 x float>> [#uses=1]
  %359 = extractelement <4 x float> %358, i32 3   ; <float> [#uses=1]
  %360 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %361 = getelementptr inbounds float addrspace(3)* %360, i32 3 ; <float addrspace(3)*> [#uses=1]
  store float %359, float addrspace(3)* %361
  call void @barrier(i32 1)
  ret void
}

declare void @barrier(i32)

define void @kfft_pass2(i32 %me, float addrspace(3)* %lds) nounwind alwaysinline {
  %1 = alloca float, align 4                      ; <float*> [#uses=2]
  %2 = alloca i32, align 4                        ; <i32*> [#uses=5]
  %3 = alloca float*, align 4                     ; <float**> [#uses=2]
  %x.i5 = alloca float, align 4                   ; <float*> [#uses=3]
  %4 = alloca float, align 4                      ; <float*> [#uses=2]
  %5 = alloca i32, align 4                        ; <i32*> [#uses=5]
  %6 = alloca float*, align 4                     ; <float**> [#uses=2]
  %x.i3 = alloca float, align 4                   ; <float*> [#uses=3]
  %7 = alloca float, align 4                      ; <float*> [#uses=2]
  %8 = alloca i32, align 4                        ; <i32*> [#uses=5]
  %9 = alloca float*, align 4                     ; <float**> [#uses=2]
  %x.i = alloca float, align 4                    ; <float*> [#uses=3]
  %10 = alloca i32, align 4                       ; <i32*> [#uses=6]
  %11 = alloca float addrspace(3)*, align 16      ; <float addrspace(3)**> [#uses=3]
  %lp = alloca float addrspace(3)*, align 16      ; <float addrspace(3)**> [#uses=94]
  %zr0 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=15]
  %zr1 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=18]
  %zr2 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=18]
  %zr3 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=18]
  %zi0 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=15]
  %zi1 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=18]
  %zi2 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=18]
  %zi3 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=18]
  %ar0 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %ar2 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %br0 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %br1 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %br2 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %br3 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %ai0 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %ai2 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %bi0 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %bi1 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %bi2 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %bi3 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %tbase = alloca i32, align 4                    ; <i32*> [#uses=4]
  %c1 = alloca float, align 4                     ; <float*> [#uses=3]
  %s1 = alloca float, align 4                     ; <float*> [#uses=3]
  %__r = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %c2 = alloca float, align 4                     ; <float*> [#uses=3]
  %s2 = alloca float, align 4                     ; <float*> [#uses=3]
  %__r1 = alloca <4 x float>, align 16            ; <<4 x float>*> [#uses=2]
  %c3 = alloca float, align 4                     ; <float*> [#uses=3]
  %s3 = alloca float, align 4                     ; <float*> [#uses=3]
  %__r2 = alloca <4 x float>, align 16            ; <<4 x float>*> [#uses=2]
  store i32 %me, i32* %10
  store float addrspace(3)* %lds, float addrspace(3)** %11
  %12 = load float addrspace(3)** %11             ; <float addrspace(3)*> [#uses=1]
  %13 = load i32* %10                             ; <i32> [#uses=1]
  %14 = load i32* %10                             ; <i32> [#uses=1]
  %15 = lshr i32 %14, 5                           ; <i32> [#uses=1]
  %16 = add i32 %13, %15                          ; <i32> [#uses=1]
  %17 = getelementptr inbounds float addrspace(3)* %12, i32 %16 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %17, float addrspace(3)** %lp
  %18 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %19 = getelementptr inbounds float addrspace(3)* %18, i32 0 ; <float addrspace(3)*> [#uses=1]
  %20 = load float addrspace(3)* %19              ; <float> [#uses=1]
  %21 = load <4 x float>* %zr0                    ; <<4 x float>> [#uses=1]
  %22 = insertelement <4 x float> %21, float %20, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %22, <4 x float>* %zr0
  %23 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %24 = getelementptr inbounds float addrspace(3)* %23, i32 66 ; <float addrspace(3)*> [#uses=1]
  %25 = load float addrspace(3)* %24              ; <float> [#uses=1]
  %26 = load <4 x float>* %zr1                    ; <<4 x float>> [#uses=1]
  %27 = insertelement <4 x float> %26, float %25, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %27, <4 x float>* %zr1
  %28 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %29 = getelementptr inbounds float addrspace(3)* %28, i32 132 ; <float addrspace(3)*> [#uses=1]
  %30 = load float addrspace(3)* %29              ; <float> [#uses=1]
  %31 = load <4 x float>* %zr2                    ; <<4 x float>> [#uses=1]
  %32 = insertelement <4 x float> %31, float %30, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %32, <4 x float>* %zr2
  %33 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %34 = getelementptr inbounds float addrspace(3)* %33, i32 198 ; <float addrspace(3)*> [#uses=1]
  %35 = load float addrspace(3)* %34              ; <float> [#uses=1]
  %36 = load <4 x float>* %zr3                    ; <<4 x float>> [#uses=1]
  %37 = insertelement <4 x float> %36, float %35, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %37, <4 x float>* %zr3
  %38 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %39 = getelementptr inbounds float addrspace(3)* %38, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %39, float addrspace(3)** %lp
  %40 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %41 = getelementptr inbounds float addrspace(3)* %40, i32 0 ; <float addrspace(3)*> [#uses=1]
  %42 = load float addrspace(3)* %41              ; <float> [#uses=1]
  %43 = load <4 x float>* %zr0                    ; <<4 x float>> [#uses=1]
  %44 = insertelement <4 x float> %43, float %42, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %44, <4 x float>* %zr0
  %45 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %46 = getelementptr inbounds float addrspace(3)* %45, i32 66 ; <float addrspace(3)*> [#uses=1]
  %47 = load float addrspace(3)* %46              ; <float> [#uses=1]
  %48 = load <4 x float>* %zr1                    ; <<4 x float>> [#uses=1]
  %49 = insertelement <4 x float> %48, float %47, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %49, <4 x float>* %zr1
  %50 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %51 = getelementptr inbounds float addrspace(3)* %50, i32 132 ; <float addrspace(3)*> [#uses=1]
  %52 = load float addrspace(3)* %51              ; <float> [#uses=1]
  %53 = load <4 x float>* %zr2                    ; <<4 x float>> [#uses=1]
  %54 = insertelement <4 x float> %53, float %52, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %54, <4 x float>* %zr2
  %55 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %56 = getelementptr inbounds float addrspace(3)* %55, i32 198 ; <float addrspace(3)*> [#uses=1]
  %57 = load float addrspace(3)* %56              ; <float> [#uses=1]
  %58 = load <4 x float>* %zr3                    ; <<4 x float>> [#uses=1]
  %59 = insertelement <4 x float> %58, float %57, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %59, <4 x float>* %zr3
  %60 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %61 = getelementptr inbounds float addrspace(3)* %60, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %61, float addrspace(3)** %lp
  %62 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %63 = getelementptr inbounds float addrspace(3)* %62, i32 0 ; <float addrspace(3)*> [#uses=1]
  %64 = load float addrspace(3)* %63              ; <float> [#uses=1]
  %65 = load <4 x float>* %zr0                    ; <<4 x float>> [#uses=1]
  %66 = insertelement <4 x float> %65, float %64, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %66, <4 x float>* %zr0
  %67 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %68 = getelementptr inbounds float addrspace(3)* %67, i32 66 ; <float addrspace(3)*> [#uses=1]
  %69 = load float addrspace(3)* %68              ; <float> [#uses=1]
  %70 = load <4 x float>* %zr1                    ; <<4 x float>> [#uses=1]
  %71 = insertelement <4 x float> %70, float %69, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %71, <4 x float>* %zr1
  %72 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %73 = getelementptr inbounds float addrspace(3)* %72, i32 132 ; <float addrspace(3)*> [#uses=1]
  %74 = load float addrspace(3)* %73              ; <float> [#uses=1]
  %75 = load <4 x float>* %zr2                    ; <<4 x float>> [#uses=1]
  %76 = insertelement <4 x float> %75, float %74, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %76, <4 x float>* %zr2
  %77 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %78 = getelementptr inbounds float addrspace(3)* %77, i32 198 ; <float addrspace(3)*> [#uses=1]
  %79 = load float addrspace(3)* %78              ; <float> [#uses=1]
  %80 = load <4 x float>* %zr3                    ; <<4 x float>> [#uses=1]
  %81 = insertelement <4 x float> %80, float %79, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %81, <4 x float>* %zr3
  %82 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %83 = getelementptr inbounds float addrspace(3)* %82, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %83, float addrspace(3)** %lp
  %84 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %85 = getelementptr inbounds float addrspace(3)* %84, i32 0 ; <float addrspace(3)*> [#uses=1]
  %86 = load float addrspace(3)* %85              ; <float> [#uses=1]
  %87 = load <4 x float>* %zr0                    ; <<4 x float>> [#uses=1]
  %88 = insertelement <4 x float> %87, float %86, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %88, <4 x float>* %zr0
  %89 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %90 = getelementptr inbounds float addrspace(3)* %89, i32 66 ; <float addrspace(3)*> [#uses=1]
  %91 = load float addrspace(3)* %90              ; <float> [#uses=1]
  %92 = load <4 x float>* %zr1                    ; <<4 x float>> [#uses=1]
  %93 = insertelement <4 x float> %92, float %91, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %93, <4 x float>* %zr1
  %94 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %95 = getelementptr inbounds float addrspace(3)* %94, i32 132 ; <float addrspace(3)*> [#uses=1]
  %96 = load float addrspace(3)* %95              ; <float> [#uses=1]
  %97 = load <4 x float>* %zr2                    ; <<4 x float>> [#uses=1]
  %98 = insertelement <4 x float> %97, float %96, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %98, <4 x float>* %zr2
  %99 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %100 = getelementptr inbounds float addrspace(3)* %99, i32 198 ; <float addrspace(3)*> [#uses=1]
  %101 = load float addrspace(3)* %100            ; <float> [#uses=1]
  %102 = load <4 x float>* %zr3                   ; <<4 x float>> [#uses=1]
  %103 = insertelement <4 x float> %102, float %101, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %103, <4 x float>* %zr3
  %104 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %105 = getelementptr inbounds float addrspace(3)* %104, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %105, float addrspace(3)** %lp
  %106 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %107 = getelementptr inbounds float addrspace(3)* %106, i32 0 ; <float addrspace(3)*> [#uses=1]
  %108 = load float addrspace(3)* %107            ; <float> [#uses=1]
  %109 = load <4 x float>* %zi0                   ; <<4 x float>> [#uses=1]
  %110 = insertelement <4 x float> %109, float %108, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %110, <4 x float>* %zi0
  %111 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %112 = getelementptr inbounds float addrspace(3)* %111, i32 66 ; <float addrspace(3)*> [#uses=1]
  %113 = load float addrspace(3)* %112            ; <float> [#uses=1]
  %114 = load <4 x float>* %zi1                   ; <<4 x float>> [#uses=1]
  %115 = insertelement <4 x float> %114, float %113, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %115, <4 x float>* %zi1
  %116 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %117 = getelementptr inbounds float addrspace(3)* %116, i32 132 ; <float addrspace(3)*> [#uses=1]
  %118 = load float addrspace(3)* %117            ; <float> [#uses=1]
  %119 = load <4 x float>* %zi2                   ; <<4 x float>> [#uses=1]
  %120 = insertelement <4 x float> %119, float %118, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %120, <4 x float>* %zi2
  %121 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %122 = getelementptr inbounds float addrspace(3)* %121, i32 198 ; <float addrspace(3)*> [#uses=1]
  %123 = load float addrspace(3)* %122            ; <float> [#uses=1]
  %124 = load <4 x float>* %zi3                   ; <<4 x float>> [#uses=1]
  %125 = insertelement <4 x float> %124, float %123, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %125, <4 x float>* %zi3
  %126 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %127 = getelementptr inbounds float addrspace(3)* %126, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %127, float addrspace(3)** %lp
  %128 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %129 = getelementptr inbounds float addrspace(3)* %128, i32 0 ; <float addrspace(3)*> [#uses=1]
  %130 = load float addrspace(3)* %129            ; <float> [#uses=1]
  %131 = load <4 x float>* %zi0                   ; <<4 x float>> [#uses=1]
  %132 = insertelement <4 x float> %131, float %130, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %132, <4 x float>* %zi0
  %133 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %134 = getelementptr inbounds float addrspace(3)* %133, i32 66 ; <float addrspace(3)*> [#uses=1]
  %135 = load float addrspace(3)* %134            ; <float> [#uses=1]
  %136 = load <4 x float>* %zi1                   ; <<4 x float>> [#uses=1]
  %137 = insertelement <4 x float> %136, float %135, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %137, <4 x float>* %zi1
  %138 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %139 = getelementptr inbounds float addrspace(3)* %138, i32 132 ; <float addrspace(3)*> [#uses=1]
  %140 = load float addrspace(3)* %139            ; <float> [#uses=1]
  %141 = load <4 x float>* %zi2                   ; <<4 x float>> [#uses=1]
  %142 = insertelement <4 x float> %141, float %140, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %142, <4 x float>* %zi2
  %143 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %144 = getelementptr inbounds float addrspace(3)* %143, i32 198 ; <float addrspace(3)*> [#uses=1]
  %145 = load float addrspace(3)* %144            ; <float> [#uses=1]
  %146 = load <4 x float>* %zi3                   ; <<4 x float>> [#uses=1]
  %147 = insertelement <4 x float> %146, float %145, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %147, <4 x float>* %zi3
  %148 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %149 = getelementptr inbounds float addrspace(3)* %148, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %149, float addrspace(3)** %lp
  %150 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %151 = getelementptr inbounds float addrspace(3)* %150, i32 0 ; <float addrspace(3)*> [#uses=1]
  %152 = load float addrspace(3)* %151            ; <float> [#uses=1]
  %153 = load <4 x float>* %zi0                   ; <<4 x float>> [#uses=1]
  %154 = insertelement <4 x float> %153, float %152, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %154, <4 x float>* %zi0
  %155 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %156 = getelementptr inbounds float addrspace(3)* %155, i32 66 ; <float addrspace(3)*> [#uses=1]
  %157 = load float addrspace(3)* %156            ; <float> [#uses=1]
  %158 = load <4 x float>* %zi1                   ; <<4 x float>> [#uses=1]
  %159 = insertelement <4 x float> %158, float %157, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %159, <4 x float>* %zi1
  %160 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %161 = getelementptr inbounds float addrspace(3)* %160, i32 132 ; <float addrspace(3)*> [#uses=1]
  %162 = load float addrspace(3)* %161            ; <float> [#uses=1]
  %163 = load <4 x float>* %zi2                   ; <<4 x float>> [#uses=1]
  %164 = insertelement <4 x float> %163, float %162, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %164, <4 x float>* %zi2
  %165 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %166 = getelementptr inbounds float addrspace(3)* %165, i32 198 ; <float addrspace(3)*> [#uses=1]
  %167 = load float addrspace(3)* %166            ; <float> [#uses=1]
  %168 = load <4 x float>* %zi3                   ; <<4 x float>> [#uses=1]
  %169 = insertelement <4 x float> %168, float %167, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %169, <4 x float>* %zi3
  %170 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %171 = getelementptr inbounds float addrspace(3)* %170, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %171, float addrspace(3)** %lp
  %172 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %173 = getelementptr inbounds float addrspace(3)* %172, i32 0 ; <float addrspace(3)*> [#uses=1]
  %174 = load float addrspace(3)* %173            ; <float> [#uses=1]
  %175 = load <4 x float>* %zi0                   ; <<4 x float>> [#uses=1]
  %176 = insertelement <4 x float> %175, float %174, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %176, <4 x float>* %zi0
  %177 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %178 = getelementptr inbounds float addrspace(3)* %177, i32 66 ; <float addrspace(3)*> [#uses=1]
  %179 = load float addrspace(3)* %178            ; <float> [#uses=1]
  %180 = load <4 x float>* %zi1                   ; <<4 x float>> [#uses=1]
  %181 = insertelement <4 x float> %180, float %179, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %181, <4 x float>* %zi1
  %182 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %183 = getelementptr inbounds float addrspace(3)* %182, i32 132 ; <float addrspace(3)*> [#uses=1]
  %184 = load float addrspace(3)* %183            ; <float> [#uses=1]
  %185 = load <4 x float>* %zi2                   ; <<4 x float>> [#uses=1]
  %186 = insertelement <4 x float> %185, float %184, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %186, <4 x float>* %zi2
  %187 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %188 = getelementptr inbounds float addrspace(3)* %187, i32 198 ; <float addrspace(3)*> [#uses=1]
  %189 = load float addrspace(3)* %188            ; <float> [#uses=1]
  %190 = load <4 x float>* %zi3                   ; <<4 x float>> [#uses=1]
  %191 = insertelement <4 x float> %190, float %189, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %191, <4 x float>* %zi3
  br label %192

; <label>:192                                     ; preds = %0
  %193 = load <4 x float>* %zr0                   ; <<4 x float>> [#uses=1]
  %194 = load <4 x float>* %zr2                   ; <<4 x float>> [#uses=1]
  %195 = fadd <4 x float> %193, %194              ; <<4 x float>> [#uses=1]
  store <4 x float> %195, <4 x float>* %ar0
  %196 = load <4 x float>* %zr1                   ; <<4 x float>> [#uses=1]
  %197 = load <4 x float>* %zr3                   ; <<4 x float>> [#uses=1]
  %198 = fadd <4 x float> %196, %197              ; <<4 x float>> [#uses=1]
  store <4 x float> %198, <4 x float>* %ar2
  %199 = load <4 x float>* %ar0                   ; <<4 x float>> [#uses=1]
  %200 = load <4 x float>* %ar2                   ; <<4 x float>> [#uses=1]
  %201 = fadd <4 x float> %199, %200              ; <<4 x float>> [#uses=1]
  store <4 x float> %201, <4 x float>* %br0
  %202 = load <4 x float>* %zr0                   ; <<4 x float>> [#uses=1]
  %203 = load <4 x float>* %zr2                   ; <<4 x float>> [#uses=1]
  %204 = fsub <4 x float> %202, %203              ; <<4 x float>> [#uses=1]
  store <4 x float> %204, <4 x float>* %br1
  %205 = load <4 x float>* %ar0                   ; <<4 x float>> [#uses=1]
  %206 = load <4 x float>* %ar2                   ; <<4 x float>> [#uses=1]
  %207 = fsub <4 x float> %205, %206              ; <<4 x float>> [#uses=1]
  store <4 x float> %207, <4 x float>* %br2
  %208 = load <4 x float>* %zr1                   ; <<4 x float>> [#uses=1]
  %209 = load <4 x float>* %zr3                   ; <<4 x float>> [#uses=1]
  %210 = fsub <4 x float> %208, %209              ; <<4 x float>> [#uses=1]
  store <4 x float> %210, <4 x float>* %br3
  %211 = load <4 x float>* %zi0                   ; <<4 x float>> [#uses=1]
  %212 = load <4 x float>* %zi2                   ; <<4 x float>> [#uses=1]
  %213 = fadd <4 x float> %211, %212              ; <<4 x float>> [#uses=1]
  store <4 x float> %213, <4 x float>* %ai0
  %214 = load <4 x float>* %zi1                   ; <<4 x float>> [#uses=1]
  %215 = load <4 x float>* %zi3                   ; <<4 x float>> [#uses=1]
  %216 = fadd <4 x float> %214, %215              ; <<4 x float>> [#uses=1]
  store <4 x float> %216, <4 x float>* %ai2
  %217 = load <4 x float>* %ai0                   ; <<4 x float>> [#uses=1]
  %218 = load <4 x float>* %ai2                   ; <<4 x float>> [#uses=1]
  %219 = fadd <4 x float> %217, %218              ; <<4 x float>> [#uses=1]
  store <4 x float> %219, <4 x float>* %bi0
  %220 = load <4 x float>* %zi0                   ; <<4 x float>> [#uses=1]
  %221 = load <4 x float>* %zi2                   ; <<4 x float>> [#uses=1]
  %222 = fsub <4 x float> %220, %221              ; <<4 x float>> [#uses=1]
  store <4 x float> %222, <4 x float>* %bi1
  %223 = load <4 x float>* %ai0                   ; <<4 x float>> [#uses=1]
  %224 = load <4 x float>* %ai2                   ; <<4 x float>> [#uses=1]
  %225 = fsub <4 x float> %223, %224              ; <<4 x float>> [#uses=1]
  store <4 x float> %225, <4 x float>* %bi2
  %226 = load <4 x float>* %zi1                   ; <<4 x float>> [#uses=1]
  %227 = load <4 x float>* %zi3                   ; <<4 x float>> [#uses=1]
  %228 = fsub <4 x float> %226, %227              ; <<4 x float>> [#uses=1]
  store <4 x float> %228, <4 x float>* %bi3
  %229 = load <4 x float>* %br0                   ; <<4 x float>> [#uses=1]
  store <4 x float> %229, <4 x float>* %zr0
  %230 = load <4 x float>* %bi0                   ; <<4 x float>> [#uses=1]
  store <4 x float> %230, <4 x float>* %zi0
  %231 = load <4 x float>* %br1                   ; <<4 x float>> [#uses=1]
  %232 = load <4 x float>* %bi3                   ; <<4 x float>> [#uses=1]
  %233 = fadd <4 x float> %231, %232              ; <<4 x float>> [#uses=1]
  store <4 x float> %233, <4 x float>* %zr1
  %234 = load <4 x float>* %bi1                   ; <<4 x float>> [#uses=1]
  %235 = load <4 x float>* %br3                   ; <<4 x float>> [#uses=1]
  %236 = fsub <4 x float> %234, %235              ; <<4 x float>> [#uses=1]
  store <4 x float> %236, <4 x float>* %zi1
  %237 = load <4 x float>* %br1                   ; <<4 x float>> [#uses=1]
  %238 = load <4 x float>* %bi3                   ; <<4 x float>> [#uses=1]
  %239 = fsub <4 x float> %237, %238              ; <<4 x float>> [#uses=1]
  store <4 x float> %239, <4 x float>* %zr3
  %240 = load <4 x float>* %br3                   ; <<4 x float>> [#uses=1]
  %241 = load <4 x float>* %bi1                   ; <<4 x float>> [#uses=1]
  %242 = fadd <4 x float> %240, %241              ; <<4 x float>> [#uses=1]
  store <4 x float> %242, <4 x float>* %zi3
  %243 = load <4 x float>* %br2                   ; <<4 x float>> [#uses=1]
  store <4 x float> %243, <4 x float>* %zr2
  %244 = load <4 x float>* %bi2                   ; <<4 x float>> [#uses=1]
  store <4 x float> %244, <4 x float>* %zi2
  br label %245

; <label>:245                                     ; preds = %192
  %246 = load i32* %10                            ; <i32> [#uses=1]
  %247 = shl i32 %246, 2                          ; <i32> [#uses=1]
  store i32 %247, i32* %tbase
  br label %248

; <label>:248                                     ; preds = %245
  %249 = load i32* %tbase                         ; <i32> [#uses=1]
  %250 = mul i32 %249, 1                          ; <i32> [#uses=1]
  store i32 %250, i32* %8
  store float* %c1, float** %9
  %251 = load i32* %8                             ; <i32> [#uses=1]
  %252 = icmp sgt i32 %251, 512                   ; <i1> [#uses=1]
  br i1 %252, label %253, label %k_sincos.exit

; <label>:253                                     ; preds = %248
  %254 = load i32* %8                             ; <i32> [#uses=1]
  %255 = sub i32 %254, 1024                       ; <i32> [#uses=1]
  store i32 %255, i32* %8
  br label %k_sincos.exit

k_sincos.exit:                                    ; preds = %248, %253
  %256 = load i32* %8                             ; <i32> [#uses=1]
  %257 = sitofp i32 %256 to float                 ; <float> [#uses=1]
  %258 = fmul float %257, 0xBF7921FB60000000      ; <float> [#uses=1]
  store float %258, float* %x.i
  %259 = load float* %x.i                         ; <float> [#uses=1]
  %260 = call float @_Z10native_cosf(float %259) nounwind ; <float> [#uses=1]
  %261 = load float** %9                          ; <float*> [#uses=1]
  store float %260, float* %261
  %262 = load float* %x.i                         ; <float> [#uses=1]
  %263 = call float @_Z10native_sinf(float %262) nounwind ; <float> [#uses=1]
  store float %263, float* %7
  %264 = load float* %7                           ; <float> [#uses=1]
  store float %264, float* %s1
  br label %265

; <label>:265                                     ; preds = %k_sincos.exit
  %266 = load float* %c1                          ; <float> [#uses=1]
  %267 = insertelement <4 x float> undef, float %266, i32 0 ; <<4 x float>> [#uses=2]
  %268 = shufflevector <4 x float> %267, <4 x float> %267, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %269 = load <4 x float>* %zr1                   ; <<4 x float>> [#uses=1]
  %270 = fmul <4 x float> %268, %269              ; <<4 x float>> [#uses=1]
  %271 = load float* %s1                          ; <float> [#uses=1]
  %272 = insertelement <4 x float> undef, float %271, i32 0 ; <<4 x float>> [#uses=2]
  %273 = shufflevector <4 x float> %272, <4 x float> %272, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %274 = load <4 x float>* %zi1                   ; <<4 x float>> [#uses=1]
  %275 = fmul <4 x float> %273, %274              ; <<4 x float>> [#uses=1]
  %276 = fsub <4 x float> %270, %275              ; <<4 x float>> [#uses=1]
  store <4 x float> %276, <4 x float>* %__r
  %277 = load float* %c1                          ; <float> [#uses=1]
  %278 = insertelement <4 x float> undef, float %277, i32 0 ; <<4 x float>> [#uses=2]
  %279 = shufflevector <4 x float> %278, <4 x float> %278, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %280 = load <4 x float>* %zi1                   ; <<4 x float>> [#uses=1]
  %281 = fmul <4 x float> %279, %280              ; <<4 x float>> [#uses=1]
  %282 = load float* %s1                          ; <float> [#uses=1]
  %283 = insertelement <4 x float> undef, float %282, i32 0 ; <<4 x float>> [#uses=2]
  %284 = shufflevector <4 x float> %283, <4 x float> %283, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %285 = load <4 x float>* %zr1                   ; <<4 x float>> [#uses=1]
  %286 = fmul <4 x float> %284, %285              ; <<4 x float>> [#uses=1]
  %287 = fadd <4 x float> %281, %286              ; <<4 x float>> [#uses=1]
  store <4 x float> %287, <4 x float>* %zi1
  %288 = load <4 x float>* %__r                   ; <<4 x float>> [#uses=1]
  store <4 x float> %288, <4 x float>* %zr1
  br label %289

; <label>:289                                     ; preds = %265
  %290 = load i32* %tbase                         ; <i32> [#uses=1]
  %291 = mul i32 %290, 2                          ; <i32> [#uses=1]
  store i32 %291, i32* %2
  store float* %c2, float** %3
  %292 = load i32* %2                             ; <i32> [#uses=1]
  %293 = icmp sgt i32 %292, 512                   ; <i1> [#uses=1]
  br i1 %293, label %294, label %k_sincos.exit6

; <label>:294                                     ; preds = %289
  %295 = load i32* %2                             ; <i32> [#uses=1]
  %296 = sub i32 %295, 1024                       ; <i32> [#uses=1]
  store i32 %296, i32* %2
  br label %k_sincos.exit6

k_sincos.exit6:                                   ; preds = %289, %294
  %297 = load i32* %2                             ; <i32> [#uses=1]
  %298 = sitofp i32 %297 to float                 ; <float> [#uses=1]
  %299 = fmul float %298, 0xBF7921FB60000000      ; <float> [#uses=1]
  store float %299, float* %x.i5
  %300 = load float* %x.i5                        ; <float> [#uses=1]
  %301 = call float @_Z10native_cosf(float %300) nounwind ; <float> [#uses=1]
  %302 = load float** %3                          ; <float*> [#uses=1]
  store float %301, float* %302
  %303 = load float* %x.i5                        ; <float> [#uses=1]
  %304 = call float @_Z10native_sinf(float %303) nounwind ; <float> [#uses=1]
  store float %304, float* %1
  %305 = load float* %1                           ; <float> [#uses=1]
  store float %305, float* %s2
  br label %306

; <label>:306                                     ; preds = %k_sincos.exit6
  %307 = load float* %c2                          ; <float> [#uses=1]
  %308 = insertelement <4 x float> undef, float %307, i32 0 ; <<4 x float>> [#uses=2]
  %309 = shufflevector <4 x float> %308, <4 x float> %308, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %310 = load <4 x float>* %zr2                   ; <<4 x float>> [#uses=1]
  %311 = fmul <4 x float> %309, %310              ; <<4 x float>> [#uses=1]
  %312 = load float* %s2                          ; <float> [#uses=1]
  %313 = insertelement <4 x float> undef, float %312, i32 0 ; <<4 x float>> [#uses=2]
  %314 = shufflevector <4 x float> %313, <4 x float> %313, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %315 = load <4 x float>* %zi2                   ; <<4 x float>> [#uses=1]
  %316 = fmul <4 x float> %314, %315              ; <<4 x float>> [#uses=1]
  %317 = fsub <4 x float> %311, %316              ; <<4 x float>> [#uses=1]
  store <4 x float> %317, <4 x float>* %__r1
  %318 = load float* %c2                          ; <float> [#uses=1]
  %319 = insertelement <4 x float> undef, float %318, i32 0 ; <<4 x float>> [#uses=2]
  %320 = shufflevector <4 x float> %319, <4 x float> %319, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %321 = load <4 x float>* %zi2                   ; <<4 x float>> [#uses=1]
  %322 = fmul <4 x float> %320, %321              ; <<4 x float>> [#uses=1]
  %323 = load float* %s2                          ; <float> [#uses=1]
  %324 = insertelement <4 x float> undef, float %323, i32 0 ; <<4 x float>> [#uses=2]
  %325 = shufflevector <4 x float> %324, <4 x float> %324, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %326 = load <4 x float>* %zr2                   ; <<4 x float>> [#uses=1]
  %327 = fmul <4 x float> %325, %326              ; <<4 x float>> [#uses=1]
  %328 = fadd <4 x float> %322, %327              ; <<4 x float>> [#uses=1]
  store <4 x float> %328, <4 x float>* %zi2
  %329 = load <4 x float>* %__r1                  ; <<4 x float>> [#uses=1]
  store <4 x float> %329, <4 x float>* %zr2
  br label %330

; <label>:330                                     ; preds = %306
  %331 = load i32* %tbase                         ; <i32> [#uses=1]
  %332 = mul i32 %331, 3                          ; <i32> [#uses=1]
  store i32 %332, i32* %5
  store float* %c3, float** %6
  %333 = load i32* %5                             ; <i32> [#uses=1]
  %334 = icmp sgt i32 %333, 512                   ; <i1> [#uses=1]
  br i1 %334, label %335, label %k_sincos.exit4

; <label>:335                                     ; preds = %330
  %336 = load i32* %5                             ; <i32> [#uses=1]
  %337 = sub i32 %336, 1024                       ; <i32> [#uses=1]
  store i32 %337, i32* %5
  br label %k_sincos.exit4

k_sincos.exit4:                                   ; preds = %330, %335
  %338 = load i32* %5                             ; <i32> [#uses=1]
  %339 = sitofp i32 %338 to float                 ; <float> [#uses=1]
  %340 = fmul float %339, 0xBF7921FB60000000      ; <float> [#uses=1]
  store float %340, float* %x.i3
  %341 = load float* %x.i3                        ; <float> [#uses=1]
  %342 = call float @_Z10native_cosf(float %341) nounwind ; <float> [#uses=1]
  %343 = load float** %6                          ; <float*> [#uses=1]
  store float %342, float* %343
  %344 = load float* %x.i3                        ; <float> [#uses=1]
  %345 = call float @_Z10native_sinf(float %344) nounwind ; <float> [#uses=1]
  store float %345, float* %4
  %346 = load float* %4                           ; <float> [#uses=1]
  store float %346, float* %s3
  br label %347

; <label>:347                                     ; preds = %k_sincos.exit4
  %348 = load float* %c3                          ; <float> [#uses=1]
  %349 = insertelement <4 x float> undef, float %348, i32 0 ; <<4 x float>> [#uses=2]
  %350 = shufflevector <4 x float> %349, <4 x float> %349, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %351 = load <4 x float>* %zr3                   ; <<4 x float>> [#uses=1]
  %352 = fmul <4 x float> %350, %351              ; <<4 x float>> [#uses=1]
  %353 = load float* %s3                          ; <float> [#uses=1]
  %354 = insertelement <4 x float> undef, float %353, i32 0 ; <<4 x float>> [#uses=2]
  %355 = shufflevector <4 x float> %354, <4 x float> %354, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %356 = load <4 x float>* %zi3                   ; <<4 x float>> [#uses=1]
  %357 = fmul <4 x float> %355, %356              ; <<4 x float>> [#uses=1]
  %358 = fsub <4 x float> %352, %357              ; <<4 x float>> [#uses=1]
  store <4 x float> %358, <4 x float>* %__r2
  %359 = load float* %c3                          ; <float> [#uses=1]
  %360 = insertelement <4 x float> undef, float %359, i32 0 ; <<4 x float>> [#uses=2]
  %361 = shufflevector <4 x float> %360, <4 x float> %360, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %362 = load <4 x float>* %zi3                   ; <<4 x float>> [#uses=1]
  %363 = fmul <4 x float> %361, %362              ; <<4 x float>> [#uses=1]
  %364 = load float* %s3                          ; <float> [#uses=1]
  %365 = insertelement <4 x float> undef, float %364, i32 0 ; <<4 x float>> [#uses=2]
  %366 = shufflevector <4 x float> %365, <4 x float> %365, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %367 = load <4 x float>* %zr3                   ; <<4 x float>> [#uses=1]
  %368 = fmul <4 x float> %366, %367              ; <<4 x float>> [#uses=1]
  %369 = fadd <4 x float> %363, %368              ; <<4 x float>> [#uses=1]
  store <4 x float> %369, <4 x float>* %zi3
  %370 = load <4 x float>* %__r2                  ; <<4 x float>> [#uses=1]
  store <4 x float> %370, <4 x float>* %zr3
  br label %371

; <label>:371                                     ; preds = %347
  br label %372

; <label>:372                                     ; preds = %371
  call void @barrier(i32 1)
  %373 = load float addrspace(3)** %11            ; <float addrspace(3)*> [#uses=1]
  %374 = load i32* %10                            ; <i32> [#uses=1]
  %375 = shl i32 %374, 2                          ; <i32> [#uses=1]
  %376 = load i32* %10                            ; <i32> [#uses=1]
  %377 = lshr i32 %376, 3                         ; <i32> [#uses=1]
  %378 = add i32 %375, %377                       ; <i32> [#uses=1]
  %379 = getelementptr inbounds float addrspace(3)* %373, i32 %378 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %379, float addrspace(3)** %lp
  %380 = load <4 x float>* %zr0                   ; <<4 x float>> [#uses=1]
  %381 = extractelement <4 x float> %380, i32 0   ; <float> [#uses=1]
  %382 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %383 = getelementptr inbounds float addrspace(3)* %382, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %381, float addrspace(3)* %383
  %384 = load <4 x float>* %zr1                   ; <<4 x float>> [#uses=1]
  %385 = extractelement <4 x float> %384, i32 0   ; <float> [#uses=1]
  %386 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %387 = getelementptr inbounds float addrspace(3)* %386, i32 1 ; <float addrspace(3)*> [#uses=1]
  store float %385, float addrspace(3)* %387
  %388 = load <4 x float>* %zr2                   ; <<4 x float>> [#uses=1]
  %389 = extractelement <4 x float> %388, i32 0   ; <float> [#uses=1]
  %390 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %391 = getelementptr inbounds float addrspace(3)* %390, i32 2 ; <float addrspace(3)*> [#uses=1]
  store float %389, float addrspace(3)* %391
  %392 = load <4 x float>* %zr3                   ; <<4 x float>> [#uses=1]
  %393 = extractelement <4 x float> %392, i32 0   ; <float> [#uses=1]
  %394 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %395 = getelementptr inbounds float addrspace(3)* %394, i32 3 ; <float addrspace(3)*> [#uses=1]
  store float %393, float addrspace(3)* %395
  %396 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %397 = getelementptr inbounds float addrspace(3)* %396, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %397, float addrspace(3)** %lp
  %398 = load <4 x float>* %zr0                   ; <<4 x float>> [#uses=1]
  %399 = extractelement <4 x float> %398, i32 1   ; <float> [#uses=1]
  %400 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %401 = getelementptr inbounds float addrspace(3)* %400, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %399, float addrspace(3)* %401
  %402 = load <4 x float>* %zr1                   ; <<4 x float>> [#uses=1]
  %403 = extractelement <4 x float> %402, i32 1   ; <float> [#uses=1]
  %404 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %405 = getelementptr inbounds float addrspace(3)* %404, i32 1 ; <float addrspace(3)*> [#uses=1]
  store float %403, float addrspace(3)* %405
  %406 = load <4 x float>* %zr2                   ; <<4 x float>> [#uses=1]
  %407 = extractelement <4 x float> %406, i32 1   ; <float> [#uses=1]
  %408 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %409 = getelementptr inbounds float addrspace(3)* %408, i32 2 ; <float addrspace(3)*> [#uses=1]
  store float %407, float addrspace(3)* %409
  %410 = load <4 x float>* %zr3                   ; <<4 x float>> [#uses=1]
  %411 = extractelement <4 x float> %410, i32 1   ; <float> [#uses=1]
  %412 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %413 = getelementptr inbounds float addrspace(3)* %412, i32 3 ; <float addrspace(3)*> [#uses=1]
  store float %411, float addrspace(3)* %413
  %414 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %415 = getelementptr inbounds float addrspace(3)* %414, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %415, float addrspace(3)** %lp
  %416 = load <4 x float>* %zr0                   ; <<4 x float>> [#uses=1]
  %417 = extractelement <4 x float> %416, i32 2   ; <float> [#uses=1]
  %418 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %419 = getelementptr inbounds float addrspace(3)* %418, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %417, float addrspace(3)* %419
  %420 = load <4 x float>* %zr1                   ; <<4 x float>> [#uses=1]
  %421 = extractelement <4 x float> %420, i32 2   ; <float> [#uses=1]
  %422 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %423 = getelementptr inbounds float addrspace(3)* %422, i32 1 ; <float addrspace(3)*> [#uses=1]
  store float %421, float addrspace(3)* %423
  %424 = load <4 x float>* %zr2                   ; <<4 x float>> [#uses=1]
  %425 = extractelement <4 x float> %424, i32 2   ; <float> [#uses=1]
  %426 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %427 = getelementptr inbounds float addrspace(3)* %426, i32 2 ; <float addrspace(3)*> [#uses=1]
  store float %425, float addrspace(3)* %427
  %428 = load <4 x float>* %zr3                   ; <<4 x float>> [#uses=1]
  %429 = extractelement <4 x float> %428, i32 2   ; <float> [#uses=1]
  %430 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %431 = getelementptr inbounds float addrspace(3)* %430, i32 3 ; <float addrspace(3)*> [#uses=1]
  store float %429, float addrspace(3)* %431
  %432 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %433 = getelementptr inbounds float addrspace(3)* %432, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %433, float addrspace(3)** %lp
  %434 = load <4 x float>* %zr0                   ; <<4 x float>> [#uses=1]
  %435 = extractelement <4 x float> %434, i32 3   ; <float> [#uses=1]
  %436 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %437 = getelementptr inbounds float addrspace(3)* %436, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %435, float addrspace(3)* %437
  %438 = load <4 x float>* %zr1                   ; <<4 x float>> [#uses=1]
  %439 = extractelement <4 x float> %438, i32 3   ; <float> [#uses=1]
  %440 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %441 = getelementptr inbounds float addrspace(3)* %440, i32 1 ; <float addrspace(3)*> [#uses=1]
  store float %439, float addrspace(3)* %441
  %442 = load <4 x float>* %zr2                   ; <<4 x float>> [#uses=1]
  %443 = extractelement <4 x float> %442, i32 3   ; <float> [#uses=1]
  %444 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %445 = getelementptr inbounds float addrspace(3)* %444, i32 2 ; <float addrspace(3)*> [#uses=1]
  store float %443, float addrspace(3)* %445
  %446 = load <4 x float>* %zr3                   ; <<4 x float>> [#uses=1]
  %447 = extractelement <4 x float> %446, i32 3   ; <float> [#uses=1]
  %448 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %449 = getelementptr inbounds float addrspace(3)* %448, i32 3 ; <float addrspace(3)*> [#uses=1]
  store float %447, float addrspace(3)* %449
  %450 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %451 = getelementptr inbounds float addrspace(3)* %450, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %451, float addrspace(3)** %lp
  %452 = load <4 x float>* %zi0                   ; <<4 x float>> [#uses=1]
  %453 = extractelement <4 x float> %452, i32 0   ; <float> [#uses=1]
  %454 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %455 = getelementptr inbounds float addrspace(3)* %454, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %453, float addrspace(3)* %455
  %456 = load <4 x float>* %zi1                   ; <<4 x float>> [#uses=1]
  %457 = extractelement <4 x float> %456, i32 0   ; <float> [#uses=1]
  %458 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %459 = getelementptr inbounds float addrspace(3)* %458, i32 1 ; <float addrspace(3)*> [#uses=1]
  store float %457, float addrspace(3)* %459
  %460 = load <4 x float>* %zi2                   ; <<4 x float>> [#uses=1]
  %461 = extractelement <4 x float> %460, i32 0   ; <float> [#uses=1]
  %462 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %463 = getelementptr inbounds float addrspace(3)* %462, i32 2 ; <float addrspace(3)*> [#uses=1]
  store float %461, float addrspace(3)* %463
  %464 = load <4 x float>* %zi3                   ; <<4 x float>> [#uses=1]
  %465 = extractelement <4 x float> %464, i32 0   ; <float> [#uses=1]
  %466 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %467 = getelementptr inbounds float addrspace(3)* %466, i32 3 ; <float addrspace(3)*> [#uses=1]
  store float %465, float addrspace(3)* %467
  %468 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %469 = getelementptr inbounds float addrspace(3)* %468, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %469, float addrspace(3)** %lp
  %470 = load <4 x float>* %zi0                   ; <<4 x float>> [#uses=1]
  %471 = extractelement <4 x float> %470, i32 1   ; <float> [#uses=1]
  %472 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %473 = getelementptr inbounds float addrspace(3)* %472, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %471, float addrspace(3)* %473
  %474 = load <4 x float>* %zi1                   ; <<4 x float>> [#uses=1]
  %475 = extractelement <4 x float> %474, i32 1   ; <float> [#uses=1]
  %476 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %477 = getelementptr inbounds float addrspace(3)* %476, i32 1 ; <float addrspace(3)*> [#uses=1]
  store float %475, float addrspace(3)* %477
  %478 = load <4 x float>* %zi2                   ; <<4 x float>> [#uses=1]
  %479 = extractelement <4 x float> %478, i32 1   ; <float> [#uses=1]
  %480 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %481 = getelementptr inbounds float addrspace(3)* %480, i32 2 ; <float addrspace(3)*> [#uses=1]
  store float %479, float addrspace(3)* %481
  %482 = load <4 x float>* %zi3                   ; <<4 x float>> [#uses=1]
  %483 = extractelement <4 x float> %482, i32 1   ; <float> [#uses=1]
  %484 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %485 = getelementptr inbounds float addrspace(3)* %484, i32 3 ; <float addrspace(3)*> [#uses=1]
  store float %483, float addrspace(3)* %485
  %486 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %487 = getelementptr inbounds float addrspace(3)* %486, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %487, float addrspace(3)** %lp
  %488 = load <4 x float>* %zi0                   ; <<4 x float>> [#uses=1]
  %489 = extractelement <4 x float> %488, i32 2   ; <float> [#uses=1]
  %490 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %491 = getelementptr inbounds float addrspace(3)* %490, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %489, float addrspace(3)* %491
  %492 = load <4 x float>* %zi1                   ; <<4 x float>> [#uses=1]
  %493 = extractelement <4 x float> %492, i32 2   ; <float> [#uses=1]
  %494 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %495 = getelementptr inbounds float addrspace(3)* %494, i32 1 ; <float addrspace(3)*> [#uses=1]
  store float %493, float addrspace(3)* %495
  %496 = load <4 x float>* %zi2                   ; <<4 x float>> [#uses=1]
  %497 = extractelement <4 x float> %496, i32 2   ; <float> [#uses=1]
  %498 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %499 = getelementptr inbounds float addrspace(3)* %498, i32 2 ; <float addrspace(3)*> [#uses=1]
  store float %497, float addrspace(3)* %499
  %500 = load <4 x float>* %zi3                   ; <<4 x float>> [#uses=1]
  %501 = extractelement <4 x float> %500, i32 2   ; <float> [#uses=1]
  %502 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %503 = getelementptr inbounds float addrspace(3)* %502, i32 3 ; <float addrspace(3)*> [#uses=1]
  store float %501, float addrspace(3)* %503
  %504 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %505 = getelementptr inbounds float addrspace(3)* %504, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %505, float addrspace(3)** %lp
  %506 = load <4 x float>* %zi0                   ; <<4 x float>> [#uses=1]
  %507 = extractelement <4 x float> %506, i32 3   ; <float> [#uses=1]
  %508 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %509 = getelementptr inbounds float addrspace(3)* %508, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %507, float addrspace(3)* %509
  %510 = load <4 x float>* %zi1                   ; <<4 x float>> [#uses=1]
  %511 = extractelement <4 x float> %510, i32 3   ; <float> [#uses=1]
  %512 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %513 = getelementptr inbounds float addrspace(3)* %512, i32 1 ; <float addrspace(3)*> [#uses=1]
  store float %511, float addrspace(3)* %513
  %514 = load <4 x float>* %zi2                   ; <<4 x float>> [#uses=1]
  %515 = extractelement <4 x float> %514, i32 3   ; <float> [#uses=1]
  %516 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %517 = getelementptr inbounds float addrspace(3)* %516, i32 2 ; <float addrspace(3)*> [#uses=1]
  store float %515, float addrspace(3)* %517
  %518 = load <4 x float>* %zi3                   ; <<4 x float>> [#uses=1]
  %519 = extractelement <4 x float> %518, i32 3   ; <float> [#uses=1]
  %520 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %521 = getelementptr inbounds float addrspace(3)* %520, i32 3 ; <float addrspace(3)*> [#uses=1]
  store float %519, float addrspace(3)* %521
  call void @barrier(i32 1)
  ret void
}

define void @kfft_pass3(i32 %me, float addrspace(3)* %lds) nounwind alwaysinline {
  %1 = alloca float, align 4                      ; <float*> [#uses=2]
  %2 = alloca i32, align 4                        ; <i32*> [#uses=5]
  %3 = alloca float*, align 4                     ; <float**> [#uses=2]
  %x.i5 = alloca float, align 4                   ; <float*> [#uses=3]
  %4 = alloca float, align 4                      ; <float*> [#uses=2]
  %5 = alloca i32, align 4                        ; <i32*> [#uses=5]
  %6 = alloca float*, align 4                     ; <float**> [#uses=2]
  %x.i3 = alloca float, align 4                   ; <float*> [#uses=3]
  %7 = alloca float, align 4                      ; <float*> [#uses=2]
  %8 = alloca i32, align 4                        ; <i32*> [#uses=5]
  %9 = alloca float*, align 4                     ; <float**> [#uses=2]
  %x.i = alloca float, align 4                    ; <float*> [#uses=3]
  %10 = alloca i32, align 4                       ; <i32*> [#uses=5]
  %11 = alloca float addrspace(3)*, align 16      ; <float addrspace(3)**> [#uses=3]
  %lp = alloca float addrspace(3)*, align 16      ; <float addrspace(3)**> [#uses=94]
  %zr0 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=15]
  %zr1 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=18]
  %zr2 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=18]
  %zr3 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=18]
  %zi0 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=15]
  %zi1 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=18]
  %zi2 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=18]
  %zi3 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=18]
  %ar0 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %ar2 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %br0 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %br1 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %br2 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %br3 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %ai0 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %ai2 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %bi0 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %bi1 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %bi2 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %bi3 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %tbase = alloca i32, align 4                    ; <i32*> [#uses=4]
  %c1 = alloca float, align 4                     ; <float*> [#uses=3]
  %s1 = alloca float, align 4                     ; <float*> [#uses=3]
  %__r = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %c2 = alloca float, align 4                     ; <float*> [#uses=3]
  %s2 = alloca float, align 4                     ; <float*> [#uses=3]
  %__r1 = alloca <4 x float>, align 16            ; <<4 x float>*> [#uses=2]
  %c3 = alloca float, align 4                     ; <float*> [#uses=3]
  %s3 = alloca float, align 4                     ; <float*> [#uses=3]
  %__r2 = alloca <4 x float>, align 16            ; <<4 x float>*> [#uses=2]
  store i32 %me, i32* %10
  store float addrspace(3)* %lds, float addrspace(3)** %11
  %12 = load float addrspace(3)** %11             ; <float addrspace(3)*> [#uses=1]
  %13 = load i32* %10                             ; <i32> [#uses=1]
  %14 = load i32* %10                             ; <i32> [#uses=1]
  %15 = lshr i32 %14, 5                           ; <i32> [#uses=1]
  %16 = add i32 %13, %15                          ; <i32> [#uses=1]
  %17 = getelementptr inbounds float addrspace(3)* %12, i32 %16 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %17, float addrspace(3)** %lp
  %18 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %19 = getelementptr inbounds float addrspace(3)* %18, i32 0 ; <float addrspace(3)*> [#uses=1]
  %20 = load float addrspace(3)* %19              ; <float> [#uses=1]
  %21 = load <4 x float>* %zr0                    ; <<4 x float>> [#uses=1]
  %22 = insertelement <4 x float> %21, float %20, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %22, <4 x float>* %zr0
  %23 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %24 = getelementptr inbounds float addrspace(3)* %23, i32 66 ; <float addrspace(3)*> [#uses=1]
  %25 = load float addrspace(3)* %24              ; <float> [#uses=1]
  %26 = load <4 x float>* %zr1                    ; <<4 x float>> [#uses=1]
  %27 = insertelement <4 x float> %26, float %25, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %27, <4 x float>* %zr1
  %28 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %29 = getelementptr inbounds float addrspace(3)* %28, i32 132 ; <float addrspace(3)*> [#uses=1]
  %30 = load float addrspace(3)* %29              ; <float> [#uses=1]
  %31 = load <4 x float>* %zr2                    ; <<4 x float>> [#uses=1]
  %32 = insertelement <4 x float> %31, float %30, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %32, <4 x float>* %zr2
  %33 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %34 = getelementptr inbounds float addrspace(3)* %33, i32 198 ; <float addrspace(3)*> [#uses=1]
  %35 = load float addrspace(3)* %34              ; <float> [#uses=1]
  %36 = load <4 x float>* %zr3                    ; <<4 x float>> [#uses=1]
  %37 = insertelement <4 x float> %36, float %35, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %37, <4 x float>* %zr3
  %38 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %39 = getelementptr inbounds float addrspace(3)* %38, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %39, float addrspace(3)** %lp
  %40 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %41 = getelementptr inbounds float addrspace(3)* %40, i32 0 ; <float addrspace(3)*> [#uses=1]
  %42 = load float addrspace(3)* %41              ; <float> [#uses=1]
  %43 = load <4 x float>* %zr0                    ; <<4 x float>> [#uses=1]
  %44 = insertelement <4 x float> %43, float %42, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %44, <4 x float>* %zr0
  %45 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %46 = getelementptr inbounds float addrspace(3)* %45, i32 66 ; <float addrspace(3)*> [#uses=1]
  %47 = load float addrspace(3)* %46              ; <float> [#uses=1]
  %48 = load <4 x float>* %zr1                    ; <<4 x float>> [#uses=1]
  %49 = insertelement <4 x float> %48, float %47, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %49, <4 x float>* %zr1
  %50 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %51 = getelementptr inbounds float addrspace(3)* %50, i32 132 ; <float addrspace(3)*> [#uses=1]
  %52 = load float addrspace(3)* %51              ; <float> [#uses=1]
  %53 = load <4 x float>* %zr2                    ; <<4 x float>> [#uses=1]
  %54 = insertelement <4 x float> %53, float %52, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %54, <4 x float>* %zr2
  %55 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %56 = getelementptr inbounds float addrspace(3)* %55, i32 198 ; <float addrspace(3)*> [#uses=1]
  %57 = load float addrspace(3)* %56              ; <float> [#uses=1]
  %58 = load <4 x float>* %zr3                    ; <<4 x float>> [#uses=1]
  %59 = insertelement <4 x float> %58, float %57, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %59, <4 x float>* %zr3
  %60 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %61 = getelementptr inbounds float addrspace(3)* %60, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %61, float addrspace(3)** %lp
  %62 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %63 = getelementptr inbounds float addrspace(3)* %62, i32 0 ; <float addrspace(3)*> [#uses=1]
  %64 = load float addrspace(3)* %63              ; <float> [#uses=1]
  %65 = load <4 x float>* %zr0                    ; <<4 x float>> [#uses=1]
  %66 = insertelement <4 x float> %65, float %64, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %66, <4 x float>* %zr0
  %67 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %68 = getelementptr inbounds float addrspace(3)* %67, i32 66 ; <float addrspace(3)*> [#uses=1]
  %69 = load float addrspace(3)* %68              ; <float> [#uses=1]
  %70 = load <4 x float>* %zr1                    ; <<4 x float>> [#uses=1]
  %71 = insertelement <4 x float> %70, float %69, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %71, <4 x float>* %zr1
  %72 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %73 = getelementptr inbounds float addrspace(3)* %72, i32 132 ; <float addrspace(3)*> [#uses=1]
  %74 = load float addrspace(3)* %73              ; <float> [#uses=1]
  %75 = load <4 x float>* %zr2                    ; <<4 x float>> [#uses=1]
  %76 = insertelement <4 x float> %75, float %74, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %76, <4 x float>* %zr2
  %77 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %78 = getelementptr inbounds float addrspace(3)* %77, i32 198 ; <float addrspace(3)*> [#uses=1]
  %79 = load float addrspace(3)* %78              ; <float> [#uses=1]
  %80 = load <4 x float>* %zr3                    ; <<4 x float>> [#uses=1]
  %81 = insertelement <4 x float> %80, float %79, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %81, <4 x float>* %zr3
  %82 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %83 = getelementptr inbounds float addrspace(3)* %82, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %83, float addrspace(3)** %lp
  %84 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %85 = getelementptr inbounds float addrspace(3)* %84, i32 0 ; <float addrspace(3)*> [#uses=1]
  %86 = load float addrspace(3)* %85              ; <float> [#uses=1]
  %87 = load <4 x float>* %zr0                    ; <<4 x float>> [#uses=1]
  %88 = insertelement <4 x float> %87, float %86, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %88, <4 x float>* %zr0
  %89 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %90 = getelementptr inbounds float addrspace(3)* %89, i32 66 ; <float addrspace(3)*> [#uses=1]
  %91 = load float addrspace(3)* %90              ; <float> [#uses=1]
  %92 = load <4 x float>* %zr1                    ; <<4 x float>> [#uses=1]
  %93 = insertelement <4 x float> %92, float %91, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %93, <4 x float>* %zr1
  %94 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %95 = getelementptr inbounds float addrspace(3)* %94, i32 132 ; <float addrspace(3)*> [#uses=1]
  %96 = load float addrspace(3)* %95              ; <float> [#uses=1]
  %97 = load <4 x float>* %zr2                    ; <<4 x float>> [#uses=1]
  %98 = insertelement <4 x float> %97, float %96, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %98, <4 x float>* %zr2
  %99 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %100 = getelementptr inbounds float addrspace(3)* %99, i32 198 ; <float addrspace(3)*> [#uses=1]
  %101 = load float addrspace(3)* %100            ; <float> [#uses=1]
  %102 = load <4 x float>* %zr3                   ; <<4 x float>> [#uses=1]
  %103 = insertelement <4 x float> %102, float %101, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %103, <4 x float>* %zr3
  %104 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %105 = getelementptr inbounds float addrspace(3)* %104, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %105, float addrspace(3)** %lp
  %106 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %107 = getelementptr inbounds float addrspace(3)* %106, i32 0 ; <float addrspace(3)*> [#uses=1]
  %108 = load float addrspace(3)* %107            ; <float> [#uses=1]
  %109 = load <4 x float>* %zi0                   ; <<4 x float>> [#uses=1]
  %110 = insertelement <4 x float> %109, float %108, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %110, <4 x float>* %zi0
  %111 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %112 = getelementptr inbounds float addrspace(3)* %111, i32 66 ; <float addrspace(3)*> [#uses=1]
  %113 = load float addrspace(3)* %112            ; <float> [#uses=1]
  %114 = load <4 x float>* %zi1                   ; <<4 x float>> [#uses=1]
  %115 = insertelement <4 x float> %114, float %113, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %115, <4 x float>* %zi1
  %116 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %117 = getelementptr inbounds float addrspace(3)* %116, i32 132 ; <float addrspace(3)*> [#uses=1]
  %118 = load float addrspace(3)* %117            ; <float> [#uses=1]
  %119 = load <4 x float>* %zi2                   ; <<4 x float>> [#uses=1]
  %120 = insertelement <4 x float> %119, float %118, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %120, <4 x float>* %zi2
  %121 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %122 = getelementptr inbounds float addrspace(3)* %121, i32 198 ; <float addrspace(3)*> [#uses=1]
  %123 = load float addrspace(3)* %122            ; <float> [#uses=1]
  %124 = load <4 x float>* %zi3                   ; <<4 x float>> [#uses=1]
  %125 = insertelement <4 x float> %124, float %123, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %125, <4 x float>* %zi3
  %126 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %127 = getelementptr inbounds float addrspace(3)* %126, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %127, float addrspace(3)** %lp
  %128 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %129 = getelementptr inbounds float addrspace(3)* %128, i32 0 ; <float addrspace(3)*> [#uses=1]
  %130 = load float addrspace(3)* %129            ; <float> [#uses=1]
  %131 = load <4 x float>* %zi0                   ; <<4 x float>> [#uses=1]
  %132 = insertelement <4 x float> %131, float %130, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %132, <4 x float>* %zi0
  %133 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %134 = getelementptr inbounds float addrspace(3)* %133, i32 66 ; <float addrspace(3)*> [#uses=1]
  %135 = load float addrspace(3)* %134            ; <float> [#uses=1]
  %136 = load <4 x float>* %zi1                   ; <<4 x float>> [#uses=1]
  %137 = insertelement <4 x float> %136, float %135, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %137, <4 x float>* %zi1
  %138 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %139 = getelementptr inbounds float addrspace(3)* %138, i32 132 ; <float addrspace(3)*> [#uses=1]
  %140 = load float addrspace(3)* %139            ; <float> [#uses=1]
  %141 = load <4 x float>* %zi2                   ; <<4 x float>> [#uses=1]
  %142 = insertelement <4 x float> %141, float %140, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %142, <4 x float>* %zi2
  %143 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %144 = getelementptr inbounds float addrspace(3)* %143, i32 198 ; <float addrspace(3)*> [#uses=1]
  %145 = load float addrspace(3)* %144            ; <float> [#uses=1]
  %146 = load <4 x float>* %zi3                   ; <<4 x float>> [#uses=1]
  %147 = insertelement <4 x float> %146, float %145, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %147, <4 x float>* %zi3
  %148 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %149 = getelementptr inbounds float addrspace(3)* %148, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %149, float addrspace(3)** %lp
  %150 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %151 = getelementptr inbounds float addrspace(3)* %150, i32 0 ; <float addrspace(3)*> [#uses=1]
  %152 = load float addrspace(3)* %151            ; <float> [#uses=1]
  %153 = load <4 x float>* %zi0                   ; <<4 x float>> [#uses=1]
  %154 = insertelement <4 x float> %153, float %152, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %154, <4 x float>* %zi0
  %155 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %156 = getelementptr inbounds float addrspace(3)* %155, i32 66 ; <float addrspace(3)*> [#uses=1]
  %157 = load float addrspace(3)* %156            ; <float> [#uses=1]
  %158 = load <4 x float>* %zi1                   ; <<4 x float>> [#uses=1]
  %159 = insertelement <4 x float> %158, float %157, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %159, <4 x float>* %zi1
  %160 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %161 = getelementptr inbounds float addrspace(3)* %160, i32 132 ; <float addrspace(3)*> [#uses=1]
  %162 = load float addrspace(3)* %161            ; <float> [#uses=1]
  %163 = load <4 x float>* %zi2                   ; <<4 x float>> [#uses=1]
  %164 = insertelement <4 x float> %163, float %162, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %164, <4 x float>* %zi2
  %165 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %166 = getelementptr inbounds float addrspace(3)* %165, i32 198 ; <float addrspace(3)*> [#uses=1]
  %167 = load float addrspace(3)* %166            ; <float> [#uses=1]
  %168 = load <4 x float>* %zi3                   ; <<4 x float>> [#uses=1]
  %169 = insertelement <4 x float> %168, float %167, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %169, <4 x float>* %zi3
  %170 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %171 = getelementptr inbounds float addrspace(3)* %170, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %171, float addrspace(3)** %lp
  %172 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %173 = getelementptr inbounds float addrspace(3)* %172, i32 0 ; <float addrspace(3)*> [#uses=1]
  %174 = load float addrspace(3)* %173            ; <float> [#uses=1]
  %175 = load <4 x float>* %zi0                   ; <<4 x float>> [#uses=1]
  %176 = insertelement <4 x float> %175, float %174, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %176, <4 x float>* %zi0
  %177 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %178 = getelementptr inbounds float addrspace(3)* %177, i32 66 ; <float addrspace(3)*> [#uses=1]
  %179 = load float addrspace(3)* %178            ; <float> [#uses=1]
  %180 = load <4 x float>* %zi1                   ; <<4 x float>> [#uses=1]
  %181 = insertelement <4 x float> %180, float %179, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %181, <4 x float>* %zi1
  %182 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %183 = getelementptr inbounds float addrspace(3)* %182, i32 132 ; <float addrspace(3)*> [#uses=1]
  %184 = load float addrspace(3)* %183            ; <float> [#uses=1]
  %185 = load <4 x float>* %zi2                   ; <<4 x float>> [#uses=1]
  %186 = insertelement <4 x float> %185, float %184, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %186, <4 x float>* %zi2
  %187 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %188 = getelementptr inbounds float addrspace(3)* %187, i32 198 ; <float addrspace(3)*> [#uses=1]
  %189 = load float addrspace(3)* %188            ; <float> [#uses=1]
  %190 = load <4 x float>* %zi3                   ; <<4 x float>> [#uses=1]
  %191 = insertelement <4 x float> %190, float %189, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %191, <4 x float>* %zi3
  br label %192

; <label>:192                                     ; preds = %0
  %193 = load <4 x float>* %zr0                   ; <<4 x float>> [#uses=1]
  %194 = load <4 x float>* %zr2                   ; <<4 x float>> [#uses=1]
  %195 = fadd <4 x float> %193, %194              ; <<4 x float>> [#uses=1]
  store <4 x float> %195, <4 x float>* %ar0
  %196 = load <4 x float>* %zr1                   ; <<4 x float>> [#uses=1]
  %197 = load <4 x float>* %zr3                   ; <<4 x float>> [#uses=1]
  %198 = fadd <4 x float> %196, %197              ; <<4 x float>> [#uses=1]
  store <4 x float> %198, <4 x float>* %ar2
  %199 = load <4 x float>* %ar0                   ; <<4 x float>> [#uses=1]
  %200 = load <4 x float>* %ar2                   ; <<4 x float>> [#uses=1]
  %201 = fadd <4 x float> %199, %200              ; <<4 x float>> [#uses=1]
  store <4 x float> %201, <4 x float>* %br0
  %202 = load <4 x float>* %zr0                   ; <<4 x float>> [#uses=1]
  %203 = load <4 x float>* %zr2                   ; <<4 x float>> [#uses=1]
  %204 = fsub <4 x float> %202, %203              ; <<4 x float>> [#uses=1]
  store <4 x float> %204, <4 x float>* %br1
  %205 = load <4 x float>* %ar0                   ; <<4 x float>> [#uses=1]
  %206 = load <4 x float>* %ar2                   ; <<4 x float>> [#uses=1]
  %207 = fsub <4 x float> %205, %206              ; <<4 x float>> [#uses=1]
  store <4 x float> %207, <4 x float>* %br2
  %208 = load <4 x float>* %zr1                   ; <<4 x float>> [#uses=1]
  %209 = load <4 x float>* %zr3                   ; <<4 x float>> [#uses=1]
  %210 = fsub <4 x float> %208, %209              ; <<4 x float>> [#uses=1]
  store <4 x float> %210, <4 x float>* %br3
  %211 = load <4 x float>* %zi0                   ; <<4 x float>> [#uses=1]
  %212 = load <4 x float>* %zi2                   ; <<4 x float>> [#uses=1]
  %213 = fadd <4 x float> %211, %212              ; <<4 x float>> [#uses=1]
  store <4 x float> %213, <4 x float>* %ai0
  %214 = load <4 x float>* %zi1                   ; <<4 x float>> [#uses=1]
  %215 = load <4 x float>* %zi3                   ; <<4 x float>> [#uses=1]
  %216 = fadd <4 x float> %214, %215              ; <<4 x float>> [#uses=1]
  store <4 x float> %216, <4 x float>* %ai2
  %217 = load <4 x float>* %ai0                   ; <<4 x float>> [#uses=1]
  %218 = load <4 x float>* %ai2                   ; <<4 x float>> [#uses=1]
  %219 = fadd <4 x float> %217, %218              ; <<4 x float>> [#uses=1]
  store <4 x float> %219, <4 x float>* %bi0
  %220 = load <4 x float>* %zi0                   ; <<4 x float>> [#uses=1]
  %221 = load <4 x float>* %zi2                   ; <<4 x float>> [#uses=1]
  %222 = fsub <4 x float> %220, %221              ; <<4 x float>> [#uses=1]
  store <4 x float> %222, <4 x float>* %bi1
  %223 = load <4 x float>* %ai0                   ; <<4 x float>> [#uses=1]
  %224 = load <4 x float>* %ai2                   ; <<4 x float>> [#uses=1]
  %225 = fsub <4 x float> %223, %224              ; <<4 x float>> [#uses=1]
  store <4 x float> %225, <4 x float>* %bi2
  %226 = load <4 x float>* %zi1                   ; <<4 x float>> [#uses=1]
  %227 = load <4 x float>* %zi3                   ; <<4 x float>> [#uses=1]
  %228 = fsub <4 x float> %226, %227              ; <<4 x float>> [#uses=1]
  store <4 x float> %228, <4 x float>* %bi3
  %229 = load <4 x float>* %br0                   ; <<4 x float>> [#uses=1]
  store <4 x float> %229, <4 x float>* %zr0
  %230 = load <4 x float>* %bi0                   ; <<4 x float>> [#uses=1]
  store <4 x float> %230, <4 x float>* %zi0
  %231 = load <4 x float>* %br1                   ; <<4 x float>> [#uses=1]
  %232 = load <4 x float>* %bi3                   ; <<4 x float>> [#uses=1]
  %233 = fadd <4 x float> %231, %232              ; <<4 x float>> [#uses=1]
  store <4 x float> %233, <4 x float>* %zr1
  %234 = load <4 x float>* %bi1                   ; <<4 x float>> [#uses=1]
  %235 = load <4 x float>* %br3                   ; <<4 x float>> [#uses=1]
  %236 = fsub <4 x float> %234, %235              ; <<4 x float>> [#uses=1]
  store <4 x float> %236, <4 x float>* %zi1
  %237 = load <4 x float>* %br1                   ; <<4 x float>> [#uses=1]
  %238 = load <4 x float>* %bi3                   ; <<4 x float>> [#uses=1]
  %239 = fsub <4 x float> %237, %238              ; <<4 x float>> [#uses=1]
  store <4 x float> %239, <4 x float>* %zr3
  %240 = load <4 x float>* %br3                   ; <<4 x float>> [#uses=1]
  %241 = load <4 x float>* %bi1                   ; <<4 x float>> [#uses=1]
  %242 = fadd <4 x float> %240, %241              ; <<4 x float>> [#uses=1]
  store <4 x float> %242, <4 x float>* %zi3
  %243 = load <4 x float>* %br2                   ; <<4 x float>> [#uses=1]
  store <4 x float> %243, <4 x float>* %zr2
  %244 = load <4 x float>* %bi2                   ; <<4 x float>> [#uses=1]
  store <4 x float> %244, <4 x float>* %zi2
  br label %245

; <label>:245                                     ; preds = %192
  %246 = load i32* %10                            ; <i32> [#uses=1]
  %247 = lshr i32 %246, 2                         ; <i32> [#uses=1]
  %248 = shl i32 %247, 4                          ; <i32> [#uses=1]
  store i32 %248, i32* %tbase
  br label %249

; <label>:249                                     ; preds = %245
  %250 = load i32* %tbase                         ; <i32> [#uses=1]
  %251 = mul i32 %250, 1                          ; <i32> [#uses=1]
  store i32 %251, i32* %8
  store float* %c1, float** %9
  %252 = load i32* %8                             ; <i32> [#uses=1]
  %253 = icmp sgt i32 %252, 512                   ; <i1> [#uses=1]
  br i1 %253, label %254, label %k_sincos.exit

; <label>:254                                     ; preds = %249
  %255 = load i32* %8                             ; <i32> [#uses=1]
  %256 = sub i32 %255, 1024                       ; <i32> [#uses=1]
  store i32 %256, i32* %8
  br label %k_sincos.exit

k_sincos.exit:                                    ; preds = %249, %254
  %257 = load i32* %8                             ; <i32> [#uses=1]
  %258 = sitofp i32 %257 to float                 ; <float> [#uses=1]
  %259 = fmul float %258, 0xBF7921FB60000000      ; <float> [#uses=1]
  store float %259, float* %x.i
  %260 = load float* %x.i                         ; <float> [#uses=1]
  %261 = call float @_Z10native_cosf(float %260) nounwind ; <float> [#uses=1]
  %262 = load float** %9                          ; <float*> [#uses=1]
  store float %261, float* %262
  %263 = load float* %x.i                         ; <float> [#uses=1]
  %264 = call float @_Z10native_sinf(float %263) nounwind ; <float> [#uses=1]
  store float %264, float* %7
  %265 = load float* %7                           ; <float> [#uses=1]
  store float %265, float* %s1
  br label %266

; <label>:266                                     ; preds = %k_sincos.exit
  %267 = load float* %c1                          ; <float> [#uses=1]
  %268 = insertelement <4 x float> undef, float %267, i32 0 ; <<4 x float>> [#uses=2]
  %269 = shufflevector <4 x float> %268, <4 x float> %268, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %270 = load <4 x float>* %zr1                   ; <<4 x float>> [#uses=1]
  %271 = fmul <4 x float> %269, %270              ; <<4 x float>> [#uses=1]
  %272 = load float* %s1                          ; <float> [#uses=1]
  %273 = insertelement <4 x float> undef, float %272, i32 0 ; <<4 x float>> [#uses=2]
  %274 = shufflevector <4 x float> %273, <4 x float> %273, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %275 = load <4 x float>* %zi1                   ; <<4 x float>> [#uses=1]
  %276 = fmul <4 x float> %274, %275              ; <<4 x float>> [#uses=1]
  %277 = fsub <4 x float> %271, %276              ; <<4 x float>> [#uses=1]
  store <4 x float> %277, <4 x float>* %__r
  %278 = load float* %c1                          ; <float> [#uses=1]
  %279 = insertelement <4 x float> undef, float %278, i32 0 ; <<4 x float>> [#uses=2]
  %280 = shufflevector <4 x float> %279, <4 x float> %279, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %281 = load <4 x float>* %zi1                   ; <<4 x float>> [#uses=1]
  %282 = fmul <4 x float> %280, %281              ; <<4 x float>> [#uses=1]
  %283 = load float* %s1                          ; <float> [#uses=1]
  %284 = insertelement <4 x float> undef, float %283, i32 0 ; <<4 x float>> [#uses=2]
  %285 = shufflevector <4 x float> %284, <4 x float> %284, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %286 = load <4 x float>* %zr1                   ; <<4 x float>> [#uses=1]
  %287 = fmul <4 x float> %285, %286              ; <<4 x float>> [#uses=1]
  %288 = fadd <4 x float> %282, %287              ; <<4 x float>> [#uses=1]
  store <4 x float> %288, <4 x float>* %zi1
  %289 = load <4 x float>* %__r                   ; <<4 x float>> [#uses=1]
  store <4 x float> %289, <4 x float>* %zr1
  br label %290

; <label>:290                                     ; preds = %266
  %291 = load i32* %tbase                         ; <i32> [#uses=1]
  %292 = mul i32 %291, 2                          ; <i32> [#uses=1]
  store i32 %292, i32* %2
  store float* %c2, float** %3
  %293 = load i32* %2                             ; <i32> [#uses=1]
  %294 = icmp sgt i32 %293, 512                   ; <i1> [#uses=1]
  br i1 %294, label %295, label %k_sincos.exit6

; <label>:295                                     ; preds = %290
  %296 = load i32* %2                             ; <i32> [#uses=1]
  %297 = sub i32 %296, 1024                       ; <i32> [#uses=1]
  store i32 %297, i32* %2
  br label %k_sincos.exit6

k_sincos.exit6:                                   ; preds = %290, %295
  %298 = load i32* %2                             ; <i32> [#uses=1]
  %299 = sitofp i32 %298 to float                 ; <float> [#uses=1]
  %300 = fmul float %299, 0xBF7921FB60000000      ; <float> [#uses=1]
  store float %300, float* %x.i5
  %301 = load float* %x.i5                        ; <float> [#uses=1]
  %302 = call float @_Z10native_cosf(float %301) nounwind ; <float> [#uses=1]
  %303 = load float** %3                          ; <float*> [#uses=1]
  store float %302, float* %303
  %304 = load float* %x.i5                        ; <float> [#uses=1]
  %305 = call float @_Z10native_sinf(float %304) nounwind ; <float> [#uses=1]
  store float %305, float* %1
  %306 = load float* %1                           ; <float> [#uses=1]
  store float %306, float* %s2
  br label %307

; <label>:307                                     ; preds = %k_sincos.exit6
  %308 = load float* %c2                          ; <float> [#uses=1]
  %309 = insertelement <4 x float> undef, float %308, i32 0 ; <<4 x float>> [#uses=2]
  %310 = shufflevector <4 x float> %309, <4 x float> %309, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %311 = load <4 x float>* %zr2                   ; <<4 x float>> [#uses=1]
  %312 = fmul <4 x float> %310, %311              ; <<4 x float>> [#uses=1]
  %313 = load float* %s2                          ; <float> [#uses=1]
  %314 = insertelement <4 x float> undef, float %313, i32 0 ; <<4 x float>> [#uses=2]
  %315 = shufflevector <4 x float> %314, <4 x float> %314, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %316 = load <4 x float>* %zi2                   ; <<4 x float>> [#uses=1]
  %317 = fmul <4 x float> %315, %316              ; <<4 x float>> [#uses=1]
  %318 = fsub <4 x float> %312, %317              ; <<4 x float>> [#uses=1]
  store <4 x float> %318, <4 x float>* %__r1
  %319 = load float* %c2                          ; <float> [#uses=1]
  %320 = insertelement <4 x float> undef, float %319, i32 0 ; <<4 x float>> [#uses=2]
  %321 = shufflevector <4 x float> %320, <4 x float> %320, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %322 = load <4 x float>* %zi2                   ; <<4 x float>> [#uses=1]
  %323 = fmul <4 x float> %321, %322              ; <<4 x float>> [#uses=1]
  %324 = load float* %s2                          ; <float> [#uses=1]
  %325 = insertelement <4 x float> undef, float %324, i32 0 ; <<4 x float>> [#uses=2]
  %326 = shufflevector <4 x float> %325, <4 x float> %325, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %327 = load <4 x float>* %zr2                   ; <<4 x float>> [#uses=1]
  %328 = fmul <4 x float> %326, %327              ; <<4 x float>> [#uses=1]
  %329 = fadd <4 x float> %323, %328              ; <<4 x float>> [#uses=1]
  store <4 x float> %329, <4 x float>* %zi2
  %330 = load <4 x float>* %__r1                  ; <<4 x float>> [#uses=1]
  store <4 x float> %330, <4 x float>* %zr2
  br label %331

; <label>:331                                     ; preds = %307
  %332 = load i32* %tbase                         ; <i32> [#uses=1]
  %333 = mul i32 %332, 3                          ; <i32> [#uses=1]
  store i32 %333, i32* %5
  store float* %c3, float** %6
  %334 = load i32* %5                             ; <i32> [#uses=1]
  %335 = icmp sgt i32 %334, 512                   ; <i1> [#uses=1]
  br i1 %335, label %336, label %k_sincos.exit4

; <label>:336                                     ; preds = %331
  %337 = load i32* %5                             ; <i32> [#uses=1]
  %338 = sub i32 %337, 1024                       ; <i32> [#uses=1]
  store i32 %338, i32* %5
  br label %k_sincos.exit4

k_sincos.exit4:                                   ; preds = %331, %336
  %339 = load i32* %5                             ; <i32> [#uses=1]
  %340 = sitofp i32 %339 to float                 ; <float> [#uses=1]
  %341 = fmul float %340, 0xBF7921FB60000000      ; <float> [#uses=1]
  store float %341, float* %x.i3
  %342 = load float* %x.i3                        ; <float> [#uses=1]
  %343 = call float @_Z10native_cosf(float %342) nounwind ; <float> [#uses=1]
  %344 = load float** %6                          ; <float*> [#uses=1]
  store float %343, float* %344
  %345 = load float* %x.i3                        ; <float> [#uses=1]
  %346 = call float @_Z10native_sinf(float %345) nounwind ; <float> [#uses=1]
  store float %346, float* %4
  %347 = load float* %4                           ; <float> [#uses=1]
  store float %347, float* %s3
  br label %348

; <label>:348                                     ; preds = %k_sincos.exit4
  %349 = load float* %c3                          ; <float> [#uses=1]
  %350 = insertelement <4 x float> undef, float %349, i32 0 ; <<4 x float>> [#uses=2]
  %351 = shufflevector <4 x float> %350, <4 x float> %350, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %352 = load <4 x float>* %zr3                   ; <<4 x float>> [#uses=1]
  %353 = fmul <4 x float> %351, %352              ; <<4 x float>> [#uses=1]
  %354 = load float* %s3                          ; <float> [#uses=1]
  %355 = insertelement <4 x float> undef, float %354, i32 0 ; <<4 x float>> [#uses=2]
  %356 = shufflevector <4 x float> %355, <4 x float> %355, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %357 = load <4 x float>* %zi3                   ; <<4 x float>> [#uses=1]
  %358 = fmul <4 x float> %356, %357              ; <<4 x float>> [#uses=1]
  %359 = fsub <4 x float> %353, %358              ; <<4 x float>> [#uses=1]
  store <4 x float> %359, <4 x float>* %__r2
  %360 = load float* %c3                          ; <float> [#uses=1]
  %361 = insertelement <4 x float> undef, float %360, i32 0 ; <<4 x float>> [#uses=2]
  %362 = shufflevector <4 x float> %361, <4 x float> %361, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %363 = load <4 x float>* %zi3                   ; <<4 x float>> [#uses=1]
  %364 = fmul <4 x float> %362, %363              ; <<4 x float>> [#uses=1]
  %365 = load float* %s3                          ; <float> [#uses=1]
  %366 = insertelement <4 x float> undef, float %365, i32 0 ; <<4 x float>> [#uses=2]
  %367 = shufflevector <4 x float> %366, <4 x float> %366, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %368 = load <4 x float>* %zr3                   ; <<4 x float>> [#uses=1]
  %369 = fmul <4 x float> %367, %368              ; <<4 x float>> [#uses=1]
  %370 = fadd <4 x float> %364, %369              ; <<4 x float>> [#uses=1]
  store <4 x float> %370, <4 x float>* %zi3
  %371 = load <4 x float>* %__r2                  ; <<4 x float>> [#uses=1]
  store <4 x float> %371, <4 x float>* %zr3
  br label %372

; <label>:372                                     ; preds = %348
  br label %373

; <label>:373                                     ; preds = %372
  call void @barrier(i32 1)
  %374 = load float addrspace(3)** %11            ; <float addrspace(3)*> [#uses=1]
  %375 = load i32* %10                            ; <i32> [#uses=1]
  %376 = getelementptr inbounds float addrspace(3)* %374, i32 %375 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %376, float addrspace(3)** %lp
  %377 = load <4 x float>* %zr0                   ; <<4 x float>> [#uses=1]
  %378 = extractelement <4 x float> %377, i32 0   ; <float> [#uses=1]
  %379 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %380 = getelementptr inbounds float addrspace(3)* %379, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %378, float addrspace(3)* %380
  %381 = load <4 x float>* %zr0                   ; <<4 x float>> [#uses=1]
  %382 = extractelement <4 x float> %381, i32 1   ; <float> [#uses=1]
  %383 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %384 = getelementptr inbounds float addrspace(3)* %383, i32 66 ; <float addrspace(3)*> [#uses=1]
  store float %382, float addrspace(3)* %384
  %385 = load <4 x float>* %zr0                   ; <<4 x float>> [#uses=1]
  %386 = extractelement <4 x float> %385, i32 2   ; <float> [#uses=1]
  %387 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %388 = getelementptr inbounds float addrspace(3)* %387, i32 132 ; <float addrspace(3)*> [#uses=1]
  store float %386, float addrspace(3)* %388
  %389 = load <4 x float>* %zr0                   ; <<4 x float>> [#uses=1]
  %390 = extractelement <4 x float> %389, i32 3   ; <float> [#uses=1]
  %391 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %392 = getelementptr inbounds float addrspace(3)* %391, i32 198 ; <float addrspace(3)*> [#uses=1]
  store float %390, float addrspace(3)* %392
  %393 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %394 = getelementptr inbounds float addrspace(3)* %393, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %394, float addrspace(3)** %lp
  %395 = load <4 x float>* %zr1                   ; <<4 x float>> [#uses=1]
  %396 = extractelement <4 x float> %395, i32 0   ; <float> [#uses=1]
  %397 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %398 = getelementptr inbounds float addrspace(3)* %397, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %396, float addrspace(3)* %398
  %399 = load <4 x float>* %zr1                   ; <<4 x float>> [#uses=1]
  %400 = extractelement <4 x float> %399, i32 1   ; <float> [#uses=1]
  %401 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %402 = getelementptr inbounds float addrspace(3)* %401, i32 66 ; <float addrspace(3)*> [#uses=1]
  store float %400, float addrspace(3)* %402
  %403 = load <4 x float>* %zr1                   ; <<4 x float>> [#uses=1]
  %404 = extractelement <4 x float> %403, i32 2   ; <float> [#uses=1]
  %405 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %406 = getelementptr inbounds float addrspace(3)* %405, i32 132 ; <float addrspace(3)*> [#uses=1]
  store float %404, float addrspace(3)* %406
  %407 = load <4 x float>* %zr1                   ; <<4 x float>> [#uses=1]
  %408 = extractelement <4 x float> %407, i32 3   ; <float> [#uses=1]
  %409 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %410 = getelementptr inbounds float addrspace(3)* %409, i32 198 ; <float addrspace(3)*> [#uses=1]
  store float %408, float addrspace(3)* %410
  %411 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %412 = getelementptr inbounds float addrspace(3)* %411, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %412, float addrspace(3)** %lp
  %413 = load <4 x float>* %zr2                   ; <<4 x float>> [#uses=1]
  %414 = extractelement <4 x float> %413, i32 0   ; <float> [#uses=1]
  %415 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %416 = getelementptr inbounds float addrspace(3)* %415, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %414, float addrspace(3)* %416
  %417 = load <4 x float>* %zr2                   ; <<4 x float>> [#uses=1]
  %418 = extractelement <4 x float> %417, i32 1   ; <float> [#uses=1]
  %419 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %420 = getelementptr inbounds float addrspace(3)* %419, i32 66 ; <float addrspace(3)*> [#uses=1]
  store float %418, float addrspace(3)* %420
  %421 = load <4 x float>* %zr2                   ; <<4 x float>> [#uses=1]
  %422 = extractelement <4 x float> %421, i32 2   ; <float> [#uses=1]
  %423 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %424 = getelementptr inbounds float addrspace(3)* %423, i32 132 ; <float addrspace(3)*> [#uses=1]
  store float %422, float addrspace(3)* %424
  %425 = load <4 x float>* %zr2                   ; <<4 x float>> [#uses=1]
  %426 = extractelement <4 x float> %425, i32 3   ; <float> [#uses=1]
  %427 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %428 = getelementptr inbounds float addrspace(3)* %427, i32 198 ; <float addrspace(3)*> [#uses=1]
  store float %426, float addrspace(3)* %428
  %429 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %430 = getelementptr inbounds float addrspace(3)* %429, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %430, float addrspace(3)** %lp
  %431 = load <4 x float>* %zr3                   ; <<4 x float>> [#uses=1]
  %432 = extractelement <4 x float> %431, i32 0   ; <float> [#uses=1]
  %433 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %434 = getelementptr inbounds float addrspace(3)* %433, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %432, float addrspace(3)* %434
  %435 = load <4 x float>* %zr3                   ; <<4 x float>> [#uses=1]
  %436 = extractelement <4 x float> %435, i32 1   ; <float> [#uses=1]
  %437 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %438 = getelementptr inbounds float addrspace(3)* %437, i32 66 ; <float addrspace(3)*> [#uses=1]
  store float %436, float addrspace(3)* %438
  %439 = load <4 x float>* %zr3                   ; <<4 x float>> [#uses=1]
  %440 = extractelement <4 x float> %439, i32 2   ; <float> [#uses=1]
  %441 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %442 = getelementptr inbounds float addrspace(3)* %441, i32 132 ; <float addrspace(3)*> [#uses=1]
  store float %440, float addrspace(3)* %442
  %443 = load <4 x float>* %zr3                   ; <<4 x float>> [#uses=1]
  %444 = extractelement <4 x float> %443, i32 3   ; <float> [#uses=1]
  %445 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %446 = getelementptr inbounds float addrspace(3)* %445, i32 198 ; <float addrspace(3)*> [#uses=1]
  store float %444, float addrspace(3)* %446
  %447 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %448 = getelementptr inbounds float addrspace(3)* %447, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %448, float addrspace(3)** %lp
  %449 = load <4 x float>* %zi0                   ; <<4 x float>> [#uses=1]
  %450 = extractelement <4 x float> %449, i32 0   ; <float> [#uses=1]
  %451 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %452 = getelementptr inbounds float addrspace(3)* %451, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %450, float addrspace(3)* %452
  %453 = load <4 x float>* %zi0                   ; <<4 x float>> [#uses=1]
  %454 = extractelement <4 x float> %453, i32 1   ; <float> [#uses=1]
  %455 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %456 = getelementptr inbounds float addrspace(3)* %455, i32 66 ; <float addrspace(3)*> [#uses=1]
  store float %454, float addrspace(3)* %456
  %457 = load <4 x float>* %zi0                   ; <<4 x float>> [#uses=1]
  %458 = extractelement <4 x float> %457, i32 2   ; <float> [#uses=1]
  %459 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %460 = getelementptr inbounds float addrspace(3)* %459, i32 132 ; <float addrspace(3)*> [#uses=1]
  store float %458, float addrspace(3)* %460
  %461 = load <4 x float>* %zi0                   ; <<4 x float>> [#uses=1]
  %462 = extractelement <4 x float> %461, i32 3   ; <float> [#uses=1]
  %463 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %464 = getelementptr inbounds float addrspace(3)* %463, i32 198 ; <float addrspace(3)*> [#uses=1]
  store float %462, float addrspace(3)* %464
  %465 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %466 = getelementptr inbounds float addrspace(3)* %465, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %466, float addrspace(3)** %lp
  %467 = load <4 x float>* %zi1                   ; <<4 x float>> [#uses=1]
  %468 = extractelement <4 x float> %467, i32 0   ; <float> [#uses=1]
  %469 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %470 = getelementptr inbounds float addrspace(3)* %469, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %468, float addrspace(3)* %470
  %471 = load <4 x float>* %zi1                   ; <<4 x float>> [#uses=1]
  %472 = extractelement <4 x float> %471, i32 1   ; <float> [#uses=1]
  %473 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %474 = getelementptr inbounds float addrspace(3)* %473, i32 66 ; <float addrspace(3)*> [#uses=1]
  store float %472, float addrspace(3)* %474
  %475 = load <4 x float>* %zi1                   ; <<4 x float>> [#uses=1]
  %476 = extractelement <4 x float> %475, i32 2   ; <float> [#uses=1]
  %477 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %478 = getelementptr inbounds float addrspace(3)* %477, i32 132 ; <float addrspace(3)*> [#uses=1]
  store float %476, float addrspace(3)* %478
  %479 = load <4 x float>* %zi1                   ; <<4 x float>> [#uses=1]
  %480 = extractelement <4 x float> %479, i32 3   ; <float> [#uses=1]
  %481 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %482 = getelementptr inbounds float addrspace(3)* %481, i32 198 ; <float addrspace(3)*> [#uses=1]
  store float %480, float addrspace(3)* %482
  %483 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %484 = getelementptr inbounds float addrspace(3)* %483, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %484, float addrspace(3)** %lp
  %485 = load <4 x float>* %zi2                   ; <<4 x float>> [#uses=1]
  %486 = extractelement <4 x float> %485, i32 0   ; <float> [#uses=1]
  %487 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %488 = getelementptr inbounds float addrspace(3)* %487, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %486, float addrspace(3)* %488
  %489 = load <4 x float>* %zi2                   ; <<4 x float>> [#uses=1]
  %490 = extractelement <4 x float> %489, i32 1   ; <float> [#uses=1]
  %491 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %492 = getelementptr inbounds float addrspace(3)* %491, i32 66 ; <float addrspace(3)*> [#uses=1]
  store float %490, float addrspace(3)* %492
  %493 = load <4 x float>* %zi2                   ; <<4 x float>> [#uses=1]
  %494 = extractelement <4 x float> %493, i32 2   ; <float> [#uses=1]
  %495 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %496 = getelementptr inbounds float addrspace(3)* %495, i32 132 ; <float addrspace(3)*> [#uses=1]
  store float %494, float addrspace(3)* %496
  %497 = load <4 x float>* %zi2                   ; <<4 x float>> [#uses=1]
  %498 = extractelement <4 x float> %497, i32 3   ; <float> [#uses=1]
  %499 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %500 = getelementptr inbounds float addrspace(3)* %499, i32 198 ; <float addrspace(3)*> [#uses=1]
  store float %498, float addrspace(3)* %500
  %501 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %502 = getelementptr inbounds float addrspace(3)* %501, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %502, float addrspace(3)** %lp
  %503 = load <4 x float>* %zi3                   ; <<4 x float>> [#uses=1]
  %504 = extractelement <4 x float> %503, i32 0   ; <float> [#uses=1]
  %505 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %506 = getelementptr inbounds float addrspace(3)* %505, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %504, float addrspace(3)* %506
  %507 = load <4 x float>* %zi3                   ; <<4 x float>> [#uses=1]
  %508 = extractelement <4 x float> %507, i32 1   ; <float> [#uses=1]
  %509 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %510 = getelementptr inbounds float addrspace(3)* %509, i32 66 ; <float addrspace(3)*> [#uses=1]
  store float %508, float addrspace(3)* %510
  %511 = load <4 x float>* %zi3                   ; <<4 x float>> [#uses=1]
  %512 = extractelement <4 x float> %511, i32 2   ; <float> [#uses=1]
  %513 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %514 = getelementptr inbounds float addrspace(3)* %513, i32 132 ; <float addrspace(3)*> [#uses=1]
  store float %512, float addrspace(3)* %514
  %515 = load <4 x float>* %zi3                   ; <<4 x float>> [#uses=1]
  %516 = extractelement <4 x float> %515, i32 3   ; <float> [#uses=1]
  %517 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %518 = getelementptr inbounds float addrspace(3)* %517, i32 198 ; <float addrspace(3)*> [#uses=1]
  store float %516, float addrspace(3)* %518
  call void @barrier(i32 1)
  ret void
}

define void @kfft_pass4(i32 %me, float addrspace(3)* %lds) nounwind alwaysinline {
  %1 = alloca float, align 4                      ; <float*> [#uses=2]
  %2 = alloca i32, align 4                        ; <i32*> [#uses=5]
  %3 = alloca float*, align 4                     ; <float**> [#uses=2]
  %x.i5 = alloca float, align 4                   ; <float*> [#uses=3]
  %4 = alloca float, align 4                      ; <float*> [#uses=2]
  %5 = alloca i32, align 4                        ; <i32*> [#uses=5]
  %6 = alloca float*, align 4                     ; <float**> [#uses=2]
  %x.i3 = alloca float, align 4                   ; <float*> [#uses=3]
  %7 = alloca float, align 4                      ; <float*> [#uses=2]
  %8 = alloca i32, align 4                        ; <i32*> [#uses=5]
  %9 = alloca float*, align 4                     ; <float**> [#uses=2]
  %x.i = alloca float, align 4                    ; <float*> [#uses=3]
  %10 = alloca i32, align 4                       ; <i32*> [#uses=6]
  %11 = alloca float addrspace(3)*, align 16      ; <float addrspace(3)**> [#uses=3]
  %lp = alloca float addrspace(3)*, align 16      ; <float addrspace(3)**> [#uses=94]
  %zr0 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=15]
  %zr1 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=18]
  %zr2 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=18]
  %zr3 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=18]
  %zi0 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=15]
  %zi1 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=18]
  %zi2 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=18]
  %zi3 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=18]
  %ar0 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %ar2 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %br0 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %br1 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %br2 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %br3 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %ai0 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %ai2 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %bi0 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %bi1 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %bi2 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %bi3 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %tbase = alloca i32, align 4                    ; <i32*> [#uses=4]
  %c1 = alloca float, align 4                     ; <float*> [#uses=3]
  %s1 = alloca float, align 4                     ; <float*> [#uses=3]
  %__r = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %c2 = alloca float, align 4                     ; <float*> [#uses=3]
  %s2 = alloca float, align 4                     ; <float*> [#uses=3]
  %__r1 = alloca <4 x float>, align 16            ; <<4 x float>*> [#uses=2]
  %c3 = alloca float, align 4                     ; <float*> [#uses=3]
  %s3 = alloca float, align 4                     ; <float*> [#uses=3]
  %__r2 = alloca <4 x float>, align 16            ; <<4 x float>*> [#uses=2]
  store i32 %me, i32* %10
  store float addrspace(3)* %lds, float addrspace(3)** %11
  %12 = load float addrspace(3)** %11             ; <float addrspace(3)*> [#uses=1]
  %13 = load i32* %10                             ; <i32> [#uses=1]
  %14 = and i32 %13, 3                            ; <i32> [#uses=1]
  %15 = load i32* %10                             ; <i32> [#uses=1]
  %16 = lshr i32 %15, 2                           ; <i32> [#uses=1]
  %17 = and i32 %16, 3                            ; <i32> [#uses=1]
  %18 = mul i32 %17, 264                          ; <i32> [#uses=1]
  %19 = add i32 %14, %18                          ; <i32> [#uses=1]
  %20 = load i32* %10                             ; <i32> [#uses=1]
  %21 = lshr i32 %20, 4                           ; <i32> [#uses=1]
  %22 = shl i32 %21, 2                            ; <i32> [#uses=1]
  %23 = add i32 %19, %22                          ; <i32> [#uses=1]
  %24 = getelementptr inbounds float addrspace(3)* %12, i32 %23 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %24, float addrspace(3)** %lp
  %25 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %26 = getelementptr inbounds float addrspace(3)* %25, i32 0 ; <float addrspace(3)*> [#uses=1]
  %27 = load float addrspace(3)* %26              ; <float> [#uses=1]
  %28 = load <4 x float>* %zr0                    ; <<4 x float>> [#uses=1]
  %29 = insertelement <4 x float> %28, float %27, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %29, <4 x float>* %zr0
  %30 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %31 = getelementptr inbounds float addrspace(3)* %30, i32 66 ; <float addrspace(3)*> [#uses=1]
  %32 = load float addrspace(3)* %31              ; <float> [#uses=1]
  %33 = load <4 x float>* %zr0                    ; <<4 x float>> [#uses=1]
  %34 = insertelement <4 x float> %33, float %32, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %34, <4 x float>* %zr0
  %35 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %36 = getelementptr inbounds float addrspace(3)* %35, i32 132 ; <float addrspace(3)*> [#uses=1]
  %37 = load float addrspace(3)* %36              ; <float> [#uses=1]
  %38 = load <4 x float>* %zr0                    ; <<4 x float>> [#uses=1]
  %39 = insertelement <4 x float> %38, float %37, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %39, <4 x float>* %zr0
  %40 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %41 = getelementptr inbounds float addrspace(3)* %40, i32 198 ; <float addrspace(3)*> [#uses=1]
  %42 = load float addrspace(3)* %41              ; <float> [#uses=1]
  %43 = load <4 x float>* %zr0                    ; <<4 x float>> [#uses=1]
  %44 = insertelement <4 x float> %43, float %42, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %44, <4 x float>* %zr0
  %45 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %46 = getelementptr inbounds float addrspace(3)* %45, i32 16 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %46, float addrspace(3)** %lp
  %47 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %48 = getelementptr inbounds float addrspace(3)* %47, i32 0 ; <float addrspace(3)*> [#uses=1]
  %49 = load float addrspace(3)* %48              ; <float> [#uses=1]
  %50 = load <4 x float>* %zr1                    ; <<4 x float>> [#uses=1]
  %51 = insertelement <4 x float> %50, float %49, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %51, <4 x float>* %zr1
  %52 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %53 = getelementptr inbounds float addrspace(3)* %52, i32 66 ; <float addrspace(3)*> [#uses=1]
  %54 = load float addrspace(3)* %53              ; <float> [#uses=1]
  %55 = load <4 x float>* %zr1                    ; <<4 x float>> [#uses=1]
  %56 = insertelement <4 x float> %55, float %54, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %56, <4 x float>* %zr1
  %57 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %58 = getelementptr inbounds float addrspace(3)* %57, i32 132 ; <float addrspace(3)*> [#uses=1]
  %59 = load float addrspace(3)* %58              ; <float> [#uses=1]
  %60 = load <4 x float>* %zr1                    ; <<4 x float>> [#uses=1]
  %61 = insertelement <4 x float> %60, float %59, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %61, <4 x float>* %zr1
  %62 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %63 = getelementptr inbounds float addrspace(3)* %62, i32 198 ; <float addrspace(3)*> [#uses=1]
  %64 = load float addrspace(3)* %63              ; <float> [#uses=1]
  %65 = load <4 x float>* %zr1                    ; <<4 x float>> [#uses=1]
  %66 = insertelement <4 x float> %65, float %64, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %66, <4 x float>* %zr1
  %67 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %68 = getelementptr inbounds float addrspace(3)* %67, i32 16 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %68, float addrspace(3)** %lp
  %69 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %70 = getelementptr inbounds float addrspace(3)* %69, i32 0 ; <float addrspace(3)*> [#uses=1]
  %71 = load float addrspace(3)* %70              ; <float> [#uses=1]
  %72 = load <4 x float>* %zr2                    ; <<4 x float>> [#uses=1]
  %73 = insertelement <4 x float> %72, float %71, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %73, <4 x float>* %zr2
  %74 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %75 = getelementptr inbounds float addrspace(3)* %74, i32 66 ; <float addrspace(3)*> [#uses=1]
  %76 = load float addrspace(3)* %75              ; <float> [#uses=1]
  %77 = load <4 x float>* %zr2                    ; <<4 x float>> [#uses=1]
  %78 = insertelement <4 x float> %77, float %76, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %78, <4 x float>* %zr2
  %79 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %80 = getelementptr inbounds float addrspace(3)* %79, i32 132 ; <float addrspace(3)*> [#uses=1]
  %81 = load float addrspace(3)* %80              ; <float> [#uses=1]
  %82 = load <4 x float>* %zr2                    ; <<4 x float>> [#uses=1]
  %83 = insertelement <4 x float> %82, float %81, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %83, <4 x float>* %zr2
  %84 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %85 = getelementptr inbounds float addrspace(3)* %84, i32 198 ; <float addrspace(3)*> [#uses=1]
  %86 = load float addrspace(3)* %85              ; <float> [#uses=1]
  %87 = load <4 x float>* %zr2                    ; <<4 x float>> [#uses=1]
  %88 = insertelement <4 x float> %87, float %86, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %88, <4 x float>* %zr2
  %89 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %90 = getelementptr inbounds float addrspace(3)* %89, i32 16 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %90, float addrspace(3)** %lp
  %91 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %92 = getelementptr inbounds float addrspace(3)* %91, i32 0 ; <float addrspace(3)*> [#uses=1]
  %93 = load float addrspace(3)* %92              ; <float> [#uses=1]
  %94 = load <4 x float>* %zr3                    ; <<4 x float>> [#uses=1]
  %95 = insertelement <4 x float> %94, float %93, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %95, <4 x float>* %zr3
  %96 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %97 = getelementptr inbounds float addrspace(3)* %96, i32 66 ; <float addrspace(3)*> [#uses=1]
  %98 = load float addrspace(3)* %97              ; <float> [#uses=1]
  %99 = load <4 x float>* %zr3                    ; <<4 x float>> [#uses=1]
  %100 = insertelement <4 x float> %99, float %98, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %100, <4 x float>* %zr3
  %101 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %102 = getelementptr inbounds float addrspace(3)* %101, i32 132 ; <float addrspace(3)*> [#uses=1]
  %103 = load float addrspace(3)* %102            ; <float> [#uses=1]
  %104 = load <4 x float>* %zr3                   ; <<4 x float>> [#uses=1]
  %105 = insertelement <4 x float> %104, float %103, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %105, <4 x float>* %zr3
  %106 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %107 = getelementptr inbounds float addrspace(3)* %106, i32 198 ; <float addrspace(3)*> [#uses=1]
  %108 = load float addrspace(3)* %107            ; <float> [#uses=1]
  %109 = load <4 x float>* %zr3                   ; <<4 x float>> [#uses=1]
  %110 = insertelement <4 x float> %109, float %108, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %110, <4 x float>* %zr3
  %111 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %112 = getelementptr inbounds float addrspace(3)* %111, i32 1008 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %112, float addrspace(3)** %lp
  %113 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %114 = getelementptr inbounds float addrspace(3)* %113, i32 0 ; <float addrspace(3)*> [#uses=1]
  %115 = load float addrspace(3)* %114            ; <float> [#uses=1]
  %116 = load <4 x float>* %zi0                   ; <<4 x float>> [#uses=1]
  %117 = insertelement <4 x float> %116, float %115, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %117, <4 x float>* %zi0
  %118 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %119 = getelementptr inbounds float addrspace(3)* %118, i32 66 ; <float addrspace(3)*> [#uses=1]
  %120 = load float addrspace(3)* %119            ; <float> [#uses=1]
  %121 = load <4 x float>* %zi0                   ; <<4 x float>> [#uses=1]
  %122 = insertelement <4 x float> %121, float %120, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %122, <4 x float>* %zi0
  %123 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %124 = getelementptr inbounds float addrspace(3)* %123, i32 132 ; <float addrspace(3)*> [#uses=1]
  %125 = load float addrspace(3)* %124            ; <float> [#uses=1]
  %126 = load <4 x float>* %zi0                   ; <<4 x float>> [#uses=1]
  %127 = insertelement <4 x float> %126, float %125, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %127, <4 x float>* %zi0
  %128 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %129 = getelementptr inbounds float addrspace(3)* %128, i32 198 ; <float addrspace(3)*> [#uses=1]
  %130 = load float addrspace(3)* %129            ; <float> [#uses=1]
  %131 = load <4 x float>* %zi0                   ; <<4 x float>> [#uses=1]
  %132 = insertelement <4 x float> %131, float %130, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %132, <4 x float>* %zi0
  %133 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %134 = getelementptr inbounds float addrspace(3)* %133, i32 16 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %134, float addrspace(3)** %lp
  %135 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %136 = getelementptr inbounds float addrspace(3)* %135, i32 0 ; <float addrspace(3)*> [#uses=1]
  %137 = load float addrspace(3)* %136            ; <float> [#uses=1]
  %138 = load <4 x float>* %zi1                   ; <<4 x float>> [#uses=1]
  %139 = insertelement <4 x float> %138, float %137, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %139, <4 x float>* %zi1
  %140 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %141 = getelementptr inbounds float addrspace(3)* %140, i32 66 ; <float addrspace(3)*> [#uses=1]
  %142 = load float addrspace(3)* %141            ; <float> [#uses=1]
  %143 = load <4 x float>* %zi1                   ; <<4 x float>> [#uses=1]
  %144 = insertelement <4 x float> %143, float %142, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %144, <4 x float>* %zi1
  %145 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %146 = getelementptr inbounds float addrspace(3)* %145, i32 132 ; <float addrspace(3)*> [#uses=1]
  %147 = load float addrspace(3)* %146            ; <float> [#uses=1]
  %148 = load <4 x float>* %zi1                   ; <<4 x float>> [#uses=1]
  %149 = insertelement <4 x float> %148, float %147, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %149, <4 x float>* %zi1
  %150 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %151 = getelementptr inbounds float addrspace(3)* %150, i32 198 ; <float addrspace(3)*> [#uses=1]
  %152 = load float addrspace(3)* %151            ; <float> [#uses=1]
  %153 = load <4 x float>* %zi1                   ; <<4 x float>> [#uses=1]
  %154 = insertelement <4 x float> %153, float %152, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %154, <4 x float>* %zi1
  %155 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %156 = getelementptr inbounds float addrspace(3)* %155, i32 16 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %156, float addrspace(3)** %lp
  %157 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %158 = getelementptr inbounds float addrspace(3)* %157, i32 0 ; <float addrspace(3)*> [#uses=1]
  %159 = load float addrspace(3)* %158            ; <float> [#uses=1]
  %160 = load <4 x float>* %zi2                   ; <<4 x float>> [#uses=1]
  %161 = insertelement <4 x float> %160, float %159, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %161, <4 x float>* %zi2
  %162 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %163 = getelementptr inbounds float addrspace(3)* %162, i32 66 ; <float addrspace(3)*> [#uses=1]
  %164 = load float addrspace(3)* %163            ; <float> [#uses=1]
  %165 = load <4 x float>* %zi2                   ; <<4 x float>> [#uses=1]
  %166 = insertelement <4 x float> %165, float %164, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %166, <4 x float>* %zi2
  %167 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %168 = getelementptr inbounds float addrspace(3)* %167, i32 132 ; <float addrspace(3)*> [#uses=1]
  %169 = load float addrspace(3)* %168            ; <float> [#uses=1]
  %170 = load <4 x float>* %zi2                   ; <<4 x float>> [#uses=1]
  %171 = insertelement <4 x float> %170, float %169, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %171, <4 x float>* %zi2
  %172 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %173 = getelementptr inbounds float addrspace(3)* %172, i32 198 ; <float addrspace(3)*> [#uses=1]
  %174 = load float addrspace(3)* %173            ; <float> [#uses=1]
  %175 = load <4 x float>* %zi2                   ; <<4 x float>> [#uses=1]
  %176 = insertelement <4 x float> %175, float %174, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %176, <4 x float>* %zi2
  %177 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %178 = getelementptr inbounds float addrspace(3)* %177, i32 16 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %178, float addrspace(3)** %lp
  %179 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %180 = getelementptr inbounds float addrspace(3)* %179, i32 0 ; <float addrspace(3)*> [#uses=1]
  %181 = load float addrspace(3)* %180            ; <float> [#uses=1]
  %182 = load <4 x float>* %zi3                   ; <<4 x float>> [#uses=1]
  %183 = insertelement <4 x float> %182, float %181, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %183, <4 x float>* %zi3
  %184 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %185 = getelementptr inbounds float addrspace(3)* %184, i32 66 ; <float addrspace(3)*> [#uses=1]
  %186 = load float addrspace(3)* %185            ; <float> [#uses=1]
  %187 = load <4 x float>* %zi3                   ; <<4 x float>> [#uses=1]
  %188 = insertelement <4 x float> %187, float %186, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %188, <4 x float>* %zi3
  %189 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %190 = getelementptr inbounds float addrspace(3)* %189, i32 132 ; <float addrspace(3)*> [#uses=1]
  %191 = load float addrspace(3)* %190            ; <float> [#uses=1]
  %192 = load <4 x float>* %zi3                   ; <<4 x float>> [#uses=1]
  %193 = insertelement <4 x float> %192, float %191, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %193, <4 x float>* %zi3
  %194 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %195 = getelementptr inbounds float addrspace(3)* %194, i32 198 ; <float addrspace(3)*> [#uses=1]
  %196 = load float addrspace(3)* %195            ; <float> [#uses=1]
  %197 = load <4 x float>* %zi3                   ; <<4 x float>> [#uses=1]
  %198 = insertelement <4 x float> %197, float %196, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %198, <4 x float>* %zi3
  br label %199

; <label>:199                                     ; preds = %0
  %200 = load <4 x float>* %zr0                   ; <<4 x float>> [#uses=1]
  %201 = load <4 x float>* %zr2                   ; <<4 x float>> [#uses=1]
  %202 = fadd <4 x float> %200, %201              ; <<4 x float>> [#uses=1]
  store <4 x float> %202, <4 x float>* %ar0
  %203 = load <4 x float>* %zr1                   ; <<4 x float>> [#uses=1]
  %204 = load <4 x float>* %zr3                   ; <<4 x float>> [#uses=1]
  %205 = fadd <4 x float> %203, %204              ; <<4 x float>> [#uses=1]
  store <4 x float> %205, <4 x float>* %ar2
  %206 = load <4 x float>* %ar0                   ; <<4 x float>> [#uses=1]
  %207 = load <4 x float>* %ar2                   ; <<4 x float>> [#uses=1]
  %208 = fadd <4 x float> %206, %207              ; <<4 x float>> [#uses=1]
  store <4 x float> %208, <4 x float>* %br0
  %209 = load <4 x float>* %zr0                   ; <<4 x float>> [#uses=1]
  %210 = load <4 x float>* %zr2                   ; <<4 x float>> [#uses=1]
  %211 = fsub <4 x float> %209, %210              ; <<4 x float>> [#uses=1]
  store <4 x float> %211, <4 x float>* %br1
  %212 = load <4 x float>* %ar0                   ; <<4 x float>> [#uses=1]
  %213 = load <4 x float>* %ar2                   ; <<4 x float>> [#uses=1]
  %214 = fsub <4 x float> %212, %213              ; <<4 x float>> [#uses=1]
  store <4 x float> %214, <4 x float>* %br2
  %215 = load <4 x float>* %zr1                   ; <<4 x float>> [#uses=1]
  %216 = load <4 x float>* %zr3                   ; <<4 x float>> [#uses=1]
  %217 = fsub <4 x float> %215, %216              ; <<4 x float>> [#uses=1]
  store <4 x float> %217, <4 x float>* %br3
  %218 = load <4 x float>* %zi0                   ; <<4 x float>> [#uses=1]
  %219 = load <4 x float>* %zi2                   ; <<4 x float>> [#uses=1]
  %220 = fadd <4 x float> %218, %219              ; <<4 x float>> [#uses=1]
  store <4 x float> %220, <4 x float>* %ai0
  %221 = load <4 x float>* %zi1                   ; <<4 x float>> [#uses=1]
  %222 = load <4 x float>* %zi3                   ; <<4 x float>> [#uses=1]
  %223 = fadd <4 x float> %221, %222              ; <<4 x float>> [#uses=1]
  store <4 x float> %223, <4 x float>* %ai2
  %224 = load <4 x float>* %ai0                   ; <<4 x float>> [#uses=1]
  %225 = load <4 x float>* %ai2                   ; <<4 x float>> [#uses=1]
  %226 = fadd <4 x float> %224, %225              ; <<4 x float>> [#uses=1]
  store <4 x float> %226, <4 x float>* %bi0
  %227 = load <4 x float>* %zi0                   ; <<4 x float>> [#uses=1]
  %228 = load <4 x float>* %zi2                   ; <<4 x float>> [#uses=1]
  %229 = fsub <4 x float> %227, %228              ; <<4 x float>> [#uses=1]
  store <4 x float> %229, <4 x float>* %bi1
  %230 = load <4 x float>* %ai0                   ; <<4 x float>> [#uses=1]
  %231 = load <4 x float>* %ai2                   ; <<4 x float>> [#uses=1]
  %232 = fsub <4 x float> %230, %231              ; <<4 x float>> [#uses=1]
  store <4 x float> %232, <4 x float>* %bi2
  %233 = load <4 x float>* %zi1                   ; <<4 x float>> [#uses=1]
  %234 = load <4 x float>* %zi3                   ; <<4 x float>> [#uses=1]
  %235 = fsub <4 x float> %233, %234              ; <<4 x float>> [#uses=1]
  store <4 x float> %235, <4 x float>* %bi3
  %236 = load <4 x float>* %br0                   ; <<4 x float>> [#uses=1]
  store <4 x float> %236, <4 x float>* %zr0
  %237 = load <4 x float>* %bi0                   ; <<4 x float>> [#uses=1]
  store <4 x float> %237, <4 x float>* %zi0
  %238 = load <4 x float>* %br1                   ; <<4 x float>> [#uses=1]
  %239 = load <4 x float>* %bi3                   ; <<4 x float>> [#uses=1]
  %240 = fadd <4 x float> %238, %239              ; <<4 x float>> [#uses=1]
  store <4 x float> %240, <4 x float>* %zr1
  %241 = load <4 x float>* %bi1                   ; <<4 x float>> [#uses=1]
  %242 = load <4 x float>* %br3                   ; <<4 x float>> [#uses=1]
  %243 = fsub <4 x float> %241, %242              ; <<4 x float>> [#uses=1]
  store <4 x float> %243, <4 x float>* %zi1
  %244 = load <4 x float>* %br1                   ; <<4 x float>> [#uses=1]
  %245 = load <4 x float>* %bi3                   ; <<4 x float>> [#uses=1]
  %246 = fsub <4 x float> %244, %245              ; <<4 x float>> [#uses=1]
  store <4 x float> %246, <4 x float>* %zr3
  %247 = load <4 x float>* %br3                   ; <<4 x float>> [#uses=1]
  %248 = load <4 x float>* %bi1                   ; <<4 x float>> [#uses=1]
  %249 = fadd <4 x float> %247, %248              ; <<4 x float>> [#uses=1]
  store <4 x float> %249, <4 x float>* %zi3
  %250 = load <4 x float>* %br2                   ; <<4 x float>> [#uses=1]
  store <4 x float> %250, <4 x float>* %zr2
  %251 = load <4 x float>* %bi2                   ; <<4 x float>> [#uses=1]
  store <4 x float> %251, <4 x float>* %zi2
  br label %252

; <label>:252                                     ; preds = %199
  %253 = load i32* %10                            ; <i32> [#uses=1]
  %254 = lshr i32 %253, 4                         ; <i32> [#uses=1]
  %255 = shl i32 %254, 6                          ; <i32> [#uses=1]
  store i32 %255, i32* %tbase
  br label %256

; <label>:256                                     ; preds = %252
  %257 = load i32* %tbase                         ; <i32> [#uses=1]
  %258 = mul i32 %257, 1                          ; <i32> [#uses=1]
  store i32 %258, i32* %8
  store float* %c1, float** %9
  %259 = load i32* %8                             ; <i32> [#uses=1]
  %260 = icmp sgt i32 %259, 512                   ; <i1> [#uses=1]
  br i1 %260, label %261, label %k_sincos.exit

; <label>:261                                     ; preds = %256
  %262 = load i32* %8                             ; <i32> [#uses=1]
  %263 = sub i32 %262, 1024                       ; <i32> [#uses=1]
  store i32 %263, i32* %8
  br label %k_sincos.exit

k_sincos.exit:                                    ; preds = %256, %261
  %264 = load i32* %8                             ; <i32> [#uses=1]
  %265 = sitofp i32 %264 to float                 ; <float> [#uses=1]
  %266 = fmul float %265, 0xBF7921FB60000000      ; <float> [#uses=1]
  store float %266, float* %x.i
  %267 = load float* %x.i                         ; <float> [#uses=1]
  %268 = call float @_Z10native_cosf(float %267) nounwind ; <float> [#uses=1]
  %269 = load float** %9                          ; <float*> [#uses=1]
  store float %268, float* %269
  %270 = load float* %x.i                         ; <float> [#uses=1]
  %271 = call float @_Z10native_sinf(float %270) nounwind ; <float> [#uses=1]
  store float %271, float* %7
  %272 = load float* %7                           ; <float> [#uses=1]
  store float %272, float* %s1
  br label %273

; <label>:273                                     ; preds = %k_sincos.exit
  %274 = load float* %c1                          ; <float> [#uses=1]
  %275 = insertelement <4 x float> undef, float %274, i32 0 ; <<4 x float>> [#uses=2]
  %276 = shufflevector <4 x float> %275, <4 x float> %275, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %277 = load <4 x float>* %zr1                   ; <<4 x float>> [#uses=1]
  %278 = fmul <4 x float> %276, %277              ; <<4 x float>> [#uses=1]
  %279 = load float* %s1                          ; <float> [#uses=1]
  %280 = insertelement <4 x float> undef, float %279, i32 0 ; <<4 x float>> [#uses=2]
  %281 = shufflevector <4 x float> %280, <4 x float> %280, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %282 = load <4 x float>* %zi1                   ; <<4 x float>> [#uses=1]
  %283 = fmul <4 x float> %281, %282              ; <<4 x float>> [#uses=1]
  %284 = fsub <4 x float> %278, %283              ; <<4 x float>> [#uses=1]
  store <4 x float> %284, <4 x float>* %__r
  %285 = load float* %c1                          ; <float> [#uses=1]
  %286 = insertelement <4 x float> undef, float %285, i32 0 ; <<4 x float>> [#uses=2]
  %287 = shufflevector <4 x float> %286, <4 x float> %286, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %288 = load <4 x float>* %zi1                   ; <<4 x float>> [#uses=1]
  %289 = fmul <4 x float> %287, %288              ; <<4 x float>> [#uses=1]
  %290 = load float* %s1                          ; <float> [#uses=1]
  %291 = insertelement <4 x float> undef, float %290, i32 0 ; <<4 x float>> [#uses=2]
  %292 = shufflevector <4 x float> %291, <4 x float> %291, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %293 = load <4 x float>* %zr1                   ; <<4 x float>> [#uses=1]
  %294 = fmul <4 x float> %292, %293              ; <<4 x float>> [#uses=1]
  %295 = fadd <4 x float> %289, %294              ; <<4 x float>> [#uses=1]
  store <4 x float> %295, <4 x float>* %zi1
  %296 = load <4 x float>* %__r                   ; <<4 x float>> [#uses=1]
  store <4 x float> %296, <4 x float>* %zr1
  br label %297

; <label>:297                                     ; preds = %273
  %298 = load i32* %tbase                         ; <i32> [#uses=1]
  %299 = mul i32 %298, 2                          ; <i32> [#uses=1]
  store i32 %299, i32* %2
  store float* %c2, float** %3
  %300 = load i32* %2                             ; <i32> [#uses=1]
  %301 = icmp sgt i32 %300, 512                   ; <i1> [#uses=1]
  br i1 %301, label %302, label %k_sincos.exit6

; <label>:302                                     ; preds = %297
  %303 = load i32* %2                             ; <i32> [#uses=1]
  %304 = sub i32 %303, 1024                       ; <i32> [#uses=1]
  store i32 %304, i32* %2
  br label %k_sincos.exit6

k_sincos.exit6:                                   ; preds = %297, %302
  %305 = load i32* %2                             ; <i32> [#uses=1]
  %306 = sitofp i32 %305 to float                 ; <float> [#uses=1]
  %307 = fmul float %306, 0xBF7921FB60000000      ; <float> [#uses=1]
  store float %307, float* %x.i5
  %308 = load float* %x.i5                        ; <float> [#uses=1]
  %309 = call float @_Z10native_cosf(float %308) nounwind ; <float> [#uses=1]
  %310 = load float** %3                          ; <float*> [#uses=1]
  store float %309, float* %310
  %311 = load float* %x.i5                        ; <float> [#uses=1]
  %312 = call float @_Z10native_sinf(float %311) nounwind ; <float> [#uses=1]
  store float %312, float* %1
  %313 = load float* %1                           ; <float> [#uses=1]
  store float %313, float* %s2
  br label %314

; <label>:314                                     ; preds = %k_sincos.exit6
  %315 = load float* %c2                          ; <float> [#uses=1]
  %316 = insertelement <4 x float> undef, float %315, i32 0 ; <<4 x float>> [#uses=2]
  %317 = shufflevector <4 x float> %316, <4 x float> %316, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %318 = load <4 x float>* %zr2                   ; <<4 x float>> [#uses=1]
  %319 = fmul <4 x float> %317, %318              ; <<4 x float>> [#uses=1]
  %320 = load float* %s2                          ; <float> [#uses=1]
  %321 = insertelement <4 x float> undef, float %320, i32 0 ; <<4 x float>> [#uses=2]
  %322 = shufflevector <4 x float> %321, <4 x float> %321, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %323 = load <4 x float>* %zi2                   ; <<4 x float>> [#uses=1]
  %324 = fmul <4 x float> %322, %323              ; <<4 x float>> [#uses=1]
  %325 = fsub <4 x float> %319, %324              ; <<4 x float>> [#uses=1]
  store <4 x float> %325, <4 x float>* %__r1
  %326 = load float* %c2                          ; <float> [#uses=1]
  %327 = insertelement <4 x float> undef, float %326, i32 0 ; <<4 x float>> [#uses=2]
  %328 = shufflevector <4 x float> %327, <4 x float> %327, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %329 = load <4 x float>* %zi2                   ; <<4 x float>> [#uses=1]
  %330 = fmul <4 x float> %328, %329              ; <<4 x float>> [#uses=1]
  %331 = load float* %s2                          ; <float> [#uses=1]
  %332 = insertelement <4 x float> undef, float %331, i32 0 ; <<4 x float>> [#uses=2]
  %333 = shufflevector <4 x float> %332, <4 x float> %332, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %334 = load <4 x float>* %zr2                   ; <<4 x float>> [#uses=1]
  %335 = fmul <4 x float> %333, %334              ; <<4 x float>> [#uses=1]
  %336 = fadd <4 x float> %330, %335              ; <<4 x float>> [#uses=1]
  store <4 x float> %336, <4 x float>* %zi2
  %337 = load <4 x float>* %__r1                  ; <<4 x float>> [#uses=1]
  store <4 x float> %337, <4 x float>* %zr2
  br label %338

; <label>:338                                     ; preds = %314
  %339 = load i32* %tbase                         ; <i32> [#uses=1]
  %340 = mul i32 %339, 3                          ; <i32> [#uses=1]
  store i32 %340, i32* %5
  store float* %c3, float** %6
  %341 = load i32* %5                             ; <i32> [#uses=1]
  %342 = icmp sgt i32 %341, 512                   ; <i1> [#uses=1]
  br i1 %342, label %343, label %k_sincos.exit4

; <label>:343                                     ; preds = %338
  %344 = load i32* %5                             ; <i32> [#uses=1]
  %345 = sub i32 %344, 1024                       ; <i32> [#uses=1]
  store i32 %345, i32* %5
  br label %k_sincos.exit4

k_sincos.exit4:                                   ; preds = %338, %343
  %346 = load i32* %5                             ; <i32> [#uses=1]
  %347 = sitofp i32 %346 to float                 ; <float> [#uses=1]
  %348 = fmul float %347, 0xBF7921FB60000000      ; <float> [#uses=1]
  store float %348, float* %x.i3
  %349 = load float* %x.i3                        ; <float> [#uses=1]
  %350 = call float @_Z10native_cosf(float %349) nounwind ; <float> [#uses=1]
  %351 = load float** %6                          ; <float*> [#uses=1]
  store float %350, float* %351
  %352 = load float* %x.i3                        ; <float> [#uses=1]
  %353 = call float @_Z10native_sinf(float %352) nounwind ; <float> [#uses=1]
  store float %353, float* %4
  %354 = load float* %4                           ; <float> [#uses=1]
  store float %354, float* %s3
  br label %355

; <label>:355                                     ; preds = %k_sincos.exit4
  %356 = load float* %c3                          ; <float> [#uses=1]
  %357 = insertelement <4 x float> undef, float %356, i32 0 ; <<4 x float>> [#uses=2]
  %358 = shufflevector <4 x float> %357, <4 x float> %357, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %359 = load <4 x float>* %zr3                   ; <<4 x float>> [#uses=1]
  %360 = fmul <4 x float> %358, %359              ; <<4 x float>> [#uses=1]
  %361 = load float* %s3                          ; <float> [#uses=1]
  %362 = insertelement <4 x float> undef, float %361, i32 0 ; <<4 x float>> [#uses=2]
  %363 = shufflevector <4 x float> %362, <4 x float> %362, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %364 = load <4 x float>* %zi3                   ; <<4 x float>> [#uses=1]
  %365 = fmul <4 x float> %363, %364              ; <<4 x float>> [#uses=1]
  %366 = fsub <4 x float> %360, %365              ; <<4 x float>> [#uses=1]
  store <4 x float> %366, <4 x float>* %__r2
  %367 = load float* %c3                          ; <float> [#uses=1]
  %368 = insertelement <4 x float> undef, float %367, i32 0 ; <<4 x float>> [#uses=2]
  %369 = shufflevector <4 x float> %368, <4 x float> %368, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %370 = load <4 x float>* %zi3                   ; <<4 x float>> [#uses=1]
  %371 = fmul <4 x float> %369, %370              ; <<4 x float>> [#uses=1]
  %372 = load float* %s3                          ; <float> [#uses=1]
  %373 = insertelement <4 x float> undef, float %372, i32 0 ; <<4 x float>> [#uses=2]
  %374 = shufflevector <4 x float> %373, <4 x float> %373, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %375 = load <4 x float>* %zr3                   ; <<4 x float>> [#uses=1]
  %376 = fmul <4 x float> %374, %375              ; <<4 x float>> [#uses=1]
  %377 = fadd <4 x float> %371, %376              ; <<4 x float>> [#uses=1]
  store <4 x float> %377, <4 x float>* %zi3
  %378 = load <4 x float>* %__r2                  ; <<4 x float>> [#uses=1]
  store <4 x float> %378, <4 x float>* %zr3
  br label %379

; <label>:379                                     ; preds = %355
  br label %380

; <label>:380                                     ; preds = %379
  call void @barrier(i32 1)
  %381 = load float addrspace(3)** %11            ; <float addrspace(3)*> [#uses=1]
  %382 = load i32* %10                            ; <i32> [#uses=1]
  %383 = getelementptr inbounds float addrspace(3)* %381, i32 %382 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %383, float addrspace(3)** %lp
  %384 = load <4 x float>* %zr0                   ; <<4 x float>> [#uses=1]
  %385 = extractelement <4 x float> %384, i32 0   ; <float> [#uses=1]
  %386 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %387 = getelementptr inbounds float addrspace(3)* %386, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %385, float addrspace(3)* %387
  %388 = load <4 x float>* %zr0                   ; <<4 x float>> [#uses=1]
  %389 = extractelement <4 x float> %388, i32 1   ; <float> [#uses=1]
  %390 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %391 = getelementptr inbounds float addrspace(3)* %390, i32 68 ; <float addrspace(3)*> [#uses=1]
  store float %389, float addrspace(3)* %391
  %392 = load <4 x float>* %zr0                   ; <<4 x float>> [#uses=1]
  %393 = extractelement <4 x float> %392, i32 2   ; <float> [#uses=1]
  %394 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %395 = getelementptr inbounds float addrspace(3)* %394, i32 136 ; <float addrspace(3)*> [#uses=1]
  store float %393, float addrspace(3)* %395
  %396 = load <4 x float>* %zr0                   ; <<4 x float>> [#uses=1]
  %397 = extractelement <4 x float> %396, i32 3   ; <float> [#uses=1]
  %398 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %399 = getelementptr inbounds float addrspace(3)* %398, i32 204 ; <float addrspace(3)*> [#uses=1]
  store float %397, float addrspace(3)* %399
  %400 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %401 = getelementptr inbounds float addrspace(3)* %400, i32 272 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %401, float addrspace(3)** %lp
  %402 = load <4 x float>* %zr1                   ; <<4 x float>> [#uses=1]
  %403 = extractelement <4 x float> %402, i32 0   ; <float> [#uses=1]
  %404 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %405 = getelementptr inbounds float addrspace(3)* %404, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %403, float addrspace(3)* %405
  %406 = load <4 x float>* %zr1                   ; <<4 x float>> [#uses=1]
  %407 = extractelement <4 x float> %406, i32 1   ; <float> [#uses=1]
  %408 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %409 = getelementptr inbounds float addrspace(3)* %408, i32 68 ; <float addrspace(3)*> [#uses=1]
  store float %407, float addrspace(3)* %409
  %410 = load <4 x float>* %zr1                   ; <<4 x float>> [#uses=1]
  %411 = extractelement <4 x float> %410, i32 2   ; <float> [#uses=1]
  %412 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %413 = getelementptr inbounds float addrspace(3)* %412, i32 136 ; <float addrspace(3)*> [#uses=1]
  store float %411, float addrspace(3)* %413
  %414 = load <4 x float>* %zr1                   ; <<4 x float>> [#uses=1]
  %415 = extractelement <4 x float> %414, i32 3   ; <float> [#uses=1]
  %416 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %417 = getelementptr inbounds float addrspace(3)* %416, i32 204 ; <float addrspace(3)*> [#uses=1]
  store float %415, float addrspace(3)* %417
  %418 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %419 = getelementptr inbounds float addrspace(3)* %418, i32 272 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %419, float addrspace(3)** %lp
  %420 = load <4 x float>* %zr2                   ; <<4 x float>> [#uses=1]
  %421 = extractelement <4 x float> %420, i32 0   ; <float> [#uses=1]
  %422 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %423 = getelementptr inbounds float addrspace(3)* %422, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %421, float addrspace(3)* %423
  %424 = load <4 x float>* %zr2                   ; <<4 x float>> [#uses=1]
  %425 = extractelement <4 x float> %424, i32 1   ; <float> [#uses=1]
  %426 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %427 = getelementptr inbounds float addrspace(3)* %426, i32 68 ; <float addrspace(3)*> [#uses=1]
  store float %425, float addrspace(3)* %427
  %428 = load <4 x float>* %zr2                   ; <<4 x float>> [#uses=1]
  %429 = extractelement <4 x float> %428, i32 2   ; <float> [#uses=1]
  %430 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %431 = getelementptr inbounds float addrspace(3)* %430, i32 136 ; <float addrspace(3)*> [#uses=1]
  store float %429, float addrspace(3)* %431
  %432 = load <4 x float>* %zr2                   ; <<4 x float>> [#uses=1]
  %433 = extractelement <4 x float> %432, i32 3   ; <float> [#uses=1]
  %434 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %435 = getelementptr inbounds float addrspace(3)* %434, i32 204 ; <float addrspace(3)*> [#uses=1]
  store float %433, float addrspace(3)* %435
  %436 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %437 = getelementptr inbounds float addrspace(3)* %436, i32 272 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %437, float addrspace(3)** %lp
  %438 = load <4 x float>* %zr3                   ; <<4 x float>> [#uses=1]
  %439 = extractelement <4 x float> %438, i32 0   ; <float> [#uses=1]
  %440 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %441 = getelementptr inbounds float addrspace(3)* %440, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %439, float addrspace(3)* %441
  %442 = load <4 x float>* %zr3                   ; <<4 x float>> [#uses=1]
  %443 = extractelement <4 x float> %442, i32 1   ; <float> [#uses=1]
  %444 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %445 = getelementptr inbounds float addrspace(3)* %444, i32 68 ; <float addrspace(3)*> [#uses=1]
  store float %443, float addrspace(3)* %445
  %446 = load <4 x float>* %zr3                   ; <<4 x float>> [#uses=1]
  %447 = extractelement <4 x float> %446, i32 2   ; <float> [#uses=1]
  %448 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %449 = getelementptr inbounds float addrspace(3)* %448, i32 136 ; <float addrspace(3)*> [#uses=1]
  store float %447, float addrspace(3)* %449
  %450 = load <4 x float>* %zr3                   ; <<4 x float>> [#uses=1]
  %451 = extractelement <4 x float> %450, i32 3   ; <float> [#uses=1]
  %452 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %453 = getelementptr inbounds float addrspace(3)* %452, i32 204 ; <float addrspace(3)*> [#uses=1]
  store float %451, float addrspace(3)* %453
  %454 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %455 = getelementptr inbounds float addrspace(3)* %454, i32 272 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %455, float addrspace(3)** %lp
  %456 = load <4 x float>* %zi0                   ; <<4 x float>> [#uses=1]
  %457 = extractelement <4 x float> %456, i32 0   ; <float> [#uses=1]
  %458 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %459 = getelementptr inbounds float addrspace(3)* %458, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %457, float addrspace(3)* %459
  %460 = load <4 x float>* %zi0                   ; <<4 x float>> [#uses=1]
  %461 = extractelement <4 x float> %460, i32 1   ; <float> [#uses=1]
  %462 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %463 = getelementptr inbounds float addrspace(3)* %462, i32 68 ; <float addrspace(3)*> [#uses=1]
  store float %461, float addrspace(3)* %463
  %464 = load <4 x float>* %zi0                   ; <<4 x float>> [#uses=1]
  %465 = extractelement <4 x float> %464, i32 2   ; <float> [#uses=1]
  %466 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %467 = getelementptr inbounds float addrspace(3)* %466, i32 136 ; <float addrspace(3)*> [#uses=1]
  store float %465, float addrspace(3)* %467
  %468 = load <4 x float>* %zi0                   ; <<4 x float>> [#uses=1]
  %469 = extractelement <4 x float> %468, i32 3   ; <float> [#uses=1]
  %470 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %471 = getelementptr inbounds float addrspace(3)* %470, i32 204 ; <float addrspace(3)*> [#uses=1]
  store float %469, float addrspace(3)* %471
  %472 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %473 = getelementptr inbounds float addrspace(3)* %472, i32 272 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %473, float addrspace(3)** %lp
  %474 = load <4 x float>* %zi1                   ; <<4 x float>> [#uses=1]
  %475 = extractelement <4 x float> %474, i32 0   ; <float> [#uses=1]
  %476 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %477 = getelementptr inbounds float addrspace(3)* %476, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %475, float addrspace(3)* %477
  %478 = load <4 x float>* %zi1                   ; <<4 x float>> [#uses=1]
  %479 = extractelement <4 x float> %478, i32 1   ; <float> [#uses=1]
  %480 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %481 = getelementptr inbounds float addrspace(3)* %480, i32 68 ; <float addrspace(3)*> [#uses=1]
  store float %479, float addrspace(3)* %481
  %482 = load <4 x float>* %zi1                   ; <<4 x float>> [#uses=1]
  %483 = extractelement <4 x float> %482, i32 2   ; <float> [#uses=1]
  %484 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %485 = getelementptr inbounds float addrspace(3)* %484, i32 136 ; <float addrspace(3)*> [#uses=1]
  store float %483, float addrspace(3)* %485
  %486 = load <4 x float>* %zi1                   ; <<4 x float>> [#uses=1]
  %487 = extractelement <4 x float> %486, i32 3   ; <float> [#uses=1]
  %488 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %489 = getelementptr inbounds float addrspace(3)* %488, i32 204 ; <float addrspace(3)*> [#uses=1]
  store float %487, float addrspace(3)* %489
  %490 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %491 = getelementptr inbounds float addrspace(3)* %490, i32 272 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %491, float addrspace(3)** %lp
  %492 = load <4 x float>* %zi2                   ; <<4 x float>> [#uses=1]
  %493 = extractelement <4 x float> %492, i32 0   ; <float> [#uses=1]
  %494 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %495 = getelementptr inbounds float addrspace(3)* %494, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %493, float addrspace(3)* %495
  %496 = load <4 x float>* %zi2                   ; <<4 x float>> [#uses=1]
  %497 = extractelement <4 x float> %496, i32 1   ; <float> [#uses=1]
  %498 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %499 = getelementptr inbounds float addrspace(3)* %498, i32 68 ; <float addrspace(3)*> [#uses=1]
  store float %497, float addrspace(3)* %499
  %500 = load <4 x float>* %zi2                   ; <<4 x float>> [#uses=1]
  %501 = extractelement <4 x float> %500, i32 2   ; <float> [#uses=1]
  %502 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %503 = getelementptr inbounds float addrspace(3)* %502, i32 136 ; <float addrspace(3)*> [#uses=1]
  store float %501, float addrspace(3)* %503
  %504 = load <4 x float>* %zi2                   ; <<4 x float>> [#uses=1]
  %505 = extractelement <4 x float> %504, i32 3   ; <float> [#uses=1]
  %506 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %507 = getelementptr inbounds float addrspace(3)* %506, i32 204 ; <float addrspace(3)*> [#uses=1]
  store float %505, float addrspace(3)* %507
  %508 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %509 = getelementptr inbounds float addrspace(3)* %508, i32 272 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %509, float addrspace(3)** %lp
  %510 = load <4 x float>* %zi3                   ; <<4 x float>> [#uses=1]
  %511 = extractelement <4 x float> %510, i32 0   ; <float> [#uses=1]
  %512 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %513 = getelementptr inbounds float addrspace(3)* %512, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %511, float addrspace(3)* %513
  %514 = load <4 x float>* %zi3                   ; <<4 x float>> [#uses=1]
  %515 = extractelement <4 x float> %514, i32 1   ; <float> [#uses=1]
  %516 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %517 = getelementptr inbounds float addrspace(3)* %516, i32 68 ; <float addrspace(3)*> [#uses=1]
  store float %515, float addrspace(3)* %517
  %518 = load <4 x float>* %zi3                   ; <<4 x float>> [#uses=1]
  %519 = extractelement <4 x float> %518, i32 2   ; <float> [#uses=1]
  %520 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %521 = getelementptr inbounds float addrspace(3)* %520, i32 136 ; <float addrspace(3)*> [#uses=1]
  store float %519, float addrspace(3)* %521
  %522 = load <4 x float>* %zi3                   ; <<4 x float>> [#uses=1]
  %523 = extractelement <4 x float> %522, i32 3   ; <float> [#uses=1]
  %524 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %525 = getelementptr inbounds float addrspace(3)* %524, i32 204 ; <float addrspace(3)*> [#uses=1]
  store float %523, float addrspace(3)* %525
  call void @barrier(i32 1)
  ret void
}

define void @kfft_pass5(i32 %me, float addrspace(3)* %lds, float addrspace(1)* %gr, float addrspace(1)* %gi) nounwind alwaysinline {
  %1 = alloca i32, align 4                        ; <i32*> [#uses=5]
  %2 = alloca float addrspace(3)*, align 16       ; <float addrspace(3)**> [#uses=2]
  %3 = alloca float addrspace(1)*, align 16       ; <float addrspace(1)**> [#uses=2]
  %4 = alloca float addrspace(1)*, align 16       ; <float addrspace(1)**> [#uses=2]
  %lp = alloca float addrspace(3)*, align 16      ; <float addrspace(3)**> [#uses=47]
  %zr0 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=12]
  %zr1 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=12]
  %zr2 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=12]
  %zr3 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=12]
  %zi0 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=12]
  %zi1 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=12]
  %zi2 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=12]
  %zi3 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=12]
  %ar0 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %ar2 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %br0 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %br1 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %br2 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %br3 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %ai0 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %ai2 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %bi0 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %bi1 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %bi2 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %bi3 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %gp = alloca <4 x float> addrspace(1)*, align 16 ; <<4 x float> addrspace(1)**> [#uses=10]
  store i32 %me, i32* %1
  store float addrspace(3)* %lds, float addrspace(3)** %2
  store float addrspace(1)* %gr, float addrspace(1)** %3
  store float addrspace(1)* %gi, float addrspace(1)** %4
  %5 = load float addrspace(3)** %2               ; <float addrspace(3)*> [#uses=1]
  %6 = load i32* %1                               ; <i32> [#uses=1]
  %7 = and i32 %6, 15                             ; <i32> [#uses=1]
  %8 = load i32* %1                               ; <i32> [#uses=1]
  %9 = lshr i32 %8, 4                             ; <i32> [#uses=1]
  %10 = mul i32 %9, 272                           ; <i32> [#uses=1]
  %11 = add i32 %7, %10                           ; <i32> [#uses=1]
  %12 = getelementptr inbounds float addrspace(3)* %5, i32 %11 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %12, float addrspace(3)** %lp
  %13 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %14 = getelementptr inbounds float addrspace(3)* %13, i32 0 ; <float addrspace(3)*> [#uses=1]
  %15 = load float addrspace(3)* %14              ; <float> [#uses=1]
  %16 = load <4 x float>* %zr0                    ; <<4 x float>> [#uses=1]
  %17 = insertelement <4 x float> %16, float %15, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %17, <4 x float>* %zr0
  %18 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %19 = getelementptr inbounds float addrspace(3)* %18, i32 68 ; <float addrspace(3)*> [#uses=1]
  %20 = load float addrspace(3)* %19              ; <float> [#uses=1]
  %21 = load <4 x float>* %zr0                    ; <<4 x float>> [#uses=1]
  %22 = insertelement <4 x float> %21, float %20, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %22, <4 x float>* %zr0
  %23 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %24 = getelementptr inbounds float addrspace(3)* %23, i32 136 ; <float addrspace(3)*> [#uses=1]
  %25 = load float addrspace(3)* %24              ; <float> [#uses=1]
  %26 = load <4 x float>* %zr0                    ; <<4 x float>> [#uses=1]
  %27 = insertelement <4 x float> %26, float %25, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %27, <4 x float>* %zr0
  %28 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %29 = getelementptr inbounds float addrspace(3)* %28, i32 204 ; <float addrspace(3)*> [#uses=1]
  %30 = load float addrspace(3)* %29              ; <float> [#uses=1]
  %31 = load <4 x float>* %zr0                    ; <<4 x float>> [#uses=1]
  %32 = insertelement <4 x float> %31, float %30, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %32, <4 x float>* %zr0
  %33 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %34 = getelementptr inbounds float addrspace(3)* %33, i32 16 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %34, float addrspace(3)** %lp
  %35 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %36 = getelementptr inbounds float addrspace(3)* %35, i32 0 ; <float addrspace(3)*> [#uses=1]
  %37 = load float addrspace(3)* %36              ; <float> [#uses=1]
  %38 = load <4 x float>* %zr1                    ; <<4 x float>> [#uses=1]
  %39 = insertelement <4 x float> %38, float %37, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %39, <4 x float>* %zr1
  %40 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %41 = getelementptr inbounds float addrspace(3)* %40, i32 68 ; <float addrspace(3)*> [#uses=1]
  %42 = load float addrspace(3)* %41              ; <float> [#uses=1]
  %43 = load <4 x float>* %zr1                    ; <<4 x float>> [#uses=1]
  %44 = insertelement <4 x float> %43, float %42, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %44, <4 x float>* %zr1
  %45 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %46 = getelementptr inbounds float addrspace(3)* %45, i32 136 ; <float addrspace(3)*> [#uses=1]
  %47 = load float addrspace(3)* %46              ; <float> [#uses=1]
  %48 = load <4 x float>* %zr1                    ; <<4 x float>> [#uses=1]
  %49 = insertelement <4 x float> %48, float %47, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %49, <4 x float>* %zr1
  %50 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %51 = getelementptr inbounds float addrspace(3)* %50, i32 204 ; <float addrspace(3)*> [#uses=1]
  %52 = load float addrspace(3)* %51              ; <float> [#uses=1]
  %53 = load <4 x float>* %zr1                    ; <<4 x float>> [#uses=1]
  %54 = insertelement <4 x float> %53, float %52, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %54, <4 x float>* %zr1
  %55 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %56 = getelementptr inbounds float addrspace(3)* %55, i32 16 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %56, float addrspace(3)** %lp
  %57 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %58 = getelementptr inbounds float addrspace(3)* %57, i32 0 ; <float addrspace(3)*> [#uses=1]
  %59 = load float addrspace(3)* %58              ; <float> [#uses=1]
  %60 = load <4 x float>* %zr2                    ; <<4 x float>> [#uses=1]
  %61 = insertelement <4 x float> %60, float %59, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %61, <4 x float>* %zr2
  %62 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %63 = getelementptr inbounds float addrspace(3)* %62, i32 68 ; <float addrspace(3)*> [#uses=1]
  %64 = load float addrspace(3)* %63              ; <float> [#uses=1]
  %65 = load <4 x float>* %zr2                    ; <<4 x float>> [#uses=1]
  %66 = insertelement <4 x float> %65, float %64, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %66, <4 x float>* %zr2
  %67 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %68 = getelementptr inbounds float addrspace(3)* %67, i32 136 ; <float addrspace(3)*> [#uses=1]
  %69 = load float addrspace(3)* %68              ; <float> [#uses=1]
  %70 = load <4 x float>* %zr2                    ; <<4 x float>> [#uses=1]
  %71 = insertelement <4 x float> %70, float %69, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %71, <4 x float>* %zr2
  %72 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %73 = getelementptr inbounds float addrspace(3)* %72, i32 204 ; <float addrspace(3)*> [#uses=1]
  %74 = load float addrspace(3)* %73              ; <float> [#uses=1]
  %75 = load <4 x float>* %zr2                    ; <<4 x float>> [#uses=1]
  %76 = insertelement <4 x float> %75, float %74, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %76, <4 x float>* %zr2
  %77 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %78 = getelementptr inbounds float addrspace(3)* %77, i32 16 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %78, float addrspace(3)** %lp
  %79 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %80 = getelementptr inbounds float addrspace(3)* %79, i32 0 ; <float addrspace(3)*> [#uses=1]
  %81 = load float addrspace(3)* %80              ; <float> [#uses=1]
  %82 = load <4 x float>* %zr3                    ; <<4 x float>> [#uses=1]
  %83 = insertelement <4 x float> %82, float %81, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %83, <4 x float>* %zr3
  %84 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %85 = getelementptr inbounds float addrspace(3)* %84, i32 68 ; <float addrspace(3)*> [#uses=1]
  %86 = load float addrspace(3)* %85              ; <float> [#uses=1]
  %87 = load <4 x float>* %zr3                    ; <<4 x float>> [#uses=1]
  %88 = insertelement <4 x float> %87, float %86, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %88, <4 x float>* %zr3
  %89 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %90 = getelementptr inbounds float addrspace(3)* %89, i32 136 ; <float addrspace(3)*> [#uses=1]
  %91 = load float addrspace(3)* %90              ; <float> [#uses=1]
  %92 = load <4 x float>* %zr3                    ; <<4 x float>> [#uses=1]
  %93 = insertelement <4 x float> %92, float %91, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %93, <4 x float>* %zr3
  %94 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %95 = getelementptr inbounds float addrspace(3)* %94, i32 204 ; <float addrspace(3)*> [#uses=1]
  %96 = load float addrspace(3)* %95              ; <float> [#uses=1]
  %97 = load <4 x float>* %zr3                    ; <<4 x float>> [#uses=1]
  %98 = insertelement <4 x float> %97, float %96, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %98, <4 x float>* %zr3
  %99 = load float addrspace(3)** %lp             ; <float addrspace(3)*> [#uses=1]
  %100 = getelementptr inbounds float addrspace(3)* %99, i32 1040 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %100, float addrspace(3)** %lp
  %101 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %102 = getelementptr inbounds float addrspace(3)* %101, i32 0 ; <float addrspace(3)*> [#uses=1]
  %103 = load float addrspace(3)* %102            ; <float> [#uses=1]
  %104 = load <4 x float>* %zi0                   ; <<4 x float>> [#uses=1]
  %105 = insertelement <4 x float> %104, float %103, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %105, <4 x float>* %zi0
  %106 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %107 = getelementptr inbounds float addrspace(3)* %106, i32 68 ; <float addrspace(3)*> [#uses=1]
  %108 = load float addrspace(3)* %107            ; <float> [#uses=1]
  %109 = load <4 x float>* %zi0                   ; <<4 x float>> [#uses=1]
  %110 = insertelement <4 x float> %109, float %108, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %110, <4 x float>* %zi0
  %111 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %112 = getelementptr inbounds float addrspace(3)* %111, i32 136 ; <float addrspace(3)*> [#uses=1]
  %113 = load float addrspace(3)* %112            ; <float> [#uses=1]
  %114 = load <4 x float>* %zi0                   ; <<4 x float>> [#uses=1]
  %115 = insertelement <4 x float> %114, float %113, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %115, <4 x float>* %zi0
  %116 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %117 = getelementptr inbounds float addrspace(3)* %116, i32 204 ; <float addrspace(3)*> [#uses=1]
  %118 = load float addrspace(3)* %117            ; <float> [#uses=1]
  %119 = load <4 x float>* %zi0                   ; <<4 x float>> [#uses=1]
  %120 = insertelement <4 x float> %119, float %118, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %120, <4 x float>* %zi0
  %121 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %122 = getelementptr inbounds float addrspace(3)* %121, i32 16 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %122, float addrspace(3)** %lp
  %123 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %124 = getelementptr inbounds float addrspace(3)* %123, i32 0 ; <float addrspace(3)*> [#uses=1]
  %125 = load float addrspace(3)* %124            ; <float> [#uses=1]
  %126 = load <4 x float>* %zi1                   ; <<4 x float>> [#uses=1]
  %127 = insertelement <4 x float> %126, float %125, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %127, <4 x float>* %zi1
  %128 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %129 = getelementptr inbounds float addrspace(3)* %128, i32 68 ; <float addrspace(3)*> [#uses=1]
  %130 = load float addrspace(3)* %129            ; <float> [#uses=1]
  %131 = load <4 x float>* %zi1                   ; <<4 x float>> [#uses=1]
  %132 = insertelement <4 x float> %131, float %130, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %132, <4 x float>* %zi1
  %133 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %134 = getelementptr inbounds float addrspace(3)* %133, i32 136 ; <float addrspace(3)*> [#uses=1]
  %135 = load float addrspace(3)* %134            ; <float> [#uses=1]
  %136 = load <4 x float>* %zi1                   ; <<4 x float>> [#uses=1]
  %137 = insertelement <4 x float> %136, float %135, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %137, <4 x float>* %zi1
  %138 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %139 = getelementptr inbounds float addrspace(3)* %138, i32 204 ; <float addrspace(3)*> [#uses=1]
  %140 = load float addrspace(3)* %139            ; <float> [#uses=1]
  %141 = load <4 x float>* %zi1                   ; <<4 x float>> [#uses=1]
  %142 = insertelement <4 x float> %141, float %140, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %142, <4 x float>* %zi1
  %143 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %144 = getelementptr inbounds float addrspace(3)* %143, i32 16 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %144, float addrspace(3)** %lp
  %145 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %146 = getelementptr inbounds float addrspace(3)* %145, i32 0 ; <float addrspace(3)*> [#uses=1]
  %147 = load float addrspace(3)* %146            ; <float> [#uses=1]
  %148 = load <4 x float>* %zi2                   ; <<4 x float>> [#uses=1]
  %149 = insertelement <4 x float> %148, float %147, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %149, <4 x float>* %zi2
  %150 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %151 = getelementptr inbounds float addrspace(3)* %150, i32 68 ; <float addrspace(3)*> [#uses=1]
  %152 = load float addrspace(3)* %151            ; <float> [#uses=1]
  %153 = load <4 x float>* %zi2                   ; <<4 x float>> [#uses=1]
  %154 = insertelement <4 x float> %153, float %152, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %154, <4 x float>* %zi2
  %155 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %156 = getelementptr inbounds float addrspace(3)* %155, i32 136 ; <float addrspace(3)*> [#uses=1]
  %157 = load float addrspace(3)* %156            ; <float> [#uses=1]
  %158 = load <4 x float>* %zi2                   ; <<4 x float>> [#uses=1]
  %159 = insertelement <4 x float> %158, float %157, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %159, <4 x float>* %zi2
  %160 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %161 = getelementptr inbounds float addrspace(3)* %160, i32 204 ; <float addrspace(3)*> [#uses=1]
  %162 = load float addrspace(3)* %161            ; <float> [#uses=1]
  %163 = load <4 x float>* %zi2                   ; <<4 x float>> [#uses=1]
  %164 = insertelement <4 x float> %163, float %162, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %164, <4 x float>* %zi2
  %165 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %166 = getelementptr inbounds float addrspace(3)* %165, i32 16 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %166, float addrspace(3)** %lp
  %167 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %168 = getelementptr inbounds float addrspace(3)* %167, i32 0 ; <float addrspace(3)*> [#uses=1]
  %169 = load float addrspace(3)* %168            ; <float> [#uses=1]
  %170 = load <4 x float>* %zi3                   ; <<4 x float>> [#uses=1]
  %171 = insertelement <4 x float> %170, float %169, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %171, <4 x float>* %zi3
  %172 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %173 = getelementptr inbounds float addrspace(3)* %172, i32 68 ; <float addrspace(3)*> [#uses=1]
  %174 = load float addrspace(3)* %173            ; <float> [#uses=1]
  %175 = load <4 x float>* %zi3                   ; <<4 x float>> [#uses=1]
  %176 = insertelement <4 x float> %175, float %174, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %176, <4 x float>* %zi3
  %177 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %178 = getelementptr inbounds float addrspace(3)* %177, i32 136 ; <float addrspace(3)*> [#uses=1]
  %179 = load float addrspace(3)* %178            ; <float> [#uses=1]
  %180 = load <4 x float>* %zi3                   ; <<4 x float>> [#uses=1]
  %181 = insertelement <4 x float> %180, float %179, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %181, <4 x float>* %zi3
  %182 = load float addrspace(3)** %lp            ; <float addrspace(3)*> [#uses=1]
  %183 = getelementptr inbounds float addrspace(3)* %182, i32 204 ; <float addrspace(3)*> [#uses=1]
  %184 = load float addrspace(3)* %183            ; <float> [#uses=1]
  %185 = load <4 x float>* %zi3                   ; <<4 x float>> [#uses=1]
  %186 = insertelement <4 x float> %185, float %184, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %186, <4 x float>* %zi3
  br label %187

; <label>:187                                     ; preds = %0
  %188 = load <4 x float>* %zr0                   ; <<4 x float>> [#uses=1]
  %189 = load <4 x float>* %zr2                   ; <<4 x float>> [#uses=1]
  %190 = fadd <4 x float> %188, %189              ; <<4 x float>> [#uses=1]
  store <4 x float> %190, <4 x float>* %ar0
  %191 = load <4 x float>* %zr1                   ; <<4 x float>> [#uses=1]
  %192 = load <4 x float>* %zr3                   ; <<4 x float>> [#uses=1]
  %193 = fadd <4 x float> %191, %192              ; <<4 x float>> [#uses=1]
  store <4 x float> %193, <4 x float>* %ar2
  %194 = load <4 x float>* %ar0                   ; <<4 x float>> [#uses=1]
  %195 = load <4 x float>* %ar2                   ; <<4 x float>> [#uses=1]
  %196 = fadd <4 x float> %194, %195              ; <<4 x float>> [#uses=1]
  store <4 x float> %196, <4 x float>* %br0
  %197 = load <4 x float>* %zr0                   ; <<4 x float>> [#uses=1]
  %198 = load <4 x float>* %zr2                   ; <<4 x float>> [#uses=1]
  %199 = fsub <4 x float> %197, %198              ; <<4 x float>> [#uses=1]
  store <4 x float> %199, <4 x float>* %br1
  %200 = load <4 x float>* %ar0                   ; <<4 x float>> [#uses=1]
  %201 = load <4 x float>* %ar2                   ; <<4 x float>> [#uses=1]
  %202 = fsub <4 x float> %200, %201              ; <<4 x float>> [#uses=1]
  store <4 x float> %202, <4 x float>* %br2
  %203 = load <4 x float>* %zr1                   ; <<4 x float>> [#uses=1]
  %204 = load <4 x float>* %zr3                   ; <<4 x float>> [#uses=1]
  %205 = fsub <4 x float> %203, %204              ; <<4 x float>> [#uses=1]
  store <4 x float> %205, <4 x float>* %br3
  %206 = load <4 x float>* %zi0                   ; <<4 x float>> [#uses=1]
  %207 = load <4 x float>* %zi2                   ; <<4 x float>> [#uses=1]
  %208 = fadd <4 x float> %206, %207              ; <<4 x float>> [#uses=1]
  store <4 x float> %208, <4 x float>* %ai0
  %209 = load <4 x float>* %zi1                   ; <<4 x float>> [#uses=1]
  %210 = load <4 x float>* %zi3                   ; <<4 x float>> [#uses=1]
  %211 = fadd <4 x float> %209, %210              ; <<4 x float>> [#uses=1]
  store <4 x float> %211, <4 x float>* %ai2
  %212 = load <4 x float>* %ai0                   ; <<4 x float>> [#uses=1]
  %213 = load <4 x float>* %ai2                   ; <<4 x float>> [#uses=1]
  %214 = fadd <4 x float> %212, %213              ; <<4 x float>> [#uses=1]
  store <4 x float> %214, <4 x float>* %bi0
  %215 = load <4 x float>* %zi0                   ; <<4 x float>> [#uses=1]
  %216 = load <4 x float>* %zi2                   ; <<4 x float>> [#uses=1]
  %217 = fsub <4 x float> %215, %216              ; <<4 x float>> [#uses=1]
  store <4 x float> %217, <4 x float>* %bi1
  %218 = load <4 x float>* %ai0                   ; <<4 x float>> [#uses=1]
  %219 = load <4 x float>* %ai2                   ; <<4 x float>> [#uses=1]
  %220 = fsub <4 x float> %218, %219              ; <<4 x float>> [#uses=1]
  store <4 x float> %220, <4 x float>* %bi2
  %221 = load <4 x float>* %zi1                   ; <<4 x float>> [#uses=1]
  %222 = load <4 x float>* %zi3                   ; <<4 x float>> [#uses=1]
  %223 = fsub <4 x float> %221, %222              ; <<4 x float>> [#uses=1]
  store <4 x float> %223, <4 x float>* %bi3
  %224 = load <4 x float>* %br0                   ; <<4 x float>> [#uses=1]
  store <4 x float> %224, <4 x float>* %zr0
  %225 = load <4 x float>* %bi0                   ; <<4 x float>> [#uses=1]
  store <4 x float> %225, <4 x float>* %zi0
  %226 = load <4 x float>* %br1                   ; <<4 x float>> [#uses=1]
  %227 = load <4 x float>* %bi3                   ; <<4 x float>> [#uses=1]
  %228 = fadd <4 x float> %226, %227              ; <<4 x float>> [#uses=1]
  store <4 x float> %228, <4 x float>* %zr1
  %229 = load <4 x float>* %bi1                   ; <<4 x float>> [#uses=1]
  %230 = load <4 x float>* %br3                   ; <<4 x float>> [#uses=1]
  %231 = fsub <4 x float> %229, %230              ; <<4 x float>> [#uses=1]
  store <4 x float> %231, <4 x float>* %zi1
  %232 = load <4 x float>* %br1                   ; <<4 x float>> [#uses=1]
  %233 = load <4 x float>* %bi3                   ; <<4 x float>> [#uses=1]
  %234 = fsub <4 x float> %232, %233              ; <<4 x float>> [#uses=1]
  store <4 x float> %234, <4 x float>* %zr3
  %235 = load <4 x float>* %br3                   ; <<4 x float>> [#uses=1]
  %236 = load <4 x float>* %bi1                   ; <<4 x float>> [#uses=1]
  %237 = fadd <4 x float> %235, %236              ; <<4 x float>> [#uses=1]
  store <4 x float> %237, <4 x float>* %zi3
  %238 = load <4 x float>* %br2                   ; <<4 x float>> [#uses=1]
  store <4 x float> %238, <4 x float>* %zr2
  %239 = load <4 x float>* %bi2                   ; <<4 x float>> [#uses=1]
  store <4 x float> %239, <4 x float>* %zi2
  br label %240

; <label>:240                                     ; preds = %187
  %241 = load float addrspace(1)** %3             ; <float addrspace(1)*> [#uses=1]
  %242 = load i32* %1                             ; <i32> [#uses=1]
  %243 = shl i32 %242, 2                          ; <i32> [#uses=1]
  %244 = getelementptr inbounds float addrspace(1)* %241, i32 %243 ; <float addrspace(1)*> [#uses=1]
  %245 = bitcast float addrspace(1)* %244 to <4 x float> addrspace(1)* ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> addrspace(1)* %245, <4 x float> addrspace(1)** %gp
  %246 = load <4 x float>* %zr0                   ; <<4 x float>> [#uses=1]
  %247 = load <4 x float> addrspace(1)** %gp      ; <<4 x float> addrspace(1)*> [#uses=1]
  %248 = getelementptr inbounds <4 x float> addrspace(1)* %247, i32 0 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %246, <4 x float> addrspace(1)* %248
  %249 = load <4 x float>* %zr1                   ; <<4 x float>> [#uses=1]
  %250 = load <4 x float> addrspace(1)** %gp      ; <<4 x float> addrspace(1)*> [#uses=1]
  %251 = getelementptr inbounds <4 x float> addrspace(1)* %250, i32 64 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %249, <4 x float> addrspace(1)* %251
  %252 = load <4 x float>* %zr2                   ; <<4 x float>> [#uses=1]
  %253 = load <4 x float> addrspace(1)** %gp      ; <<4 x float> addrspace(1)*> [#uses=1]
  %254 = getelementptr inbounds <4 x float> addrspace(1)* %253, i32 128 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %252, <4 x float> addrspace(1)* %254
  %255 = load <4 x float>* %zr3                   ; <<4 x float>> [#uses=1]
  %256 = load <4 x float> addrspace(1)** %gp      ; <<4 x float> addrspace(1)*> [#uses=1]
  %257 = getelementptr inbounds <4 x float> addrspace(1)* %256, i32 192 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %255, <4 x float> addrspace(1)* %257
  %258 = load float addrspace(1)** %4             ; <float addrspace(1)*> [#uses=1]
  %259 = load i32* %1                             ; <i32> [#uses=1]
  %260 = shl i32 %259, 2                          ; <i32> [#uses=1]
  %261 = getelementptr inbounds float addrspace(1)* %258, i32 %260 ; <float addrspace(1)*> [#uses=1]
  %262 = bitcast float addrspace(1)* %261 to <4 x float> addrspace(1)* ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> addrspace(1)* %262, <4 x float> addrspace(1)** %gp
  %263 = load <4 x float>* %zi0                   ; <<4 x float>> [#uses=1]
  %264 = load <4 x float> addrspace(1)** %gp      ; <<4 x float> addrspace(1)*> [#uses=1]
  %265 = getelementptr inbounds <4 x float> addrspace(1)* %264, i32 0 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %263, <4 x float> addrspace(1)* %265
  %266 = load <4 x float>* %zi1                   ; <<4 x float>> [#uses=1]
  %267 = load <4 x float> addrspace(1)** %gp      ; <<4 x float> addrspace(1)*> [#uses=1]
  %268 = getelementptr inbounds <4 x float> addrspace(1)* %267, i32 64 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %266, <4 x float> addrspace(1)* %268
  %269 = load <4 x float>* %zi2                   ; <<4 x float>> [#uses=1]
  %270 = load <4 x float> addrspace(1)** %gp      ; <<4 x float> addrspace(1)*> [#uses=1]
  %271 = getelementptr inbounds <4 x float> addrspace(1)* %270, i32 128 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %269, <4 x float> addrspace(1)* %271
  %272 = load <4 x float>* %zi3                   ; <<4 x float>> [#uses=1]
  %273 = load <4 x float> addrspace(1)** %gp      ; <<4 x float> addrspace(1)*> [#uses=1]
  %274 = getelementptr inbounds <4 x float> addrspace(1)* %273, i32 192 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %272, <4 x float> addrspace(1)* %274
  ret void
}

define void @kfft(float addrspace(1)* %greal, float addrspace(1)* %gimag) nounwind {
  %1 = alloca float, align 4                      ; <float*> [#uses=2]
  %2 = alloca i32, align 4                        ; <i32*> [#uses=5]
  %3 = alloca float*, align 4                     ; <float**> [#uses=2]
  %x.i5.i92 = alloca float, align 4               ; <float*> [#uses=3]
  %4 = alloca float, align 4                      ; <float*> [#uses=2]
  %5 = alloca i32, align 4                        ; <i32*> [#uses=5]
  %6 = alloca float*, align 4                     ; <float**> [#uses=2]
  %x.i3.i93 = alloca float, align 4               ; <float*> [#uses=3]
  %7 = alloca float, align 4                      ; <float*> [#uses=2]
  %8 = alloca i32, align 4                        ; <i32*> [#uses=5]
  %9 = alloca float*, align 4                     ; <float**> [#uses=2]
  %x.i.i94 = alloca float, align 4                ; <float*> [#uses=3]
  %10 = alloca i32, align 4                       ; <i32*> [#uses=6]
  %11 = alloca float addrspace(3)*, align 16      ; <float addrspace(3)**> [#uses=3]
  %lp.i95 = alloca float addrspace(3)*, align 16  ; <float addrspace(3)**> [#uses=94]
  %zr0.i96 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=15]
  %zr1.i97 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=18]
  %zr2.i98 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=18]
  %zr3.i99 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=18]
  %zi0.i100 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=15]
  %zi1.i101 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=18]
  %zi2.i102 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=18]
  %zi3.i103 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=18]
  %ar0.i104 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=3]
  %ar2.i105 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=3]
  %br0.i106 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=2]
  %br1.i107 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=3]
  %br2.i108 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=2]
  %br3.i109 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=3]
  %ai0.i110 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=3]
  %ai2.i111 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=3]
  %bi0.i112 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=2]
  %bi1.i113 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=3]
  %bi2.i114 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=2]
  %bi3.i115 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=3]
  %tbase.i116 = alloca i32, align 4               ; <i32*> [#uses=4]
  %c1.i117 = alloca float, align 4                ; <float*> [#uses=3]
  %s1.i118 = alloca float, align 4                ; <float*> [#uses=3]
  %__r.i119 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=2]
  %c2.i120 = alloca float, align 4                ; <float*> [#uses=3]
  %s2.i121 = alloca float, align 4                ; <float*> [#uses=3]
  %__r1.i122 = alloca <4 x float>, align 16       ; <<4 x float>*> [#uses=2]
  %c3.i123 = alloca float, align 4                ; <float*> [#uses=3]
  %s3.i124 = alloca float, align 4                ; <float*> [#uses=3]
  %__r2.i125 = alloca <4 x float>, align 16       ; <<4 x float>*> [#uses=2]
  %12 = alloca float, align 4                     ; <float*> [#uses=2]
  %13 = alloca i32, align 4                       ; <i32*> [#uses=5]
  %14 = alloca float*, align 4                    ; <float**> [#uses=2]
  %x.i5.i56 = alloca float, align 4               ; <float*> [#uses=3]
  %15 = alloca float, align 4                     ; <float*> [#uses=2]
  %16 = alloca i32, align 4                       ; <i32*> [#uses=5]
  %17 = alloca float*, align 4                    ; <float**> [#uses=2]
  %x.i3.i57 = alloca float, align 4               ; <float*> [#uses=3]
  %18 = alloca float, align 4                     ; <float*> [#uses=2]
  %19 = alloca i32, align 4                       ; <i32*> [#uses=5]
  %20 = alloca float*, align 4                    ; <float**> [#uses=2]
  %x.i.i58 = alloca float, align 4                ; <float*> [#uses=3]
  %21 = alloca i32, align 4                       ; <i32*> [#uses=5]
  %22 = alloca float addrspace(3)*, align 16      ; <float addrspace(3)**> [#uses=3]
  %lp.i59 = alloca float addrspace(3)*, align 16  ; <float addrspace(3)**> [#uses=94]
  %zr0.i60 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=15]
  %zr1.i61 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=18]
  %zr2.i62 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=18]
  %zr3.i63 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=18]
  %zi0.i64 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=15]
  %zi1.i65 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=18]
  %zi2.i66 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=18]
  %zi3.i67 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=18]
  %ar0.i68 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=3]
  %ar2.i69 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=3]
  %br0.i70 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=2]
  %br1.i71 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=3]
  %br2.i72 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=2]
  %br3.i73 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=3]
  %ai0.i74 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=3]
  %ai2.i75 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=3]
  %bi0.i76 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=2]
  %bi1.i77 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=3]
  %bi2.i78 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=2]
  %bi3.i79 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=3]
  %tbase.i80 = alloca i32, align 4                ; <i32*> [#uses=4]
  %c1.i81 = alloca float, align 4                 ; <float*> [#uses=3]
  %s1.i82 = alloca float, align 4                 ; <float*> [#uses=3]
  %__r.i83 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=2]
  %c2.i84 = alloca float, align 4                 ; <float*> [#uses=3]
  %s2.i85 = alloca float, align 4                 ; <float*> [#uses=3]
  %__r1.i86 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=2]
  %c3.i87 = alloca float, align 4                 ; <float*> [#uses=3]
  %s3.i88 = alloca float, align 4                 ; <float*> [#uses=3]
  %__r2.i89 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=2]
  %23 = alloca float, align 4                     ; <float*> [#uses=2]
  %24 = alloca i32, align 4                       ; <i32*> [#uses=5]
  %25 = alloca float*, align 4                    ; <float**> [#uses=2]
  %x.i5.i = alloca float, align 4                 ; <float*> [#uses=3]
  %26 = alloca float, align 4                     ; <float*> [#uses=2]
  %27 = alloca i32, align 4                       ; <i32*> [#uses=5]
  %28 = alloca float*, align 4                    ; <float**> [#uses=2]
  %x.i3.i23 = alloca float, align 4               ; <float*> [#uses=3]
  %29 = alloca float, align 4                     ; <float*> [#uses=2]
  %30 = alloca i32, align 4                       ; <i32*> [#uses=5]
  %31 = alloca float*, align 4                    ; <float**> [#uses=2]
  %x.i.i24 = alloca float, align 4                ; <float*> [#uses=3]
  %32 = alloca i32, align 4                       ; <i32*> [#uses=6]
  %33 = alloca float addrspace(3)*, align 16      ; <float addrspace(3)**> [#uses=3]
  %lp.i25 = alloca float addrspace(3)*, align 16  ; <float addrspace(3)**> [#uses=94]
  %zr0.i26 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=15]
  %zr1.i27 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=18]
  %zr2.i28 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=18]
  %zr3.i29 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=18]
  %zi0.i30 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=15]
  %zi1.i31 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=18]
  %zi2.i32 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=18]
  %zi3.i33 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=18]
  %ar0.i34 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=3]
  %ar2.i35 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=3]
  %br0.i36 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=2]
  %br1.i37 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=3]
  %br2.i38 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=2]
  %br3.i39 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=3]
  %ai0.i40 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=3]
  %ai2.i41 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=3]
  %bi0.i42 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=2]
  %bi1.i43 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=3]
  %bi2.i44 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=2]
  %bi3.i45 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=3]
  %tbase.i46 = alloca i32, align 4                ; <i32*> [#uses=4]
  %c1.i47 = alloca float, align 4                 ; <float*> [#uses=3]
  %s1.i48 = alloca float, align 4                 ; <float*> [#uses=3]
  %__r.i49 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=2]
  %c2.i50 = alloca float, align 4                 ; <float*> [#uses=3]
  %s2.i51 = alloca float, align 4                 ; <float*> [#uses=3]
  %__r1.i52 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=2]
  %c3.i53 = alloca float, align 4                 ; <float*> [#uses=3]
  %s3.i54 = alloca float, align 4                 ; <float*> [#uses=3]
  %__r2.i55 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=2]
  %34 = alloca i32, align 4                       ; <i32*> [#uses=5]
  %35 = alloca float addrspace(3)*, align 16      ; <float addrspace(3)**> [#uses=2]
  %36 = alloca float addrspace(1)*, align 16      ; <float addrspace(1)**> [#uses=2]
  %37 = alloca float addrspace(1)*, align 16      ; <float addrspace(1)**> [#uses=2]
  %lp.i1 = alloca float addrspace(3)*, align 16   ; <float addrspace(3)**> [#uses=47]
  %zr0.i2 = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=12]
  %zr1.i3 = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=12]
  %zr2.i4 = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=12]
  %zr3.i5 = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=12]
  %zi0.i6 = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=12]
  %zi1.i7 = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=12]
  %zi2.i8 = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=12]
  %zi3.i9 = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=12]
  %ar0.i10 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=3]
  %ar2.i11 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=3]
  %br0.i12 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=2]
  %br1.i13 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=3]
  %br2.i14 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=2]
  %br3.i15 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=3]
  %ai0.i16 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=3]
  %ai2.i17 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=3]
  %bi0.i18 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=2]
  %bi1.i19 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=3]
  %bi2.i20 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=2]
  %bi3.i21 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=3]
  %gp.i22 = alloca <4 x float> addrspace(1)*, align 16 ; <<4 x float> addrspace(1)**> [#uses=10]
  %38 = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=2]
  %39 = alloca <4 x i32>, align 16                ; <<4 x i32>*> [#uses=5]
  %40 = alloca <4 x float>*, align 4              ; <<4 x float>**> [#uses=2]
  %x.i4.i = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=3]
  %41 = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=2]
  %42 = alloca <4 x i32>, align 16                ; <<4 x i32>*> [#uses=5]
  %43 = alloca <4 x float>*, align 4              ; <<4 x float>**> [#uses=2]
  %x.i3.i = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=3]
  %44 = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=2]
  %45 = alloca <4 x i32>, align 16                ; <<4 x i32>*> [#uses=5]
  %46 = alloca <4 x float>*, align 4              ; <<4 x float>**> [#uses=2]
  %x.i.i = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=3]
  %47 = alloca i32, align 4                       ; <i32*> [#uses=6]
  %48 = alloca float addrspace(1)*, align 16      ; <float addrspace(1)**> [#uses=2]
  %49 = alloca float addrspace(1)*, align 16      ; <float addrspace(1)**> [#uses=2]
  %50 = alloca float addrspace(3)*, align 16      ; <float addrspace(3)**> [#uses=2]
  %gp.i = alloca <4 x float> addrspace(1)*, align 16 ; <<4 x float> addrspace(1)**> [#uses=10]
  %lp.i = alloca float addrspace(3)*, align 16    ; <float addrspace(3)**> [#uses=47]
  %zr0.i = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=8]
  %zr1.i = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=11]
  %zr2.i = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=11]
  %zr3.i = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=11]
  %zi0.i = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=8]
  %zi1.i = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=11]
  %zi2.i = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=11]
  %zi3.i = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=11]
  %ar0.i = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=3]
  %ar2.i = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=3]
  %br0.i = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=2]
  %br1.i = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=3]
  %br2.i = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=2]
  %br3.i = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=3]
  %ai0.i = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=3]
  %ai2.i = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=3]
  %bi0.i = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=2]
  %bi1.i = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=3]
  %bi2.i = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=2]
  %bi3.i = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=3]
  %tbase.i = alloca <4 x i32>, align 16           ; <<4 x i32>*> [#uses=4]
  %51 = alloca <4 x i32>, align 16                ; <<4 x i32>*> [#uses=2]
  %c1.i = alloca <4 x float>, align 16            ; <<4 x float>*> [#uses=3]
  %s1.i = alloca <4 x float>, align 16            ; <<4 x float>*> [#uses=3]
  %__r.i = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=2]
  %c2.i = alloca <4 x float>, align 16            ; <<4 x float>*> [#uses=3]
  %s2.i = alloca <4 x float>, align 16            ; <<4 x float>*> [#uses=3]
  %__r1.i = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=2]
  %c3.i = alloca <4 x float>, align 16            ; <<4 x float>*> [#uses=3]
  %s3.i = alloca <4 x float>, align 16            ; <<4 x float>*> [#uses=3]
  %__r2.i = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=2]
  %52 = alloca float addrspace(1)*, align 16      ; <float addrspace(1)**> [#uses=2]
  %53 = alloca float addrspace(1)*, align 16      ; <float addrspace(1)**> [#uses=2]
  %gr = alloca float addrspace(1)*, align 16      ; <float addrspace(1)**> [#uses=3]
  %gi = alloca float addrspace(1)*, align 16      ; <float addrspace(1)**> [#uses=3]
  %gid = alloca i32, align 4                      ; <i32*> [#uses=3]
  %me = alloca i32, align 4                       ; <i32*> [#uses=6]
  %dg = alloca i32, align 4                       ; <i32*> [#uses=3]
  store float addrspace(1)* %greal, float addrspace(1)** %52
  store float addrspace(1)* %gimag, float addrspace(1)** %53
  %54 = call i32 @get_global_id(i32 0)            ; <i32> [#uses=1]
  store i32 %54, i32* %gid
  %55 = load i32* %gid                            ; <i32> [#uses=1]
  %56 = and i32 %55, 63                           ; <i32> [#uses=1]
  store i32 %56, i32* %me
  %57 = load i32* %gid                            ; <i32> [#uses=1]
  %58 = lshr i32 %57, 6                           ; <i32> [#uses=1]
  %59 = mul i32 %58, 1024                         ; <i32> [#uses=1]
  store i32 %59, i32* %dg
  %60 = load float addrspace(1)** %52             ; <float addrspace(1)*> [#uses=1]
  %61 = load i32* %dg                             ; <i32> [#uses=1]
  %62 = getelementptr inbounds float addrspace(1)* %60, i32 %61 ; <float addrspace(1)*> [#uses=1]
  store float addrspace(1)* %62, float addrspace(1)** %gr
  %63 = load float addrspace(1)** %53             ; <float addrspace(1)*> [#uses=1]
  %64 = load i32* %dg                             ; <i32> [#uses=1]
  %65 = getelementptr inbounds float addrspace(1)* %63, i32 %64 ; <float addrspace(1)*> [#uses=1]
  store float addrspace(1)* %65, float addrspace(1)** %gi
  %66 = load i32* %me                             ; <i32> [#uses=1]
  %67 = load float addrspace(1)** %gr             ; <float addrspace(1)*> [#uses=1]
  %68 = load float addrspace(1)** %gi             ; <float addrspace(1)*> [#uses=1]
  store i32 %66, i32* %47
  store float addrspace(1)* %67, float addrspace(1)** %48
  store float addrspace(1)* %68, float addrspace(1)** %49
  store float addrspace(3)* getelementptr inbounds ([2176 x float] addrspace(3)* @opencl_kfft_local_lds, i32 0, i32 0), float addrspace(3)** %50
  %69 = load float addrspace(1)** %48             ; <float addrspace(1)*> [#uses=1]
  %70 = load i32* %47                             ; <i32> [#uses=1]
  %71 = shl i32 %70, 2                            ; <i32> [#uses=1]
  %72 = getelementptr inbounds float addrspace(1)* %69, i32 %71 ; <float addrspace(1)*> [#uses=1]
  %73 = bitcast float addrspace(1)* %72 to <4 x float> addrspace(1)* ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> addrspace(1)* %73, <4 x float> addrspace(1)** %gp.i
  %74 = load <4 x float> addrspace(1)** %gp.i     ; <<4 x float> addrspace(1)*> [#uses=1]
  %75 = getelementptr inbounds <4 x float> addrspace(1)* %74, i32 0 ; <<4 x float> addrspace(1)*> [#uses=1]
  %76 = load <4 x float> addrspace(1)* %75        ; <<4 x float>> [#uses=1]
  store <4 x float> %76, <4 x float>* %zr0.i
  %77 = load <4 x float> addrspace(1)** %gp.i     ; <<4 x float> addrspace(1)*> [#uses=1]
  %78 = getelementptr inbounds <4 x float> addrspace(1)* %77, i32 64 ; <<4 x float> addrspace(1)*> [#uses=1]
  %79 = load <4 x float> addrspace(1)* %78        ; <<4 x float>> [#uses=1]
  store <4 x float> %79, <4 x float>* %zr1.i
  %80 = load <4 x float> addrspace(1)** %gp.i     ; <<4 x float> addrspace(1)*> [#uses=1]
  %81 = getelementptr inbounds <4 x float> addrspace(1)* %80, i32 128 ; <<4 x float> addrspace(1)*> [#uses=1]
  %82 = load <4 x float> addrspace(1)* %81        ; <<4 x float>> [#uses=1]
  store <4 x float> %82, <4 x float>* %zr2.i
  %83 = load <4 x float> addrspace(1)** %gp.i     ; <<4 x float> addrspace(1)*> [#uses=1]
  %84 = getelementptr inbounds <4 x float> addrspace(1)* %83, i32 192 ; <<4 x float> addrspace(1)*> [#uses=1]
  %85 = load <4 x float> addrspace(1)* %84        ; <<4 x float>> [#uses=1]
  store <4 x float> %85, <4 x float>* %zr3.i
  %86 = load float addrspace(1)** %49             ; <float addrspace(1)*> [#uses=1]
  %87 = load i32* %47                             ; <i32> [#uses=1]
  %88 = shl i32 %87, 2                            ; <i32> [#uses=1]
  %89 = getelementptr inbounds float addrspace(1)* %86, i32 %88 ; <float addrspace(1)*> [#uses=1]
  %90 = bitcast float addrspace(1)* %89 to <4 x float> addrspace(1)* ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> addrspace(1)* %90, <4 x float> addrspace(1)** %gp.i
  %91 = load <4 x float> addrspace(1)** %gp.i     ; <<4 x float> addrspace(1)*> [#uses=1]
  %92 = getelementptr inbounds <4 x float> addrspace(1)* %91, i32 0 ; <<4 x float> addrspace(1)*> [#uses=1]
  %93 = load <4 x float> addrspace(1)* %92        ; <<4 x float>> [#uses=1]
  store <4 x float> %93, <4 x float>* %zi0.i
  %94 = load <4 x float> addrspace(1)** %gp.i     ; <<4 x float> addrspace(1)*> [#uses=1]
  %95 = getelementptr inbounds <4 x float> addrspace(1)* %94, i32 64 ; <<4 x float> addrspace(1)*> [#uses=1]
  %96 = load <4 x float> addrspace(1)* %95        ; <<4 x float>> [#uses=1]
  store <4 x float> %96, <4 x float>* %zi1.i
  %97 = load <4 x float> addrspace(1)** %gp.i     ; <<4 x float> addrspace(1)*> [#uses=1]
  %98 = getelementptr inbounds <4 x float> addrspace(1)* %97, i32 128 ; <<4 x float> addrspace(1)*> [#uses=1]
  %99 = load <4 x float> addrspace(1)* %98        ; <<4 x float>> [#uses=1]
  store <4 x float> %99, <4 x float>* %zi2.i
  %100 = load <4 x float> addrspace(1)** %gp.i    ; <<4 x float> addrspace(1)*> [#uses=1]
  %101 = getelementptr inbounds <4 x float> addrspace(1)* %100, i32 192 ; <<4 x float> addrspace(1)*> [#uses=1]
  %102 = load <4 x float> addrspace(1)* %101      ; <<4 x float>> [#uses=1]
  store <4 x float> %102, <4 x float>* %zi3.i
  %103 = load <4 x float>* %zr0.i                 ; <<4 x float>> [#uses=1]
  %104 = load <4 x float>* %zr2.i                 ; <<4 x float>> [#uses=1]
  %105 = fadd <4 x float> %103, %104              ; <<4 x float>> [#uses=1]
  store <4 x float> %105, <4 x float>* %ar0.i
  %106 = load <4 x float>* %zr1.i                 ; <<4 x float>> [#uses=1]
  %107 = load <4 x float>* %zr3.i                 ; <<4 x float>> [#uses=1]
  %108 = fadd <4 x float> %106, %107              ; <<4 x float>> [#uses=1]
  store <4 x float> %108, <4 x float>* %ar2.i
  %109 = load <4 x float>* %ar0.i                 ; <<4 x float>> [#uses=1]
  %110 = load <4 x float>* %ar2.i                 ; <<4 x float>> [#uses=1]
  %111 = fadd <4 x float> %109, %110              ; <<4 x float>> [#uses=1]
  store <4 x float> %111, <4 x float>* %br0.i
  %112 = load <4 x float>* %zr0.i                 ; <<4 x float>> [#uses=1]
  %113 = load <4 x float>* %zr2.i                 ; <<4 x float>> [#uses=1]
  %114 = fsub <4 x float> %112, %113              ; <<4 x float>> [#uses=1]
  store <4 x float> %114, <4 x float>* %br1.i
  %115 = load <4 x float>* %ar0.i                 ; <<4 x float>> [#uses=1]
  %116 = load <4 x float>* %ar2.i                 ; <<4 x float>> [#uses=1]
  %117 = fsub <4 x float> %115, %116              ; <<4 x float>> [#uses=1]
  store <4 x float> %117, <4 x float>* %br2.i
  %118 = load <4 x float>* %zr1.i                 ; <<4 x float>> [#uses=1]
  %119 = load <4 x float>* %zr3.i                 ; <<4 x float>> [#uses=1]
  %120 = fsub <4 x float> %118, %119              ; <<4 x float>> [#uses=1]
  store <4 x float> %120, <4 x float>* %br3.i
  %121 = load <4 x float>* %zi0.i                 ; <<4 x float>> [#uses=1]
  %122 = load <4 x float>* %zi2.i                 ; <<4 x float>> [#uses=1]
  %123 = fadd <4 x float> %121, %122              ; <<4 x float>> [#uses=1]
  store <4 x float> %123, <4 x float>* %ai0.i
  %124 = load <4 x float>* %zi1.i                 ; <<4 x float>> [#uses=1]
  %125 = load <4 x float>* %zi3.i                 ; <<4 x float>> [#uses=1]
  %126 = fadd <4 x float> %124, %125              ; <<4 x float>> [#uses=1]
  store <4 x float> %126, <4 x float>* %ai2.i
  %127 = load <4 x float>* %ai0.i                 ; <<4 x float>> [#uses=1]
  %128 = load <4 x float>* %ai2.i                 ; <<4 x float>> [#uses=1]
  %129 = fadd <4 x float> %127, %128              ; <<4 x float>> [#uses=1]
  store <4 x float> %129, <4 x float>* %bi0.i
  %130 = load <4 x float>* %zi0.i                 ; <<4 x float>> [#uses=1]
  %131 = load <4 x float>* %zi2.i                 ; <<4 x float>> [#uses=1]
  %132 = fsub <4 x float> %130, %131              ; <<4 x float>> [#uses=1]
  store <4 x float> %132, <4 x float>* %bi1.i
  %133 = load <4 x float>* %ai0.i                 ; <<4 x float>> [#uses=1]
  %134 = load <4 x float>* %ai2.i                 ; <<4 x float>> [#uses=1]
  %135 = fsub <4 x float> %133, %134              ; <<4 x float>> [#uses=1]
  store <4 x float> %135, <4 x float>* %bi2.i
  %136 = load <4 x float>* %zi1.i                 ; <<4 x float>> [#uses=1]
  %137 = load <4 x float>* %zi3.i                 ; <<4 x float>> [#uses=1]
  %138 = fsub <4 x float> %136, %137              ; <<4 x float>> [#uses=1]
  store <4 x float> %138, <4 x float>* %bi3.i
  %139 = load <4 x float>* %br0.i                 ; <<4 x float>> [#uses=1]
  store <4 x float> %139, <4 x float>* %zr0.i
  %140 = load <4 x float>* %bi0.i                 ; <<4 x float>> [#uses=1]
  store <4 x float> %140, <4 x float>* %zi0.i
  %141 = load <4 x float>* %br1.i                 ; <<4 x float>> [#uses=1]
  %142 = load <4 x float>* %bi3.i                 ; <<4 x float>> [#uses=1]
  %143 = fadd <4 x float> %141, %142              ; <<4 x float>> [#uses=1]
  store <4 x float> %143, <4 x float>* %zr1.i
  %144 = load <4 x float>* %bi1.i                 ; <<4 x float>> [#uses=1]
  %145 = load <4 x float>* %br3.i                 ; <<4 x float>> [#uses=1]
  %146 = fsub <4 x float> %144, %145              ; <<4 x float>> [#uses=1]
  store <4 x float> %146, <4 x float>* %zi1.i
  %147 = load <4 x float>* %br1.i                 ; <<4 x float>> [#uses=1]
  %148 = load <4 x float>* %bi3.i                 ; <<4 x float>> [#uses=1]
  %149 = fsub <4 x float> %147, %148              ; <<4 x float>> [#uses=1]
  store <4 x float> %149, <4 x float>* %zr3.i
  %150 = load <4 x float>* %br3.i                 ; <<4 x float>> [#uses=1]
  %151 = load <4 x float>* %bi1.i                 ; <<4 x float>> [#uses=1]
  %152 = fadd <4 x float> %150, %151              ; <<4 x float>> [#uses=1]
  store <4 x float> %152, <4 x float>* %zi3.i
  %153 = load <4 x float>* %br2.i                 ; <<4 x float>> [#uses=1]
  store <4 x float> %153, <4 x float>* %zr2.i
  %154 = load <4 x float>* %bi2.i                 ; <<4 x float>> [#uses=1]
  store <4 x float> %154, <4 x float>* %zi2.i
  %155 = load i32* %47                            ; <i32> [#uses=1]
  %156 = shl i32 %155, 2                          ; <i32> [#uses=1]
  %157 = insertelement <4 x i32> undef, i32 %156, i32 0 ; <<4 x i32>> [#uses=2]
  %158 = shufflevector <4 x i32> %157, <4 x i32> %157, <4 x i32> zeroinitializer ; <<4 x i32>> [#uses=1]
  store <4 x i32> <i32 0, i32 1, i32 2, i32 3>, <4 x i32>* %51
  %159 = load <4 x i32>* %51                      ; <<4 x i32>> [#uses=1]
  %160 = add nsw <4 x i32> %158, %159             ; <<4 x i32>> [#uses=1]
  store <4 x i32> %160, <4 x i32>* %tbase.i
  %161 = load <4 x i32>* %tbase.i                 ; <<4 x i32>> [#uses=1]
  %162 = mul <4 x i32> %161, <i32 1, i32 1, i32 1, i32 1> ; <<4 x i32>> [#uses=1]
  store <4 x i32> %162, <4 x i32>* %45
  store <4 x float>* %c1.i, <4 x float>** %46
  %163 = load <4 x i32>* %45                      ; <<4 x i32>> [#uses=1]
  %164 = icmp sgt <4 x i32> %163, <i32 512, i32 512, i32 512, i32 512> ; <<4 x i1>> [#uses=1]
  %165 = sext <4 x i1> %164 to <4 x i32>          ; <<4 x i32>> [#uses=1]
  %166 = and <4 x i32> %165, <i32 1024, i32 1024, i32 1024, i32 1024> ; <<4 x i32>> [#uses=1]
  %167 = load <4 x i32>* %45                      ; <<4 x i32>> [#uses=1]
  %168 = sub <4 x i32> %167, %166                 ; <<4 x i32>> [#uses=1]
  store <4 x i32> %168, <4 x i32>* %45
  %169 = load <4 x i32>* %45                      ; <<4 x i32>> [#uses=1]
  %170 = call <4 x float> @_Z14convert_float4U8__vector4i(<4 x i32> %169) nounwind ; <<4 x float>> [#uses=1]
  %171 = fmul <4 x float> %170, <float 0xBF7921FB60000000, float 0xBF7921FB60000000, float 0xBF7921FB60000000, float 0xBF7921FB60000000> ; <<4 x float>> [#uses=1]
  store <4 x float> %171, <4 x float>* %x.i.i
  %172 = load <4 x float>* %x.i.i                 ; <<4 x float>> [#uses=1]
  %173 = call <4 x float> @_Z10native_cosU8__vector4f(<4 x float> %172) nounwind ; <<4 x float>> [#uses=1]
  %174 = load <4 x float>** %46                   ; <<4 x float>*> [#uses=1]
  store <4 x float> %173, <4 x float>* %174
  %175 = load <4 x float>* %x.i.i                 ; <<4 x float>> [#uses=1]
  %176 = call <4 x float> @_Z10native_sinU8__vector4f(<4 x float> %175) nounwind ; <<4 x float>> [#uses=1]
  store <4 x float> %176, <4 x float>* %44
  %177 = load <4 x float>* %44                    ; <<4 x float>> [#uses=1]
  store <4 x float> %177, <4 x float>* %s1.i
  %178 = load <4 x float>* %c1.i                  ; <<4 x float>> [#uses=1]
  %179 = load <4 x float>* %zr1.i                 ; <<4 x float>> [#uses=1]
  %180 = fmul <4 x float> %178, %179              ; <<4 x float>> [#uses=1]
  %181 = load <4 x float>* %s1.i                  ; <<4 x float>> [#uses=1]
  %182 = load <4 x float>* %zi1.i                 ; <<4 x float>> [#uses=1]
  %183 = fmul <4 x float> %181, %182              ; <<4 x float>> [#uses=1]
  %184 = fsub <4 x float> %180, %183              ; <<4 x float>> [#uses=1]
  store <4 x float> %184, <4 x float>* %__r.i
  %185 = load <4 x float>* %c1.i                  ; <<4 x float>> [#uses=1]
  %186 = load <4 x float>* %zi1.i                 ; <<4 x float>> [#uses=1]
  %187 = fmul <4 x float> %185, %186              ; <<4 x float>> [#uses=1]
  %188 = load <4 x float>* %s1.i                  ; <<4 x float>> [#uses=1]
  %189 = load <4 x float>* %zr1.i                 ; <<4 x float>> [#uses=1]
  %190 = fmul <4 x float> %188, %189              ; <<4 x float>> [#uses=1]
  %191 = fadd <4 x float> %187, %190              ; <<4 x float>> [#uses=1]
  store <4 x float> %191, <4 x float>* %zi1.i
  %192 = load <4 x float>* %__r.i                 ; <<4 x float>> [#uses=1]
  store <4 x float> %192, <4 x float>* %zr1.i
  %193 = load <4 x i32>* %tbase.i                 ; <<4 x i32>> [#uses=1]
  %194 = mul <4 x i32> %193, <i32 2, i32 2, i32 2, i32 2> ; <<4 x i32>> [#uses=1]
  store <4 x i32> %194, <4 x i32>* %39
  store <4 x float>* %c2.i, <4 x float>** %40
  %195 = load <4 x i32>* %39                      ; <<4 x i32>> [#uses=1]
  %196 = icmp sgt <4 x i32> %195, <i32 512, i32 512, i32 512, i32 512> ; <<4 x i1>> [#uses=1]
  %197 = sext <4 x i1> %196 to <4 x i32>          ; <<4 x i32>> [#uses=1]
  %198 = and <4 x i32> %197, <i32 1024, i32 1024, i32 1024, i32 1024> ; <<4 x i32>> [#uses=1]
  %199 = load <4 x i32>* %39                      ; <<4 x i32>> [#uses=1]
  %200 = sub <4 x i32> %199, %198                 ; <<4 x i32>> [#uses=1]
  store <4 x i32> %200, <4 x i32>* %39
  %201 = load <4 x i32>* %39                      ; <<4 x i32>> [#uses=1]
  %202 = call <4 x float> @_Z14convert_float4U8__vector4i(<4 x i32> %201) nounwind ; <<4 x float>> [#uses=1]
  %203 = fmul <4 x float> %202, <float 0xBF7921FB60000000, float 0xBF7921FB60000000, float 0xBF7921FB60000000, float 0xBF7921FB60000000> ; <<4 x float>> [#uses=1]
  store <4 x float> %203, <4 x float>* %x.i4.i
  %204 = load <4 x float>* %x.i4.i                ; <<4 x float>> [#uses=1]
  %205 = call <4 x float> @_Z10native_cosU8__vector4f(<4 x float> %204) nounwind ; <<4 x float>> [#uses=1]
  %206 = load <4 x float>** %40                   ; <<4 x float>*> [#uses=1]
  store <4 x float> %205, <4 x float>* %206
  %207 = load <4 x float>* %x.i4.i                ; <<4 x float>> [#uses=1]
  %208 = call <4 x float> @_Z10native_sinU8__vector4f(<4 x float> %207) nounwind ; <<4 x float>> [#uses=1]
  store <4 x float> %208, <4 x float>* %38
  %209 = load <4 x float>* %38                    ; <<4 x float>> [#uses=1]
  store <4 x float> %209, <4 x float>* %s2.i
  %210 = load <4 x float>* %c2.i                  ; <<4 x float>> [#uses=1]
  %211 = load <4 x float>* %zr2.i                 ; <<4 x float>> [#uses=1]
  %212 = fmul <4 x float> %210, %211              ; <<4 x float>> [#uses=1]
  %213 = load <4 x float>* %s2.i                  ; <<4 x float>> [#uses=1]
  %214 = load <4 x float>* %zi2.i                 ; <<4 x float>> [#uses=1]
  %215 = fmul <4 x float> %213, %214              ; <<4 x float>> [#uses=1]
  %216 = fsub <4 x float> %212, %215              ; <<4 x float>> [#uses=1]
  store <4 x float> %216, <4 x float>* %__r1.i
  %217 = load <4 x float>* %c2.i                  ; <<4 x float>> [#uses=1]
  %218 = load <4 x float>* %zi2.i                 ; <<4 x float>> [#uses=1]
  %219 = fmul <4 x float> %217, %218              ; <<4 x float>> [#uses=1]
  %220 = load <4 x float>* %s2.i                  ; <<4 x float>> [#uses=1]
  %221 = load <4 x float>* %zr2.i                 ; <<4 x float>> [#uses=1]
  %222 = fmul <4 x float> %220, %221              ; <<4 x float>> [#uses=1]
  %223 = fadd <4 x float> %219, %222              ; <<4 x float>> [#uses=1]
  store <4 x float> %223, <4 x float>* %zi2.i
  %224 = load <4 x float>* %__r1.i                ; <<4 x float>> [#uses=1]
  store <4 x float> %224, <4 x float>* %zr2.i
  %225 = load <4 x i32>* %tbase.i                 ; <<4 x i32>> [#uses=1]
  %226 = mul <4 x i32> %225, <i32 3, i32 3, i32 3, i32 3> ; <<4 x i32>> [#uses=1]
  store <4 x i32> %226, <4 x i32>* %42
  store <4 x float>* %c3.i, <4 x float>** %43
  %227 = load <4 x i32>* %42                      ; <<4 x i32>> [#uses=1]
  %228 = icmp sgt <4 x i32> %227, <i32 512, i32 512, i32 512, i32 512> ; <<4 x i1>> [#uses=1]
  %229 = sext <4 x i1> %228 to <4 x i32>          ; <<4 x i32>> [#uses=1]
  %230 = and <4 x i32> %229, <i32 1024, i32 1024, i32 1024, i32 1024> ; <<4 x i32>> [#uses=1]
  %231 = load <4 x i32>* %42                      ; <<4 x i32>> [#uses=1]
  %232 = sub <4 x i32> %231, %230                 ; <<4 x i32>> [#uses=1]
  store <4 x i32> %232, <4 x i32>* %42
  %233 = load <4 x i32>* %42                      ; <<4 x i32>> [#uses=1]
  %234 = call <4 x float> @_Z14convert_float4U8__vector4i(<4 x i32> %233) nounwind ; <<4 x float>> [#uses=1]
  %235 = fmul <4 x float> %234, <float 0xBF7921FB60000000, float 0xBF7921FB60000000, float 0xBF7921FB60000000, float 0xBF7921FB60000000> ; <<4 x float>> [#uses=1]
  store <4 x float> %235, <4 x float>* %x.i3.i
  %236 = load <4 x float>* %x.i3.i                ; <<4 x float>> [#uses=1]
  %237 = call <4 x float> @_Z10native_cosU8__vector4f(<4 x float> %236) nounwind ; <<4 x float>> [#uses=1]
  %238 = load <4 x float>** %43                   ; <<4 x float>*> [#uses=1]
  store <4 x float> %237, <4 x float>* %238
  %239 = load <4 x float>* %x.i3.i                ; <<4 x float>> [#uses=1]
  %240 = call <4 x float> @_Z10native_sinU8__vector4f(<4 x float> %239) nounwind ; <<4 x float>> [#uses=1]
  store <4 x float> %240, <4 x float>* %41
  %241 = load <4 x float>* %41                    ; <<4 x float>> [#uses=1]
  store <4 x float> %241, <4 x float>* %s3.i
  %242 = load <4 x float>* %c3.i                  ; <<4 x float>> [#uses=1]
  %243 = load <4 x float>* %zr3.i                 ; <<4 x float>> [#uses=1]
  %244 = fmul <4 x float> %242, %243              ; <<4 x float>> [#uses=1]
  %245 = load <4 x float>* %s3.i                  ; <<4 x float>> [#uses=1]
  %246 = load <4 x float>* %zi3.i                 ; <<4 x float>> [#uses=1]
  %247 = fmul <4 x float> %245, %246              ; <<4 x float>> [#uses=1]
  %248 = fsub <4 x float> %244, %247              ; <<4 x float>> [#uses=1]
  store <4 x float> %248, <4 x float>* %__r2.i
  %249 = load <4 x float>* %c3.i                  ; <<4 x float>> [#uses=1]
  %250 = load <4 x float>* %zi3.i                 ; <<4 x float>> [#uses=1]
  %251 = fmul <4 x float> %249, %250              ; <<4 x float>> [#uses=1]
  %252 = load <4 x float>* %s3.i                  ; <<4 x float>> [#uses=1]
  %253 = load <4 x float>* %zr3.i                 ; <<4 x float>> [#uses=1]
  %254 = fmul <4 x float> %252, %253              ; <<4 x float>> [#uses=1]
  %255 = fadd <4 x float> %251, %254              ; <<4 x float>> [#uses=1]
  store <4 x float> %255, <4 x float>* %zi3.i
  %256 = load <4 x float>* %__r2.i                ; <<4 x float>> [#uses=1]
  store <4 x float> %256, <4 x float>* %zr3.i
  %257 = load float addrspace(3)** %50            ; <float addrspace(3)*> [#uses=1]
  %258 = load i32* %47                            ; <i32> [#uses=1]
  %259 = shl i32 %258, 2                          ; <i32> [#uses=1]
  %260 = load i32* %47                            ; <i32> [#uses=1]
  %261 = lshr i32 %260, 3                         ; <i32> [#uses=1]
  %262 = add i32 %259, %261                       ; <i32> [#uses=1]
  %263 = getelementptr inbounds float addrspace(3)* %257, i32 %262 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %263, float addrspace(3)** %lp.i
  %264 = load <4 x float>* %zr0.i                 ; <<4 x float>> [#uses=1]
  %265 = extractelement <4 x float> %264, i32 0   ; <float> [#uses=1]
  %266 = load float addrspace(3)** %lp.i          ; <float addrspace(3)*> [#uses=1]
  %267 = getelementptr inbounds float addrspace(3)* %266, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %265, float addrspace(3)* %267
  %268 = load <4 x float>* %zr0.i                 ; <<4 x float>> [#uses=1]
  %269 = extractelement <4 x float> %268, i32 1   ; <float> [#uses=1]
  %270 = load float addrspace(3)** %lp.i          ; <float addrspace(3)*> [#uses=1]
  %271 = getelementptr inbounds float addrspace(3)* %270, i32 1 ; <float addrspace(3)*> [#uses=1]
  store float %269, float addrspace(3)* %271
  %272 = load <4 x float>* %zr0.i                 ; <<4 x float>> [#uses=1]
  %273 = extractelement <4 x float> %272, i32 2   ; <float> [#uses=1]
  %274 = load float addrspace(3)** %lp.i          ; <float addrspace(3)*> [#uses=1]
  %275 = getelementptr inbounds float addrspace(3)* %274, i32 2 ; <float addrspace(3)*> [#uses=1]
  store float %273, float addrspace(3)* %275
  %276 = load <4 x float>* %zr0.i                 ; <<4 x float>> [#uses=1]
  %277 = extractelement <4 x float> %276, i32 3   ; <float> [#uses=1]
  %278 = load float addrspace(3)** %lp.i          ; <float addrspace(3)*> [#uses=1]
  %279 = getelementptr inbounds float addrspace(3)* %278, i32 3 ; <float addrspace(3)*> [#uses=1]
  store float %277, float addrspace(3)* %279
  %280 = load float addrspace(3)** %lp.i          ; <float addrspace(3)*> [#uses=1]
  %281 = getelementptr inbounds float addrspace(3)* %280, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %281, float addrspace(3)** %lp.i
  %282 = load <4 x float>* %zr1.i                 ; <<4 x float>> [#uses=1]
  %283 = extractelement <4 x float> %282, i32 0   ; <float> [#uses=1]
  %284 = load float addrspace(3)** %lp.i          ; <float addrspace(3)*> [#uses=1]
  %285 = getelementptr inbounds float addrspace(3)* %284, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %283, float addrspace(3)* %285
  %286 = load <4 x float>* %zr1.i                 ; <<4 x float>> [#uses=1]
  %287 = extractelement <4 x float> %286, i32 1   ; <float> [#uses=1]
  %288 = load float addrspace(3)** %lp.i          ; <float addrspace(3)*> [#uses=1]
  %289 = getelementptr inbounds float addrspace(3)* %288, i32 1 ; <float addrspace(3)*> [#uses=1]
  store float %287, float addrspace(3)* %289
  %290 = load <4 x float>* %zr1.i                 ; <<4 x float>> [#uses=1]
  %291 = extractelement <4 x float> %290, i32 2   ; <float> [#uses=1]
  %292 = load float addrspace(3)** %lp.i          ; <float addrspace(3)*> [#uses=1]
  %293 = getelementptr inbounds float addrspace(3)* %292, i32 2 ; <float addrspace(3)*> [#uses=1]
  store float %291, float addrspace(3)* %293
  %294 = load <4 x float>* %zr1.i                 ; <<4 x float>> [#uses=1]
  %295 = extractelement <4 x float> %294, i32 3   ; <float> [#uses=1]
  %296 = load float addrspace(3)** %lp.i          ; <float addrspace(3)*> [#uses=1]
  %297 = getelementptr inbounds float addrspace(3)* %296, i32 3 ; <float addrspace(3)*> [#uses=1]
  store float %295, float addrspace(3)* %297
  %298 = load float addrspace(3)** %lp.i          ; <float addrspace(3)*> [#uses=1]
  %299 = getelementptr inbounds float addrspace(3)* %298, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %299, float addrspace(3)** %lp.i
  %300 = load <4 x float>* %zr2.i                 ; <<4 x float>> [#uses=1]
  %301 = extractelement <4 x float> %300, i32 0   ; <float> [#uses=1]
  %302 = load float addrspace(3)** %lp.i          ; <float addrspace(3)*> [#uses=1]
  %303 = getelementptr inbounds float addrspace(3)* %302, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %301, float addrspace(3)* %303
  %304 = load <4 x float>* %zr2.i                 ; <<4 x float>> [#uses=1]
  %305 = extractelement <4 x float> %304, i32 1   ; <float> [#uses=1]
  %306 = load float addrspace(3)** %lp.i          ; <float addrspace(3)*> [#uses=1]
  %307 = getelementptr inbounds float addrspace(3)* %306, i32 1 ; <float addrspace(3)*> [#uses=1]
  store float %305, float addrspace(3)* %307
  %308 = load <4 x float>* %zr2.i                 ; <<4 x float>> [#uses=1]
  %309 = extractelement <4 x float> %308, i32 2   ; <float> [#uses=1]
  %310 = load float addrspace(3)** %lp.i          ; <float addrspace(3)*> [#uses=1]
  %311 = getelementptr inbounds float addrspace(3)* %310, i32 2 ; <float addrspace(3)*> [#uses=1]
  store float %309, float addrspace(3)* %311
  %312 = load <4 x float>* %zr2.i                 ; <<4 x float>> [#uses=1]
  %313 = extractelement <4 x float> %312, i32 3   ; <float> [#uses=1]
  %314 = load float addrspace(3)** %lp.i          ; <float addrspace(3)*> [#uses=1]
  %315 = getelementptr inbounds float addrspace(3)* %314, i32 3 ; <float addrspace(3)*> [#uses=1]
  store float %313, float addrspace(3)* %315
  %316 = load float addrspace(3)** %lp.i          ; <float addrspace(3)*> [#uses=1]
  %317 = getelementptr inbounds float addrspace(3)* %316, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %317, float addrspace(3)** %lp.i
  %318 = load <4 x float>* %zr3.i                 ; <<4 x float>> [#uses=1]
  %319 = extractelement <4 x float> %318, i32 0   ; <float> [#uses=1]
  %320 = load float addrspace(3)** %lp.i          ; <float addrspace(3)*> [#uses=1]
  %321 = getelementptr inbounds float addrspace(3)* %320, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %319, float addrspace(3)* %321
  %322 = load <4 x float>* %zr3.i                 ; <<4 x float>> [#uses=1]
  %323 = extractelement <4 x float> %322, i32 1   ; <float> [#uses=1]
  %324 = load float addrspace(3)** %lp.i          ; <float addrspace(3)*> [#uses=1]
  %325 = getelementptr inbounds float addrspace(3)* %324, i32 1 ; <float addrspace(3)*> [#uses=1]
  store float %323, float addrspace(3)* %325
  %326 = load <4 x float>* %zr3.i                 ; <<4 x float>> [#uses=1]
  %327 = extractelement <4 x float> %326, i32 2   ; <float> [#uses=1]
  %328 = load float addrspace(3)** %lp.i          ; <float addrspace(3)*> [#uses=1]
  %329 = getelementptr inbounds float addrspace(3)* %328, i32 2 ; <float addrspace(3)*> [#uses=1]
  store float %327, float addrspace(3)* %329
  %330 = load <4 x float>* %zr3.i                 ; <<4 x float>> [#uses=1]
  %331 = extractelement <4 x float> %330, i32 3   ; <float> [#uses=1]
  %332 = load float addrspace(3)** %lp.i          ; <float addrspace(3)*> [#uses=1]
  %333 = getelementptr inbounds float addrspace(3)* %332, i32 3 ; <float addrspace(3)*> [#uses=1]
  store float %331, float addrspace(3)* %333
  %334 = load float addrspace(3)** %lp.i          ; <float addrspace(3)*> [#uses=1]
  %335 = getelementptr inbounds float addrspace(3)* %334, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %335, float addrspace(3)** %lp.i
  %336 = load <4 x float>* %zi0.i                 ; <<4 x float>> [#uses=1]
  %337 = extractelement <4 x float> %336, i32 0   ; <float> [#uses=1]
  %338 = load float addrspace(3)** %lp.i          ; <float addrspace(3)*> [#uses=1]
  %339 = getelementptr inbounds float addrspace(3)* %338, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %337, float addrspace(3)* %339
  %340 = load <4 x float>* %zi0.i                 ; <<4 x float>> [#uses=1]
  %341 = extractelement <4 x float> %340, i32 1   ; <float> [#uses=1]
  %342 = load float addrspace(3)** %lp.i          ; <float addrspace(3)*> [#uses=1]
  %343 = getelementptr inbounds float addrspace(3)* %342, i32 1 ; <float addrspace(3)*> [#uses=1]
  store float %341, float addrspace(3)* %343
  %344 = load <4 x float>* %zi0.i                 ; <<4 x float>> [#uses=1]
  %345 = extractelement <4 x float> %344, i32 2   ; <float> [#uses=1]
  %346 = load float addrspace(3)** %lp.i          ; <float addrspace(3)*> [#uses=1]
  %347 = getelementptr inbounds float addrspace(3)* %346, i32 2 ; <float addrspace(3)*> [#uses=1]
  store float %345, float addrspace(3)* %347
  %348 = load <4 x float>* %zi0.i                 ; <<4 x float>> [#uses=1]
  %349 = extractelement <4 x float> %348, i32 3   ; <float> [#uses=1]
  %350 = load float addrspace(3)** %lp.i          ; <float addrspace(3)*> [#uses=1]
  %351 = getelementptr inbounds float addrspace(3)* %350, i32 3 ; <float addrspace(3)*> [#uses=1]
  store float %349, float addrspace(3)* %351
  %352 = load float addrspace(3)** %lp.i          ; <float addrspace(3)*> [#uses=1]
  %353 = getelementptr inbounds float addrspace(3)* %352, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %353, float addrspace(3)** %lp.i
  %354 = load <4 x float>* %zi1.i                 ; <<4 x float>> [#uses=1]
  %355 = extractelement <4 x float> %354, i32 0   ; <float> [#uses=1]
  %356 = load float addrspace(3)** %lp.i          ; <float addrspace(3)*> [#uses=1]
  %357 = getelementptr inbounds float addrspace(3)* %356, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %355, float addrspace(3)* %357
  %358 = load <4 x float>* %zi1.i                 ; <<4 x float>> [#uses=1]
  %359 = extractelement <4 x float> %358, i32 1   ; <float> [#uses=1]
  %360 = load float addrspace(3)** %lp.i          ; <float addrspace(3)*> [#uses=1]
  %361 = getelementptr inbounds float addrspace(3)* %360, i32 1 ; <float addrspace(3)*> [#uses=1]
  store float %359, float addrspace(3)* %361
  %362 = load <4 x float>* %zi1.i                 ; <<4 x float>> [#uses=1]
  %363 = extractelement <4 x float> %362, i32 2   ; <float> [#uses=1]
  %364 = load float addrspace(3)** %lp.i          ; <float addrspace(3)*> [#uses=1]
  %365 = getelementptr inbounds float addrspace(3)* %364, i32 2 ; <float addrspace(3)*> [#uses=1]
  store float %363, float addrspace(3)* %365
  %366 = load <4 x float>* %zi1.i                 ; <<4 x float>> [#uses=1]
  %367 = extractelement <4 x float> %366, i32 3   ; <float> [#uses=1]
  %368 = load float addrspace(3)** %lp.i          ; <float addrspace(3)*> [#uses=1]
  %369 = getelementptr inbounds float addrspace(3)* %368, i32 3 ; <float addrspace(3)*> [#uses=1]
  store float %367, float addrspace(3)* %369
  %370 = load float addrspace(3)** %lp.i          ; <float addrspace(3)*> [#uses=1]
  %371 = getelementptr inbounds float addrspace(3)* %370, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %371, float addrspace(3)** %lp.i
  %372 = load <4 x float>* %zi2.i                 ; <<4 x float>> [#uses=1]
  %373 = extractelement <4 x float> %372, i32 0   ; <float> [#uses=1]
  %374 = load float addrspace(3)** %lp.i          ; <float addrspace(3)*> [#uses=1]
  %375 = getelementptr inbounds float addrspace(3)* %374, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %373, float addrspace(3)* %375
  %376 = load <4 x float>* %zi2.i                 ; <<4 x float>> [#uses=1]
  %377 = extractelement <4 x float> %376, i32 1   ; <float> [#uses=1]
  %378 = load float addrspace(3)** %lp.i          ; <float addrspace(3)*> [#uses=1]
  %379 = getelementptr inbounds float addrspace(3)* %378, i32 1 ; <float addrspace(3)*> [#uses=1]
  store float %377, float addrspace(3)* %379
  %380 = load <4 x float>* %zi2.i                 ; <<4 x float>> [#uses=1]
  %381 = extractelement <4 x float> %380, i32 2   ; <float> [#uses=1]
  %382 = load float addrspace(3)** %lp.i          ; <float addrspace(3)*> [#uses=1]
  %383 = getelementptr inbounds float addrspace(3)* %382, i32 2 ; <float addrspace(3)*> [#uses=1]
  store float %381, float addrspace(3)* %383
  %384 = load <4 x float>* %zi2.i                 ; <<4 x float>> [#uses=1]
  %385 = extractelement <4 x float> %384, i32 3   ; <float> [#uses=1]
  %386 = load float addrspace(3)** %lp.i          ; <float addrspace(3)*> [#uses=1]
  %387 = getelementptr inbounds float addrspace(3)* %386, i32 3 ; <float addrspace(3)*> [#uses=1]
  store float %385, float addrspace(3)* %387
  %388 = load float addrspace(3)** %lp.i          ; <float addrspace(3)*> [#uses=1]
  %389 = getelementptr inbounds float addrspace(3)* %388, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %389, float addrspace(3)** %lp.i
  %390 = load <4 x float>* %zi3.i                 ; <<4 x float>> [#uses=1]
  %391 = extractelement <4 x float> %390, i32 0   ; <float> [#uses=1]
  %392 = load float addrspace(3)** %lp.i          ; <float addrspace(3)*> [#uses=1]
  %393 = getelementptr inbounds float addrspace(3)* %392, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %391, float addrspace(3)* %393
  %394 = load <4 x float>* %zi3.i                 ; <<4 x float>> [#uses=1]
  %395 = extractelement <4 x float> %394, i32 1   ; <float> [#uses=1]
  %396 = load float addrspace(3)** %lp.i          ; <float addrspace(3)*> [#uses=1]
  %397 = getelementptr inbounds float addrspace(3)* %396, i32 1 ; <float addrspace(3)*> [#uses=1]
  store float %395, float addrspace(3)* %397
  %398 = load <4 x float>* %zi3.i                 ; <<4 x float>> [#uses=1]
  %399 = extractelement <4 x float> %398, i32 2   ; <float> [#uses=1]
  %400 = load float addrspace(3)** %lp.i          ; <float addrspace(3)*> [#uses=1]
  %401 = getelementptr inbounds float addrspace(3)* %400, i32 2 ; <float addrspace(3)*> [#uses=1]
  store float %399, float addrspace(3)* %401
  %402 = load <4 x float>* %zi3.i                 ; <<4 x float>> [#uses=1]
  %403 = extractelement <4 x float> %402, i32 3   ; <float> [#uses=1]
  %404 = load float addrspace(3)** %lp.i          ; <float addrspace(3)*> [#uses=1]
  %405 = getelementptr inbounds float addrspace(3)* %404, i32 3 ; <float addrspace(3)*> [#uses=1]
  store float %403, float addrspace(3)* %405
  call void @barrier(i32 1) nounwind
  %406 = load i32* %me                            ; <i32> [#uses=1]
  store i32 %406, i32* %10
  store float addrspace(3)* getelementptr inbounds ([2176 x float] addrspace(3)* @opencl_kfft_local_lds, i32 0, i32 0), float addrspace(3)** %11
  %407 = load float addrspace(3)** %11            ; <float addrspace(3)*> [#uses=1]
  %408 = load i32* %10                            ; <i32> [#uses=1]
  %409 = load i32* %10                            ; <i32> [#uses=1]
  %410 = lshr i32 %409, 5                         ; <i32> [#uses=1]
  %411 = add i32 %408, %410                       ; <i32> [#uses=1]
  %412 = getelementptr inbounds float addrspace(3)* %407, i32 %411 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %412, float addrspace(3)** %lp.i95
  %413 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %414 = getelementptr inbounds float addrspace(3)* %413, i32 0 ; <float addrspace(3)*> [#uses=1]
  %415 = load float addrspace(3)* %414            ; <float> [#uses=1]
  %416 = load <4 x float>* %zr0.i96               ; <<4 x float>> [#uses=1]
  %417 = insertelement <4 x float> %416, float %415, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %417, <4 x float>* %zr0.i96
  %418 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %419 = getelementptr inbounds float addrspace(3)* %418, i32 66 ; <float addrspace(3)*> [#uses=1]
  %420 = load float addrspace(3)* %419            ; <float> [#uses=1]
  %421 = load <4 x float>* %zr1.i97               ; <<4 x float>> [#uses=1]
  %422 = insertelement <4 x float> %421, float %420, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %422, <4 x float>* %zr1.i97
  %423 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %424 = getelementptr inbounds float addrspace(3)* %423, i32 132 ; <float addrspace(3)*> [#uses=1]
  %425 = load float addrspace(3)* %424            ; <float> [#uses=1]
  %426 = load <4 x float>* %zr2.i98               ; <<4 x float>> [#uses=1]
  %427 = insertelement <4 x float> %426, float %425, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %427, <4 x float>* %zr2.i98
  %428 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %429 = getelementptr inbounds float addrspace(3)* %428, i32 198 ; <float addrspace(3)*> [#uses=1]
  %430 = load float addrspace(3)* %429            ; <float> [#uses=1]
  %431 = load <4 x float>* %zr3.i99               ; <<4 x float>> [#uses=1]
  %432 = insertelement <4 x float> %431, float %430, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %432, <4 x float>* %zr3.i99
  %433 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %434 = getelementptr inbounds float addrspace(3)* %433, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %434, float addrspace(3)** %lp.i95
  %435 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %436 = getelementptr inbounds float addrspace(3)* %435, i32 0 ; <float addrspace(3)*> [#uses=1]
  %437 = load float addrspace(3)* %436            ; <float> [#uses=1]
  %438 = load <4 x float>* %zr0.i96               ; <<4 x float>> [#uses=1]
  %439 = insertelement <4 x float> %438, float %437, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %439, <4 x float>* %zr0.i96
  %440 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %441 = getelementptr inbounds float addrspace(3)* %440, i32 66 ; <float addrspace(3)*> [#uses=1]
  %442 = load float addrspace(3)* %441            ; <float> [#uses=1]
  %443 = load <4 x float>* %zr1.i97               ; <<4 x float>> [#uses=1]
  %444 = insertelement <4 x float> %443, float %442, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %444, <4 x float>* %zr1.i97
  %445 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %446 = getelementptr inbounds float addrspace(3)* %445, i32 132 ; <float addrspace(3)*> [#uses=1]
  %447 = load float addrspace(3)* %446            ; <float> [#uses=1]
  %448 = load <4 x float>* %zr2.i98               ; <<4 x float>> [#uses=1]
  %449 = insertelement <4 x float> %448, float %447, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %449, <4 x float>* %zr2.i98
  %450 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %451 = getelementptr inbounds float addrspace(3)* %450, i32 198 ; <float addrspace(3)*> [#uses=1]
  %452 = load float addrspace(3)* %451            ; <float> [#uses=1]
  %453 = load <4 x float>* %zr3.i99               ; <<4 x float>> [#uses=1]
  %454 = insertelement <4 x float> %453, float %452, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %454, <4 x float>* %zr3.i99
  %455 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %456 = getelementptr inbounds float addrspace(3)* %455, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %456, float addrspace(3)** %lp.i95
  %457 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %458 = getelementptr inbounds float addrspace(3)* %457, i32 0 ; <float addrspace(3)*> [#uses=1]
  %459 = load float addrspace(3)* %458            ; <float> [#uses=1]
  %460 = load <4 x float>* %zr0.i96               ; <<4 x float>> [#uses=1]
  %461 = insertelement <4 x float> %460, float %459, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %461, <4 x float>* %zr0.i96
  %462 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %463 = getelementptr inbounds float addrspace(3)* %462, i32 66 ; <float addrspace(3)*> [#uses=1]
  %464 = load float addrspace(3)* %463            ; <float> [#uses=1]
  %465 = load <4 x float>* %zr1.i97               ; <<4 x float>> [#uses=1]
  %466 = insertelement <4 x float> %465, float %464, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %466, <4 x float>* %zr1.i97
  %467 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %468 = getelementptr inbounds float addrspace(3)* %467, i32 132 ; <float addrspace(3)*> [#uses=1]
  %469 = load float addrspace(3)* %468            ; <float> [#uses=1]
  %470 = load <4 x float>* %zr2.i98               ; <<4 x float>> [#uses=1]
  %471 = insertelement <4 x float> %470, float %469, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %471, <4 x float>* %zr2.i98
  %472 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %473 = getelementptr inbounds float addrspace(3)* %472, i32 198 ; <float addrspace(3)*> [#uses=1]
  %474 = load float addrspace(3)* %473            ; <float> [#uses=1]
  %475 = load <4 x float>* %zr3.i99               ; <<4 x float>> [#uses=1]
  %476 = insertelement <4 x float> %475, float %474, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %476, <4 x float>* %zr3.i99
  %477 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %478 = getelementptr inbounds float addrspace(3)* %477, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %478, float addrspace(3)** %lp.i95
  %479 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %480 = getelementptr inbounds float addrspace(3)* %479, i32 0 ; <float addrspace(3)*> [#uses=1]
  %481 = load float addrspace(3)* %480            ; <float> [#uses=1]
  %482 = load <4 x float>* %zr0.i96               ; <<4 x float>> [#uses=1]
  %483 = insertelement <4 x float> %482, float %481, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %483, <4 x float>* %zr0.i96
  %484 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %485 = getelementptr inbounds float addrspace(3)* %484, i32 66 ; <float addrspace(3)*> [#uses=1]
  %486 = load float addrspace(3)* %485            ; <float> [#uses=1]
  %487 = load <4 x float>* %zr1.i97               ; <<4 x float>> [#uses=1]
  %488 = insertelement <4 x float> %487, float %486, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %488, <4 x float>* %zr1.i97
  %489 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %490 = getelementptr inbounds float addrspace(3)* %489, i32 132 ; <float addrspace(3)*> [#uses=1]
  %491 = load float addrspace(3)* %490            ; <float> [#uses=1]
  %492 = load <4 x float>* %zr2.i98               ; <<4 x float>> [#uses=1]
  %493 = insertelement <4 x float> %492, float %491, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %493, <4 x float>* %zr2.i98
  %494 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %495 = getelementptr inbounds float addrspace(3)* %494, i32 198 ; <float addrspace(3)*> [#uses=1]
  %496 = load float addrspace(3)* %495            ; <float> [#uses=1]
  %497 = load <4 x float>* %zr3.i99               ; <<4 x float>> [#uses=1]
  %498 = insertelement <4 x float> %497, float %496, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %498, <4 x float>* %zr3.i99
  %499 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %500 = getelementptr inbounds float addrspace(3)* %499, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %500, float addrspace(3)** %lp.i95
  %501 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %502 = getelementptr inbounds float addrspace(3)* %501, i32 0 ; <float addrspace(3)*> [#uses=1]
  %503 = load float addrspace(3)* %502            ; <float> [#uses=1]
  %504 = load <4 x float>* %zi0.i100              ; <<4 x float>> [#uses=1]
  %505 = insertelement <4 x float> %504, float %503, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %505, <4 x float>* %zi0.i100
  %506 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %507 = getelementptr inbounds float addrspace(3)* %506, i32 66 ; <float addrspace(3)*> [#uses=1]
  %508 = load float addrspace(3)* %507            ; <float> [#uses=1]
  %509 = load <4 x float>* %zi1.i101              ; <<4 x float>> [#uses=1]
  %510 = insertelement <4 x float> %509, float %508, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %510, <4 x float>* %zi1.i101
  %511 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %512 = getelementptr inbounds float addrspace(3)* %511, i32 132 ; <float addrspace(3)*> [#uses=1]
  %513 = load float addrspace(3)* %512            ; <float> [#uses=1]
  %514 = load <4 x float>* %zi2.i102              ; <<4 x float>> [#uses=1]
  %515 = insertelement <4 x float> %514, float %513, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %515, <4 x float>* %zi2.i102
  %516 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %517 = getelementptr inbounds float addrspace(3)* %516, i32 198 ; <float addrspace(3)*> [#uses=1]
  %518 = load float addrspace(3)* %517            ; <float> [#uses=1]
  %519 = load <4 x float>* %zi3.i103              ; <<4 x float>> [#uses=1]
  %520 = insertelement <4 x float> %519, float %518, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %520, <4 x float>* %zi3.i103
  %521 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %522 = getelementptr inbounds float addrspace(3)* %521, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %522, float addrspace(3)** %lp.i95
  %523 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %524 = getelementptr inbounds float addrspace(3)* %523, i32 0 ; <float addrspace(3)*> [#uses=1]
  %525 = load float addrspace(3)* %524            ; <float> [#uses=1]
  %526 = load <4 x float>* %zi0.i100              ; <<4 x float>> [#uses=1]
  %527 = insertelement <4 x float> %526, float %525, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %527, <4 x float>* %zi0.i100
  %528 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %529 = getelementptr inbounds float addrspace(3)* %528, i32 66 ; <float addrspace(3)*> [#uses=1]
  %530 = load float addrspace(3)* %529            ; <float> [#uses=1]
  %531 = load <4 x float>* %zi1.i101              ; <<4 x float>> [#uses=1]
  %532 = insertelement <4 x float> %531, float %530, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %532, <4 x float>* %zi1.i101
  %533 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %534 = getelementptr inbounds float addrspace(3)* %533, i32 132 ; <float addrspace(3)*> [#uses=1]
  %535 = load float addrspace(3)* %534            ; <float> [#uses=1]
  %536 = load <4 x float>* %zi2.i102              ; <<4 x float>> [#uses=1]
  %537 = insertelement <4 x float> %536, float %535, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %537, <4 x float>* %zi2.i102
  %538 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %539 = getelementptr inbounds float addrspace(3)* %538, i32 198 ; <float addrspace(3)*> [#uses=1]
  %540 = load float addrspace(3)* %539            ; <float> [#uses=1]
  %541 = load <4 x float>* %zi3.i103              ; <<4 x float>> [#uses=1]
  %542 = insertelement <4 x float> %541, float %540, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %542, <4 x float>* %zi3.i103
  %543 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %544 = getelementptr inbounds float addrspace(3)* %543, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %544, float addrspace(3)** %lp.i95
  %545 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %546 = getelementptr inbounds float addrspace(3)* %545, i32 0 ; <float addrspace(3)*> [#uses=1]
  %547 = load float addrspace(3)* %546            ; <float> [#uses=1]
  %548 = load <4 x float>* %zi0.i100              ; <<4 x float>> [#uses=1]
  %549 = insertelement <4 x float> %548, float %547, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %549, <4 x float>* %zi0.i100
  %550 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %551 = getelementptr inbounds float addrspace(3)* %550, i32 66 ; <float addrspace(3)*> [#uses=1]
  %552 = load float addrspace(3)* %551            ; <float> [#uses=1]
  %553 = load <4 x float>* %zi1.i101              ; <<4 x float>> [#uses=1]
  %554 = insertelement <4 x float> %553, float %552, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %554, <4 x float>* %zi1.i101
  %555 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %556 = getelementptr inbounds float addrspace(3)* %555, i32 132 ; <float addrspace(3)*> [#uses=1]
  %557 = load float addrspace(3)* %556            ; <float> [#uses=1]
  %558 = load <4 x float>* %zi2.i102              ; <<4 x float>> [#uses=1]
  %559 = insertelement <4 x float> %558, float %557, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %559, <4 x float>* %zi2.i102
  %560 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %561 = getelementptr inbounds float addrspace(3)* %560, i32 198 ; <float addrspace(3)*> [#uses=1]
  %562 = load float addrspace(3)* %561            ; <float> [#uses=1]
  %563 = load <4 x float>* %zi3.i103              ; <<4 x float>> [#uses=1]
  %564 = insertelement <4 x float> %563, float %562, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %564, <4 x float>* %zi3.i103
  %565 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %566 = getelementptr inbounds float addrspace(3)* %565, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %566, float addrspace(3)** %lp.i95
  %567 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %568 = getelementptr inbounds float addrspace(3)* %567, i32 0 ; <float addrspace(3)*> [#uses=1]
  %569 = load float addrspace(3)* %568            ; <float> [#uses=1]
  %570 = load <4 x float>* %zi0.i100              ; <<4 x float>> [#uses=1]
  %571 = insertelement <4 x float> %570, float %569, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %571, <4 x float>* %zi0.i100
  %572 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %573 = getelementptr inbounds float addrspace(3)* %572, i32 66 ; <float addrspace(3)*> [#uses=1]
  %574 = load float addrspace(3)* %573            ; <float> [#uses=1]
  %575 = load <4 x float>* %zi1.i101              ; <<4 x float>> [#uses=1]
  %576 = insertelement <4 x float> %575, float %574, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %576, <4 x float>* %zi1.i101
  %577 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %578 = getelementptr inbounds float addrspace(3)* %577, i32 132 ; <float addrspace(3)*> [#uses=1]
  %579 = load float addrspace(3)* %578            ; <float> [#uses=1]
  %580 = load <4 x float>* %zi2.i102              ; <<4 x float>> [#uses=1]
  %581 = insertelement <4 x float> %580, float %579, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %581, <4 x float>* %zi2.i102
  %582 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %583 = getelementptr inbounds float addrspace(3)* %582, i32 198 ; <float addrspace(3)*> [#uses=1]
  %584 = load float addrspace(3)* %583            ; <float> [#uses=1]
  %585 = load <4 x float>* %zi3.i103              ; <<4 x float>> [#uses=1]
  %586 = insertelement <4 x float> %585, float %584, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %586, <4 x float>* %zi3.i103
  %587 = load <4 x float>* %zr0.i96               ; <<4 x float>> [#uses=1]
  %588 = load <4 x float>* %zr2.i98               ; <<4 x float>> [#uses=1]
  %589 = fadd <4 x float> %587, %588              ; <<4 x float>> [#uses=1]
  store <4 x float> %589, <4 x float>* %ar0.i104
  %590 = load <4 x float>* %zr1.i97               ; <<4 x float>> [#uses=1]
  %591 = load <4 x float>* %zr3.i99               ; <<4 x float>> [#uses=1]
  %592 = fadd <4 x float> %590, %591              ; <<4 x float>> [#uses=1]
  store <4 x float> %592, <4 x float>* %ar2.i105
  %593 = load <4 x float>* %ar0.i104              ; <<4 x float>> [#uses=1]
  %594 = load <4 x float>* %ar2.i105              ; <<4 x float>> [#uses=1]
  %595 = fadd <4 x float> %593, %594              ; <<4 x float>> [#uses=1]
  store <4 x float> %595, <4 x float>* %br0.i106
  %596 = load <4 x float>* %zr0.i96               ; <<4 x float>> [#uses=1]
  %597 = load <4 x float>* %zr2.i98               ; <<4 x float>> [#uses=1]
  %598 = fsub <4 x float> %596, %597              ; <<4 x float>> [#uses=1]
  store <4 x float> %598, <4 x float>* %br1.i107
  %599 = load <4 x float>* %ar0.i104              ; <<4 x float>> [#uses=1]
  %600 = load <4 x float>* %ar2.i105              ; <<4 x float>> [#uses=1]
  %601 = fsub <4 x float> %599, %600              ; <<4 x float>> [#uses=1]
  store <4 x float> %601, <4 x float>* %br2.i108
  %602 = load <4 x float>* %zr1.i97               ; <<4 x float>> [#uses=1]
  %603 = load <4 x float>* %zr3.i99               ; <<4 x float>> [#uses=1]
  %604 = fsub <4 x float> %602, %603              ; <<4 x float>> [#uses=1]
  store <4 x float> %604, <4 x float>* %br3.i109
  %605 = load <4 x float>* %zi0.i100              ; <<4 x float>> [#uses=1]
  %606 = load <4 x float>* %zi2.i102              ; <<4 x float>> [#uses=1]
  %607 = fadd <4 x float> %605, %606              ; <<4 x float>> [#uses=1]
  store <4 x float> %607, <4 x float>* %ai0.i110
  %608 = load <4 x float>* %zi1.i101              ; <<4 x float>> [#uses=1]
  %609 = load <4 x float>* %zi3.i103              ; <<4 x float>> [#uses=1]
  %610 = fadd <4 x float> %608, %609              ; <<4 x float>> [#uses=1]
  store <4 x float> %610, <4 x float>* %ai2.i111
  %611 = load <4 x float>* %ai0.i110              ; <<4 x float>> [#uses=1]
  %612 = load <4 x float>* %ai2.i111              ; <<4 x float>> [#uses=1]
  %613 = fadd <4 x float> %611, %612              ; <<4 x float>> [#uses=1]
  store <4 x float> %613, <4 x float>* %bi0.i112
  %614 = load <4 x float>* %zi0.i100              ; <<4 x float>> [#uses=1]
  %615 = load <4 x float>* %zi2.i102              ; <<4 x float>> [#uses=1]
  %616 = fsub <4 x float> %614, %615              ; <<4 x float>> [#uses=1]
  store <4 x float> %616, <4 x float>* %bi1.i113
  %617 = load <4 x float>* %ai0.i110              ; <<4 x float>> [#uses=1]
  %618 = load <4 x float>* %ai2.i111              ; <<4 x float>> [#uses=1]
  %619 = fsub <4 x float> %617, %618              ; <<4 x float>> [#uses=1]
  store <4 x float> %619, <4 x float>* %bi2.i114
  %620 = load <4 x float>* %zi1.i101              ; <<4 x float>> [#uses=1]
  %621 = load <4 x float>* %zi3.i103              ; <<4 x float>> [#uses=1]
  %622 = fsub <4 x float> %620, %621              ; <<4 x float>> [#uses=1]
  store <4 x float> %622, <4 x float>* %bi3.i115
  %623 = load <4 x float>* %br0.i106              ; <<4 x float>> [#uses=1]
  store <4 x float> %623, <4 x float>* %zr0.i96
  %624 = load <4 x float>* %bi0.i112              ; <<4 x float>> [#uses=1]
  store <4 x float> %624, <4 x float>* %zi0.i100
  %625 = load <4 x float>* %br1.i107              ; <<4 x float>> [#uses=1]
  %626 = load <4 x float>* %bi3.i115              ; <<4 x float>> [#uses=1]
  %627 = fadd <4 x float> %625, %626              ; <<4 x float>> [#uses=1]
  store <4 x float> %627, <4 x float>* %zr1.i97
  %628 = load <4 x float>* %bi1.i113              ; <<4 x float>> [#uses=1]
  %629 = load <4 x float>* %br3.i109              ; <<4 x float>> [#uses=1]
  %630 = fsub <4 x float> %628, %629              ; <<4 x float>> [#uses=1]
  store <4 x float> %630, <4 x float>* %zi1.i101
  %631 = load <4 x float>* %br1.i107              ; <<4 x float>> [#uses=1]
  %632 = load <4 x float>* %bi3.i115              ; <<4 x float>> [#uses=1]
  %633 = fsub <4 x float> %631, %632              ; <<4 x float>> [#uses=1]
  store <4 x float> %633, <4 x float>* %zr3.i99
  %634 = load <4 x float>* %br3.i109              ; <<4 x float>> [#uses=1]
  %635 = load <4 x float>* %bi1.i113              ; <<4 x float>> [#uses=1]
  %636 = fadd <4 x float> %634, %635              ; <<4 x float>> [#uses=1]
  store <4 x float> %636, <4 x float>* %zi3.i103
  %637 = load <4 x float>* %br2.i108              ; <<4 x float>> [#uses=1]
  store <4 x float> %637, <4 x float>* %zr2.i98
  %638 = load <4 x float>* %bi2.i114              ; <<4 x float>> [#uses=1]
  store <4 x float> %638, <4 x float>* %zi2.i102
  %639 = load i32* %10                            ; <i32> [#uses=1]
  %640 = shl i32 %639, 2                          ; <i32> [#uses=1]
  store i32 %640, i32* %tbase.i116
  %641 = load i32* %tbase.i116                    ; <i32> [#uses=1]
  %642 = mul i32 %641, 1                          ; <i32> [#uses=1]
  store i32 %642, i32* %8
  store float* %c1.i117, float** %9
  %643 = load i32* %8                             ; <i32> [#uses=1]
  %644 = icmp sgt i32 %643, 512                   ; <i1> [#uses=1]
  br i1 %644, label %645, label %k_sincos.exit.i126

; <label>:645                                     ; preds = %0
  %646 = load i32* %8                             ; <i32> [#uses=1]
  %647 = sub i32 %646, 1024                       ; <i32> [#uses=1]
  store i32 %647, i32* %8
  br label %k_sincos.exit.i126

k_sincos.exit.i126:                               ; preds = %645, %0
  %648 = load i32* %8                             ; <i32> [#uses=1]
  %649 = sitofp i32 %648 to float                 ; <float> [#uses=1]
  %650 = fmul float %649, 0xBF7921FB60000000      ; <float> [#uses=1]
  store float %650, float* %x.i.i94
  %651 = load float* %x.i.i94                     ; <float> [#uses=1]
  %652 = call float @_Z10native_cosf(float %651) nounwind ; <float> [#uses=1]
  %653 = load float** %9                          ; <float*> [#uses=1]
  store float %652, float* %653
  %654 = load float* %x.i.i94                     ; <float> [#uses=1]
  %655 = call float @_Z10native_sinf(float %654) nounwind ; <float> [#uses=1]
  store float %655, float* %7
  %656 = load float* %7                           ; <float> [#uses=1]
  store float %656, float* %s1.i118
  %657 = load float* %c1.i117                     ; <float> [#uses=1]
  %658 = insertelement <4 x float> undef, float %657, i32 0 ; <<4 x float>> [#uses=2]
  %659 = shufflevector <4 x float> %658, <4 x float> %658, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %660 = load <4 x float>* %zr1.i97               ; <<4 x float>> [#uses=1]
  %661 = fmul <4 x float> %659, %660              ; <<4 x float>> [#uses=1]
  %662 = load float* %s1.i118                     ; <float> [#uses=1]
  %663 = insertelement <4 x float> undef, float %662, i32 0 ; <<4 x float>> [#uses=2]
  %664 = shufflevector <4 x float> %663, <4 x float> %663, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %665 = load <4 x float>* %zi1.i101              ; <<4 x float>> [#uses=1]
  %666 = fmul <4 x float> %664, %665              ; <<4 x float>> [#uses=1]
  %667 = fsub <4 x float> %661, %666              ; <<4 x float>> [#uses=1]
  store <4 x float> %667, <4 x float>* %__r.i119
  %668 = load float* %c1.i117                     ; <float> [#uses=1]
  %669 = insertelement <4 x float> undef, float %668, i32 0 ; <<4 x float>> [#uses=2]
  %670 = shufflevector <4 x float> %669, <4 x float> %669, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %671 = load <4 x float>* %zi1.i101              ; <<4 x float>> [#uses=1]
  %672 = fmul <4 x float> %670, %671              ; <<4 x float>> [#uses=1]
  %673 = load float* %s1.i118                     ; <float> [#uses=1]
  %674 = insertelement <4 x float> undef, float %673, i32 0 ; <<4 x float>> [#uses=2]
  %675 = shufflevector <4 x float> %674, <4 x float> %674, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %676 = load <4 x float>* %zr1.i97               ; <<4 x float>> [#uses=1]
  %677 = fmul <4 x float> %675, %676              ; <<4 x float>> [#uses=1]
  %678 = fadd <4 x float> %672, %677              ; <<4 x float>> [#uses=1]
  store <4 x float> %678, <4 x float>* %zi1.i101
  %679 = load <4 x float>* %__r.i119              ; <<4 x float>> [#uses=1]
  store <4 x float> %679, <4 x float>* %zr1.i97
  %680 = load i32* %tbase.i116                    ; <i32> [#uses=1]
  %681 = mul i32 %680, 2                          ; <i32> [#uses=1]
  store i32 %681, i32* %2
  store float* %c2.i120, float** %3
  %682 = load i32* %2                             ; <i32> [#uses=1]
  %683 = icmp sgt i32 %682, 512                   ; <i1> [#uses=1]
  br i1 %683, label %684, label %k_sincos.exit6.i127

; <label>:684                                     ; preds = %k_sincos.exit.i126
  %685 = load i32* %2                             ; <i32> [#uses=1]
  %686 = sub i32 %685, 1024                       ; <i32> [#uses=1]
  store i32 %686, i32* %2
  br label %k_sincos.exit6.i127

k_sincos.exit6.i127:                              ; preds = %684, %k_sincos.exit.i126
  %687 = load i32* %2                             ; <i32> [#uses=1]
  %688 = sitofp i32 %687 to float                 ; <float> [#uses=1]
  %689 = fmul float %688, 0xBF7921FB60000000      ; <float> [#uses=1]
  store float %689, float* %x.i5.i92
  %690 = load float* %x.i5.i92                    ; <float> [#uses=1]
  %691 = call float @_Z10native_cosf(float %690) nounwind ; <float> [#uses=1]
  %692 = load float** %3                          ; <float*> [#uses=1]
  store float %691, float* %692
  %693 = load float* %x.i5.i92                    ; <float> [#uses=1]
  %694 = call float @_Z10native_sinf(float %693) nounwind ; <float> [#uses=1]
  store float %694, float* %1
  %695 = load float* %1                           ; <float> [#uses=1]
  store float %695, float* %s2.i121
  %696 = load float* %c2.i120                     ; <float> [#uses=1]
  %697 = insertelement <4 x float> undef, float %696, i32 0 ; <<4 x float>> [#uses=2]
  %698 = shufflevector <4 x float> %697, <4 x float> %697, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %699 = load <4 x float>* %zr2.i98               ; <<4 x float>> [#uses=1]
  %700 = fmul <4 x float> %698, %699              ; <<4 x float>> [#uses=1]
  %701 = load float* %s2.i121                     ; <float> [#uses=1]
  %702 = insertelement <4 x float> undef, float %701, i32 0 ; <<4 x float>> [#uses=2]
  %703 = shufflevector <4 x float> %702, <4 x float> %702, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %704 = load <4 x float>* %zi2.i102              ; <<4 x float>> [#uses=1]
  %705 = fmul <4 x float> %703, %704              ; <<4 x float>> [#uses=1]
  %706 = fsub <4 x float> %700, %705              ; <<4 x float>> [#uses=1]
  store <4 x float> %706, <4 x float>* %__r1.i122
  %707 = load float* %c2.i120                     ; <float> [#uses=1]
  %708 = insertelement <4 x float> undef, float %707, i32 0 ; <<4 x float>> [#uses=2]
  %709 = shufflevector <4 x float> %708, <4 x float> %708, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %710 = load <4 x float>* %zi2.i102              ; <<4 x float>> [#uses=1]
  %711 = fmul <4 x float> %709, %710              ; <<4 x float>> [#uses=1]
  %712 = load float* %s2.i121                     ; <float> [#uses=1]
  %713 = insertelement <4 x float> undef, float %712, i32 0 ; <<4 x float>> [#uses=2]
  %714 = shufflevector <4 x float> %713, <4 x float> %713, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %715 = load <4 x float>* %zr2.i98               ; <<4 x float>> [#uses=1]
  %716 = fmul <4 x float> %714, %715              ; <<4 x float>> [#uses=1]
  %717 = fadd <4 x float> %711, %716              ; <<4 x float>> [#uses=1]
  store <4 x float> %717, <4 x float>* %zi2.i102
  %718 = load <4 x float>* %__r1.i122             ; <<4 x float>> [#uses=1]
  store <4 x float> %718, <4 x float>* %zr2.i98
  %719 = load i32* %tbase.i116                    ; <i32> [#uses=1]
  %720 = mul i32 %719, 3                          ; <i32> [#uses=1]
  store i32 %720, i32* %5
  store float* %c3.i123, float** %6
  %721 = load i32* %5                             ; <i32> [#uses=1]
  %722 = icmp sgt i32 %721, 512                   ; <i1> [#uses=1]
  br i1 %722, label %723, label %kfft_pass2.exit

; <label>:723                                     ; preds = %k_sincos.exit6.i127
  %724 = load i32* %5                             ; <i32> [#uses=1]
  %725 = sub i32 %724, 1024                       ; <i32> [#uses=1]
  store i32 %725, i32* %5
  br label %kfft_pass2.exit

kfft_pass2.exit:                                  ; preds = %k_sincos.exit6.i127, %723
  %726 = load i32* %5                             ; <i32> [#uses=1]
  %727 = sitofp i32 %726 to float                 ; <float> [#uses=1]
  %728 = fmul float %727, 0xBF7921FB60000000      ; <float> [#uses=1]
  store float %728, float* %x.i3.i93
  %729 = load float* %x.i3.i93                    ; <float> [#uses=1]
  %730 = call float @_Z10native_cosf(float %729) nounwind ; <float> [#uses=1]
  %731 = load float** %6                          ; <float*> [#uses=1]
  store float %730, float* %731
  %732 = load float* %x.i3.i93                    ; <float> [#uses=1]
  %733 = call float @_Z10native_sinf(float %732) nounwind ; <float> [#uses=1]
  store float %733, float* %4
  %734 = load float* %4                           ; <float> [#uses=1]
  store float %734, float* %s3.i124
  %735 = load float* %c3.i123                     ; <float> [#uses=1]
  %736 = insertelement <4 x float> undef, float %735, i32 0 ; <<4 x float>> [#uses=2]
  %737 = shufflevector <4 x float> %736, <4 x float> %736, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %738 = load <4 x float>* %zr3.i99               ; <<4 x float>> [#uses=1]
  %739 = fmul <4 x float> %737, %738              ; <<4 x float>> [#uses=1]
  %740 = load float* %s3.i124                     ; <float> [#uses=1]
  %741 = insertelement <4 x float> undef, float %740, i32 0 ; <<4 x float>> [#uses=2]
  %742 = shufflevector <4 x float> %741, <4 x float> %741, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %743 = load <4 x float>* %zi3.i103              ; <<4 x float>> [#uses=1]
  %744 = fmul <4 x float> %742, %743              ; <<4 x float>> [#uses=1]
  %745 = fsub <4 x float> %739, %744              ; <<4 x float>> [#uses=1]
  store <4 x float> %745, <4 x float>* %__r2.i125
  %746 = load float* %c3.i123                     ; <float> [#uses=1]
  %747 = insertelement <4 x float> undef, float %746, i32 0 ; <<4 x float>> [#uses=2]
  %748 = shufflevector <4 x float> %747, <4 x float> %747, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %749 = load <4 x float>* %zi3.i103              ; <<4 x float>> [#uses=1]
  %750 = fmul <4 x float> %748, %749              ; <<4 x float>> [#uses=1]
  %751 = load float* %s3.i124                     ; <float> [#uses=1]
  %752 = insertelement <4 x float> undef, float %751, i32 0 ; <<4 x float>> [#uses=2]
  %753 = shufflevector <4 x float> %752, <4 x float> %752, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %754 = load <4 x float>* %zr3.i99               ; <<4 x float>> [#uses=1]
  %755 = fmul <4 x float> %753, %754              ; <<4 x float>> [#uses=1]
  %756 = fadd <4 x float> %750, %755              ; <<4 x float>> [#uses=1]
  store <4 x float> %756, <4 x float>* %zi3.i103
  %757 = load <4 x float>* %__r2.i125             ; <<4 x float>> [#uses=1]
  store <4 x float> %757, <4 x float>* %zr3.i99
  call void @barrier(i32 1) nounwind
  %758 = load float addrspace(3)** %11            ; <float addrspace(3)*> [#uses=1]
  %759 = load i32* %10                            ; <i32> [#uses=1]
  %760 = shl i32 %759, 2                          ; <i32> [#uses=1]
  %761 = load i32* %10                            ; <i32> [#uses=1]
  %762 = lshr i32 %761, 3                         ; <i32> [#uses=1]
  %763 = add i32 %760, %762                       ; <i32> [#uses=1]
  %764 = getelementptr inbounds float addrspace(3)* %758, i32 %763 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %764, float addrspace(3)** %lp.i95
  %765 = load <4 x float>* %zr0.i96               ; <<4 x float>> [#uses=1]
  %766 = extractelement <4 x float> %765, i32 0   ; <float> [#uses=1]
  %767 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %768 = getelementptr inbounds float addrspace(3)* %767, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %766, float addrspace(3)* %768
  %769 = load <4 x float>* %zr1.i97               ; <<4 x float>> [#uses=1]
  %770 = extractelement <4 x float> %769, i32 0   ; <float> [#uses=1]
  %771 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %772 = getelementptr inbounds float addrspace(3)* %771, i32 1 ; <float addrspace(3)*> [#uses=1]
  store float %770, float addrspace(3)* %772
  %773 = load <4 x float>* %zr2.i98               ; <<4 x float>> [#uses=1]
  %774 = extractelement <4 x float> %773, i32 0   ; <float> [#uses=1]
  %775 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %776 = getelementptr inbounds float addrspace(3)* %775, i32 2 ; <float addrspace(3)*> [#uses=1]
  store float %774, float addrspace(3)* %776
  %777 = load <4 x float>* %zr3.i99               ; <<4 x float>> [#uses=1]
  %778 = extractelement <4 x float> %777, i32 0   ; <float> [#uses=1]
  %779 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %780 = getelementptr inbounds float addrspace(3)* %779, i32 3 ; <float addrspace(3)*> [#uses=1]
  store float %778, float addrspace(3)* %780
  %781 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %782 = getelementptr inbounds float addrspace(3)* %781, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %782, float addrspace(3)** %lp.i95
  %783 = load <4 x float>* %zr0.i96               ; <<4 x float>> [#uses=1]
  %784 = extractelement <4 x float> %783, i32 1   ; <float> [#uses=1]
  %785 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %786 = getelementptr inbounds float addrspace(3)* %785, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %784, float addrspace(3)* %786
  %787 = load <4 x float>* %zr1.i97               ; <<4 x float>> [#uses=1]
  %788 = extractelement <4 x float> %787, i32 1   ; <float> [#uses=1]
  %789 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %790 = getelementptr inbounds float addrspace(3)* %789, i32 1 ; <float addrspace(3)*> [#uses=1]
  store float %788, float addrspace(3)* %790
  %791 = load <4 x float>* %zr2.i98               ; <<4 x float>> [#uses=1]
  %792 = extractelement <4 x float> %791, i32 1   ; <float> [#uses=1]
  %793 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %794 = getelementptr inbounds float addrspace(3)* %793, i32 2 ; <float addrspace(3)*> [#uses=1]
  store float %792, float addrspace(3)* %794
  %795 = load <4 x float>* %zr3.i99               ; <<4 x float>> [#uses=1]
  %796 = extractelement <4 x float> %795, i32 1   ; <float> [#uses=1]
  %797 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %798 = getelementptr inbounds float addrspace(3)* %797, i32 3 ; <float addrspace(3)*> [#uses=1]
  store float %796, float addrspace(3)* %798
  %799 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %800 = getelementptr inbounds float addrspace(3)* %799, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %800, float addrspace(3)** %lp.i95
  %801 = load <4 x float>* %zr0.i96               ; <<4 x float>> [#uses=1]
  %802 = extractelement <4 x float> %801, i32 2   ; <float> [#uses=1]
  %803 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %804 = getelementptr inbounds float addrspace(3)* %803, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %802, float addrspace(3)* %804
  %805 = load <4 x float>* %zr1.i97               ; <<4 x float>> [#uses=1]
  %806 = extractelement <4 x float> %805, i32 2   ; <float> [#uses=1]
  %807 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %808 = getelementptr inbounds float addrspace(3)* %807, i32 1 ; <float addrspace(3)*> [#uses=1]
  store float %806, float addrspace(3)* %808
  %809 = load <4 x float>* %zr2.i98               ; <<4 x float>> [#uses=1]
  %810 = extractelement <4 x float> %809, i32 2   ; <float> [#uses=1]
  %811 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %812 = getelementptr inbounds float addrspace(3)* %811, i32 2 ; <float addrspace(3)*> [#uses=1]
  store float %810, float addrspace(3)* %812
  %813 = load <4 x float>* %zr3.i99               ; <<4 x float>> [#uses=1]
  %814 = extractelement <4 x float> %813, i32 2   ; <float> [#uses=1]
  %815 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %816 = getelementptr inbounds float addrspace(3)* %815, i32 3 ; <float addrspace(3)*> [#uses=1]
  store float %814, float addrspace(3)* %816
  %817 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %818 = getelementptr inbounds float addrspace(3)* %817, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %818, float addrspace(3)** %lp.i95
  %819 = load <4 x float>* %zr0.i96               ; <<4 x float>> [#uses=1]
  %820 = extractelement <4 x float> %819, i32 3   ; <float> [#uses=1]
  %821 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %822 = getelementptr inbounds float addrspace(3)* %821, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %820, float addrspace(3)* %822
  %823 = load <4 x float>* %zr1.i97               ; <<4 x float>> [#uses=1]
  %824 = extractelement <4 x float> %823, i32 3   ; <float> [#uses=1]
  %825 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %826 = getelementptr inbounds float addrspace(3)* %825, i32 1 ; <float addrspace(3)*> [#uses=1]
  store float %824, float addrspace(3)* %826
  %827 = load <4 x float>* %zr2.i98               ; <<4 x float>> [#uses=1]
  %828 = extractelement <4 x float> %827, i32 3   ; <float> [#uses=1]
  %829 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %830 = getelementptr inbounds float addrspace(3)* %829, i32 2 ; <float addrspace(3)*> [#uses=1]
  store float %828, float addrspace(3)* %830
  %831 = load <4 x float>* %zr3.i99               ; <<4 x float>> [#uses=1]
  %832 = extractelement <4 x float> %831, i32 3   ; <float> [#uses=1]
  %833 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %834 = getelementptr inbounds float addrspace(3)* %833, i32 3 ; <float addrspace(3)*> [#uses=1]
  store float %832, float addrspace(3)* %834
  %835 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %836 = getelementptr inbounds float addrspace(3)* %835, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %836, float addrspace(3)** %lp.i95
  %837 = load <4 x float>* %zi0.i100              ; <<4 x float>> [#uses=1]
  %838 = extractelement <4 x float> %837, i32 0   ; <float> [#uses=1]
  %839 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %840 = getelementptr inbounds float addrspace(3)* %839, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %838, float addrspace(3)* %840
  %841 = load <4 x float>* %zi1.i101              ; <<4 x float>> [#uses=1]
  %842 = extractelement <4 x float> %841, i32 0   ; <float> [#uses=1]
  %843 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %844 = getelementptr inbounds float addrspace(3)* %843, i32 1 ; <float addrspace(3)*> [#uses=1]
  store float %842, float addrspace(3)* %844
  %845 = load <4 x float>* %zi2.i102              ; <<4 x float>> [#uses=1]
  %846 = extractelement <4 x float> %845, i32 0   ; <float> [#uses=1]
  %847 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %848 = getelementptr inbounds float addrspace(3)* %847, i32 2 ; <float addrspace(3)*> [#uses=1]
  store float %846, float addrspace(3)* %848
  %849 = load <4 x float>* %zi3.i103              ; <<4 x float>> [#uses=1]
  %850 = extractelement <4 x float> %849, i32 0   ; <float> [#uses=1]
  %851 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %852 = getelementptr inbounds float addrspace(3)* %851, i32 3 ; <float addrspace(3)*> [#uses=1]
  store float %850, float addrspace(3)* %852
  %853 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %854 = getelementptr inbounds float addrspace(3)* %853, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %854, float addrspace(3)** %lp.i95
  %855 = load <4 x float>* %zi0.i100              ; <<4 x float>> [#uses=1]
  %856 = extractelement <4 x float> %855, i32 1   ; <float> [#uses=1]
  %857 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %858 = getelementptr inbounds float addrspace(3)* %857, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %856, float addrspace(3)* %858
  %859 = load <4 x float>* %zi1.i101              ; <<4 x float>> [#uses=1]
  %860 = extractelement <4 x float> %859, i32 1   ; <float> [#uses=1]
  %861 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %862 = getelementptr inbounds float addrspace(3)* %861, i32 1 ; <float addrspace(3)*> [#uses=1]
  store float %860, float addrspace(3)* %862
  %863 = load <4 x float>* %zi2.i102              ; <<4 x float>> [#uses=1]
  %864 = extractelement <4 x float> %863, i32 1   ; <float> [#uses=1]
  %865 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %866 = getelementptr inbounds float addrspace(3)* %865, i32 2 ; <float addrspace(3)*> [#uses=1]
  store float %864, float addrspace(3)* %866
  %867 = load <4 x float>* %zi3.i103              ; <<4 x float>> [#uses=1]
  %868 = extractelement <4 x float> %867, i32 1   ; <float> [#uses=1]
  %869 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %870 = getelementptr inbounds float addrspace(3)* %869, i32 3 ; <float addrspace(3)*> [#uses=1]
  store float %868, float addrspace(3)* %870
  %871 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %872 = getelementptr inbounds float addrspace(3)* %871, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %872, float addrspace(3)** %lp.i95
  %873 = load <4 x float>* %zi0.i100              ; <<4 x float>> [#uses=1]
  %874 = extractelement <4 x float> %873, i32 2   ; <float> [#uses=1]
  %875 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %876 = getelementptr inbounds float addrspace(3)* %875, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %874, float addrspace(3)* %876
  %877 = load <4 x float>* %zi1.i101              ; <<4 x float>> [#uses=1]
  %878 = extractelement <4 x float> %877, i32 2   ; <float> [#uses=1]
  %879 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %880 = getelementptr inbounds float addrspace(3)* %879, i32 1 ; <float addrspace(3)*> [#uses=1]
  store float %878, float addrspace(3)* %880
  %881 = load <4 x float>* %zi2.i102              ; <<4 x float>> [#uses=1]
  %882 = extractelement <4 x float> %881, i32 2   ; <float> [#uses=1]
  %883 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %884 = getelementptr inbounds float addrspace(3)* %883, i32 2 ; <float addrspace(3)*> [#uses=1]
  store float %882, float addrspace(3)* %884
  %885 = load <4 x float>* %zi3.i103              ; <<4 x float>> [#uses=1]
  %886 = extractelement <4 x float> %885, i32 2   ; <float> [#uses=1]
  %887 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %888 = getelementptr inbounds float addrspace(3)* %887, i32 3 ; <float addrspace(3)*> [#uses=1]
  store float %886, float addrspace(3)* %888
  %889 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %890 = getelementptr inbounds float addrspace(3)* %889, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %890, float addrspace(3)** %lp.i95
  %891 = load <4 x float>* %zi0.i100              ; <<4 x float>> [#uses=1]
  %892 = extractelement <4 x float> %891, i32 3   ; <float> [#uses=1]
  %893 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %894 = getelementptr inbounds float addrspace(3)* %893, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %892, float addrspace(3)* %894
  %895 = load <4 x float>* %zi1.i101              ; <<4 x float>> [#uses=1]
  %896 = extractelement <4 x float> %895, i32 3   ; <float> [#uses=1]
  %897 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %898 = getelementptr inbounds float addrspace(3)* %897, i32 1 ; <float addrspace(3)*> [#uses=1]
  store float %896, float addrspace(3)* %898
  %899 = load <4 x float>* %zi2.i102              ; <<4 x float>> [#uses=1]
  %900 = extractelement <4 x float> %899, i32 3   ; <float> [#uses=1]
  %901 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %902 = getelementptr inbounds float addrspace(3)* %901, i32 2 ; <float addrspace(3)*> [#uses=1]
  store float %900, float addrspace(3)* %902
  %903 = load <4 x float>* %zi3.i103              ; <<4 x float>> [#uses=1]
  %904 = extractelement <4 x float> %903, i32 3   ; <float> [#uses=1]
  %905 = load float addrspace(3)** %lp.i95        ; <float addrspace(3)*> [#uses=1]
  %906 = getelementptr inbounds float addrspace(3)* %905, i32 3 ; <float addrspace(3)*> [#uses=1]
  store float %904, float addrspace(3)* %906
  call void @barrier(i32 1) nounwind
  %907 = load i32* %me                            ; <i32> [#uses=1]
  store i32 %907, i32* %21
  store float addrspace(3)* getelementptr inbounds ([2176 x float] addrspace(3)* @opencl_kfft_local_lds, i32 0, i32 0), float addrspace(3)** %22
  %908 = load float addrspace(3)** %22            ; <float addrspace(3)*> [#uses=1]
  %909 = load i32* %21                            ; <i32> [#uses=1]
  %910 = load i32* %21                            ; <i32> [#uses=1]
  %911 = lshr i32 %910, 5                         ; <i32> [#uses=1]
  %912 = add i32 %909, %911                       ; <i32> [#uses=1]
  %913 = getelementptr inbounds float addrspace(3)* %908, i32 %912 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %913, float addrspace(3)** %lp.i59
  %914 = load float addrspace(3)** %lp.i59        ; <float addrspace(3)*> [#uses=1]
  %915 = getelementptr inbounds float addrspace(3)* %914, i32 0 ; <float addrspace(3)*> [#uses=1]
  %916 = load float addrspace(3)* %915            ; <float> [#uses=1]
  %917 = load <4 x float>* %zr0.i60               ; <<4 x float>> [#uses=1]
  %918 = insertelement <4 x float> %917, float %916, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %918, <4 x float>* %zr0.i60
  %919 = load float addrspace(3)** %lp.i59        ; <float addrspace(3)*> [#uses=1]
  %920 = getelementptr inbounds float addrspace(3)* %919, i32 66 ; <float addrspace(3)*> [#uses=1]
  %921 = load float addrspace(3)* %920            ; <float> [#uses=1]
  %922 = load <4 x float>* %zr1.i61               ; <<4 x float>> [#uses=1]
  %923 = insertelement <4 x float> %922, float %921, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %923, <4 x float>* %zr1.i61
  %924 = load float addrspace(3)** %lp.i59        ; <float addrspace(3)*> [#uses=1]
  %925 = getelementptr inbounds float addrspace(3)* %924, i32 132 ; <float addrspace(3)*> [#uses=1]
  %926 = load float addrspace(3)* %925            ; <float> [#uses=1]
  %927 = load <4 x float>* %zr2.i62               ; <<4 x float>> [#uses=1]
  %928 = insertelement <4 x float> %927, float %926, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %928, <4 x float>* %zr2.i62
  %929 = load float addrspace(3)** %lp.i59        ; <float addrspace(3)*> [#uses=1]
  %930 = getelementptr inbounds float addrspace(3)* %929, i32 198 ; <float addrspace(3)*> [#uses=1]
  %931 = load float addrspace(3)* %930            ; <float> [#uses=1]
  %932 = load <4 x float>* %zr3.i63               ; <<4 x float>> [#uses=1]
  %933 = insertelement <4 x float> %932, float %931, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %933, <4 x float>* %zr3.i63
  %934 = load float addrspace(3)** %lp.i59        ; <float addrspace(3)*> [#uses=1]
  %935 = getelementptr inbounds float addrspace(3)* %934, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %935, float addrspace(3)** %lp.i59
  %936 = load float addrspace(3)** %lp.i59        ; <float addrspace(3)*> [#uses=1]
  %937 = getelementptr inbounds float addrspace(3)* %936, i32 0 ; <float addrspace(3)*> [#uses=1]
  %938 = load float addrspace(3)* %937            ; <float> [#uses=1]
  %939 = load <4 x float>* %zr0.i60               ; <<4 x float>> [#uses=1]
  %940 = insertelement <4 x float> %939, float %938, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %940, <4 x float>* %zr0.i60
  %941 = load float addrspace(3)** %lp.i59        ; <float addrspace(3)*> [#uses=1]
  %942 = getelementptr inbounds float addrspace(3)* %941, i32 66 ; <float addrspace(3)*> [#uses=1]
  %943 = load float addrspace(3)* %942            ; <float> [#uses=1]
  %944 = load <4 x float>* %zr1.i61               ; <<4 x float>> [#uses=1]
  %945 = insertelement <4 x float> %944, float %943, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %945, <4 x float>* %zr1.i61
  %946 = load float addrspace(3)** %lp.i59        ; <float addrspace(3)*> [#uses=1]
  %947 = getelementptr inbounds float addrspace(3)* %946, i32 132 ; <float addrspace(3)*> [#uses=1]
  %948 = load float addrspace(3)* %947            ; <float> [#uses=1]
  %949 = load <4 x float>* %zr2.i62               ; <<4 x float>> [#uses=1]
  %950 = insertelement <4 x float> %949, float %948, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %950, <4 x float>* %zr2.i62
  %951 = load float addrspace(3)** %lp.i59        ; <float addrspace(3)*> [#uses=1]
  %952 = getelementptr inbounds float addrspace(3)* %951, i32 198 ; <float addrspace(3)*> [#uses=1]
  %953 = load float addrspace(3)* %952            ; <float> [#uses=1]
  %954 = load <4 x float>* %zr3.i63               ; <<4 x float>> [#uses=1]
  %955 = insertelement <4 x float> %954, float %953, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %955, <4 x float>* %zr3.i63
  %956 = load float addrspace(3)** %lp.i59        ; <float addrspace(3)*> [#uses=1]
  %957 = getelementptr inbounds float addrspace(3)* %956, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %957, float addrspace(3)** %lp.i59
  %958 = load float addrspace(3)** %lp.i59        ; <float addrspace(3)*> [#uses=1]
  %959 = getelementptr inbounds float addrspace(3)* %958, i32 0 ; <float addrspace(3)*> [#uses=1]
  %960 = load float addrspace(3)* %959            ; <float> [#uses=1]
  %961 = load <4 x float>* %zr0.i60               ; <<4 x float>> [#uses=1]
  %962 = insertelement <4 x float> %961, float %960, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %962, <4 x float>* %zr0.i60
  %963 = load float addrspace(3)** %lp.i59        ; <float addrspace(3)*> [#uses=1]
  %964 = getelementptr inbounds float addrspace(3)* %963, i32 66 ; <float addrspace(3)*> [#uses=1]
  %965 = load float addrspace(3)* %964            ; <float> [#uses=1]
  %966 = load <4 x float>* %zr1.i61               ; <<4 x float>> [#uses=1]
  %967 = insertelement <4 x float> %966, float %965, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %967, <4 x float>* %zr1.i61
  %968 = load float addrspace(3)** %lp.i59        ; <float addrspace(3)*> [#uses=1]
  %969 = getelementptr inbounds float addrspace(3)* %968, i32 132 ; <float addrspace(3)*> [#uses=1]
  %970 = load float addrspace(3)* %969            ; <float> [#uses=1]
  %971 = load <4 x float>* %zr2.i62               ; <<4 x float>> [#uses=1]
  %972 = insertelement <4 x float> %971, float %970, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %972, <4 x float>* %zr2.i62
  %973 = load float addrspace(3)** %lp.i59        ; <float addrspace(3)*> [#uses=1]
  %974 = getelementptr inbounds float addrspace(3)* %973, i32 198 ; <float addrspace(3)*> [#uses=1]
  %975 = load float addrspace(3)* %974            ; <float> [#uses=1]
  %976 = load <4 x float>* %zr3.i63               ; <<4 x float>> [#uses=1]
  %977 = insertelement <4 x float> %976, float %975, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %977, <4 x float>* %zr3.i63
  %978 = load float addrspace(3)** %lp.i59        ; <float addrspace(3)*> [#uses=1]
  %979 = getelementptr inbounds float addrspace(3)* %978, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %979, float addrspace(3)** %lp.i59
  %980 = load float addrspace(3)** %lp.i59        ; <float addrspace(3)*> [#uses=1]
  %981 = getelementptr inbounds float addrspace(3)* %980, i32 0 ; <float addrspace(3)*> [#uses=1]
  %982 = load float addrspace(3)* %981            ; <float> [#uses=1]
  %983 = load <4 x float>* %zr0.i60               ; <<4 x float>> [#uses=1]
  %984 = insertelement <4 x float> %983, float %982, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %984, <4 x float>* %zr0.i60
  %985 = load float addrspace(3)** %lp.i59        ; <float addrspace(3)*> [#uses=1]
  %986 = getelementptr inbounds float addrspace(3)* %985, i32 66 ; <float addrspace(3)*> [#uses=1]
  %987 = load float addrspace(3)* %986            ; <float> [#uses=1]
  %988 = load <4 x float>* %zr1.i61               ; <<4 x float>> [#uses=1]
  %989 = insertelement <4 x float> %988, float %987, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %989, <4 x float>* %zr1.i61
  %990 = load float addrspace(3)** %lp.i59        ; <float addrspace(3)*> [#uses=1]
  %991 = getelementptr inbounds float addrspace(3)* %990, i32 132 ; <float addrspace(3)*> [#uses=1]
  %992 = load float addrspace(3)* %991            ; <float> [#uses=1]
  %993 = load <4 x float>* %zr2.i62               ; <<4 x float>> [#uses=1]
  %994 = insertelement <4 x float> %993, float %992, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %994, <4 x float>* %zr2.i62
  %995 = load float addrspace(3)** %lp.i59        ; <float addrspace(3)*> [#uses=1]
  %996 = getelementptr inbounds float addrspace(3)* %995, i32 198 ; <float addrspace(3)*> [#uses=1]
  %997 = load float addrspace(3)* %996            ; <float> [#uses=1]
  %998 = load <4 x float>* %zr3.i63               ; <<4 x float>> [#uses=1]
  %999 = insertelement <4 x float> %998, float %997, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %999, <4 x float>* %zr3.i63
  %1000 = load float addrspace(3)** %lp.i59       ; <float addrspace(3)*> [#uses=1]
  %1001 = getelementptr inbounds float addrspace(3)* %1000, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %1001, float addrspace(3)** %lp.i59
  %1002 = load float addrspace(3)** %lp.i59       ; <float addrspace(3)*> [#uses=1]
  %1003 = getelementptr inbounds float addrspace(3)* %1002, i32 0 ; <float addrspace(3)*> [#uses=1]
  %1004 = load float addrspace(3)* %1003          ; <float> [#uses=1]
  %1005 = load <4 x float>* %zi0.i64              ; <<4 x float>> [#uses=1]
  %1006 = insertelement <4 x float> %1005, float %1004, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %1006, <4 x float>* %zi0.i64
  %1007 = load float addrspace(3)** %lp.i59       ; <float addrspace(3)*> [#uses=1]
  %1008 = getelementptr inbounds float addrspace(3)* %1007, i32 66 ; <float addrspace(3)*> [#uses=1]
  %1009 = load float addrspace(3)* %1008          ; <float> [#uses=1]
  %1010 = load <4 x float>* %zi1.i65              ; <<4 x float>> [#uses=1]
  %1011 = insertelement <4 x float> %1010, float %1009, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %1011, <4 x float>* %zi1.i65
  %1012 = load float addrspace(3)** %lp.i59       ; <float addrspace(3)*> [#uses=1]
  %1013 = getelementptr inbounds float addrspace(3)* %1012, i32 132 ; <float addrspace(3)*> [#uses=1]
  %1014 = load float addrspace(3)* %1013          ; <float> [#uses=1]
  %1015 = load <4 x float>* %zi2.i66              ; <<4 x float>> [#uses=1]
  %1016 = insertelement <4 x float> %1015, float %1014, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %1016, <4 x float>* %zi2.i66
  %1017 = load float addrspace(3)** %lp.i59       ; <float addrspace(3)*> [#uses=1]
  %1018 = getelementptr inbounds float addrspace(3)* %1017, i32 198 ; <float addrspace(3)*> [#uses=1]
  %1019 = load float addrspace(3)* %1018          ; <float> [#uses=1]
  %1020 = load <4 x float>* %zi3.i67              ; <<4 x float>> [#uses=1]
  %1021 = insertelement <4 x float> %1020, float %1019, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %1021, <4 x float>* %zi3.i67
  %1022 = load float addrspace(3)** %lp.i59       ; <float addrspace(3)*> [#uses=1]
  %1023 = getelementptr inbounds float addrspace(3)* %1022, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %1023, float addrspace(3)** %lp.i59
  %1024 = load float addrspace(3)** %lp.i59       ; <float addrspace(3)*> [#uses=1]
  %1025 = getelementptr inbounds float addrspace(3)* %1024, i32 0 ; <float addrspace(3)*> [#uses=1]
  %1026 = load float addrspace(3)* %1025          ; <float> [#uses=1]
  %1027 = load <4 x float>* %zi0.i64              ; <<4 x float>> [#uses=1]
  %1028 = insertelement <4 x float> %1027, float %1026, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %1028, <4 x float>* %zi0.i64
  %1029 = load float addrspace(3)** %lp.i59       ; <float addrspace(3)*> [#uses=1]
  %1030 = getelementptr inbounds float addrspace(3)* %1029, i32 66 ; <float addrspace(3)*> [#uses=1]
  %1031 = load float addrspace(3)* %1030          ; <float> [#uses=1]
  %1032 = load <4 x float>* %zi1.i65              ; <<4 x float>> [#uses=1]
  %1033 = insertelement <4 x float> %1032, float %1031, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %1033, <4 x float>* %zi1.i65
  %1034 = load float addrspace(3)** %lp.i59       ; <float addrspace(3)*> [#uses=1]
  %1035 = getelementptr inbounds float addrspace(3)* %1034, i32 132 ; <float addrspace(3)*> [#uses=1]
  %1036 = load float addrspace(3)* %1035          ; <float> [#uses=1]
  %1037 = load <4 x float>* %zi2.i66              ; <<4 x float>> [#uses=1]
  %1038 = insertelement <4 x float> %1037, float %1036, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %1038, <4 x float>* %zi2.i66
  %1039 = load float addrspace(3)** %lp.i59       ; <float addrspace(3)*> [#uses=1]
  %1040 = getelementptr inbounds float addrspace(3)* %1039, i32 198 ; <float addrspace(3)*> [#uses=1]
  %1041 = load float addrspace(3)* %1040          ; <float> [#uses=1]
  %1042 = load <4 x float>* %zi3.i67              ; <<4 x float>> [#uses=1]
  %1043 = insertelement <4 x float> %1042, float %1041, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %1043, <4 x float>* %zi3.i67
  %1044 = load float addrspace(3)** %lp.i59       ; <float addrspace(3)*> [#uses=1]
  %1045 = getelementptr inbounds float addrspace(3)* %1044, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %1045, float addrspace(3)** %lp.i59
  %1046 = load float addrspace(3)** %lp.i59       ; <float addrspace(3)*> [#uses=1]
  %1047 = getelementptr inbounds float addrspace(3)* %1046, i32 0 ; <float addrspace(3)*> [#uses=1]
  %1048 = load float addrspace(3)* %1047          ; <float> [#uses=1]
  %1049 = load <4 x float>* %zi0.i64              ; <<4 x float>> [#uses=1]
  %1050 = insertelement <4 x float> %1049, float %1048, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %1050, <4 x float>* %zi0.i64
  %1051 = load float addrspace(3)** %lp.i59       ; <float addrspace(3)*> [#uses=1]
  %1052 = getelementptr inbounds float addrspace(3)* %1051, i32 66 ; <float addrspace(3)*> [#uses=1]
  %1053 = load float addrspace(3)* %1052          ; <float> [#uses=1]
  %1054 = load <4 x float>* %zi1.i65              ; <<4 x float>> [#uses=1]
  %1055 = insertelement <4 x float> %1054, float %1053, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %1055, <4 x float>* %zi1.i65
  %1056 = load float addrspace(3)** %lp.i59       ; <float addrspace(3)*> [#uses=1]
  %1057 = getelementptr inbounds float addrspace(3)* %1056, i32 132 ; <float addrspace(3)*> [#uses=1]
  %1058 = load float addrspace(3)* %1057          ; <float> [#uses=1]
  %1059 = load <4 x float>* %zi2.i66              ; <<4 x float>> [#uses=1]
  %1060 = insertelement <4 x float> %1059, float %1058, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %1060, <4 x float>* %zi2.i66
  %1061 = load float addrspace(3)** %lp.i59       ; <float addrspace(3)*> [#uses=1]
  %1062 = getelementptr inbounds float addrspace(3)* %1061, i32 198 ; <float addrspace(3)*> [#uses=1]
  %1063 = load float addrspace(3)* %1062          ; <float> [#uses=1]
  %1064 = load <4 x float>* %zi3.i67              ; <<4 x float>> [#uses=1]
  %1065 = insertelement <4 x float> %1064, float %1063, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %1065, <4 x float>* %zi3.i67
  %1066 = load float addrspace(3)** %lp.i59       ; <float addrspace(3)*> [#uses=1]
  %1067 = getelementptr inbounds float addrspace(3)* %1066, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %1067, float addrspace(3)** %lp.i59
  %1068 = load float addrspace(3)** %lp.i59       ; <float addrspace(3)*> [#uses=1]
  %1069 = getelementptr inbounds float addrspace(3)* %1068, i32 0 ; <float addrspace(3)*> [#uses=1]
  %1070 = load float addrspace(3)* %1069          ; <float> [#uses=1]
  %1071 = load <4 x float>* %zi0.i64              ; <<4 x float>> [#uses=1]
  %1072 = insertelement <4 x float> %1071, float %1070, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %1072, <4 x float>* %zi0.i64
  %1073 = load float addrspace(3)** %lp.i59       ; <float addrspace(3)*> [#uses=1]
  %1074 = getelementptr inbounds float addrspace(3)* %1073, i32 66 ; <float addrspace(3)*> [#uses=1]
  %1075 = load float addrspace(3)* %1074          ; <float> [#uses=1]
  %1076 = load <4 x float>* %zi1.i65              ; <<4 x float>> [#uses=1]
  %1077 = insertelement <4 x float> %1076, float %1075, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %1077, <4 x float>* %zi1.i65
  %1078 = load float addrspace(3)** %lp.i59       ; <float addrspace(3)*> [#uses=1]
  %1079 = getelementptr inbounds float addrspace(3)* %1078, i32 132 ; <float addrspace(3)*> [#uses=1]
  %1080 = load float addrspace(3)* %1079          ; <float> [#uses=1]
  %1081 = load <4 x float>* %zi2.i66              ; <<4 x float>> [#uses=1]
  %1082 = insertelement <4 x float> %1081, float %1080, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %1082, <4 x float>* %zi2.i66
  %1083 = load float addrspace(3)** %lp.i59       ; <float addrspace(3)*> [#uses=1]
  %1084 = getelementptr inbounds float addrspace(3)* %1083, i32 198 ; <float addrspace(3)*> [#uses=1]
  %1085 = load float addrspace(3)* %1084          ; <float> [#uses=1]
  %1086 = load <4 x float>* %zi3.i67              ; <<4 x float>> [#uses=1]
  %1087 = insertelement <4 x float> %1086, float %1085, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %1087, <4 x float>* %zi3.i67
  %1088 = load <4 x float>* %zr0.i60              ; <<4 x float>> [#uses=1]
  %1089 = load <4 x float>* %zr2.i62              ; <<4 x float>> [#uses=1]
  %1090 = fadd <4 x float> %1088, %1089           ; <<4 x float>> [#uses=1]
  store <4 x float> %1090, <4 x float>* %ar0.i68
  %1091 = load <4 x float>* %zr1.i61              ; <<4 x float>> [#uses=1]
  %1092 = load <4 x float>* %zr3.i63              ; <<4 x float>> [#uses=1]
  %1093 = fadd <4 x float> %1091, %1092           ; <<4 x float>> [#uses=1]
  store <4 x float> %1093, <4 x float>* %ar2.i69
  %1094 = load <4 x float>* %ar0.i68              ; <<4 x float>> [#uses=1]
  %1095 = load <4 x float>* %ar2.i69              ; <<4 x float>> [#uses=1]
  %1096 = fadd <4 x float> %1094, %1095           ; <<4 x float>> [#uses=1]
  store <4 x float> %1096, <4 x float>* %br0.i70
  %1097 = load <4 x float>* %zr0.i60              ; <<4 x float>> [#uses=1]
  %1098 = load <4 x float>* %zr2.i62              ; <<4 x float>> [#uses=1]
  %1099 = fsub <4 x float> %1097, %1098           ; <<4 x float>> [#uses=1]
  store <4 x float> %1099, <4 x float>* %br1.i71
  %1100 = load <4 x float>* %ar0.i68              ; <<4 x float>> [#uses=1]
  %1101 = load <4 x float>* %ar2.i69              ; <<4 x float>> [#uses=1]
  %1102 = fsub <4 x float> %1100, %1101           ; <<4 x float>> [#uses=1]
  store <4 x float> %1102, <4 x float>* %br2.i72
  %1103 = load <4 x float>* %zr1.i61              ; <<4 x float>> [#uses=1]
  %1104 = load <4 x float>* %zr3.i63              ; <<4 x float>> [#uses=1]
  %1105 = fsub <4 x float> %1103, %1104           ; <<4 x float>> [#uses=1]
  store <4 x float> %1105, <4 x float>* %br3.i73
  %1106 = load <4 x float>* %zi0.i64              ; <<4 x float>> [#uses=1]
  %1107 = load <4 x float>* %zi2.i66              ; <<4 x float>> [#uses=1]
  %1108 = fadd <4 x float> %1106, %1107           ; <<4 x float>> [#uses=1]
  store <4 x float> %1108, <4 x float>* %ai0.i74
  %1109 = load <4 x float>* %zi1.i65              ; <<4 x float>> [#uses=1]
  %1110 = load <4 x float>* %zi3.i67              ; <<4 x float>> [#uses=1]
  %1111 = fadd <4 x float> %1109, %1110           ; <<4 x float>> [#uses=1]
  store <4 x float> %1111, <4 x float>* %ai2.i75
  %1112 = load <4 x float>* %ai0.i74              ; <<4 x float>> [#uses=1]
  %1113 = load <4 x float>* %ai2.i75              ; <<4 x float>> [#uses=1]
  %1114 = fadd <4 x float> %1112, %1113           ; <<4 x float>> [#uses=1]
  store <4 x float> %1114, <4 x float>* %bi0.i76
  %1115 = load <4 x float>* %zi0.i64              ; <<4 x float>> [#uses=1]
  %1116 = load <4 x float>* %zi2.i66              ; <<4 x float>> [#uses=1]
  %1117 = fsub <4 x float> %1115, %1116           ; <<4 x float>> [#uses=1]
  store <4 x float> %1117, <4 x float>* %bi1.i77
  %1118 = load <4 x float>* %ai0.i74              ; <<4 x float>> [#uses=1]
  %1119 = load <4 x float>* %ai2.i75              ; <<4 x float>> [#uses=1]
  %1120 = fsub <4 x float> %1118, %1119           ; <<4 x float>> [#uses=1]
  store <4 x float> %1120, <4 x float>* %bi2.i78
  %1121 = load <4 x float>* %zi1.i65              ; <<4 x float>> [#uses=1]
  %1122 = load <4 x float>* %zi3.i67              ; <<4 x float>> [#uses=1]
  %1123 = fsub <4 x float> %1121, %1122           ; <<4 x float>> [#uses=1]
  store <4 x float> %1123, <4 x float>* %bi3.i79
  %1124 = load <4 x float>* %br0.i70              ; <<4 x float>> [#uses=1]
  store <4 x float> %1124, <4 x float>* %zr0.i60
  %1125 = load <4 x float>* %bi0.i76              ; <<4 x float>> [#uses=1]
  store <4 x float> %1125, <4 x float>* %zi0.i64
  %1126 = load <4 x float>* %br1.i71              ; <<4 x float>> [#uses=1]
  %1127 = load <4 x float>* %bi3.i79              ; <<4 x float>> [#uses=1]
  %1128 = fadd <4 x float> %1126, %1127           ; <<4 x float>> [#uses=1]
  store <4 x float> %1128, <4 x float>* %zr1.i61
  %1129 = load <4 x float>* %bi1.i77              ; <<4 x float>> [#uses=1]
  %1130 = load <4 x float>* %br3.i73              ; <<4 x float>> [#uses=1]
  %1131 = fsub <4 x float> %1129, %1130           ; <<4 x float>> [#uses=1]
  store <4 x float> %1131, <4 x float>* %zi1.i65
  %1132 = load <4 x float>* %br1.i71              ; <<4 x float>> [#uses=1]
  %1133 = load <4 x float>* %bi3.i79              ; <<4 x float>> [#uses=1]
  %1134 = fsub <4 x float> %1132, %1133           ; <<4 x float>> [#uses=1]
  store <4 x float> %1134, <4 x float>* %zr3.i63
  %1135 = load <4 x float>* %br3.i73              ; <<4 x float>> [#uses=1]
  %1136 = load <4 x float>* %bi1.i77              ; <<4 x float>> [#uses=1]
  %1137 = fadd <4 x float> %1135, %1136           ; <<4 x float>> [#uses=1]
  store <4 x float> %1137, <4 x float>* %zi3.i67
  %1138 = load <4 x float>* %br2.i72              ; <<4 x float>> [#uses=1]
  store <4 x float> %1138, <4 x float>* %zr2.i62
  %1139 = load <4 x float>* %bi2.i78              ; <<4 x float>> [#uses=1]
  store <4 x float> %1139, <4 x float>* %zi2.i66
  %1140 = load i32* %21                           ; <i32> [#uses=1]
  %1141 = lshr i32 %1140, 2                       ; <i32> [#uses=1]
  %1142 = shl i32 %1141, 4                        ; <i32> [#uses=1]
  store i32 %1142, i32* %tbase.i80
  %1143 = load i32* %tbase.i80                    ; <i32> [#uses=1]
  %1144 = mul i32 %1143, 1                        ; <i32> [#uses=1]
  store i32 %1144, i32* %19
  store float* %c1.i81, float** %20
  %1145 = load i32* %19                           ; <i32> [#uses=1]
  %1146 = icmp sgt i32 %1145, 512                 ; <i1> [#uses=1]
  br i1 %1146, label %1147, label %k_sincos.exit.i90

; <label>:1147                                    ; preds = %kfft_pass2.exit
  %1148 = load i32* %19                           ; <i32> [#uses=1]
  %1149 = sub i32 %1148, 1024                     ; <i32> [#uses=1]
  store i32 %1149, i32* %19
  br label %k_sincos.exit.i90

k_sincos.exit.i90:                                ; preds = %1147, %kfft_pass2.exit
  %1150 = load i32* %19                           ; <i32> [#uses=1]
  %1151 = sitofp i32 %1150 to float               ; <float> [#uses=1]
  %1152 = fmul float %1151, 0xBF7921FB60000000    ; <float> [#uses=1]
  store float %1152, float* %x.i.i58
  %1153 = load float* %x.i.i58                    ; <float> [#uses=1]
  %1154 = call float @_Z10native_cosf(float %1153) nounwind ; <float> [#uses=1]
  %1155 = load float** %20                        ; <float*> [#uses=1]
  store float %1154, float* %1155
  %1156 = load float* %x.i.i58                    ; <float> [#uses=1]
  %1157 = call float @_Z10native_sinf(float %1156) nounwind ; <float> [#uses=1]
  store float %1157, float* %18
  %1158 = load float* %18                         ; <float> [#uses=1]
  store float %1158, float* %s1.i82
  %1159 = load float* %c1.i81                     ; <float> [#uses=1]
  %1160 = insertelement <4 x float> undef, float %1159, i32 0 ; <<4 x float>> [#uses=2]
  %1161 = shufflevector <4 x float> %1160, <4 x float> %1160, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %1162 = load <4 x float>* %zr1.i61              ; <<4 x float>> [#uses=1]
  %1163 = fmul <4 x float> %1161, %1162           ; <<4 x float>> [#uses=1]
  %1164 = load float* %s1.i82                     ; <float> [#uses=1]
  %1165 = insertelement <4 x float> undef, float %1164, i32 0 ; <<4 x float>> [#uses=2]
  %1166 = shufflevector <4 x float> %1165, <4 x float> %1165, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %1167 = load <4 x float>* %zi1.i65              ; <<4 x float>> [#uses=1]
  %1168 = fmul <4 x float> %1166, %1167           ; <<4 x float>> [#uses=1]
  %1169 = fsub <4 x float> %1163, %1168           ; <<4 x float>> [#uses=1]
  store <4 x float> %1169, <4 x float>* %__r.i83
  %1170 = load float* %c1.i81                     ; <float> [#uses=1]
  %1171 = insertelement <4 x float> undef, float %1170, i32 0 ; <<4 x float>> [#uses=2]
  %1172 = shufflevector <4 x float> %1171, <4 x float> %1171, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %1173 = load <4 x float>* %zi1.i65              ; <<4 x float>> [#uses=1]
  %1174 = fmul <4 x float> %1172, %1173           ; <<4 x float>> [#uses=1]
  %1175 = load float* %s1.i82                     ; <float> [#uses=1]
  %1176 = insertelement <4 x float> undef, float %1175, i32 0 ; <<4 x float>> [#uses=2]
  %1177 = shufflevector <4 x float> %1176, <4 x float> %1176, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %1178 = load <4 x float>* %zr1.i61              ; <<4 x float>> [#uses=1]
  %1179 = fmul <4 x float> %1177, %1178           ; <<4 x float>> [#uses=1]
  %1180 = fadd <4 x float> %1174, %1179           ; <<4 x float>> [#uses=1]
  store <4 x float> %1180, <4 x float>* %zi1.i65
  %1181 = load <4 x float>* %__r.i83              ; <<4 x float>> [#uses=1]
  store <4 x float> %1181, <4 x float>* %zr1.i61
  %1182 = load i32* %tbase.i80                    ; <i32> [#uses=1]
  %1183 = mul i32 %1182, 2                        ; <i32> [#uses=1]
  store i32 %1183, i32* %13
  store float* %c2.i84, float** %14
  %1184 = load i32* %13                           ; <i32> [#uses=1]
  %1185 = icmp sgt i32 %1184, 512                 ; <i1> [#uses=1]
  br i1 %1185, label %1186, label %k_sincos.exit6.i91

; <label>:1186                                    ; preds = %k_sincos.exit.i90
  %1187 = load i32* %13                           ; <i32> [#uses=1]
  %1188 = sub i32 %1187, 1024                     ; <i32> [#uses=1]
  store i32 %1188, i32* %13
  br label %k_sincos.exit6.i91

k_sincos.exit6.i91:                               ; preds = %1186, %k_sincos.exit.i90
  %1189 = load i32* %13                           ; <i32> [#uses=1]
  %1190 = sitofp i32 %1189 to float               ; <float> [#uses=1]
  %1191 = fmul float %1190, 0xBF7921FB60000000    ; <float> [#uses=1]
  store float %1191, float* %x.i5.i56
  %1192 = load float* %x.i5.i56                   ; <float> [#uses=1]
  %1193 = call float @_Z10native_cosf(float %1192) nounwind ; <float> [#uses=1]
  %1194 = load float** %14                        ; <float*> [#uses=1]
  store float %1193, float* %1194
  %1195 = load float* %x.i5.i56                   ; <float> [#uses=1]
  %1196 = call float @_Z10native_sinf(float %1195) nounwind ; <float> [#uses=1]
  store float %1196, float* %12
  %1197 = load float* %12                         ; <float> [#uses=1]
  store float %1197, float* %s2.i85
  %1198 = load float* %c2.i84                     ; <float> [#uses=1]
  %1199 = insertelement <4 x float> undef, float %1198, i32 0 ; <<4 x float>> [#uses=2]
  %1200 = shufflevector <4 x float> %1199, <4 x float> %1199, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %1201 = load <4 x float>* %zr2.i62              ; <<4 x float>> [#uses=1]
  %1202 = fmul <4 x float> %1200, %1201           ; <<4 x float>> [#uses=1]
  %1203 = load float* %s2.i85                     ; <float> [#uses=1]
  %1204 = insertelement <4 x float> undef, float %1203, i32 0 ; <<4 x float>> [#uses=2]
  %1205 = shufflevector <4 x float> %1204, <4 x float> %1204, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %1206 = load <4 x float>* %zi2.i66              ; <<4 x float>> [#uses=1]
  %1207 = fmul <4 x float> %1205, %1206           ; <<4 x float>> [#uses=1]
  %1208 = fsub <4 x float> %1202, %1207           ; <<4 x float>> [#uses=1]
  store <4 x float> %1208, <4 x float>* %__r1.i86
  %1209 = load float* %c2.i84                     ; <float> [#uses=1]
  %1210 = insertelement <4 x float> undef, float %1209, i32 0 ; <<4 x float>> [#uses=2]
  %1211 = shufflevector <4 x float> %1210, <4 x float> %1210, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %1212 = load <4 x float>* %zi2.i66              ; <<4 x float>> [#uses=1]
  %1213 = fmul <4 x float> %1211, %1212           ; <<4 x float>> [#uses=1]
  %1214 = load float* %s2.i85                     ; <float> [#uses=1]
  %1215 = insertelement <4 x float> undef, float %1214, i32 0 ; <<4 x float>> [#uses=2]
  %1216 = shufflevector <4 x float> %1215, <4 x float> %1215, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %1217 = load <4 x float>* %zr2.i62              ; <<4 x float>> [#uses=1]
  %1218 = fmul <4 x float> %1216, %1217           ; <<4 x float>> [#uses=1]
  %1219 = fadd <4 x float> %1213, %1218           ; <<4 x float>> [#uses=1]
  store <4 x float> %1219, <4 x float>* %zi2.i66
  %1220 = load <4 x float>* %__r1.i86             ; <<4 x float>> [#uses=1]
  store <4 x float> %1220, <4 x float>* %zr2.i62
  %1221 = load i32* %tbase.i80                    ; <i32> [#uses=1]
  %1222 = mul i32 %1221, 3                        ; <i32> [#uses=1]
  store i32 %1222, i32* %16
  store float* %c3.i87, float** %17
  %1223 = load i32* %16                           ; <i32> [#uses=1]
  %1224 = icmp sgt i32 %1223, 512                 ; <i1> [#uses=1]
  br i1 %1224, label %1225, label %kfft_pass3.exit

; <label>:1225                                    ; preds = %k_sincos.exit6.i91
  %1226 = load i32* %16                           ; <i32> [#uses=1]
  %1227 = sub i32 %1226, 1024                     ; <i32> [#uses=1]
  store i32 %1227, i32* %16
  br label %kfft_pass3.exit

kfft_pass3.exit:                                  ; preds = %k_sincos.exit6.i91, %1225
  %1228 = load i32* %16                           ; <i32> [#uses=1]
  %1229 = sitofp i32 %1228 to float               ; <float> [#uses=1]
  %1230 = fmul float %1229, 0xBF7921FB60000000    ; <float> [#uses=1]
  store float %1230, float* %x.i3.i57
  %1231 = load float* %x.i3.i57                   ; <float> [#uses=1]
  %1232 = call float @_Z10native_cosf(float %1231) nounwind ; <float> [#uses=1]
  %1233 = load float** %17                        ; <float*> [#uses=1]
  store float %1232, float* %1233
  %1234 = load float* %x.i3.i57                   ; <float> [#uses=1]
  %1235 = call float @_Z10native_sinf(float %1234) nounwind ; <float> [#uses=1]
  store float %1235, float* %15
  %1236 = load float* %15                         ; <float> [#uses=1]
  store float %1236, float* %s3.i88
  %1237 = load float* %c3.i87                     ; <float> [#uses=1]
  %1238 = insertelement <4 x float> undef, float %1237, i32 0 ; <<4 x float>> [#uses=2]
  %1239 = shufflevector <4 x float> %1238, <4 x float> %1238, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %1240 = load <4 x float>* %zr3.i63              ; <<4 x float>> [#uses=1]
  %1241 = fmul <4 x float> %1239, %1240           ; <<4 x float>> [#uses=1]
  %1242 = load float* %s3.i88                     ; <float> [#uses=1]
  %1243 = insertelement <4 x float> undef, float %1242, i32 0 ; <<4 x float>> [#uses=2]
  %1244 = shufflevector <4 x float> %1243, <4 x float> %1243, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %1245 = load <4 x float>* %zi3.i67              ; <<4 x float>> [#uses=1]
  %1246 = fmul <4 x float> %1244, %1245           ; <<4 x float>> [#uses=1]
  %1247 = fsub <4 x float> %1241, %1246           ; <<4 x float>> [#uses=1]
  store <4 x float> %1247, <4 x float>* %__r2.i89
  %1248 = load float* %c3.i87                     ; <float> [#uses=1]
  %1249 = insertelement <4 x float> undef, float %1248, i32 0 ; <<4 x float>> [#uses=2]
  %1250 = shufflevector <4 x float> %1249, <4 x float> %1249, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %1251 = load <4 x float>* %zi3.i67              ; <<4 x float>> [#uses=1]
  %1252 = fmul <4 x float> %1250, %1251           ; <<4 x float>> [#uses=1]
  %1253 = load float* %s3.i88                     ; <float> [#uses=1]
  %1254 = insertelement <4 x float> undef, float %1253, i32 0 ; <<4 x float>> [#uses=2]
  %1255 = shufflevector <4 x float> %1254, <4 x float> %1254, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %1256 = load <4 x float>* %zr3.i63              ; <<4 x float>> [#uses=1]
  %1257 = fmul <4 x float> %1255, %1256           ; <<4 x float>> [#uses=1]
  %1258 = fadd <4 x float> %1252, %1257           ; <<4 x float>> [#uses=1]
  store <4 x float> %1258, <4 x float>* %zi3.i67
  %1259 = load <4 x float>* %__r2.i89             ; <<4 x float>> [#uses=1]
  store <4 x float> %1259, <4 x float>* %zr3.i63
  call void @barrier(i32 1) nounwind
  %1260 = load float addrspace(3)** %22           ; <float addrspace(3)*> [#uses=1]
  %1261 = load i32* %21                           ; <i32> [#uses=1]
  %1262 = getelementptr inbounds float addrspace(3)* %1260, i32 %1261 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %1262, float addrspace(3)** %lp.i59
  %1263 = load <4 x float>* %zr0.i60              ; <<4 x float>> [#uses=1]
  %1264 = extractelement <4 x float> %1263, i32 0 ; <float> [#uses=1]
  %1265 = load float addrspace(3)** %lp.i59       ; <float addrspace(3)*> [#uses=1]
  %1266 = getelementptr inbounds float addrspace(3)* %1265, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %1264, float addrspace(3)* %1266
  %1267 = load <4 x float>* %zr0.i60              ; <<4 x float>> [#uses=1]
  %1268 = extractelement <4 x float> %1267, i32 1 ; <float> [#uses=1]
  %1269 = load float addrspace(3)** %lp.i59       ; <float addrspace(3)*> [#uses=1]
  %1270 = getelementptr inbounds float addrspace(3)* %1269, i32 66 ; <float addrspace(3)*> [#uses=1]
  store float %1268, float addrspace(3)* %1270
  %1271 = load <4 x float>* %zr0.i60              ; <<4 x float>> [#uses=1]
  %1272 = extractelement <4 x float> %1271, i32 2 ; <float> [#uses=1]
  %1273 = load float addrspace(3)** %lp.i59       ; <float addrspace(3)*> [#uses=1]
  %1274 = getelementptr inbounds float addrspace(3)* %1273, i32 132 ; <float addrspace(3)*> [#uses=1]
  store float %1272, float addrspace(3)* %1274
  %1275 = load <4 x float>* %zr0.i60              ; <<4 x float>> [#uses=1]
  %1276 = extractelement <4 x float> %1275, i32 3 ; <float> [#uses=1]
  %1277 = load float addrspace(3)** %lp.i59       ; <float addrspace(3)*> [#uses=1]
  %1278 = getelementptr inbounds float addrspace(3)* %1277, i32 198 ; <float addrspace(3)*> [#uses=1]
  store float %1276, float addrspace(3)* %1278
  %1279 = load float addrspace(3)** %lp.i59       ; <float addrspace(3)*> [#uses=1]
  %1280 = getelementptr inbounds float addrspace(3)* %1279, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %1280, float addrspace(3)** %lp.i59
  %1281 = load <4 x float>* %zr1.i61              ; <<4 x float>> [#uses=1]
  %1282 = extractelement <4 x float> %1281, i32 0 ; <float> [#uses=1]
  %1283 = load float addrspace(3)** %lp.i59       ; <float addrspace(3)*> [#uses=1]
  %1284 = getelementptr inbounds float addrspace(3)* %1283, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %1282, float addrspace(3)* %1284
  %1285 = load <4 x float>* %zr1.i61              ; <<4 x float>> [#uses=1]
  %1286 = extractelement <4 x float> %1285, i32 1 ; <float> [#uses=1]
  %1287 = load float addrspace(3)** %lp.i59       ; <float addrspace(3)*> [#uses=1]
  %1288 = getelementptr inbounds float addrspace(3)* %1287, i32 66 ; <float addrspace(3)*> [#uses=1]
  store float %1286, float addrspace(3)* %1288
  %1289 = load <4 x float>* %zr1.i61              ; <<4 x float>> [#uses=1]
  %1290 = extractelement <4 x float> %1289, i32 2 ; <float> [#uses=1]
  %1291 = load float addrspace(3)** %lp.i59       ; <float addrspace(3)*> [#uses=1]
  %1292 = getelementptr inbounds float addrspace(3)* %1291, i32 132 ; <float addrspace(3)*> [#uses=1]
  store float %1290, float addrspace(3)* %1292
  %1293 = load <4 x float>* %zr1.i61              ; <<4 x float>> [#uses=1]
  %1294 = extractelement <4 x float> %1293, i32 3 ; <float> [#uses=1]
  %1295 = load float addrspace(3)** %lp.i59       ; <float addrspace(3)*> [#uses=1]
  %1296 = getelementptr inbounds float addrspace(3)* %1295, i32 198 ; <float addrspace(3)*> [#uses=1]
  store float %1294, float addrspace(3)* %1296
  %1297 = load float addrspace(3)** %lp.i59       ; <float addrspace(3)*> [#uses=1]
  %1298 = getelementptr inbounds float addrspace(3)* %1297, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %1298, float addrspace(3)** %lp.i59
  %1299 = load <4 x float>* %zr2.i62              ; <<4 x float>> [#uses=1]
  %1300 = extractelement <4 x float> %1299, i32 0 ; <float> [#uses=1]
  %1301 = load float addrspace(3)** %lp.i59       ; <float addrspace(3)*> [#uses=1]
  %1302 = getelementptr inbounds float addrspace(3)* %1301, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %1300, float addrspace(3)* %1302
  %1303 = load <4 x float>* %zr2.i62              ; <<4 x float>> [#uses=1]
  %1304 = extractelement <4 x float> %1303, i32 1 ; <float> [#uses=1]
  %1305 = load float addrspace(3)** %lp.i59       ; <float addrspace(3)*> [#uses=1]
  %1306 = getelementptr inbounds float addrspace(3)* %1305, i32 66 ; <float addrspace(3)*> [#uses=1]
  store float %1304, float addrspace(3)* %1306
  %1307 = load <4 x float>* %zr2.i62              ; <<4 x float>> [#uses=1]
  %1308 = extractelement <4 x float> %1307, i32 2 ; <float> [#uses=1]
  %1309 = load float addrspace(3)** %lp.i59       ; <float addrspace(3)*> [#uses=1]
  %1310 = getelementptr inbounds float addrspace(3)* %1309, i32 132 ; <float addrspace(3)*> [#uses=1]
  store float %1308, float addrspace(3)* %1310
  %1311 = load <4 x float>* %zr2.i62              ; <<4 x float>> [#uses=1]
  %1312 = extractelement <4 x float> %1311, i32 3 ; <float> [#uses=1]
  %1313 = load float addrspace(3)** %lp.i59       ; <float addrspace(3)*> [#uses=1]
  %1314 = getelementptr inbounds float addrspace(3)* %1313, i32 198 ; <float addrspace(3)*> [#uses=1]
  store float %1312, float addrspace(3)* %1314
  %1315 = load float addrspace(3)** %lp.i59       ; <float addrspace(3)*> [#uses=1]
  %1316 = getelementptr inbounds float addrspace(3)* %1315, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %1316, float addrspace(3)** %lp.i59
  %1317 = load <4 x float>* %zr3.i63              ; <<4 x float>> [#uses=1]
  %1318 = extractelement <4 x float> %1317, i32 0 ; <float> [#uses=1]
  %1319 = load float addrspace(3)** %lp.i59       ; <float addrspace(3)*> [#uses=1]
  %1320 = getelementptr inbounds float addrspace(3)* %1319, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %1318, float addrspace(3)* %1320
  %1321 = load <4 x float>* %zr3.i63              ; <<4 x float>> [#uses=1]
  %1322 = extractelement <4 x float> %1321, i32 1 ; <float> [#uses=1]
  %1323 = load float addrspace(3)** %lp.i59       ; <float addrspace(3)*> [#uses=1]
  %1324 = getelementptr inbounds float addrspace(3)* %1323, i32 66 ; <float addrspace(3)*> [#uses=1]
  store float %1322, float addrspace(3)* %1324
  %1325 = load <4 x float>* %zr3.i63              ; <<4 x float>> [#uses=1]
  %1326 = extractelement <4 x float> %1325, i32 2 ; <float> [#uses=1]
  %1327 = load float addrspace(3)** %lp.i59       ; <float addrspace(3)*> [#uses=1]
  %1328 = getelementptr inbounds float addrspace(3)* %1327, i32 132 ; <float addrspace(3)*> [#uses=1]
  store float %1326, float addrspace(3)* %1328
  %1329 = load <4 x float>* %zr3.i63              ; <<4 x float>> [#uses=1]
  %1330 = extractelement <4 x float> %1329, i32 3 ; <float> [#uses=1]
  %1331 = load float addrspace(3)** %lp.i59       ; <float addrspace(3)*> [#uses=1]
  %1332 = getelementptr inbounds float addrspace(3)* %1331, i32 198 ; <float addrspace(3)*> [#uses=1]
  store float %1330, float addrspace(3)* %1332
  %1333 = load float addrspace(3)** %lp.i59       ; <float addrspace(3)*> [#uses=1]
  %1334 = getelementptr inbounds float addrspace(3)* %1333, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %1334, float addrspace(3)** %lp.i59
  %1335 = load <4 x float>* %zi0.i64              ; <<4 x float>> [#uses=1]
  %1336 = extractelement <4 x float> %1335, i32 0 ; <float> [#uses=1]
  %1337 = load float addrspace(3)** %lp.i59       ; <float addrspace(3)*> [#uses=1]
  %1338 = getelementptr inbounds float addrspace(3)* %1337, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %1336, float addrspace(3)* %1338
  %1339 = load <4 x float>* %zi0.i64              ; <<4 x float>> [#uses=1]
  %1340 = extractelement <4 x float> %1339, i32 1 ; <float> [#uses=1]
  %1341 = load float addrspace(3)** %lp.i59       ; <float addrspace(3)*> [#uses=1]
  %1342 = getelementptr inbounds float addrspace(3)* %1341, i32 66 ; <float addrspace(3)*> [#uses=1]
  store float %1340, float addrspace(3)* %1342
  %1343 = load <4 x float>* %zi0.i64              ; <<4 x float>> [#uses=1]
  %1344 = extractelement <4 x float> %1343, i32 2 ; <float> [#uses=1]
  %1345 = load float addrspace(3)** %lp.i59       ; <float addrspace(3)*> [#uses=1]
  %1346 = getelementptr inbounds float addrspace(3)* %1345, i32 132 ; <float addrspace(3)*> [#uses=1]
  store float %1344, float addrspace(3)* %1346
  %1347 = load <4 x float>* %zi0.i64              ; <<4 x float>> [#uses=1]
  %1348 = extractelement <4 x float> %1347, i32 3 ; <float> [#uses=1]
  %1349 = load float addrspace(3)** %lp.i59       ; <float addrspace(3)*> [#uses=1]
  %1350 = getelementptr inbounds float addrspace(3)* %1349, i32 198 ; <float addrspace(3)*> [#uses=1]
  store float %1348, float addrspace(3)* %1350
  %1351 = load float addrspace(3)** %lp.i59       ; <float addrspace(3)*> [#uses=1]
  %1352 = getelementptr inbounds float addrspace(3)* %1351, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %1352, float addrspace(3)** %lp.i59
  %1353 = load <4 x float>* %zi1.i65              ; <<4 x float>> [#uses=1]
  %1354 = extractelement <4 x float> %1353, i32 0 ; <float> [#uses=1]
  %1355 = load float addrspace(3)** %lp.i59       ; <float addrspace(3)*> [#uses=1]
  %1356 = getelementptr inbounds float addrspace(3)* %1355, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %1354, float addrspace(3)* %1356
  %1357 = load <4 x float>* %zi1.i65              ; <<4 x float>> [#uses=1]
  %1358 = extractelement <4 x float> %1357, i32 1 ; <float> [#uses=1]
  %1359 = load float addrspace(3)** %lp.i59       ; <float addrspace(3)*> [#uses=1]
  %1360 = getelementptr inbounds float addrspace(3)* %1359, i32 66 ; <float addrspace(3)*> [#uses=1]
  store float %1358, float addrspace(3)* %1360
  %1361 = load <4 x float>* %zi1.i65              ; <<4 x float>> [#uses=1]
  %1362 = extractelement <4 x float> %1361, i32 2 ; <float> [#uses=1]
  %1363 = load float addrspace(3)** %lp.i59       ; <float addrspace(3)*> [#uses=1]
  %1364 = getelementptr inbounds float addrspace(3)* %1363, i32 132 ; <float addrspace(3)*> [#uses=1]
  store float %1362, float addrspace(3)* %1364
  %1365 = load <4 x float>* %zi1.i65              ; <<4 x float>> [#uses=1]
  %1366 = extractelement <4 x float> %1365, i32 3 ; <float> [#uses=1]
  %1367 = load float addrspace(3)** %lp.i59       ; <float addrspace(3)*> [#uses=1]
  %1368 = getelementptr inbounds float addrspace(3)* %1367, i32 198 ; <float addrspace(3)*> [#uses=1]
  store float %1366, float addrspace(3)* %1368
  %1369 = load float addrspace(3)** %lp.i59       ; <float addrspace(3)*> [#uses=1]
  %1370 = getelementptr inbounds float addrspace(3)* %1369, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %1370, float addrspace(3)** %lp.i59
  %1371 = load <4 x float>* %zi2.i66              ; <<4 x float>> [#uses=1]
  %1372 = extractelement <4 x float> %1371, i32 0 ; <float> [#uses=1]
  %1373 = load float addrspace(3)** %lp.i59       ; <float addrspace(3)*> [#uses=1]
  %1374 = getelementptr inbounds float addrspace(3)* %1373, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %1372, float addrspace(3)* %1374
  %1375 = load <4 x float>* %zi2.i66              ; <<4 x float>> [#uses=1]
  %1376 = extractelement <4 x float> %1375, i32 1 ; <float> [#uses=1]
  %1377 = load float addrspace(3)** %lp.i59       ; <float addrspace(3)*> [#uses=1]
  %1378 = getelementptr inbounds float addrspace(3)* %1377, i32 66 ; <float addrspace(3)*> [#uses=1]
  store float %1376, float addrspace(3)* %1378
  %1379 = load <4 x float>* %zi2.i66              ; <<4 x float>> [#uses=1]
  %1380 = extractelement <4 x float> %1379, i32 2 ; <float> [#uses=1]
  %1381 = load float addrspace(3)** %lp.i59       ; <float addrspace(3)*> [#uses=1]
  %1382 = getelementptr inbounds float addrspace(3)* %1381, i32 132 ; <float addrspace(3)*> [#uses=1]
  store float %1380, float addrspace(3)* %1382
  %1383 = load <4 x float>* %zi2.i66              ; <<4 x float>> [#uses=1]
  %1384 = extractelement <4 x float> %1383, i32 3 ; <float> [#uses=1]
  %1385 = load float addrspace(3)** %lp.i59       ; <float addrspace(3)*> [#uses=1]
  %1386 = getelementptr inbounds float addrspace(3)* %1385, i32 198 ; <float addrspace(3)*> [#uses=1]
  store float %1384, float addrspace(3)* %1386
  %1387 = load float addrspace(3)** %lp.i59       ; <float addrspace(3)*> [#uses=1]
  %1388 = getelementptr inbounds float addrspace(3)* %1387, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %1388, float addrspace(3)** %lp.i59
  %1389 = load <4 x float>* %zi3.i67              ; <<4 x float>> [#uses=1]
  %1390 = extractelement <4 x float> %1389, i32 0 ; <float> [#uses=1]
  %1391 = load float addrspace(3)** %lp.i59       ; <float addrspace(3)*> [#uses=1]
  %1392 = getelementptr inbounds float addrspace(3)* %1391, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %1390, float addrspace(3)* %1392
  %1393 = load <4 x float>* %zi3.i67              ; <<4 x float>> [#uses=1]
  %1394 = extractelement <4 x float> %1393, i32 1 ; <float> [#uses=1]
  %1395 = load float addrspace(3)** %lp.i59       ; <float addrspace(3)*> [#uses=1]
  %1396 = getelementptr inbounds float addrspace(3)* %1395, i32 66 ; <float addrspace(3)*> [#uses=1]
  store float %1394, float addrspace(3)* %1396
  %1397 = load <4 x float>* %zi3.i67              ; <<4 x float>> [#uses=1]
  %1398 = extractelement <4 x float> %1397, i32 2 ; <float> [#uses=1]
  %1399 = load float addrspace(3)** %lp.i59       ; <float addrspace(3)*> [#uses=1]
  %1400 = getelementptr inbounds float addrspace(3)* %1399, i32 132 ; <float addrspace(3)*> [#uses=1]
  store float %1398, float addrspace(3)* %1400
  %1401 = load <4 x float>* %zi3.i67              ; <<4 x float>> [#uses=1]
  %1402 = extractelement <4 x float> %1401, i32 3 ; <float> [#uses=1]
  %1403 = load float addrspace(3)** %lp.i59       ; <float addrspace(3)*> [#uses=1]
  %1404 = getelementptr inbounds float addrspace(3)* %1403, i32 198 ; <float addrspace(3)*> [#uses=1]
  store float %1402, float addrspace(3)* %1404
  call void @barrier(i32 1) nounwind
  %1405 = load i32* %me                           ; <i32> [#uses=1]
  store i32 %1405, i32* %32
  store float addrspace(3)* getelementptr inbounds ([2176 x float] addrspace(3)* @opencl_kfft_local_lds, i32 0, i32 0), float addrspace(3)** %33
  %1406 = load float addrspace(3)** %33           ; <float addrspace(3)*> [#uses=1]
  %1407 = load i32* %32                           ; <i32> [#uses=1]
  %1408 = and i32 %1407, 3                        ; <i32> [#uses=1]
  %1409 = load i32* %32                           ; <i32> [#uses=1]
  %1410 = lshr i32 %1409, 2                       ; <i32> [#uses=1]
  %1411 = and i32 %1410, 3                        ; <i32> [#uses=1]
  %1412 = mul i32 %1411, 264                      ; <i32> [#uses=1]
  %1413 = add i32 %1408, %1412                    ; <i32> [#uses=1]
  %1414 = load i32* %32                           ; <i32> [#uses=1]
  %1415 = lshr i32 %1414, 4                       ; <i32> [#uses=1]
  %1416 = shl i32 %1415, 2                        ; <i32> [#uses=1]
  %1417 = add i32 %1413, %1416                    ; <i32> [#uses=1]
  %1418 = getelementptr inbounds float addrspace(3)* %1406, i32 %1417 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %1418, float addrspace(3)** %lp.i25
  %1419 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1420 = getelementptr inbounds float addrspace(3)* %1419, i32 0 ; <float addrspace(3)*> [#uses=1]
  %1421 = load float addrspace(3)* %1420          ; <float> [#uses=1]
  %1422 = load <4 x float>* %zr0.i26              ; <<4 x float>> [#uses=1]
  %1423 = insertelement <4 x float> %1422, float %1421, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %1423, <4 x float>* %zr0.i26
  %1424 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1425 = getelementptr inbounds float addrspace(3)* %1424, i32 66 ; <float addrspace(3)*> [#uses=1]
  %1426 = load float addrspace(3)* %1425          ; <float> [#uses=1]
  %1427 = load <4 x float>* %zr0.i26              ; <<4 x float>> [#uses=1]
  %1428 = insertelement <4 x float> %1427, float %1426, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %1428, <4 x float>* %zr0.i26
  %1429 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1430 = getelementptr inbounds float addrspace(3)* %1429, i32 132 ; <float addrspace(3)*> [#uses=1]
  %1431 = load float addrspace(3)* %1430          ; <float> [#uses=1]
  %1432 = load <4 x float>* %zr0.i26              ; <<4 x float>> [#uses=1]
  %1433 = insertelement <4 x float> %1432, float %1431, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %1433, <4 x float>* %zr0.i26
  %1434 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1435 = getelementptr inbounds float addrspace(3)* %1434, i32 198 ; <float addrspace(3)*> [#uses=1]
  %1436 = load float addrspace(3)* %1435          ; <float> [#uses=1]
  %1437 = load <4 x float>* %zr0.i26              ; <<4 x float>> [#uses=1]
  %1438 = insertelement <4 x float> %1437, float %1436, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %1438, <4 x float>* %zr0.i26
  %1439 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1440 = getelementptr inbounds float addrspace(3)* %1439, i32 16 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %1440, float addrspace(3)** %lp.i25
  %1441 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1442 = getelementptr inbounds float addrspace(3)* %1441, i32 0 ; <float addrspace(3)*> [#uses=1]
  %1443 = load float addrspace(3)* %1442          ; <float> [#uses=1]
  %1444 = load <4 x float>* %zr1.i27              ; <<4 x float>> [#uses=1]
  %1445 = insertelement <4 x float> %1444, float %1443, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %1445, <4 x float>* %zr1.i27
  %1446 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1447 = getelementptr inbounds float addrspace(3)* %1446, i32 66 ; <float addrspace(3)*> [#uses=1]
  %1448 = load float addrspace(3)* %1447          ; <float> [#uses=1]
  %1449 = load <4 x float>* %zr1.i27              ; <<4 x float>> [#uses=1]
  %1450 = insertelement <4 x float> %1449, float %1448, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %1450, <4 x float>* %zr1.i27
  %1451 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1452 = getelementptr inbounds float addrspace(3)* %1451, i32 132 ; <float addrspace(3)*> [#uses=1]
  %1453 = load float addrspace(3)* %1452          ; <float> [#uses=1]
  %1454 = load <4 x float>* %zr1.i27              ; <<4 x float>> [#uses=1]
  %1455 = insertelement <4 x float> %1454, float %1453, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %1455, <4 x float>* %zr1.i27
  %1456 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1457 = getelementptr inbounds float addrspace(3)* %1456, i32 198 ; <float addrspace(3)*> [#uses=1]
  %1458 = load float addrspace(3)* %1457          ; <float> [#uses=1]
  %1459 = load <4 x float>* %zr1.i27              ; <<4 x float>> [#uses=1]
  %1460 = insertelement <4 x float> %1459, float %1458, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %1460, <4 x float>* %zr1.i27
  %1461 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1462 = getelementptr inbounds float addrspace(3)* %1461, i32 16 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %1462, float addrspace(3)** %lp.i25
  %1463 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1464 = getelementptr inbounds float addrspace(3)* %1463, i32 0 ; <float addrspace(3)*> [#uses=1]
  %1465 = load float addrspace(3)* %1464          ; <float> [#uses=1]
  %1466 = load <4 x float>* %zr2.i28              ; <<4 x float>> [#uses=1]
  %1467 = insertelement <4 x float> %1466, float %1465, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %1467, <4 x float>* %zr2.i28
  %1468 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1469 = getelementptr inbounds float addrspace(3)* %1468, i32 66 ; <float addrspace(3)*> [#uses=1]
  %1470 = load float addrspace(3)* %1469          ; <float> [#uses=1]
  %1471 = load <4 x float>* %zr2.i28              ; <<4 x float>> [#uses=1]
  %1472 = insertelement <4 x float> %1471, float %1470, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %1472, <4 x float>* %zr2.i28
  %1473 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1474 = getelementptr inbounds float addrspace(3)* %1473, i32 132 ; <float addrspace(3)*> [#uses=1]
  %1475 = load float addrspace(3)* %1474          ; <float> [#uses=1]
  %1476 = load <4 x float>* %zr2.i28              ; <<4 x float>> [#uses=1]
  %1477 = insertelement <4 x float> %1476, float %1475, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %1477, <4 x float>* %zr2.i28
  %1478 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1479 = getelementptr inbounds float addrspace(3)* %1478, i32 198 ; <float addrspace(3)*> [#uses=1]
  %1480 = load float addrspace(3)* %1479          ; <float> [#uses=1]
  %1481 = load <4 x float>* %zr2.i28              ; <<4 x float>> [#uses=1]
  %1482 = insertelement <4 x float> %1481, float %1480, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %1482, <4 x float>* %zr2.i28
  %1483 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1484 = getelementptr inbounds float addrspace(3)* %1483, i32 16 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %1484, float addrspace(3)** %lp.i25
  %1485 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1486 = getelementptr inbounds float addrspace(3)* %1485, i32 0 ; <float addrspace(3)*> [#uses=1]
  %1487 = load float addrspace(3)* %1486          ; <float> [#uses=1]
  %1488 = load <4 x float>* %zr3.i29              ; <<4 x float>> [#uses=1]
  %1489 = insertelement <4 x float> %1488, float %1487, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %1489, <4 x float>* %zr3.i29
  %1490 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1491 = getelementptr inbounds float addrspace(3)* %1490, i32 66 ; <float addrspace(3)*> [#uses=1]
  %1492 = load float addrspace(3)* %1491          ; <float> [#uses=1]
  %1493 = load <4 x float>* %zr3.i29              ; <<4 x float>> [#uses=1]
  %1494 = insertelement <4 x float> %1493, float %1492, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %1494, <4 x float>* %zr3.i29
  %1495 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1496 = getelementptr inbounds float addrspace(3)* %1495, i32 132 ; <float addrspace(3)*> [#uses=1]
  %1497 = load float addrspace(3)* %1496          ; <float> [#uses=1]
  %1498 = load <4 x float>* %zr3.i29              ; <<4 x float>> [#uses=1]
  %1499 = insertelement <4 x float> %1498, float %1497, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %1499, <4 x float>* %zr3.i29
  %1500 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1501 = getelementptr inbounds float addrspace(3)* %1500, i32 198 ; <float addrspace(3)*> [#uses=1]
  %1502 = load float addrspace(3)* %1501          ; <float> [#uses=1]
  %1503 = load <4 x float>* %zr3.i29              ; <<4 x float>> [#uses=1]
  %1504 = insertelement <4 x float> %1503, float %1502, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %1504, <4 x float>* %zr3.i29
  %1505 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1506 = getelementptr inbounds float addrspace(3)* %1505, i32 1008 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %1506, float addrspace(3)** %lp.i25
  %1507 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1508 = getelementptr inbounds float addrspace(3)* %1507, i32 0 ; <float addrspace(3)*> [#uses=1]
  %1509 = load float addrspace(3)* %1508          ; <float> [#uses=1]
  %1510 = load <4 x float>* %zi0.i30              ; <<4 x float>> [#uses=1]
  %1511 = insertelement <4 x float> %1510, float %1509, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %1511, <4 x float>* %zi0.i30
  %1512 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1513 = getelementptr inbounds float addrspace(3)* %1512, i32 66 ; <float addrspace(3)*> [#uses=1]
  %1514 = load float addrspace(3)* %1513          ; <float> [#uses=1]
  %1515 = load <4 x float>* %zi0.i30              ; <<4 x float>> [#uses=1]
  %1516 = insertelement <4 x float> %1515, float %1514, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %1516, <4 x float>* %zi0.i30
  %1517 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1518 = getelementptr inbounds float addrspace(3)* %1517, i32 132 ; <float addrspace(3)*> [#uses=1]
  %1519 = load float addrspace(3)* %1518          ; <float> [#uses=1]
  %1520 = load <4 x float>* %zi0.i30              ; <<4 x float>> [#uses=1]
  %1521 = insertelement <4 x float> %1520, float %1519, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %1521, <4 x float>* %zi0.i30
  %1522 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1523 = getelementptr inbounds float addrspace(3)* %1522, i32 198 ; <float addrspace(3)*> [#uses=1]
  %1524 = load float addrspace(3)* %1523          ; <float> [#uses=1]
  %1525 = load <4 x float>* %zi0.i30              ; <<4 x float>> [#uses=1]
  %1526 = insertelement <4 x float> %1525, float %1524, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %1526, <4 x float>* %zi0.i30
  %1527 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1528 = getelementptr inbounds float addrspace(3)* %1527, i32 16 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %1528, float addrspace(3)** %lp.i25
  %1529 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1530 = getelementptr inbounds float addrspace(3)* %1529, i32 0 ; <float addrspace(3)*> [#uses=1]
  %1531 = load float addrspace(3)* %1530          ; <float> [#uses=1]
  %1532 = load <4 x float>* %zi1.i31              ; <<4 x float>> [#uses=1]
  %1533 = insertelement <4 x float> %1532, float %1531, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %1533, <4 x float>* %zi1.i31
  %1534 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1535 = getelementptr inbounds float addrspace(3)* %1534, i32 66 ; <float addrspace(3)*> [#uses=1]
  %1536 = load float addrspace(3)* %1535          ; <float> [#uses=1]
  %1537 = load <4 x float>* %zi1.i31              ; <<4 x float>> [#uses=1]
  %1538 = insertelement <4 x float> %1537, float %1536, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %1538, <4 x float>* %zi1.i31
  %1539 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1540 = getelementptr inbounds float addrspace(3)* %1539, i32 132 ; <float addrspace(3)*> [#uses=1]
  %1541 = load float addrspace(3)* %1540          ; <float> [#uses=1]
  %1542 = load <4 x float>* %zi1.i31              ; <<4 x float>> [#uses=1]
  %1543 = insertelement <4 x float> %1542, float %1541, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %1543, <4 x float>* %zi1.i31
  %1544 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1545 = getelementptr inbounds float addrspace(3)* %1544, i32 198 ; <float addrspace(3)*> [#uses=1]
  %1546 = load float addrspace(3)* %1545          ; <float> [#uses=1]
  %1547 = load <4 x float>* %zi1.i31              ; <<4 x float>> [#uses=1]
  %1548 = insertelement <4 x float> %1547, float %1546, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %1548, <4 x float>* %zi1.i31
  %1549 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1550 = getelementptr inbounds float addrspace(3)* %1549, i32 16 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %1550, float addrspace(3)** %lp.i25
  %1551 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1552 = getelementptr inbounds float addrspace(3)* %1551, i32 0 ; <float addrspace(3)*> [#uses=1]
  %1553 = load float addrspace(3)* %1552          ; <float> [#uses=1]
  %1554 = load <4 x float>* %zi2.i32              ; <<4 x float>> [#uses=1]
  %1555 = insertelement <4 x float> %1554, float %1553, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %1555, <4 x float>* %zi2.i32
  %1556 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1557 = getelementptr inbounds float addrspace(3)* %1556, i32 66 ; <float addrspace(3)*> [#uses=1]
  %1558 = load float addrspace(3)* %1557          ; <float> [#uses=1]
  %1559 = load <4 x float>* %zi2.i32              ; <<4 x float>> [#uses=1]
  %1560 = insertelement <4 x float> %1559, float %1558, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %1560, <4 x float>* %zi2.i32
  %1561 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1562 = getelementptr inbounds float addrspace(3)* %1561, i32 132 ; <float addrspace(3)*> [#uses=1]
  %1563 = load float addrspace(3)* %1562          ; <float> [#uses=1]
  %1564 = load <4 x float>* %zi2.i32              ; <<4 x float>> [#uses=1]
  %1565 = insertelement <4 x float> %1564, float %1563, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %1565, <4 x float>* %zi2.i32
  %1566 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1567 = getelementptr inbounds float addrspace(3)* %1566, i32 198 ; <float addrspace(3)*> [#uses=1]
  %1568 = load float addrspace(3)* %1567          ; <float> [#uses=1]
  %1569 = load <4 x float>* %zi2.i32              ; <<4 x float>> [#uses=1]
  %1570 = insertelement <4 x float> %1569, float %1568, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %1570, <4 x float>* %zi2.i32
  %1571 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1572 = getelementptr inbounds float addrspace(3)* %1571, i32 16 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %1572, float addrspace(3)** %lp.i25
  %1573 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1574 = getelementptr inbounds float addrspace(3)* %1573, i32 0 ; <float addrspace(3)*> [#uses=1]
  %1575 = load float addrspace(3)* %1574          ; <float> [#uses=1]
  %1576 = load <4 x float>* %zi3.i33              ; <<4 x float>> [#uses=1]
  %1577 = insertelement <4 x float> %1576, float %1575, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %1577, <4 x float>* %zi3.i33
  %1578 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1579 = getelementptr inbounds float addrspace(3)* %1578, i32 66 ; <float addrspace(3)*> [#uses=1]
  %1580 = load float addrspace(3)* %1579          ; <float> [#uses=1]
  %1581 = load <4 x float>* %zi3.i33              ; <<4 x float>> [#uses=1]
  %1582 = insertelement <4 x float> %1581, float %1580, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %1582, <4 x float>* %zi3.i33
  %1583 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1584 = getelementptr inbounds float addrspace(3)* %1583, i32 132 ; <float addrspace(3)*> [#uses=1]
  %1585 = load float addrspace(3)* %1584          ; <float> [#uses=1]
  %1586 = load <4 x float>* %zi3.i33              ; <<4 x float>> [#uses=1]
  %1587 = insertelement <4 x float> %1586, float %1585, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %1587, <4 x float>* %zi3.i33
  %1588 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1589 = getelementptr inbounds float addrspace(3)* %1588, i32 198 ; <float addrspace(3)*> [#uses=1]
  %1590 = load float addrspace(3)* %1589          ; <float> [#uses=1]
  %1591 = load <4 x float>* %zi3.i33              ; <<4 x float>> [#uses=1]
  %1592 = insertelement <4 x float> %1591, float %1590, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %1592, <4 x float>* %zi3.i33
  %1593 = load <4 x float>* %zr0.i26              ; <<4 x float>> [#uses=1]
  %1594 = load <4 x float>* %zr2.i28              ; <<4 x float>> [#uses=1]
  %1595 = fadd <4 x float> %1593, %1594           ; <<4 x float>> [#uses=1]
  store <4 x float> %1595, <4 x float>* %ar0.i34
  %1596 = load <4 x float>* %zr1.i27              ; <<4 x float>> [#uses=1]
  %1597 = load <4 x float>* %zr3.i29              ; <<4 x float>> [#uses=1]
  %1598 = fadd <4 x float> %1596, %1597           ; <<4 x float>> [#uses=1]
  store <4 x float> %1598, <4 x float>* %ar2.i35
  %1599 = load <4 x float>* %ar0.i34              ; <<4 x float>> [#uses=1]
  %1600 = load <4 x float>* %ar2.i35              ; <<4 x float>> [#uses=1]
  %1601 = fadd <4 x float> %1599, %1600           ; <<4 x float>> [#uses=1]
  store <4 x float> %1601, <4 x float>* %br0.i36
  %1602 = load <4 x float>* %zr0.i26              ; <<4 x float>> [#uses=1]
  %1603 = load <4 x float>* %zr2.i28              ; <<4 x float>> [#uses=1]
  %1604 = fsub <4 x float> %1602, %1603           ; <<4 x float>> [#uses=1]
  store <4 x float> %1604, <4 x float>* %br1.i37
  %1605 = load <4 x float>* %ar0.i34              ; <<4 x float>> [#uses=1]
  %1606 = load <4 x float>* %ar2.i35              ; <<4 x float>> [#uses=1]
  %1607 = fsub <4 x float> %1605, %1606           ; <<4 x float>> [#uses=1]
  store <4 x float> %1607, <4 x float>* %br2.i38
  %1608 = load <4 x float>* %zr1.i27              ; <<4 x float>> [#uses=1]
  %1609 = load <4 x float>* %zr3.i29              ; <<4 x float>> [#uses=1]
  %1610 = fsub <4 x float> %1608, %1609           ; <<4 x float>> [#uses=1]
  store <4 x float> %1610, <4 x float>* %br3.i39
  %1611 = load <4 x float>* %zi0.i30              ; <<4 x float>> [#uses=1]
  %1612 = load <4 x float>* %zi2.i32              ; <<4 x float>> [#uses=1]
  %1613 = fadd <4 x float> %1611, %1612           ; <<4 x float>> [#uses=1]
  store <4 x float> %1613, <4 x float>* %ai0.i40
  %1614 = load <4 x float>* %zi1.i31              ; <<4 x float>> [#uses=1]
  %1615 = load <4 x float>* %zi3.i33              ; <<4 x float>> [#uses=1]
  %1616 = fadd <4 x float> %1614, %1615           ; <<4 x float>> [#uses=1]
  store <4 x float> %1616, <4 x float>* %ai2.i41
  %1617 = load <4 x float>* %ai0.i40              ; <<4 x float>> [#uses=1]
  %1618 = load <4 x float>* %ai2.i41              ; <<4 x float>> [#uses=1]
  %1619 = fadd <4 x float> %1617, %1618           ; <<4 x float>> [#uses=1]
  store <4 x float> %1619, <4 x float>* %bi0.i42
  %1620 = load <4 x float>* %zi0.i30              ; <<4 x float>> [#uses=1]
  %1621 = load <4 x float>* %zi2.i32              ; <<4 x float>> [#uses=1]
  %1622 = fsub <4 x float> %1620, %1621           ; <<4 x float>> [#uses=1]
  store <4 x float> %1622, <4 x float>* %bi1.i43
  %1623 = load <4 x float>* %ai0.i40              ; <<4 x float>> [#uses=1]
  %1624 = load <4 x float>* %ai2.i41              ; <<4 x float>> [#uses=1]
  %1625 = fsub <4 x float> %1623, %1624           ; <<4 x float>> [#uses=1]
  store <4 x float> %1625, <4 x float>* %bi2.i44
  %1626 = load <4 x float>* %zi1.i31              ; <<4 x float>> [#uses=1]
  %1627 = load <4 x float>* %zi3.i33              ; <<4 x float>> [#uses=1]
  %1628 = fsub <4 x float> %1626, %1627           ; <<4 x float>> [#uses=1]
  store <4 x float> %1628, <4 x float>* %bi3.i45
  %1629 = load <4 x float>* %br0.i36              ; <<4 x float>> [#uses=1]
  store <4 x float> %1629, <4 x float>* %zr0.i26
  %1630 = load <4 x float>* %bi0.i42              ; <<4 x float>> [#uses=1]
  store <4 x float> %1630, <4 x float>* %zi0.i30
  %1631 = load <4 x float>* %br1.i37              ; <<4 x float>> [#uses=1]
  %1632 = load <4 x float>* %bi3.i45              ; <<4 x float>> [#uses=1]
  %1633 = fadd <4 x float> %1631, %1632           ; <<4 x float>> [#uses=1]
  store <4 x float> %1633, <4 x float>* %zr1.i27
  %1634 = load <4 x float>* %bi1.i43              ; <<4 x float>> [#uses=1]
  %1635 = load <4 x float>* %br3.i39              ; <<4 x float>> [#uses=1]
  %1636 = fsub <4 x float> %1634, %1635           ; <<4 x float>> [#uses=1]
  store <4 x float> %1636, <4 x float>* %zi1.i31
  %1637 = load <4 x float>* %br1.i37              ; <<4 x float>> [#uses=1]
  %1638 = load <4 x float>* %bi3.i45              ; <<4 x float>> [#uses=1]
  %1639 = fsub <4 x float> %1637, %1638           ; <<4 x float>> [#uses=1]
  store <4 x float> %1639, <4 x float>* %zr3.i29
  %1640 = load <4 x float>* %br3.i39              ; <<4 x float>> [#uses=1]
  %1641 = load <4 x float>* %bi1.i43              ; <<4 x float>> [#uses=1]
  %1642 = fadd <4 x float> %1640, %1641           ; <<4 x float>> [#uses=1]
  store <4 x float> %1642, <4 x float>* %zi3.i33
  %1643 = load <4 x float>* %br2.i38              ; <<4 x float>> [#uses=1]
  store <4 x float> %1643, <4 x float>* %zr2.i28
  %1644 = load <4 x float>* %bi2.i44              ; <<4 x float>> [#uses=1]
  store <4 x float> %1644, <4 x float>* %zi2.i32
  %1645 = load i32* %32                           ; <i32> [#uses=1]
  %1646 = lshr i32 %1645, 4                       ; <i32> [#uses=1]
  %1647 = shl i32 %1646, 6                        ; <i32> [#uses=1]
  store i32 %1647, i32* %tbase.i46
  %1648 = load i32* %tbase.i46                    ; <i32> [#uses=1]
  %1649 = mul i32 %1648, 1                        ; <i32> [#uses=1]
  store i32 %1649, i32* %30
  store float* %c1.i47, float** %31
  %1650 = load i32* %30                           ; <i32> [#uses=1]
  %1651 = icmp sgt i32 %1650, 512                 ; <i1> [#uses=1]
  br i1 %1651, label %1652, label %k_sincos.exit.i

; <label>:1652                                    ; preds = %kfft_pass3.exit
  %1653 = load i32* %30                           ; <i32> [#uses=1]
  %1654 = sub i32 %1653, 1024                     ; <i32> [#uses=1]
  store i32 %1654, i32* %30
  br label %k_sincos.exit.i

k_sincos.exit.i:                                  ; preds = %1652, %kfft_pass3.exit
  %1655 = load i32* %30                           ; <i32> [#uses=1]
  %1656 = sitofp i32 %1655 to float               ; <float> [#uses=1]
  %1657 = fmul float %1656, 0xBF7921FB60000000    ; <float> [#uses=1]
  store float %1657, float* %x.i.i24
  %1658 = load float* %x.i.i24                    ; <float> [#uses=1]
  %1659 = call float @_Z10native_cosf(float %1658) nounwind ; <float> [#uses=1]
  %1660 = load float** %31                        ; <float*> [#uses=1]
  store float %1659, float* %1660
  %1661 = load float* %x.i.i24                    ; <float> [#uses=1]
  %1662 = call float @_Z10native_sinf(float %1661) nounwind ; <float> [#uses=1]
  store float %1662, float* %29
  %1663 = load float* %29                         ; <float> [#uses=1]
  store float %1663, float* %s1.i48
  %1664 = load float* %c1.i47                     ; <float> [#uses=1]
  %1665 = insertelement <4 x float> undef, float %1664, i32 0 ; <<4 x float>> [#uses=2]
  %1666 = shufflevector <4 x float> %1665, <4 x float> %1665, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %1667 = load <4 x float>* %zr1.i27              ; <<4 x float>> [#uses=1]
  %1668 = fmul <4 x float> %1666, %1667           ; <<4 x float>> [#uses=1]
  %1669 = load float* %s1.i48                     ; <float> [#uses=1]
  %1670 = insertelement <4 x float> undef, float %1669, i32 0 ; <<4 x float>> [#uses=2]
  %1671 = shufflevector <4 x float> %1670, <4 x float> %1670, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %1672 = load <4 x float>* %zi1.i31              ; <<4 x float>> [#uses=1]
  %1673 = fmul <4 x float> %1671, %1672           ; <<4 x float>> [#uses=1]
  %1674 = fsub <4 x float> %1668, %1673           ; <<4 x float>> [#uses=1]
  store <4 x float> %1674, <4 x float>* %__r.i49
  %1675 = load float* %c1.i47                     ; <float> [#uses=1]
  %1676 = insertelement <4 x float> undef, float %1675, i32 0 ; <<4 x float>> [#uses=2]
  %1677 = shufflevector <4 x float> %1676, <4 x float> %1676, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %1678 = load <4 x float>* %zi1.i31              ; <<4 x float>> [#uses=1]
  %1679 = fmul <4 x float> %1677, %1678           ; <<4 x float>> [#uses=1]
  %1680 = load float* %s1.i48                     ; <float> [#uses=1]
  %1681 = insertelement <4 x float> undef, float %1680, i32 0 ; <<4 x float>> [#uses=2]
  %1682 = shufflevector <4 x float> %1681, <4 x float> %1681, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %1683 = load <4 x float>* %zr1.i27              ; <<4 x float>> [#uses=1]
  %1684 = fmul <4 x float> %1682, %1683           ; <<4 x float>> [#uses=1]
  %1685 = fadd <4 x float> %1679, %1684           ; <<4 x float>> [#uses=1]
  store <4 x float> %1685, <4 x float>* %zi1.i31
  %1686 = load <4 x float>* %__r.i49              ; <<4 x float>> [#uses=1]
  store <4 x float> %1686, <4 x float>* %zr1.i27
  %1687 = load i32* %tbase.i46                    ; <i32> [#uses=1]
  %1688 = mul i32 %1687, 2                        ; <i32> [#uses=1]
  store i32 %1688, i32* %24
  store float* %c2.i50, float** %25
  %1689 = load i32* %24                           ; <i32> [#uses=1]
  %1690 = icmp sgt i32 %1689, 512                 ; <i1> [#uses=1]
  br i1 %1690, label %1691, label %k_sincos.exit6.i

; <label>:1691                                    ; preds = %k_sincos.exit.i
  %1692 = load i32* %24                           ; <i32> [#uses=1]
  %1693 = sub i32 %1692, 1024                     ; <i32> [#uses=1]
  store i32 %1693, i32* %24
  br label %k_sincos.exit6.i

k_sincos.exit6.i:                                 ; preds = %1691, %k_sincos.exit.i
  %1694 = load i32* %24                           ; <i32> [#uses=1]
  %1695 = sitofp i32 %1694 to float               ; <float> [#uses=1]
  %1696 = fmul float %1695, 0xBF7921FB60000000    ; <float> [#uses=1]
  store float %1696, float* %x.i5.i
  %1697 = load float* %x.i5.i                     ; <float> [#uses=1]
  %1698 = call float @_Z10native_cosf(float %1697) nounwind ; <float> [#uses=1]
  %1699 = load float** %25                        ; <float*> [#uses=1]
  store float %1698, float* %1699
  %1700 = load float* %x.i5.i                     ; <float> [#uses=1]
  %1701 = call float @_Z10native_sinf(float %1700) nounwind ; <float> [#uses=1]
  store float %1701, float* %23
  %1702 = load float* %23                         ; <float> [#uses=1]
  store float %1702, float* %s2.i51
  %1703 = load float* %c2.i50                     ; <float> [#uses=1]
  %1704 = insertelement <4 x float> undef, float %1703, i32 0 ; <<4 x float>> [#uses=2]
  %1705 = shufflevector <4 x float> %1704, <4 x float> %1704, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %1706 = load <4 x float>* %zr2.i28              ; <<4 x float>> [#uses=1]
  %1707 = fmul <4 x float> %1705, %1706           ; <<4 x float>> [#uses=1]
  %1708 = load float* %s2.i51                     ; <float> [#uses=1]
  %1709 = insertelement <4 x float> undef, float %1708, i32 0 ; <<4 x float>> [#uses=2]
  %1710 = shufflevector <4 x float> %1709, <4 x float> %1709, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %1711 = load <4 x float>* %zi2.i32              ; <<4 x float>> [#uses=1]
  %1712 = fmul <4 x float> %1710, %1711           ; <<4 x float>> [#uses=1]
  %1713 = fsub <4 x float> %1707, %1712           ; <<4 x float>> [#uses=1]
  store <4 x float> %1713, <4 x float>* %__r1.i52
  %1714 = load float* %c2.i50                     ; <float> [#uses=1]
  %1715 = insertelement <4 x float> undef, float %1714, i32 0 ; <<4 x float>> [#uses=2]
  %1716 = shufflevector <4 x float> %1715, <4 x float> %1715, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %1717 = load <4 x float>* %zi2.i32              ; <<4 x float>> [#uses=1]
  %1718 = fmul <4 x float> %1716, %1717           ; <<4 x float>> [#uses=1]
  %1719 = load float* %s2.i51                     ; <float> [#uses=1]
  %1720 = insertelement <4 x float> undef, float %1719, i32 0 ; <<4 x float>> [#uses=2]
  %1721 = shufflevector <4 x float> %1720, <4 x float> %1720, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %1722 = load <4 x float>* %zr2.i28              ; <<4 x float>> [#uses=1]
  %1723 = fmul <4 x float> %1721, %1722           ; <<4 x float>> [#uses=1]
  %1724 = fadd <4 x float> %1718, %1723           ; <<4 x float>> [#uses=1]
  store <4 x float> %1724, <4 x float>* %zi2.i32
  %1725 = load <4 x float>* %__r1.i52             ; <<4 x float>> [#uses=1]
  store <4 x float> %1725, <4 x float>* %zr2.i28
  %1726 = load i32* %tbase.i46                    ; <i32> [#uses=1]
  %1727 = mul i32 %1726, 3                        ; <i32> [#uses=1]
  store i32 %1727, i32* %27
  store float* %c3.i53, float** %28
  %1728 = load i32* %27                           ; <i32> [#uses=1]
  %1729 = icmp sgt i32 %1728, 512                 ; <i1> [#uses=1]
  br i1 %1729, label %1730, label %kfft_pass4.exit

; <label>:1730                                    ; preds = %k_sincos.exit6.i
  %1731 = load i32* %27                           ; <i32> [#uses=1]
  %1732 = sub i32 %1731, 1024                     ; <i32> [#uses=1]
  store i32 %1732, i32* %27
  br label %kfft_pass4.exit

kfft_pass4.exit:                                  ; preds = %k_sincos.exit6.i, %1730
  %1733 = load i32* %27                           ; <i32> [#uses=1]
  %1734 = sitofp i32 %1733 to float               ; <float> [#uses=1]
  %1735 = fmul float %1734, 0xBF7921FB60000000    ; <float> [#uses=1]
  store float %1735, float* %x.i3.i23
  %1736 = load float* %x.i3.i23                   ; <float> [#uses=1]
  %1737 = call float @_Z10native_cosf(float %1736) nounwind ; <float> [#uses=1]
  %1738 = load float** %28                        ; <float*> [#uses=1]
  store float %1737, float* %1738
  %1739 = load float* %x.i3.i23                   ; <float> [#uses=1]
  %1740 = call float @_Z10native_sinf(float %1739) nounwind ; <float> [#uses=1]
  store float %1740, float* %26
  %1741 = load float* %26                         ; <float> [#uses=1]
  store float %1741, float* %s3.i54
  %1742 = load float* %c3.i53                     ; <float> [#uses=1]
  %1743 = insertelement <4 x float> undef, float %1742, i32 0 ; <<4 x float>> [#uses=2]
  %1744 = shufflevector <4 x float> %1743, <4 x float> %1743, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %1745 = load <4 x float>* %zr3.i29              ; <<4 x float>> [#uses=1]
  %1746 = fmul <4 x float> %1744, %1745           ; <<4 x float>> [#uses=1]
  %1747 = load float* %s3.i54                     ; <float> [#uses=1]
  %1748 = insertelement <4 x float> undef, float %1747, i32 0 ; <<4 x float>> [#uses=2]
  %1749 = shufflevector <4 x float> %1748, <4 x float> %1748, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %1750 = load <4 x float>* %zi3.i33              ; <<4 x float>> [#uses=1]
  %1751 = fmul <4 x float> %1749, %1750           ; <<4 x float>> [#uses=1]
  %1752 = fsub <4 x float> %1746, %1751           ; <<4 x float>> [#uses=1]
  store <4 x float> %1752, <4 x float>* %__r2.i55
  %1753 = load float* %c3.i53                     ; <float> [#uses=1]
  %1754 = insertelement <4 x float> undef, float %1753, i32 0 ; <<4 x float>> [#uses=2]
  %1755 = shufflevector <4 x float> %1754, <4 x float> %1754, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %1756 = load <4 x float>* %zi3.i33              ; <<4 x float>> [#uses=1]
  %1757 = fmul <4 x float> %1755, %1756           ; <<4 x float>> [#uses=1]
  %1758 = load float* %s3.i54                     ; <float> [#uses=1]
  %1759 = insertelement <4 x float> undef, float %1758, i32 0 ; <<4 x float>> [#uses=2]
  %1760 = shufflevector <4 x float> %1759, <4 x float> %1759, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %1761 = load <4 x float>* %zr3.i29              ; <<4 x float>> [#uses=1]
  %1762 = fmul <4 x float> %1760, %1761           ; <<4 x float>> [#uses=1]
  %1763 = fadd <4 x float> %1757, %1762           ; <<4 x float>> [#uses=1]
  store <4 x float> %1763, <4 x float>* %zi3.i33
  %1764 = load <4 x float>* %__r2.i55             ; <<4 x float>> [#uses=1]
  store <4 x float> %1764, <4 x float>* %zr3.i29
  call void @barrier(i32 1) nounwind
  %1765 = load float addrspace(3)** %33           ; <float addrspace(3)*> [#uses=1]
  %1766 = load i32* %32                           ; <i32> [#uses=1]
  %1767 = getelementptr inbounds float addrspace(3)* %1765, i32 %1766 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %1767, float addrspace(3)** %lp.i25
  %1768 = load <4 x float>* %zr0.i26              ; <<4 x float>> [#uses=1]
  %1769 = extractelement <4 x float> %1768, i32 0 ; <float> [#uses=1]
  %1770 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1771 = getelementptr inbounds float addrspace(3)* %1770, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %1769, float addrspace(3)* %1771
  %1772 = load <4 x float>* %zr0.i26              ; <<4 x float>> [#uses=1]
  %1773 = extractelement <4 x float> %1772, i32 1 ; <float> [#uses=1]
  %1774 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1775 = getelementptr inbounds float addrspace(3)* %1774, i32 68 ; <float addrspace(3)*> [#uses=1]
  store float %1773, float addrspace(3)* %1775
  %1776 = load <4 x float>* %zr0.i26              ; <<4 x float>> [#uses=1]
  %1777 = extractelement <4 x float> %1776, i32 2 ; <float> [#uses=1]
  %1778 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1779 = getelementptr inbounds float addrspace(3)* %1778, i32 136 ; <float addrspace(3)*> [#uses=1]
  store float %1777, float addrspace(3)* %1779
  %1780 = load <4 x float>* %zr0.i26              ; <<4 x float>> [#uses=1]
  %1781 = extractelement <4 x float> %1780, i32 3 ; <float> [#uses=1]
  %1782 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1783 = getelementptr inbounds float addrspace(3)* %1782, i32 204 ; <float addrspace(3)*> [#uses=1]
  store float %1781, float addrspace(3)* %1783
  %1784 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1785 = getelementptr inbounds float addrspace(3)* %1784, i32 272 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %1785, float addrspace(3)** %lp.i25
  %1786 = load <4 x float>* %zr1.i27              ; <<4 x float>> [#uses=1]
  %1787 = extractelement <4 x float> %1786, i32 0 ; <float> [#uses=1]
  %1788 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1789 = getelementptr inbounds float addrspace(3)* %1788, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %1787, float addrspace(3)* %1789
  %1790 = load <4 x float>* %zr1.i27              ; <<4 x float>> [#uses=1]
  %1791 = extractelement <4 x float> %1790, i32 1 ; <float> [#uses=1]
  %1792 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1793 = getelementptr inbounds float addrspace(3)* %1792, i32 68 ; <float addrspace(3)*> [#uses=1]
  store float %1791, float addrspace(3)* %1793
  %1794 = load <4 x float>* %zr1.i27              ; <<4 x float>> [#uses=1]
  %1795 = extractelement <4 x float> %1794, i32 2 ; <float> [#uses=1]
  %1796 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1797 = getelementptr inbounds float addrspace(3)* %1796, i32 136 ; <float addrspace(3)*> [#uses=1]
  store float %1795, float addrspace(3)* %1797
  %1798 = load <4 x float>* %zr1.i27              ; <<4 x float>> [#uses=1]
  %1799 = extractelement <4 x float> %1798, i32 3 ; <float> [#uses=1]
  %1800 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1801 = getelementptr inbounds float addrspace(3)* %1800, i32 204 ; <float addrspace(3)*> [#uses=1]
  store float %1799, float addrspace(3)* %1801
  %1802 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1803 = getelementptr inbounds float addrspace(3)* %1802, i32 272 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %1803, float addrspace(3)** %lp.i25
  %1804 = load <4 x float>* %zr2.i28              ; <<4 x float>> [#uses=1]
  %1805 = extractelement <4 x float> %1804, i32 0 ; <float> [#uses=1]
  %1806 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1807 = getelementptr inbounds float addrspace(3)* %1806, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %1805, float addrspace(3)* %1807
  %1808 = load <4 x float>* %zr2.i28              ; <<4 x float>> [#uses=1]
  %1809 = extractelement <4 x float> %1808, i32 1 ; <float> [#uses=1]
  %1810 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1811 = getelementptr inbounds float addrspace(3)* %1810, i32 68 ; <float addrspace(3)*> [#uses=1]
  store float %1809, float addrspace(3)* %1811
  %1812 = load <4 x float>* %zr2.i28              ; <<4 x float>> [#uses=1]
  %1813 = extractelement <4 x float> %1812, i32 2 ; <float> [#uses=1]
  %1814 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1815 = getelementptr inbounds float addrspace(3)* %1814, i32 136 ; <float addrspace(3)*> [#uses=1]
  store float %1813, float addrspace(3)* %1815
  %1816 = load <4 x float>* %zr2.i28              ; <<4 x float>> [#uses=1]
  %1817 = extractelement <4 x float> %1816, i32 3 ; <float> [#uses=1]
  %1818 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1819 = getelementptr inbounds float addrspace(3)* %1818, i32 204 ; <float addrspace(3)*> [#uses=1]
  store float %1817, float addrspace(3)* %1819
  %1820 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1821 = getelementptr inbounds float addrspace(3)* %1820, i32 272 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %1821, float addrspace(3)** %lp.i25
  %1822 = load <4 x float>* %zr3.i29              ; <<4 x float>> [#uses=1]
  %1823 = extractelement <4 x float> %1822, i32 0 ; <float> [#uses=1]
  %1824 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1825 = getelementptr inbounds float addrspace(3)* %1824, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %1823, float addrspace(3)* %1825
  %1826 = load <4 x float>* %zr3.i29              ; <<4 x float>> [#uses=1]
  %1827 = extractelement <4 x float> %1826, i32 1 ; <float> [#uses=1]
  %1828 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1829 = getelementptr inbounds float addrspace(3)* %1828, i32 68 ; <float addrspace(3)*> [#uses=1]
  store float %1827, float addrspace(3)* %1829
  %1830 = load <4 x float>* %zr3.i29              ; <<4 x float>> [#uses=1]
  %1831 = extractelement <4 x float> %1830, i32 2 ; <float> [#uses=1]
  %1832 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1833 = getelementptr inbounds float addrspace(3)* %1832, i32 136 ; <float addrspace(3)*> [#uses=1]
  store float %1831, float addrspace(3)* %1833
  %1834 = load <4 x float>* %zr3.i29              ; <<4 x float>> [#uses=1]
  %1835 = extractelement <4 x float> %1834, i32 3 ; <float> [#uses=1]
  %1836 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1837 = getelementptr inbounds float addrspace(3)* %1836, i32 204 ; <float addrspace(3)*> [#uses=1]
  store float %1835, float addrspace(3)* %1837
  %1838 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1839 = getelementptr inbounds float addrspace(3)* %1838, i32 272 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %1839, float addrspace(3)** %lp.i25
  %1840 = load <4 x float>* %zi0.i30              ; <<4 x float>> [#uses=1]
  %1841 = extractelement <4 x float> %1840, i32 0 ; <float> [#uses=1]
  %1842 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1843 = getelementptr inbounds float addrspace(3)* %1842, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %1841, float addrspace(3)* %1843
  %1844 = load <4 x float>* %zi0.i30              ; <<4 x float>> [#uses=1]
  %1845 = extractelement <4 x float> %1844, i32 1 ; <float> [#uses=1]
  %1846 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1847 = getelementptr inbounds float addrspace(3)* %1846, i32 68 ; <float addrspace(3)*> [#uses=1]
  store float %1845, float addrspace(3)* %1847
  %1848 = load <4 x float>* %zi0.i30              ; <<4 x float>> [#uses=1]
  %1849 = extractelement <4 x float> %1848, i32 2 ; <float> [#uses=1]
  %1850 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1851 = getelementptr inbounds float addrspace(3)* %1850, i32 136 ; <float addrspace(3)*> [#uses=1]
  store float %1849, float addrspace(3)* %1851
  %1852 = load <4 x float>* %zi0.i30              ; <<4 x float>> [#uses=1]
  %1853 = extractelement <4 x float> %1852, i32 3 ; <float> [#uses=1]
  %1854 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1855 = getelementptr inbounds float addrspace(3)* %1854, i32 204 ; <float addrspace(3)*> [#uses=1]
  store float %1853, float addrspace(3)* %1855
  %1856 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1857 = getelementptr inbounds float addrspace(3)* %1856, i32 272 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %1857, float addrspace(3)** %lp.i25
  %1858 = load <4 x float>* %zi1.i31              ; <<4 x float>> [#uses=1]
  %1859 = extractelement <4 x float> %1858, i32 0 ; <float> [#uses=1]
  %1860 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1861 = getelementptr inbounds float addrspace(3)* %1860, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %1859, float addrspace(3)* %1861
  %1862 = load <4 x float>* %zi1.i31              ; <<4 x float>> [#uses=1]
  %1863 = extractelement <4 x float> %1862, i32 1 ; <float> [#uses=1]
  %1864 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1865 = getelementptr inbounds float addrspace(3)* %1864, i32 68 ; <float addrspace(3)*> [#uses=1]
  store float %1863, float addrspace(3)* %1865
  %1866 = load <4 x float>* %zi1.i31              ; <<4 x float>> [#uses=1]
  %1867 = extractelement <4 x float> %1866, i32 2 ; <float> [#uses=1]
  %1868 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1869 = getelementptr inbounds float addrspace(3)* %1868, i32 136 ; <float addrspace(3)*> [#uses=1]
  store float %1867, float addrspace(3)* %1869
  %1870 = load <4 x float>* %zi1.i31              ; <<4 x float>> [#uses=1]
  %1871 = extractelement <4 x float> %1870, i32 3 ; <float> [#uses=1]
  %1872 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1873 = getelementptr inbounds float addrspace(3)* %1872, i32 204 ; <float addrspace(3)*> [#uses=1]
  store float %1871, float addrspace(3)* %1873
  %1874 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1875 = getelementptr inbounds float addrspace(3)* %1874, i32 272 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %1875, float addrspace(3)** %lp.i25
  %1876 = load <4 x float>* %zi2.i32              ; <<4 x float>> [#uses=1]
  %1877 = extractelement <4 x float> %1876, i32 0 ; <float> [#uses=1]
  %1878 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1879 = getelementptr inbounds float addrspace(3)* %1878, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %1877, float addrspace(3)* %1879
  %1880 = load <4 x float>* %zi2.i32              ; <<4 x float>> [#uses=1]
  %1881 = extractelement <4 x float> %1880, i32 1 ; <float> [#uses=1]
  %1882 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1883 = getelementptr inbounds float addrspace(3)* %1882, i32 68 ; <float addrspace(3)*> [#uses=1]
  store float %1881, float addrspace(3)* %1883
  %1884 = load <4 x float>* %zi2.i32              ; <<4 x float>> [#uses=1]
  %1885 = extractelement <4 x float> %1884, i32 2 ; <float> [#uses=1]
  %1886 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1887 = getelementptr inbounds float addrspace(3)* %1886, i32 136 ; <float addrspace(3)*> [#uses=1]
  store float %1885, float addrspace(3)* %1887
  %1888 = load <4 x float>* %zi2.i32              ; <<4 x float>> [#uses=1]
  %1889 = extractelement <4 x float> %1888, i32 3 ; <float> [#uses=1]
  %1890 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1891 = getelementptr inbounds float addrspace(3)* %1890, i32 204 ; <float addrspace(3)*> [#uses=1]
  store float %1889, float addrspace(3)* %1891
  %1892 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1893 = getelementptr inbounds float addrspace(3)* %1892, i32 272 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %1893, float addrspace(3)** %lp.i25
  %1894 = load <4 x float>* %zi3.i33              ; <<4 x float>> [#uses=1]
  %1895 = extractelement <4 x float> %1894, i32 0 ; <float> [#uses=1]
  %1896 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1897 = getelementptr inbounds float addrspace(3)* %1896, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %1895, float addrspace(3)* %1897
  %1898 = load <4 x float>* %zi3.i33              ; <<4 x float>> [#uses=1]
  %1899 = extractelement <4 x float> %1898, i32 1 ; <float> [#uses=1]
  %1900 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1901 = getelementptr inbounds float addrspace(3)* %1900, i32 68 ; <float addrspace(3)*> [#uses=1]
  store float %1899, float addrspace(3)* %1901
  %1902 = load <4 x float>* %zi3.i33              ; <<4 x float>> [#uses=1]
  %1903 = extractelement <4 x float> %1902, i32 2 ; <float> [#uses=1]
  %1904 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1905 = getelementptr inbounds float addrspace(3)* %1904, i32 136 ; <float addrspace(3)*> [#uses=1]
  store float %1903, float addrspace(3)* %1905
  %1906 = load <4 x float>* %zi3.i33              ; <<4 x float>> [#uses=1]
  %1907 = extractelement <4 x float> %1906, i32 3 ; <float> [#uses=1]
  %1908 = load float addrspace(3)** %lp.i25       ; <float addrspace(3)*> [#uses=1]
  %1909 = getelementptr inbounds float addrspace(3)* %1908, i32 204 ; <float addrspace(3)*> [#uses=1]
  store float %1907, float addrspace(3)* %1909
  call void @barrier(i32 1) nounwind
  %1910 = load i32* %me                           ; <i32> [#uses=1]
  %1911 = load float addrspace(1)** %gr           ; <float addrspace(1)*> [#uses=1]
  %1912 = load float addrspace(1)** %gi           ; <float addrspace(1)*> [#uses=1]
  store i32 %1910, i32* %34
  store float addrspace(3)* getelementptr inbounds ([2176 x float] addrspace(3)* @opencl_kfft_local_lds, i32 0, i32 0), float addrspace(3)** %35
  store float addrspace(1)* %1911, float addrspace(1)** %36
  store float addrspace(1)* %1912, float addrspace(1)** %37
  %1913 = load float addrspace(3)** %35           ; <float addrspace(3)*> [#uses=1]
  %1914 = load i32* %34                           ; <i32> [#uses=1]
  %1915 = and i32 %1914, 15                       ; <i32> [#uses=1]
  %1916 = load i32* %34                           ; <i32> [#uses=1]
  %1917 = lshr i32 %1916, 4                       ; <i32> [#uses=1]
  %1918 = mul i32 %1917, 272                      ; <i32> [#uses=1]
  %1919 = add i32 %1915, %1918                    ; <i32> [#uses=1]
  %1920 = getelementptr inbounds float addrspace(3)* %1913, i32 %1919 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %1920, float addrspace(3)** %lp.i1
  %1921 = load float addrspace(3)** %lp.i1        ; <float addrspace(3)*> [#uses=1]
  %1922 = getelementptr inbounds float addrspace(3)* %1921, i32 0 ; <float addrspace(3)*> [#uses=1]
  %1923 = load float addrspace(3)* %1922          ; <float> [#uses=1]
  %1924 = load <4 x float>* %zr0.i2               ; <<4 x float>> [#uses=1]
  %1925 = insertelement <4 x float> %1924, float %1923, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %1925, <4 x float>* %zr0.i2
  %1926 = load float addrspace(3)** %lp.i1        ; <float addrspace(3)*> [#uses=1]
  %1927 = getelementptr inbounds float addrspace(3)* %1926, i32 68 ; <float addrspace(3)*> [#uses=1]
  %1928 = load float addrspace(3)* %1927          ; <float> [#uses=1]
  %1929 = load <4 x float>* %zr0.i2               ; <<4 x float>> [#uses=1]
  %1930 = insertelement <4 x float> %1929, float %1928, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %1930, <4 x float>* %zr0.i2
  %1931 = load float addrspace(3)** %lp.i1        ; <float addrspace(3)*> [#uses=1]
  %1932 = getelementptr inbounds float addrspace(3)* %1931, i32 136 ; <float addrspace(3)*> [#uses=1]
  %1933 = load float addrspace(3)* %1932          ; <float> [#uses=1]
  %1934 = load <4 x float>* %zr0.i2               ; <<4 x float>> [#uses=1]
  %1935 = insertelement <4 x float> %1934, float %1933, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %1935, <4 x float>* %zr0.i2
  %1936 = load float addrspace(3)** %lp.i1        ; <float addrspace(3)*> [#uses=1]
  %1937 = getelementptr inbounds float addrspace(3)* %1936, i32 204 ; <float addrspace(3)*> [#uses=1]
  %1938 = load float addrspace(3)* %1937          ; <float> [#uses=1]
  %1939 = load <4 x float>* %zr0.i2               ; <<4 x float>> [#uses=1]
  %1940 = insertelement <4 x float> %1939, float %1938, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %1940, <4 x float>* %zr0.i2
  %1941 = load float addrspace(3)** %lp.i1        ; <float addrspace(3)*> [#uses=1]
  %1942 = getelementptr inbounds float addrspace(3)* %1941, i32 16 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %1942, float addrspace(3)** %lp.i1
  %1943 = load float addrspace(3)** %lp.i1        ; <float addrspace(3)*> [#uses=1]
  %1944 = getelementptr inbounds float addrspace(3)* %1943, i32 0 ; <float addrspace(3)*> [#uses=1]
  %1945 = load float addrspace(3)* %1944          ; <float> [#uses=1]
  %1946 = load <4 x float>* %zr1.i3               ; <<4 x float>> [#uses=1]
  %1947 = insertelement <4 x float> %1946, float %1945, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %1947, <4 x float>* %zr1.i3
  %1948 = load float addrspace(3)** %lp.i1        ; <float addrspace(3)*> [#uses=1]
  %1949 = getelementptr inbounds float addrspace(3)* %1948, i32 68 ; <float addrspace(3)*> [#uses=1]
  %1950 = load float addrspace(3)* %1949          ; <float> [#uses=1]
  %1951 = load <4 x float>* %zr1.i3               ; <<4 x float>> [#uses=1]
  %1952 = insertelement <4 x float> %1951, float %1950, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %1952, <4 x float>* %zr1.i3
  %1953 = load float addrspace(3)** %lp.i1        ; <float addrspace(3)*> [#uses=1]
  %1954 = getelementptr inbounds float addrspace(3)* %1953, i32 136 ; <float addrspace(3)*> [#uses=1]
  %1955 = load float addrspace(3)* %1954          ; <float> [#uses=1]
  %1956 = load <4 x float>* %zr1.i3               ; <<4 x float>> [#uses=1]
  %1957 = insertelement <4 x float> %1956, float %1955, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %1957, <4 x float>* %zr1.i3
  %1958 = load float addrspace(3)** %lp.i1        ; <float addrspace(3)*> [#uses=1]
  %1959 = getelementptr inbounds float addrspace(3)* %1958, i32 204 ; <float addrspace(3)*> [#uses=1]
  %1960 = load float addrspace(3)* %1959          ; <float> [#uses=1]
  %1961 = load <4 x float>* %zr1.i3               ; <<4 x float>> [#uses=1]
  %1962 = insertelement <4 x float> %1961, float %1960, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %1962, <4 x float>* %zr1.i3
  %1963 = load float addrspace(3)** %lp.i1        ; <float addrspace(3)*> [#uses=1]
  %1964 = getelementptr inbounds float addrspace(3)* %1963, i32 16 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %1964, float addrspace(3)** %lp.i1
  %1965 = load float addrspace(3)** %lp.i1        ; <float addrspace(3)*> [#uses=1]
  %1966 = getelementptr inbounds float addrspace(3)* %1965, i32 0 ; <float addrspace(3)*> [#uses=1]
  %1967 = load float addrspace(3)* %1966          ; <float> [#uses=1]
  %1968 = load <4 x float>* %zr2.i4               ; <<4 x float>> [#uses=1]
  %1969 = insertelement <4 x float> %1968, float %1967, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %1969, <4 x float>* %zr2.i4
  %1970 = load float addrspace(3)** %lp.i1        ; <float addrspace(3)*> [#uses=1]
  %1971 = getelementptr inbounds float addrspace(3)* %1970, i32 68 ; <float addrspace(3)*> [#uses=1]
  %1972 = load float addrspace(3)* %1971          ; <float> [#uses=1]
  %1973 = load <4 x float>* %zr2.i4               ; <<4 x float>> [#uses=1]
  %1974 = insertelement <4 x float> %1973, float %1972, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %1974, <4 x float>* %zr2.i4
  %1975 = load float addrspace(3)** %lp.i1        ; <float addrspace(3)*> [#uses=1]
  %1976 = getelementptr inbounds float addrspace(3)* %1975, i32 136 ; <float addrspace(3)*> [#uses=1]
  %1977 = load float addrspace(3)* %1976          ; <float> [#uses=1]
  %1978 = load <4 x float>* %zr2.i4               ; <<4 x float>> [#uses=1]
  %1979 = insertelement <4 x float> %1978, float %1977, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %1979, <4 x float>* %zr2.i4
  %1980 = load float addrspace(3)** %lp.i1        ; <float addrspace(3)*> [#uses=1]
  %1981 = getelementptr inbounds float addrspace(3)* %1980, i32 204 ; <float addrspace(3)*> [#uses=1]
  %1982 = load float addrspace(3)* %1981          ; <float> [#uses=1]
  %1983 = load <4 x float>* %zr2.i4               ; <<4 x float>> [#uses=1]
  %1984 = insertelement <4 x float> %1983, float %1982, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %1984, <4 x float>* %zr2.i4
  %1985 = load float addrspace(3)** %lp.i1        ; <float addrspace(3)*> [#uses=1]
  %1986 = getelementptr inbounds float addrspace(3)* %1985, i32 16 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %1986, float addrspace(3)** %lp.i1
  %1987 = load float addrspace(3)** %lp.i1        ; <float addrspace(3)*> [#uses=1]
  %1988 = getelementptr inbounds float addrspace(3)* %1987, i32 0 ; <float addrspace(3)*> [#uses=1]
  %1989 = load float addrspace(3)* %1988          ; <float> [#uses=1]
  %1990 = load <4 x float>* %zr3.i5               ; <<4 x float>> [#uses=1]
  %1991 = insertelement <4 x float> %1990, float %1989, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %1991, <4 x float>* %zr3.i5
  %1992 = load float addrspace(3)** %lp.i1        ; <float addrspace(3)*> [#uses=1]
  %1993 = getelementptr inbounds float addrspace(3)* %1992, i32 68 ; <float addrspace(3)*> [#uses=1]
  %1994 = load float addrspace(3)* %1993          ; <float> [#uses=1]
  %1995 = load <4 x float>* %zr3.i5               ; <<4 x float>> [#uses=1]
  %1996 = insertelement <4 x float> %1995, float %1994, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %1996, <4 x float>* %zr3.i5
  %1997 = load float addrspace(3)** %lp.i1        ; <float addrspace(3)*> [#uses=1]
  %1998 = getelementptr inbounds float addrspace(3)* %1997, i32 136 ; <float addrspace(3)*> [#uses=1]
  %1999 = load float addrspace(3)* %1998          ; <float> [#uses=1]
  %2000 = load <4 x float>* %zr3.i5               ; <<4 x float>> [#uses=1]
  %2001 = insertelement <4 x float> %2000, float %1999, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %2001, <4 x float>* %zr3.i5
  %2002 = load float addrspace(3)** %lp.i1        ; <float addrspace(3)*> [#uses=1]
  %2003 = getelementptr inbounds float addrspace(3)* %2002, i32 204 ; <float addrspace(3)*> [#uses=1]
  %2004 = load float addrspace(3)* %2003          ; <float> [#uses=1]
  %2005 = load <4 x float>* %zr3.i5               ; <<4 x float>> [#uses=1]
  %2006 = insertelement <4 x float> %2005, float %2004, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %2006, <4 x float>* %zr3.i5
  %2007 = load float addrspace(3)** %lp.i1        ; <float addrspace(3)*> [#uses=1]
  %2008 = getelementptr inbounds float addrspace(3)* %2007, i32 1040 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %2008, float addrspace(3)** %lp.i1
  %2009 = load float addrspace(3)** %lp.i1        ; <float addrspace(3)*> [#uses=1]
  %2010 = getelementptr inbounds float addrspace(3)* %2009, i32 0 ; <float addrspace(3)*> [#uses=1]
  %2011 = load float addrspace(3)* %2010          ; <float> [#uses=1]
  %2012 = load <4 x float>* %zi0.i6               ; <<4 x float>> [#uses=1]
  %2013 = insertelement <4 x float> %2012, float %2011, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %2013, <4 x float>* %zi0.i6
  %2014 = load float addrspace(3)** %lp.i1        ; <float addrspace(3)*> [#uses=1]
  %2015 = getelementptr inbounds float addrspace(3)* %2014, i32 68 ; <float addrspace(3)*> [#uses=1]
  %2016 = load float addrspace(3)* %2015          ; <float> [#uses=1]
  %2017 = load <4 x float>* %zi0.i6               ; <<4 x float>> [#uses=1]
  %2018 = insertelement <4 x float> %2017, float %2016, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %2018, <4 x float>* %zi0.i6
  %2019 = load float addrspace(3)** %lp.i1        ; <float addrspace(3)*> [#uses=1]
  %2020 = getelementptr inbounds float addrspace(3)* %2019, i32 136 ; <float addrspace(3)*> [#uses=1]
  %2021 = load float addrspace(3)* %2020          ; <float> [#uses=1]
  %2022 = load <4 x float>* %zi0.i6               ; <<4 x float>> [#uses=1]
  %2023 = insertelement <4 x float> %2022, float %2021, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %2023, <4 x float>* %zi0.i6
  %2024 = load float addrspace(3)** %lp.i1        ; <float addrspace(3)*> [#uses=1]
  %2025 = getelementptr inbounds float addrspace(3)* %2024, i32 204 ; <float addrspace(3)*> [#uses=1]
  %2026 = load float addrspace(3)* %2025          ; <float> [#uses=1]
  %2027 = load <4 x float>* %zi0.i6               ; <<4 x float>> [#uses=1]
  %2028 = insertelement <4 x float> %2027, float %2026, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %2028, <4 x float>* %zi0.i6
  %2029 = load float addrspace(3)** %lp.i1        ; <float addrspace(3)*> [#uses=1]
  %2030 = getelementptr inbounds float addrspace(3)* %2029, i32 16 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %2030, float addrspace(3)** %lp.i1
  %2031 = load float addrspace(3)** %lp.i1        ; <float addrspace(3)*> [#uses=1]
  %2032 = getelementptr inbounds float addrspace(3)* %2031, i32 0 ; <float addrspace(3)*> [#uses=1]
  %2033 = load float addrspace(3)* %2032          ; <float> [#uses=1]
  %2034 = load <4 x float>* %zi1.i7               ; <<4 x float>> [#uses=1]
  %2035 = insertelement <4 x float> %2034, float %2033, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %2035, <4 x float>* %zi1.i7
  %2036 = load float addrspace(3)** %lp.i1        ; <float addrspace(3)*> [#uses=1]
  %2037 = getelementptr inbounds float addrspace(3)* %2036, i32 68 ; <float addrspace(3)*> [#uses=1]
  %2038 = load float addrspace(3)* %2037          ; <float> [#uses=1]
  %2039 = load <4 x float>* %zi1.i7               ; <<4 x float>> [#uses=1]
  %2040 = insertelement <4 x float> %2039, float %2038, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %2040, <4 x float>* %zi1.i7
  %2041 = load float addrspace(3)** %lp.i1        ; <float addrspace(3)*> [#uses=1]
  %2042 = getelementptr inbounds float addrspace(3)* %2041, i32 136 ; <float addrspace(3)*> [#uses=1]
  %2043 = load float addrspace(3)* %2042          ; <float> [#uses=1]
  %2044 = load <4 x float>* %zi1.i7               ; <<4 x float>> [#uses=1]
  %2045 = insertelement <4 x float> %2044, float %2043, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %2045, <4 x float>* %zi1.i7
  %2046 = load float addrspace(3)** %lp.i1        ; <float addrspace(3)*> [#uses=1]
  %2047 = getelementptr inbounds float addrspace(3)* %2046, i32 204 ; <float addrspace(3)*> [#uses=1]
  %2048 = load float addrspace(3)* %2047          ; <float> [#uses=1]
  %2049 = load <4 x float>* %zi1.i7               ; <<4 x float>> [#uses=1]
  %2050 = insertelement <4 x float> %2049, float %2048, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %2050, <4 x float>* %zi1.i7
  %2051 = load float addrspace(3)** %lp.i1        ; <float addrspace(3)*> [#uses=1]
  %2052 = getelementptr inbounds float addrspace(3)* %2051, i32 16 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %2052, float addrspace(3)** %lp.i1
  %2053 = load float addrspace(3)** %lp.i1        ; <float addrspace(3)*> [#uses=1]
  %2054 = getelementptr inbounds float addrspace(3)* %2053, i32 0 ; <float addrspace(3)*> [#uses=1]
  %2055 = load float addrspace(3)* %2054          ; <float> [#uses=1]
  %2056 = load <4 x float>* %zi2.i8               ; <<4 x float>> [#uses=1]
  %2057 = insertelement <4 x float> %2056, float %2055, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %2057, <4 x float>* %zi2.i8
  %2058 = load float addrspace(3)** %lp.i1        ; <float addrspace(3)*> [#uses=1]
  %2059 = getelementptr inbounds float addrspace(3)* %2058, i32 68 ; <float addrspace(3)*> [#uses=1]
  %2060 = load float addrspace(3)* %2059          ; <float> [#uses=1]
  %2061 = load <4 x float>* %zi2.i8               ; <<4 x float>> [#uses=1]
  %2062 = insertelement <4 x float> %2061, float %2060, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %2062, <4 x float>* %zi2.i8
  %2063 = load float addrspace(3)** %lp.i1        ; <float addrspace(3)*> [#uses=1]
  %2064 = getelementptr inbounds float addrspace(3)* %2063, i32 136 ; <float addrspace(3)*> [#uses=1]
  %2065 = load float addrspace(3)* %2064          ; <float> [#uses=1]
  %2066 = load <4 x float>* %zi2.i8               ; <<4 x float>> [#uses=1]
  %2067 = insertelement <4 x float> %2066, float %2065, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %2067, <4 x float>* %zi2.i8
  %2068 = load float addrspace(3)** %lp.i1        ; <float addrspace(3)*> [#uses=1]
  %2069 = getelementptr inbounds float addrspace(3)* %2068, i32 204 ; <float addrspace(3)*> [#uses=1]
  %2070 = load float addrspace(3)* %2069          ; <float> [#uses=1]
  %2071 = load <4 x float>* %zi2.i8               ; <<4 x float>> [#uses=1]
  %2072 = insertelement <4 x float> %2071, float %2070, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %2072, <4 x float>* %zi2.i8
  %2073 = load float addrspace(3)** %lp.i1        ; <float addrspace(3)*> [#uses=1]
  %2074 = getelementptr inbounds float addrspace(3)* %2073, i32 16 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %2074, float addrspace(3)** %lp.i1
  %2075 = load float addrspace(3)** %lp.i1        ; <float addrspace(3)*> [#uses=1]
  %2076 = getelementptr inbounds float addrspace(3)* %2075, i32 0 ; <float addrspace(3)*> [#uses=1]
  %2077 = load float addrspace(3)* %2076          ; <float> [#uses=1]
  %2078 = load <4 x float>* %zi3.i9               ; <<4 x float>> [#uses=1]
  %2079 = insertelement <4 x float> %2078, float %2077, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %2079, <4 x float>* %zi3.i9
  %2080 = load float addrspace(3)** %lp.i1        ; <float addrspace(3)*> [#uses=1]
  %2081 = getelementptr inbounds float addrspace(3)* %2080, i32 68 ; <float addrspace(3)*> [#uses=1]
  %2082 = load float addrspace(3)* %2081          ; <float> [#uses=1]
  %2083 = load <4 x float>* %zi3.i9               ; <<4 x float>> [#uses=1]
  %2084 = insertelement <4 x float> %2083, float %2082, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %2084, <4 x float>* %zi3.i9
  %2085 = load float addrspace(3)** %lp.i1        ; <float addrspace(3)*> [#uses=1]
  %2086 = getelementptr inbounds float addrspace(3)* %2085, i32 136 ; <float addrspace(3)*> [#uses=1]
  %2087 = load float addrspace(3)* %2086          ; <float> [#uses=1]
  %2088 = load <4 x float>* %zi3.i9               ; <<4 x float>> [#uses=1]
  %2089 = insertelement <4 x float> %2088, float %2087, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %2089, <4 x float>* %zi3.i9
  %2090 = load float addrspace(3)** %lp.i1        ; <float addrspace(3)*> [#uses=1]
  %2091 = getelementptr inbounds float addrspace(3)* %2090, i32 204 ; <float addrspace(3)*> [#uses=1]
  %2092 = load float addrspace(3)* %2091          ; <float> [#uses=1]
  %2093 = load <4 x float>* %zi3.i9               ; <<4 x float>> [#uses=1]
  %2094 = insertelement <4 x float> %2093, float %2092, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %2094, <4 x float>* %zi3.i9
  %2095 = load <4 x float>* %zr0.i2               ; <<4 x float>> [#uses=1]
  %2096 = load <4 x float>* %zr2.i4               ; <<4 x float>> [#uses=1]
  %2097 = fadd <4 x float> %2095, %2096           ; <<4 x float>> [#uses=1]
  store <4 x float> %2097, <4 x float>* %ar0.i10
  %2098 = load <4 x float>* %zr1.i3               ; <<4 x float>> [#uses=1]
  %2099 = load <4 x float>* %zr3.i5               ; <<4 x float>> [#uses=1]
  %2100 = fadd <4 x float> %2098, %2099           ; <<4 x float>> [#uses=1]
  store <4 x float> %2100, <4 x float>* %ar2.i11
  %2101 = load <4 x float>* %ar0.i10              ; <<4 x float>> [#uses=1]
  %2102 = load <4 x float>* %ar2.i11              ; <<4 x float>> [#uses=1]
  %2103 = fadd <4 x float> %2101, %2102           ; <<4 x float>> [#uses=1]
  store <4 x float> %2103, <4 x float>* %br0.i12
  %2104 = load <4 x float>* %zr0.i2               ; <<4 x float>> [#uses=1]
  %2105 = load <4 x float>* %zr2.i4               ; <<4 x float>> [#uses=1]
  %2106 = fsub <4 x float> %2104, %2105           ; <<4 x float>> [#uses=1]
  store <4 x float> %2106, <4 x float>* %br1.i13
  %2107 = load <4 x float>* %ar0.i10              ; <<4 x float>> [#uses=1]
  %2108 = load <4 x float>* %ar2.i11              ; <<4 x float>> [#uses=1]
  %2109 = fsub <4 x float> %2107, %2108           ; <<4 x float>> [#uses=1]
  store <4 x float> %2109, <4 x float>* %br2.i14
  %2110 = load <4 x float>* %zr1.i3               ; <<4 x float>> [#uses=1]
  %2111 = load <4 x float>* %zr3.i5               ; <<4 x float>> [#uses=1]
  %2112 = fsub <4 x float> %2110, %2111           ; <<4 x float>> [#uses=1]
  store <4 x float> %2112, <4 x float>* %br3.i15
  %2113 = load <4 x float>* %zi0.i6               ; <<4 x float>> [#uses=1]
  %2114 = load <4 x float>* %zi2.i8               ; <<4 x float>> [#uses=1]
  %2115 = fadd <4 x float> %2113, %2114           ; <<4 x float>> [#uses=1]
  store <4 x float> %2115, <4 x float>* %ai0.i16
  %2116 = load <4 x float>* %zi1.i7               ; <<4 x float>> [#uses=1]
  %2117 = load <4 x float>* %zi3.i9               ; <<4 x float>> [#uses=1]
  %2118 = fadd <4 x float> %2116, %2117           ; <<4 x float>> [#uses=1]
  store <4 x float> %2118, <4 x float>* %ai2.i17
  %2119 = load <4 x float>* %ai0.i16              ; <<4 x float>> [#uses=1]
  %2120 = load <4 x float>* %ai2.i17              ; <<4 x float>> [#uses=1]
  %2121 = fadd <4 x float> %2119, %2120           ; <<4 x float>> [#uses=1]
  store <4 x float> %2121, <4 x float>* %bi0.i18
  %2122 = load <4 x float>* %zi0.i6               ; <<4 x float>> [#uses=1]
  %2123 = load <4 x float>* %zi2.i8               ; <<4 x float>> [#uses=1]
  %2124 = fsub <4 x float> %2122, %2123           ; <<4 x float>> [#uses=1]
  store <4 x float> %2124, <4 x float>* %bi1.i19
  %2125 = load <4 x float>* %ai0.i16              ; <<4 x float>> [#uses=1]
  %2126 = load <4 x float>* %ai2.i17              ; <<4 x float>> [#uses=1]
  %2127 = fsub <4 x float> %2125, %2126           ; <<4 x float>> [#uses=1]
  store <4 x float> %2127, <4 x float>* %bi2.i20
  %2128 = load <4 x float>* %zi1.i7               ; <<4 x float>> [#uses=1]
  %2129 = load <4 x float>* %zi3.i9               ; <<4 x float>> [#uses=1]
  %2130 = fsub <4 x float> %2128, %2129           ; <<4 x float>> [#uses=1]
  store <4 x float> %2130, <4 x float>* %bi3.i21
  %2131 = load <4 x float>* %br0.i12              ; <<4 x float>> [#uses=1]
  store <4 x float> %2131, <4 x float>* %zr0.i2
  %2132 = load <4 x float>* %bi0.i18              ; <<4 x float>> [#uses=1]
  store <4 x float> %2132, <4 x float>* %zi0.i6
  %2133 = load <4 x float>* %br1.i13              ; <<4 x float>> [#uses=1]
  %2134 = load <4 x float>* %bi3.i21              ; <<4 x float>> [#uses=1]
  %2135 = fadd <4 x float> %2133, %2134           ; <<4 x float>> [#uses=1]
  store <4 x float> %2135, <4 x float>* %zr1.i3
  %2136 = load <4 x float>* %bi1.i19              ; <<4 x float>> [#uses=1]
  %2137 = load <4 x float>* %br3.i15              ; <<4 x float>> [#uses=1]
  %2138 = fsub <4 x float> %2136, %2137           ; <<4 x float>> [#uses=1]
  store <4 x float> %2138, <4 x float>* %zi1.i7
  %2139 = load <4 x float>* %br1.i13              ; <<4 x float>> [#uses=1]
  %2140 = load <4 x float>* %bi3.i21              ; <<4 x float>> [#uses=1]
  %2141 = fsub <4 x float> %2139, %2140           ; <<4 x float>> [#uses=1]
  store <4 x float> %2141, <4 x float>* %zr3.i5
  %2142 = load <4 x float>* %br3.i15              ; <<4 x float>> [#uses=1]
  %2143 = load <4 x float>* %bi1.i19              ; <<4 x float>> [#uses=1]
  %2144 = fadd <4 x float> %2142, %2143           ; <<4 x float>> [#uses=1]
  store <4 x float> %2144, <4 x float>* %zi3.i9
  %2145 = load <4 x float>* %br2.i14              ; <<4 x float>> [#uses=1]
  store <4 x float> %2145, <4 x float>* %zr2.i4
  %2146 = load <4 x float>* %bi2.i20              ; <<4 x float>> [#uses=1]
  store <4 x float> %2146, <4 x float>* %zi2.i8
  %2147 = load float addrspace(1)** %36           ; <float addrspace(1)*> [#uses=1]
  %2148 = load i32* %34                           ; <i32> [#uses=1]
  %2149 = shl i32 %2148, 2                        ; <i32> [#uses=1]
  %2150 = getelementptr inbounds float addrspace(1)* %2147, i32 %2149 ; <float addrspace(1)*> [#uses=1]
  %2151 = bitcast float addrspace(1)* %2150 to <4 x float> addrspace(1)* ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> addrspace(1)* %2151, <4 x float> addrspace(1)** %gp.i22
  %2152 = load <4 x float>* %zr0.i2               ; <<4 x float>> [#uses=1]
  %2153 = load <4 x float> addrspace(1)** %gp.i22 ; <<4 x float> addrspace(1)*> [#uses=1]
  %2154 = getelementptr inbounds <4 x float> addrspace(1)* %2153, i32 0 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %2152, <4 x float> addrspace(1)* %2154
  %2155 = load <4 x float>* %zr1.i3               ; <<4 x float>> [#uses=1]
  %2156 = load <4 x float> addrspace(1)** %gp.i22 ; <<4 x float> addrspace(1)*> [#uses=1]
  %2157 = getelementptr inbounds <4 x float> addrspace(1)* %2156, i32 64 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %2155, <4 x float> addrspace(1)* %2157
  %2158 = load <4 x float>* %zr2.i4               ; <<4 x float>> [#uses=1]
  %2159 = load <4 x float> addrspace(1)** %gp.i22 ; <<4 x float> addrspace(1)*> [#uses=1]
  %2160 = getelementptr inbounds <4 x float> addrspace(1)* %2159, i32 128 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %2158, <4 x float> addrspace(1)* %2160
  %2161 = load <4 x float>* %zr3.i5               ; <<4 x float>> [#uses=1]
  %2162 = load <4 x float> addrspace(1)** %gp.i22 ; <<4 x float> addrspace(1)*> [#uses=1]
  %2163 = getelementptr inbounds <4 x float> addrspace(1)* %2162, i32 192 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %2161, <4 x float> addrspace(1)* %2163
  %2164 = load float addrspace(1)** %37           ; <float addrspace(1)*> [#uses=1]
  %2165 = load i32* %34                           ; <i32> [#uses=1]
  %2166 = shl i32 %2165, 2                        ; <i32> [#uses=1]
  %2167 = getelementptr inbounds float addrspace(1)* %2164, i32 %2166 ; <float addrspace(1)*> [#uses=1]
  %2168 = bitcast float addrspace(1)* %2167 to <4 x float> addrspace(1)* ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> addrspace(1)* %2168, <4 x float> addrspace(1)** %gp.i22
  %2169 = load <4 x float>* %zi0.i6               ; <<4 x float>> [#uses=1]
  %2170 = load <4 x float> addrspace(1)** %gp.i22 ; <<4 x float> addrspace(1)*> [#uses=1]
  %2171 = getelementptr inbounds <4 x float> addrspace(1)* %2170, i32 0 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %2169, <4 x float> addrspace(1)* %2171
  %2172 = load <4 x float>* %zi1.i7               ; <<4 x float>> [#uses=1]
  %2173 = load <4 x float> addrspace(1)** %gp.i22 ; <<4 x float> addrspace(1)*> [#uses=1]
  %2174 = getelementptr inbounds <4 x float> addrspace(1)* %2173, i32 64 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %2172, <4 x float> addrspace(1)* %2174
  %2175 = load <4 x float>* %zi2.i8               ; <<4 x float>> [#uses=1]
  %2176 = load <4 x float> addrspace(1)** %gp.i22 ; <<4 x float> addrspace(1)*> [#uses=1]
  %2177 = getelementptr inbounds <4 x float> addrspace(1)* %2176, i32 128 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %2175, <4 x float> addrspace(1)* %2177
  %2178 = load <4 x float>* %zi3.i9               ; <<4 x float>> [#uses=1]
  %2179 = load <4 x float> addrspace(1)** %gp.i22 ; <<4 x float> addrspace(1)*> [#uses=1]
  %2180 = getelementptr inbounds <4 x float> addrspace(1)* %2179, i32 192 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %2178, <4 x float> addrspace(1)* %2180
  ret void
}

declare i32 @get_global_id(i32)