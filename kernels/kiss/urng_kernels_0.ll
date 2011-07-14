; ModuleID = '-'
target datalayout = "E-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-n32"
target triple = "GHAL3D_2_1"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_noise_uniform_local_iv = internal addrspace(3) global [1024 x i32] zeroinitializer, align 4 ; <[1024 x i32] addrspace(3)*> [#uses=2]
@opencl_noise_uniform_locals = appending global [2 x i8*] [i8* bitcast ([1024 x i32] addrspace(3)* @opencl_noise_uniform_local_iv to i8*), i8* null], section "llvm.metadata" ; <[2 x i8*]*> [#uses=1]
@opencl_noise_uniform_parameters = appending global [92 x i8] c"uchar4 __attribute__((address_space(1))) *, uchar4 __attribute__((address_space(1))) *, int\00", section "llvm.metadata" ; <[92 x i8]*> [#uses=1]
@opencl_metadata = appending global [1 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (<4 x i8> addrspace(1)*, <4 x i8> addrspace(1)*, i32)* @noise_uniform to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([2 x i8*]* @opencl_noise_uniform_locals to i8*), i8* getelementptr inbounds ([92 x i8]* @opencl_noise_uniform_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[1 x %opencl_metadata_type]*> [#uses=0]

define float @ran1(i32 %idum, i32 addrspace(3)* %iv) nounwind {
  %1 = alloca float, align 4                      ; <float*> [#uses=2]
  %2 = alloca i32, align 4                        ; <i32*> [#uses=14]
  %3 = alloca i32 addrspace(3)*, align 16         ; <i32 addrspace(3)**> [#uses=4]
  %j = alloca i32, align 4                        ; <i32*> [#uses=8]
  %k = alloca i32, align 4                        ; <i32*> [#uses=6]
  %iy = alloca i32, align 4                       ; <i32*> [#uses=5]
  %tid = alloca i32, align 4                      ; <i32*> [#uses=4]
  store i32 %idum, i32* %2
  store i32 addrspace(3)* %iv, i32 addrspace(3)** %3
  store i32 0, i32* %iy
  %4 = call i32 @get_local_id(i32 0)              ; <i32> [#uses=1]
  store i32 %4, i32* %tid
  store i32 16, i32* %j
  br label %5

; <label>:5                                       ; preds = %36, %0
  %6 = load i32* %j                               ; <i32> [#uses=1]
  %7 = icmp sge i32 %6, 0                         ; <i1> [#uses=1]
  br i1 %7, label %8, label %39

; <label>:8                                       ; preds = %5
  %9 = load i32* %2                               ; <i32> [#uses=1]
  %10 = sdiv i32 %9, 127773                       ; <i32> [#uses=1]
  store i32 %10, i32* %k
  %11 = load i32* %2                              ; <i32> [#uses=1]
  %12 = load i32* %k                              ; <i32> [#uses=1]
  %13 = mul i32 %12, 127773                       ; <i32> [#uses=1]
  %14 = sub i32 %11, %13                          ; <i32> [#uses=1]
  %15 = mul i32 16807, %14                        ; <i32> [#uses=1]
  %16 = load i32* %k                              ; <i32> [#uses=1]
  %17 = mul i32 2836, %16                         ; <i32> [#uses=1]
  %18 = sub i32 %15, %17                          ; <i32> [#uses=1]
  store i32 %18, i32* %2
  %19 = load i32* %2                              ; <i32> [#uses=1]
  %20 = icmp slt i32 %19, 0                       ; <i1> [#uses=1]
  br i1 %20, label %21, label %24

; <label>:21                                      ; preds = %8
  %22 = load i32* %2                              ; <i32> [#uses=1]
  %23 = add nsw i32 %22, 2147483647               ; <i32> [#uses=1]
  store i32 %23, i32* %2
  br label %24

; <label>:24                                      ; preds = %21, %8
  %25 = load i32* %j                              ; <i32> [#uses=1]
  %26 = icmp slt i32 %25, 16                      ; <i1> [#uses=1]
  br i1 %26, label %27, label %35

; <label>:27                                      ; preds = %24
  %28 = load i32* %2                              ; <i32> [#uses=1]
  %29 = load i32* %tid                            ; <i32> [#uses=1]
  %30 = mul i32 16, %29                           ; <i32> [#uses=1]
  %31 = load i32* %j                              ; <i32> [#uses=1]
  %32 = add nsw i32 %30, %31                      ; <i32> [#uses=1]
  %33 = load i32 addrspace(3)** %3                ; <i32 addrspace(3)*> [#uses=1]
  %34 = getelementptr inbounds i32 addrspace(3)* %33, i32 %32 ; <i32 addrspace(3)*> [#uses=1]
  store i32 %28, i32 addrspace(3)* %34
  br label %35

; <label>:35                                      ; preds = %27, %24
  br label %36

; <label>:36                                      ; preds = %35
  %37 = load i32* %j                              ; <i32> [#uses=1]
  %38 = add nsw i32 %37, -1                       ; <i32> [#uses=1]
  store i32 %38, i32* %j
  br label %5

; <label>:39                                      ; preds = %5
  %40 = load i32* %tid                            ; <i32> [#uses=1]
  %41 = mul i32 16, %40                           ; <i32> [#uses=1]
  %42 = load i32 addrspace(3)** %3                ; <i32 addrspace(3)*> [#uses=1]
  %43 = getelementptr inbounds i32 addrspace(3)* %42, i32 %41 ; <i32 addrspace(3)*> [#uses=1]
  %44 = load i32 addrspace(3)* %43                ; <i32> [#uses=1]
  store i32 %44, i32* %iy
  %45 = load i32* %2                              ; <i32> [#uses=1]
  %46 = sdiv i32 %45, 127773                      ; <i32> [#uses=1]
  store i32 %46, i32* %k
  %47 = load i32* %2                              ; <i32> [#uses=1]
  %48 = load i32* %k                              ; <i32> [#uses=1]
  %49 = mul i32 %48, 127773                       ; <i32> [#uses=1]
  %50 = sub i32 %47, %49                          ; <i32> [#uses=1]
  %51 = mul i32 16807, %50                        ; <i32> [#uses=1]
  %52 = load i32* %k                              ; <i32> [#uses=1]
  %53 = mul i32 2836, %52                         ; <i32> [#uses=1]
  %54 = sub i32 %51, %53                          ; <i32> [#uses=1]
  store i32 %54, i32* %2
  %55 = load i32* %2                              ; <i32> [#uses=1]
  %56 = icmp slt i32 %55, 0                       ; <i1> [#uses=1]
  br i1 %56, label %57, label %60

; <label>:57                                      ; preds = %39
  %58 = load i32* %2                              ; <i32> [#uses=1]
  %59 = add nsw i32 %58, 2147483647               ; <i32> [#uses=1]
  store i32 %59, i32* %2
  br label %60

; <label>:60                                      ; preds = %57, %39
  %61 = load i32* %iy                             ; <i32> [#uses=1]
  %62 = sdiv i32 %61, 134217728                   ; <i32> [#uses=1]
  store i32 %62, i32* %j
  %63 = load i32* %tid                            ; <i32> [#uses=1]
  %64 = mul i32 16, %63                           ; <i32> [#uses=1]
  %65 = load i32* %j                              ; <i32> [#uses=1]
  %66 = add nsw i32 %64, %65                      ; <i32> [#uses=1]
  %67 = load i32 addrspace(3)** %3                ; <i32 addrspace(3)*> [#uses=1]
  %68 = getelementptr inbounds i32 addrspace(3)* %67, i32 %66 ; <i32 addrspace(3)*> [#uses=1]
  %69 = load i32 addrspace(3)* %68                ; <i32> [#uses=1]
  store i32 %69, i32* %iy
  %70 = load i32* %iy                             ; <i32> [#uses=1]
  %71 = sitofp i32 %70 to float                   ; <float> [#uses=1]
  %72 = fmul float 0x3E00000000000000, %71        ; <float> [#uses=1]
  store float %72, float* %1
  %73 = load float* %1                            ; <float> [#uses=1]
  ret float %73
}

declare i32 @get_local_id(i32)

define void @noise_uniform(<4 x i8> addrspace(1)* %inputImage, <4 x i8> addrspace(1)* %outputImage, i32 %factor) nounwind {
  %1 = alloca <4 x i8> addrspace(1)*, align 16    ; <<4 x i8> addrspace(1)**> [#uses=2]
  %2 = alloca <4 x i8> addrspace(1)*, align 16    ; <<4 x i8> addrspace(1)**> [#uses=2]
  %3 = alloca i32, align 4                        ; <i32*> [#uses=2]
  %pos = alloca i32, align 4                      ; <i32*> [#uses=3]
  %temp = alloca <4 x float>, align 16            ; <<4 x float>*> [#uses=6]
  %avg = alloca float, align 4                    ; <float*> [#uses=2]
  %dev = alloca float, align 4                    ; <float*> [#uses=7]
  %4 = alloca <4 x float>, align 16               ; <<4 x float>*> [#uses=2]
  store <4 x i8> addrspace(1)* %inputImage, <4 x i8> addrspace(1)** %1
  store <4 x i8> addrspace(1)* %outputImage, <4 x i8> addrspace(1)** %2
  store i32 %factor, i32* %3
  %5 = call i32 @get_global_id(i32 0)             ; <i32> [#uses=1]
  %6 = call i32 @get_global_id(i32 1)             ; <i32> [#uses=1]
  %7 = call i32 @get_global_size(i32 0)           ; <i32> [#uses=1]
  %8 = mul i32 %6, %7                             ; <i32> [#uses=1]
  %9 = add i32 %5, %8                             ; <i32> [#uses=1]
  store i32 %9, i32* %pos
  %10 = load i32* %pos                            ; <i32> [#uses=1]
  %11 = load <4 x i8> addrspace(1)** %1           ; <<4 x i8> addrspace(1)*> [#uses=1]
  %12 = getelementptr inbounds <4 x i8> addrspace(1)* %11, i32 %10 ; <<4 x i8> addrspace(1)*> [#uses=1]
  %13 = load <4 x i8> addrspace(1)* %12           ; <<4 x i8>> [#uses=1]
  %14 = call <4 x float> @_Z14convert_float4U8__vector4h(<4 x i8> %13) ; <<4 x float>> [#uses=1]
  store <4 x float> %14, <4 x float>* %temp
  %15 = load <4 x float>* %temp                   ; <<4 x float>> [#uses=1]
  %16 = extractelement <4 x float> %15, i32 0     ; <float> [#uses=1]
  %17 = load <4 x float>* %temp                   ; <<4 x float>> [#uses=1]
  %18 = extractelement <4 x float> %17, i32 1     ; <float> [#uses=1]
  %19 = fadd float %16, %18                       ; <float> [#uses=1]
  %20 = load <4 x float>* %temp                   ; <<4 x float>> [#uses=1]
  %21 = extractelement <4 x float> %20, i32 2     ; <float> [#uses=1]
  %22 = fadd float %19, %21                       ; <float> [#uses=1]
  %23 = load <4 x float>* %temp                   ; <<4 x float>> [#uses=1]
  %24 = extractelement <4 x float> %23, i32 1     ; <float> [#uses=1]
  %25 = fadd float %22, %24                       ; <float> [#uses=1]
  %26 = fdiv float %25, 4.000000e+000             ; <float> [#uses=1]
  store float %26, float* %avg
  %27 = load float* %avg                          ; <float> [#uses=1]
  %28 = fsub float -0.000000e+000, %27            ; <float> [#uses=1]
  %29 = fptosi float %28 to i32                   ; <i32> [#uses=1]
  %30 = call float @ran1(i32 %29, i32 addrspace(3)* getelementptr inbounds ([1024 x i32] addrspace(3)* @opencl_noise_uniform_local_iv, i32 0, i32 0)) ; <float> [#uses=1]
  store float %30, float* %dev
  %31 = load float* %dev                          ; <float> [#uses=1]
  %32 = fsub float %31, 0x3FE19999A0000000        ; <float> [#uses=1]
  %33 = load i32* %3                              ; <i32> [#uses=1]
  %34 = sitofp i32 %33 to float                   ; <float> [#uses=1]
  %35 = fmul float %32, %34                       ; <float> [#uses=1]
  store float %35, float* %dev
  %36 = load <4 x float>* %temp                   ; <<4 x float>> [#uses=1]
  %37 = load float* %dev                          ; <float> [#uses=1]
  %38 = insertelement <4 x float> undef, float %37, i32 0 ; <<4 x float>> [#uses=1]
  %39 = load float* %dev                          ; <float> [#uses=1]
  %40 = insertelement <4 x float> %38, float %39, i32 1 ; <<4 x float>> [#uses=1]
  %41 = load float* %dev                          ; <float> [#uses=1]
  %42 = insertelement <4 x float> %40, float %41, i32 2 ; <<4 x float>> [#uses=1]
  %43 = load float* %dev                          ; <float> [#uses=1]
  %44 = insertelement <4 x float> %42, float %43, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %44, <4 x float>* %4
  %45 = load <4 x float>* %4                      ; <<4 x float>> [#uses=1]
  %46 = fadd <4 x float> %36, %45                 ; <<4 x float>> [#uses=1]
  %47 = call <4 x i8> @_Z18convert_uchar4_satU8__vector4f(<4 x float> %46) ; <<4 x i8>> [#uses=1]
  %48 = load i32* %pos                            ; <i32> [#uses=1]
  %49 = load <4 x i8> addrspace(1)** %2           ; <<4 x i8> addrspace(1)*> [#uses=1]
  %50 = getelementptr inbounds <4 x i8> addrspace(1)* %49, i32 %48 ; <<4 x i8> addrspace(1)*> [#uses=1]
  store <4 x i8> %47, <4 x i8> addrspace(1)* %50
  ret void
}

declare i32 @get_global_id(i32)

declare i32 @get_global_size(i32)

declare <4 x float> @_Z14convert_float4U8__vector4h(<4 x i8>)

declare <4 x i8> @_Z18convert_uchar4_satU8__vector4f(<4 x float>)