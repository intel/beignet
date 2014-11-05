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
 * \file gen8_context.hpp
 */
#ifndef __GBE_GEN8_CONTEXT_HPP__
#define __GBE_GEN8_CONTEXT_HPP__

#include "backend/gen_context.hpp"
#include "backend/gen8_encoder.hpp"

namespace gbe
{
  /* This class is used to implement the HSW
     specific logic for context. */
  class Gen8Context : public GenContext
  {
  public:
    virtual ~Gen8Context(void) { }
    Gen8Context(const ir::Unit &unit, const std::string &name, uint32_t deviceID, bool relaxMath = false)
            : GenContext(unit, name, deviceID, relaxMath) {
    };
    /*! device's max srcatch buffer size */
    #define GEN8_SCRATCH_SIZE  (2 * KB * KB)
    /*! Align the scratch size to the device's scratch unit size */
    virtual uint32_t alignScratchSize(uint32_t size);
    /*! Get the device's max srcatch size */
    virtual uint32_t getScratchSize(void) {
      //Because the allocate is use uint16_t, so clamp it, need refine
      return std::min(GEN8_SCRATCH_SIZE, 0x7fff);
    }
    /*! Get the pointer argument size for curbe alloc */
    virtual uint32_t getPointerSize(void) { return 8; }

  protected:
    virtual GenEncoder* generateEncoder(void) {
      return GBE_NEW(Gen8Encoder, this->simdWidth, 8, deviceID);
    }

  private:
    virtual void emitSLMOffset(void);
    virtual void allocSLMOffsetCurbe(void);
    virtual void newSelection(void);
  };
}
#endif /* __GBE_GEN8_CONTEXT_HPP__ */
