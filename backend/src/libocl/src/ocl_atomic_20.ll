target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"

;32bit version.
define i32 @__gen_ocl_atomic_exchange32(i32 addrspace(4)* nocapture %ptr, i32 %value, i32 %order, i32 %scope) nounwind alwaysinline {
entry:
    %0 = atomicrmw volatile xchg i32 addrspace(4)* %ptr, i32 %value seq_cst
    ret i32 %0
}

define i32 @__gen_ocl_atomic_exchangef(i32 addrspace(4)* nocapture %ptr, i32 %value, i32 %order, i32 %scope) nounwind alwaysinline {
entry:
    %0 = atomicrmw volatile xchg i32 addrspace(4)* %ptr, i32 %value seq_cst
    ret i32 %0
}

define i32 @__gen_ocl_atomic_fetch_add32(i32 addrspace(4)* nocapture %ptr, i32 %value, i32 %order, i32 %scope) nounwind alwaysinline {
entry:
    %0 = atomicrmw volatile add i32 addrspace(4)* %ptr, i32 %value seq_cst
    ret i32 %0
}

define i32 @__gen_ocl_atomic_fetch_addf(i32 addrspace(4)* nocapture %ptr, i32 %value, i32 %order, i32 %scope) nounwind alwaysinline {
entry:
    %0 = atomicrmw volatile add i32 addrspace(4)* %ptr, i32 %value seq_cst
    ret i32 %0
}

define i32 @__gen_ocl_atomic_fetch_sub32(i32 addrspace(4)* nocapture %ptr, i32 %value, i32 %order, i32 %scope) nounwind alwaysinline {
entry:
    %0 = atomicrmw volatile sub i32 addrspace(4)* %ptr, i32 %value seq_cst
    ret i32 %0
}

define i32 @__gen_ocl_atomic_fetch_or32(i32 addrspace(4)* nocapture %ptr, i32 %value, i32 %order, i32 %scope) nounwind alwaysinline {
entry:
    %0 = atomicrmw volatile or i32 addrspace(4)* %ptr, i32 %value seq_cst
    ret i32 %0
}

define i32 @__gen_ocl_atomic_fetch_xor32(i32 addrspace(4)* nocapture %ptr, i32 %value, i32 %order, i32 %scope) nounwind alwaysinline {
entry:
    %0 = atomicrmw volatile xor i32 addrspace(4)* %ptr, i32 %value seq_cst
    ret i32 %0
}

define i32 @__gen_ocl_atomic_fetch_and32(i32 addrspace(4)* nocapture %ptr, i32 %value, i32 %order, i32 %scope) nounwind alwaysinline {
entry:
    %0 = atomicrmw volatile and i32 addrspace(4)* %ptr, i32 %value seq_cst
    ret i32 %0
}

define i32 @__gen_ocl_atomic_fetch_imin32(i32 addrspace(4)* nocapture %ptr, i32 %value, i32 %order, i32 %scope) nounwind alwaysinline {
entry:
    %0 = atomicrmw volatile min i32 addrspace(4)* %ptr, i32 %value seq_cst
    ret i32 %0
}

define i32 @__gen_ocl_atomic_fetch_imax32(i32 addrspace(4)* nocapture %ptr, i32 %value, i32 %order, i32 %scope) nounwind alwaysinline {
entry:
    %0 = atomicrmw volatile max i32 addrspace(4)* %ptr, i32 %value seq_cst
    ret i32 %0
}

define i32 @__gen_ocl_atomic_fetch_umin32(i32 addrspace(4)* nocapture %ptr, i32 %value, i32 %order, i32 %scope) nounwind alwaysinline {
entry:
    %0 = atomicrmw volatile umin i32 addrspace(4)* %ptr, i32 %value seq_cst
    ret i32 %0
}

define i32 @__gen_ocl_atomic_fetch_umax32(i32 addrspace(4)* nocapture %ptr, i32 %value, i32 %order, i32 %scope) nounwind alwaysinline {
entry:
    %0 = atomicrmw volatile umax i32 addrspace(4)* %ptr, i32 %value seq_cst
    ret i32 %0
}

define i32 @__gen_ocl_atomic_compare_exchange_strong32(i32 addrspace(4)* nocapture %ptr,i32 %compare, i32 %value, i32 %success, i32 %failure, i32 %scope) nounwind alwaysinline {
entry:
  %0 = cmpxchg volatile i32 addrspace(4)* %ptr, i32 %compare, i32 %value seq_cst seq_cst
  %1 = extractvalue { i32, i1 } %0, 0
  ret i32 %1
}

