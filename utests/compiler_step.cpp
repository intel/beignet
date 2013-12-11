#include "utest_helper.hpp"
#include "string.h"

template <typename T, int N>
struct cl_vec {
    T ptr[((N+1)/2)*2]; //align to 2 elements.

    typedef cl_vec<T, N> vec_type;

    cl_vec(void) {
        memset(ptr, 0, sizeof(T) * ((N+1)/2)*2);
    }
    cl_vec(vec_type & other) {
        memset(ptr, 0, sizeof(T) * ((N+1)/2)*2);
        memcpy (this->ptr, other.ptr, sizeof(T) * N);
    }

    vec_type& operator= (vec_type & other) {
        memset(ptr, 0, sizeof(T) * ((N+1)/2)*2);
        memcpy (this->ptr, other.ptr, sizeof(T) * N);
        return *this;
    }

    template <typename U> vec_type& operator= (cl_vec<U, N> & other) {
        memset(ptr, 0, sizeof(T) * ((N+1)/2)*2);
        memcpy (this->ptr, other.ptr, sizeof(T) * N);
        return *this;
    }

    bool operator== (vec_type & other) {
        return !memcmp (this->ptr, other.ptr, sizeof(T) * N);
    }

    void step (vec_type & other) {
        int i = 0;
        for (; i < N; i++) {
            T a = ptr[i];
            T edge = other.ptr[i];
            T f = a < edge ? 0.0 : 1.0;
            ptr[i] = f;
        }
    }

    void step (float & edge) {
        int i = 0;
        for (; i < N; i++) {
            T a = ptr[i];
            T f = a < edge ? 0.0 : 1.0;
            ptr[i] = f;
        }
    }
};

template <typename T, typename U, int N> static void cpu (int global_id,
        cl_vec<T, N> *edge, cl_vec<T, N> *src, cl_vec<U, N> *dst)
{
    cl_vec<T, N> v  = src[global_id];
    v.step(edge[global_id]);
    dst[global_id] = v;
}

template <typename T, typename U> static void cpu(int global_id, T *edge, T *src, U *dst)
{
    T f = src[global_id];
    T e = edge[global_id];
    f = f < e ? 0.0 : 1.0;
    dst[global_id] = (U)f;
}

template <typename T, typename U, int N> static void cpu (int global_id,
        float edge, cl_vec<T, N> *src, cl_vec<U, N> *dst)
{
    cl_vec<T, N> v  = src[global_id];
    v.step(edge);
    dst[global_id] = v;
}

template <typename T, typename U> static void cpu(int global_id, float edge, T *src, U *dst)
{
    T f = src[global_id];
    f = f < edge ? 0.0 : 1.0;
    dst[global_id] = (U)f;
}

template <typename T, int N> static void gen_rand_val (cl_vec<T, N>& vect)
{
    int i = 0;

    memset(vect.ptr, 0, sizeof(T) * ((N+1)/2)*2);
    for (; i < N; i++) {
        vect.ptr[i] = static_cast<T>(.1f * (rand() & 15) - .75f);
    }
}

template <typename T> static void gen_rand_val (T & val)
{
    val = static_cast<T>(.1f * (rand() & 15) - .75f);
}

template <typename T>
inline static void print_data (T& val)
{
    if (std::is_unsigned<T>::value)
        printf(" %u", val);
    else
        printf(" %d", val);
}

inline static void print_data (float& val)
{
    printf(" %f", val);
}

template <typename T, typename U, int N> static void dump_data (cl_vec<T, N>* edge,
        cl_vec<T, N>* src, cl_vec<U, N>* dst, int n)
{
    U* val = reinterpret_cast<U *>(dst);

    n = n*((N+1)/2)*2;

    printf("\nEdge: \n");
    for (int32_t i = 0; i < (int32_t) n; ++i) {
        print_data(((T *)buf_data[0])[i]);
    }
    printf("\nx: \n");
    for (int32_t i = 0; i < (int32_t) n; ++i) {
        print_data(((T *)buf_data[1])[i]);
    }

    printf("\nCPU: \n");
    for (int32_t i = 0; i < (int32_t) n; ++i) {
        print_data(val[i]);
    }
    printf("\nGPU: \n");
    for (int32_t i = 0; i < (int32_t) n; ++i) {
        print_data(((U *)buf_data[2])[i]);
    }
}

