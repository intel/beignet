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

/**
 * \file assert.hpp
 *
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */

#ifndef __UTEST_FILE_MAP_HPP__
#define __UTEST_FILE_MAP_HPP__

#include "CL/cl.h"
#include <cstdlib>

/* Map a file into memory for direct / cached / simple accesses */
typedef struct cl_file_map {
  void *start, *stop; /* First character and last one */
  size_t size;        /* Total size of the file */
  int fd;             /* Posix file descriptor */
  cl_bool mapped;     /* Indicate if a file was mapped or not */
  char *name;         /* File itself */
} cl_file_map_t;

/* Report information about an open temptative */
enum {
  CL_FILE_MAP_SUCCESS         = 0,
  CL_FILE_MAP_FILE_NOT_FOUND  = 1,
  CL_FILE_MAP_FAILED_TO_MMAP  = 2
};

/* Allocate and Initialize a file mapper (but do not map any file */
extern cl_file_map_t *cl_file_map_new(void);

/* Initialize a file mapper (but do not map any file */
extern int cl_file_map_init(cl_file_map_t *fm);

/* Destroy but do not deallocate a file map */
extern void cl_file_map_destroy(cl_file_map_t *fm);

/* Destroy and free it */
extern void cl_file_map_delete(cl_file_map_t *fm);

/* Open a file and returns the error code */
extern int cl_file_map_open(cl_file_map_t *fm, const char *name);

static inline cl_bool
cl_file_map_is_mapped(const cl_file_map_t *fm) {
  return fm->mapped;
}

static inline const char*
cl_file_map_begin(const cl_file_map_t *fm) {
  return (const char*) fm->start;
}

static inline const char*
cl_file_map_end(const cl_file_map_t *fm) {
  return (const char*) fm->stop;
}

static inline size_t
cl_file_map_size(const cl_file_map_t *fm) {
  return fm->size;
}

#endif /* __UTEST_FILE_MAP_HPP__ */