define i32 @__gen_ocl_atomic_compare_exchange_weak32(i32 addrspace(4)* nocapture %ptr,i32 %compare, i32 %value, i32 %sucess, i32 %failure, i32 %scope) nounwind alwaysinline {
entry:
  %0 = cmpxchg weak volatile i32 addrspace(4)* %ptr, i32 %compare, i32 %value seq_cst seq_cst
  %1 = extractvalue { i32, i1 } %0, 0
  ret i32 %1
}

;64bit version

define i64 @__gen_ocl_atomic_exchange64(i64 addrspace(4)* nocapture %ptr, i64 %value, i32 %order, i32 %scope) nounwind alwaysinline {
entry:
    %0 = atomicrmw volatile xchg i64 addrspace(4)* %ptr, i64 %value seq_cst
    ret i64 %0
}

define i64 @__gen_ocl_atomic_fetch_add64(i64 addrspace(4)* nocapture %ptr, i64 %value, i32 %order, i32 %scope) nounwind alwaysinline {
entry:
    %0 = atomicrmw volatile add i64 addrspace(4)* %ptr, i64 %value seq_cst
    ret i64 %0
}

define i64 @__gen_ocl_atomic_fetch_sub64(i64 addrspace(4)* nocapture %ptr, i64 %value, i32 %order, i32 %scope) nounwind alwaysinline {
entry:
    %0 = atomicrmw volatile sub i64 addrspace(4)* %ptr, i64 %value seq_cst
    ret i64 %0
}

define i64 @__gen_ocl_atomic_fetch_or64(i64 addrspace(4)* nocapture %ptr, i64 %value, i32 %order, i32 %scope) nounwind alwaysinline {
entry:
    %0 = atomicrmw volatile or i64 addrspace(4)* %ptr, i64 %value seq_cst
    ret i64 %0
}

define i64 @__gen_ocl_atomic_fetch_xor64(i64 addrspace(4)* nocapture %ptr, i64 %value, i32 %order, i32 %scope) nounwind alwaysinline {
entry:
    %0 = atomicrmw volatile xor i64 addrspace(4)* %ptr, i64 %value seq_cst
    ret i64 %0
}

define i64 @__gen_ocl_atomic_fetch_and64(i64 addrspace(4)* nocapture %ptr, i64 %value, i32 %order, i32 %scope) nounwind alwaysinline {
entry:
    %0 = atomicrmw volatile and i64 addrspace(4)* %ptr, i64 %value seq_cst
    ret i64 %0
}

define i64 @__gen_ocl_atomic_fetch_imin64(i64 addrspace(4)* nocapture %ptr, i64 %value, i32 %order, i32 %scope) nounwind alwaysinline {
entry:
    %0 = atomicrmw volatile min i64 addrspace(4)* %ptr, i64 %value seq_cst
    ret i64 %0
}

define i64 @__gen_ocl_atomic_fetch_imax64(i64 addrspace(4)* nocapture %ptr, i64 %value, i32 %order, i32 %scope) nounwind alwaysinline {
entry:
    %0 = atomicrmw volatile max i64 addrspace(4)* %ptr, i64 %value seq_cst
    ret i64 %0
}

define i64 @__gen_ocl_atomic_fetch_umin64(i64 addrspace(4)* nocapture %ptr, i64 %value, i32 %order, i32 %scope) nounwind alwaysinline {
entry:
    %0 = atomicrmw volatile umin i64 addrspace(4)* %ptr, i64 %value seq_cst
    ret i64 %0
}

define i64 @__gen_ocl_atomic_fetch_umax64(i64 addrspace(4)* nocapture %ptr, i64 %value, i32 %order, i32 %scope) nounwind alwaysinline {
entry:
    %0 = atomicrmw volatile umax i64 addrspace(4)* %ptr, i64 %value seq_cst
    ret i64 %0
}

define i64 @__gen_ocl_atomic_compare_exchange_strong64(i64 addrspace(4)* nocapture %ptr,i64 %compare, i64 %value, i32 %sucess, i32 %failure, i32 %scope) nounwind alwaysinline {
entry:
  %0 = cmpxchg volatile i64 addrspace(4)* %ptr, i64 %compare, i64 %value seq_cst seq_cst
  %1 = extractvalue { i64, i1 } %0, 0
  ret i64 %1
}

define i64 @__gen_ocl_atomic_compare_exchange_weak64(i64 addrspace(4)* nocapture %ptr,i64 %compare, i64 %value, i32 %sucess, i32 %failure, i32 %scope) nounwind alwaysinline {
entry:
  %0 = cmpxchg weak volatile i64 addrspace(4)* %ptr, i64 %compare, i64 %value seq_cst seq_cst
  %1 = extractvalue { i64, i1 } %0, 0
  ret i64 %1
}
