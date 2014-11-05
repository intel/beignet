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
 * \file map.hpp
 *
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */
#ifndef __GBE_MAP_HPP__
#define __GBE_MAP_HPP__

#include "sys/platform.hpp"
#include <map>

namespace gbe
{
  /*! Use custom allocator instead of std one */
  template<class Key, class T, class Pred = std::less<Key>>
  class map : public std::map<Key,T,Pred,Allocator<std::pair<const Key, T>>>,
              public NonCopyable
  {
  public:
    // Typedefs
    typedef std::pair<const Key, T> value_type;
    typedef Allocator<value_type> allocator_type;
    typedef std::map<Key,T,Pred,allocator_type> parent_type;
    typedef Key key_type;
    typedef T mapped_type;
    typedef Pred key_compare;
    typedef typename allocator_type::pointer pointer;
    typedef typename allocator_type::const_pointer const_pointer;
    typedef typename allocator_type::reference reference;
    typedef typename allocator_type::const_reference const_reference;

    /*! Default constructor */
    INLINE map(const key_compare &comp = key_compare(),
               const allocator_type &a = allocator_type()) :
      parent_type(comp, a) {}
    /*! Iteration constructor */
    template<class InputIterator>
    INLINE map(InputIterator first,
               InputIterator last,
               const key_compare &comp = key_compare(),
               const allocator_type& a = allocator_type()) :
      parent_type(first, last, comp, a) {}
#if 0
    /*! Copy constructor */
    INLINE map(const map& x) : parent_type(x) {}
#endif
    /*! Better than using find if we do not care about the iterator itself */
    INLINE bool contains(const Key &key) const {
      return this->find(key) != this->end();
    }
    GBE_CLASS(map);
  };
} /* namespace gbe */

#endif /* __GBE_MAP_HPP__ */

