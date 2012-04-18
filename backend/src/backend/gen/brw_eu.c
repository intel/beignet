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
  

// #include "brw_context.h"
#include "brw_defines.h"
#include "brw_eu.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>

// #include "glsl/ralloc.h"

/* Returns the corresponding conditional mod for swapping src0 and
 * src1 in e.g. CMP.
 */
uint32_t
brw_swap_cmod(uint32_t cmod)
{
   switch (cmod) {
   case BRW_CONDITIONAL_Z:
   case BRW_CONDITIONAL_NZ:
      return cmod;
   case BRW_CONDITIONAL_G:
      return BRW_CONDITIONAL_LE;
   case BRW_CONDITIONAL_GE:
      return BRW_CONDITIONAL_L;
   case BRW_CONDITIONAL_L:
      return BRW_CONDITIONAL_GE;
   case BRW_CONDITIONAL_LE:
      return BRW_CONDITIONAL_G;
   default:
      return ~0;
   }
}


/* How does predicate control work when execution_size != 8?  Do I
 * need to test/set for 0xffff when execution_size is 16?
 */
void brw_set_predicate_control_flag_value(struct brw_compile *p, uint32_t value)
{
//   p->current->header.predicate_control = BRW_PREDICATE_NONE;

   if (value != 0xff) {
      if (value != p->flag_value) {
         brw_push_insn_state(p);
         brw_MOV(p, brw_flag_reg(), brw_imm_uw(value));
         p->flag_value = value;
         brw_pop_insn_state(p);
      }

//      p->current->header.predicate_control = BRW_PREDICATE_NORMAL;
   }
}

void brw_set_predicate_control(struct brw_compile *p, uint32_t pc)
{
  // p->current->header.predicate_control = pc;
}

void brw_set_predicate_inverse(struct brw_compile *p, bool predicate_inverse)
{
  // p->current->header.predicate_inverse = predicate_inverse;
}

void brw_set_conditionalmod(struct brw_compile *p, uint32_t conditional)
{
 //  p->current->header.destreg__conditionalmod = conditional;
}

void brw_set_access_mode(struct brw_compile *p, uint32_t access_mode)
{
  // p->current->header.access_mode = access_mode;
}

void
brw_set_compression_control(struct brw_compile *p,
                            enum brw_compression compression_control)
{
#if 0
   p->compressed = (compression_control == BRW_COMPRESSION_COMPRESSED);

   if (p->gen >= 6) {
      /* Since we don't use the 32-wide support in gen6, we translate
       * the pre-gen6 compression control here.
       */
      switch (compression_control) {
      case BRW_COMPRESSION_NONE:
         /* This is the "use the first set of bits of dmask/vmask/arf
          * according to execsize" option.
          */
         p->current->header.compression_control = GEN6_COMPRESSION_1Q;
         break;
      case BRW_COMPRESSION_2NDHALF:
         /* For 8-wide, this is "use the second set of 8 bits." */
         p->current->header.compression_control = GEN6_COMPRESSION_2Q;
         break;
      case BRW_COMPRESSION_COMPRESSED:
         /* For 16-wide instruction compression, use the first set of 16 bits
          * since we don't do 32-wide dispatch.
          */
         p->current->header.compression_control = GEN6_COMPRESSION_1H;
         break;
      default:
         assert(!"not reached");
         p->current->header.compression_control = GEN6_COMPRESSION_1H;
         break;
      }
   } else {
      p->current->header.compression_control = compression_control;
   }
#endif
}

