#define COMPILER_ABS_FUNC_N(N) \
    kernel void __cl_fill_region_align8_##N ( global float##N* dst, float##N pattern, \
                                              unsigned int offset, unsigned int size) { \
         int i = get_global_id(0); \
         if (i < size) { \
             dst[i+offset] = pattern; \
         }  \
    }


COMPILER_ABS_FUNC_N(2)
COMPILER_ABS_FUNC_N(4)
COMPILER_ABS_FUNC_N(8)
COMPILER_ABS_FUNC_N(16)
