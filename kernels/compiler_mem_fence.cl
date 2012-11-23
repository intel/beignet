kernel void compiler_mem_fence() {
  barrier(CLK_LOCAL_MEM_FENCE);
  barrier(CLK_GLOBAL_MEM_FENCE);
  mem_fence(CLK_LOCAL_MEM_FENCE);
  mem_fence(CLK_GLOBAL_MEM_FENCE);
  read_mem_fence(CLK_LOCAL_MEM_FENCE);
  read_mem_fence(CLK_GLOBAL_MEM_FENCE);
  write_mem_fence(CLK_LOCAL_MEM_FENCE);
  write_mem_fence(CLK_GLOBAL_MEM_FENCE);
}
