/* test case for OpenCL 1.1 work-item built-in functions */
__kernel void compiler_workitem_builtin()
{
  uint x = get_work_dim();
  size_t y = get_global_size(0);
  y = get_global_id(0);
  y = get_local_size(0);
  y = get_local_id(0);
  y = get_num_groups(0);
  y = get_group_id(0);
  y = get_global_offset(0);
}
