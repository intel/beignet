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

#ifndef __GBE_ARRAY_HPP__
#define __GBE_ARRAY_HPP__

#include "sys/platform.hpp"
#include <vector>

namespace gbe
{
  /*! Non resizable array with no checking. We make it non-copiable right now
   *  since we do not want to implement an expensive deep copy
   */
  template<class T>
  class array
  {
  public:
    /*! Create an empty array */
    INLINE array(void) : elem(NULL), elemNum(0) {}
    /*! Allocate an array with elemNum allocated elements */
    INLINE array(size_t elemNum) : elem(NULL), elemNum(0) { this->resize(elemNum); }
    /*! Copy constructor */
    INLINE array(const array &other) {
      this->elemNum = other.elemNum;
      if (this->elemNum) {
        this->elem = GBE_NEW_ARRAY(T, this->elemNum);
        for (size_t i = 0; i < this->elemNum; ++i) this->elem[i] = other.elem[i];
      } else
        this->elem = NULL;
    }
    /*! Assignment operator */
    INLINE array& operator= (const array &other) {
      if (this->elem != NULL && this->elemNum != other->elemNum) {
        GBE_DELETE_ARRAY(this->elem);
        this->elem = NULL;
        this->elemNum = 0;
      }
      this->elemNum = other.elemNum;
      if (this->elemNum) {
        if (this->elem == NULL)
          this->elem = GBE_NEW_ARRAY(T, this->elemNum);
        for (size_t i = 0; i < this->elemNum; ++i) this->elem[i] = other.elem[i];
      } else
        this->elem = NULL;
      return *this;
    }
    /*! Delete the allocated elements */
    INLINE ~array(void) { GBE_SAFE_DELETE_ARRAY(elem); }
    /*! Free the already allocated elements and allocate a new array */
    INLINE void resize(size_t elemNum_) {
      if (elemNum_ != this->elemNum) {
        GBE_SAFE_DELETE_ARRAY(elem);
        if (elemNum_)
          this->elem = GBE_NEW_ARRAY(T, elemNum_);
        else
          this->elem = NULL;
        this->elemNum = elemNum_;
      }
    }
    /*! Steal the pointer. The array becomes emtpy */
    INLINE T *steal(void) {
      T *stolen = this->elem;
      this->elem = NULL;
      this->elemNum = 0;
      return stolen;
    }
    /*! First element */
    INLINE T *begin(void) { return this->elem; }
    /*! First non-valid element */
    INLINE T *end(void) { return this->elem + elemNum; }
    /*! Get element at position index (with a bound check) */
    INLINE T &operator[] (size_t index) {
      GBE_ASSERT(elem && index < elemNum);
      return elem[index];
    }
    /*! Get element at position index (with bound check) */
    INLINE const T &operator[] (size_t index) const {
      GBE_ASSERT(elem && index < elemNum);
      return elem[index];
    }
    /*! Return the number of elements */
    INLINE size_t size(void) const { return this->elemNum; }
  private:
    T *elem;        //!< Points to the elements
    size_t elemNum; //!< Number of elements in the array
    GBE_CLASS(array);
  };
} /* namespace gbe */

#endif /* __GBE_ARRAY_HPP__ */

