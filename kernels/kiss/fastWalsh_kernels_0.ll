; ModuleID = '-'
target datalayout = "E-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-n32"
target triple = "GHAL3D_2_1"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_fastWalshTransform_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_fastWalshTransform_parameters = appending global [53 x i8] c"float __attribute__((address_space(1))) *, int const\00", section "llvm.metadata" ; <[53 x i8]*> [#uses=1]
@opencl_metadata = appending global [1 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (float addrspace(1)*, i32)* @fastWalshTransform to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_fastWalshTransform_locals to i8*), i8* getelementptr inbounds ([53 x i8]* @opencl_fastWalshTransform_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[1 x %opencl_metadata_type]*> [#uses=0]

define void @fastWalshTransform(float addrspace(1)* %tArray, i32 %step) nounwind {
  %1 = alloca float addrspace(1)*, align 16       ; <float addrspace(1)**> [#uses=5]
  %2 = alloca i32, align 4                        ; <i32*> [#uses=5]
  %tid = alloca i32, align 4                      ; <i32*> [#uses=3]
  %group = alloca i32, align 4                    ; <i32*> [#uses=2]
  %pair = alloca i32, align 4                     ; <i32*> [#uses=4]
  %match = alloca i32, align 4                    ; <i32*> [#uses=3]
  %T1 = alloca float, align 4                     ; <float*> [#uses=3]
  %T2 = alloca float, align 4                     ; <float*> [#uses=3]
  store float addrspace(1)* %tArray, float addrspace(1)** %1
  store i32 %step, i32* %2
  %3 = call i32 @get_global_id(i32 0)             ; <i32> [#uses=1]
  store i32 %3, i32* %tid
  %4 = load i32* %tid                             ; <i32> [#uses=1]
  %5 = load i32* %2                               ; <i32> [#uses=1]
  %6 = urem i32 %4, %5                            ; <i32> [#uses=1]
  store i32 %6, i32* %group
  %7 = load i32* %2                               ; <i32> [#uses=1]
  %8 = mul i32 2, %7                              ; <i32> [#uses=1]
  %9 = load i32* %tid                             ; <i32> [#uses=1]
  %10 = load i32* %2                              ; <i32> [#uses=1]
  %11 = udiv i32 %9, %10                          ; <i32> [#uses=1]
  %12 = mul i32 %8, %11                           ; <i32> [#uses=1]
  %13 = load i32* %group                          ; <i32> [#uses=1]
  %14 = add i32 %12, %13                          ; <i32> [#uses=1]
  store i32 %14, i32* %pair
  %15 = load i32* %pair                           ; <i32> [#uses=1]
  %16 = load i32* %2                              ; <i32> [#uses=1]
  %17 = add i32 %15, %16                          ; <i32> [#uses=1]
  store i32 %17, i32* %match
  %18 = load i32* %pair                           ; <i32> [#uses=1]
  %19 = load float addrspace(1)** %1              ; <float addrspace(1)*> [#uses=1]
  %20 = getelementptr inbounds float addrspace(1)* %19, i32 %18 ; <float addrspace(1)*> [#uses=1]
  %21 = load float addrspace(1)* %20              ; <float> [#uses=1]
  store float %21, float* %T1
  %22 = load i32* %match                          ; <i32> [#uses=1]
  %23 = load float addrspace(1)** %1              ; <float addrspace(1)*> [#uses=1]
  %24 = getelementptr inbounds float addrspace(1)* %23, i32 %22 ; <float addrspace(1)*> [#uses=1]
  %25 = load float addrspace(1)* %24              ; <float> [#uses=1]
  store float %25, float* %T2
  %26 = load float* %T1                           ; <float> [#uses=1]
  %27 = load float* %T2                           ; <float> [#uses=1]
  %28 = fadd float %26, %27                       ; <float> [#uses=1]
  %29 = load i32* %pair                           ; <i32> [#uses=1]
  %30 = load float addrspace(1)** %1              ; <float addrspace(1)*> [#uses=1]
  %31 = getelementptr inbounds float addrspace(1)* %30, i32 %29 ; <float addrspace(1)*> [#uses=1]
  store float %28, float addrspace(1)* %31
  %32 = load float* %T1                           ; <float> [#uses=1]
  %33 = load float* %T2                           ; <float> [#uses=1]
  %34 = fsub float %32, %33                       ; <float> [#uses=1]
  %35 = load i32* %match                          ; <i32> [#uses=1]
  %36 = load float addrspace(1)** %1              ; <float addrspace(1)*> [#uses=1]
  %37 = getelementptr inbounds float addrspace(1)* %36, i32 %35 ; <float addrspace(1)*> [#uses=1]
  store float %34, float addrspace(1)* %37
  ret void
}

declare i32 @get_global_id(i32)