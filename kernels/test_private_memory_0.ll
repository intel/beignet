; ModuleID = '-'
target datalayout = "E-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-n32"
target triple = "GHAL3D_2_1"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_test_private_memory_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_test_private_memory_parameters = appending global [40 x i8] c"int __attribute__((address_space(1))) *\00", section "llvm.metadata" ; <[40 x i8]*> [#uses=1]
@opencl_metadata = appending global [1 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (i32 addrspace(1)*)* @test_private_memory to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_test_private_memory_locals to i8*), i8* getelementptr inbounds ([40 x i8]* @opencl_test_private_memory_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[1 x %opencl_metadata_type]*> [#uses=0]

define void @test_private_memory(i32 addrspace(1)* %dst) nounwind {
  %1 = alloca i32 addrspace(1)*, align 16         ; <i32 addrspace(1)**> [#uses=2]
  %local0 = alloca [32 x i32], align 4            ; <[32 x i32]*> [#uses=2]
  %tid = alloca i32, align 4                      ; <i32*> [#uses=3]
  %i = alloca i32, align 4                        ; <i32*> [#uses=11]
  %res = alloca i32, align 4                      ; <i32*> [#uses=4]
  store i32 addrspace(1)* %dst, i32 addrspace(1)** %1
  %2 = call i32 @get_global_id(i32 0)             ; <i32> [#uses=1]
  store i32 %2, i32* %tid
  store i32 0, i32* %res
  store i32 0, i32* %i
  br label %3

; <label>:3                                       ; preds = %13, %0
  %4 = load i32* %i                               ; <i32> [#uses=1]
  %5 = icmp slt i32 %4, 32                        ; <i1> [#uses=1]
  br i1 %5, label %6, label %16

; <label>:6                                       ; preds = %3
  %7 = load i32* %i                               ; <i32> [#uses=1]
  %8 = load i32* %tid                             ; <i32> [#uses=1]
  %9 = add nsw i32 %7, %8                         ; <i32> [#uses=1]
  %10 = load i32* %i                              ; <i32> [#uses=1]
  %11 = getelementptr inbounds [32 x i32]* %local0, i32 0, i32 0 ; <i32*> [#uses=1]
  %12 = getelementptr inbounds i32* %11, i32 %10  ; <i32*> [#uses=1]
  store i32 %9, i32* %12
  br label %13

; <label>:13                                      ; preds = %6
  %14 = load i32* %i                              ; <i32> [#uses=1]
  %15 = add nsw i32 %14, 1                        ; <i32> [#uses=1]
  store i32 %15, i32* %i
  br label %3

; <label>:16                                      ; preds = %3
  store i32 0, i32* %i
  br label %17

; <label>:17                                      ; preds = %27, %16
  %18 = load i32* %i                              ; <i32> [#uses=1]
  %19 = icmp slt i32 %18, 32                      ; <i1> [#uses=1]
  br i1 %19, label %20, label %30

; <label>:20                                      ; preds = %17
  %21 = load i32* %i                              ; <i32> [#uses=1]
  %22 = getelementptr inbounds [32 x i32]* %local0, i32 0, i32 0 ; <i32*> [#uses=1]
  %23 = getelementptr inbounds i32* %22, i32 %21  ; <i32*> [#uses=1]
  %24 = load i32* %23                             ; <i32> [#uses=1]
  %25 = load i32* %res                            ; <i32> [#uses=1]
  %26 = add nsw i32 %25, %24                      ; <i32> [#uses=1]
  store i32 %26, i32* %res
  br label %27

; <label>:27                                      ; preds = %20
  %28 = load i32* %i                              ; <i32> [#uses=1]
  %29 = add nsw i32 %28, 1                        ; <i32> [#uses=1]
  store i32 %29, i32* %i
  br label %17

; <label>:30                                      ; preds = %17
  %31 = load i32* %res                            ; <i32> [#uses=1]
  %32 = load i32* %tid                            ; <i32> [#uses=1]
  %33 = load i32 addrspace(1)** %1                ; <i32 addrspace(1)*> [#uses=1]
  %34 = getelementptr inbounds i32 addrspace(1)* %33, i32 %32 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %31, i32 addrspace(1)* %34
  ret void
}

declare i32 @get_global_id(i32)