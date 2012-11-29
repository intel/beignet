/* test OpenCL 1.1 Async Copies and Prefetch Functions (section 6.11.10) */
kernel void compiler_async_copy_and_prefetch(__global float *p) {
  prefetch(p, 10);
  local float l[10];
  event_t e[2];
  async_work_group_copy(l, p, 10, 0);
  async_work_group_copy(p, l, 10, 0);
  wait_group_events(2, e);
}
