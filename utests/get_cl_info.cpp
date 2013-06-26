#include <string.h>
#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <algorithm>
#include "utest_helper.hpp"

using namespace std;

/* ***************************************************** *
 * This file to test all the API like: clGetXXXXInfo     *
 * ***************************************************** */
#define NO_STANDARD_REF 0xFFFFF

template <typename T = cl_uint>
struct Info_Result {
    T ret;
    T refer;
    int size;
    typedef T type_value;

    void * get_ret(void) {
        return (void *)&ret;
    }

    Info_Result(T other) {
        refer = other;
        size = sizeof(T);
    }

    bool check_result (void) {
        if (ret != refer && refer != (T)NO_STANDARD_REF)
            return false;

        return true;
    }
};

template <>
struct Info_Result<char *> {
    char * ret;
    char * refer;
    int size;
    typedef char* type_value;

    Info_Result(char *other, int sz) {
        size = sz;
        ret = (char *)malloc(sizeof(char) * sz);
        if (other) {
            refer = (char *)malloc(sizeof(char) * sz);
            memcpy(refer, other, sz);
        }
    }

    ~Info_Result(void) {
        free(refer);
        free(ret);
    }

    void * get_ret(void) {
        return (void *)ret;
    }

    bool check_result (void) {
        if (refer && ::memcmp(ret, refer, size))
            return false;

        return true;
    }
};

template <> //Used for such as CL_PROGRAM_BINARIES
struct Info_Result<char **> {
    char ** ret;
    char ** refer;
    int *elt_size;
    int size;
    typedef char** type_value;

    Info_Result(char **other, int *sz, int elt_num) {
        size = elt_num;

        ret = (char **)malloc(elt_num * sizeof(char *));
        memset(ret, 0, (elt_num * sizeof(char *)));
        refer = (char **)malloc(elt_num * sizeof(char *));
        memset(refer, 0, (elt_num * sizeof(char *)));
        elt_size = (int *)malloc(elt_num * sizeof(int));
        memset(elt_size, 0, (elt_num * sizeof(int)));
        if (sz) {
            int i = 0;
            for (; i < elt_num; i++) {
                elt_size[i] = sz[i];
                ret[i] = (char *)malloc(sz[i] * sizeof(char));

                if (other[i] && elt_size[i] > 0) {
                    refer[i] = (char *)malloc(sz[i] * sizeof(char));
                    memcpy(&refer[i], &other[i], sz[i]);
                }
                else
                    refer[i] = NULL;
            }
        }
    }

    ~Info_Result(void) {
        int i = 0;
        for (; i < size; i++) {
            if (refer[i])
                free(refer[i]);
            free(ret[i]);
        }
        free(ret);
        free(refer);
        free(elt_size);
    }

    void * get_ret(void) {
        return (void *)ret;
    }

    bool check_result (void) {
        int i = 0;
        for (; i < size; i++) {
            if (refer[i] && ::memcmp(ret[i], refer[i], elt_size[i]))
                return false;
        }

        return true;
    }
};

template <typename T1, typename T2>
struct Traits {
    static bool Is_Same(void) {
        return false;
    };
};

template <typename T1>
struct Traits<T1, T1> {
    static bool Is_Same(void) {
        return true;
    };
};

template <typename T>
Info_Result<T>* cast_as(void *info)
{
    Info_Result<T>* ret;
    ret = reinterpret_cast<Info_Result<T>*>(info);
    OCL_ASSERT((Traits<T, typename Info_Result<T>::type_value>::Is_Same()));
    return ret;
}


#define CALL_INFO_AND_RET(TYPE, FUNC, ...) \
    do { \
	cl_int ret; \
	size_t ret_size; \
	\
	Info_Result<TYPE>* info = cast_as<TYPE>(x.second); \
	ret = FUNC (__VA_ARGS__, x.first, \
		info->size, info->get_ret(), &ret_size); \
	OCL_ASSERT((!ret)); \
	OCL_ASSERT((info->check_result())); \
	delete info; \
    } while(0)

