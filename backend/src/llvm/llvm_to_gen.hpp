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
 * \file llvm_to_gen.hpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */
#ifndef __GBE_IR_LLVM_TO_GEN_HPP__
#define __GBE_IR_LLVM_TO_GEN_HPP__

namespace gbe {
  namespace ir {
    // The code is output into an IR unit
    class Unit;
  } /* namespace ir */

  /*! Convert the LLVM IR code to a GEN IR code,
		  optLevel 0 equal to clang -O1 and 1 equal to clang -O2*/
  bool llvmToGen(ir::Unit &unit, const char *fileName, const void* module, int optLevel, bool strictMath);

} /* namespace gbe */

#endif /* __GBE_IR_LLVM_TO_GEN_HPP__ */

