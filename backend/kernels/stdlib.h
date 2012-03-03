/* 
 * Copyright © 2012 Intel Corporation
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
 * Author: Benjamin Segovia <benjamin.segovia@intel.com>
 */

__attribute__((pure,const)) unsigned int __gen_ocl_get_global_id0(void);
__attribute__((pure,const)) unsigned int __gen_ocl_get_global_id1(void);
__attribute__((pure,const)) unsigned int __gen_ocl_get_global_id2(void);
__attribute__((pure,const)) unsigned int __gen_ocl_get_local_id0(void);
__attribute__((pure,const)) unsigned int __gen_ocl_get_local_id1(void);
__attribute__((pure,const)) unsigned int __gen_ocl_get_local_id2(void);

inline unsigned get_global_id(unsigned int dim) {
  if (dim == 0) return __gen_ocl_get_global_id0();
  else if (dim == 1) return __gen_ocl_get_global_id1();
  else if (dim == 2) return __gen_ocl_get_global_id2();
  else return 0;
}

inline unsigned get_local_id(unsigned int dim) {
  if (dim == 0) return __gen_ocl_get_local_id0();
  else if (dim == 1) return __gen_ocl_get_local_id1();
  else if (dim == 2) return __gen_ocl_get_local_id2();
  else return 0;
}

typedef float float2 __attribute__((ext_vector_type(2)));
typedef float float3 __attribute__((ext_vector_type(3)));
typedef float float4 __attribute__((ext_vector_type(4)));
typedef int int2 __attribute__((ext_vector_type(2)));
typedef int int3 __attribute__((ext_vector_type(3)));
typedef int int4 __attribute__((ext_vector_type(4)));

