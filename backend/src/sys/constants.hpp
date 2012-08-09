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

#ifndef __PF_CONSTANTS_HPP__
#define __PF_CONSTANTS_HPP__

#ifndef NULL
#define NULL 0
#endif

#include <limits>

namespace pf
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
    INLINE operator int64 ( ) const { return 0; }
    INLINE operator uint64( ) const { return 0; }
    INLINE operator int32 ( ) const { return 0; }
    INLINE operator uint32( ) const { return 0; }
    INLINE operator int16 ( ) const { return 0; }
    INLINE operator uint16( ) const { return 0; }
    INLINE operator int8  ( ) const { return 0; }
    INLINE operator uint8 ( ) const { return 0; }
#ifndef __MSVC__
    INLINE operator size_t( ) const { return 0; }
#endif

  } zero MAYBE_UNUSED;

  static struct OneTy
  {
    INLINE operator double( ) const { return 1; }
    INLINE operator float ( ) const { return 1; }
    INLINE operator int64 ( ) const { return 1; }
    INLINE operator uint64( ) const { return 1; }
    INLINE operator int32 ( ) const { return 1; }
    INLINE operator uint32( ) const { return 1; }
    INLINE operator int16 ( ) const { return 1; }
    INLINE operator uint16( ) const { return 1; }
    INLINE operator int8  ( ) const { return 1; }
    INLINE operator uint8 ( ) const { return 1; }
#ifndef __MSVC__
    INLINE operator size_t( ) const { return 1; }
#endif
  } one MAYBE_UNUSED;

  static struct NegInfTy
  {
    INLINE operator double( ) const { return -std::numeric_limits<double>::infinity(); }
    INLINE operator float ( ) const { return -std::numeric_limits<float>::infinity(); }
    INLINE operator int64 ( ) const { return std::numeric_limits<int64>::min(); }
    INLINE operator uint64( ) const { return std::numeric_limits<uint64>::min(); }
    INLINE operator int32 ( ) const { return std::numeric_limits<int32>::min(); }
    INLINE operator uint32( ) const { return std::numeric_limits<uint32>::min(); }
    INLINE operator int16 ( ) const { return std::numeric_limits<int16>::min(); }
    INLINE operator uint16( ) const { return std::numeric_limits<uint16>::min(); }
    INLINE operator int8  ( ) const { return std::numeric_limits<int8>::min(); }
    INLINE operator uint8 ( ) const { return std::numeric_limits<uint8>::min(); }
#ifndef __MSVC__
    INLINE operator size_t( ) const { return std::numeric_limits<size_t>::min(); }
#endif

  } neg_inf MAYBE_UNUSED;

  static struct PosInfTy
  {
    INLINE operator double( ) const { return std::numeric_limits<double>::infinity(); }
    INLINE operator float ( ) const { return std::numeric_limits<float>::infinity(); }
    INLINE operator int64 ( ) const { return std::numeric_limits<int64>::max(); }
    INLINE operator uint64( ) const { return std::numeric_limits<uint64>::max(); }
    INLINE operator int32 ( ) const { return std::numeric_limits<int32>::max(); }
    INLINE operator uint32( ) const { return std::numeric_limits<uint32>::max(); }
    INLINE operator int16 ( ) const { return std::numeric_limits<int16>::max(); }
    INLINE operator uint16( ) const { return std::numeric_limits<uint16>::max(); }
    INLINE operator int8  ( ) const { return std::numeric_limits<int8>::max(); }
    INLINE operator uint8 ( ) const { return std::numeric_limits<uint8>::max(); }
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
