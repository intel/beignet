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
 * \file half.cpp
 *
 */
#include "llvm/ADT/APSInt.h"
#include "half.hpp"

namespace gbe {
namespace ir {
  static llvm::APFloat convU16ToAPFloat(const uint16_t v)
  {
    uint64_t v64 = static_cast<uint64_t>(v);
    llvm::APInt apInt(16, v64, false);
    return llvm::APFloat(llvm::APFloat::IEEEhalf, apInt);
  }

  static uint16_t convAPFloatToU16(const llvm::APFloat& apf)
  {
    llvm::APInt api = apf.bitcastToAPInt();
    uint64_t v64 = api.getZExtValue();
    return static_cast<uint16_t>(v64);
  }

  half::operator float(void) const {
    bool loseInfo;
    llvm::APFloat apf_self = convU16ToAPFloat(this->val);
    apf_self.convert(llvm::APFloat::IEEEsingle, llvm::APFloat::rmNearestTiesToEven, &loseInfo);
    return apf_self.convertToFloat();
  }

  half::operator double(void) const {
    bool loseInfo;
    llvm::APFloat apf_self = convU16ToAPFloat(this->val);
    apf_self.convert(llvm::APFloat::IEEEdouble, llvm::APFloat::rmNearestTiesToEven, &loseInfo);
    return apf_self.convertToDouble();
  }

  half::operator uint16_t(void) const {
    llvm::APSInt apsInt(16, false);
    bool isExact;
    llvm::APFloat apf_self = convU16ToAPFloat(this->val);
    apf_self.convertToInteger(apsInt, llvm::APFloat::rmNearestTiesToEven, &isExact);
    return static_cast<uint16_t>(apsInt.getZExtValue());
  }

  half::operator int16_t(void) const {
    llvm::APSInt apsInt(16, true);
    bool isExact;
    llvm::APFloat apf_self = convU16ToAPFloat(this->val);
    apf_self.convertToInteger(apsInt, llvm::APFloat::rmNearestTiesToEven, &isExact);
    return static_cast<int16_t>(apsInt.getZExtValue());
  }

  half half::convToHalf(uint16_t u16) {
    llvm::APFloat res(llvm::APFloat::IEEEhalf, llvm::APInt(16, 0, false));
    uint64_t u64 = static_cast<uint64_t>(u16);
    llvm::APInt apInt(16, u64, false);
    res.convertFromAPInt(apInt, false, llvm::APFloat::rmNearestTiesToEven);
    return half(convAPFloatToU16(res));
  }

  half half::convToHalf(int16_t v16) {
    llvm::APFloat res(llvm::APFloat::IEEEhalf, llvm::APInt(16, 0, true));
    uint64_t u64 = static_cast<uint64_t>(v16);
    llvm::APInt apInt(16, u64, true);
    res.convertFromAPInt(apInt, true, llvm::APFloat::rmNearestTiesToEven);
    return half(convAPFloatToU16(res));
  }

  half half::operator +(const half& other) const
  {
    llvm::APFloat apf_self = convU16ToAPFloat(this->val);
    llvm::APFloat apf_other = convU16ToAPFloat(other.val);
    apf_self.add(apf_other, llvm::APFloat::rmNearestTiesToEven);
    uint16_t ret = convAPFloatToU16(apf_self);
    return half(ret);
  }

  half half::operator -(const half& other) const
  {
    llvm::APFloat apf_self = convU16ToAPFloat(this->val);
    llvm::APFloat apf_other = convU16ToAPFloat(other.val);
    apf_self.subtract(apf_other, llvm::APFloat::rmNearestTiesToEven);
    uint16_t ret = convAPFloatToU16(apf_self);
    return half(ret);
  }

  half half::operator *(const half& other) const
  {
    llvm::APFloat apf_self = convU16ToAPFloat(this->val);
    llvm::APFloat apf_other = convU16ToAPFloat(other.val);
    apf_self.multiply(apf_other, llvm::APFloat::rmNearestTiesToEven);
    uint16_t ret = convAPFloatToU16(apf_self);
    return half(ret);
  }

  half half::operator /(const half& other) const
  {
    llvm::APFloat apf_self = convU16ToAPFloat(this->val);
    llvm::APFloat apf_other = convU16ToAPFloat(other.val);
    apf_self.divide(apf_other, llvm::APFloat::rmNearestTiesToEven);
    uint16_t ret = convAPFloatToU16(apf_self);
    return half(ret);
  }

  half half::operator %(const half& other) const
  {
    llvm::APFloat apf_self = convU16ToAPFloat(this->val);
    llvm::APFloat apf_other = convU16ToAPFloat(other.val);
    apf_self.remainder(apf_other);
    uint16_t ret = convAPFloatToU16(apf_self);
    return half(ret);
  }

  bool half::operator ==(const half& other) const
  {
    llvm::APFloat apf_self = convU16ToAPFloat(this->val);
    llvm::APFloat apf_other = convU16ToAPFloat(other.val);
    llvm::APFloat::cmpResult res = apf_self.compare(apf_other);
    if (res == llvm::APFloat::cmpEqual)
      return true;

    return false;
  }

  bool half::operator !=(const half& other) const
  {
    llvm::APFloat apf_self = convU16ToAPFloat(this->val);
    llvm::APFloat apf_other = convU16ToAPFloat(other.val);
    llvm::APFloat::cmpResult res = apf_self.compare(apf_other);
    if (res == llvm::APFloat::cmpEqual)
      return false;

    return true;
  }

  bool half::operator <(const half& other) const
  {
    llvm::APFloat apf_self = convU16ToAPFloat(this->val);
    llvm::APFloat apf_other = convU16ToAPFloat(other.val);
    llvm::APFloat::cmpResult res = apf_self.compare(apf_other);
    if (res == llvm::APFloat::cmpLessThan)
      return true;

    return false;
  }

  bool half::operator >(const half& other) const
  {
    llvm::APFloat apf_self = convU16ToAPFloat(this->val);
    llvm::APFloat apf_other = convU16ToAPFloat(other.val);
    llvm::APFloat::cmpResult res = apf_self.compare(apf_other);
    if (res == llvm::APFloat::cmpGreaterThan)
      return true;

    return false;
  }

  bool half::operator <=(const half& other) const
  {
    llvm::APFloat apf_self = convU16ToAPFloat(this->val);
    llvm::APFloat apf_other = convU16ToAPFloat(other.val);
    llvm::APFloat::cmpResult res = apf_self.compare(apf_other);
    if (res == llvm::APFloat::cmpLessThan || res == llvm::APFloat::cmpEqual)
      return true;

    return false;
  }

  bool half::operator >=(const half& other) const
  {
    llvm::APFloat apf_self = convU16ToAPFloat(this->val);
    llvm::APFloat apf_other = convU16ToAPFloat(other.val);
    llvm::APFloat::cmpResult res = apf_self.compare(apf_other);
    if (res == llvm::APFloat::cmpGreaterThan || res == llvm::APFloat::cmpEqual)
      return true;

    return false;
  }

  bool half::operator &&(const half& other) const
  {
    llvm::APFloat apf_self = convU16ToAPFloat(this->val);
    llvm::APFloat apf_other = convU16ToAPFloat(other.val);
    if (apf_self.isZero() || apf_other.isZero())
      return false;

    return true;
  }

  bool half::operator ||(const half& other) const
  {
    llvm::APFloat apf_self = convU16ToAPFloat(this->val);
    llvm::APFloat apf_other = convU16ToAPFloat(other.val);
    if (apf_self.isZero() && apf_other.isZero())
      return false;

    return true;
  }

} /* namespace ir */
} /* namespace gbe */
