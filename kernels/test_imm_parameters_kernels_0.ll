; ModuleID = '-'
target datalayout = "E-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-n32"
target triple = "GHAL3D_2_1"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_test_imm_parameters_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_test_imm_parameters_parameters = appending global [153 x i8] c"int __attribute__((address_space(1))) *, float const, char const, int const, int __attribute__((address_space(1))) *, short const, uint const, int const\00", section "llvm.metadata" ; <[153 x i8]*> [#uses=1]
@opencl_metadata = appending global [1 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (i32 addrspace(1)*, float, i8, i32, i32 addrspace(1)*, i16, i32, i32)* @test_imm_parameters to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_test_imm_parameters_locals to i8*), i8* getelementptr inbounds ([153 x i8]* @opencl_test_imm_parameters_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[1 x %opencl_metadata_type]*> [#uses=0]

define void @test_imm_parameters(i32 addrspace(1)* %dst, float %a0, i8 signext %a1, i32 %a2, i32 addrspace(1)* %src, i16 signext %a3, i32 %a4, i32 %a5) nounwind {
  %1 = alloca i32 addrspace(1)*, align 16         ; <i32 addrspace(1)**> [#uses=7]
  %2 = alloca float, align 4                      ; <float*> [#uses=2]
  %3 = alloca i8, align 1                         ; <i8*> [#uses=2]
  %4 = alloca i32, align 4                        ; <i32*> [#uses=2]
  %5 = alloca i32 addrspace(1)*, align 16         ; <i32 addrspace(1)**> [#uses=7]
  %6 = alloca i16, align 2                        ; <i16*> [#uses=2]
  %7 = alloca i32, align 4                        ; <i32*> [#uses=2]
  %8 = alloca i32, align 4                        ; <i32*> [#uses=2]
  %tid = alloca i32, align 4                      ; <i32*> [#uses=13]
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
  %12 = load i32* %tid                            ; <i32> [#uses=1]
  %13 = load i32 addrspace(1)** %5                ; <i32 addrspace(1)*> [#uses=1]
  %14 = getelementptr inbounds i32 addrspace(1)* %13, i32 %12 ; <i32 addrspace(1)*> [#uses=1]
  %15 = load i32 addrspace(1)* %14                ; <i32> [#uses=1]
  %16 = add nsw i32 %11, %15                      ; <i32> [#uses=1]
  %17 = load i32* %tid                            ; <i32> [#uses=1]
  %18 = mul i32 6, %17                            ; <i32> [#uses=1]
  %19 = add nsw i32 %18, 0                        ; <i32> [#uses=1]
  %20 = load i32 addrspace(1)** %1                ; <i32 addrspace(1)*> [#uses=1]
  %21 = getelementptr inbounds i32 addrspace(1)* %20, i32 %19 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %16, i32 addrspace(1)* %21
  %22 = load i8* %3                               ; <i8> [#uses=1]
  %23 = sext i8 %22 to i32                        ; <i32> [#uses=1]
  %24 = load i32* %tid                            ; <i32> [#uses=1]
  %25 = load i32 addrspace(1)** %5                ; <i32 addrspace(1)*> [#uses=1]
  %26 = getelementptr inbounds i32 addrspace(1)* %25, i32 %24 ; <i32 addrspace(1)*> [#uses=1]
  %27 = load i32 addrspace(1)* %26                ; <i32> [#uses=1]
  %28 = add nsw i32 %23, %27                      ; <i32> [#uses=1]
  %29 = load i32* %tid                            ; <i32> [#uses=1]
  %30 = mul i32 6, %29                            ; <i32> [#uses=1]
  %31 = add nsw i32 %30, 1                        ; <i32> [#uses=1]
  %32 = load i32 addrspace(1)** %1                ; <i32 addrspace(1)*> [#uses=1]
  %33 = getelementptr inbounds i32 addrspace(1)* %32, i32 %31 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %28, i32 addrspace(1)* %33
  %34 = load i32* %4                              ; <i32> [#uses=1]
  %35 = load i32* %tid                            ; <i32> [#uses=1]
  %36 = load i32 addrspace(1)** %5                ; <i32 addrspace(1)*> [#uses=1]
  %37 = getelementptr inbounds i32 addrspace(1)* %36, i32 %35 ; <i32 addrspace(1)*> [#uses=1]
  %38 = load i32 addrspace(1)* %37                ; <i32> [#uses=1]
  %39 = add nsw i32 %34, %38                      ; <i32> [#uses=1]
  %40 = load i32* %tid                            ; <i32> [#uses=1]
  %41 = mul i32 6, %40                            ; <i32> [#uses=1]
  %42 = add nsw i32 %41, 2                        ; <i32> [#uses=1]
  %43 = load i32 addrspace(1)** %1                ; <i32 addrspace(1)*> [#uses=1]
  %44 = getelementptr inbounds i32 addrspace(1)* %43, i32 %42 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %39, i32 addrspace(1)* %44
  %45 = load i16* %6                              ; <i16> [#uses=1]
  %46 = sext i16 %45 to i32                       ; <i32> [#uses=1]
  %47 = load i32* %tid                            ; <i32> [#uses=1]
  %48 = load i32 addrspace(1)** %5                ; <i32 addrspace(1)*> [#uses=1]
  %49 = getelementptr inbounds i32 addrspace(1)* %48, i32 %47 ; <i32 addrspace(1)*> [#uses=1]
  %50 = load i32 addrspace(1)* %49                ; <i32> [#uses=1]
  %51 = add nsw i32 %46, %50                      ; <i32> [#uses=1]
  %52 = load i32* %tid                            ; <i32> [#uses=1]
  %53 = mul i32 6, %52                            ; <i32> [#uses=1]
  %54 = add nsw i32 %53, 3                        ; <i32> [#uses=1]
  %55 = load i32 addrspace(1)** %1                ; <i32 addrspace(1)*> [#uses=1]
  %56 = getelementptr inbounds i32 addrspace(1)* %55, i32 %54 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %51, i32 addrspace(1)* %56
  %57 = load i32* %7                              ; <i32> [#uses=1]
  %58 = load i32* %tid                            ; <i32> [#uses=1]
  %59 = load i32 addrspace(1)** %5                ; <i32 addrspace(1)*> [#uses=1]
  %60 = getelementptr inbounds i32 addrspace(1)* %59, i32 %58 ; <i32 addrspace(1)*> [#uses=1]
  %61 = load i32 addrspace(1)* %60                ; <i32> [#uses=1]
  %62 = add nsw i32 %57, %61                      ; <i32> [#uses=1]
  %63 = load i32* %tid                            ; <i32> [#uses=1]
  %64 = mul i32 6, %63                            ; <i32> [#uses=1]
  %65 = add nsw i32 %64, 4                        ; <i32> [#uses=1]
  %66 = load i32 addrspace(1)** %1                ; <i32 addrspace(1)*> [#uses=1]
  %67 = getelementptr inbounds i32 addrspace(1)* %66, i32 %65 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %62, i32 addrspace(1)* %67
  %68 = load i32* %8                              ; <i32> [#uses=1]
  %69 = load i32* %tid                            ; <i32> [#uses=1]
  %70 = load i32 addrspace(1)** %5                ; <i32 addrspace(1)*> [#uses=1]
  %71 = getelementptr inbounds i32 addrspace(1)* %70, i32 %69 ; <i32 addrspace(1)*> [#uses=1]
  %72 = load i32 addrspace(1)* %71                ; <i32> [#uses=1]
  %73 = add nsw i32 %68, %72                      ; <i32> [#uses=1]
  %74 = load i32* %tid                            ; <i32> [#uses=1]
  %75 = mul i32 6, %74                            ; <i32> [#uses=1]
  %76 = add nsw i32 %75, 5                        ; <i32> [#uses=1]
  %77 = load i32 addrspace(1)** %1                ; <i32 addrspace(1)*> [#uses=1]
  %78 = getelementptr inbounds i32 addrspace(1)* %77, i32 %76 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %73, i32 addrspace(1)* %78
  ret void
}

declare i32 @get_global_id(i32)