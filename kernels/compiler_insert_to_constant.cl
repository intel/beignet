__kernel void compiler_insert_to_constant(__global int4 *dst) {
  int4 value = (int4)(0,1,2,3);
  value.z = get_global_id(0);
  dst[get_global_id(0)] = value;
}

