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
 * \file hash_map.hpp
 *
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */

#ifndef __GBE_HASH_MAP_HPP__
#define __GBE_HASH_MAP_HPP__

#include "sys/platform.hpp"

#ifdef __MSVC__
#include <unordered_map>
#else
#include <tr1/unordered_map>
#endif /* __MSVC__ */

namespace gbe
{
  /*! Add specific allocator to the hash map */
  template <class Key,
            class T,
            class Hash = std::hash<Key>,
            class Pred = std::equal_to<Key>>
  class hash_map : public std::tr1::unordered_map<Key,T,Hash,Pred,Allocator<std::pair<const Key,T>>>,
                   public NonCopyable
  {
  public:
    // Typedefs
    typedef std::pair<const Key, T> value_type;
    typedef Allocator<value_type> allocator_type;
    typedef std::tr1::unordered_map<Key,T,Hash,Pred,allocator_type> parent_type;
    typedef typename allocator_type::size_type size_type;
    typedef Key key_type;
    typedef T mapped_type;
    typedef Hash hasher;
    typedef Pred key_equal;

    /*! Default constructor */
    INLINE explicit hash_map(size_type n = 3,
                             const hasher& hf = hasher(),
                             const key_equal& eql = key_equal(),
                             const allocator_type& a = allocator_type()) :
      parent_type(n, hf, eql, a) {}
    /*! Iteration constructor */
    template <class InputIterator>
    INLINE hash_map(InputIterator first,
                    InputIterator last,
                    size_type n = 3,
                    const hasher& hf = hasher(),
                    const key_equal& eql = key_equal(),
                    const allocator_type& a = allocator_type()) :
      parent_type(first,last,n,hf,eql,a) {}
#if 0
    /*! Copy constructor */
    INLINE hash_map(const hash_map &other) : parent_type(other) {}
#endif
    GBE_CLASS(hash_map);
  };
} /* namespace gbe */

#endif /* __GBE_HASH_MAP_HPP__ */

