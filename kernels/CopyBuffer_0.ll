; ModuleID = '-'
target datalayout = "E-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-n32"
target triple = "GHAL3D_2_1"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_CopyBuffer_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_CopyBuffer_parameters = appending global [85 x i8] c"float __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *\00", section "llvm.metadata" ; <[85 x i8]*> [#uses=1]
@opencl_metadata = appending global [1 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (float addrspace(1)*, float addrspace(1)*)* @CopyBuffer to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_CopyBuffer_locals to i8*), i8* getelementptr inbounds ([85 x i8]* @opencl_CopyBuffer_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[1 x %opencl_metadata_type]*> [#uses=0]

define void @CopyBuffer(float addrspace(1)* %src, float addrspace(1)* %dst) nounwind {
  %1 = alloca float addrspace(1)*, align 16       ; <float addrspace(1)**> [#uses=2]
  %2 = alloca float addrspace(1)*, align 16       ; <float addrspace(1)**> [#uses=2]
  %id = alloca i32, align 4                       ; <i32*> [#uses=3]
  store float addrspace(1)* %src, float addrspace(1)** %1
  store float addrspace(1)* %dst, float addrspace(1)** %2
  %3 = call i32 @get_global_id(i32 0)             ; <i32> [#uses=1]
  store i32 %3, i32* %id
  %4 = load i32* %id                              ; <i32> [#uses=1]
  %5 = load float addrspace(1)** %1               ; <float addrspace(1)*> [#uses=1]
  %6 = getelementptr inbounds float addrspace(1)* %5, i32 %4 ; <float addrspace(1)*> [#uses=1]
  %7 = load float addrspace(1)* %6                ; <float> [#uses=1]
  %8 = load i32* %id                              ; <i32> [#uses=1]
  %9 = load float addrspace(1)** %2               ; <float addrspace(1)*> [#uses=1]
  %10 = getelementptr inbounds float addrspace(1)* %9, i32 %8 ; <float addrspace(1)*> [#uses=1]
  store float %7, float addrspace(1)* %10
  ret void
}

declare i32 @get_global_id(i32)