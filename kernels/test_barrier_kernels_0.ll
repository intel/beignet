; ModuleID = '-'
target datalayout = "E-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-n32"
target triple = "GHAL3D_2_1"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_test_barrier_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_test_barrier_parameters = appending global [153 x i8] c"int __attribute__((address_space(1))) *, float const, char const, int const, int __attribute__((address_space(1))) *, short const, uint const, int const\00", section "llvm.metadata" ; <[153 x i8]*> [#uses=1]
@opencl_metadata = appending global [1 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (i32 addrspace(1)*, float, i8, i32, i32 addrspace(1)*, i16, i32, i32)* @test_barrier to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_test_barrier_locals to i8*), i8* getelementptr inbounds ([153 x i8]* @opencl_test_barrier_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[1 x %opencl_metadata_type]*> [#uses=0]

define void @test_barrier(i32 addrspace(1)* %dst, float %a0, i8 signext %a1, i32 %a2, i32 addrspace(1)* %src, i16 signext %a3, i32 %a4, i32 %a5) nounwind {
  %1 = alloca i32 addrspace(1)*, align 16         ; <i32 addrspace(1)**> [#uses=2]
  %2 = alloca float, align 4                      ; <float*> [#uses=2]
  %3 = alloca i8, align 1                         ; <i8*> [#uses=2]
  %4 = alloca i32, align 4                        ; <i32*> [#uses=2]
  %5 = alloca i32 addrspace(1)*, align 16         ; <i32 addrspace(1)**> [#uses=2]
  %6 = alloca i16, align 2                        ; <i16*> [#uses=2]
  %7 = alloca i32, align 4                        ; <i32*> [#uses=2]
  %8 = alloca i32, align 4                        ; <i32*> [#uses=2]
  %tid = alloca i32, align 4                      ; <i32*> [#uses=3]
  store i32 addrspace(1)* %dst, i32 addrspace(1)** %1
  store float %a0, float* %2
  store i8 %a1, i8* %3
  store i32 %a2, i32* %4
  store i32 addrspace(1)* %src, i32 addrspace(1)** %5
  store i16 %a3, i16* %6
  store i32 %a4, i32* %7
  store i32 %a5, i32* %8
  %9 = call i32 @get_global_id(i32 0)             ; <i32> [#uses=1]
  store i32 %9, i32* %tid
  %10 = load float* %2                            ; <float> [#uses=1]
  %11 = fptosi float %10 to i32                   ; <i32> [#uses=1]
  %12 = load i8* %3                               ; <i8> [#uses=1]
  %13 = sext i8 %12 to i32                        ; <i32> [#uses=1]
  %14 = add nsw i32 %11, %13                      ; <i32> [#uses=1]
  %15 = load i32* %4                              ; <i32> [#uses=1]
  %16 = add nsw i32 %14, %15                      ; <i32> [#uses=1]
  %17 = load i16* %6                              ; <i16> [#uses=1]
  %18 = sext i16 %17 to i32                       ; <i32> [#uses=1]
  %19 = add nsw i32 %16, %18                      ; <i32> [#uses=1]
  %20 = load i32* %7                              ; <i32> [#uses=1]
  %21 = add nsw i32 %19, %20                      ; <i32> [#uses=1]
  %22 = load i32* %8                              ; <i32> [#uses=1]
  %23 = add nsw i32 %21, %22                      ; <i32> [#uses=1]
  %24 = load i32* %tid                            ; <i32> [#uses=1]
  %25 = load i32 addrspace(1)** %5                ; <i32 addrspace(1)*> [#uses=1]
  %26 = getelementptr inbounds i32 addrspace(1)* %25, i32 %24 ; <i32 addrspace(1)*> [#uses=1]
  %27 = load i32 addrspace(1)* %26                ; <i32> [#uses=1]
  %28 = add nsw i32 %23, %27                      ; <i32> [#uses=1]
  %29 = load i32* %tid                            ; <i32> [#uses=1]
  %30 = load i32 addrspace(1)** %1                ; <i32 addrspace(1)*> [#uses=1]
  %31 = getelementptr inbounds i32 addrspace(1)* %30, i32 %29 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %28, i32 addrspace(1)* %31
  call void @barrier(i32 1)
  ret void
}

declare i32 @get_global_id(i32)

declare void @barrier(i32)