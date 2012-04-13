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
 * \file llvm_gen_backend.hpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 *
 * Pass generation functions
 */
#ifndef __GBE_LLVM_GEN_BACKEND_HPP__
#define __GBE_LLVM_GEN_BACKEND_HPP__

#include "llvm/Pass.h"
#include "sys/platform.hpp"

// LLVM Type
namespace llvm { class Type; }

namespace gbe
{
  // Final target of the Gen backend
  namespace ir { class Unit; }

  /*! Pad the offset */
  uint32_t getPadding(uint32_t offset, uint32_t align);

  /*! Get the type alignment in bytes */
  uint32_t getAlignmentByte(const ir::Unit &unit, llvm::Type* Ty);

  /*! Get the type size in bits */
  uint32_t getTypeBitSize(const ir::Unit &unit, llvm::Type* Ty);

  /*! Get the type size in bytes */
  uint32_t getTypeByteSize(const ir::Unit &unit, llvm::Type* Ty);

  /*! Create a Gen-IR unit */
  llvm::FunctionPass *createGenPass(ir::Unit &unit);

  /*! Remove the GEP instructions */
  llvm::BasicBlockPass *createRemoveGEPPass(const ir::Unit &unit);

} /* namespace gbe */

#endif /* __GBE_LLVM_GEN_BACKEND_HPP__ */

