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

#ifndef __PF_SET_HPP__
#define __PF_SET_HPP__

#include "sys/platform.hpp"
#include <set>

namespace pf
{
  /*! Add our custom allocator to std::set */
  template<class Key, class Pred = std::less<Key>>
  class set : public std::set<Key,Pred,Allocator<Key>>
  {
  public:
    // Typedefs
    typedef Key value_type;
    typedef Allocator<value_type> allocator_type;
    typedef std::set<Key,Pred,Allocator<Key>> parent_type;
    typedef Key key_type;
    typedef Pred key_compare;

    /*! Default constructor */
    INLINE set(const key_compare &comp = key_compare(),
               const allocator_type &a = allocator_type()) :
      parent_type(comp, a) {}
    /*! Iteration constructor */
    template<class InputIterator>
    INLINE set(InputIterator first,
               InputIterator last,
               const key_compare &comp = key_compare(),
               const allocator_type& a = allocator_type()) :
      parent_type(first, last, comp, a) {}
    /*! Copy constructor */
    INLINE set(const set& x) : parent_type(x) {}
    PF_CLASS(set);
  };

} /* namespace pf */

#endif /* __PF_SET_HPP__ */

