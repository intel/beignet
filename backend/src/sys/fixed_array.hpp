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
 * \file fixed_array.hpp
 *
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */

#ifndef __GBE_FIXED_ARRAY_HPP__
#define __GBE_FIXED_ARRAY_HPP__

#include "platform.hpp"
#include <cstring>

namespace gbe
{
  /*! Regular C array but with bound checks */
  template<typename T, size_t N>
  class fixed_array
  {
  public:
    /*! Do not initialize the data */
    fixed_array(void) {}
    /*! Copy the input array */
    fixed_array(const T array[N]) { std::memcpy(elem, array, N * sizeof(T)); }
    /*! First element (non const) */
    T* begin(void) { return &elem[0]; }
    /*! First non-valid element (non const) */
    T* end(void) { return begin() + N; }
    /*! First element (const) */
    const T* begin(void) const { return &elem[0]; }
    /*! First non-valid element (const) */
    const T* end(void) const { return begin() + N; }
    /*! Number of elements in the array */
    size_t size(void) const { return N; }
    /*! Get the pointer to the data (non-const) */
    T* data(void) { return &elem[0]; }
    /*! Get the pointer to the data (const) */
    const T* data(void) const { return &elem[0]; }
    /*! First element (const) */
    const T& front(void) const { return *begin(); }
    /*! Last element (const) */
    const T& back(void) const { return *(end() - 1); }
    /*! First element (non-const) */
    T& front(void) { return *begin(); }
    /*! Last element (non-const) */
    T& back(void) { return *(end() - 1); }
    /*! Get element at position index (with bound check) */
    INLINE T& operator[] (size_t index) {
      GBE_ASSERT(index < size());
      return elem[index];
    }
    /*! Get element at position index (with bound check) */
    INLINE const T& operator[] (size_t index) const {
      GBE_ASSERT(index < size());
      return elem[index];
    }
  private:
    T elem[N];            //!< Store the elements
    STATIC_ASSERT(N > 0); //!< zero element is not allowed
    GBE_CLASS(fixed_array);
  };

} /* namespace gbe */

#endif /* __GBE_FIXED_ARRAY_HPP__ */

