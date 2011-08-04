; ModuleID = '-'
target datalayout = "E-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-n32"
target triple = "GHAL3D_2_1"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@hop = addrspace(2) global [1 x i32] [i32 8], align 4 ; <[1 x i32] addrspace(2)*> [#uses=1]
@opencl_test_constant_memory_local_local1 = internal addrspace(3) global [32 x i32] zeroinitializer, align 4 ; <[32 x i32] addrspace(3)*> [#uses=2]
@opencl_test_constant_memory_locals = appending global [2 x i8*] [i8* bitcast ([32 x i32] addrspace(3)* @opencl_test_constant_memory_local_local1 to i8*), i8* null], section "llvm.metadata" ; <[2 x i8*]*> [#uses=1]
@opencl_test_constant_memory_parameters = appending global [122 x i8] c"int __attribute__((address_space(1))) *, int __attribute__((address_space(3))) *, int __attribute__((address_space(2))) *\00", section "llvm.metadata" ; <[122 x i8]*> [#uses=1]
@opencl_metadata = appending global [1 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (i32 addrspace(1)*, i32 addrspace(3)*, i32 addrspace(2)*)* @test_constant_memory to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([2 x i8*]* @opencl_test_constant_memory_locals to i8*), i8* getelementptr inbounds ([122 x i8]* @opencl_test_constant_memory_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[1 x %opencl_metadata_type]*> [#uses=0]

define void @test_constant_memory(i32 addrspace(1)* %dst, i32 addrspace(3)* %local0, i32 addrspace(2)* %constants) nounwind {
  %1 = alloca i32 addrspace(1)*, align 16         ; <i32 addrspace(1)**> [#uses=2]
  %2 = alloca i32 addrspace(3)*, align 16         ; <i32 addrspace(3)**> [#uses=3]
  %3 = alloca i32 addrspace(2)*, align 16         ; <i32 addrspace(2)**> [#uses=2]
  %id = alloca i32, align 4                       ; <i32*> [#uses=7]
  %tid = alloca i32, align 4                      ; <i32*> [#uses=2]
  store i32 addrspace(1)* %dst, i32 addrspace(1)** %1
  store i32 addrspace(3)* %local0, i32 addrspace(3)** %2
  store i32 addrspace(2)* %constants, i32 addrspace(2)** %3
  %4 = call i32 @get_local_id(i32 0)              ; <i32> [#uses=1]
  store i32 %4, i32* %id
  %5 = call i32 @get_global_id(i32 0)             ; <i32> [#uses=1]
  store i32 %5, i32* %tid
  %6 = load i32* %id                              ; <i32> [#uses=1]
  %7 = load i32* %id                              ; <i32> [#uses=1]
  %8 = getelementptr inbounds i32 addrspace(3)* getelementptr inbounds ([32 x i32] addrspace(3)* @opencl_test_constant_memory_local_local1, i32 0, i32 0), i32 %7 ; <i32 addrspace(3)*> [#uses=2]
  store i32 %6, i32 addrspace(3)* %8
  %9 = load i32 addrspace(3)* %8                  ; <i32> [#uses=1]
  %10 = load i32* %id                             ; <i32> [#uses=1]
  %11 = load i32 addrspace(3)** %2                ; <i32 addrspace(3)*> [#uses=1]
  %12 = getelementptr inbounds i32 addrspace(3)* %11, i32 %10 ; <i32 addrspace(3)*> [#uses=1]
  store i32 %9, i32 addrspace(3)* %12
  call void @barrier(i32 1)
  %13 = load i32* %id                             ; <i32> [#uses=1]
  %14 = sub i32 32, %13                           ; <i32> [#uses=1]
  %15 = sub i32 %14, 1                            ; <i32> [#uses=1]
  %16 = load i32 addrspace(3)** %2                ; <i32 addrspace(3)*> [#uses=1]
  %17 = getelementptr inbounds i32 addrspace(3)* %16, i32 %15 ; <i32 addrspace(3)*> [#uses=1]
  %18 = load i32 addrspace(3)* %17                ; <i32> [#uses=1]
  %19 = load i32* %id                             ; <i32> [#uses=1]
  %20 = sub i32 32, %19                           ; <i32> [#uses=1]
  %21 = sub i32 %20, 1                            ; <i32> [#uses=1]
  %22 = getelementptr inbounds i32 addrspace(3)* getelementptr inbounds ([32 x i32] addrspace(3)* @opencl_test_constant_memory_local_local1, i32 0, i32 0), i32 %21 ; <i32 addrspace(3)*> [#uses=1]
  %23 = load i32 addrspace(3)* %22                ; <i32> [#uses=1]
  %24 = add nsw i32 %18, %23                      ; <i32> [#uses=1]
  %25 = load i32* %id                             ; <i32> [#uses=1]
  %26 = load i32 addrspace(2)** %3                ; <i32 addrspace(2)*> [#uses=1]
  %27 = getelementptr inbounds i32 addrspace(2)* %26, i32 %25 ; <i32 addrspace(2)*> [#uses=1]
  %28 = load i32 addrspace(2)* %27                ; <i32> [#uses=1]
  %29 = add nsw i32 %24, %28                      ; <i32> [#uses=1]
  %30 = load i32 addrspace(2)* getelementptr inbounds ([1 x i32] addrspace(2)* @hop, i32 0, i32 0) ; <i32> [#uses=1]
  %31 = add nsw i32 %29, %30                      ; <i32> [#uses=1]
  %32 = load i32* %tid                            ; <i32> [#uses=1]
  %33 = load i32 addrspace(1)** %1                ; <i32 addrspace(1)*> [#uses=1]
  %34 = getelementptr inbounds i32 addrspace(1)* %33, i32 %32 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %31, i32 addrspace(1)* %34
  ret void
}

declare i32 @get_local_id(i32)

declare i32 @get_global_id(i32)

declare void @barrier(i32)