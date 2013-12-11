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

    void abs(void) {
        int i = 0;
        for (; i < N; i++) {
            T f = ptr[i];
            f = f < 0 ? -f : f;
            ptr[i] = f;
        }
    }
};

template <typename T, typename U, int N> static void cpu (int global_id,
        cl_vec<T, N> *src, cl_vec<U, N> *dst)
{
    cl_vec<T, N> v  = src[global_id];
    v.abs();
    dst[global_id] = v;
}

template <typename T, typename U> static void cpu(int global_id, T *src, U *dst)
{
    T f = src[global_id];
    f = f < 0 ? -f : f;
    dst[global_id] = (U)f;
}

template <typename T, int N> static void gen_rand_val (cl_vec<T, N>& vect)
{
    int i = 0;

    memset(vect.ptr, 0, sizeof(T) * ((N+1)/2)*2);
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

template <typename T, typename U, int N> static void dump_data (cl_vec<T, N>* src,
        cl_vec<U, N>* dst, int n)
{
    U* val = reinterpret_cast<U *>(dst);

    n = n*((N+1)/2)*2;

    printf("\nRaw: \n");
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

template <typename T, typename U> static void dump_data (T* src, U* dst, int n)
{
    printf("\nRaw: \n");
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

template <typename T, typename U> static void compiler_abs_with_type(void)
{
    const size_t n = 16;
    U cpu_dst[16];
    T cpu_src[16];

    // Setup buffers
    OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(T), NULL);
    OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(T), NULL);
    OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
    OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
    globals[0] = 16;
    locals[0] = 16;

    // Run random tests
    for (uint32_t pass = 0; pass < 8; ++pass) {
        OCL_MAP_BUFFER(0);

        /* Clear the dst buffer to avoid random data. */
        OCL_MAP_BUFFER(1);
        memset(buf_data[1], 0, sizeof(U) * n);
        OCL_UNMAP_BUFFER(1);

        for (int32_t i = 0; i < (int32_t) n; ++i) {
            gen_rand_val(cpu_src[i]);
        }

        memcpy(buf_data[0], cpu_src, sizeof(T) * n);

        // Run the kernel on GPU
        OCL_NDRANGE(1);

        // Run on CPU
        for (int32_t i = 0; i < (int32_t) n; ++i)
            cpu(i, cpu_src, cpu_dst);

        // Compare
        OCL_MAP_BUFFER(1);

//      dump_data(cpu_src, cpu_dst, n);

        OCL_ASSERT(!memcmp(buf_data[1], cpu_dst, sizeof(T) * n));
        OCL_UNMAP_BUFFER(1);
        OCL_UNMAP_BUFFER(0);
    }
}

#define ABS_TEST_TYPE_1(TYPE, UTYPE, KEEP_PROGRAM) \
	static void compiler_abs_##TYPE (void) \
        { \
           OCL_CALL (cl_kernel_init, "compiler_abs.cl", "compiler_abs_"#TYPE, SOURCE, NULL);  \
           compiler_abs_with_type<TYPE, UTYPE>(); \
        } \
	MAKE_UTEST_FROM_FUNCTION_KEEP_PROGRAM(compiler_abs_##TYPE, KEEP_PROGRAM);

#define ABS_TEST_TYPE(TYPE, UTYPE) ABS_TEST_TYPE_1(TYPE, UTYPE, true)
#define ABS_TEST_TYPE_END(TYPE, UTYPE) ABS_TEST_TYPE_1(TYPE, UTYPE, false)

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
ABS_TEST_TYPE(int, uint)
ABS_TEST_TYPE(short, ushort)
ABS_TEST_TYPE(char, uchar)
ABS_TEST_TYPE(uint, uint)
ABS_TEST_TYPE(ushort, ushort)
ABS_TEST_TYPE(uchar, uchar)


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
ABS_TEST_TYPE(int2, uint2)
ABS_TEST_TYPE(int3, uint3)
ABS_TEST_TYPE(int4, uint4)
ABS_TEST_TYPE(int8, uint8)
ABS_TEST_TYPE(int16, uint16)
ABS_TEST_TYPE(uint2, uint2)
ABS_TEST_TYPE(uint3, uint3)
ABS_TEST_TYPE(uint4, uint4)
ABS_TEST_TYPE(uint8, uint8)
ABS_TEST_TYPE(uint16, uint16)


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
ABS_TEST_TYPE(char2, uchar2)
ABS_TEST_TYPE(char3, uchar3)
ABS_TEST_TYPE(char4, uchar4)
ABS_TEST_TYPE(char8, uchar8)
ABS_TEST_TYPE(char16, uchar16)
ABS_TEST_TYPE(uchar2, uchar2)
ABS_TEST_TYPE(uchar3, uchar3)
ABS_TEST_TYPE(uchar4, uchar4)
ABS_TEST_TYPE(uchar8, uchar8)
ABS_TEST_TYPE(uchar16, uchar16)


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
ABS_TEST_TYPE(short2, ushort2)
ABS_TEST_TYPE(short3, ushort3)
ABS_TEST_TYPE(short4, ushort4)
ABS_TEST_TYPE(short8, ushort8)
ABS_TEST_TYPE(short16, ushort16)
ABS_TEST_TYPE(ushort2, ushort2)
ABS_TEST_TYPE(ushort3, ushort3)
ABS_TEST_TYPE(ushort4, ushort4)
ABS_TEST_TYPE(ushort8, ushort8)
ABS_TEST_TYPE_END(ushort16, ushort16)
