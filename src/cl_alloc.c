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

#include "cl_alloc.h"
#include "cl_utils.h"

#include <stdlib.h>
#include <assert.h>
#include <malloc.h>

static volatile int32_t cl_alloc_n = 0;

LOCAL void*
cl_malloc(size_t sz)
{
  void * p = NULL;
  atomic_inc(&cl_alloc_n);
  p = malloc(sz);
  assert(p);
  return p;
}

LOCAL void*
cl_aligned_malloc(size_t sz, size_t align)
{
  void * p = NULL;
  atomic_inc(&cl_alloc_n);
  p = memalign(align, sz);
  assert(p);
  return p;
}

LOCAL void*
cl_calloc(size_t n, size_t elem_size)
{
  void *p = NULL;
  atomic_inc(&cl_alloc_n);
  p = calloc(n, elem_size);
  assert(p);
  return p;
}

LOCAL void*
cl_realloc(void *ptr, size_t sz)
{
  if (ptr == NULL)
    atomic_inc(&cl_alloc_n);
  return realloc(ptr, sz);
}

LOCAL void
cl_free(void *ptr)
{
  if (ptr == NULL)
    return;
  atomic_dec(&cl_alloc_n);
  free(ptr);
  ptr = NULL;
}

LOCAL size_t
cl_report_unfreed(void)
{
  return cl_alloc_n;
}

LOCAL void
cl_report_set_all_freed(void)
{
  cl_alloc_n = 0;
}

