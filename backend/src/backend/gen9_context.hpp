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
 */

/**
 * \file gen9_context.hpp
 */
#ifndef __GBE_gen9_CONTEXT_HPP__
#define __GBE_gen9_CONTEXT_HPP__

#include "backend/gen8_context.hpp"
#include "backend/gen9_encoder.hpp"

namespace gbe
{
  /* This class is used to implement the skylake
     specific logic for context. */
  class Gen9Context : public Gen8Context
  {
  public:
    virtual ~Gen9Context(void) { };
    Gen9Context(const ir::Unit &unit, const std::string &name, uint32_t deviceID, bool relaxMath = false)
            : Gen8Context(unit, name, deviceID, relaxMath) {
    };
    virtual void emitBarrierInstruction(const SelectionInstruction &insn);

  protected:
    virtual GenEncoder* generateEncoder(void) {
      return GBE_NEW(Gen9Encoder, this->simdWidth, 9, deviceID);
    }

  private:
    virtual void newSelection(void);
  };

  //most code of BxtContext are copied from ChvContext, it results in two physical copy of the same code.
  //there are two possible ways to resolve it: 1) virtual inheritance  2) class template
  //but either way makes BxtContext and ChvContext tied closely, it might impact the flexibility of future changes
  //so, choose the method of two physical copies.
  class BxtContext : public Gen9Context
  {
  public:
    virtual ~BxtContext(void) { }
    BxtContext(const ir::Unit &unit, const std::string &name, uint32_t deviceID, bool relaxMath = false)
            : Gen9Context(unit, name, deviceID, relaxMath) {
    };
    virtual void emitI64MULInstruction(const SelectionInstruction &insn);

  protected:
    virtual void setA0Content(uint16_t new_a0[16], uint16_t max_offset = 0, int sz = 0);

  private:
    virtual void newSelection(void);
    virtual void calculateFullU64MUL(GenRegister src0, GenRegister src1, GenRegister dst_h,
                                           GenRegister dst_l, GenRegister s0l_s1h, GenRegister s0h_s1l);
    virtual void emitStackPointer(void);
  };
  /* This class is used to implement the kabylake
     specific logic for context. */
  class KblContext : public Gen9Context
  {
    public:
      virtual ~KblContext(void) { };
      KblContext(const ir::Unit &unit, const std::string &name, uint32_t deviceID, bool relaxMath = false)
        : Gen9Context(unit, name, deviceID, relaxMath) {
        };

    private:
      virtual void newSelection(void);
  };

  /* This class is used to implement the geminilake
     specific logic for context. */
  class GlkContext : public BxtContext
  {
    public:
      virtual ~GlkContext(void) { };
      GlkContext(const ir::Unit &unit, const std::string &name, uint32_t deviceID, bool relaxMath = false)
        : BxtContext(unit, name, deviceID, relaxMath) {
        };

    private:
      virtual void newSelection(void);
  };
}
#endif /* __GBE_GEN9_CONTEXT_HPP__ */
