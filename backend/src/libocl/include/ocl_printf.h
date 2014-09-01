#ifndef __OCL_PRINTF_H__
#define __OCL_PRINTF_H__

#include "ocl_types.h"

/* The printf function. */
/* From LLVM 3.4, c string are all in constant address space */
#if 100*__clang_major__ + __clang_minor__ < 304
int __gen_ocl_printf_stub(const char * format, ...);
#else
int __gen_ocl_printf_stub(constant char * format, ...);
#endif
#define printf __gen_ocl_printf_stub

#endif
