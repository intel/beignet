;XXX FIXME as llvm can't use macros, we hardcoded 3, 1, 2
;here, we may need to use a more grace way to handle this type
;of values latter.
;#define CLK_LOCAL_MEM_FENCE  (1 << 0)
;#define CLK_GLOBAL_MEM_FENCE (1 << 1)

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"

declare i32 @_get_local_mem_fence() nounwind alwaysinline
declare i32 @_get_global_mem_fence() nounwind alwaysinline
declare void @__gen_ocl_barrier_local() nounwind alwaysinline noduplicate
declare void @__gen_ocl_barrier_global() nounwind alwaysinline noduplicate
declare void @__gen_ocl_debugwait() nounwind alwaysinline noduplicate
declare void @__gen_ocl_barrier(i32) nounwind alwaysinline noduplicate

define void @_Z7barrierj(i32 %flags) nounwind noduplicate alwaysinline {
  call void @__gen_ocl_barrier(i32 %flags)
  ret void
}

define void @_Z9debugwaitv() nounwind noduplicate alwaysinline {
  call void @__gen_ocl_debugwait()
  ret void
}
