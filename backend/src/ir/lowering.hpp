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
 * \file lowering.hpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 *  Lower instructions that are not supported properly. Typical example is
 *  handling returns or unsupported vector scatters / gathers
 */

#ifndef __GBE_IR_LOWERING_HPP__
#define __GBE_IR_LOWERING_HPP__

namespace gbe {
namespace ir {

  // Structure to update
  class Unit;

  /*! Remove all return instructions and replace them to forward branches that
   *  point to the only return instruction in a dedicated basic block and the end
   *  of the function.
   *  Typically this code:
   *
   *  dst[x] = 1;
   *  if (x > 4) return;
   *  dst[x] = 3;
   *
   *  will be replaced by:
   *
   *  dst[x] = 1;
   *  if (x > 4) goto end;
   *  dst[x] = 3;
   *  end:
   *  return;
   *
   *  There will be only one return at the end of the function. This return will
   *  be simply encoded as a End-of-thread instruction (EOT)
   */
  void lowerReturn(Unit &unit, const std::string &functionName);

  /*! Function arguments are a bit tricky since we must implement the proper C
   *  semantic: we can therefore address the function arguments as we want and
   *  we can even modify them. This leads to interesting challenges. We identify
   *  several cases:
   *
   *  case 1:
   *  int f (__global int *dst, int x[16], int y) {
   *    dst[get_global_id(0)] = x[16] + y;
   *  }
   *  Here x and y will be pushed to registers using the Curbe. No problem, we
   *  can directly used the pushed registers
   *
   *  case 2:
   *  int f (__global int *dst, int x[16], int y) {
   *    dst[get_global_id(0)] = x[get_local_id(0)] + y;
   *  }
   *  Here x is indirectly accessed. We need to perform a gather from memory. We
   *  can simply gather it from the curbe in memory
   *
   *  case 3:
   *  int f (__global int *dst, int x[16], int y) {
   *    x[get_local_id(0)] = y + 1;
   *    int *ptr = get_local_id(0) % 2 ? x[0] : x[1];
   *    dst[get_global_id(0)] = *ptr;
   *  }
   *  Here we modify the function argument since it is valid C. Problem is that
   *  we are running in SIMD mode while the data are scalar (in both memory and
   *  registers). In that case, we just spill everything to memory (using the
   *  stack) and reload it from here when needed.
   */
  void lowerFunctionArguments(Unit &unit, const std::string &functionName);

} /* namespace ir */
} /* namespace gbe */

#endif /* __GBE_IR_LOWERING_HPP__ */

