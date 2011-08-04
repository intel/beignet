; ModuleID = '-'
target datalayout = "E-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-n32"
target triple = "GHAL3D_2_1"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_binomial_options_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_binomial_options_parameters = appending global [186 x i8] c"int, float4 const __attribute__((address_space(1))) *, float4 __attribute__((address_space(1))) *, float4 __attribute__((address_space(3))) *, float4 __attribute__((address_space(3))) *\00", section "llvm.metadata" ; <[186 x i8]*> [#uses=1]
@opencl_metadata = appending global [1 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (i32, <4 x float> addrspace(1)*, <4 x float> addrspace(1)*, <4 x float> addrspace(3)*, <4 x float> addrspace(3)*)* @binomial_options to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_binomial_options_locals to i8*), i8* getelementptr inbounds ([186 x i8]* @opencl_binomial_options_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[1 x %opencl_metadata_type]*> [#uses=0]

define void @binomial_options(i32 %numSteps, <4 x float> addrspace(1)* %randArray, <4 x float> addrspace(1)* %output, <4 x float> addrspace(3)* %callA, <4 x float> addrspace(3)* %callB) nounwind {
  %1 = alloca i32, align 4                        ; <i32*> [#uses=4]
  %2 = alloca <4 x float> addrspace(1)*, align 16 ; <<4 x float> addrspace(1)**> [#uses=2]
  %3 = alloca <4 x float> addrspace(1)*, align 16 ; <<4 x float> addrspace(1)**> [#uses=2]
  %4 = alloca <4 x float> addrspace(3)*, align 16 ; <<4 x float> addrspace(3)**> [#uses=9]
  %5 = alloca <4 x float> addrspace(3)*, align 16 ; <<4 x float> addrspace(3)**> [#uses=4]
  %tid = alloca i32, align 4                      ; <i32*> [#uses=15]
  %bid = alloca i32, align 4                      ; <i32*> [#uses=3]
  %inRand = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=7]
  %s = alloca <4 x float>, align 16               ; <<4 x float>*> [#uses=2]
  %x = alloca <4 x float>, align 16               ; <<4 x float>*> [#uses=2]
  %optionYears = alloca <4 x float>, align 16     ; <<4 x float>*> [#uses=2]
  %dt = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=3]
  %vsdt = alloca <4 x float>, align 16            ; <<4 x float>*> [#uses=3]
  %rdt = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %r = alloca <4 x float>, align 16               ; <<4 x float>*> [#uses=3]
  %rInv = alloca <4 x float>, align 16            ; <<4 x float>*> [#uses=3]
  %u = alloca <4 x float>, align 16               ; <<4 x float>*> [#uses=3]
  %d = alloca <4 x float>, align 16               ; <<4 x float>*> [#uses=3]
  %pu = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=3]
  %pd = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=2]
  %puByr = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=3]
  %pdByr = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=3]
  %profit = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=9]
  %j = alloca i32, align 4                        ; <i32*> [#uses=6]
  store i32 %numSteps, i32* %1
  store <4 x float> addrspace(1)* %randArray, <4 x float> addrspace(1)** %2
  store <4 x float> addrspace(1)* %output, <4 x float> addrspace(1)** %3
  store <4 x float> addrspace(3)* %callA, <4 x float> addrspace(3)** %4
  store <4 x float> addrspace(3)* %callB, <4 x float> addrspace(3)** %5
  %6 = call i32 @get_local_id(i32 0)              ; <i32> [#uses=1]
  store i32 %6, i32* %tid
  %7 = call i32 @get_group_id(i32 0)              ; <i32> [#uses=1]
  store i32 %7, i32* %bid
  %8 = load i32* %bid                             ; <i32> [#uses=1]
  %9 = load <4 x float> addrspace(1)** %2         ; <<4 x float> addrspace(1)*> [#uses=1]
  %10 = getelementptr inbounds <4 x float> addrspace(1)* %9, i32 %8 ; <<4 x float> addrspace(1)*> [#uses=1]
  %11 = load <4 x float> addrspace(1)* %10        ; <<4 x float>> [#uses=1]
  store <4 x float> %11, <4 x float>* %inRand
  %12 = load <4 x float>* %inRand                 ; <<4 x float>> [#uses=1]
  %13 = fsub <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, %12 ; <<4 x float>> [#uses=1]
  %14 = fmul <4 x float> %13, <float 5.000000e+000, float 5.000000e+000, float 5.000000e+000, float 5.000000e+000> ; <<4 x float>> [#uses=1]
  %15 = load <4 x float>* %inRand                 ; <<4 x float>> [#uses=1]
  %16 = fmul <4 x float> %15, <float 3.000000e+001, float 3.000000e+001, float 3.000000e+001, float 3.000000e+001> ; <<4 x float>> [#uses=1]
  %17 = fadd <4 x float> %14, %16                 ; <<4 x float>> [#uses=1]
  store <4 x float> %17, <4 x float>* %s
  %18 = load <4 x float>* %inRand                 ; <<4 x float>> [#uses=1]
  %19 = fsub <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, %18 ; <<4 x float>> [#uses=1]
  %20 = fmul <4 x float> %19, <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000> ; <<4 x float>> [#uses=1]
  %21 = load <4 x float>* %inRand                 ; <<4 x float>> [#uses=1]
  %22 = fmul <4 x float> %21, <float 1.000000e+002, float 1.000000e+002, float 1.000000e+002, float 1.000000e+002> ; <<4 x float>> [#uses=1]
  %23 = fadd <4 x float> %20, %22                 ; <<4 x float>> [#uses=1]
  store <4 x float> %23, <4 x float>* %x
  %24 = load <4 x float>* %inRand                 ; <<4 x float>> [#uses=1]
  %25 = fsub <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, %24 ; <<4 x float>> [#uses=1]
  %26 = fmul <4 x float> %25, <float 2.500000e-001, float 2.500000e-001, float 2.500000e-001, float 2.500000e-001> ; <<4 x float>> [#uses=1]
  %27 = load <4 x float>* %inRand                 ; <<4 x float>> [#uses=1]
  %28 = fmul <4 x float> %27, <float 1.000000e+001, float 1.000000e+001, float 1.000000e+001, float 1.000000e+001> ; <<4 x float>> [#uses=1]
  %29 = fadd <4 x float> %26, %28                 ; <<4 x float>> [#uses=1]
  store <4 x float> %29, <4 x float>* %optionYears
  %30 = load <4 x float>* %optionYears            ; <<4 x float>> [#uses=1]
  %31 = load i32* %1                              ; <i32> [#uses=1]
  %32 = sitofp i32 %31 to float                   ; <float> [#uses=1]
  %33 = fdiv float 1.000000e+000, %32             ; <float> [#uses=1]
  %34 = insertelement <4 x float> undef, float %33, i32 0 ; <<4 x float>> [#uses=2]
  %35 = shufflevector <4 x float> %34, <4 x float> %34, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %36 = fmul <4 x float> %30, %35                 ; <<4 x float>> [#uses=1]
  store <4 x float> %36, <4 x float>* %dt
  %37 = load <4 x float>* %dt                     ; <<4 x float>> [#uses=1]
  %38 = call <4 x float> @_Z4sqrtU8__vector4f(<4 x float> %37) ; <<4 x float>> [#uses=1]
  %39 = fmul <4 x float> <float 0x3FD3333340000000, float 0x3FD3333340000000, float 0x3FD3333340000000, float 0x3FD3333340000000>, %38 ; <<4 x float>> [#uses=1]
  store <4 x float> %39, <4 x float>* %vsdt
  %40 = load <4 x float>* %dt                     ; <<4 x float>> [#uses=1]
  %41 = fmul <4 x float> <float 0x3F947AE140000000, float 0x3F947AE140000000, float 0x3F947AE140000000, float 0x3F947AE140000000>, %40 ; <<4 x float>> [#uses=1]
  store <4 x float> %41, <4 x float>* %rdt
  %42 = load <4 x float>* %rdt                    ; <<4 x float>> [#uses=1]
  %43 = call <4 x float> @_Z3expU8__vector4f(<4 x float> %42) ; <<4 x float>> [#uses=1]
  store <4 x float> %43, <4 x float>* %r
  %44 = load <4 x float>* %r                      ; <<4 x float>> [#uses=1]
  %45 = fdiv <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, %44 ; <<4 x float>> [#uses=1]
  store <4 x float> %45, <4 x float>* %rInv
  %46 = load <4 x float>* %vsdt                   ; <<4 x float>> [#uses=1]
  %47 = call <4 x float> @_Z3expU8__vector4f(<4 x float> %46) ; <<4 x float>> [#uses=1]
  store <4 x float> %47, <4 x float>* %u
  %48 = load <4 x float>* %u                      ; <<4 x float>> [#uses=1]
  %49 = fdiv <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, %48 ; <<4 x float>> [#uses=1]
  store <4 x float> %49, <4 x float>* %d
  %50 = load <4 x float>* %r                      ; <<4 x float>> [#uses=1]
  %51 = load <4 x float>* %d                      ; <<4 x float>> [#uses=1]
  %52 = fsub <4 x float> %50, %51                 ; <<4 x float>> [#uses=1]
  %53 = load <4 x float>* %u                      ; <<4 x float>> [#uses=1]
  %54 = load <4 x float>* %d                      ; <<4 x float>> [#uses=1]
  %55 = fsub <4 x float> %53, %54                 ; <<4 x float>> [#uses=1]
  %56 = fdiv <4 x float> %52, %55                 ; <<4 x float>> [#uses=1]
  store <4 x float> %56, <4 x float>* %pu
  %57 = load <4 x float>* %pu                     ; <<4 x float>> [#uses=1]
  %58 = fsub <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, %57 ; <<4 x float>> [#uses=1]
  store <4 x float> %58, <4 x float>* %pd
  %59 = load <4 x float>* %pu                     ; <<4 x float>> [#uses=1]
  %60 = load <4 x float>* %rInv                   ; <<4 x float>> [#uses=1]
  %61 = fmul <4 x float> %59, %60                 ; <<4 x float>> [#uses=1]
  store <4 x float> %61, <4 x float>* %puByr
  %62 = load <4 x float>* %pd                     ; <<4 x float>> [#uses=1]
  %63 = load <4 x float>* %rInv                   ; <<4 x float>> [#uses=1]
  %64 = fmul <4 x float> %62, %63                 ; <<4 x float>> [#uses=1]
  store <4 x float> %64, <4 x float>* %pdByr
  %65 = load <4 x float>* %s                      ; <<4 x float>> [#uses=1]
  %66 = load <4 x float>* %vsdt                   ; <<4 x float>> [#uses=1]
  %67 = load i32* %tid                            ; <i32> [#uses=1]
  %68 = uitofp i32 %67 to float                   ; <float> [#uses=1]
  %69 = fmul float 2.000000e+000, %68             ; <float> [#uses=1]
  %70 = load i32* %1                              ; <i32> [#uses=1]
  %71 = sitofp i32 %70 to float                   ; <float> [#uses=1]
  %72 = fsub float %69, %71                       ; <float> [#uses=1]
  %73 = insertelement <4 x float> undef, float %72, i32 0 ; <<4 x float>> [#uses=2]
  %74 = shufflevector <4 x float> %73, <4 x float> %73, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %75 = fmul <4 x float> %66, %74                 ; <<4 x float>> [#uses=1]
  %76 = call <4 x float> @_Z3expU8__vector4f(<4 x float> %75) ; <<4 x float>> [#uses=1]
  %77 = fmul <4 x float> %65, %76                 ; <<4 x float>> [#uses=1]
  %78 = load <4 x float>* %x                      ; <<4 x float>> [#uses=1]
  %79 = fsub <4 x float> %77, %78                 ; <<4 x float>> [#uses=1]
  store <4 x float> %79, <4 x float>* %profit
  %80 = load <4 x float>* %profit                 ; <<4 x float>> [#uses=1]
  %81 = extractelement <4 x float> %80, i32 0     ; <float> [#uses=1]
  %82 = fcmp ogt float %81, 0.000000e+000         ; <i1> [#uses=1]
  %83 = zext i1 %82 to i32                        ; <i32> [#uses=1]
  %84 = load <4 x float>* %profit                 ; <<4 x float>> [#uses=1]
  %85 = extractelement <4 x float> %84, i32 0     ; <float> [#uses=1]
  %86 = icmp ne i32 %83, 0                        ; <i1> [#uses=1]
  %87 = select i1 %86, float %85, float 0.000000e+000 ; <float> [#uses=1]
  %88 = load i32* %tid                            ; <i32> [#uses=1]
  %89 = load <4 x float> addrspace(3)** %4        ; <<4 x float> addrspace(3)*> [#uses=1]
  %90 = getelementptr inbounds <4 x float> addrspace(3)* %89, i32 %88 ; <<4 x float> addrspace(3)*> [#uses=2]
  %91 = load <4 x float> addrspace(3)* %90        ; <<4 x float>> [#uses=1]
  %92 = insertelement <4 x float> %91, float %87, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %92, <4 x float> addrspace(3)* %90
  %93 = load <4 x float>* %profit                 ; <<4 x float>> [#uses=1]
  %94 = extractelement <4 x float> %93, i32 1     ; <float> [#uses=1]
  %95 = fcmp ogt float %94, 0.000000e+000         ; <i1> [#uses=1]
  %96 = zext i1 %95 to i32                        ; <i32> [#uses=1]
  %97 = load <4 x float>* %profit                 ; <<4 x float>> [#uses=1]
  %98 = extractelement <4 x float> %97, i32 1     ; <float> [#uses=1]
  %99 = icmp ne i32 %96, 0                        ; <i1> [#uses=1]
  %100 = select i1 %99, float %98, float 0.000000e+000 ; <float> [#uses=1]
  %101 = load i32* %tid                           ; <i32> [#uses=1]
  %102 = load <4 x float> addrspace(3)** %4       ; <<4 x float> addrspace(3)*> [#uses=1]
  %103 = getelementptr inbounds <4 x float> addrspace(3)* %102, i32 %101 ; <<4 x float> addrspace(3)*> [#uses=2]
  %104 = load <4 x float> addrspace(3)* %103      ; <<4 x float>> [#uses=1]
  %105 = insertelement <4 x float> %104, float %100, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %105, <4 x float> addrspace(3)* %103
  %106 = load <4 x float>* %profit                ; <<4 x float>> [#uses=1]
  %107 = extractelement <4 x float> %106, i32 2   ; <float> [#uses=1]
  %108 = fcmp ogt float %107, 0.000000e+000       ; <i1> [#uses=1]
  %109 = zext i1 %108 to i32                      ; <i32> [#uses=1]
  %110 = load <4 x float>* %profit                ; <<4 x float>> [#uses=1]
  %111 = extractelement <4 x float> %110, i32 2   ; <float> [#uses=1]
  %112 = icmp ne i32 %109, 0                      ; <i1> [#uses=1]
  %113 = select i1 %112, float %111, float 0.000000e+000 ; <float> [#uses=1]
  %114 = load i32* %tid                           ; <i32> [#uses=1]
  %115 = load <4 x float> addrspace(3)** %4       ; <<4 x float> addrspace(3)*> [#uses=1]
  %116 = getelementptr inbounds <4 x float> addrspace(3)* %115, i32 %114 ; <<4 x float> addrspace(3)*> [#uses=2]
  %117 = load <4 x float> addrspace(3)* %116      ; <<4 x float>> [#uses=1]
  %118 = insertelement <4 x float> %117, float %113, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %118, <4 x float> addrspace(3)* %116
  %119 = load <4 x float>* %profit                ; <<4 x float>> [#uses=1]
  %120 = extractelement <4 x float> %119, i32 3   ; <float> [#uses=1]
  %121 = fcmp ogt float %120, 0.000000e+000       ; <i1> [#uses=1]
  %122 = zext i1 %121 to i32                      ; <i32> [#uses=1]
  %123 = load <4 x float>* %profit                ; <<4 x float>> [#uses=1]
  %124 = extractelement <4 x float> %123, i32 3   ; <float> [#uses=1]
  %125 = icmp ne i32 %122, 0                      ; <i1> [#uses=1]
  %126 = select i1 %125, float %124, float 0.000000e+000 ; <float> [#uses=1]
  %127 = load i32* %tid                           ; <i32> [#uses=1]
  %128 = load <4 x float> addrspace(3)** %4       ; <<4 x float> addrspace(3)*> [#uses=1]
  %129 = getelementptr inbounds <4 x float> addrspace(3)* %128, i32 %127 ; <<4 x float> addrspace(3)*> [#uses=2]
  %130 = load <4 x float> addrspace(3)* %129      ; <<4 x float>> [#uses=1]
  %131 = insertelement <4 x float> %130, float %126, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %131, <4 x float> addrspace(3)* %129
  call void @barrier(i32 1)
  %132 = load i32* %1                             ; <i32> [#uses=1]
  store i32 %132, i32* %j
  br label %133

; <label>:133                                     ; preds = %182, %0
  %134 = load i32* %j                             ; <i32> [#uses=1]
  %135 = icmp sgt i32 %134, 0                     ; <i1> [#uses=1]
  br i1 %135, label %136, label %185

; <label>:136                                     ; preds = %133
  %137 = load i32* %tid                           ; <i32> [#uses=1]
  %138 = load i32* %j                             ; <i32> [#uses=1]
  %139 = icmp ult i32 %137, %138                  ; <i1> [#uses=1]
  br i1 %139, label %140, label %158

; <label>:140                                     ; preds = %136
  %141 = load <4 x float>* %puByr                 ; <<4 x float>> [#uses=1]
  %142 = load i32* %tid                           ; <i32> [#uses=1]
  %143 = load <4 x float> addrspace(3)** %4       ; <<4 x float> addrspace(3)*> [#uses=1]
  %144 = getelementptr inbounds <4 x float> addrspace(3)* %143, i32 %142 ; <<4 x float> addrspace(3)*> [#uses=1]
  %145 = load <4 x float> addrspace(3)* %144      ; <<4 x float>> [#uses=1]
  %146 = fmul <4 x float> %141, %145              ; <<4 x float>> [#uses=1]
  %147 = load <4 x float>* %pdByr                 ; <<4 x float>> [#uses=1]
  %148 = load i32* %tid                           ; <i32> [#uses=1]
  %149 = add i32 %148, 1                          ; <i32> [#uses=1]
  %150 = load <4 x float> addrspace(3)** %4       ; <<4 x float> addrspace(3)*> [#uses=1]
  %151 = getelementptr inbounds <4 x float> addrspace(3)* %150, i32 %149 ; <<4 x float> addrspace(3)*> [#uses=1]
  %152 = load <4 x float> addrspace(3)* %151      ; <<4 x float>> [#uses=1]
  %153 = fmul <4 x float> %147, %152              ; <<4 x float>> [#uses=1]
  %154 = fadd <4 x float> %146, %153              ; <<4 x float>> [#uses=1]
  %155 = load i32* %tid                           ; <i32> [#uses=1]
  %156 = load <4 x float> addrspace(3)** %5       ; <<4 x float> addrspace(3)*> [#uses=1]
  %157 = getelementptr inbounds <4 x float> addrspace(3)* %156, i32 %155 ; <<4 x float> addrspace(3)*> [#uses=1]
  store <4 x float> %154, <4 x float> addrspace(3)* %157
  br label %158

; <label>:158                                     ; preds = %140, %136
  call void @barrier(i32 1)
  %159 = load i32* %tid                           ; <i32> [#uses=1]
  %160 = load i32* %j                             ; <i32> [#uses=1]
  %161 = sub i32 %160, 1                          ; <i32> [#uses=1]
  %162 = icmp ult i32 %159, %161                  ; <i1> [#uses=1]
  br i1 %162, label %163, label %181

; <label>:163                                     ; preds = %158
  %164 = load <4 x float>* %puByr                 ; <<4 x float>> [#uses=1]
  %165 = load i32* %tid                           ; <i32> [#uses=1]
  %166 = load <4 x float> addrspace(3)** %5       ; <<4 x float> addrspace(3)*> [#uses=1]
  %167 = getelementptr inbounds <4 x float> addrspace(3)* %166, i32 %165 ; <<4 x float> addrspace(3)*> [#uses=1]
  %168 = load <4 x float> addrspace(3)* %167      ; <<4 x float>> [#uses=1]
  %169 = fmul <4 x float> %164, %168              ; <<4 x float>> [#uses=1]
  %170 = load <4 x float>* %pdByr                 ; <<4 x float>> [#uses=1]
  %171 = load i32* %tid                           ; <i32> [#uses=1]
  %172 = add i32 %171, 1                          ; <i32> [#uses=1]
  %173 = load <4 x float> addrspace(3)** %5       ; <<4 x float> addrspace(3)*> [#uses=1]
  %174 = getelementptr inbounds <4 x float> addrspace(3)* %173, i32 %172 ; <<4 x float> addrspace(3)*> [#uses=1]
  %175 = load <4 x float> addrspace(3)* %174      ; <<4 x float>> [#uses=1]
  %176 = fmul <4 x float> %170, %175              ; <<4 x float>> [#uses=1]
  %177 = fadd <4 x float> %169, %176              ; <<4 x float>> [#uses=1]
  %178 = load i32* %tid                           ; <i32> [#uses=1]
  %179 = load <4 x float> addrspace(3)** %4       ; <<4 x float> addrspace(3)*> [#uses=1]
  %180 = getelementptr inbounds <4 x float> addrspace(3)* %179, i32 %178 ; <<4 x float> addrspace(3)*> [#uses=1]
  store <4 x float> %177, <4 x float> addrspace(3)* %180
  br label %181

; <label>:181                                     ; preds = %163, %158
  call void @barrier(i32 1)
  br label %182

; <label>:182                                     ; preds = %181
  %183 = load i32* %j                             ; <i32> [#uses=1]
  %184 = sub i32 %183, 2                          ; <i32> [#uses=1]
  store i32 %184, i32* %j
  br label %133

; <label>:185                                     ; preds = %133
  %186 = load i32* %tid                           ; <i32> [#uses=1]
  %187 = icmp eq i32 %186, 0                      ; <i1> [#uses=1]
  br i1 %187, label %188, label %195

; <label>:188                                     ; preds = %185
  %189 = load <4 x float> addrspace(3)** %4       ; <<4 x float> addrspace(3)*> [#uses=1]
  %190 = getelementptr inbounds <4 x float> addrspace(3)* %189, i32 0 ; <<4 x float> addrspace(3)*> [#uses=1]
  %191 = load <4 x float> addrspace(3)* %190      ; <<4 x float>> [#uses=1]
  %192 = load i32* %bid                           ; <i32> [#uses=1]
  %193 = load <4 x float> addrspace(1)** %3       ; <<4 x float> addrspace(1)*> [#uses=1]
  %194 = getelementptr inbounds <4 x float> addrspace(1)* %193, i32 %192 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %191, <4 x float> addrspace(1)* %194
  br label %195

; <label>:195                                     ; preds = %188, %185
  ret void
}

declare i32 @get_local_id(i32)

declare i32 @get_group_id(i32)

declare <4 x float> @_Z4sqrtU8__vector4f(<4 x float>)

declare <4 x float> @_Z3expU8__vector4f(<4 x float>)

declare void @barrier(i32)