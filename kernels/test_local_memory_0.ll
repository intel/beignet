; ModuleID = '-'
target datalayout = "E-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-n32"
target triple = "GHAL3D_2_1"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_test_local_memory_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_test_local_memory_parameters = appending global [122 x i8] c"int __attribute__((address_space(1))) *, int __attribute__((address_space(3))) *, int __attribute__((address_space(3))) *\00", section "llvm.metadata" ; <[122 x i8]*> [#uses=1]
@opencl_metadata = appending global [1 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (i32 addrspace(1)*, i32 addrspace(3)*, i32 addrspace(3)*)* @test_local_memory to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_test_local_memory_locals to i8*), i8* getelementptr inbounds ([122 x i8]* @opencl_test_local_memory_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[1 x %opencl_metadata_type]*> [#uses=0]

define void @test_local_memory(i32 addrspace(1)* %dst, i32 addrspace(3)* %local0, i32 addrspace(3)* %local1) nounwind {
  %1 = alloca i32 addrspace(1)*, align 16         ; <i32 addrspace(1)**> [#uses=2]
  %2 = alloca i32 addrspace(3)*, align 16         ; <i32 addrspace(3)**> [#uses=3]
  %3 = alloca i32 addrspace(3)*, align 16         ; <i32 addrspace(3)**> [#uses=3]
  %id = alloca i32, align 4                       ; <i32*> [#uses=6]
  %tid = alloca i32, align 4                      ; <i32*> [#uses=2]
  store i32 addrspace(1)* %dst, i32 addrspace(1)** %1
  store i32 addrspace(3)* %local0, i32 addrspace(3)** %2
  store i32 addrspace(3)* %local1, i32 addrspace(3)** %3
  %4 = call i32 @get_local_id(i32 0)              ; <i32> [#uses=1]
  store i32 %4, i32* %id
  %5 = call i32 @get_global_id(i32 0)             ; <i32> [#uses=1]
  store i32 %5, i32* %tid
  %6 = load i32* %id                              ; <i32> [#uses=1]
  %7 = load i32* %id                              ; <i32> [#uses=1]
  %8 = load i32 addrspace(3)** %3                 ; <i32 addrspace(3)*> [#uses=1]
  %9 = getelementptr inbounds i32 addrspace(3)* %8, i32 %7 ; <i32 addrspace(3)*> [#uses=2]
  store i32 %6, i32 addrspace(3)* %9
  %10 = load i32 addrspace(3)* %9                 ; <i32> [#uses=1]
  %11 = load i32* %id                             ; <i32> [#uses=1]
  %12 = load i32 addrspace(3)** %2                ; <i32 addrspace(3)*> [#uses=1]
  %13 = getelementptr inbounds i32 addrspace(3)* %12, i32 %11 ; <i32 addrspace(3)*> [#uses=1]
  store i32 %10, i32 addrspace(3)* %13
  call void @barrier(i32 1)
  %14 = load i32* %id                             ; <i32> [#uses=1]
  %15 = sub i32 32, %14                           ; <i32> [#uses=1]
  %16 = sub i32 %15, 1                            ; <i32> [#uses=1]
  %17 = load i32 addrspace(3)** %2                ; <i32 addrspace(3)*> [#uses=1]
  %18 = getelementptr inbounds i32 addrspace(3)* %17, i32 %16 ; <i32 addrspace(3)*> [#uses=1]
  %19 = load i32 addrspace(3)* %18                ; <i32> [#uses=1]
  %20 = load i32* %id                             ; <i32> [#uses=1]
  %21 = sub i32 32, %20                           ; <i32> [#uses=1]
  %22 = sub i32 %21, 1                            ; <i32> [#uses=1]
  %23 = load i32 addrspace(3)** %3                ; <i32 addrspace(3)*> [#uses=1]
  %24 = getelementptr inbounds i32 addrspace(3)* %23, i32 %22 ; <i32 addrspace(3)*> [#uses=1]
  %25 = load i32 addrspace(3)* %24                ; <i32> [#uses=1]
  %26 = add nsw i32 %19, %25                      ; <i32> [#uses=1]
  %27 = load i32* %tid                            ; <i32> [#uses=1]
  %28 = load i32 addrspace(1)** %1                ; <i32 addrspace(1)*> [#uses=1]
  %29 = getelementptr inbounds i32 addrspace(1)* %28, i32 %27 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %26, i32 addrspace(1)* %29
  ret void
}

declare i32 @get_local_id(i32)

declare i32 @get_global_id(i32)

declare void @barrier(i32)