; ModuleID = '-'
target datalayout = "E-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-n32"
target triple = "GHAL3D_2_1"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_gaussianRand_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_gaussianRand_parameters = appending global [104 x i8] c"uint4 const __attribute__((address_space(1))) *, uint, uint, float4 __attribute__((address_space(1))) *\00", section "llvm.metadata" ; <[104 x i8]*> [#uses=1]
@opencl_metadata = appending global [1 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (<4 x i32> addrspace(1)*, i32, i32, <4 x float> addrspace(1)*)* @gaussianRand to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_gaussianRand_locals to i8*), i8* getelementptr inbounds ([104 x i8]* @opencl_gaussianRand_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[1 x %opencl_metadata_type]*> [#uses=0]

define void @lshift128(<4 x i32> %input, i32 %shift, <4 x i32>* %output) nounwind {
  %1 = alloca <4 x i32>, align 16                 ; <<4 x i32>*> [#uses=8]
  %2 = alloca i32, align 4                        ; <i32*> [#uses=6]
  %3 = alloca <4 x i32>*, align 4                 ; <<4 x i32>**> [#uses=2]
  %invshift = alloca i32, align 4                 ; <i32*> [#uses=4]
  %temp = alloca <4 x i32>, align 16              ; <<4 x i32>*> [#uses=9]
  store <4 x i32> %input, <4 x i32>* %1
  store i32 %shift, i32* %2
  store <4 x i32>* %output, <4 x i32>** %3
  %4 = load i32* %2                               ; <i32> [#uses=1]
  %5 = sub i32 32, %4                             ; <i32> [#uses=1]
  store i32 %5, i32* %invshift
  %6 = load <4 x i32>* %1                         ; <<4 x i32>> [#uses=1]
  %7 = extractelement <4 x i32> %6, i32 0         ; <i32> [#uses=1]
  %8 = load i32* %2                               ; <i32> [#uses=1]
  %9 = shl i32 %7, %8                             ; <i32> [#uses=1]
  %10 = load <4 x i32>* %temp                     ; <<4 x i32>> [#uses=1]
  %11 = insertelement <4 x i32> %10, i32 %9, i32 0 ; <<4 x i32>> [#uses=1]
  store <4 x i32> %11, <4 x i32>* %temp
  %12 = load <4 x i32>* %1                        ; <<4 x i32>> [#uses=1]
  %13 = extractelement <4 x i32> %12, i32 1       ; <i32> [#uses=1]
  %14 = load i32* %2                              ; <i32> [#uses=1]
  %15 = shl i32 %13, %14                          ; <i32> [#uses=1]
  %16 = load <4 x i32>* %1                        ; <<4 x i32>> [#uses=1]
  %17 = extractelement <4 x i32> %16, i32 0       ; <i32> [#uses=1]
  %18 = load i32* %invshift                       ; <i32> [#uses=1]
  %19 = lshr i32 %17, %18                         ; <i32> [#uses=1]
  %20 = or i32 %15, %19                           ; <i32> [#uses=1]
  %21 = load <4 x i32>* %temp                     ; <<4 x i32>> [#uses=1]
  %22 = insertelement <4 x i32> %21, i32 %20, i32 1 ; <<4 x i32>> [#uses=1]
  store <4 x i32> %22, <4 x i32>* %temp
  %23 = load <4 x i32>* %1                        ; <<4 x i32>> [#uses=1]
  %24 = extractelement <4 x i32> %23, i32 2       ; <i32> [#uses=1]
  %25 = load i32* %2                              ; <i32> [#uses=1]
  %26 = shl i32 %24, %25                          ; <i32> [#uses=1]
  %27 = load <4 x i32>* %1                        ; <<4 x i32>> [#uses=1]
  %28 = extractelement <4 x i32> %27, i32 1       ; <i32> [#uses=1]
  %29 = load i32* %invshift                       ; <i32> [#uses=1]
  %30 = lshr i32 %28, %29                         ; <i32> [#uses=1]
  %31 = or i32 %26, %30                           ; <i32> [#uses=1]
  %32 = load <4 x i32>* %temp                     ; <<4 x i32>> [#uses=1]
  %33 = insertelement <4 x i32> %32, i32 %31, i32 2 ; <<4 x i32>> [#uses=1]
  store <4 x i32> %33, <4 x i32>* %temp
  %34 = load <4 x i32>* %1                        ; <<4 x i32>> [#uses=1]
  %35 = extractelement <4 x i32> %34, i32 3       ; <i32> [#uses=1]
  %36 = load i32* %2                              ; <i32> [#uses=1]
  %37 = shl i32 %35, %36                          ; <i32> [#uses=1]
  %38 = load <4 x i32>* %1                        ; <<4 x i32>> [#uses=1]
  %39 = extractelement <4 x i32> %38, i32 2       ; <i32> [#uses=1]
  %40 = load i32* %invshift                       ; <i32> [#uses=1]
  %41 = lshr i32 %39, %40                         ; <i32> [#uses=1]
  %42 = or i32 %37, %41                           ; <i32> [#uses=1]
  %43 = load <4 x i32>* %temp                     ; <<4 x i32>> [#uses=1]
  %44 = insertelement <4 x i32> %43, i32 %42, i32 3 ; <<4 x i32>> [#uses=1]
  store <4 x i32> %44, <4 x i32>* %temp
  %45 = load <4 x i32>* %temp                     ; <<4 x i32>> [#uses=1]
  %46 = load <4 x i32>** %3                       ; <<4 x i32>*> [#uses=1]
  store <4 x i32> %45, <4 x i32>* %46
  ret void
}

define void @rshift128(<4 x i32> %input, i32 %shift, <4 x i32>* %output) nounwind {
  %1 = alloca <4 x i32>, align 16                 ; <<4 x i32>*> [#uses=8]
  %2 = alloca i32, align 4                        ; <i32*> [#uses=6]
  %3 = alloca <4 x i32>*, align 4                 ; <<4 x i32>**> [#uses=2]
  %invshift = alloca i32, align 4                 ; <i32*> [#uses=4]
  %temp = alloca <4 x i32>, align 16              ; <<4 x i32>*> [#uses=9]
  store <4 x i32> %input, <4 x i32>* %1
  store i32 %shift, i32* %2
  store <4 x i32>* %output, <4 x i32>** %3
  %4 = load i32* %2                               ; <i32> [#uses=1]
  %5 = sub i32 32, %4                             ; <i32> [#uses=1]
  store i32 %5, i32* %invshift
  %6 = load <4 x i32>* %1                         ; <<4 x i32>> [#uses=1]
  %7 = extractelement <4 x i32> %6, i32 3         ; <i32> [#uses=1]
  %8 = load i32* %2                               ; <i32> [#uses=1]
  %9 = lshr i32 %7, %8                            ; <i32> [#uses=1]
  %10 = load <4 x i32>* %temp                     ; <<4 x i32>> [#uses=1]
  %11 = insertelement <4 x i32> %10, i32 %9, i32 3 ; <<4 x i32>> [#uses=1]
  store <4 x i32> %11, <4 x i32>* %temp
  %12 = load <4 x i32>* %1                        ; <<4 x i32>> [#uses=1]
  %13 = extractelement <4 x i32> %12, i32 2       ; <i32> [#uses=1]
  %14 = load i32* %2                              ; <i32> [#uses=1]
  %15 = lshr i32 %13, %14                         ; <i32> [#uses=1]
  %16 = load <4 x i32>* %1                        ; <<4 x i32>> [#uses=1]
  %17 = extractelement <4 x i32> %16, i32 3       ; <i32> [#uses=1]
  %18 = load i32* %invshift                       ; <i32> [#uses=1]
  %19 = shl i32 %17, %18                          ; <i32> [#uses=1]
  %20 = or i32 %15, %19                           ; <i32> [#uses=1]
  %21 = load <4 x i32>* %temp                     ; <<4 x i32>> [#uses=1]
  %22 = insertelement <4 x i32> %21, i32 %20, i32 2 ; <<4 x i32>> [#uses=1]
  store <4 x i32> %22, <4 x i32>* %temp
  %23 = load <4 x i32>* %1                        ; <<4 x i32>> [#uses=1]
  %24 = extractelement <4 x i32> %23, i32 1       ; <i32> [#uses=1]
  %25 = load i32* %2                              ; <i32> [#uses=1]
  %26 = lshr i32 %24, %25                         ; <i32> [#uses=1]
  %27 = load <4 x i32>* %1                        ; <<4 x i32>> [#uses=1]
  %28 = extractelement <4 x i32> %27, i32 2       ; <i32> [#uses=1]
  %29 = load i32* %invshift                       ; <i32> [#uses=1]
  %30 = shl i32 %28, %29                          ; <i32> [#uses=1]
  %31 = or i32 %26, %30                           ; <i32> [#uses=1]
  %32 = load <4 x i32>* %temp                     ; <<4 x i32>> [#uses=1]
  %33 = insertelement <4 x i32> %32, i32 %31, i32 1 ; <<4 x i32>> [#uses=1]
  store <4 x i32> %33, <4 x i32>* %temp
  %34 = load <4 x i32>* %1                        ; <<4 x i32>> [#uses=1]
  %35 = extractelement <4 x i32> %34, i32 0       ; <i32> [#uses=1]
  %36 = load i32* %2                              ; <i32> [#uses=1]
  %37 = lshr i32 %35, %36                         ; <i32> [#uses=1]
  %38 = load <4 x i32>* %1                        ; <<4 x i32>> [#uses=1]
  %39 = extractelement <4 x i32> %38, i32 1       ; <i32> [#uses=1]
  %40 = load i32* %invshift                       ; <i32> [#uses=1]
  %41 = shl i32 %39, %40                          ; <i32> [#uses=1]
  %42 = or i32 %37, %41                           ; <i32> [#uses=1]
  %43 = load <4 x i32>* %temp                     ; <<4 x i32>> [#uses=1]
  %44 = insertelement <4 x i32> %43, i32 %42, i32 0 ; <<4 x i32>> [#uses=1]
  store <4 x i32> %44, <4 x i32>* %temp
  %45 = load <4 x i32>* %temp                     ; <<4 x i32>> [#uses=1]
  %46 = load <4 x i32>** %3                       ; <<4 x i32>*> [#uses=1]
  store <4 x i32> %45, <4 x i32>* %46
  ret void
}

define void @gaussianRand(<4 x i32> addrspace(1)* %seedArray, i32 %width, i32 %mulFactor, <4 x float> addrspace(1)* %gaussianRand) nounwind {
  %1 = alloca <4 x i32> addrspace(1)*, align 16   ; <<4 x i32> addrspace(1)**> [#uses=2]
  %2 = alloca i32, align 4                        ; <i32*> [#uses=3]
  %3 = alloca i32, align 4                        ; <i32*> [#uses=4]
  %4 = alloca <4 x float> addrspace(1)*, align 16 ; <<4 x float> addrspace(1)**> [#uses=3]
  %temp = alloca [8 x <4 x i32>], align 16        ; <[8 x <4 x i32>]*> [#uses=19]
  %xPid = alloca i32, align 4                     ; <i32*> [#uses=3]
  %yPid = alloca i32, align 4                     ; <i32*> [#uses=3]
  %state1 = alloca <4 x i32>, align 16            ; <<4 x i32>*> [#uses=5]
  %state2 = alloca <4 x i32>, align 16            ; <<4 x i32>*> [#uses=6]
  %5 = alloca <4 x i32>, align 16                 ; <<4 x i32>*> [#uses=2]
  %state3 = alloca <4 x i32>, align 16            ; <<4 x i32>*> [#uses=6]
  %6 = alloca <4 x i32>, align 16                 ; <<4 x i32>*> [#uses=2]
  %state4 = alloca <4 x i32>, align 16            ; <<4 x i32>*> [#uses=7]
  %7 = alloca <4 x i32>, align 16                 ; <<4 x i32>*> [#uses=2]
  %state5 = alloca <4 x i32>, align 16            ; <<4 x i32>*> [#uses=5]
  %8 = alloca <4 x i32>, align 16                 ; <<4 x i32>*> [#uses=2]
  %stateMask = alloca i32, align 4                ; <i32*> [#uses=5]
  %thirty = alloca i32, align 4                   ; <i32*> [#uses=5]
  %mask4 = alloca <4 x i32>, align 16             ; <<4 x i32>*> [#uses=5]
  %9 = alloca <4 x i32>, align 16                 ; <<4 x i32>*> [#uses=2]
  %thirty4 = alloca <4 x i32>, align 16           ; <<4 x i32>*> [#uses=5]
  %10 = alloca <4 x i32>, align 16                ; <<4 x i32>*> [#uses=2]
  %one4 = alloca <4 x i32>, align 16              ; <<4 x i32>*> [#uses=2]
  %11 = alloca <4 x i32>, align 16                ; <<4 x i32>*> [#uses=2]
  %two4 = alloca <4 x i32>, align 16              ; <<4 x i32>*> [#uses=2]
  %12 = alloca <4 x i32>, align 16                ; <<4 x i32>*> [#uses=2]
  %three4 = alloca <4 x i32>, align 16            ; <<4 x i32>*> [#uses=2]
  %13 = alloca <4 x i32>, align 16                ; <<4 x i32>*> [#uses=2]
  %four4 = alloca <4 x i32>, align 16             ; <<4 x i32>*> [#uses=2]
  %14 = alloca <4 x i32>, align 16                ; <<4 x i32>*> [#uses=2]
  %r1 = alloca <4 x i32>, align 16                ; <<4 x i32>*> [#uses=10]
  %15 = alloca <4 x i32>, align 16                ; <<4 x i32>*> [#uses=2]
  %r2 = alloca <4 x i32>, align 16                ; <<4 x i32>*> [#uses=20]
  %16 = alloca <4 x i32>, align 16                ; <<4 x i32>*> [#uses=2]
  %a = alloca <4 x i32>, align 16                 ; <<4 x i32>*> [#uses=14]
  %17 = alloca <4 x i32>, align 16                ; <<4 x i32>*> [#uses=2]
  %b = alloca <4 x i32>, align 16                 ; <<4 x i32>*> [#uses=13]
  %18 = alloca <4 x i32>, align 16                ; <<4 x i32>*> [#uses=2]
  %e = alloca <4 x i32>, align 16                 ; <<4 x i32>*> [#uses=6]
  %19 = alloca <4 x i32>, align 16                ; <<4 x i32>*> [#uses=2]
  %f = alloca <4 x i32>, align 16                 ; <<4 x i32>*> [#uses=6]
  %20 = alloca <4 x i32>, align 16                ; <<4 x i32>*> [#uses=2]
  %thirteen = alloca i32, align 4                 ; <i32*> [#uses=5]
  %fifteen = alloca i32, align 4                  ; <i32*> [#uses=5]
  %shift = alloca i32, align 4                    ; <i32*> [#uses=3]
  %mask11 = alloca i32, align 4                   ; <i32*> [#uses=2]
  %mask12 = alloca i32, align 4                   ; <i32*> [#uses=2]
  %mask13 = alloca i32, align 4                   ; <i32*> [#uses=2]
  %mask14 = alloca i32, align 4                   ; <i32*> [#uses=2]
  %actualPos = alloca i32, align 4                ; <i32*> [#uses=4]
  %one = alloca float, align 4                    ; <float*> [#uses=3]
  %intMax = alloca float, align 4                 ; <float*> [#uses=3]
  %PI = alloca float, align 4                     ; <float*> [#uses=2]
  %two = alloca float, align 4                    ; <float*> [#uses=3]
  %r = alloca <4 x float>, align 16               ; <<4 x float>*> [#uses=3]
  %phi = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %temp1 = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=2]
  %temp2 = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=2]
  %i = alloca i32, align 4                        ; <i32*> [#uses=18]
  store <4 x i32> addrspace(1)* %seedArray, <4 x i32> addrspace(1)** %1
  store i32 %width, i32* %2
  store i32 %mulFactor, i32* %3
  store <4 x float> addrspace(1)* %gaussianRand, <4 x float> addrspace(1)** %4
  %21 = call i32 @get_global_id(i32 0)            ; <i32> [#uses=1]
  store i32 %21, i32* %xPid
  %22 = call i32 @get_global_id(i32 1)            ; <i32> [#uses=1]
  store i32 %22, i32* %yPid
  %23 = load i32* %yPid                           ; <i32> [#uses=1]
  %24 = load i32* %2                              ; <i32> [#uses=1]
  %25 = mul i32 %23, %24                          ; <i32> [#uses=1]
  %26 = load i32* %xPid                           ; <i32> [#uses=1]
  %27 = add i32 %25, %26                          ; <i32> [#uses=1]
  %28 = load <4 x i32> addrspace(1)** %1          ; <<4 x i32> addrspace(1)*> [#uses=1]
  %29 = getelementptr inbounds <4 x i32> addrspace(1)* %28, i32 %27 ; <<4 x i32> addrspace(1)*> [#uses=1]
  %30 = load <4 x i32> addrspace(1)* %29          ; <<4 x i32>> [#uses=1]
  store <4 x i32> %30, <4 x i32>* %state1
  store <4 x i32> zeroinitializer, <4 x i32>* %5
  %31 = load <4 x i32>* %5                        ; <<4 x i32>> [#uses=1]
  store <4 x i32> %31, <4 x i32>* %state2
  store <4 x i32> zeroinitializer, <4 x i32>* %6
  %32 = load <4 x i32>* %6                        ; <<4 x i32>> [#uses=1]
  store <4 x i32> %32, <4 x i32>* %state3
  store <4 x i32> zeroinitializer, <4 x i32>* %7
  %33 = load <4 x i32>* %7                        ; <<4 x i32>> [#uses=1]
  store <4 x i32> %33, <4 x i32>* %state4
  store <4 x i32> zeroinitializer, <4 x i32>* %8
  %34 = load <4 x i32>* %8                        ; <<4 x i32>> [#uses=1]
  store <4 x i32> %34, <4 x i32>* %state5
  store i32 1812433253, i32* %stateMask
  store i32 30, i32* %thirty
  %35 = load i32* %stateMask                      ; <i32> [#uses=1]
  %36 = insertelement <4 x i32> undef, i32 %35, i32 0 ; <<4 x i32>> [#uses=1]
  %37 = load i32* %stateMask                      ; <i32> [#uses=1]
  %38 = insertelement <4 x i32> %36, i32 %37, i32 1 ; <<4 x i32>> [#uses=1]
  %39 = load i32* %stateMask                      ; <i32> [#uses=1]
  %40 = insertelement <4 x i32> %38, i32 %39, i32 2 ; <<4 x i32>> [#uses=1]
  %41 = load i32* %stateMask                      ; <i32> [#uses=1]
  %42 = insertelement <4 x i32> %40, i32 %41, i32 3 ; <<4 x i32>> [#uses=1]
  store <4 x i32> %42, <4 x i32>* %9
  %43 = load <4 x i32>* %9                        ; <<4 x i32>> [#uses=1]
  store <4 x i32> %43, <4 x i32>* %mask4
  %44 = load i32* %thirty                         ; <i32> [#uses=1]
  %45 = insertelement <4 x i32> undef, i32 %44, i32 0 ; <<4 x i32>> [#uses=1]
  %46 = load i32* %thirty                         ; <i32> [#uses=1]
  %47 = insertelement <4 x i32> %45, i32 %46, i32 1 ; <<4 x i32>> [#uses=1]
  %48 = load i32* %thirty                         ; <i32> [#uses=1]
  %49 = insertelement <4 x i32> %47, i32 %48, i32 2 ; <<4 x i32>> [#uses=1]
  %50 = load i32* %thirty                         ; <i32> [#uses=1]
  %51 = insertelement <4 x i32> %49, i32 %50, i32 3 ; <<4 x i32>> [#uses=1]
  store <4 x i32> %51, <4 x i32>* %10
  %52 = load <4 x i32>* %10                       ; <<4 x i32>> [#uses=1]
  store <4 x i32> %52, <4 x i32>* %thirty4
  store <4 x i32> <i32 1, i32 1, i32 1, i32 1>, <4 x i32>* %11
  %53 = load <4 x i32>* %11                       ; <<4 x i32>> [#uses=1]
  store <4 x i32> %53, <4 x i32>* %one4
  store <4 x i32> <i32 2, i32 2, i32 2, i32 2>, <4 x i32>* %12
  %54 = load <4 x i32>* %12                       ; <<4 x i32>> [#uses=1]
  store <4 x i32> %54, <4 x i32>* %two4
  store <4 x i32> <i32 3, i32 3, i32 3, i32 3>, <4 x i32>* %13
  %55 = load <4 x i32>* %13                       ; <<4 x i32>> [#uses=1]
  store <4 x i32> %55, <4 x i32>* %three4
  store <4 x i32> <i32 4, i32 4, i32 4, i32 4>, <4 x i32>* %14
  %56 = load <4 x i32>* %14                       ; <<4 x i32>> [#uses=1]
  store <4 x i32> %56, <4 x i32>* %four4
  store <4 x i32> zeroinitializer, <4 x i32>* %15
  %57 = load <4 x i32>* %15                       ; <<4 x i32>> [#uses=1]
  store <4 x i32> %57, <4 x i32>* %r1
  store <4 x i32> zeroinitializer, <4 x i32>* %16
  %58 = load <4 x i32>* %16                       ; <<4 x i32>> [#uses=1]
  store <4 x i32> %58, <4 x i32>* %r2
  store <4 x i32> zeroinitializer, <4 x i32>* %17
  %59 = load <4 x i32>* %17                       ; <<4 x i32>> [#uses=1]
  store <4 x i32> %59, <4 x i32>* %a
  store <4 x i32> zeroinitializer, <4 x i32>* %18
  %60 = load <4 x i32>* %18                       ; <<4 x i32>> [#uses=1]
  store <4 x i32> %60, <4 x i32>* %b
  store <4 x i32> zeroinitializer, <4 x i32>* %19
  %61 = load <4 x i32>* %19                       ; <<4 x i32>> [#uses=1]
  store <4 x i32> %61, <4 x i32>* %e
  store <4 x i32> zeroinitializer, <4 x i32>* %20
  %62 = load <4 x i32>* %20                       ; <<4 x i32>> [#uses=1]
  store <4 x i32> %62, <4 x i32>* %f
  store i32 13, i32* %thirteen
  store i32 15, i32* %fifteen
  store i32 24, i32* %shift
  store i32 -33605633, i32* %mask11
  store i32 -276873347, i32* %mask12
  store i32 -8946819, i32* %mask13
  store i32 2146958127, i32* %mask14
  store i32 0, i32* %actualPos
  store float 1.000000e+000, float* %one
  store float 0x41F0000000000000, float* %intMax
  store float 0x400921FB60000000, float* %PI
  store float 2.000000e+000, float* %two
  %63 = load <4 x i32>* %mask4                    ; <<4 x i32>> [#uses=1]
  %64 = load <4 x i32>* %state1                   ; <<4 x i32>> [#uses=1]
  %65 = load <4 x i32>* %state1                   ; <<4 x i32>> [#uses=1]
  %66 = load <4 x i32>* %thirty4                  ; <<4 x i32>> [#uses=1]
  %67 = and <4 x i32> %66, <i32 31, i32 31, i32 31, i32 31> ; <<4 x i32>> [#uses=1]
  %68 = lshr <4 x i32> %65, %67                   ; <<4 x i32>> [#uses=1]
  %69 = xor <4 x i32> %64, %68                    ; <<4 x i32>> [#uses=1]
  %70 = mul <4 x i32> %63, %69                    ; <<4 x i32>> [#uses=1]
  %71 = load <4 x i32>* %one4                     ; <<4 x i32>> [#uses=1]
  %72 = add <4 x i32> %70, %71                    ; <<4 x i32>> [#uses=1]
  store <4 x i32> %72, <4 x i32>* %state2
  %73 = load <4 x i32>* %mask4                    ; <<4 x i32>> [#uses=1]
  %74 = load <4 x i32>* %state2                   ; <<4 x i32>> [#uses=1]
  %75 = load <4 x i32>* %state2                   ; <<4 x i32>> [#uses=1]
  %76 = load <4 x i32>* %thirty4                  ; <<4 x i32>> [#uses=1]
  %77 = and <4 x i32> %76, <i32 31, i32 31, i32 31, i32 31> ; <<4 x i32>> [#uses=1]
  %78 = lshr <4 x i32> %75, %77                   ; <<4 x i32>> [#uses=1]
  %79 = xor <4 x i32> %74, %78                    ; <<4 x i32>> [#uses=1]
  %80 = mul <4 x i32> %73, %79                    ; <<4 x i32>> [#uses=1]
  %81 = load <4 x i32>* %two4                     ; <<4 x i32>> [#uses=1]
  %82 = add <4 x i32> %80, %81                    ; <<4 x i32>> [#uses=1]
  store <4 x i32> %82, <4 x i32>* %state3
  %83 = load <4 x i32>* %mask4                    ; <<4 x i32>> [#uses=1]
  %84 = load <4 x i32>* %state3                   ; <<4 x i32>> [#uses=1]
  %85 = load <4 x i32>* %state3                   ; <<4 x i32>> [#uses=1]
  %86 = load <4 x i32>* %thirty4                  ; <<4 x i32>> [#uses=1]
  %87 = and <4 x i32> %86, <i32 31, i32 31, i32 31, i32 31> ; <<4 x i32>> [#uses=1]
  %88 = lshr <4 x i32> %85, %87                   ; <<4 x i32>> [#uses=1]
  %89 = xor <4 x i32> %84, %88                    ; <<4 x i32>> [#uses=1]
  %90 = mul <4 x i32> %83, %89                    ; <<4 x i32>> [#uses=1]
  %91 = load <4 x i32>* %three4                   ; <<4 x i32>> [#uses=1]
  %92 = add <4 x i32> %90, %91                    ; <<4 x i32>> [#uses=1]
  store <4 x i32> %92, <4 x i32>* %state4
  %93 = load <4 x i32>* %mask4                    ; <<4 x i32>> [#uses=1]
  %94 = load <4 x i32>* %state4                   ; <<4 x i32>> [#uses=1]
  %95 = load <4 x i32>* %state4                   ; <<4 x i32>> [#uses=1]
  %96 = load <4 x i32>* %thirty4                  ; <<4 x i32>> [#uses=1]
  %97 = and <4 x i32> %96, <i32 31, i32 31, i32 31, i32 31> ; <<4 x i32>> [#uses=1]
  %98 = lshr <4 x i32> %95, %97                   ; <<4 x i32>> [#uses=1]
  %99 = xor <4 x i32> %94, %98                    ; <<4 x i32>> [#uses=1]
  %100 = mul <4 x i32> %93, %99                   ; <<4 x i32>> [#uses=1]
  %101 = load <4 x i32>* %four4                   ; <<4 x i32>> [#uses=1]
  %102 = add <4 x i32> %100, %101                 ; <<4 x i32>> [#uses=1]
  store <4 x i32> %102, <4 x i32>* %state5
  store i32 0, i32* %i
  store i32 0, i32* %i
  br label %103

; <label>:103                                     ; preds = %281, %0
  %104 = load i32* %i                             ; <i32> [#uses=1]
  %105 = load i32* %3                             ; <i32> [#uses=1]
  %106 = icmp ult i32 %104, %105                  ; <i1> [#uses=1]
  br i1 %106, label %107, label %284

; <label>:107                                     ; preds = %103
  %108 = load i32* %i                             ; <i32> [#uses=1]
  switch i32 %108, label %175 [
    i32 0, label %109
    i32 1, label %114
    i32 2, label %121
    i32 3, label %128
    i32 4, label %135
    i32 5, label %142
    i32 6, label %153
    i32 7, label %164
  ]

; <label>:109                                     ; preds = %107
  %110 = load <4 x i32>* %state4                  ; <<4 x i32>> [#uses=1]
  store <4 x i32> %110, <4 x i32>* %r1
  %111 = load <4 x i32>* %state5                  ; <<4 x i32>> [#uses=1]
  store <4 x i32> %111, <4 x i32>* %r2
  %112 = load <4 x i32>* %state1                  ; <<4 x i32>> [#uses=1]
  store <4 x i32> %112, <4 x i32>* %a
  %113 = load <4 x i32>* %state3                  ; <<4 x i32>> [#uses=1]
  store <4 x i32> %113, <4 x i32>* %b
  br label %176

; <label>:114                                     ; preds = %107
  %115 = load <4 x i32>* %r2                      ; <<4 x i32>> [#uses=1]
  store <4 x i32> %115, <4 x i32>* %r1
  %116 = getelementptr inbounds [8 x <4 x i32>]* %temp, i32 0, i32 0 ; <<4 x i32>*> [#uses=1]
  %117 = getelementptr inbounds <4 x i32>* %116, i32 0 ; <<4 x i32>*> [#uses=1]
  %118 = load <4 x i32>* %117                     ; <<4 x i32>> [#uses=1]
  store <4 x i32> %118, <4 x i32>* %r2
  %119 = load <4 x i32>* %state2                  ; <<4 x i32>> [#uses=1]
  store <4 x i32> %119, <4 x i32>* %a
  %120 = load <4 x i32>* %state4                  ; <<4 x i32>> [#uses=1]
  store <4 x i32> %120, <4 x i32>* %b
  br label %176

; <label>:121                                     ; preds = %107
  %122 = load <4 x i32>* %r2                      ; <<4 x i32>> [#uses=1]
  store <4 x i32> %122, <4 x i32>* %r1
  %123 = getelementptr inbounds [8 x <4 x i32>]* %temp, i32 0, i32 0 ; <<4 x i32>*> [#uses=1]
  %124 = getelementptr inbounds <4 x i32>* %123, i32 1 ; <<4 x i32>*> [#uses=1]
  %125 = load <4 x i32>* %124                     ; <<4 x i32>> [#uses=1]
  store <4 x i32> %125, <4 x i32>* %r2
  %126 = load <4 x i32>* %state3                  ; <<4 x i32>> [#uses=1]
  store <4 x i32> %126, <4 x i32>* %a
  %127 = load <4 x i32>* %state5                  ; <<4 x i32>> [#uses=1]
  store <4 x i32> %127, <4 x i32>* %b
  br label %176

; <label>:128                                     ; preds = %107
  %129 = load <4 x i32>* %r2                      ; <<4 x i32>> [#uses=1]
  store <4 x i32> %129, <4 x i32>* %r1
  %130 = getelementptr inbounds [8 x <4 x i32>]* %temp, i32 0, i32 0 ; <<4 x i32>*> [#uses=1]
  %131 = getelementptr inbounds <4 x i32>* %130, i32 2 ; <<4 x i32>*> [#uses=1]
  %132 = load <4 x i32>* %131                     ; <<4 x i32>> [#uses=1]
  store <4 x i32> %132, <4 x i32>* %r2
  %133 = load <4 x i32>* %state4                  ; <<4 x i32>> [#uses=1]
  store <4 x i32> %133, <4 x i32>* %a
  %134 = load <4 x i32>* %state1                  ; <<4 x i32>> [#uses=1]
  store <4 x i32> %134, <4 x i32>* %b
  br label %176

; <label>:135                                     ; preds = %107
  %136 = load <4 x i32>* %r2                      ; <<4 x i32>> [#uses=1]
  store <4 x i32> %136, <4 x i32>* %r1
  %137 = getelementptr inbounds [8 x <4 x i32>]* %temp, i32 0, i32 0 ; <<4 x i32>*> [#uses=1]
  %138 = getelementptr inbounds <4 x i32>* %137, i32 3 ; <<4 x i32>*> [#uses=1]
  %139 = load <4 x i32>* %138                     ; <<4 x i32>> [#uses=1]
  store <4 x i32> %139, <4 x i32>* %r2
  %140 = load <4 x i32>* %state5                  ; <<4 x i32>> [#uses=1]
  store <4 x i32> %140, <4 x i32>* %a
  %141 = load <4 x i32>* %state2                  ; <<4 x i32>> [#uses=1]
  store <4 x i32> %141, <4 x i32>* %b
  br label %176

; <label>:142                                     ; preds = %107
  %143 = load <4 x i32>* %r2                      ; <<4 x i32>> [#uses=1]
  store <4 x i32> %143, <4 x i32>* %r1
  %144 = getelementptr inbounds [8 x <4 x i32>]* %temp, i32 0, i32 0 ; <<4 x i32>*> [#uses=1]
  %145 = getelementptr inbounds <4 x i32>* %144, i32 4 ; <<4 x i32>*> [#uses=1]
  %146 = load <4 x i32>* %145                     ; <<4 x i32>> [#uses=1]
  store <4 x i32> %146, <4 x i32>* %r2
  %147 = getelementptr inbounds [8 x <4 x i32>]* %temp, i32 0, i32 0 ; <<4 x i32>*> [#uses=1]
  %148 = getelementptr inbounds <4 x i32>* %147, i32 0 ; <<4 x i32>*> [#uses=1]
  %149 = load <4 x i32>* %148                     ; <<4 x i32>> [#uses=1]
  store <4 x i32> %149, <4 x i32>* %a
  %150 = getelementptr inbounds [8 x <4 x i32>]* %temp, i32 0, i32 0 ; <<4 x i32>*> [#uses=1]
  %151 = getelementptr inbounds <4 x i32>* %150, i32 2 ; <<4 x i32>*> [#uses=1]
  %152 = load <4 x i32>* %151                     ; <<4 x i32>> [#uses=1]
  store <4 x i32> %152, <4 x i32>* %b
  br label %176

; <label>:153                                     ; preds = %107
  %154 = load <4 x i32>* %r2                      ; <<4 x i32>> [#uses=1]
  store <4 x i32> %154, <4 x i32>* %r1
  %155 = getelementptr inbounds [8 x <4 x i32>]* %temp, i32 0, i32 0 ; <<4 x i32>*> [#uses=1]
  %156 = getelementptr inbounds <4 x i32>* %155, i32 5 ; <<4 x i32>*> [#uses=1]
  %157 = load <4 x i32>* %156                     ; <<4 x i32>> [#uses=1]
  store <4 x i32> %157, <4 x i32>* %r2
  %158 = getelementptr inbounds [8 x <4 x i32>]* %temp, i32 0, i32 0 ; <<4 x i32>*> [#uses=1]
  %159 = getelementptr inbounds <4 x i32>* %158, i32 1 ; <<4 x i32>*> [#uses=1]
  %160 = load <4 x i32>* %159                     ; <<4 x i32>> [#uses=1]
  store <4 x i32> %160, <4 x i32>* %a
  %161 = getelementptr inbounds [8 x <4 x i32>]* %temp, i32 0, i32 0 ; <<4 x i32>*> [#uses=1]
  %162 = getelementptr inbounds <4 x i32>* %161, i32 3 ; <<4 x i32>*> [#uses=1]
  %163 = load <4 x i32>* %162                     ; <<4 x i32>> [#uses=1]
  store <4 x i32> %163, <4 x i32>* %b
  br label %176

; <label>:164                                     ; preds = %107
  %165 = load <4 x i32>* %r2                      ; <<4 x i32>> [#uses=1]
  store <4 x i32> %165, <4 x i32>* %r1
  %166 = getelementptr inbounds [8 x <4 x i32>]* %temp, i32 0, i32 0 ; <<4 x i32>*> [#uses=1]
  %167 = getelementptr inbounds <4 x i32>* %166, i32 6 ; <<4 x i32>*> [#uses=1]
  %168 = load <4 x i32>* %167                     ; <<4 x i32>> [#uses=1]
  store <4 x i32> %168, <4 x i32>* %r2
  %169 = getelementptr inbounds [8 x <4 x i32>]* %temp, i32 0, i32 0 ; <<4 x i32>*> [#uses=1]
  %170 = getelementptr inbounds <4 x i32>* %169, i32 2 ; <<4 x i32>*> [#uses=1]
  %171 = load <4 x i32>* %170                     ; <<4 x i32>> [#uses=1]
  store <4 x i32> %171, <4 x i32>* %a
  %172 = getelementptr inbounds [8 x <4 x i32>]* %temp, i32 0, i32 0 ; <<4 x i32>*> [#uses=1]
  %173 = getelementptr inbounds <4 x i32>* %172, i32 4 ; <<4 x i32>*> [#uses=1]
  %174 = load <4 x i32>* %173                     ; <<4 x i32>> [#uses=1]
  store <4 x i32> %174, <4 x i32>* %b
  br label %176

; <label>:175                                     ; preds = %107
  br label %176

; <label>:176                                     ; preds = %175, %164, %153, %142, %135, %128, %121, %114, %109
  %177 = load <4 x i32>* %a                       ; <<4 x i32>> [#uses=1]
  %178 = load i32* %shift                         ; <i32> [#uses=1]
  call void @lshift128(<4 x i32> %177, i32 %178, <4 x i32>* %e)
  %179 = load <4 x i32>* %r1                      ; <<4 x i32>> [#uses=1]
  %180 = load i32* %shift                         ; <i32> [#uses=1]
  call void @rshift128(<4 x i32> %179, i32 %180, <4 x i32>* %f)
  %181 = load <4 x i32>* %a                       ; <<4 x i32>> [#uses=1]
  %182 = extractelement <4 x i32> %181, i32 0     ; <i32> [#uses=1]
  %183 = load <4 x i32>* %e                       ; <<4 x i32>> [#uses=1]
  %184 = extractelement <4 x i32> %183, i32 0     ; <i32> [#uses=1]
  %185 = xor i32 %182, %184                       ; <i32> [#uses=1]
  %186 = load <4 x i32>* %b                       ; <<4 x i32>> [#uses=1]
  %187 = extractelement <4 x i32> %186, i32 0     ; <i32> [#uses=1]
  %188 = load i32* %thirteen                      ; <i32> [#uses=1]
  %189 = lshr i32 %187, %188                      ; <i32> [#uses=1]
  %190 = load i32* %mask11                        ; <i32> [#uses=1]
  %191 = and i32 %189, %190                       ; <i32> [#uses=1]
  %192 = xor i32 %185, %191                       ; <i32> [#uses=1]
  %193 = load <4 x i32>* %f                       ; <<4 x i32>> [#uses=1]
  %194 = extractelement <4 x i32> %193, i32 0     ; <i32> [#uses=1]
  %195 = xor i32 %192, %194                       ; <i32> [#uses=1]
  %196 = load <4 x i32>* %r2                      ; <<4 x i32>> [#uses=1]
  %197 = extractelement <4 x i32> %196, i32 0     ; <i32> [#uses=1]
  %198 = load i32* %fifteen                       ; <i32> [#uses=1]
  %199 = shl i32 %197, %198                       ; <i32> [#uses=1]
  %200 = xor i32 %195, %199                       ; <i32> [#uses=1]
  %201 = load i32* %i                             ; <i32> [#uses=1]
  %202 = getelementptr inbounds [8 x <4 x i32>]* %temp, i32 0, i32 0 ; <<4 x i32>*> [#uses=1]
  %203 = getelementptr inbounds <4 x i32>* %202, i32 %201 ; <<4 x i32>*> [#uses=2]
  %204 = load <4 x i32>* %203                     ; <<4 x i32>> [#uses=1]
  %205 = insertelement <4 x i32> %204, i32 %200, i32 0 ; <<4 x i32>> [#uses=1]
  store <4 x i32> %205, <4 x i32>* %203
  %206 = load <4 x i32>* %a                       ; <<4 x i32>> [#uses=1]
  %207 = extractelement <4 x i32> %206, i32 1     ; <i32> [#uses=1]
  %208 = load <4 x i32>* %e                       ; <<4 x i32>> [#uses=1]
  %209 = extractelement <4 x i32> %208, i32 1     ; <i32> [#uses=1]
  %210 = xor i32 %207, %209                       ; <i32> [#uses=1]
  %211 = load <4 x i32>* %b                       ; <<4 x i32>> [#uses=1]
  %212 = extractelement <4 x i32> %211, i32 1     ; <i32> [#uses=1]
  %213 = load i32* %thirteen                      ; <i32> [#uses=1]
  %214 = lshr i32 %212, %213                      ; <i32> [#uses=1]
  %215 = load i32* %mask12                        ; <i32> [#uses=1]
  %216 = and i32 %214, %215                       ; <i32> [#uses=1]
  %217 = xor i32 %210, %216                       ; <i32> [#uses=1]
  %218 = load <4 x i32>* %f                       ; <<4 x i32>> [#uses=1]
  %219 = extractelement <4 x i32> %218, i32 1     ; <i32> [#uses=1]
  %220 = xor i32 %217, %219                       ; <i32> [#uses=1]
  %221 = load <4 x i32>* %r2                      ; <<4 x i32>> [#uses=1]
  %222 = extractelement <4 x i32> %221, i32 1     ; <i32> [#uses=1]
  %223 = load i32* %fifteen                       ; <i32> [#uses=1]
  %224 = shl i32 %222, %223                       ; <i32> [#uses=1]
  %225 = xor i32 %220, %224                       ; <i32> [#uses=1]
  %226 = load i32* %i                             ; <i32> [#uses=1]
  %227 = getelementptr inbounds [8 x <4 x i32>]* %temp, i32 0, i32 0 ; <<4 x i32>*> [#uses=1]
  %228 = getelementptr inbounds <4 x i32>* %227, i32 %226 ; <<4 x i32>*> [#uses=2]
  %229 = load <4 x i32>* %228                     ; <<4 x i32>> [#uses=1]
  %230 = insertelement <4 x i32> %229, i32 %225, i32 1 ; <<4 x i32>> [#uses=1]
  store <4 x i32> %230, <4 x i32>* %228
  %231 = load <4 x i32>* %a                       ; <<4 x i32>> [#uses=1]
  %232 = extractelement <4 x i32> %231, i32 2     ; <i32> [#uses=1]
  %233 = load <4 x i32>* %e                       ; <<4 x i32>> [#uses=1]
  %234 = extractelement <4 x i32> %233, i32 2     ; <i32> [#uses=1]
  %235 = xor i32 %232, %234                       ; <i32> [#uses=1]
  %236 = load <4 x i32>* %b                       ; <<4 x i32>> [#uses=1]
  %237 = extractelement <4 x i32> %236, i32 2     ; <i32> [#uses=1]
  %238 = load i32* %thirteen                      ; <i32> [#uses=1]
  %239 = lshr i32 %237, %238                      ; <i32> [#uses=1]
  %240 = load i32* %mask13                        ; <i32> [#uses=1]
  %241 = and i32 %239, %240                       ; <i32> [#uses=1]
  %242 = xor i32 %235, %241                       ; <i32> [#uses=1]
  %243 = load <4 x i32>* %f                       ; <<4 x i32>> [#uses=1]
  %244 = extractelement <4 x i32> %243, i32 2     ; <i32> [#uses=1]
  %245 = xor i32 %242, %244                       ; <i32> [#uses=1]
  %246 = load <4 x i32>* %r2                      ; <<4 x i32>> [#uses=1]
  %247 = extractelement <4 x i32> %246, i32 2     ; <i32> [#uses=1]
  %248 = load i32* %fifteen                       ; <i32> [#uses=1]
  %249 = shl i32 %247, %248                       ; <i32> [#uses=1]
  %250 = xor i32 %245, %249                       ; <i32> [#uses=1]
  %251 = load i32* %i                             ; <i32> [#uses=1]
  %252 = getelementptr inbounds [8 x <4 x i32>]* %temp, i32 0, i32 0 ; <<4 x i32>*> [#uses=1]
  %253 = getelementptr inbounds <4 x i32>* %252, i32 %251 ; <<4 x i32>*> [#uses=2]
  %254 = load <4 x i32>* %253                     ; <<4 x i32>> [#uses=1]
  %255 = insertelement <4 x i32> %254, i32 %250, i32 2 ; <<4 x i32>> [#uses=1]
  store <4 x i32> %255, <4 x i32>* %253
  %256 = load <4 x i32>* %a                       ; <<4 x i32>> [#uses=1]
  %257 = extractelement <4 x i32> %256, i32 3     ; <i32> [#uses=1]
  %258 = load <4 x i32>* %e                       ; <<4 x i32>> [#uses=1]
  %259 = extractelement <4 x i32> %258, i32 3     ; <i32> [#uses=1]
  %260 = xor i32 %257, %259                       ; <i32> [#uses=1]
  %261 = load <4 x i32>* %b                       ; <<4 x i32>> [#uses=1]
  %262 = extractelement <4 x i32> %261, i32 3     ; <i32> [#uses=1]
  %263 = load i32* %thirteen                      ; <i32> [#uses=1]
  %264 = lshr i32 %262, %263                      ; <i32> [#uses=1]
  %265 = load i32* %mask14                        ; <i32> [#uses=1]
  %266 = and i32 %264, %265                       ; <i32> [#uses=1]
  %267 = xor i32 %260, %266                       ; <i32> [#uses=1]
  %268 = load <4 x i32>* %f                       ; <<4 x i32>> [#uses=1]
  %269 = extractelement <4 x i32> %268, i32 3     ; <i32> [#uses=1]
  %270 = xor i32 %267, %269                       ; <i32> [#uses=1]
  %271 = load <4 x i32>* %r2                      ; <<4 x i32>> [#uses=1]
  %272 = extractelement <4 x i32> %271, i32 3     ; <i32> [#uses=1]
  %273 = load i32* %fifteen                       ; <i32> [#uses=1]
  %274 = shl i32 %272, %273                       ; <i32> [#uses=1]
  %275 = xor i32 %270, %274                       ; <i32> [#uses=1]
  %276 = load i32* %i                             ; <i32> [#uses=1]
  %277 = getelementptr inbounds [8 x <4 x i32>]* %temp, i32 0, i32 0 ; <<4 x i32>*> [#uses=1]
  %278 = getelementptr inbounds <4 x i32>* %277, i32 %276 ; <<4 x i32>*> [#uses=2]
  %279 = load <4 x i32>* %278                     ; <<4 x i32>> [#uses=1]
  %280 = insertelement <4 x i32> %279, i32 %275, i32 3 ; <<4 x i32>> [#uses=1]
  store <4 x i32> %280, <4 x i32>* %278
  br label %281

; <label>:281                                     ; preds = %176
  %282 = load i32* %i                             ; <i32> [#uses=1]
  %283 = add i32 %282, 1                          ; <i32> [#uses=1]
  store i32 %283, i32* %i
  br label %103

; <label>:284                                     ; preds = %103
  %285 = load i32* %yPid                          ; <i32> [#uses=1]
  %286 = load i32* %2                             ; <i32> [#uses=1]
  %287 = mul i32 %285, %286                       ; <i32> [#uses=1]
  %288 = load i32* %xPid                          ; <i32> [#uses=1]
  %289 = add i32 %287, %288                       ; <i32> [#uses=1]
  %290 = load i32* %3                             ; <i32> [#uses=1]
  %291 = mul i32 %289, %290                       ; <i32> [#uses=1]
  store i32 %291, i32* %actualPos
  store i32 0, i32* %i
  br label %292

; <label>:292                                     ; preds = %362, %284
  %293 = load i32* %i                             ; <i32> [#uses=1]
  %294 = load i32* %3                             ; <i32> [#uses=1]
  %295 = udiv i32 %294, 2                         ; <i32> [#uses=1]
  %296 = icmp ult i32 %293, %295                  ; <i1> [#uses=1]
  br i1 %296, label %297, label %365

; <label>:297                                     ; preds = %292
  %298 = load i32* %i                             ; <i32> [#uses=1]
  %299 = getelementptr inbounds [8 x <4 x i32>]* %temp, i32 0, i32 0 ; <<4 x i32>*> [#uses=1]
  %300 = getelementptr inbounds <4 x i32>* %299, i32 %298 ; <<4 x i32>*> [#uses=1]
  %301 = load <4 x i32>* %300                     ; <<4 x i32>> [#uses=1]
  %302 = call <4 x float> @_Z14convert_float4U8__vector4j(<4 x i32> %301) ; <<4 x float>> [#uses=1]
  %303 = load float* %one                         ; <float> [#uses=1]
  %304 = insertelement <4 x float> undef, float %303, i32 0 ; <<4 x float>> [#uses=2]
  %305 = shufflevector <4 x float> %304, <4 x float> %304, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %306 = fmul <4 x float> %302, %305              ; <<4 x float>> [#uses=1]
  %307 = load float* %intMax                      ; <float> [#uses=1]
  %308 = insertelement <4 x float> undef, float %307, i32 0 ; <<4 x float>> [#uses=2]
  %309 = shufflevector <4 x float> %308, <4 x float> %308, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %310 = fdiv <4 x float> %306, %309              ; <<4 x float>> [#uses=1]
  store <4 x float> %310, <4 x float>* %temp1
  %311 = load i32* %i                             ; <i32> [#uses=1]
  %312 = add i32 %311, 1                          ; <i32> [#uses=1]
  %313 = getelementptr inbounds [8 x <4 x i32>]* %temp, i32 0, i32 0 ; <<4 x i32>*> [#uses=1]
  %314 = getelementptr inbounds <4 x i32>* %313, i32 %312 ; <<4 x i32>*> [#uses=1]
  %315 = load <4 x i32>* %314                     ; <<4 x i32>> [#uses=1]
  %316 = call <4 x float> @_Z14convert_float4U8__vector4j(<4 x i32> %315) ; <<4 x float>> [#uses=1]
  %317 = load float* %one                         ; <float> [#uses=1]
  %318 = insertelement <4 x float> undef, float %317, i32 0 ; <<4 x float>> [#uses=2]
  %319 = shufflevector <4 x float> %318, <4 x float> %318, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %320 = fmul <4 x float> %316, %319              ; <<4 x float>> [#uses=1]
  %321 = load float* %intMax                      ; <float> [#uses=1]
  %322 = insertelement <4 x float> undef, float %321, i32 0 ; <<4 x float>> [#uses=2]
  %323 = shufflevector <4 x float> %322, <4 x float> %322, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %324 = fdiv <4 x float> %320, %323              ; <<4 x float>> [#uses=1]
  store <4 x float> %324, <4 x float>* %temp2
  %325 = load float* %two                         ; <float> [#uses=1]
  %326 = fsub float -0.000000e+000, %325          ; <float> [#uses=1]
  %327 = insertelement <4 x float> undef, float %326, i32 0 ; <<4 x float>> [#uses=2]
  %328 = shufflevector <4 x float> %327, <4 x float> %327, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %329 = load <4 x float>* %temp1                 ; <<4 x float>> [#uses=1]
  %330 = call <4 x float> @_Z3logU8__vector4f(<4 x float> %329) ; <<4 x float>> [#uses=1]
  %331 = fmul <4 x float> %328, %330              ; <<4 x float>> [#uses=1]
  %332 = call <4 x float> @_Z4sqrtU8__vector4f(<4 x float> %331) ; <<4 x float>> [#uses=1]
  store <4 x float> %332, <4 x float>* %r
  %333 = load float* %two                         ; <float> [#uses=1]
  %334 = load float* %PI                          ; <float> [#uses=1]
  %335 = fmul float %333, %334                    ; <float> [#uses=1]
  %336 = insertelement <4 x float> undef, float %335, i32 0 ; <<4 x float>> [#uses=2]
  %337 = shufflevector <4 x float> %336, <4 x float> %336, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %338 = load <4 x float>* %temp2                 ; <<4 x float>> [#uses=1]
  %339 = fmul <4 x float> %337, %338              ; <<4 x float>> [#uses=1]
  store <4 x float> %339, <4 x float>* %phi
  %340 = load <4 x float>* %r                     ; <<4 x float>> [#uses=1]
  %341 = load <4 x float>* %phi                   ; <<4 x float>> [#uses=1]
  %342 = call <4 x float> @_Z3cosU8__vector4f(<4 x float> %341) ; <<4 x float>> [#uses=1]
  %343 = fmul <4 x float> %340, %342              ; <<4 x float>> [#uses=1]
  %344 = load i32* %actualPos                     ; <i32> [#uses=1]
  %345 = load i32* %i                             ; <i32> [#uses=1]
  %346 = mul i32 %345, 2                          ; <i32> [#uses=1]
  %347 = add i32 %344, %346                       ; <i32> [#uses=1]
  %348 = add i32 %347, 0                          ; <i32> [#uses=1]
  %349 = load <4 x float> addrspace(1)** %4       ; <<4 x float> addrspace(1)*> [#uses=1]
  %350 = getelementptr inbounds <4 x float> addrspace(1)* %349, i32 %348 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %343, <4 x float> addrspace(1)* %350
  %351 = load <4 x float>* %r                     ; <<4 x float>> [#uses=1]
  %352 = load <4 x float>* %phi                   ; <<4 x float>> [#uses=1]
  %353 = call <4 x float> @_Z3sinU8__vector4f(<4 x float> %352) ; <<4 x float>> [#uses=1]
  %354 = fmul <4 x float> %351, %353              ; <<4 x float>> [#uses=1]
  %355 = load i32* %actualPos                     ; <i32> [#uses=1]
  %356 = load i32* %i                             ; <i32> [#uses=1]
  %357 = mul i32 %356, 2                          ; <i32> [#uses=1]
  %358 = add i32 %355, %357                       ; <i32> [#uses=1]
  %359 = add i32 %358, 1                          ; <i32> [#uses=1]
  %360 = load <4 x float> addrspace(1)** %4       ; <<4 x float> addrspace(1)*> [#uses=1]
  %361 = getelementptr inbounds <4 x float> addrspace(1)* %360, i32 %359 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %354, <4 x float> addrspace(1)* %361
  br label %362

; <label>:362                                     ; preds = %297
  %363 = load i32* %i                             ; <i32> [#uses=1]
  %364 = add i32 %363, 1                          ; <i32> [#uses=1]
  store i32 %364, i32* %i
  br label %292

; <label>:365                                     ; preds = %292
  ret void
}

declare i32 @get_global_id(i32)

declare <4 x float> @_Z14convert_float4U8__vector4j(<4 x i32>)

declare <4 x float> @_Z4sqrtU8__vector4f(<4 x float>)

declare <4 x float> @_Z3logU8__vector4f(<4 x float>)

declare <4 x float> @_Z3cosU8__vector4f(<4 x float>)

declare <4 x float> @_Z3sinU8__vector4f(<4 x float>)