template <typename T, typename U> static void dump_data (T* edge, T* src, U* dst, int n)
{
    printf("\nedge: \n");
    for (int32_t i = 0; i < (int32_t) n; ++i) {
        print_data(((T *)buf_data[0])[i]);
    }

    printf("\nx: \n");
    for (int32_t i = 0; i < (int32_t) n; ++i) {
        print_data(((T *)buf_data[1])[i]);
    }

    printf("\nCPU: \n");
    for (int32_t i = 0; i < (int32_t) n; ++i) {
        print_data(dst[i]);
    }
    printf("\nGPU: \n");
    for (int32_t i = 0; i < (int32_t) n; ++i) {
        print_data(((U *)buf_data[2])[i]);
    }
}

template <typename T, typename U, int N> static void dump_data (float edge,
        cl_vec<T, N>* src, cl_vec<U, N>* dst, int n)
{
    U* val = reinterpret_cast<U *>(dst);

    n = n*((N+1)/2)*2;

    printf("\nEdge: %f\n", edge);
    printf("\nx: \n");
    for (int32_t i = 0; i < (int32_t) n; ++i) {
        print_data(((T *)buf_data[0])[i]);
    }

    printf("\nCPU: \n");
    for (int32_t i = 0; i < (int32_t) n; ++i) {
        print_data(val[i]);
    }
    printf("\nGPU: \n");
    for (int32_t i = 0; i < (int32_t) n; ++i) {
        print_data(((U *)buf_data[1])[i]);
    }
}

template <typename T, typename U> static void dump_data (float edge, T* src, U* dst, int n)
{
    printf("\nedge: %f\n", edge);
    printf("\nx: \n");
    for (int32_t i = 0; i < (int32_t) n; ++i) {
        print_data(((T *)buf_data[0])[i]);
    }

    printf("\nCPU: \n");
    for (int32_t i = 0; i < (int32_t) n; ++i) {
        print_data(dst[i]);
    }
    printf("\nGPU: \n");
    for (int32_t i = 0; i < (int32_t) n; ++i) {
        print_data(((U *)buf_data[1])[i]);
    }
}

template <typename T> static void compiler_step_with_type(void)
{
    const size_t n = 16;
    T cpu_dst[n], cpu_src[n];
    T edge[n];

    // Setup buffers
    OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(T), NULL);
    OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(T), NULL);
    OCL_CREATE_BUFFER(buf[2], 0, n * sizeof(T), NULL);
    OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
    OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
    OCL_SET_ARG(2, sizeof(cl_mem), &buf[2]);
    globals[0] = n;
    locals[0] = n;

    // Run random tests
    for (uint32_t pass = 0; pass < 8; ++pass) {
        OCL_MAP_BUFFER(0);
        OCL_MAP_BUFFER(1);

        /* Clear the dst buffer to avoid random data. */
        OCL_MAP_BUFFER(2);
        memset(buf_data[2], 0, sizeof(T) * n);
        OCL_UNMAP_BUFFER(2);

        for (int32_t i = 0; i < (int32_t) n; ++i) {
            gen_rand_val(cpu_src[i]);
            gen_rand_val(edge[i]);
        }

        memcpy(buf_data[1], cpu_src, sizeof(T) * n);
        memcpy(buf_data[0], edge, sizeof(T) * n);

        // Run the kernel on GPU
        OCL_NDRANGE(1);

        // Run on CPU
        for (int32_t i = 0; i < (int32_t) n; ++i)
            cpu(i, edge, cpu_src, cpu_dst);

        // Compare
        OCL_MAP_BUFFER(2);

        //dump_data(edge, cpu_src, cpu_dst, n);

        OCL_ASSERT(!memcmp(buf_data[2], cpu_dst, sizeof(T) * n));
        OCL_UNMAP_BUFFER(2);
        OCL_UNMAP_BUFFER(1);
        OCL_UNMAP_BUFFER(0);
    }
}

