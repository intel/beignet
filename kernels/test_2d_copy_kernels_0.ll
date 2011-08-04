; ModuleID = '-'
target datalayout = "E-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-n32"
target triple = "GHAL3D_2_1"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_test_2d_copy_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_test_2d_copy_parameters = appending global [127 x i8] c"int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *, int\00", section "llvm.metadata" ; <[127 x i8]*> [#uses=1]
@opencl_metadata = appending global [1 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (i32 addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)*, i32)* @test_2d_copy to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_test_2d_copy_locals to i8*), i8* getelementptr inbounds ([127 x i8]* @opencl_test_2d_copy_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[1 x %opencl_metadata_type]*> [#uses=0]

define void @test_2d_copy(i32 addrspace(1)* %dst0, i32 addrspace(1)* %dst1, i32 addrspace(1)* %src, i32 %w) nounwind {
  %1 = alloca i32 addrspace(1)*, align 16         ; <i32 addrspace(1)**> [#uses=2]
  %2 = alloca i32 addrspace(1)*, align 16         ; <i32 addrspace(1)**> [#uses=2]
  %3 = alloca i32 addrspace(1)*, align 16         ; <i32 addrspace(1)**> [#uses=2]
  %4 = alloca i32, align 4                        ; <i32*> [#uses=2]
  %x = alloca i32, align 4                        ; <i32*> [#uses=3]
  %y = alloca i32, align 4                        ; <i32*> [#uses=3]
  %index = alloca i32, align 4                    ; <i32*> [#uses=4]
  store i32 addrspace(1)* %dst0, i32 addrspace(1)** %1
  store i32 addrspace(1)* %dst1, i32 addrspace(1)** %2
  store i32 addrspace(1)* %src, i32 addrspace(1)** %3
  store i32 %w, i32* %4
  %5 = call i32 @get_global_id(i32 0)             ; <i32> [#uses=1]
  store i32 %5, i32* %x
  %6 = call i32 @get_global_id(i32 1)             ; <i32> [#uses=1]
  store i32 %6, i32* %y
  %7 = load i32* %x                               ; <i32> [#uses=1]
  %8 = load i32* %y                               ; <i32> [#uses=1]
  %9 = load i32* %4                               ; <i32> [#uses=1]
  %10 = mul i32 %8, %9                            ; <i32> [#uses=1]
  %11 = add nsw i32 %7, %10                       ; <i32> [#uses=1]
  store i32 %11, i32* %index
  %12 = load i32* %index                          ; <i32> [#uses=1]
  %13 = load i32 addrspace(1)** %3                ; <i32 addrspace(1)*> [#uses=1]
  %14 = getelementptr inbounds i32 addrspace(1)* %13, i32 %12 ; <i32 addrspace(1)*> [#uses=1]
  %15 = load i32 addrspace(1)* %14                ; <i32> [#uses=1]
  %16 = load i32* %index                          ; <i32> [#uses=1]
  %17 = load i32 addrspace(1)** %1                ; <i32 addrspace(1)*> [#uses=1]
  %18 = getelementptr inbounds i32 addrspace(1)* %17, i32 %16 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %15, i32 addrspace(1)* %18
  %19 = load i32* %x                              ; <i32> [#uses=1]
  %20 = load i32* %y                              ; <i32> [#uses=1]
  %21 = add nsw i32 %19, %20                      ; <i32> [#uses=1]
  %22 = load i32* %index                          ; <i32> [#uses=1]
  %23 = load i32 addrspace(1)** %2                ; <i32 addrspace(1)*> [#uses=1]
  %24 = getelementptr inbounds i32 addrspace(1)* %23, i32 %22 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %21, i32 addrspace(1)* %24
  ret void
}

declare i32 @get_global_id(i32)