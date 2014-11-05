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

#ifndef __GBE_ATOMIC_HPP__
#define __GBE_ATOMIC_HPP__

#include "sys/intrinsics.hpp"

namespace gbe
{
  template <typename T>
  struct AtomicInternal {
  protected:
    AtomicInternal(const AtomicInternal&); // don't implement
    AtomicInternal& operator= (const AtomicInternal&); // don't implement

  public:
    INLINE AtomicInternal(void) {}
    INLINE AtomicInternal(T data) : data(data) {}
    INLINE AtomicInternal& operator =(const T input) { data = input; return *this; }
    INLINE operator T() const { return data; }
    INLINE void storeRelease(T x) { __store_release(&data, x); }
  public:
    INLINE friend T operator+= (AtomicInternal& value, T input) { return atomic_add(&value.data, input) + input; }
    INLINE friend T operator++ (AtomicInternal& value) { return atomic_add(&value.data,  1) + 1; }
    INLINE friend T operator-- (AtomicInternal& value) { return atomic_add(&value.data, -1) - 1; }
    INLINE friend T operator++ (AtomicInternal& value, int) { return atomic_add(&value.data,  1); }
    INLINE friend T operator-- (AtomicInternal& value, int) { return atomic_add(&value.data, -1); }
    INLINE friend T cmpxchg    (AtomicInternal& value, const T v, const T c) { return atomic_cmpxchg(&value.data,v,c); }

  private:
    volatile T data;
    GBE_STRUCT(AtomicInternal);
  };

  typedef AtomicInternal<atomic32_t> Atomic32;
  typedef AtomicInternal<atomic_t> Atomic;
}

#endif /* __GBE_ATOMIC_HPP__ */

