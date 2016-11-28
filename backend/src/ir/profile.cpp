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
        "enqueued_local_size_0", "enqueued_local_size_1", "enqueued_local_size_2",
        "global_size_0", "global_size_1", "global_size_2",
        "global_offset_0", "global_offset_1", "global_offset_2",
        "stack_pointer", "stack_buffer",
        "block_ip",
        "barrier_id", "thread_number", "work_dimension",
        "zero", "one",
        "retVal",
        "dwblockip",
        "profiling_buffer_pointer",
        "profiling_timestamps0", "profiling_timestamps1",
        "profiling_timestamps2", "profiling_timestamps3",
        "profiling_timestamps4",
        "threadid",
        "constant_addrspace_start",
        "stack_size", "enqueue_buffer_pointer",
    };

#if GBE_DEBUG
#define DECL_NEW_REG(FAMILY, REG, ...) \
   r = fn.newRegister(FAMILY, __VA_ARGS__); \
   GBE_ASSERT(r == REG);
#else
#define DECL_NEW_REG(FAMILY, REG, ...) \
   fn.newRegister(FAMILY, __VA_ARGS__);
#endif /* GBE_DEBUG */
    static void init(Function &fn) {
      IF_DEBUG(Register r);
      DECL_NEW_REG(FAMILY_DWORD, lid0, 0, GBE_CURBE_LOCAL_ID_X);
      DECL_NEW_REG(FAMILY_DWORD, lid1, 0, GBE_CURBE_LOCAL_ID_Y);
      DECL_NEW_REG(FAMILY_DWORD, lid2, 0, GBE_CURBE_LOCAL_ID_Z);
      DECL_NEW_REG(FAMILY_DWORD, groupid0, 1);
      DECL_NEW_REG(FAMILY_DWORD, groupid1, 1);
      DECL_NEW_REG(FAMILY_DWORD, groupid2, 1);
      DECL_NEW_REG(FAMILY_DWORD, numgroup0, 1, GBE_CURBE_GROUP_NUM_X);
      DECL_NEW_REG(FAMILY_DWORD, numgroup1, 1, GBE_CURBE_GROUP_NUM_Y);
      DECL_NEW_REG(FAMILY_DWORD, numgroup2, 1, GBE_CURBE_GROUP_NUM_Z);
      DECL_NEW_REG(FAMILY_DWORD, lsize0, 1, GBE_CURBE_LOCAL_SIZE_X);
      DECL_NEW_REG(FAMILY_DWORD, lsize1, 1, GBE_CURBE_LOCAL_SIZE_Y);
      DECL_NEW_REG(FAMILY_DWORD, lsize2, 1, GBE_CURBE_LOCAL_SIZE_Z);
      DECL_NEW_REG(FAMILY_DWORD, enqlsize0, 1, GBE_CURBE_ENQUEUED_LOCAL_SIZE_X);
      DECL_NEW_REG(FAMILY_DWORD, enqlsize1, 1, GBE_CURBE_ENQUEUED_LOCAL_SIZE_Y);
      DECL_NEW_REG(FAMILY_DWORD, enqlsize2, 1, GBE_CURBE_ENQUEUED_LOCAL_SIZE_Z);
      DECL_NEW_REG(FAMILY_DWORD, gsize0, 1, GBE_CURBE_GLOBAL_SIZE_X);
      DECL_NEW_REG(FAMILY_DWORD, gsize1, 1, GBE_CURBE_GLOBAL_SIZE_Y);
      DECL_NEW_REG(FAMILY_DWORD, gsize2, 1, GBE_CURBE_GLOBAL_SIZE_Z);
      DECL_NEW_REG(FAMILY_DWORD, goffset0, 1, GBE_CURBE_GLOBAL_OFFSET_X);
      DECL_NEW_REG(FAMILY_DWORD, goffset1, 1, GBE_CURBE_GLOBAL_OFFSET_Y);
      DECL_NEW_REG(FAMILY_DWORD, goffset2, 1, GBE_CURBE_GLOBAL_OFFSET_Z);
      if(fn.getOclVersion() >= 200) {
        DECL_NEW_REG(FAMILY_QWORD, stackptr, 0);
      } else {
        DECL_NEW_REG(FAMILY_DWORD, stackptr, 0);
      }
      DECL_NEW_REG(FAMILY_QWORD, stackbuffer, 1, GBE_CURBE_EXTRA_ARGUMENT, GBE_STACK_BUFFER);
      DECL_NEW_REG(FAMILY_WORD,  blockip, 0, GBE_CURBE_BLOCK_IP);
      DECL_NEW_REG(FAMILY_DWORD, barrierid, 1);
      DECL_NEW_REG(FAMILY_DWORD, threadn, 1, GBE_CURBE_THREAD_NUM);
      DECL_NEW_REG(FAMILY_DWORD, workdim, 1, GBE_CURBE_WORK_DIM);
      DECL_NEW_REG(FAMILY_DWORD, zero, 1);
      DECL_NEW_REG(FAMILY_DWORD, one, 1);
      DECL_NEW_REG(FAMILY_WORD, retVal, 1);
      DECL_NEW_REG(FAMILY_DWORD, dwblockip, 0, GBE_CURBE_DW_BLOCK_IP);
      DECL_NEW_REG(FAMILY_QWORD, profilingbptr, 1, GBE_CURBE_PROFILING_BUF_POINTER);
      DECL_NEW_REG(FAMILY_DWORD, profilingts0, 0, GBE_CURBE_PROFILING_TIMESTAMP0);
      DECL_NEW_REG(FAMILY_DWORD, profilingts1, 0, GBE_CURBE_PROFILING_TIMESTAMP1);
      DECL_NEW_REG(FAMILY_DWORD, profilingts2, 0, GBE_CURBE_PROFILING_TIMESTAMP2);
      DECL_NEW_REG(FAMILY_DWORD, profilingts3, 0, GBE_CURBE_PROFILING_TIMESTAMP3);
      DECL_NEW_REG(FAMILY_DWORD, profilingts4, 0, GBE_CURBE_PROFILING_TIMESTAMP4);
      DECL_NEW_REG(FAMILY_DWORD, threadid, 1, GBE_CURBE_THREAD_ID);
      DECL_NEW_REG(FAMILY_QWORD, constant_addrspace, 1, GBE_CURBE_CONSTANT_ADDRSPACE);
      DECL_NEW_REG(FAMILY_QWORD, stacksize, 1, GBE_CURBE_STACK_SIZE);
      DECL_NEW_REG(FAMILY_QWORD, enqueuebufptr, 1, GBE_CURBE_ENQUEUE_BUF_POINTER);
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


