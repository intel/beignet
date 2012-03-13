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
    static void init(Function &fn) {
      IF_DEBUG(Register r);
      IF_DEBUG(r =) fn.newRegister(FAMILY_DWORD);
      GBE_ASSERT(r == lid0);
      IF_DEBUG(r =) fn.newRegister(FAMILY_DWORD);
      GBE_ASSERT(r == lid1);
      IF_DEBUG(r =) fn.newRegister(FAMILY_DWORD);
      GBE_ASSERT(r == lid2);
      IF_DEBUG(r =) fn.newRegister(FAMILY_DWORD);
      GBE_ASSERT(r == gid0);
      IF_DEBUG(r =) fn.newRegister(FAMILY_DWORD);
      GBE_ASSERT(r == gid1);
      IF_DEBUG(r =) fn.newRegister(FAMILY_DWORD);
      GBE_ASSERT(r == gid2);
    }
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


