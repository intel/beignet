#define TEST_TYPE(TYPE, LENGTH)                                       \
kernel void compiler_bswap_##TYPE(global TYPE * src, global TYPE * dst){ \
   dst[get_global_id(0)]= __builtin_bswap##LENGTH(src[get_global_id(0)]); \
}


TEST_TYPE(short, 16)
TEST_TYPE(ushort, 16)
TEST_TYPE(int, 32)
TEST_TYPE(uint, 32)

#undef TEST_TYPE
