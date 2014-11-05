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
 * \file register.cpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */
#include "ir/profile.hpp"
#include "ir/register.hpp"

namespace gbe {
namespace ir {

  std::ostream &operator<< (std::ostream &out, const RegisterData &regData)
  {
    switch (regData.family) {
      case FAMILY_BOOL: return out << "bool";
      case FAMILY_BYTE: return out << "byte";
      case FAMILY_WORD: return out << "word";
      case FAMILY_DWORD: return out << "dword";
      case FAMILY_QWORD: return out << "qword";
    };
    return out;
  }

  std::ostream &operator<< (std::ostream &out, const RegisterFile &file)
  {
    out << "## " << file.regNum() << " register"
        << (file.regNum() ? "s" : "") << " ##" << std::endl;
    for (uint32_t i = 0; i < file.regNum(); ++i) {
      const RegisterData reg = file.get(Register(i));
      out << ".decl." << reg << " %" << i;
      if (i < ocl::regNum)
        out << " " << ocl::specialRegMean[i];
      out << std::endl;
    }
    return out;
  }

  Tuple RegisterFile::appendArrayTuple(const Register *reg, uint32_t regNum) {
    const Tuple index = Tuple(regTuples.size());
    for (uint32_t regID = 0; regID < regNum; ++regID) {
      GBE_ASSERTM(reg[regID] < this->regNum(), "Out-of-bound register");
      regTuples.push_back(reg[regID]);
    }
    return index;
  }

} /* namespace ir */
} /* namespace gbe */

