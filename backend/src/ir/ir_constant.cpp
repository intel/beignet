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

#include "ir_constant.hpp"

namespace gbe
{

  void ConstantSet::append(const char *data,
                           const std::string &name,
                           uint32_t size,
                           uint32_t alignment)
  {
    const uint32_t offset = ALIGN(this->data.size(), alignment);
    const uint32_t padding = offset - this->data.size();
    const Constant constant(name, size, alignment, offset);
    constants.push_back(constant);
    for (uint32_t i = 0; i < padding; ++i) this->data.push_back(0);
    for (uint32_t i = 0; i < size; ++i) this->data.push_back(data[i]);
  }

} /* namespace gbe */