/* ***************************************************** *
 * clGetProgramInfo                                      *
 * ***************************************************** */
#define CALL_PROGINFO_AND_RET(TYPE) CALL_INFO_AND_RET(TYPE, clGetProgramInfo, program)

void get_program_info(void)
{
    map<cl_program_info, void *> maps;
    int expect_value;
    char * expect_source;
    int sz;
    char *ker_path = (char *)malloc(4096 * sizeof(char));
    const char *kiss_path = getenv("OCL_KERNEL_PATH");
    string line;
    string source_code;

    sprintf(ker_path, "%s/%s", kiss_path, "compiler_if_else.cl");

    ifstream in(ker_path);
    while (getline(in,line)) {
        source_code = (source_code == "") ?
                      source_code + line : source_code + "\n" + line;
    }
    free(ker_path);
    //cout<< source_code;
    source_code = source_code + "\n";

    expect_source = (char *)source_code.c_str();

    OCL_CREATE_KERNEL("compiler_if_else");

    /* First test for clGetProgramInfo. We just have 1 devices now */
    expect_value = 2;//One program, one kernel.
    maps.insert(make_pair(CL_PROGRAM_REFERENCE_COUNT,
                          (void *)(new Info_Result<>(((cl_uint)expect_value)))));
    maps.insert(make_pair(CL_PROGRAM_CONTEXT,
                          (void *)(new Info_Result<cl_context>(ctx))));
    expect_value = 1;
    maps.insert(make_pair(CL_PROGRAM_NUM_DEVICES,
                          (void *)(new Info_Result<>(((cl_uint)expect_value)))));
    maps.insert(make_pair(CL_PROGRAM_DEVICES,
                          (void *)(new Info_Result<cl_device_id>(device))));
    sz = (strlen(expect_source) + 1);
    maps.insert(make_pair(CL_PROGRAM_SOURCE,
                          (void *)(new Info_Result<char *>(expect_source, sz))));
    expect_value = NO_STANDARD_REF;
    maps.insert(make_pair(CL_PROGRAM_BINARY_SIZES,
                          (void *)(new Info_Result<size_t>((size_t)expect_value))));
    sz = 4096; //big enough?
    expect_source = NULL;
    maps.insert(make_pair(CL_PROGRAM_BINARIES,
                          (void *)(new Info_Result<char **>(&expect_source, &sz, 1))));

    std::for_each(maps.begin(), maps.end(), [](pair<cl_program_info, void *> x) {
        switch (x.first) {
        case CL_PROGRAM_REFERENCE_COUNT:
        case CL_PROGRAM_NUM_DEVICES:
            CALL_PROGINFO_AND_RET(cl_uint);
            break;
        case CL_PROGRAM_CONTEXT:
            CALL_PROGINFO_AND_RET(cl_context);
            break;
        case CL_PROGRAM_DEVICES:
            CALL_PROGINFO_AND_RET(cl_device_id);
            break;
        case CL_PROGRAM_SOURCE:
            CALL_PROGINFO_AND_RET(char *);
            break;
        case CL_PROGRAM_BINARY_SIZES:
            CALL_PROGINFO_AND_RET(size_t);
            break;
        case CL_PROGRAM_BINARIES:
            CALL_PROGINFO_AND_RET(char **);
            break;
        default:
            break;
        }
    });
}

MAKE_UTEST_FROM_FUNCTION(get_program_info);

/* ***************************************************** *
 * clGetCommandQueueInfo                                 *
 * ***************************************************** */
#define CALL_QUEUEINFO_AND_RET(TYPE) CALL_INFO_AND_RET(TYPE, clGetCommandQueueInfo, queue)

