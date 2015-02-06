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
  /* This class is used to implement the HSW
     specific logic for context. */
  class Gen9Context : public Gen8Context
  {
  public:
    virtual ~Gen9Context(void) { };
    Gen9Context(const ir::Unit &unit, const std::string &name, uint32_t deviceID, bool relaxMath = false)
            : Gen8Context(unit, name, deviceID, relaxMath) {
    };

  protected:
    virtual GenEncoder* generateEncoder(void) {
      return GBE_NEW(Gen9Encoder, this->simdWidth, 9, deviceID);
    }

  private:
    virtual void newSelection(void);
  };
}
#endif /* __GBE_GEN9_CONTEXT_HPP__ */
