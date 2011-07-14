; ModuleID = '-'
target datalayout = "E-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-n32"
target triple = "GHAL3D_2_1"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_DCT_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_DCT_parameters = appending global [207 x i8] c"float __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, float __attribute__((address_space(3))) *, uint const, uint const, uint const\00", section "llvm.metadata" ; <[207 x i8]*> [#uses=1]
@opencl_metadata = appending global [1 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, float addrspace(3)*, i32, i32, i32)* @DCT to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_DCT_locals to i8*), i8* getelementptr inbounds ([207 x i8]* @opencl_DCT_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[1 x %opencl_metadata_type]*> [#uses=0]

define i32 @getIdx(i32 %blockIdx, i32 %blockIdy, i32 %localIdx, i32 %localIdy, i32 %blockWidth, i32 %globalWidth) nounwind {
  %1 = alloca i32, align 4                        ; <i32*> [#uses=2]
  %2 = alloca i32, align 4                        ; <i32*> [#uses=2]
  %3 = alloca i32, align 4                        ; <i32*> [#uses=2]
  %4 = alloca i32, align 4                        ; <i32*> [#uses=2]
  %5 = alloca i32, align 4                        ; <i32*> [#uses=2]
  %6 = alloca i32, align 4                        ; <i32*> [#uses=3]
  %7 = alloca i32, align 4                        ; <i32*> [#uses=2]
  %globalIdx = alloca i32, align 4                ; <i32*> [#uses=2]
  %globalIdy = alloca i32, align 4                ; <i32*> [#uses=2]
  store i32 %blockIdx, i32* %2
  store i32 %blockIdy, i32* %3
  store i32 %localIdx, i32* %4
  store i32 %localIdy, i32* %5
  store i32 %blockWidth, i32* %6
  store i32 %globalWidth, i32* %7
  %8 = load i32* %2                               ; <i32> [#uses=1]
  %9 = load i32* %6                               ; <i32> [#uses=1]
  %10 = mul i32 %8, %9                            ; <i32> [#uses=1]
  %11 = load i32* %4                              ; <i32> [#uses=1]
  %12 = add i32 %10, %11                          ; <i32> [#uses=1]
  store i32 %12, i32* %globalIdx
  %13 = load i32* %3                              ; <i32> [#uses=1]
  %14 = load i32* %6                              ; <i32> [#uses=1]
  %15 = mul i32 %13, %14                          ; <i32> [#uses=1]
  %16 = load i32* %5                              ; <i32> [#uses=1]
  %17 = add i32 %15, %16                          ; <i32> [#uses=1]
  store i32 %17, i32* %globalIdy
  %18 = load i32* %globalIdy                      ; <i32> [#uses=1]
  %19 = load i32* %7                              ; <i32> [#uses=1]
  %20 = mul i32 %18, %19                          ; <i32> [#uses=1]
  %21 = load i32* %globalIdx                      ; <i32> [#uses=1]
  %22 = add i32 %20, %21                          ; <i32> [#uses=1]
  store i32 %22, i32* %1
  %23 = load i32* %1                              ; <i32> [#uses=1]
  ret i32 %23
}

define void @DCT(float addrspace(1)* %output, float addrspace(1)* %input, float addrspace(1)* %dct8x8, float addrspace(3)* %inter, i32 %width, i32 %blockWidth, i32 %inverse) nounwind {
  %1 = alloca float addrspace(1)*, align 16       ; <float addrspace(1)**> [#uses=2]
  %2 = alloca float addrspace(1)*, align 16       ; <float addrspace(1)**> [#uses=2]
  %3 = alloca float addrspace(1)*, align 16       ; <float addrspace(1)**> [#uses=3]
  %4 = alloca float addrspace(3)*, align 16       ; <float addrspace(3)**> [#uses=3]
  %5 = alloca i32, align 4                        ; <i32*> [#uses=3]
  %6 = alloca i32, align 4                        ; <i32*> [#uses=10]
  %7 = alloca i32, align 4                        ; <i32*> [#uses=3]
  %globalIdx = alloca i32, align 4                ; <i32*> [#uses=2]
  %globalIdy = alloca i32, align 4                ; <i32*> [#uses=2]
  %groupIdx = alloca i32, align 4                 ; <i32*> [#uses=2]
  %groupIdy = alloca i32, align 4                 ; <i32*> [#uses=2]
  %i = alloca i32, align 4                        ; <i32*> [#uses=5]
  %j = alloca i32, align 4                        ; <i32*> [#uses=5]
  %idx = alloca i32, align 4                      ; <i32*> [#uses=2]
  %acc = alloca float, align 4                    ; <float*> [#uses=8]
  %k = alloca i32, align 4                        ; <i32*> [#uses=7]
  %index1 = alloca i32, align 4                   ; <i32*> [#uses=2]
  %index2 = alloca i32, align 4                   ; <i32*> [#uses=2]
  %k1 = alloca i32, align 4                       ; <i32*> [#uses=7]
  %index12 = alloca i32, align 4                  ; <i32*> [#uses=2]
  %index23 = alloca i32, align 4                  ; <i32*> [#uses=2]
  store float addrspace(1)* %output, float addrspace(1)** %1
  store float addrspace(1)* %input, float addrspace(1)** %2
  store float addrspace(1)* %dct8x8, float addrspace(1)** %3
  store float addrspace(3)* %inter, float addrspace(3)** %4
  store i32 %width, i32* %5
  store i32 %blockWidth, i32* %6
  store i32 %inverse, i32* %7
  %8 = call i32 @get_global_id(i32 0)             ; <i32> [#uses=1]
  store i32 %8, i32* %globalIdx
  %9 = call i32 @get_global_id(i32 1)             ; <i32> [#uses=1]
  store i32 %9, i32* %globalIdy
  %10 = call i32 @get_group_id(i32 0)             ; <i32> [#uses=1]
  store i32 %10, i32* %groupIdx
  %11 = call i32 @get_group_id(i32 1)             ; <i32> [#uses=1]
  store i32 %11, i32* %groupIdy
  %12 = call i32 @get_local_id(i32 0)             ; <i32> [#uses=1]
  store i32 %12, i32* %i
  %13 = call i32 @get_local_id(i32 1)             ; <i32> [#uses=1]
  store i32 %13, i32* %j
  %14 = load i32* %globalIdy                      ; <i32> [#uses=1]
  %15 = load i32* %5                              ; <i32> [#uses=1]
  %16 = mul i32 %14, %15                          ; <i32> [#uses=1]
  %17 = load i32* %globalIdx                      ; <i32> [#uses=1]
  %18 = add i32 %16, %17                          ; <i32> [#uses=1]
  store i32 %18, i32* %idx
  store float 0.000000e+000, float* %acc
  store i32 0, i32* %k
  br label %19

; <label>:19                                      ; preds = %55, %0
  %20 = load i32* %k                              ; <i32> [#uses=1]
  %21 = load i32* %6                              ; <i32> [#uses=1]
  %22 = icmp ult i32 %20, %21                     ; <i1> [#uses=1]
  br i1 %22, label %23, label %58

; <label>:23                                      ; preds = %19
  %24 = load i32* %7                              ; <i32> [#uses=1]
  %25 = load i32* %i                              ; <i32> [#uses=1]
  %26 = load i32* %6                              ; <i32> [#uses=1]
  %27 = mul i32 %25, %26                          ; <i32> [#uses=1]
  %28 = load i32* %k                              ; <i32> [#uses=1]
  %29 = add i32 %27, %28                          ; <i32> [#uses=1]
  %30 = load i32* %k                              ; <i32> [#uses=1]
  %31 = load i32* %6                              ; <i32> [#uses=1]
  %32 = mul i32 %30, %31                          ; <i32> [#uses=1]
  %33 = load i32* %i                              ; <i32> [#uses=1]
  %34 = add i32 %32, %33                          ; <i32> [#uses=1]
  %35 = icmp ne i32 %24, 0                        ; <i1> [#uses=1]
  %36 = select i1 %35, i32 %29, i32 %34           ; <i32> [#uses=1]
  store i32 %36, i32* %index1
  %37 = load i32* %groupIdx                       ; <i32> [#uses=1]
  %38 = load i32* %groupIdy                       ; <i32> [#uses=1]
  %39 = load i32* %j                              ; <i32> [#uses=1]
  %40 = load i32* %k                              ; <i32> [#uses=1]
  %41 = load i32* %6                              ; <i32> [#uses=1]
  %42 = load i32* %5                              ; <i32> [#uses=1]
  %43 = call i32 @getIdx(i32 %37, i32 %38, i32 %39, i32 %40, i32 %41, i32 %42) ; <i32> [#uses=1]
  store i32 %43, i32* %index2
  %44 = load i32* %index1                         ; <i32> [#uses=1]
  %45 = load float addrspace(1)** %3              ; <float addrspace(1)*> [#uses=1]
  %46 = getelementptr inbounds float addrspace(1)* %45, i32 %44 ; <float addrspace(1)*> [#uses=1]
  %47 = load float addrspace(1)* %46              ; <float> [#uses=1]
  %48 = load i32* %index2                         ; <i32> [#uses=1]
  %49 = load float addrspace(1)** %2              ; <float addrspace(1)*> [#uses=1]
  %50 = getelementptr inbounds float addrspace(1)* %49, i32 %48 ; <float addrspace(1)*> [#uses=1]
  %51 = load float addrspace(1)* %50              ; <float> [#uses=1]
  %52 = fmul float %47, %51                       ; <float> [#uses=1]
  %53 = load float* %acc                          ; <float> [#uses=1]
  %54 = fadd float %53, %52                       ; <float> [#uses=1]
  store float %54, float* %acc
  br label %55

; <label>:55                                      ; preds = %23
  %56 = load i32* %k                              ; <i32> [#uses=1]
  %57 = add i32 %56, 1                            ; <i32> [#uses=1]
  store i32 %57, i32* %k
  br label %19

; <label>:58                                      ; preds = %19
  %59 = load float* %acc                          ; <float> [#uses=1]
  %60 = load i32* %j                              ; <i32> [#uses=1]
  %61 = load i32* %6                              ; <i32> [#uses=1]
  %62 = mul i32 %60, %61                          ; <i32> [#uses=1]
  %63 = load i32* %i                              ; <i32> [#uses=1]
  %64 = add i32 %62, %63                          ; <i32> [#uses=1]
  %65 = load float addrspace(3)** %4              ; <float addrspace(3)*> [#uses=1]
  %66 = getelementptr inbounds float addrspace(3)* %65, i32 %64 ; <float addrspace(3)*> [#uses=1]
  store float %59, float addrspace(3)* %66
  call void @barrier(i32 1)
  store float 0.000000e+000, float* %acc
  store i32 0, i32* %k1
  br label %67

; <label>:67                                      ; preds = %101, %58
  %68 = load i32* %k1                             ; <i32> [#uses=1]
  %69 = load i32* %6                              ; <i32> [#uses=1]
  %70 = icmp ult i32 %68, %69                     ; <i1> [#uses=1]
  br i1 %70, label %71, label %104

; <label>:71                                      ; preds = %67
  %72 = load i32* %i                              ; <i32> [#uses=1]
  %73 = load i32* %6                              ; <i32> [#uses=1]
  %74 = mul i32 %72, %73                          ; <i32> [#uses=1]
  %75 = load i32* %k1                             ; <i32> [#uses=1]
  %76 = add i32 %74, %75                          ; <i32> [#uses=1]
  store i32 %76, i32* %index12
  %77 = load i32* %7                              ; <i32> [#uses=1]
  %78 = load i32* %j                              ; <i32> [#uses=1]
  %79 = load i32* %6                              ; <i32> [#uses=1]
  %80 = mul i32 %78, %79                          ; <i32> [#uses=1]
  %81 = load i32* %k1                             ; <i32> [#uses=1]
  %82 = add i32 %80, %81                          ; <i32> [#uses=1]
  %83 = load i32* %k1                             ; <i32> [#uses=1]
  %84 = load i32* %6                              ; <i32> [#uses=1]
  %85 = mul i32 %83, %84                          ; <i32> [#uses=1]
  %86 = load i32* %j                              ; <i32> [#uses=1]
  %87 = add i32 %85, %86                          ; <i32> [#uses=1]
  %88 = icmp ne i32 %77, 0                        ; <i1> [#uses=1]
  %89 = select i1 %88, i32 %82, i32 %87           ; <i32> [#uses=1]
  store i32 %89, i32* %index23
  %90 = load i32* %index12                        ; <i32> [#uses=1]
  %91 = load float addrspace(3)** %4              ; <float addrspace(3)*> [#uses=1]
  %92 = getelementptr inbounds float addrspace(3)* %91, i32 %90 ; <float addrspace(3)*> [#uses=1]
  %93 = load float addrspace(3)* %92              ; <float> [#uses=1]
  %94 = load i32* %index23                        ; <i32> [#uses=1]
  %95 = load float addrspace(1)** %3              ; <float addrspace(1)*> [#uses=1]
  %96 = getelementptr inbounds float addrspace(1)* %95, i32 %94 ; <float addrspace(1)*> [#uses=1]
  %97 = load float addrspace(1)* %96              ; <float> [#uses=1]
  %98 = fmul float %93, %97                       ; <float> [#uses=1]
  %99 = load float* %acc                          ; <float> [#uses=1]
  %100 = fadd float %99, %98                      ; <float> [#uses=1]
  store float %100, float* %acc
  br label %101

; <label>:101                                     ; preds = %71
  %102 = load i32* %k1                            ; <i32> [#uses=1]
  %103 = add i32 %102, 1                          ; <i32> [#uses=1]
  store i32 %103, i32* %k1
  br label %67

; <label>:104                                     ; preds = %67
  %105 = load float* %acc                         ; <float> [#uses=1]
  %106 = load i32* %idx                           ; <i32> [#uses=1]
  %107 = load float addrspace(1)** %1             ; <float addrspace(1)*> [#uses=1]
  %108 = getelementptr inbounds float addrspace(1)* %107, i32 %106 ; <float addrspace(1)*> [#uses=1]
  store float %105, float addrspace(1)* %108
  ret void
}

declare i32 @get_global_id(i32)

declare i32 @get_group_id(i32)

declare i32 @get_local_id(i32)

declare void @barrier(i32)