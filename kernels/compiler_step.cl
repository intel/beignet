#define COMPILER_STEP_FUNC_N(TYPE, N) \
    kernel void compiler_step_##TYPE##N ( \
           global TYPE##N* edge, global TYPE##N* x, global TYPE##N* dst) { \
        int i = get_global_id(0); \
        dst[i] = step(edge[i], x[i]);     \
    }

kernel void compiler_step_float (global float* edge,
                                 global float* x, global float* dst)
{
    int i = get_global_id(0);
    dst[i] = step(edge[i], x[i]);
}

COMPILER_STEP_FUNC_N(float, 2)
COMPILER_STEP_FUNC_N(float, 3)
COMPILER_STEP_FUNC_N(float, 4)
COMPILER_STEP_FUNC_N(float, 8)
COMPILER_STEP_FUNC_N(float, 16)

#define COMPILER_STEPF_FUNC_N(TYPE, N) \
    kernel void compiler_stepf_##TYPE##N ( \
           float edge, global TYPE##N* x, global TYPE##N* dst) { \
        int i = get_global_id(0); \
        dst[i] = step(edge, x[i]);     \
    }

kernel void compiler_stepf_float (float edge, global float* x, global float* dst)
{
    int i = get_global_id(0);
    dst[i] = step(edge, x[i]);
}

COMPILER_STEPF_FUNC_N(float, 2)
COMPILER_STEPF_FUNC_N(float, 3)
COMPILER_STEPF_FUNC_N(float, 4)
COMPILER_STEPF_FUNC_N(float, 8)
COMPILER_STEPF_FUNC_N(float, 16)
