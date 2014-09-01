;XXX FIXME as llvm can't use macros, we hardcoded 3, 1, 2
;here, we may need to use a more grace way to handle this type
;of values latter.
;#define CLK_LOCAL_MEM_FENCE  (1 << 0)
;#define CLK_GLOBAL_MEM_FENCE (1 << 1)

declare i32 @_get_local_mem_fence() nounwind alwaysinline
declare i32 @_get_global_mem_fence() nounwind alwaysinline
declare void @__gen_ocl_barrier_local() nounwind alwaysinline noduplicate
declare void @__gen_ocl_barrier_global() nounwind alwaysinline noduplicate
declare void @__gen_ocl_barrier_local_and_global() nounwind alwaysinline noduplicate

define void @barrier(i32 %flags) nounwind noduplicate alwaysinline {
  %1 = icmp eq i32 %flags, 3
  br i1 %1, label %barrier_local_global, label %barrier_local_check

barrier_local_global:
  call void @__gen_ocl_barrier_local_and_global()
  br label %done

barrier_local_check:
  %2 = icmp eq i32 %flags, 1
  br i1 %2, label %barrier_local, label %barrier_global_check

barrier_local:
  call void @__gen_ocl_barrier_local()
  br label %done

barrier_global_check:
  %3 = icmp eq i32 %flags, 2
  br i1 %3, label %barrier_global, label %done

barrier_global:
  call void @__gen_ocl_barrier_global()
  br label %done

done:
  ret void
}
