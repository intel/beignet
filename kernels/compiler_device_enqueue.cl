void block_fn(__global uint* val)
{
  atomic_add(val, get_global_id(0));
}

kernel void compiler_device_enqueue(uint glob_size_arr, __global uint* val)
{
  size_t tid = get_global_id(0);

  for(int i = 0; i < glob_size_arr; i++)
  {
    ndrange_t ndrange = ndrange_1D(glob_size_arr);
    __global uint * v = val + tid;
    void (^kernelBlock)(void) = ^{ block_fn(v); };
    queue_t q = get_default_queue();
    enqueue_kernel(q, CLK_ENQUEUE_FLAGS_WAIT_KERNEL, ndrange, kernelBlock);
  }
}
