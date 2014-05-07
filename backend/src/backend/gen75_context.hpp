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
 */

/**
 * \file gen75_context.hpp
 */
#ifndef __GBE_GEN75_CONTEXT_HPP__
#define __GBE_GEN75_CONTEXT_HPP__

#include "backend/gen_context.hpp"
#include "backend/gen75_encoder.hpp"

namespace gbe
{
  /* This class is used to implement the HSW
     specific logic for context. */
  class Gen75Context : public GenContext
  {
  public:
    Gen75Context(const ir::Unit &unit, const std::string &name, uint32_t deviceID, bool relaxMath = false)
            : GenContext(unit, name, deviceID, relaxMath) {
    };

  protected:
    virtual GenEncoder* generateEncoder(void) {
      return GBE_NEW(Gen75Encoder, this->simdWidth, 75, deviceID);
    }

  };
}
#endif /* __GBE_GEN75_CONTEXT_HPP__ */
