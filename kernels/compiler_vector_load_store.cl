/* test OpenCL 1.1 Vector Data Load/Store Functions (section 6.11.7) */
#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#define OFFSET2(type)  (type ##2) {(type)1, (type)2}
#define OFFSET3(type)  (type ##3) {(type)1, (type)2, (type)3}
#define OFFSET4(type)  (type ##4) {(type)1, (type)2, (type)3, (type)4}
#define OFFSET8(type)  (type ##8) {(type)1, (type)2, (type)3, (type)4, (type)5, (type)6, (type)7, (type)8}
#define OFFSET16(type)  (type ##16)  {(type)1, (type)2, (type)3, (type)4, (type)5, (type)6, (type)7, (type)8, (type)9, (type)10, (type)11, (type)12, (type)13, (type)14, (type)15, (type)16}

#define  TEST_TYPE(type, n) \
__kernel void test_##type ##n(__global type *pin, \
                            __global type *pout)  \
{\
  int x = get_global_id(0); \
  type ##n value; \
  value = vload ##n(x, pin); \
  value += OFFSET ##n(type); \
  vstore ##n(value, x, pout); \
}

#define TEST_ALL_TYPE(n) \
  TEST_TYPE(char,n)  \
  TEST_TYPE(uchar,n) \
  TEST_TYPE(short,n) \
  TEST_TYPE(ushort,n)\
  TEST_TYPE(int,n)   \
  TEST_TYPE(uint,n)  \
  TEST_TYPE(float,n) \
  TEST_TYPE(long,n)  \
  TEST_TYPE(ulong,n)
//  TEST_TYPE(double,n)

#if 0
  TEST_TYPE(half,n)
#endif

TEST_ALL_TYPE(2)
TEST_ALL_TYPE(3)
TEST_ALL_TYPE(4)
TEST_ALL_TYPE(8)
TEST_ALL_TYPE(16)
