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
 * \file register.cpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */
#include "ir/register.hpp"
#include "sys/string.hpp"

namespace gbe {
namespace ir {

  std::ostream &operator<< (std::ostream &out, const RegisterData &regData)
  {
    switch (regData.family) {
      case RegisterData::BOOL: return out << "bool";
      case RegisterData::BYTE: return out << "byte";
      case RegisterData::WORD: return out << "word";
      case RegisterData::DWORD: return out << "dword";
      case RegisterData::QWORD: return out << "qword";
    };
    return out;
  }

  std::ostream &operator<< (std::ostream &out, const RegisterFile &file)
  {
    out << "## " << file.regNum() << " register"
        << plural(file.regNum()) << " ##" << std::endl;
    for (uint32_t i = 0; i < file.regNum(); ++i) {
      const RegisterData reg = file.get(Register(i));
      out << ".decl." << reg << " %" << i << std::endl;
    }
    return out;
  }

} /* namespace ir */
} /* namespace gbe */

