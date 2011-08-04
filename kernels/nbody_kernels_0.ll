; ModuleID = '-'
target datalayout = "E-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-n32"
target triple = "GHAL3D_2_1"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_nbody_sim_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_nbody_sim_parameters = appending global [238 x i8] c"float4 __attribute__((address_space(1))) *, float4 __attribute__((address_space(1))) *, int, float, float, float4 __attribute__((address_space(3))) *, float4 __attribute__((address_space(1))) *, float4 __attribute__((address_space(1))) *\00", section "llvm.metadata" ; <[238 x i8]*> [#uses=1]
@opencl_metadata = appending global [1 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (<4 x float> addrspace(1)*, <4 x float> addrspace(1)*, i32, float, float, <4 x float> addrspace(3)*, <4 x float> addrspace(1)*, <4 x float> addrspace(1)*)* @nbody_sim to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_nbody_sim_locals to i8*), i8* getelementptr inbounds ([238 x i8]* @opencl_nbody_sim_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[1 x %opencl_metadata_type]*> [#uses=0]

define void @nbody_sim(<4 x float> addrspace(1)* %pos, <4 x float> addrspace(1)* %vel, i32 %numBodies, float %deltaTime, float %epsSqr, <4 x float> addrspace(3)* %localPos, <4 x float> addrspace(1)* %newPosition, <4 x float> addrspace(1)* %newVelocity) nounwind {
  %1 = alloca <4 x float> addrspace(1)*, align 16 ; <<4 x float> addrspace(1)**> [#uses=3]
  %2 = alloca <4 x float> addrspace(1)*, align 16 ; <<4 x float> addrspace(1)**> [#uses=2]
  %3 = alloca i32, align 4                        ; <i32*> [#uses=2]
  %4 = alloca float, align 4                      ; <float*> [#uses=5]
  %5 = alloca float, align 4                      ; <float*> [#uses=2]
  %6 = alloca <4 x float> addrspace(3)*, align 16 ; <<4 x float> addrspace(3)**> [#uses=4]
  %7 = alloca <4 x float> addrspace(1)*, align 16 ; <<4 x float> addrspace(1)**> [#uses=2]
  %8 = alloca <4 x float> addrspace(1)*, align 16 ; <<4 x float> addrspace(1)**> [#uses=2]
  %tid = alloca i32, align 4                      ; <i32*> [#uses=3]
  %gid = alloca i32, align 4                      ; <i32*> [#uses=5]
  %localSize = alloca i32, align 4                ; <i32*> [#uses=3]
  %numTiles = alloca i32, align 4                 ; <i32*> [#uses=2]
  %myPos = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=4]
  %acc = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=5]
  %9 = alloca <4 x float>, align 16               ; <<4 x float>*> [#uses=2]
  %i = alloca i32, align 4                        ; <i32*> [#uses=5]
  %idx = alloca i32, align 4                      ; <i32*> [#uses=2]
  %j = alloca i32, align 4                        ; <i32*> [#uses=3]
  %r = alloca <4 x float>, align 16               ; <<4 x float>*> [#uses=8]
  %distSqr = alloca float, align 4                ; <float*> [#uses=2]
  %invDist = alloca float, align 4                ; <float*> [#uses=4]
  %invDistCube = alloca float, align 4            ; <float*> [#uses=2]
  %s = alloca float, align 4                      ; <float*> [#uses=2]
  %oldVel = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=3]
  %newPos = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=4]
  %newVel = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=2]
  store <4 x float> addrspace(1)* %pos, <4 x float> addrspace(1)** %1
  store <4 x float> addrspace(1)* %vel, <4 x float> addrspace(1)** %2
  store i32 %numBodies, i32* %3
  store float %deltaTime, float* %4
  store float %epsSqr, float* %5
  store <4 x float> addrspace(3)* %localPos, <4 x float> addrspace(3)** %6
  store <4 x float> addrspace(1)* %newPosition, <4 x float> addrspace(1)** %7
  store <4 x float> addrspace(1)* %newVelocity, <4 x float> addrspace(1)** %8
  %10 = call i32 @get_local_id(i32 0)             ; <i32> [#uses=1]
  store i32 %10, i32* %tid
  %11 = call i32 @get_global_id(i32 0)            ; <i32> [#uses=1]
  store i32 %11, i32* %gid
  %12 = call i32 @get_local_size(i32 0)           ; <i32> [#uses=1]
  store i32 %12, i32* %localSize
  %13 = load i32* %3                              ; <i32> [#uses=1]
  %14 = load i32* %localSize                      ; <i32> [#uses=1]
  %15 = udiv i32 %13, %14                         ; <i32> [#uses=1]
  store i32 %15, i32* %numTiles
  %16 = load i32* %gid                            ; <i32> [#uses=1]
  %17 = load <4 x float> addrspace(1)** %1        ; <<4 x float> addrspace(1)*> [#uses=1]
  %18 = getelementptr inbounds <4 x float> addrspace(1)* %17, i32 %16 ; <<4 x float> addrspace(1)*> [#uses=1]
  %19 = load <4 x float> addrspace(1)* %18        ; <<4 x float>> [#uses=1]
  store <4 x float> %19, <4 x float>* %myPos
  store <4 x float> zeroinitializer, <4 x float>* %9
  %20 = load <4 x float>* %9                      ; <<4 x float>> [#uses=1]
  store <4 x float> %20, <4 x float>* %acc
  store i32 0, i32* %i
  br label %21

; <label>:21                                      ; preds = %85, %0
  %22 = load i32* %i                              ; <i32> [#uses=1]
  %23 = load i32* %numTiles                       ; <i32> [#uses=1]
  %24 = icmp ult i32 %22, %23                     ; <i1> [#uses=1]
  br i1 %24, label %25, label %88

; <label>:25                                      ; preds = %21
  %26 = load i32* %i                              ; <i32> [#uses=1]
  %27 = load i32* %localSize                      ; <i32> [#uses=1]
  %28 = mul i32 %26, %27                          ; <i32> [#uses=1]
  %29 = load i32* %tid                            ; <i32> [#uses=1]
  %30 = add i32 %28, %29                          ; <i32> [#uses=1]
  store i32 %30, i32* %idx
  %31 = load i32* %idx                            ; <i32> [#uses=1]
  %32 = load <4 x float> addrspace(1)** %1        ; <<4 x float> addrspace(1)*> [#uses=1]
  %33 = getelementptr inbounds <4 x float> addrspace(1)* %32, i32 %31 ; <<4 x float> addrspace(1)*> [#uses=1]
  %34 = load <4 x float> addrspace(1)* %33        ; <<4 x float>> [#uses=1]
  %35 = load i32* %tid                            ; <i32> [#uses=1]
  %36 = load <4 x float> addrspace(3)** %6        ; <<4 x float> addrspace(3)*> [#uses=1]
  %37 = getelementptr inbounds <4 x float> addrspace(3)* %36, i32 %35 ; <<4 x float> addrspace(3)*> [#uses=1]
  store <4 x float> %34, <4 x float> addrspace(3)* %37
  call void @barrier(i32 1)
  store i32 0, i32* %j
  %38 = load i32* %j                              ; <i32> [#uses=1]
  %39 = load <4 x float> addrspace(3)** %6        ; <<4 x float> addrspace(3)*> [#uses=1]
  %40 = getelementptr inbounds <4 x float> addrspace(3)* %39, i32 %38 ; <<4 x float> addrspace(3)*> [#uses=1]
  %41 = load <4 x float> addrspace(3)* %40        ; <<4 x float>> [#uses=1]
  %42 = load <4 x float>* %myPos                  ; <<4 x float>> [#uses=1]
  %43 = fsub <4 x float> %41, %42                 ; <<4 x float>> [#uses=1]
  store <4 x float> %43, <4 x float>* %r
  %44 = load <4 x float>* %r                      ; <<4 x float>> [#uses=1]
  %45 = extractelement <4 x float> %44, i32 0     ; <float> [#uses=1]
  %46 = load <4 x float>* %r                      ; <<4 x float>> [#uses=1]
  %47 = extractelement <4 x float> %46, i32 0     ; <float> [#uses=1]
  %48 = fmul float %45, %47                       ; <float> [#uses=1]
  %49 = load <4 x float>* %r                      ; <<4 x float>> [#uses=1]
  %50 = extractelement <4 x float> %49, i32 1     ; <float> [#uses=1]
  %51 = load <4 x float>* %r                      ; <<4 x float>> [#uses=1]
  %52 = extractelement <4 x float> %51, i32 1     ; <float> [#uses=1]
  %53 = fmul float %50, %52                       ; <float> [#uses=1]
  %54 = fadd float %48, %53                       ; <float> [#uses=1]
  %55 = load <4 x float>* %r                      ; <<4 x float>> [#uses=1]
  %56 = extractelement <4 x float> %55, i32 2     ; <float> [#uses=1]
  %57 = load <4 x float>* %r                      ; <<4 x float>> [#uses=1]
  %58 = extractelement <4 x float> %57, i32 2     ; <float> [#uses=1]
  %59 = fmul float %56, %58                       ; <float> [#uses=1]
  %60 = fadd float %54, %59                       ; <float> [#uses=1]
  store float %60, float* %distSqr
  %61 = load float* %distSqr                      ; <float> [#uses=1]
  %62 = load float* %5                            ; <float> [#uses=1]
  %63 = fadd float %61, %62                       ; <float> [#uses=1]
  %64 = call float @_Z4sqrtf(float %63)           ; <float> [#uses=1]
  %65 = fdiv float 1.000000e+000, %64             ; <float> [#uses=1]
  store float %65, float* %invDist
  %66 = load float* %invDist                      ; <float> [#uses=1]
  %67 = load float* %invDist                      ; <float> [#uses=1]
  %68 = fmul float %66, %67                       ; <float> [#uses=1]
  %69 = load float* %invDist                      ; <float> [#uses=1]
  %70 = fmul float %68, %69                       ; <float> [#uses=1]
  store float %70, float* %invDistCube
  %71 = load i32* %j                              ; <i32> [#uses=1]
  %72 = load <4 x float> addrspace(3)** %6        ; <<4 x float> addrspace(3)*> [#uses=1]
  %73 = getelementptr inbounds <4 x float> addrspace(3)* %72, i32 %71 ; <<4 x float> addrspace(3)*> [#uses=1]
  %74 = load <4 x float> addrspace(3)* %73        ; <<4 x float>> [#uses=1]
  %75 = extractelement <4 x float> %74, i32 3     ; <float> [#uses=1]
  %76 = load float* %invDistCube                  ; <float> [#uses=1]
  %77 = fmul float %75, %76                       ; <float> [#uses=1]
  store float %77, float* %s
  %78 = load float* %s                            ; <float> [#uses=1]
  %79 = insertelement <4 x float> undef, float %78, i32 0 ; <<4 x float>> [#uses=2]
  %80 = shufflevector <4 x float> %79, <4 x float> %79, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %81 = load <4 x float>* %r                      ; <<4 x float>> [#uses=1]
  %82 = fmul <4 x float> %80, %81                 ; <<4 x float>> [#uses=1]
  %83 = load <4 x float>* %acc                    ; <<4 x float>> [#uses=1]
  %84 = fadd <4 x float> %83, %82                 ; <<4 x float>> [#uses=1]
  store <4 x float> %84, <4 x float>* %acc
  br label %85

; <label>:85                                      ; preds = %25
  %86 = load i32* %i                              ; <i32> [#uses=1]
  %87 = add nsw i32 %86, 1                        ; <i32> [#uses=1]
  store i32 %87, i32* %i
  br label %21

; <label>:88                                      ; preds = %21
  %89 = load i32* %gid                            ; <i32> [#uses=1]
  %90 = load <4 x float> addrspace(1)** %2        ; <<4 x float> addrspace(1)*> [#uses=1]
  %91 = getelementptr inbounds <4 x float> addrspace(1)* %90, i32 %89 ; <<4 x float> addrspace(1)*> [#uses=1]
  %92 = load <4 x float> addrspace(1)* %91        ; <<4 x float>> [#uses=1]
  store <4 x float> %92, <4 x float>* %oldVel
  %93 = load <4 x float>* %myPos                  ; <<4 x float>> [#uses=1]
  %94 = load <4 x float>* %oldVel                 ; <<4 x float>> [#uses=1]
  %95 = load float* %4                            ; <float> [#uses=1]
  %96 = insertelement <4 x float> undef, float %95, i32 0 ; <<4 x float>> [#uses=2]
  %97 = shufflevector <4 x float> %96, <4 x float> %96, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %98 = fmul <4 x float> %94, %97                 ; <<4 x float>> [#uses=1]
  %99 = fadd <4 x float> %93, %98                 ; <<4 x float>> [#uses=1]
  %100 = load <4 x float>* %acc                   ; <<4 x float>> [#uses=1]
  %101 = fmul <4 x float> %100, <float 5.000000e-001, float 5.000000e-001, float 5.000000e-001, float 5.000000e-001> ; <<4 x float>> [#uses=1]
  %102 = load float* %4                           ; <float> [#uses=1]
  %103 = insertelement <4 x float> undef, float %102, i32 0 ; <<4 x float>> [#uses=2]
  %104 = shufflevector <4 x float> %103, <4 x float> %103, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %105 = fmul <4 x float> %101, %104              ; <<4 x float>> [#uses=1]
  %106 = load float* %4                           ; <float> [#uses=1]
  %107 = insertelement <4 x float> undef, float %106, i32 0 ; <<4 x float>> [#uses=2]
  %108 = shufflevector <4 x float> %107, <4 x float> %107, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %109 = fmul <4 x float> %105, %108              ; <<4 x float>> [#uses=1]
  %110 = fadd <4 x float> %99, %109               ; <<4 x float>> [#uses=1]
  store <4 x float> %110, <4 x float>* %newPos
  %111 = load <4 x float>* %myPos                 ; <<4 x float>> [#uses=1]
  %112 = extractelement <4 x float> %111, i32 3   ; <float> [#uses=1]
  %113 = load <4 x float>* %newPos                ; <<4 x float>> [#uses=1]
  %114 = insertelement <4 x float> %113, float %112, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %114, <4 x float>* %newPos
  %115 = load <4 x float>* %oldVel                ; <<4 x float>> [#uses=1]
  %116 = load <4 x float>* %acc                   ; <<4 x float>> [#uses=1]
  %117 = load float* %4                           ; <float> [#uses=1]
  %118 = insertelement <4 x float> undef, float %117, i32 0 ; <<4 x float>> [#uses=2]
  %119 = shufflevector <4 x float> %118, <4 x float> %118, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %120 = fmul <4 x float> %116, %119              ; <<4 x float>> [#uses=1]
  %121 = fadd <4 x float> %115, %120              ; <<4 x float>> [#uses=1]
  store <4 x float> %121, <4 x float>* %newVel
  %122 = load <4 x float>* %newPos                ; <<4 x float>> [#uses=1]
  %123 = load i32* %gid                           ; <i32> [#uses=1]
  %124 = load <4 x float> addrspace(1)** %7       ; <<4 x float> addrspace(1)*> [#uses=1]
  %125 = getelementptr inbounds <4 x float> addrspace(1)* %124, i32 %123 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %122, <4 x float> addrspace(1)* %125
  %126 = load <4 x float>* %newVel                ; <<4 x float>> [#uses=1]
  %127 = load i32* %gid                           ; <i32> [#uses=1]
  %128 = load <4 x float> addrspace(1)** %8       ; <<4 x float> addrspace(1)*> [#uses=1]
  %129 = getelementptr inbounds <4 x float> addrspace(1)* %128, i32 %127 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %126, <4 x float> addrspace(1)* %129
  ret void
}

declare i32 @get_local_id(i32)

declare i32 @get_global_id(i32)

declare i32 @get_local_size(i32)

declare void @barrier(i32)

declare float @_Z4sqrtf(float)