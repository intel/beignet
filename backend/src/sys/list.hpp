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
 * \file list.hpp
 *
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */
#ifndef __GBE_LIST_HPP__
#define __GBE_LIST_HPP__

#include "sys/platform.hpp"
#include <list>

namespace gbe
{
  /*! Use custom allocator instead of std one */
  template <typename T>
  class list : public std::list<T, Allocator<T>>
  {
  public:
    // Typedefs
    typedef T value_type;
    typedef Allocator<value_type> allocator_type;
    typedef std::list<T, allocator_type> parent_type;
    typedef typename allocator_type::size_type size_type;

    /*! Default constructor */
    INLINE explicit list(const allocator_type &a = allocator_type()) :
      parent_type(a) {}
    /*! Repetitive constructor */
    INLINE explicit list(size_type n,
                         const T &value = T(),
                         const allocator_type &a = allocator_type()) :
      parent_type(n, value, a) {}
    /*! Iteration constructor */
    template <class InputIterator>
    INLINE list(InputIterator first,
                InputIterator last,
                const allocator_type &a = allocator_type()) :
      parent_type(first, last, a) {}
    /*! Copy constructor */
    INLINE list(const list &x) : parent_type(x) {}
    GBE_CLASS(list);
  };
} /* namespace gbe */

#endif /* __GBE_LIST_HPP__ */

