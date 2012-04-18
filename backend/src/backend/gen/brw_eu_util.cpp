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

 /*
  * Authors:
  *   Keith Whitwell <keith@tungstengraphics.com>
  */
      

#include "brw_context.h"
#include "brw_defines.h"
#include "brw_eu.h"


void brw_math_invert( struct brw_compile *p, 
			     struct brw_reg dst,
			     struct brw_reg src)
{
   brw_math( p, 
	     dst,
	     BRW_MATH_FUNCTION_INV, 
	     BRW_MATH_SATURATE_NONE,
	     0,
	     src,
	     BRW_MATH_PRECISION_FULL, 
	     BRW_MATH_DATA_VECTOR );
}



void brw_copy4(struct brw_compile *p,
	       struct brw_reg dst,
	       struct brw_reg src,
	       GLuint count)
{
   GLuint i;

   dst = vec4(dst);
   src = vec4(src);

   for (i = 0; i < count; i++)
   {
      GLuint delta = i*32;
      brw_MOV(p, byte_offset(dst, delta),    byte_offset(src, delta));
      brw_MOV(p, byte_offset(dst, delta+16), byte_offset(src, delta+16));
   }
}


void brw_copy8(struct brw_compile *p,
	       struct brw_reg dst,
	       struct brw_reg src,
	       GLuint count)
{
   GLuint i;

   dst = vec8(dst);
   src = vec8(src);

   for (i = 0; i < count; i++)
   {
      GLuint delta = i*32;
      brw_MOV(p, byte_offset(dst, delta),    byte_offset(src, delta));
   }
}


void brw_copy_indirect_to_indirect(struct brw_compile *p,
				   struct brw_indirect dst_ptr,
				   struct brw_indirect src_ptr,
				   GLuint count)
{
   GLuint i;

   for (i = 0; i < count; i++)
   {
      GLuint delta = i*32;
      brw_MOV(p, deref_4f(dst_ptr, delta),    deref_4f(src_ptr, delta));
      brw_MOV(p, deref_4f(dst_ptr, delta+16), deref_4f(src_ptr, delta+16));
   }
}


void brw_copy_from_indirect(struct brw_compile *p,
			    struct brw_reg dst,
			    struct brw_indirect ptr,
			    GLuint count)
{
   GLuint i;

   dst = vec4(dst);

   for (i = 0; i < count; i++)
   {
      GLuint delta = i*32;
      brw_MOV(p, byte_offset(dst, delta),    deref_4f(ptr, delta));
      brw_MOV(p, byte_offset(dst, delta+16), deref_4f(ptr, delta+16));
   }
}




