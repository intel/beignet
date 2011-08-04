; ModuleID = '-'
target datalayout = "E-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-n32"
target triple = "GHAL3D_2_1"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_test_static_local_memory_local_local0 = internal addrspace(3) global [32 x i32] zeroinitializer, align 4 ; <[32 x i32] addrspace(3)*> [#uses=2]
@opencl_test_static_local_memory_local_local1 = internal addrspace(3) global [32 x i32] zeroinitializer, align 4 ; <[32 x i32] addrspace(3)*> [#uses=2]
@opencl_test_static_local_memory_locals = appending global [3 x i8*] [i8* bitcast ([32 x i32] addrspace(3)* @opencl_test_static_local_memory_local_local0 to i8*), i8* bitcast ([32 x i32] addrspace(3)* @opencl_test_static_local_memory_local_local1 to i8*), i8* null], section "llvm.metadata" ; <[3 x i8*]*> [#uses=1]
@opencl_test_static_local_memory_parameters = appending global [40 x i8] c"int __attribute__((address_space(1))) *\00", section "llvm.metadata" ; <[40 x i8]*> [#uses=1]
@opencl_metadata = appending global [1 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (i32 addrspace(1)*)* @test_static_local_memory to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([3 x i8*]* @opencl_test_static_local_memory_locals to i8*), i8* getelementptr inbounds ([40 x i8]* @opencl_test_static_local_memory_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[1 x %opencl_metadata_type]*> [#uses=0]

define void @test_static_local_memory(i32 addrspace(1)* %dst) nounwind {
  %1 = alloca i32 addrspace(1)*, align 16         ; <i32 addrspace(1)**> [#uses=2]
  %id = alloca i32, align 4                       ; <i32*> [#uses=6]
  %tid = alloca i32, align 4                      ; <i32*> [#uses=2]
  store i32 addrspace(1)* %dst, i32 addrspace(1)** %1
  %2 = call i32 @get_local_id(i32 0)              ; <i32> [#uses=1]
  store i32 %2, i32* %id
  %3 = call i32 @get_global_id(i32 0)             ; <i32> [#uses=1]
  store i32 %3, i32* %tid
  %4 = load i32* %id                              ; <i32> [#uses=1]
  %5 = load i32* %id                              ; <i32> [#uses=1]
  %6 = getelementptr inbounds i32 addrspace(3)* getelementptr inbounds ([32 x i32] addrspace(3)* @opencl_test_static_local_memory_local_local1, i32 0, i32 0), i32 %5 ; <i32 addrspace(3)*> [#uses=2]
  store i32 %4, i32 addrspace(3)* %6
  %7 = load i32 addrspace(3)* %6                  ; <i32> [#uses=1]
  %8 = load i32* %id                              ; <i32> [#uses=1]
  %9 = getelementptr inbounds i32 addrspace(3)* getelementptr inbounds ([32 x i32] addrspace(3)* @opencl_test_static_local_memory_local_local0, i32 0, i32 0), i32 %8 ; <i32 addrspace(3)*> [#uses=1]
  store i32 %7, i32 addrspace(3)* %9
  call void @barrier(i32 1)
  %10 = load i32* %id                             ; <i32> [#uses=1]
  %11 = sub i32 32, %10                           ; <i32> [#uses=1]
  %12 = sub i32 %11, 1                            ; <i32> [#uses=1]
  %13 = getelementptr inbounds i32 addrspace(3)* getelementptr inbounds ([32 x i32] addrspace(3)* @opencl_test_static_local_memory_local_local0, i32 0, i32 0), i32 %12 ; <i32 addrspace(3)*> [#uses=1]
  %14 = load i32 addrspace(3)* %13                ; <i32> [#uses=1]
  %15 = load i32* %id                             ; <i32> [#uses=1]
  %16 = sub i32 32, %15                           ; <i32> [#uses=1]
  %17 = sub i32 %16, 1                            ; <i32> [#uses=1]
  %18 = getelementptr inbounds i32 addrspace(3)* getelementptr inbounds ([32 x i32] addrspace(3)* @opencl_test_static_local_memory_local_local1, i32 0, i32 0), i32 %17 ; <i32 addrspace(3)*> [#uses=1]
  %19 = load i32 addrspace(3)* %18                ; <i32> [#uses=1]
  %20 = add nsw i32 %14, %19                      ; <i32> [#uses=1]
  %21 = load i32* %tid                            ; <i32> [#uses=1]
  %22 = load i32 addrspace(1)** %1                ; <i32 addrspace(1)*> [#uses=1]
  %23 = getelementptr inbounds i32 addrspace(1)* %22, i32 %21 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %20, i32 addrspace(1)* %23
  ret void
}

declare i32 @get_local_id(i32)

declare i32 @get_global_id(i32)

declare void @barrier(i32)