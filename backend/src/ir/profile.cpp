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

/**
 * \file profile.hpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */
#include "ir/profile.hpp"
#include "ir/function.hpp"
#include "sys/platform.hpp"

namespace gbe {
namespace ir {

  namespace ocl
  {
    const char *specialRegMean[] = {
        "local_id_0", "local_id_1", "local_id_2",
        "group_id_0", "group_id_1", "group_id_2",
        "num_groups_0", "num_groups_1", "num_groups_2",
        "local_size_0", "local_size_1", "local_size_2",
        "global_size_0", "global_size_1", "global_size_2",
        "global_offset_0", "global_offset_1", "global_offset_2",
        "stack_pointer", "stack_buffer",
        "block_ip",
        "barrier_id", "thread_number", "work_dimension",
        "zero", "one",
        "retVal", "slm_offset",
        "printf_buffer_pointer", "printf_index_buffer_pointer",
        "invalid"
    };

#if GBE_DEBUG
#define DECL_NEW_REG(FAMILY, REG, UNIFORM) \
   r = fn.newRegister(FAMILY_DWORD, UNIFORM); \
   GBE_ASSERT(r == REG);
#else
#define DECL_NEW_REG(FAMILY, REG, UNIFORM) \
   fn.newRegister(FAMILY_DWORD, UNIFORM);
#endif /* GBE_DEBUG */
    static void init(Function &fn) {
      IF_DEBUG(Register r);
      DECL_NEW_REG(FAMILY_DWORD, lid0, 0);
      DECL_NEW_REG(FAMILY_DWORD, lid1, 0);
      DECL_NEW_REG(FAMILY_DWORD, lid2, 0);
      DECL_NEW_REG(FAMILY_DWORD, groupid0, 1);
      DECL_NEW_REG(FAMILY_DWORD, groupid1, 1);
      DECL_NEW_REG(FAMILY_DWORD, groupid2, 1);
      DECL_NEW_REG(FAMILY_DWORD, numgroup0, 1);
      DECL_NEW_REG(FAMILY_DWORD, numgroup1, 1);
      DECL_NEW_REG(FAMILY_DWORD, numgroup2, 1);
      DECL_NEW_REG(FAMILY_DWORD, lsize0, 1);
      DECL_NEW_REG(FAMILY_DWORD, lsize1, 1);
      DECL_NEW_REG(FAMILY_DWORD, lsize2, 1);
      DECL_NEW_REG(FAMILY_DWORD, gsize0, 1);
      DECL_NEW_REG(FAMILY_DWORD, gsize1, 1);
      DECL_NEW_REG(FAMILY_DWORD, gsize2, 1);
      DECL_NEW_REG(FAMILY_DWORD, goffset0, 1);
      DECL_NEW_REG(FAMILY_DWORD, goffset1, 1);
      DECL_NEW_REG(FAMILY_DWORD, goffset2, 1);
      DECL_NEW_REG(FAMILY_DWORD, stackptr, 0);
      DECL_NEW_REG(FAMILY_DWORD, stackbuffer, 1);
      DECL_NEW_REG(FAMILY_WORD,  blockip, 0);
      DECL_NEW_REG(FAMILY_DWORD, barrierid, 1);
      DECL_NEW_REG(FAMILY_DWORD, threadn, 1);
      DECL_NEW_REG(FAMILY_DWORD, workdim, 1);
      DECL_NEW_REG(FAMILY_DWORD, zero, 1);
      DECL_NEW_REG(FAMILY_DWORD, one, 1);
      DECL_NEW_REG(FAMILY_WORD, retVal, 1);
      DECL_NEW_REG(FAMILY_WORD, slmoffset, 1);
      DECL_NEW_REG(FAMILY_DWORD, printfbptr, 1);
      DECL_NEW_REG(FAMILY_DWORD, printfiptr, 1);
      DECL_NEW_REG(FAMILY_DWORD, invalid, 1);
    }
#undef DECL_NEW_REG

  } /* namespace ocl */

  void initProfile(Function &fn) {
    const Profile profile = fn.getProfile();
    switch (profile) {
      case PROFILE_C: GBE_ASSERTM(false, "Unsupported profile"); break;
      case PROFILE_OCL: ocl::init(fn);
    };
  }

} /* namespace ir */
} /* namespace gbe */


