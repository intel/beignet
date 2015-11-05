#pragma OPENCL EXTENSION cl_khr_fp64 : enable
kernel void compiler_double_convert_int(global double *src, global int *dst0, global uint* dst1) {
  int i = get_global_id(0);

  if (i%3) {
    int i32 = src[i];
    dst0[i] = i32;

    uint u32 = src[i];
    dst1[i] = u32;
  }
}

kernel void compiler_double_convert_float(global double *src, global float *dst) {
  int i = get_global_id(0);

  float f = src[i];
  dst[i] = f;
}

kernel void compiler_double_convert_short(global double *src, global short *dst0, global ushort * dst1) {
  int i = get_global_id(0);

  if (i%3) {
    short i16 = src[i];
    dst0[i] = i16;

    ushort u16 = src[i];
    dst1[i] = u16;
  }
}

kernel void compiler_double_convert_long(global double *src, global long *dst0, global ulong * dst1) {
  int i = get_global_id(0);

  if (i%3) {
    long i64 = src[i];
    dst0[i] = i64;

    ulong u64 = src[i];
    dst1[i] = u64;
  }
}

kernel void compiler_double_convert_char(global double *src, global char *dst0, global uchar * dst1) {
  int i = get_global_id(0);

  if (i%3) {
    char i8 = src[i];
    dst0[i] = i8;

    uchar u8 = src[i];
    dst1[i] = u8;
  }
}

kernel void compiler_long_convert_double(global long *src0, global ulong *src1, global double * dst0, global double *dst1) {
  int i = get_global_id(0);

  double d = src0[i];
  dst0[i] = d;

  d = src1[i];
  dst1[i] = d;
}

kernel void compiler_int_convert_double(global int *src0, global uint *src1, global double * dst0, global double *dst1) {
  int i = get_global_id(0);

  double d = src0[i];
  dst0[i] = d;

  d = src1[i];
  dst1[i] = d;
}

kernel void compiler_short_convert_double(global short *src0, global ushort *src1, global double * dst0, global double *dst1) {
  int i = get_global_id(0);

  double d = src0[i];
  dst0[i] = d;

  d = src1[i];
  dst1[i] = d;
}

kernel void compiler_char_convert_double(global char *src0, global uchar *src1, global double * dst0, global double *dst1) {
  int i = get_global_id(0);

  double d = src0[i];
  dst0[i] = d;

  d = src1[i];
  dst1[i] = d;
}

kernel void compiler_float_convert_double(global float *src, global double *dst) {
  int i = get_global_id(0);

  double d = src[i];
  dst[i] = d;
}
