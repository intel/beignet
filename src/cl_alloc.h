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

#include "cl_internals.h"
#include <stdlib.h>

/* Return a valid pointer for the requested memory block size */
extern void *cl_malloc(size_t sz);

/* Aligned malloc */
extern void* cl_aligned_malloc(size_t sz, size_t align);

/* malloc + memzero */
extern void *cl_calloc(size_t n, size_t elem_size);

/* Regular realloc */
extern void *cl_realloc(void *ptr, size_t sz);

/* Free a pointer allocated with cl_*alloc */
extern void  cl_free(void *ptr);

/* We count the number of allocation. This function report the number of
 * allocation still unfreed
 */
extern size_t cl_report_unfreed(void);

#endif /* __CL_ALLOC_H__ */

