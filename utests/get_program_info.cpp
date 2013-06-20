#include <string.h>
#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <algorithm>
#include "utest_helper.hpp"

using namespace std;

/* ********************************************** *
 * This file to test the API of:                  *
 * clGetProgramInfo                               *
 * ********************************************** */
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


#define CALL_PROGINFO_AND_RET(TYPE) \
    do { \
	cl_int ret; \
	size_t ret_size; \
	\
	Info_Result<TYPE>* info = cast_as<TYPE>(x.second); \
	ret = clGetProgramInfo(program, x.first, \
		info->size, info->get_ret(), &ret_size); \
	OCL_ASSERT((!ret)); \
	OCL_ASSERT((info->check_result())); \
	delete info; \
    } while(0)

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

    sprintf(ker_path, "%s/%s", kiss_path, "get_program_info.cl");

    ifstream in(ker_path);
    while (getline(in,line)) {
        source_code = (source_code == "") ?
                      source_code + line : source_code + "\n" + line;
    }
    free(ker_path);
    //cout<< source_code;
    source_code = source_code + "\n";

    expect_source = (char *)source_code.c_str();

    OCL_CREATE_KERNEL("get_program_info");

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

