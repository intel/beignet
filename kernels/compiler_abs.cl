#define COMPILER_ABS_FUNC_1(TYPE, UTYPE) \
    kernel void compiler_abs_##TYPE ( \
           global TYPE* src, global UTYPE* dst) { \
        int i = get_global_id(0); \
        dst[i] = abs(src[i]);     \
    }

#define COMPILER_ABS_FUNC_N(TYPE, UTYPE, N) \
    kernel void compiler_abs_##TYPE##N ( \
           global TYPE##N* src, global UTYPE##N* dst) { \
        int i = get_global_id(0); \
        dst[i] = abs(src[i]);     \
    }

#define COMPILER_ABS(TYPE, UTYPE)  \
    COMPILER_ABS_FUNC_1(TYPE, UTYPE) \
    COMPILER_ABS_FUNC_N(TYPE, UTYPE, 2) \
    COMPILER_ABS_FUNC_N(TYPE, UTYPE, 3) \
    COMPILER_ABS_FUNC_N(TYPE, UTYPE, 4) \
    COMPILER_ABS_FUNC_N(TYPE, UTYPE, 8) \
    COMPILER_ABS_FUNC_N(TYPE, UTYPE, 16)

COMPILER_ABS(int, uint)
COMPILER_ABS(uint, uint)
COMPILER_ABS(char, uchar)
COMPILER_ABS(uchar, uchar)
COMPILER_ABS(short, ushort)
COMPILER_ABS(ushort, ushort)
