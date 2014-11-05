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
 * \file gen_insn_scheduling.hpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */

#ifndef __GBE_GEN_INSN_SCHEDULING_HPP__
#define __GBE_GEN_INSN_SCHEDULING_HPP__

namespace gbe
{
  class Selection;  // Pre ISA code
  class GenContext; // Handle compilation for Gen

  /*! Schedule the code per basic block (tends to limit register number) */
  void schedulePreRegAllocation(GenContext &ctx, Selection &selection);

  /*! Schedule the code per basic block (tends to deal with insn latency) */
  void schedulePostRegAllocation(GenContext &ctx, Selection &selection);

} /* namespace gbe */

#endif /* __GBE_GEN_INSN_SCHEDULING_HPP__ */

