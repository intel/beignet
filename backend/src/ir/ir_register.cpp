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

#include "ir_register.hpp"
#include "sys/map.hpp"
#include "sys/fixed_array.hpp"
#include "sys/vector.hpp"

namespace gbe
{
  const uint8 Register::familySize[] = {
    1, // bool
    1, // byte
    2, // word
    4, // dword
    5, // qword
  };

  /*! Implement register file class */
  class RegisterFileInternal : public RegisterFile
  {
    // Implements base class methods
    virtual ~RegisterFileInternal(void);
    virtual Register *create(Register::Family family, uint8 width);
    virtual Register *get(uint32 id);
    virtual void destroy(Register *reg);

    
  };

} /* namespace gbe */

