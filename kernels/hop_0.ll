; ModuleID = '-'
target datalayout = "E-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-n32"
target triple = "GHAL3D_2_1"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_test_private_memory_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_test_private_memory_parameters = appending global [40 x i8] c"int __attribute__((address_space(1))) *\00", section "llvm.metadata" ; <[40 x i8]*> [#uses=1]
@opencl_metadata = appending global [1 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (i32 addrspace(1)*)* @test_private_memory to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_test_private_memory_locals to i8*), i8* getelementptr inbounds ([40 x i8]* @opencl_test_private_memory_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[1 x %opencl_metadata_type]*> [#uses=0]

define void @test_private_memory(i32 addrspace(1)* %dst) nounwind {
  %1 = alloca i32 addrspace(1)*, align 16         ; <i32 addrspace(1)**> [#uses=2]
  %tid = alloca i32, align 4                      ; <i32*> [#uses=3]
  store i32 addrspace(1)* %dst, i32 addrspace(1)** %1
  %2 = call i32 @get_global_id(i32 0)             ; <i32> [#uses=1]
  store i32 %2, i32* %tid
  %3 = load i32* %tid                             ; <i32> [#uses=1]
  %4 = load i32* %tid                             ; <i32> [#uses=1]
  %5 = load i32 addrspace(1)** %1                 ; <i32 addrspace(1)*> [#uses=1]
  %6 = getelementptr inbounds i32 addrspace(1)* %5, i32 %4 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %3, i32 addrspace(1)* %6
  ret void
}

declare i32 @get_global_id(i32)