#define TEST_TYPE(TYPE)                                                           \
__kernel void test_##TYPE(__global TYPE *C, __global TYPE *A, __global TYPE *B) { \
  int id = get_global_id(0);                                                      \
  C[id] = add_sat(A[id], B[id]);                                                  \
}

TEST_TYPE(char)
TEST_TYPE(uchar)
TEST_TYPE(short)
TEST_TYPE(ushort)
TEST_TYPE(int)
TEST_TYPE(uint)
//TEST_TYPE(long)
//TEST_TYPE(ulong)

#undef TEST_TYPE
