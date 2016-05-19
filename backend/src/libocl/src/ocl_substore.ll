target datalayout = "e-p:32:32-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir"

declare void @__gen_ocl_sub_group_block_write_mem(i32 addrspace(1)* nocapture, i32) nounwind alwaysinline noduplicate

define void @_Z27intel_sub_group_block_writePKU3AS1jj(i32 addrspace(1)* %p, i32 %data) nounwind alwaysinline noduplicate {
  call void @__gen_ocl_sub_group_block_write_mem(i32 addrspace(1)* %p, i32 %data)
  ret void
}
