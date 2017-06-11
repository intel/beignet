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
 * Author: He Junyan <junyan.he@intel.com>
 */
#ifndef __CL_COMPILER_H__
#define __CL_COMPILER_H__

#include "cl_utils.h"
#include "CL/cl.h"

typedef struct _cl_compiler {
  void *opaque;
  cl_bool available;
  char *compiler_name;

  cl_bool (*check_compiler_option)(const char *option);
  cl_bool (*build_program)(cl_uint device_id, const char *source, size_t src_length,
                           const char *options, size_t err_buf_size, char *err,
                           size_t *err_ret_size, char **binary, size_t *binary_size);
  cl_bool (*compile_program)(cl_uint device_id, const char *source, size_t src_length, const char **headers,
                             size_t *header_lengths, const char **header_names, int header_num,
                             const char *options, size_t err_buf_size, char *err, size_t *err_ret_size,
                             char **binary, size_t *binary_size);
  cl_bool (*link_program)(cl_uint device_id, int binary_num, char **binaries, size_t *bin_sizes,
                          const char *options, size_t err_buf_size, char *err, size_t *err_ret_size,
                          char **ret_binary, size_t *ret_binary_size);
} _cl_compiler;
typedef _cl_compiler *cl_compiler;

extern cl_int cl_compiler_check_available(cl_device_id device);
extern cl_int cl_compiler_unload(cl_device_id device);

#endif /* End of __CL_COMPILER_H__ */
