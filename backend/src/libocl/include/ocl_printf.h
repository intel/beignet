/*
 * Copyright © 2012 - 2014 Intel Corporation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef __OCL_PRINTF_H__
#define __OCL_PRINTF_H__

#include "ocl_types.h"

/* The printf function. */
/* From LLVM 3.4, c string are all in constant address space */
#if 100*__clang_major__ + __clang_minor__ < 304
int __gen_ocl_printf_stub(const char * format, ...);
int __gen_ocl_puts_stub(const char * format);
#else
int __gen_ocl_printf_stub(constant char * format, ...);
int __gen_ocl_puts_stub(constant char * format);
#endif
#define printf __gen_ocl_printf_stub
#define puts __gen_ocl_puts_stub

#endif