#define STEP_TEST_TYPE(TYPE) \
	static void compiler_step_##TYPE (void) \
        { \
           OCL_CALL (cl_kernel_init, "compiler_step.cl", "compiler_step_"#TYPE, SOURCE, NULL);  \
           compiler_step_with_type<TYPE>(); \
        } \
	MAKE_UTEST_FROM_FUNCTION(compiler_step_##TYPE);

typedef cl_vec<float, 2> float2;
typedef cl_vec<float, 3> float3;
typedef cl_vec<float, 4> float4;
typedef cl_vec<float, 8> float8;
typedef cl_vec<float, 16> float16;
STEP_TEST_TYPE(float)
STEP_TEST_TYPE(float2)
STEP_TEST_TYPE(float3)
STEP_TEST_TYPE(float4)
STEP_TEST_TYPE(float8)
STEP_TEST_TYPE(float16)


template <typename T> static void compiler_stepf_with_type(void)
{
    const size_t n = 16;
    T cpu_dst[n], cpu_src[n];
    float edge = (float)(.1f * (rand() & 15) - .75f);

    // Setup buffers
    OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(T), NULL);
    OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(T), NULL);
    OCL_SET_ARG(0, sizeof(float), &edge);
    OCL_SET_ARG(1, sizeof(cl_mem), &buf[0]);
    OCL_SET_ARG(2, sizeof(cl_mem), &buf[1]);
    globals[0] = n;
    locals[0] = n;

    // Run random tests
    for (uint32_t pass = 0; pass < 8; ++pass) {
        OCL_MAP_BUFFER(0);

        /* Clear the dst buffer to avoid random data. */
        OCL_MAP_BUFFER(1);
        memset(buf_data[1], 0, sizeof(T) * n);
        OCL_UNMAP_BUFFER(1);

        for (int32_t i = 0; i < (int32_t) n; ++i) {
            gen_rand_val(cpu_src[i]);
        }

        memcpy(buf_data[0], cpu_src, sizeof(T) * n);

        // Run the kernel on GPU
        OCL_NDRANGE(1);

        // Run on CPU
        for (int32_t i = 0; i < (int32_t) n; ++i)
            cpu(i, edge, cpu_src, cpu_dst);

        // Compare
        OCL_MAP_BUFFER(1);

        //dump_data(edge, cpu_src, cpu_dst, n);

        OCL_ASSERT(!memcmp(buf_data[1], cpu_dst, sizeof(T) * n));
        OCL_UNMAP_BUFFER(1);
        OCL_UNMAP_BUFFER(0);
    }
}

#define _STEPF_TEST_TYPE(TYPE, keep_program) \
	static void compiler_stepf_##TYPE (void) \
        { \
           OCL_CALL (cl_kernel_init, "compiler_step.cl", "compiler_stepf_"#TYPE, SOURCE, NULL);  \
           compiler_stepf_with_type<TYPE>(); \
        } \
	MAKE_UTEST_FROM_FUNCTION_KEEP_PROGRAM(compiler_stepf_##TYPE, keep_program);

#define STEPF_TEST_TYPE(TYPE) _STEPF_TEST_TYPE(TYPE, true)
#define STEPF_TEST_TYPE_END(TYPE) _STEPF_TEST_TYPE(TYPE, false)


STEPF_TEST_TYPE(float)
STEPF_TEST_TYPE(float2)
STEPF_TEST_TYPE(float3)
STEPF_TEST_TYPE(float4)
STEPF_TEST_TYPE(float8)
STEPF_TEST_TYPE_END(float16)