void get_queue_info(void)
{
    /* use the compiler_fabs case to test us. */
    const size_t n = 16;
    map<cl_program_info, void *> maps;
    int expect_ref;
    cl_command_queue_properties prop;

    OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(float), NULL);
    OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(float), NULL);
    OCL_CREATE_KERNEL("compiler_fabs");

    OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
    OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);

    globals[0] = 16;
    locals[0] = 16;

    OCL_MAP_BUFFER(0);
    for (int32_t i = 0; i < (int32_t) n; ++i)
        ((float*)buf_data[0])[i] = .1f * (rand() & 15) - .75f;
    OCL_UNMAP_BUFFER(0);

    // Run the kernel on GPU
    OCL_NDRANGE(1);

    /* Do our test.*/
    maps.insert(make_pair(CL_QUEUE_CONTEXT,
                          (void *)(new Info_Result<cl_context>(ctx))));
    maps.insert(make_pair(CL_QUEUE_DEVICE,
                          (void *)(new Info_Result<cl_device_id>(device))));

    expect_ref = 1;
    maps.insert(make_pair(CL_QUEUE_REFERENCE_COUNT,
                          (void *)(new Info_Result<>(((cl_uint)expect_ref)))));

    prop = 0;
    maps.insert(make_pair(CL_QUEUE_PROPERTIES,
                          (void *)(new Info_Result<cl_command_queue_properties>(
                                       ((cl_command_queue_properties)prop)))));

    std::for_each(maps.begin(), maps.end(), [](pair<cl_program_info, void *> x) {
        switch (x.first) {
        case CL_QUEUE_CONTEXT:
            CALL_QUEUEINFO_AND_RET(cl_context);
            break;
        case CL_QUEUE_DEVICE:
            CALL_QUEUEINFO_AND_RET(cl_device_id);
            break;
        case CL_QUEUE_REFERENCE_COUNT:
            CALL_QUEUEINFO_AND_RET(cl_uint);
            break;
        case CL_QUEUE_PROPERTIES:
            CALL_QUEUEINFO_AND_RET(cl_command_queue_properties);
            break;
        default:
            break;
        }
    });
}

MAKE_UTEST_FROM_FUNCTION(get_queue_info);

/* ***************************************************** *
 * clGetProgramBuildInfo                                 *
 * ***************************************************** */
#define CALL_PROG_BUILD_INFO_AND_RET(TYPE)  CALL_INFO_AND_RET(TYPE, \
             clGetProgramBuildInfo, program, device)

void get_program_build_info(void)
{
    map<cl_program_info, void *> maps;
    cl_build_status expect_status;
    char build_opt[] = "-emit-llvm";
    char log[] = "";
    int sz;

    OCL_CALL (cl_kernel_init, "compiler_if_else.cl", "compiler_if_else", SOURCE, build_opt);

    /* Do our test.*/
    expect_status = CL_BUILD_SUCCESS;
    maps.insert(make_pair(CL_PROGRAM_BUILD_STATUS,
                          (void *)(new Info_Result<cl_build_status>(expect_status))));
    sz = strlen(build_opt) + 1;
    maps.insert(make_pair(CL_PROGRAM_BUILD_OPTIONS,
                          (void *)(new Info_Result<char *>(build_opt, sz))));
    sz = strlen(log) + 1;
    maps.insert(make_pair(CL_PROGRAM_BUILD_LOG, /* not supported now, just "" */
                          (void *)(new Info_Result<char *>(log, sz))));

    std::for_each(maps.begin(), maps.end(), [](pair<cl_program_info, void *> x) {
        switch (x.first) {
        case CL_PROGRAM_BUILD_STATUS:
            CALL_PROG_BUILD_INFO_AND_RET(cl_build_status);
            break;
        case CL_PROGRAM_BUILD_OPTIONS:
            CALL_PROG_BUILD_INFO_AND_RET(char *);
            break;
        case CL_PROGRAM_BUILD_LOG:
            CALL_PROG_BUILD_INFO_AND_RET(char *);
            break;
        default:
            break;
        }
    });
}

MAKE_UTEST_FROM_FUNCTION(get_program_build_info);
