__kernel
void compiler_assignment_operation_in_if(__global int *dst){
  int gidx = (int)get_global_id(0);

  int3 d1 = (int3) (gidx, gidx-1, gidx-3);
  int k = gidx % 5;
  if (k == 1){
    d1.x = d1.y;
  }
  global int * addr = dst + gidx;
  *addr = d1.x;
}
