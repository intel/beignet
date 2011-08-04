; ModuleID = '-'
target datalayout = "E-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-n32"
target triple = "GHAL3D_2_1"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_test1_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_test1_parameters = appending global [53 x i8] c"char __attribute__((address_space(1))) *, uint, uint\00", section "llvm.metadata" ; <[53 x i8]*> [#uses=1]
@opencl_metadata = appending global [1 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (i8 addrspace(1)*, i32, i32)* @test1 to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_test1_locals to i8*), i8* getelementptr inbounds ([53 x i8]* @opencl_test1_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[1 x %opencl_metadata_type]*> [#uses=0]

define void @test1(i8 addrspace(1)* %svm, i32 %svmBase, i32 %context) nounwind {
  %1 = alloca i8 addrspace(1)*, align 16          ; <i8 addrspace(1)**> [#uses=4]
  %2 = alloca i32, align 4                        ; <i32*> [#uses=2]
  %3 = alloca i32, align 4                        ; <i32*> [#uses=2]
  %i = alloca i32, align 4                        ; <i32*> [#uses=3]
  %ptr = alloca i32 addrspace(1)*, align 16       ; <i32 addrspace(1)**> [#uses=2]
  store i8 addrspace(1)* %svm, i8 addrspace(1)** %1
  store i32 %svmBase, i32* %2
  store i32 %context, i32* %3
  %4 = call i32 @get_global_id(i32 0)             ; <i32> [#uses=1]
  store i32 %4, i32* %i
  %5 = load i32* %2                               ; <i32> [#uses=1]
  %6 = load i8 addrspace(1)** %1                  ; <i8 addrspace(1)*> [#uses=1]
  %7 = sub i32 0, %5                              ; <i32> [#uses=1]
  %8 = getelementptr inbounds i8 addrspace(1)* %6, i32 %7 ; <i8 addrspace(1)*> [#uses=1]
  store i8 addrspace(1)* %8, i8 addrspace(1)** %1
  %9 = load i32* %3                               ; <i32> [#uses=1]
  %10 = load i8 addrspace(1)** %1                 ; <i8 addrspace(1)*> [#uses=1]
  %11 = getelementptr inbounds i8 addrspace(1)* %10, i32 %9 ; <i8 addrspace(1)*> [#uses=1]
  %12 = bitcast i8 addrspace(1)* %11 to i32 addrspace(1)* ; <i32 addrspace(1)*> [#uses=1]
  store i32 addrspace(1)* %12, i32 addrspace(1)** %ptr
  %13 = load i32* %i                              ; <i32> [#uses=1]
  %14 = load i32* %i                              ; <i32> [#uses=1]
  %15 = load i32 addrspace(1)** %ptr              ; <i32 addrspace(1)*> [#uses=1]
  %16 = getelementptr inbounds i32 addrspace(1)* %15, i32 %14 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %13, i32 addrspace(1)* %16
  ret void
}

declare i32 @get_global_id(i32)