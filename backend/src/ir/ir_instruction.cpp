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

#include "ir_instruction.hpp"
#include "ir_function.hpp"

namespace gbe
{
  namespace internal
  {
    /*! All unary and binary arithmetic instructions */
    template <uint32 srcNum> // 1 or 2
    class ALIGNED(AlignOf<Instruction>::value) NaryInstruction
    {
    public:
      INLINE uint32 getOpcode(void) const { return opcode; }
      INLINE uint32 getSrcNum(void) const { return srcNum; }
      INLINE uint32 getDstNum(void) const { return 1; }
      INLINE uint32 getDstIndex(const Function &fn, uint32 ID) const {
        assert(ID == 0);
        return dst;
      }
      INLINE uint32 getSrcIndex(const Function &fn, uint32 ID) const {
        assert(ID <= srcNum);
        return src[ID];
      }
      INLINE Register getDst(const Function &fn, uint32 ID) const {
        return fn.getRegister(this->getDstIndex(fn, ID));
      }
      INLINE Register getSrc(const Function &fn, uint32 ID) const {
        return fn.getRegister(this->getSrcIndex(fn, ID));
      }
    protected:
      uint8 opcode;       //!< Instruction opcode
      uint8 type;         //!< Type of the instruction
      uint16 dst;         //!< Index of the register in the register file
      uint16 src[srcNum]; //!< Indices of the sources
    };

    /*! All 1-source arithmetic instructions */
    class UnaryInstruction : public NaryInstruction<1>
    {
    public:
      UnaryInstruction(uint32 opcode, uint32 type, uint32 dst, uint32 src) {
        this->opcode = opcode;
        this->type = type;
        this->dst = dst;
        this->src[0] = src;
      }
    };

    /*! All 2-source arithmetic instructions */
    class BinaryInstruction : public NaryInstruction<2>
    {
    public:
      BinaryInstruction(uint32 opcode, uint32 type, uint32 dst, uint32 src0, uint32 src1) {
        this->opcode = opcode;
        this->type = type;
        this->dst = dst;
        this->src[0] = src0;
        this->src[1] = src1;
      }
    };

    STATIC_ASSERT(sizeof(UnaryInstruction) <= sizeof(Instruction));
    STATIC_ASSERT(sizeof(BinaryInstruction) <= sizeof(Instruction));

  } /* namespace internal */
} /* namespace gbe */

