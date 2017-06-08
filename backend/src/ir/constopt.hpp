/*
 * Copyright © 2017 Intel Corporation
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
 * Author: Guo Yejun <yejun.guo@intel.com>
 */

#ifndef __GBE_IR_CONSTOPT_HPP__
#define __GBE_IR_CONSTOPT_HPP__

namespace gbe {
namespace ir {

  // Structure to update
  class Unit;

  // TODO
  void foldConstant(Unit &unit, const std::string &functionName);
  void propagateConstant(Unit &unit, const std::string &functionName);

  // for the following GEN IR, %41 is kernel argument (struct)
  // the first LOAD will be mov, and the second LOAD will be indirect move
  // (see lowerFunctionArguments). It hurts performance,
  // and even impacts the correctness of reg liveness of indriect mov
  //
  // LOADI.uint64 %1114 72
  // ADD.int64 %78 %41 %1114
  // LOAD.int64.private.aligned {%79} %78 bti:255
  // LOADI.int64 %1115 8
  // ADD.int64 %1116 %78 %1115
  // LOAD.int64.private.aligned {%80} %1116 bti:255
  //
  // this function folds the constants of 72 and 8 together,
  // and so it will be direct mov.
  // the GEN IR looks like:
  // LOADI.int64 %1115 80
  // ADD.int64 %1116 %41 %1115
  void foldFunctionStructArgConstOffset(Unit &unit, const std::string &functionName);
} /* namespace ir */
} /* namespace gbe */

#endif /* __GBE_IR_LOWERING_HPP__ */
