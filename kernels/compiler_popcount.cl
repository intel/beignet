#define TEST_TYPE(TYPE)                                       \
kernel void test_##TYPE(global TYPE *src, global TYPE *dst) { \
  int i = get_global_id(0);                                   \
  dst[i] = popcount(src[i]);                                  \
}

TEST_TYPE(char)
TEST_TYPE(uchar)
TEST_TYPE(short)
TEST_TYPE(ushort)
TEST_TYPE(int)
TEST_TYPE(uint)
TEST_TYPE(long)
TEST_TYPE(ulong)

#undef TEST_TYPE
