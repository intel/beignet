; ModuleID = '-'
target datalayout = "E-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-n32"
target triple = "GHAL3D_2_1"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_blackScholes_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_blackScholes_parameters = appending global [142 x i8] c"float4 const __attribute__((address_space(1))) *, int, float4 __attribute__((address_space(1))) *, float4 __attribute__((address_space(1))) *\00", section "llvm.metadata" ; <[142 x i8]*> [#uses=1]
@opencl_metadata = appending global [1 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (<4 x float> addrspace(1)*, i32, <4 x float> addrspace(1)*, <4 x float> addrspace(1)*)* @blackScholes to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_blackScholes_locals to i8*), i8* getelementptr inbounds ([142 x i8]* @opencl_blackScholes_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[1 x %opencl_metadata_type]*> [#uses=0]

define void @phi(<4 x float> %X, <4 x float>* %phi) nounwind {
  %1 = alloca <4 x float>, align 16               ; <<4 x float>*> [#uses=5]
  %2 = alloca <4 x float>*, align 4               ; <<4 x float>**> [#uses=2]
  %y = alloca <4 x float>, align 16               ; <<4 x float>*> [#uses=3]
  %absX = alloca <4 x float>, align 16            ; <<4 x float>*> [#uses=2]
  %t = alloca <4 x float>, align 16               ; <<4 x float>*> [#uses=6]
  %result = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=2]
  %c1 = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=2]
  %c2 = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=2]
  %c3 = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=2]
  %c4 = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=2]
  %c5 = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=2]
  %zero = alloca <4 x float>, align 16            ; <<4 x float>*> [#uses=2]
  %one = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=5]
  %two = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %temp4 = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=2]
  %oneBySqrt2pi = alloca <4 x float>, align 16    ; <<4 x float>*> [#uses=2]
  store <4 x float> %X, <4 x float>* %1
  store <4 x float>* %phi, <4 x float>** %2
  store <4 x float> <float 0x3FD470BF40000000, float 0x3FD470BF40000000, float 0x3FD470BF40000000, float 0x3FD470BF40000000>, <4 x float>* %c1
  store <4 x float> <float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000>, <4 x float>* %c2
  store <4 x float> <float 0x3FFC80EF00000000, float 0x3FFC80EF00000000, float 0x3FFC80EF00000000, float 0x3FFC80EF00000000>, <4 x float>* %c3
  store <4 x float> <float 0xBFFD23DD40000000, float 0xBFFD23DD40000000, float 0xBFFD23DD40000000, float 0xBFFD23DD40000000>, <4 x float>* %c4
  store <4 x float> <float 0x3FF548CDE0000000, float 0x3FF548CDE0000000, float 0x3FF548CDE0000000, float 0x3FF548CDE0000000>, <4 x float>* %c5
  store <4 x float> zeroinitializer, <4 x float>* %zero
  store <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float>* %one
  store <4 x float> <float 2.000000e+000, float 2.000000e+000, float 2.000000e+000, float 2.000000e+000>, <4 x float>* %two
  store <4 x float> <float 0x3FCDA67120000000, float 0x3FCDA67120000000, float 0x3FCDA67120000000, float 0x3FCDA67120000000>, <4 x float>* %temp4
  store <4 x float> <float 0x3FD9884540000000, float 0x3FD9884540000000, float 0x3FD9884540000000, float 0x3FD9884540000000>, <4 x float>* %oneBySqrt2pi
  %3 = load <4 x float>* %1                       ; <<4 x float>> [#uses=1]
  %4 = call <4 x float> @_Z4fabsU8__vector4f(<4 x float> %3) ; <<4 x float>> [#uses=1]
  store <4 x float> %4, <4 x float>* %absX
  %5 = load <4 x float>* %one                     ; <<4 x float>> [#uses=1]
  %6 = load <4 x float>* %one                     ; <<4 x float>> [#uses=1]
  %7 = load <4 x float>* %temp4                   ; <<4 x float>> [#uses=1]
  %8 = load <4 x float>* %absX                    ; <<4 x float>> [#uses=1]
  %9 = fmul <4 x float> %7, %8                    ; <<4 x float>> [#uses=1]
  %10 = fadd <4 x float> %6, %9                   ; <<4 x float>> [#uses=1]
  %11 = fdiv <4 x float> %5, %10                  ; <<4 x float>> [#uses=1]
  store <4 x float> %11, <4 x float>* %t
  %12 = load <4 x float>* %one                    ; <<4 x float>> [#uses=1]
  %13 = load <4 x float>* %oneBySqrt2pi           ; <<4 x float>> [#uses=1]
  %14 = load <4 x float>* %1                      ; <<4 x float>> [#uses=1]
  %15 = fsub <4 x float> <float -0.000000e+000, float -0.000000e+000, float -0.000000e+000, float -0.000000e+000>, %14 ; <<4 x float>> [#uses=1]
  %16 = load <4 x float>* %1                      ; <<4 x float>> [#uses=1]
  %17 = fmul <4 x float> %15, %16                 ; <<4 x float>> [#uses=1]
  %18 = load <4 x float>* %two                    ; <<4 x float>> [#uses=1]
  %19 = fdiv <4 x float> %17, %18                 ; <<4 x float>> [#uses=1]
  %20 = call <4 x float> @_Z3expU8__vector4f(<4 x float> %19) ; <<4 x float>> [#uses=1]
  %21 = fmul <4 x float> %13, %20                 ; <<4 x float>> [#uses=1]
  %22 = load <4 x float>* %t                      ; <<4 x float>> [#uses=1]
  %23 = fmul <4 x float> %21, %22                 ; <<4 x float>> [#uses=1]
  %24 = load <4 x float>* %c1                     ; <<4 x float>> [#uses=1]
  %25 = load <4 x float>* %t                      ; <<4 x float>> [#uses=1]
  %26 = load <4 x float>* %c2                     ; <<4 x float>> [#uses=1]
  %27 = load <4 x float>* %t                      ; <<4 x float>> [#uses=1]
  %28 = load <4 x float>* %c3                     ; <<4 x float>> [#uses=1]
  %29 = load <4 x float>* %t                      ; <<4 x float>> [#uses=1]
  %30 = load <4 x float>* %c4                     ; <<4 x float>> [#uses=1]
  %31 = load <4 x float>* %t                      ; <<4 x float>> [#uses=1]
  %32 = load <4 x float>* %c5                     ; <<4 x float>> [#uses=1]
  %33 = fmul <4 x float> %31, %32                 ; <<4 x float>> [#uses=1]
  %34 = fadd <4 x float> %30, %33                 ; <<4 x float>> [#uses=1]
  %35 = fmul <4 x float> %29, %34                 ; <<4 x float>> [#uses=1]
  %36 = fadd <4 x float> %28, %35                 ; <<4 x float>> [#uses=1]
  %37 = fmul <4 x float> %27, %36                 ; <<4 x float>> [#uses=1]
  %38 = fadd <4 x float> %26, %37                 ; <<4 x float>> [#uses=1]
  %39 = fmul <4 x float> %25, %38                 ; <<4 x float>> [#uses=1]
  %40 = fadd <4 x float> %24, %39                 ; <<4 x float>> [#uses=1]
  %41 = fmul <4 x float> %23, %40                 ; <<4 x float>> [#uses=1]
  %42 = fsub <4 x float> %12, %41                 ; <<4 x float>> [#uses=1]
  store <4 x float> %42, <4 x float>* %y
  %43 = load <4 x float>* %1                      ; <<4 x float>> [#uses=1]
  %44 = load <4 x float>* %zero                   ; <<4 x float>> [#uses=1]
  %45 = fcmp olt <4 x float> %43, %44             ; <<4 x i1>> [#uses=1]
  %46 = sext <4 x i1> %45 to <4 x i32>            ; <<4 x i32>> [#uses=1]
  %47 = load <4 x float>* %one                    ; <<4 x float>> [#uses=1]
  %48 = load <4 x float>* %y                      ; <<4 x float>> [#uses=1]
  %49 = fsub <4 x float> %47, %48                 ; <<4 x float>> [#uses=1]
  %50 = load <4 x float>* %y                      ; <<4 x float>> [#uses=1]
  %51 = ashr <4 x i32> %46, <i32 31, i32 31, i32 31, i32 31> ; <<4 x i32>> [#uses=1]
  %52 = trunc <4 x i32> %51 to <4 x i1>           ; <<4 x i1>> [#uses=1]
  %53 = select <4 x i1> %52, <4 x float> %49, <4 x float> %50 ; <<4 x float>> [#uses=1]
  store <4 x float> %53, <4 x float>* %result
  %54 = load <4 x float>* %result                 ; <<4 x float>> [#uses=1]
  %55 = load <4 x float>** %2                     ; <<4 x float>*> [#uses=1]
  store <4 x float> %54, <4 x float>* %55
  ret void
}

declare <4 x float> @_Z4fabsU8__vector4f(<4 x float>)

declare <4 x float> @_Z3expU8__vector4f(<4 x float>)

define void @blackScholes(<4 x float> addrspace(1)* %randArray, i32 %width, <4 x float> addrspace(1)* %call, <4 x float> addrspace(1)* %put) nounwind {
  %1 = alloca <4 x float> addrspace(1)*, align 16 ; <<4 x float> addrspace(1)**> [#uses=2]
  %2 = alloca i32, align 4                        ; <i32*> [#uses=4]
  %3 = alloca <4 x float> addrspace(1)*, align 16 ; <<4 x float> addrspace(1)**> [#uses=2]
  %4 = alloca <4 x float> addrspace(1)*, align 16 ; <<4 x float> addrspace(1)**> [#uses=2]
  %d1 = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=4]
  %d2 = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=3]
  %phiD1 = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=4]
  %phiD2 = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=4]
  %sigmaSqrtT = alloca <4 x float>, align 16      ; <<4 x float>*> [#uses=3]
  %KexpMinusRT = alloca <4 x float>, align 16     ; <<4 x float>*> [#uses=3]
  %xPos = alloca i32, align 4                     ; <i32*> [#uses=4]
  %yPos = alloca i32, align 4                     ; <i32*> [#uses=4]
  %two = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %inRand = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=11]
  %S = alloca <4 x float>, align 16               ; <<4 x float>*> [#uses=4]
  %K = alloca <4 x float>, align 16               ; <<4 x float>*> [#uses=3]
  %T = alloca <4 x float>, align 16               ; <<4 x float>*> [#uses=4]
  %R = alloca <4 x float>, align 16               ; <<4 x float>*> [#uses=3]
  %sigmaVal = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=4]
  store <4 x float> addrspace(1)* %randArray, <4 x float> addrspace(1)** %1
  store i32 %width, i32* %2
  store <4 x float> addrspace(1)* %call, <4 x float> addrspace(1)** %3
  store <4 x float> addrspace(1)* %put, <4 x float> addrspace(1)** %4
  %5 = call i32 @get_global_id(i32 0)             ; <i32> [#uses=1]
  store i32 %5, i32* %xPos
  %6 = call i32 @get_global_id(i32 1)             ; <i32> [#uses=1]
  store i32 %6, i32* %yPos
  store <4 x float> <float 2.000000e+000, float 2.000000e+000, float 2.000000e+000, float 2.000000e+000>, <4 x float>* %two
  %7 = load i32* %yPos                            ; <i32> [#uses=1]
  %8 = load i32* %2                               ; <i32> [#uses=1]
  %9 = mul i32 %7, %8                             ; <i32> [#uses=1]
  %10 = load i32* %xPos                           ; <i32> [#uses=1]
  %11 = add i32 %9, %10                           ; <i32> [#uses=1]
  %12 = load <4 x float> addrspace(1)** %1        ; <<4 x float> addrspace(1)*> [#uses=1]
  %13 = getelementptr inbounds <4 x float> addrspace(1)* %12, i32 %11 ; <<4 x float> addrspace(1)*> [#uses=1]
  %14 = load <4 x float> addrspace(1)* %13        ; <<4 x float>> [#uses=1]
  store <4 x float> %14, <4 x float>* %inRand
  %15 = load <4 x float>* %inRand                 ; <<4 x float>> [#uses=1]
  %16 = fmul <4 x float> <float 1.000000e+001, float 1.000000e+001, float 1.000000e+001, float 1.000000e+001>, %15 ; <<4 x float>> [#uses=1]
  %17 = load <4 x float>* %inRand                 ; <<4 x float>> [#uses=1]
  %18 = fsub <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, %17 ; <<4 x float>> [#uses=1]
  %19 = fmul <4 x float> <float 1.000000e+002, float 1.000000e+002, float 1.000000e+002, float 1.000000e+002>, %18 ; <<4 x float>> [#uses=1]
  %20 = fadd <4 x float> %16, %19                 ; <<4 x float>> [#uses=1]
  store <4 x float> %20, <4 x float>* %S
  %21 = load <4 x float>* %inRand                 ; <<4 x float>> [#uses=1]
  %22 = fmul <4 x float> <float 1.000000e+001, float 1.000000e+001, float 1.000000e+001, float 1.000000e+001>, %21 ; <<4 x float>> [#uses=1]
  %23 = load <4 x float>* %inRand                 ; <<4 x float>> [#uses=1]
  %24 = fsub <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, %23 ; <<4 x float>> [#uses=1]
  %25 = fmul <4 x float> <float 1.000000e+002, float 1.000000e+002, float 1.000000e+002, float 1.000000e+002>, %24 ; <<4 x float>> [#uses=1]
  %26 = fadd <4 x float> %22, %25                 ; <<4 x float>> [#uses=1]
  store <4 x float> %26, <4 x float>* %K
  %27 = load <4 x float>* %inRand                 ; <<4 x float>> [#uses=1]
  %28 = fmul <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, %27 ; <<4 x float>> [#uses=1]
  %29 = load <4 x float>* %inRand                 ; <<4 x float>> [#uses=1]
  %30 = fsub <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, %29 ; <<4 x float>> [#uses=1]
  %31 = fmul <4 x float> <float 1.000000e+001, float 1.000000e+001, float 1.000000e+001, float 1.000000e+001>, %30 ; <<4 x float>> [#uses=1]
  %32 = fadd <4 x float> %28, %31                 ; <<4 x float>> [#uses=1]
  store <4 x float> %32, <4 x float>* %T
  %33 = load <4 x float>* %inRand                 ; <<4 x float>> [#uses=1]
  %34 = fmul <4 x float> <float 0x3F847AE140000000, float 0x3F847AE140000000, float 0x3F847AE140000000, float 0x3F847AE140000000>, %33 ; <<4 x float>> [#uses=1]
  %35 = load <4 x float>* %inRand                 ; <<4 x float>> [#uses=1]
  %36 = fsub <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, %35 ; <<4 x float>> [#uses=1]
  %37 = fmul <4 x float> <float 0x3FA99999A0000000, float 0x3FA99999A0000000, float 0x3FA99999A0000000, float 0x3FA99999A0000000>, %36 ; <<4 x float>> [#uses=1]
  %38 = fadd <4 x float> %34, %37                 ; <<4 x float>> [#uses=1]
  store <4 x float> %38, <4 x float>* %R
  %39 = load <4 x float>* %inRand                 ; <<4 x float>> [#uses=1]
  %40 = fmul <4 x float> <float 0x3F847AE140000000, float 0x3F847AE140000000, float 0x3F847AE140000000, float 0x3F847AE140000000>, %39 ; <<4 x float>> [#uses=1]
  %41 = load <4 x float>* %inRand                 ; <<4 x float>> [#uses=1]
  %42 = fsub <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, %41 ; <<4 x float>> [#uses=1]
  %43 = fmul <4 x float> <float 0x3FB99999A0000000, float 0x3FB99999A0000000, float 0x3FB99999A0000000, float 0x3FB99999A0000000>, %42 ; <<4 x float>> [#uses=1]
  %44 = fadd <4 x float> %40, %43                 ; <<4 x float>> [#uses=1]
  store <4 x float> %44, <4 x float>* %sigmaVal
  %45 = load <4 x float>* %sigmaVal               ; <<4 x float>> [#uses=1]
  %46 = load <4 x float>* %T                      ; <<4 x float>> [#uses=1]
  %47 = call <4 x float> @_Z4sqrtU8__vector4f(<4 x float> %46) ; <<4 x float>> [#uses=1]
  %48 = fmul <4 x float> %45, %47                 ; <<4 x float>> [#uses=1]
  store <4 x float> %48, <4 x float>* %sigmaSqrtT
  %49 = load <4 x float>* %S                      ; <<4 x float>> [#uses=1]
  %50 = load <4 x float>* %K                      ; <<4 x float>> [#uses=1]
  %51 = fdiv <4 x float> %49, %50                 ; <<4 x float>> [#uses=1]
  %52 = call <4 x float> @_Z3logU8__vector4f(<4 x float> %51) ; <<4 x float>> [#uses=1]
  %53 = load <4 x float>* %R                      ; <<4 x float>> [#uses=1]
  %54 = load <4 x float>* %sigmaVal               ; <<4 x float>> [#uses=1]
  %55 = load <4 x float>* %sigmaVal               ; <<4 x float>> [#uses=1]
  %56 = fmul <4 x float> %54, %55                 ; <<4 x float>> [#uses=1]
  %57 = load <4 x float>* %two                    ; <<4 x float>> [#uses=1]
  %58 = fdiv <4 x float> %56, %57                 ; <<4 x float>> [#uses=1]
  %59 = fadd <4 x float> %53, %58                 ; <<4 x float>> [#uses=1]
  %60 = load <4 x float>* %T                      ; <<4 x float>> [#uses=1]
  %61 = fmul <4 x float> %59, %60                 ; <<4 x float>> [#uses=1]
  %62 = fadd <4 x float> %52, %61                 ; <<4 x float>> [#uses=1]
  %63 = load <4 x float>* %sigmaSqrtT             ; <<4 x float>> [#uses=1]
  %64 = fdiv <4 x float> %62, %63                 ; <<4 x float>> [#uses=1]
  store <4 x float> %64, <4 x float>* %d1
  %65 = load <4 x float>* %d1                     ; <<4 x float>> [#uses=1]
  %66 = load <4 x float>* %sigmaSqrtT             ; <<4 x float>> [#uses=1]
  %67 = fsub <4 x float> %65, %66                 ; <<4 x float>> [#uses=1]
  store <4 x float> %67, <4 x float>* %d2
  %68 = load <4 x float>* %K                      ; <<4 x float>> [#uses=1]
  %69 = load <4 x float>* %R                      ; <<4 x float>> [#uses=1]
  %70 = fsub <4 x float> <float -0.000000e+000, float -0.000000e+000, float -0.000000e+000, float -0.000000e+000>, %69 ; <<4 x float>> [#uses=1]
  %71 = load <4 x float>* %T                      ; <<4 x float>> [#uses=1]
  %72 = fmul <4 x float> %70, %71                 ; <<4 x float>> [#uses=1]
  %73 = call <4 x float> @_Z3expU8__vector4f(<4 x float> %72) ; <<4 x float>> [#uses=1]
  %74 = fmul <4 x float> %68, %73                 ; <<4 x float>> [#uses=1]
  store <4 x float> %74, <4 x float>* %KexpMinusRT
  %75 = load <4 x float>* %d1                     ; <<4 x float>> [#uses=1]
  call void @phi(<4 x float> %75, <4 x float>* %phiD1)
  %76 = load <4 x float>* %d2                     ; <<4 x float>> [#uses=1]
  call void @phi(<4 x float> %76, <4 x float>* %phiD2)
  %77 = load <4 x float>* %S                      ; <<4 x float>> [#uses=1]
  %78 = load <4 x float>* %phiD1                  ; <<4 x float>> [#uses=1]
  %79 = fmul <4 x float> %77, %78                 ; <<4 x float>> [#uses=1]
  %80 = load <4 x float>* %KexpMinusRT            ; <<4 x float>> [#uses=1]
  %81 = load <4 x float>* %phiD2                  ; <<4 x float>> [#uses=1]
  %82 = fmul <4 x float> %80, %81                 ; <<4 x float>> [#uses=1]
  %83 = fsub <4 x float> %79, %82                 ; <<4 x float>> [#uses=1]
  %84 = load i32* %yPos                           ; <i32> [#uses=1]
  %85 = load i32* %2                              ; <i32> [#uses=1]
  %86 = mul i32 %84, %85                          ; <i32> [#uses=1]
  %87 = load i32* %xPos                           ; <i32> [#uses=1]
  %88 = add i32 %86, %87                          ; <i32> [#uses=1]
  %89 = load <4 x float> addrspace(1)** %3        ; <<4 x float> addrspace(1)*> [#uses=1]
  %90 = getelementptr inbounds <4 x float> addrspace(1)* %89, i32 %88 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %83, <4 x float> addrspace(1)* %90
  %91 = load <4 x float>* %d1                     ; <<4 x float>> [#uses=1]
  %92 = fsub <4 x float> <float -0.000000e+000, float -0.000000e+000, float -0.000000e+000, float -0.000000e+000>, %91 ; <<4 x float>> [#uses=1]
  call void @phi(<4 x float> %92, <4 x float>* %phiD1)
  %93 = load <4 x float>* %d2                     ; <<4 x float>> [#uses=1]
  %94 = fsub <4 x float> <float -0.000000e+000, float -0.000000e+000, float -0.000000e+000, float -0.000000e+000>, %93 ; <<4 x float>> [#uses=1]
  call void @phi(<4 x float> %94, <4 x float>* %phiD2)
  %95 = load <4 x float>* %KexpMinusRT            ; <<4 x float>> [#uses=1]
  %96 = load <4 x float>* %phiD2                  ; <<4 x float>> [#uses=1]
  %97 = fmul <4 x float> %95, %96                 ; <<4 x float>> [#uses=1]
  %98 = load <4 x float>* %S                      ; <<4 x float>> [#uses=1]
  %99 = load <4 x float>* %phiD1                  ; <<4 x float>> [#uses=1]
  %100 = fmul <4 x float> %98, %99                ; <<4 x float>> [#uses=1]
  %101 = fsub <4 x float> %97, %100               ; <<4 x float>> [#uses=1]
  %102 = load i32* %yPos                          ; <i32> [#uses=1]
  %103 = load i32* %2                             ; <i32> [#uses=1]
  %104 = mul i32 %102, %103                       ; <i32> [#uses=1]
  %105 = load i32* %xPos                          ; <i32> [#uses=1]
  %106 = add i32 %104, %105                       ; <i32> [#uses=1]
  %107 = load <4 x float> addrspace(1)** %4       ; <<4 x float> addrspace(1)*> [#uses=1]
  %108 = getelementptr inbounds <4 x float> addrspace(1)* %107, i32 %106 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %101, <4 x float> addrspace(1)* %108
  ret void
}

declare i32 @get_global_id(i32)

declare <4 x float> @_Z4sqrtU8__vector4f(<4 x float>)

declare <4 x float> @_Z3logU8__vector4f(<4 x float>)