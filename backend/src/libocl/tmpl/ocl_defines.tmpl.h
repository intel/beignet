#ifndef __OCL_COMMON_DEF_H__
#define __OCL_COMMON_DEF_H__

#define __OPENCL_VERSION__ 110
#define __CL_VERSION_1_0__ 100
#define __CL_VERSION_1_1__ 110
#define __ENDIAN_LITTLE__ 1
#define __IMAGE_SUPPORT__ 1
#define __kernel_exec(X, TYPE) __kernel __attribute__((work_group_size_hint(X,1,1))) \
                                        __attribute__((vec_type_hint(TYPE)))
#define kernel_exec(X, TYPE) __kernel_exec(X, TYPE)
#define cl_khr_global_int32_base_atomics
#define cl_khr_global_int32_extended_atomics
#define cl_khr_local_int32_base_atomics
#define cl_khr_local_int32_extended_atomics
#define cl_khr_byte_addressable_store
#define cl_khr_icd
#define cl_khr_gl_sharing

#endif /* end of __OCL_COMMON_DEF_H__ */
