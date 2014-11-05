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
 * \file vector.hpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */

#ifndef __GBE_VECTOR_HPP__
#define __GBE_VECTOR_HPP__

#include "sys/platform.hpp"
#include <vector>

namespace gbe
{
  /*! Add bound checks to the standard vector class and use the internal
   *  allocator
   */
  template<class T>
  class vector : public std::vector<T, Allocator<T>>
  {
  public:
    // Typedefs
    typedef std::vector<T, Allocator<T>>       parent_type;
    typedef Allocator<T>                       allocator_type;
    typedef typename allocator_type::size_type size_type;
    typedef typename parent_type::iterator     iterator;

    /*! Default constructor */
    INLINE explicit vector(const allocator_type &a = allocator_type()) :
      parent_type(a) {}
#if 0
    /*! Copy constructor */
    INLINE vector(const vector &x) : parent_type(x) {}
#endif
    /*! Repetitive sequence constructor */
    INLINE explicit vector(size_type n,
                           const T& value= T(),
                           const allocator_type &a = allocator_type()) :
      parent_type(n, value, a) {}
    /*! Iteration constructor */
    template <class InputIterator>
    INLINE vector(InputIterator first,
                  InputIterator last,
                  const allocator_type &a = allocator_type()) :
      parent_type(first, last, a) {}
    /*! Get element at position index (with a bound check) */
    T &operator[] (size_t index) {
      GBE_ASSERT(index < this->size());
      return parent_type::operator[] (index);
    }
    /*! Get element at position index (with a bound check) */
    const T &operator[] (size_t index) const {
      GBE_ASSERT(index < this->size());
      return parent_type::operator[] (index);
    }
    GBE_CLASS(vector);
  };
} /* namespace gbe */

#endif /* __GBE_VECTOR_HPP__ */

