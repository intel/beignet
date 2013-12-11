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

    void abs_diff(vec_type & other) {
        int i = 0;
        for (; i < N; i++) {
            T a = ptr[i];
            T b = other.ptr[i];
            T f = a > b ? (a - b) : (b - a);
            ptr[i] = f;
        }
    }
};

template <typename T, typename U, int N> static void cpu (int global_id,
        cl_vec<T, N> *x, cl_vec<T, N> *y, cl_vec<U, N> *diff)
{
    cl_vec<T, N> v  = x[global_id];
    v.abs_diff(y[global_id]);
    diff[global_id] = v;
}

template <typename T, typename U> static void cpu(int global_id, T *x, T *y, U *diff)
{
    T a = x[global_id];
    T b = y[global_id];
    U f = a > b ? (a - b) : (b - a);
    diff[global_id] = f;
}

template <typename T, int N> static void gen_rand_val (cl_vec<T, N>& vect)
{
    int i = 0;
    for (; i < N; i++) {
        vect.ptr[i] = static_cast<T>((rand() & 63) - 32);
    }
}

template <typename T> static void gen_rand_val (T & val)
{
    val = static_cast<T>((rand() & 63) - 32);
}

template <typename T>
inline static void print_data (T& val)
{
    if (std::is_unsigned<T>::value)
        printf(" %u", val);
    else
        printf(" %d", val);
}

template <typename T, typename U, int N> static void dump_data (cl_vec<T, N>* x,
        cl_vec<T, N>* y, cl_vec<U, N>* diff, int n)
{
    U* val = reinterpret_cast<U *>(diff);

    n = n*((N+1)/2)*2;

    printf("\nRaw x: \n");
    for (int32_t i = 0; i < (int32_t) n; ++i) {
        print_data(((T *)buf_data[0])[i]);
    }
    printf("\nRaw y: \n");
    for (int32_t i = 0; i < (int32_t) n; ++i) {
        print_data(((T *)buf_data[1])[i]);
    }

    printf("\nCPU diff: \n");
    for (int32_t i = 0; i < (int32_t) n; ++i) {
        print_data(val[i]);
    }
    printf("\nGPU diff: \n");
    for (int32_t i = 0; i < (int32_t) n; ++i) {
        print_data(((U *)buf_data[2])[i]);
    }
}

template <typename T, typename U> static void dump_data (T* x, T* y, U* diff, int n)
{
    printf("\nRaw x: \n");
    for (int32_t i = 0; i < (int32_t) n; ++i) {
        print_data(((T *)buf_data[0])[i]);
    }
    printf("\nRaw y: \n");
    for (int32_t i = 0; i < (int32_t) n; ++i) {
        print_data(((T *)buf_data[1])[i]);
    }

    printf("\nCPU diff: \n");
    for (int32_t i = 0; i < (int32_t) n; ++i) {
        print_data(diff[i]);
    }
    printf("\nGPU diff: \n");
    for (int32_t i = 0; i < (int32_t) n; ++i) {
        print_data(((U *)buf_data[2])[i]);
    }
}

template <typename T, typename U> static void compiler_abs_diff_with_type(void)
{
    const size_t n = 16;
    U cpu_diff[16];
    T cpu_x[16];
    T cpu_y[16];

    // Setup buffers
    OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(T), NULL);
    OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(T), NULL);
    OCL_CREATE_BUFFER(buf[2], 0, n * sizeof(U), NULL);
    OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
    OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
    OCL_SET_ARG(2, sizeof(cl_mem), &buf[2]);
    globals[0] = 16;
    locals[0] = 16;

    // Run random tests
    for (uint32_t pass = 0; pass < 8; ++pass) {
        OCL_MAP_BUFFER(0);
        OCL_MAP_BUFFER(1);

        /* Clear the dst buffer to avoid random data. */
        OCL_MAP_BUFFER(2);
        memset(buf_data[2], 0, sizeof(U) * n);
        OCL_UNMAP_BUFFER(2);

        for (int32_t i = 0; i < (int32_t) n; ++i) {
            gen_rand_val(cpu_x[i]);
            gen_rand_val(cpu_y[i]);
        }

        memcpy(buf_data[0], cpu_x, sizeof(T) * n);
        memcpy(buf_data[1], cpu_y, sizeof(T) * n);

        // Run the kernel on GPU
        OCL_NDRANGE(1);

        // Run on CPU
        for (int32_t i = 0; i < (int32_t) n; ++i)
            cpu(i, cpu_x, cpu_y, cpu_diff);

        // Compare
        OCL_MAP_BUFFER(2);

//      dump_data(cpu_x, cpu_y, cpu_diff, n);

        OCL_ASSERT(!memcmp(buf_data[2], cpu_diff, sizeof(T) * n));

        OCL_UNMAP_BUFFER(0);
        OCL_UNMAP_BUFFER(1);
        OCL_UNMAP_BUFFER(2);
    }
}


