/*
 * Copyright © 2014 Intel Corporation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
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

#ifndef __CL_GBE_LOADER_H__
#define __CL_GBE_LOADER_H__

#include "program.h"

#ifdef __cplusplus
extern "C" {
#endif
extern gbe_program_new_from_source_cb *compiler_program_new_from_source;
extern gbe_program_serialize_to_binary_cb *compiler_program_serialize_to_binary;
extern gbe_program_new_from_llvm_cb *compiler_program_new_from_llvm;
extern gbe_kernel_set_const_buffer_size_cb *compiler_kernel_set_const_buffer_size;
extern gbe_set_image_base_index_cb *compiler_set_image_base_index;
extern gbe_set_image_base_index_cb *gbe_set_image_base_index_interp;
int CompilerSupported();
#ifdef __cplusplus
}
#endif

#endif /* __CL_GBE_LOADER_H__ */
