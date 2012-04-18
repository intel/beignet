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
    

#include "main/mtypes.h"
#include "main/imports.h"
#include "brw_eu.h"

void brw_print_reg( struct brw_reg hwreg )
{
   static const char *file[] = {
      "arf",
      "grf",
      "msg",
      "imm"
   };

   static const char *type[] = {
      "ud",
      "d",
      "uw",
      "w",
      "ub",
      "vf",
      "hf",
      "f"
   };

   printf("%s%s", 
	  hwreg.abs ? "abs/" : "",
	  hwreg.negate ? "-" : "");
     
   if (hwreg.file == BRW_GENERAL_REGISTER_FILE &&
       hwreg.nr % 2 == 0 &&
       hwreg.subnr == 0 &&
       hwreg.vstride == BRW_VERTICAL_STRIDE_8 &&
       hwreg.width == BRW_WIDTH_8 &&
       hwreg.hstride == BRW_HORIZONTAL_STRIDE_1 &&
       hwreg.type == BRW_REGISTER_TYPE_F) {
      /* vector register */
      printf("vec%d", hwreg.nr);
   }
   else if (hwreg.file == BRW_GENERAL_REGISTER_FILE &&
	    hwreg.vstride == BRW_VERTICAL_STRIDE_0 &&
	    hwreg.width == BRW_WIDTH_1 &&
	    hwreg.hstride == BRW_HORIZONTAL_STRIDE_0 &&
	    hwreg.type == BRW_REGISTER_TYPE_F) {      
      /* "scalar" register */
      printf("scl%d.%d", hwreg.nr, hwreg.subnr / 4);
   }
   else if (hwreg.file == BRW_IMMEDIATE_VALUE) {
      printf("imm %f", hwreg.dw1.f);
   }
   else {
      printf("%s%d.%d<%d;%d,%d>:%s", 
		   file[hwreg.file],
		   hwreg.nr,
		   hwreg.subnr / type_sz(hwreg.type),
		   hwreg.vstride ? (1<<(hwreg.vstride-1)) : 0,
		   1<<hwreg.width,
		   hwreg.hstride ? (1<<(hwreg.hstride-1)) : 0,		
		   type[hwreg.type]);
   }
}



