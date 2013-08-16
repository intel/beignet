#define COMPILER_ABS_FUNC_1(TYPE, UTYPE) \
    kernel void compiler_abs_diff_##TYPE ( \
           global TYPE* x, global TYPE* y, global UTYPE* diff) { \
        int i = get_global_id(0); \
        diff[i] = abs_diff(x[i], y[i]);     \
    }

#define COMPILER_ABS_FUNC_N(TYPE, UTYPE, N) \
    kernel void compiler_abs_diff_##TYPE##N ( \
           global TYPE##N* x, global TYPE##N* y, global UTYPE##N* diff) { \
        int i = get_global_id(0); \
        diff[i] = abs_diff(x[i], y[i]);     \
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
COMPILER_ABS(long, ulong)
COMPILER_ABS(ulong, ulong)