#define ABS_TEST_DIFF_TYPE_2(TYPE, CLTYPE, UTYPE, KEEP_PROGRAM) \
	static void compiler_abs_diff_##CLTYPE (void) \
        { \
           OCL_CALL (cl_kernel_init, "compiler_abs_diff.cl", "compiler_abs_diff_"#CLTYPE, SOURCE, NULL);  \
           compiler_abs_diff_with_type<TYPE, UTYPE>(); \
        } \
	MAKE_UTEST_FROM_FUNCTION_KEEP_PROGRAM(compiler_abs_diff_##CLTYPE, KEEP_PROGRAM);

#define ABS_TEST_DIFF_TYPE(TYPE, UTYPE) ABS_TEST_DIFF_TYPE_2(TYPE, TYPE, UTYPE, true)

#define ABS_TEST_DIFF_TYPE_END(TYPE, UTYPE) ABS_TEST_DIFF_TYPE_2(TYPE, TYPE, UTYPE, false)


typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef uint64_t ulong64;
ABS_TEST_DIFF_TYPE(int, uint)
ABS_TEST_DIFF_TYPE_2(int64_t, long, ulong64, true)
ABS_TEST_DIFF_TYPE(short, ushort)
ABS_TEST_DIFF_TYPE(char, uchar)
ABS_TEST_DIFF_TYPE(uint, uint)
ABS_TEST_DIFF_TYPE_2(ulong64, ulong, ulong64, true)
ABS_TEST_DIFF_TYPE(ushort, ushort)
ABS_TEST_DIFF_TYPE(uchar, uchar)

typedef cl_vec<int, 2> int2;
typedef cl_vec<int, 3> int3;
typedef cl_vec<int, 4> int4;
typedef cl_vec<int, 8> int8;
typedef cl_vec<int, 16> int16;
typedef cl_vec<unsigned int, 2> uint2;
typedef cl_vec<unsigned int, 3> uint3;
typedef cl_vec<unsigned int, 4> uint4;
typedef cl_vec<unsigned int, 8> uint8;
typedef cl_vec<unsigned int, 16> uint16;
ABS_TEST_DIFF_TYPE(int2, uint2)
ABS_TEST_DIFF_TYPE(int3, uint3)
ABS_TEST_DIFF_TYPE(int4, uint4)
ABS_TEST_DIFF_TYPE(int8, uint8)
ABS_TEST_DIFF_TYPE(int16, uint16)
ABS_TEST_DIFF_TYPE(uint2, uint2)
ABS_TEST_DIFF_TYPE(uint3, uint3)
ABS_TEST_DIFF_TYPE(uint4, uint4)
ABS_TEST_DIFF_TYPE(uint8, uint8)
ABS_TEST_DIFF_TYPE(uint16, uint16)

typedef cl_vec<int64_t, 2> long2;
typedef cl_vec<int64_t, 3> long3;
typedef cl_vec<int64_t, 4> long4;
typedef cl_vec<int64_t, 8> long8;
typedef cl_vec<int64_t, 16> long16;
typedef cl_vec<uint64_t, 2> ulong2;
typedef cl_vec<uint64_t, 3> ulong3;
typedef cl_vec<uint64_t, 4> ulong4;
typedef cl_vec<uint64_t, 8> ulong8;
typedef cl_vec<uint64_t, 16> ulong16;
ABS_TEST_DIFF_TYPE(long2, ulong2)
ABS_TEST_DIFF_TYPE(long3, ulong3)
ABS_TEST_DIFF_TYPE(long4, ulong4)
ABS_TEST_DIFF_TYPE(long8, ulong8)
ABS_TEST_DIFF_TYPE(long16, ulong16)
ABS_TEST_DIFF_TYPE(ulong2, ulong2)
ABS_TEST_DIFF_TYPE(ulong3, ulong3)
ABS_TEST_DIFF_TYPE(ulong4, ulong4)
ABS_TEST_DIFF_TYPE(ulong8, ulong8)
ABS_TEST_DIFF_TYPE(ulong16, ulong16)

typedef cl_vec<char, 2> char2;
typedef cl_vec<char, 3> char3;
typedef cl_vec<char, 4> char4;
typedef cl_vec<char, 8> char8;
typedef cl_vec<char, 16> char16;
typedef cl_vec<unsigned char, 2> uchar2;
typedef cl_vec<unsigned char, 3> uchar3;
typedef cl_vec<unsigned char, 4> uchar4;
typedef cl_vec<unsigned char, 8> uchar8;
typedef cl_vec<unsigned char, 16> uchar16;
ABS_TEST_DIFF_TYPE(char2, uchar2)
ABS_TEST_DIFF_TYPE(char3, uchar3)
ABS_TEST_DIFF_TYPE(char4, uchar4)
ABS_TEST_DIFF_TYPE(char8, uchar8)
ABS_TEST_DIFF_TYPE(char16, uchar16)
ABS_TEST_DIFF_TYPE(uchar2, uchar2)
ABS_TEST_DIFF_TYPE(uchar3, uchar3)
ABS_TEST_DIFF_TYPE(uchar4, uchar4)
ABS_TEST_DIFF_TYPE(uchar8, uchar8)
ABS_TEST_DIFF_TYPE(uchar16, uchar16)


typedef cl_vec<short, 2> short2;
typedef cl_vec<short, 3> short3;
typedef cl_vec<short, 4> short4;
typedef cl_vec<short, 8> short8;
typedef cl_vec<short, 16> short16;
typedef cl_vec<unsigned short, 2> ushort2;
typedef cl_vec<unsigned short, 3> ushort3;
typedef cl_vec<unsigned short, 4> ushort4;
typedef cl_vec<unsigned short, 8> ushort8;
typedef cl_vec<unsigned short, 16> ushort16;
ABS_TEST_DIFF_TYPE(short2, ushort2)
ABS_TEST_DIFF_TYPE(short3, ushort3)
ABS_TEST_DIFF_TYPE(short4, ushort4)
ABS_TEST_DIFF_TYPE(short8, ushort8)
ABS_TEST_DIFF_TYPE(short16, ushort16)
ABS_TEST_DIFF_TYPE(ushort2, ushort2)
ABS_TEST_DIFF_TYPE(ushort3, ushort3)
ABS_TEST_DIFF_TYPE(ushort4, ushort4)
ABS_TEST_DIFF_TYPE(ushort8, ushort8)
ABS_TEST_DIFF_TYPE_END(ushort16, ushort16)
