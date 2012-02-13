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

#ifndef __GBE_CONSTANTS_HPP__
#define __GBE_CONSTANTS_HPP__

#ifndef NULL
#define NULL 0
#endif

#include <limits>

namespace gbe
{
  static struct NullTy {
  } null MAYBE_UNUSED;

  static struct TrueTy {
    INLINE operator bool( ) const { return true; }
  } True MAYBE_UNUSED;

  static struct FalseTy {
    INLINE operator bool( ) const { return false; }
  } False MAYBE_UNUSED;

  static struct ZeroTy
  {
    INLINE operator double( ) const { return 0; }
    INLINE operator float ( ) const { return 0; }
    INLINE operator int64_t ( ) const { return 0; }
    INLINE operator uint64_t( ) const { return 0; }
    INLINE operator int32_t ( ) const { return 0; }
    INLINE operator uint32_t( ) const { return 0; }
    INLINE operator int16_t ( ) const { return 0; }
    INLINE operator uint16_t( ) const { return 0; }
    INLINE operator int8_t  ( ) const { return 0; }
    INLINE operator uint8_t ( ) const { return 0; }
#ifndef __MSVC__
    INLINE operator size_t( ) const { return 0; }
#endif

  } zero MAYBE_UNUSED;

  static struct OneTy
  {
    INLINE operator double( ) const { return 1; }
    INLINE operator float ( ) const { return 1; }
    INLINE operator int64_t ( ) const { return 1; }
    INLINE operator uint64_t( ) const { return 1; }
    INLINE operator int32_t ( ) const { return 1; }
    INLINE operator uint32_t( ) const { return 1; }
    INLINE operator int16_t ( ) const { return 1; }
    INLINE operator uint16_t( ) const { return 1; }
    INLINE operator int8_t  ( ) const { return 1; }
    INLINE operator uint8_t ( ) const { return 1; }
#ifndef __MSVC__
    INLINE operator size_t( ) const { return 1; }
#endif
  } one MAYBE_UNUSED;

  static struct NegInfTy
  {
    INLINE operator double( ) const { return -std::numeric_limits<double>::infinity(); }
    INLINE operator float ( ) const { return -std::numeric_limits<float>::infinity(); }
    INLINE operator int64_t ( ) const { return std::numeric_limits<int64_t>::min(); }
    INLINE operator uint64_t( ) const { return std::numeric_limits<uint64_t>::min(); }
    INLINE operator int32_t ( ) const { return std::numeric_limits<int32_t>::min(); }
    INLINE operator uint32_t( ) const { return std::numeric_limits<uint32_t>::min(); }
    INLINE operator int16_t ( ) const { return std::numeric_limits<int16_t>::min(); }
    INLINE operator uint16_t( ) const { return std::numeric_limits<uint16_t>::min(); }
    INLINE operator int8_t  ( ) const { return std::numeric_limits<int8_t>::min(); }
    INLINE operator uint8_t ( ) const { return std::numeric_limits<uint8_t>::min(); }
#ifndef __MSVC__
    INLINE operator size_t( ) const { return std::numeric_limits<size_t>::min(); }
#endif

  } neg_inf MAYBE_UNUSED;

  static struct PosInfTy
  {
    INLINE operator double( ) const { return std::numeric_limits<double>::infinity(); }
    INLINE operator float ( ) const { return std::numeric_limits<float>::infinity(); }
    INLINE operator int64_t ( ) const { return std::numeric_limits<int64_t>::max(); }
    INLINE operator uint64_t( ) const { return std::numeric_limits<uint64_t>::max(); }
    INLINE operator int32_t ( ) const { return std::numeric_limits<int32_t>::max(); }
    INLINE operator uint32_t( ) const { return std::numeric_limits<uint32_t>::max(); }
    INLINE operator int16_t ( ) const { return std::numeric_limits<int16_t>::max(); }
    INLINE operator uint16_t( ) const { return std::numeric_limits<uint16_t>::max(); }
    INLINE operator int8_t  ( ) const { return std::numeric_limits<int8_t>::max(); }
    INLINE operator uint8_t ( ) const { return std::numeric_limits<uint8_t>::max(); }
#ifndef _WIN32
    INLINE operator size_t( ) const { return std::numeric_limits<size_t>::max(); }
#endif
  } inf MAYBE_UNUSED, pos_inf MAYBE_UNUSED;

  static struct NaNTy
  {
    INLINE operator double( ) const { return std::numeric_limits<double>::quiet_NaN(); }
    INLINE operator float ( ) const { return std::numeric_limits<float>::quiet_NaN(); }
  } nan MAYBE_UNUSED;

  static struct UlpTy
  {
    INLINE operator double( ) const { return std::numeric_limits<double>::epsilon(); }
    INLINE operator float ( ) const { return std::numeric_limits<float>::epsilon(); }
  } ulp MAYBE_UNUSED;

  static struct PiTy
  {
    INLINE operator double( ) const { return 3.14159265358979323846; }
    INLINE operator float ( ) const { return 3.14159265358979323846f; }
  } pi MAYBE_UNUSED;

  static struct StepTy {
  } step MAYBE_UNUSED;

  static struct EmptyTy {
  } empty MAYBE_UNUSED;

  static struct FullTy {
  } full MAYBE_UNUSED;

  static const size_t KB = 1024u;
  static const size_t MB = KB*KB;
  static const size_t GB = KB*MB;
}

#endif
