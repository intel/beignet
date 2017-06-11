/*
 * Copyright © 2012 Intel Corporation
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
 * Author: Benjamin Segovia <benjamin.segovia@intel.com>
 */

#ifndef __CL_ALLOC_H__
#define __CL_ALLOC_H__

#include <stdlib.h>
#include <malloc.h>

//#define CL_ALLOC_DEBUG 1
#ifdef CL_ALLOC_DEBUG

/* Register some ptr allocated by other part */
extern void cl_register_alloc_ptr(void *ptr, size_t sz, char *file, int line);
#define CL_REGISTER_ALLOC_PTR(PTR, SZ) cl_register_alloc_ptr(PTR, SZ, __FILE__, __LINE__)

/* Return a valid pointer for the requested memory block size */
extern void *cl_malloc(size_t sz, char *file, int line);
#define CL_MALLOC(SZ) cl_malloc(SZ, __FILE__, __LINE__)

/* Aligned malloc */
extern void *cl_memalign(size_t align, size_t sz, char *file, int line);
#define CL_MEMALIGN(ALIGN, SZ) cl_memalign(ALIGN, SZ, __FILE__, __LINE__)

/* malloc + memzero */
extern void *cl_calloc(size_t n, size_t elem_size, char *file, int line);
#define CL_CALLOC(N, ELEM_SIZE) cl_calloc(N, ELEM_SIZE, __FILE__, __LINE__)

/* Regular realloc */
extern void *cl_realloc(void *ptr, size_t sz, char *file, int line);
#define CL_REALLOC(PTR, SZ) cl_realloc(PTR, SZ, __FILE__, __LINE__)

/* Free a pointer allocated with cl_*alloc */
extern void cl_free(void *ptr, char *file, int line);
#define CL_FREE(PTR) cl_free(PTR, __FILE__, __LINE__)

/* We count the number of allocation. This function report the number of
 * allocation still unfreed
 */
extern void cl_alloc_report_unfreed(void);
#define CL_ALLOC_REPORT_UNFREED() cl_alloc_report_unfreed()

extern void cl_alloc_debug_init(void);
#define CL_ALLOC_DEBUG_INIT() cl_alloc_debug_init()

#else
#define CL_REGISTER_ALLOC_PTR(PTR, SZ)
#define CL_MALLOC(SZ) malloc(SZ)
#define CL_MEMALIGN(ALIGN, SZ) memalign(ALIGN, SZ)
#define CL_CALLOC(N, ELEM_SIZE) calloc(N, ELEM_SIZE)
#define CL_REALLOC(PTR, SZ) realloc(PTR, SZ)
#define CL_FREE(PTR) free(PTR)
#define CL_ALLOC_REPORT_UNFREED()
#define CL_ALLOC_DEBUG_INIT()
#endif /* end of CL_ALLOC_DEBUG */
#endif /* __CL_ALLOC_H__ */
