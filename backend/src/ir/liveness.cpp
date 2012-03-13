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
 * \file liveness.cpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */
#include <sys/map.hpp>
#include <sys/set.hpp>

namespace gbe
{
  /*! Compute liveness of each register */
  class LivenessInfo
  {
  public:
    LivenessInfo(Function &fn) : fn(fn) {}
    /*! Set of variables used upwards in the block (before a definition) */
    typedef set<Register> UsedVar;
    /*! Set of variables alive at the exit of the block */
    typedef set<Register> LiveOut;
    /*! Set of variables actually killed in each block */
    typedef set<Register> Kill;
    /*! Per-block info */
    struct BlockInfo {
      UEVar upwardUsed;
      LiveOut liveOut;
      Kill kill;
    };
    /*! Gives for each block the variables alive at entry / exit */
    typedef map<BasicBlock*, BlockInfo> BlockLiveness;
    /*! Compute the liveness for this function */
    Function &fn;
  };

  LivenessInfo::LivenessInfo(Function &fn) : fn(fn) {
    

  }

} /* namespace gbe */

