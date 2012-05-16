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
 * \file lowering.hpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 *  Lower instructions that are not supported properly. Typical example is
 *  handling returns or unsupported vector scatters / gathers
 */

namespace gbe {
namespace ir {

  // Structure to update
  class Unit;

  /*! Remove all return instructions and replace them to forward branches that
   *  point to the only return instruction in a dedicated basic block and the end
   *  of the function
   */
  void lowerReturn(Unit &unit, const std::string &functionName);

} /* namespace ir */
} /* namespace gbe */

